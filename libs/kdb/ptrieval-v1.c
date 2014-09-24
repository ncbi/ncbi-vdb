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
#include "idstats-priv.h"

#include <kdb/index.h>
#include <klib/ptrie.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KPTrieIndexCCParms
 *  consistency check parameter block
 */
typedef struct KPTrieIndexCCParms_v1 KPTrieIndexCCParms_v1;
struct KPTrieIndexCCParms_v1
{
    KIdStats stats;
    rc_t rc;
    const KPTrieIndex_v1 *self;
    const KIndex *outer;
    bool key2id;
    bool id2key;
    bool failed;
};

/* Init
 */
static
void KPTrieIndexCCParmsInit_v1 ( KPTrieIndexCCParms_v1 *pb,
    const KPTrieIndex_v1 *self, const KIndex *outer, bool key2id, bool id2key )
{
    KIdStatsInit ( & pb -> stats );
    pb -> rc = 0;
    pb -> self = self;
    pb -> outer = outer;
    pb -> key2id = key2id;
    pb -> id2key = self -> id2node ? id2key : false;
    pb -> failed = false;
}

/* Whack
 */
static
void KPTrieIndexCCParmsWhack_v1 ( KPTrieIndexCCParms_v1 *pb )
{
    KIdStatsWhack ( & pb -> stats );
}

/*--------------------------------------------------------------------------
 * KPTrieIndex_v1
 *  persisted keymap
 */

/* CheckConsistency
 *  runs a consistency check as efficiently as possible
 *
 *  in all cases, we make use of "PTrieForEach" to visit each node
 *  using the natural storage order. each node returned is counted,
 *  read and inserted into a BSTree whose nodes merge adjacent and
 *  overlapping ids into existing nodes.
 *
 *  if running a deep "key->id" test, then the key is first regenerated
 *  from the node, and then used to retrieve the id via the KIndex.
 *
 *  if the projection index exists, the id is tested against the node
 *  to ensure that projection works. if "id->key" is true, then use
 *  the KIndex to produce a key and compare it against the node.
 */
static
bool CC KPTrieIndexCCVisit_v1 ( PTNode *n, void *data )
{
    KPTrieIndexCCParms_v1 *pb = data;
    const KPTrieIndex_v1 *self = pb -> self;

    /* payload of v1 PTNode is a 32-bit spot id */
    uint32_t id;
    assert ( n -> data . size == sizeof id );
    memcpy ( & id, n -> data . addr, sizeof id );
    if ( self -> byteswap )
        id = bswap_32 ( id );

    /* record the node, row id, and range of 1 */
    pb -> rc = KIdStatsInsert ( & pb -> stats, id, 1 );
    if ( pb -> rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, pb -> rc, "failed when examining node id $(nid) with row id $(rid)",
                              "nid=0x%08x,rid=%u", n -> id, id ));
        return true;
    }

    /* if we have a projection index, test it */
    if ( self -> id2node != NULL )
    {
        if ( id < self -> first || id > self -> last )
        {
            PLOGMSG ( klogWarn, ( klogWarn, "node id $(nid) with row id $(rid) is not within projection range of $(min_rid)..$(max_rid)",
                                 "nid=0x%08x,rid=%u,min_rid=%u,max_rid=%u", n -> id, id, self -> first, self -> last ));
            pb -> failed = true;
            return false;
        }
        if ( self -> id2node [ id - self -> first ] != n -> id )
        {
            PLOGMSG ( klogWarn, ( klogWarn, "node id $(nid) with row id $(rid) does not match projection node id of $(pnid)",
                                 "nid=0x%08x,rid=%u,pnid=0x%08x", n -> id, id, self -> id2node [ id - self -> first ] ));
            pb -> failed = true;
            return false;
        }
    }

    /* if performing deeper tests */
    if ( pb -> key2id || pb -> id2key )
    {
        rc_t rc;
        int64_t start_id;
        uint64_t id_count;

        /* need to recover key from given node */
        const String *orig;
        pb -> rc = PTNodeMakeKey ( n, & orig );
        if ( pb -> rc != 0 )
        {
            PLOGERR ( klogSys, ( klogSys, pb -> rc, "failed when retrieving text for node id $(nid) with row id $(rid)",
                                 "nid=0x%08x,rid=%u", n -> id, id ));
            return true;
        }

        /* key->id test */
        if ( pb -> key2id )
        {
            rc = KIndexFindText ( pb -> outer, orig -> addr, & start_id, & id_count, NULL, NULL );
            if ( rc != 0 )
            {
                PLOGERR ( klogWarn, ( klogWarn, rc, "failed to retrieve start id and count for key '$(key)', row id $(rid)",
                                      "key=%S,rid=%u", orig, id ) );
                pb -> failed = true;
            }
            else if ( start_id != ( int64_t ) id || id_count != 1 )
            {
                PLOGERR ( klogWarn, ( klogWarn, rc, "key '$(key)' maps to start id $(start_id), count $(id_count): expected id $(rid), count 1.",
                                      "key=%S,rid=%u,start_id=%ld,id_count=%lu", orig, id, start_id, id_count ) );
                pb -> failed = true;
            }
        }

        /* id->key test */
        if ( pb -> id2key )
        {
            char buffer [ 256 ], *key = buffer;
            size_t key_size, bsize = sizeof buffer;
            if ( sizeof buffer <= orig -> size )
            {
                key = malloc ( bsize = orig -> size + 1 );
                if ( key == 0 )
                {
                    pb -> rc = RC ( rcDB, rcIndex, rcValidating, rcMemory, rcExhausted );
                    StringWhack ( ( String* ) orig );
                    return true;
                }
            }

            rc = KIndexProjectText ( pb -> outer, id, & start_id, & id_count, key, bsize, & key_size );
            if ( rc != 0 )
            {
                PLOGERR ( klogWarn, ( klogWarn, rc, "failed to retrieve key, start id and count for row id $(rid)",
                                      "rid=%u", id ) );
                pb -> failed = true;
            }
            else
            {
                if ( orig -> size != key_size || memcmp ( orig -> addr, key, key_size ) != 0 )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "row $(rid) maps to key '$(key)': expected key '$(orig)'.",
                                          "rid=%u,key=%.*s,orig=%S", id, ( int ) key_size, key, orig ) );
                    pb -> failed = true;
                }
                if ( start_id != ( int64_t ) id || id_count != 1 )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "single row $(rid) maps to start id $(start_id), count $(id_count).",
                                          "rid=%u,id_count=%u,start_id=%ld", id, id_count, start_id ) );
                    pb -> failed = true;
                }
            }

            if ( key != buffer )
                free ( key );
        }

        StringWhack ( ( String* ) orig );
    }

    return false;
}

rc_t KPTrieIndexCheckConsistency_v1 ( const KPTrieIndex_v1 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    const KIndex *outer, bool key2id, bool id2key )
{
    rc_t rc = 0;
    KPTrieIndexCCParms_v1 pb;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcParam, rcNull );

    if ( ( key2id || id2key ) && outer == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcSelf, rcNull );

    KPTrieIndexCCParmsInit_v1 ( & pb, self, outer, key2id, id2key );
    if ( PTrieDoUntil ( self -> key2id, KPTrieIndexCCVisit_v1, & pb ) )
        rc = pb . rc;
    else if ( pb . failed )
        rc = RC ( rcDB, rcIndex, rcValidating, rcSelf, rcCorrupt );

    if ( start_id != NULL )
        * start_id = pb . stats . i_min_id;
    if ( id_range != NULL )
        * id_range = ( uint32_t ) ( pb . stats . x_max_id - pb . stats . i_min_id );
    if ( num_keys != NULL )
        * num_keys = pb . stats . num_entries;
    if ( num_rows != NULL )
        * num_rows = pb . stats . num_ids;
    if ( num_holes != NULL )
        * num_holes = pb . stats . num_holes;

    KPTrieIndexCCParmsWhack_v1 ( & pb );
    return rc;
}
