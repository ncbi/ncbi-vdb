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

#include "rindex.h"
#include "kdbfmt.h"

#include <klib/ptrie.h>
#include <klib/text.h>
#include <klib/rc.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>

#include <sysalloc.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <byteswap.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KRTrieIndex_v1
 */

/* whack whack */
void KRTrieIndexWhack_v1 ( KRTrieIndex_v1 *self )
{
    KPTrieIndexWhack_v1 ( & self -> pt );
}

/* initialize an index from file */
rc_t KRTrieIndexOpen_v1 ( KRTrieIndex_v1 *self, const KMMap *mm, bool byteswap )
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

    KRTrieIndexWhack_v1 ( self );
    return rc;
}

/* map key to id ( was Key2Id ) */
rc_t KRTrieIndexFind_v1 ( const KRTrieIndex_v1 *self, const char *str, uint32_t *id,
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
            memmove ( id, n . data . addr, sizeof * id );
            return 0;
        }
    }

    return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNotFound );
}

/* projection index id to key-string ( was Id2Key ) */
rc_t KRTrieIndexProject_v1 ( const KRTrieIndex_v1 *self,
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
