/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#include <kdb/extern.h>
#include "index-priv.h"
#include "kdbfmt-priv.h"
#include <klib/ptrie.h>
#include <klib/text.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <byteswap.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KPTrieIndex_v1
 *  persisted keymap
 */

/* KPTrieIndexInit
 *  opens and initializes persisted keymap structure
 */
rc_t KPTrieIndexInit_v1 ( KPTrieIndex_v1 *self, const KMMap *mm, bool byteswap )
{
    size_t size;
    rc_t rc = KMMapSize ( mm, & size );
    if ( rc == 0 )
    {
        const KDBHdr *hdr;
        rc = KMMapAddrRead ( mm, ( const void** ) & hdr );
        if ( rc == 0 )
        {
            /* try to create the pttree */
            rc = PTrieMakeOrig ( & self -> key2id,
                hdr + 1, size -= sizeof * hdr, byteswap );
            if ( rc == 0 )
            {
                size_t ptsize = PTrieSize ( self -> key2id );
                if ( ptsize <= size )
                {
                    /* just in case */
                    self -> mm = NULL;

                    /* record for projection */
                    self -> byteswap = byteswap;

                    /* it could be stored without projection */
                    if ( ptsize == size )
                    {
                        self -> id2node = NULL;
                        self -> first = self -> last = 0;
                        return 0;
                    }
                            
                    /* assume this is projection index */
                    self -> id2node = ( void* )
                        ( ( char* ) ( hdr + 1 ) + ptsize );
                    size -= ptsize;
                            
                    /* it must have at least 4 bytes
                       and be 4 byte aligned */
                    if ( size >= sizeof ( uint32_t ) && ( size & 3 ) == 0 )
                    {
                        /* first entry is starting key
                           remaining entries are node ids */
                        self -> first = * self -> id2node ++;
                        size -= sizeof self -> id2node [ 0 ];
                        if ( size == 0 )
                        {
                            /* forget if empty */
                            self -> id2node = NULL;
                            self -> first = self -> last = 0;
                            return 0;
                        }
                        /* remaining entries */
                        self -> last = self -> first + ( size >> 2 ) - 1;
                        return 0;
                    }
                }

                PTrieWhack ( self -> key2id );
                self -> key2id = NULL;

                rc = RC ( rcDB, rcIndex, rcConstructing, rcTrie, rcCorrupt );
            }
        }
    }

    return rc;
}

/* KPTrieIndexWhack_v1
 *  closes down keymap
 */
void KPTrieIndexWhack_v1 ( KPTrieIndex_v1 *self )
{
    PTrieWhack ( self -> key2id );
    KMMapRelease ( self -> mm );
    memset ( self, 0, sizeof * self );
}


/*--------------------------------------------------------------------------
 * KTrieIndex_v1
 */

/* whack whack */
void KTrieIndexWhack_v1 ( KTrieIndex_v1 *self )
{
    KPTrieIndexWhack_v1 ( & self -> pt );
}

/* initialize an index from file */
rc_t KTrieIndexOpen_v1 ( KTrieIndex_v1 *self, const KMMap *mm, bool byteswap )
{
    rc_t rc;

    memset ( self, 0, sizeof * self );

    rc = KPTrieIndexInit_v1 ( & self -> pt, mm, byteswap );
    if ( rc == 0 )
    {
        rc = KMMapAddRef ( mm );
        if ( rc == 0 )
        {
            self -> pt . mm = mm;
            return 0;
        }
    }

    KTrieIndexWhack_v1 ( self );
    return rc;
}

/* map key to id ( was Key2Id ) */
rc_t KTrieIndexFind_v1 ( const KTrieIndex_v1 *self, const char *str, uint32_t *id,
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void * data )
{
    if ( self -> pt . key2id != NULL )
    {
        PTNode n;
        String key;
        unsigned int nid;

        StringInitCString ( & key, str );
        nid = PTrieFind ( self -> pt . key2id, & key, & n, custom_cmp, data );
        if ( nid != 0 )
        {
            assert ( n . data . size == sizeof * id );
            memcpy ( id, n . data . addr, sizeof * id );
            return 0;
        }
    }

    return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
}

/* projection index id to key-string ( was Id2Key ) */
rc_t KTrieIndexProject_v1 ( const KTrieIndex_v1 *self,
    uint32_t id, char *key_buff, size_t buff_size, size_t *actsize)
{
    if ( self -> pt . id2node != NULL &&
         id >= self -> pt . first &&
         id <= self -> pt . last )
    {
        PTNode n;
        uint32_t node = self -> pt . id2node [ id - self -> pt . first ];
        rc_t rc = PTrieGetNode ( self -> pt . key2id,
            & n, self -> pt . byteswap ? bswap_32 ( node ) : node );
        if ( rc == 0 )
        {
            const String *key;
            rc = PTNodeMakeKey ( & n, & key );
            if ( rc == 0 )
            {
                if ( actsize != NULL )
                    * actsize = key -> size;
                if ( key -> size >= buff_size )
                    rc = RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcInsufficient );
                else
                    string_copy ( key_buff, buff_size, key -> addr, key -> size );

                StringWhack ( ( String* ) key );
            }
            return rc;
        }
    }

    return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );
}
