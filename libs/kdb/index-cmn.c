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

#include "index-cmn.h"

#include <klib/ptrie.h>
#include <klib/rc.h>

#include <kfs/mmap.h>

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
                        self -> last = self -> first + (uint32_t)( size >> 2 ) - 1;
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

#include <kdb/index.h>
#include <byteswap.h>
rc_t KIndexValidateHeader(bool *reversedByteOrder, uint32_t *type, void const *data, size_t size)
{
    union
    {
        KIndexFileHeader_v1 v1;
        KIndexFileHeader_v2 v2;
        KIndexFileHeader_v3 v3;
    } u;

    if (size < sizeof(u.v1))
        return RC(rcDB, rcIndex, rcValidating, rcData, rcTooShort);
    if (size > sizeof(u))
        size = sizeof(u);

    memset(&u, 0, sizeof(u));
    memmove(&u, data, size);

    switch (u.v1.endian) {
    case eByteOrderTag:
        *reversedByteOrder = false;
        break;
    case eByteOrderReverse:
        *reversedByteOrder = true;
        u.v1.version = bswap_32(u.v1.version);
        break;
    default:
        return RC(rcDB, rcIndex, rcValidating, rcByteOrder, rcIncorrect);
    }
    switch (u.v1.version) {
    case 1:
    case 2:
        u.v3.index_type = kitText;
        break;
    case 3:
    case 4:
        if (*reversedByteOrder)
            u.v3.index_type = bswap_32(u.v3.index_type);
        break;
    default:
        return RC(rcDB, rcIndex, rcValidating, rcIndex, rcBadVersion);
    }
    switch (u.v3.index_type) {
    case kitText:
        *type = kitText;
        break;
    case kitU64:
        *type = kitU64;
        break;
    default:
        return RC(rcDB, rcIndex, rcValidating, rcIndex, rcUnrecognized);
    }
    return 0;
}
