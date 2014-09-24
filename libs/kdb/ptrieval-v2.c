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
#include <klib/pack.h>
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
typedef struct KPTrieIndexCCParms_v2 KPTrieIndexCCParms_v2;
struct KPTrieIndexCCParms_v2
{
    KIdStats stats;
    rc_t rc;
    const KPTrieIndex_v2 *self;
    const KIndex *outer;
    bool key2id;
    bool id2key;
    bool all_ids;
    bool convertFromV1;
    bool failed;
};

/* Init
 */
static
void KPTrieIndexCCParmsInit_v2 ( KPTrieIndexCCParms_v2 *pb,
    const KPTrieIndex_v2 *self, const KIndex *outer, bool key2id, bool id2key, bool all_ids, bool convertFromV1 )
{
    KIdStatsInit ( & pb -> stats );
    pb -> rc = 0;
    pb -> self = self;
    pb -> outer = outer;
    pb -> key2id = key2id;
    pb -> id2key = self -> ord2node ? id2key : false;
    pb -> all_ids = all_ids;
    pb -> convertFromV1 = convertFromV1;
    pb -> failed = false;
}

/* Whack
 */
static
void KPTrieIndexCCParmsWhack_v2 ( KPTrieIndexCCParms_v2 *pb )
{
    KIdStatsWhack ( & pb -> stats );
}

/*--------------------------------------------------------------------------
 * KPTrieIndex_v2
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
bool CC KPTrieIndexCCVisit_v2 ( PTNode *n, void *data )
{
    KPTrieIndexCCParms_v2 *pb = (KPTrieIndexCCParms_v2 *)data;
    const KPTrieIndex_v2 *self = pb -> self;

    rc_t rc;
    int64_t id;
    size_t usize;
    uint64_t span;
    uint32_t i, ord;

    /* detect conversion from v1 */
    if ( pb -> convertFromV1 && self -> id_bits == 0 )
    {
        uint32_t id32;

        /* payload of v1 PTNode is a 32-bit spot id */
        assert ( n -> data . size == sizeof id32 );
        memcpy ( & id32, n -> data . addr, sizeof id32 );
        id = self -> byteswap ? bswap_32 ( id32 ) : id32;
    }
    else
    {
        /* native v2 */
        /* TBD - should this pass n -> data . size * 8 ??? */
        if ( self -> id_bits != 0 )
        {
            rc = Unpack ( self -> id_bits, sizeof id * 8,
                n -> data . addr, 0, self -> id_bits, NULL,
                & id, sizeof id, & usize );
            if ( rc != 0 )
            {
                PLOGMSG ( klogWarn, ( klogWarn, "could not determine row id of v2 node $(nid)",
                                 "nid=0x%08x", n -> id ));
                pb -> failed = true;
                return false;
            }
        }
        else
        {
            id = 0;
        }

        id += self -> first;
    }

    /* convert from row-id to 1-based ordinal index */
    ord = KPTrieIndexID2Ord_v2 ( self, id );
    if ( ord == 0 )
    {
        /* 0 means not found */
        PLOGMSG ( klogWarn, ( klogWarn, "v2 node $(nid): row id $(rid) not found in trie",
                              "nid=0x%08x,rid=%ld", n -> id, id ));
        pb -> failed = true;
        return false;
    }

    if ( self -> ord2node != NULL )
    {
        /* to calculate span of last entry, where
           1-based "ord" == the number of nodes,
           just find the distance between the last row-id
           in index ( self->maxid ) and the current row-id */
        if ( ord == self -> count )
            span = self -> maxid - id + 1;
        else
        {
            /* from here on, we will use "ord" to be the
               ZERO-BASED index of the slot FOLLOWING the
               one corresponding to id. we want to find the
               row-id AFTER the current one and calculate distance */
            switch ( self -> variant )
            {
            /* linear array */
            case 0:
                /* starting with the FOLLOWING slot,
                   count duplicate entries */
                for ( i = ord; i < self -> count; ++ i )
                {
                    if ( n -> id != self -> ord2node [ i ] )
                        break;
                }
                span = self -> first + i - id;
                break;

            /* packed ordered array of sparse row-ids from here on
               we already have "id" for this node, so the span will
               be the distance from NEXT id - 1-based ord is 0-based next */
            case 1:
                span = self -> first + self -> id2ord . v8 [ ord ] - id;
                break;
            case 2:
                span = self -> first + self -> id2ord . v16 [ ord ] - id;
                break;
            case 3:
                span = self -> first + self -> id2ord . v32 [ ord ] - id;
                break;
            case 4:
                span = self -> first + self -> id2ord . v64 [ ord ] - id;
                break;
            default:
                PLOGMSG ( klogErr, ( klogErr, "PTrie v2 index has bad variant code: $(variant)",
                                     "variant=%u", self -> variant ));
                pb -> rc = RC ( rcDB, rcIndex, rcValidating, rcIndex, rcCorrupt );
                return true;
            }
        }
    }
    else if ( self -> span_bits == 0 )
        span = 1;
    else
    {
        /* TBD - this case is never used in practice.
           it would be an skey without a projection index */
        rc = Unpack ( self -> span_bits, sizeof span * 8,
                      n -> data . addr, 0, self -> id_bits, NULL,
                      & span, sizeof span, & usize );
        if ( rc != 0 )
        {
            PLOGMSG ( klogWarn, ( klogWarn, "could not determine span of v2 node $(nid), row id $(rid)",
                                  "nid=0x%08x,rid=%ld", n -> id, id ));
            pb -> failed = true;
            return false;
        }
    }

    /* record the node, row id, span */
    pb -> rc = KIdStatsInsert ( & pb -> stats, id, span );
    if ( pb -> rc != 0 )
    {
        PLOGERR ( klogSys, ( klogSys, pb -> rc, "failed when examining node id $(nid) with row id $(rid), span $(span)",
                             "nid=0x%08x,span=%u,rid=%ld", n -> id, span, id ));
        return true;
    }

    /* if we have a projection index, test it */
    if ( self -> ord2node != NULL )
    {
        if ( id < self -> first || id > self -> last )
        {
            PLOGMSG ( klogWarn, ( klogWarn, "node id $(nid) with row id $(rid) is not within projection range of $(min_rid)..$(max_rid)",
                                 "nid=0x%08x,rid=%ld,min_rid=%ld,max_rid=%ld",
                                  n -> id, id, self -> first, self -> last ));
            pb -> failed = true;
            return false;
        }
        for ( i = 0; i < span; ++ i )
        {
            if ( self -> ord2node [ i + ord - 1 ] != n -> id )
            {
                PLOGMSG ( klogWarn, ( klogWarn, "node id $(nid) with row id $(rid) does not match projection node id of $(pnid)",
                                      "nid=0x%08x,rid=%ld,pnid=0x%08x", n -> id, id + 1, self -> ord2node [ i + ord - 1 ] ));
                pb -> failed = true;
                return false;
            }
            if ( ! pb -> all_ids || self -> variant != 0 )
                break;
        }
    }

    /* if performing deeper tests */
    if ( pb -> key2id || pb -> id2key )
    {
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
            else if ( start_id != ( int64_t ) id || id_count != span )
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
                key = (char *)malloc ( bsize = orig -> size + 1 );
                if ( key == 0 )
                {
                    pb -> rc = RC ( rcDB, rcIndex, rcValidating, rcMemory, rcExhausted );
                    StringWhack ( ( String* ) orig );
                    return true;
                }
            }

            for ( i = 0; i < span; ++ i )
            {
                rc = KIndexProjectText ( pb -> outer, id + i, & start_id, & id_count, key, bsize, & key_size );
                if ( rc != 0 )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "failed to retrieve key, start id and count for row id $(rid)",
                                          "rid=%u", id + i ) );
                    pb -> failed = true;
                    break;
                }

                if ( orig -> size != key_size || memcmp ( orig -> addr, key, key_size ) != 0 )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "row $(rid) maps to key '$(key)': expected key '$(orig)'.",
                                          "rid=%u,key=%.*s,orig=%S", id + i, ( int ) key_size, key, orig ) );
                    pb -> failed = true;
                }
                if ( start_id != id || id_count != ( uint64_t ) span )
                {
                    PLOGERR ( klogWarn, ( klogWarn, rc, "row $(rid) maps to start id $(start_id), count $(id_count): expected $(row_start), $(span).",
                                          "rid=%u,id_count=%lu,start_id=%ld,row_start=%ld,span=%u",
                                          id, id_count, start_id, id, span ) );
                    pb -> failed = true;
                }

                if ( ! pb -> all_ids || pb -> failed )
                    break;
            }

            if ( key != buffer )
                free ( key );
        }

        StringWhack ( ( String* ) orig );
    }

    return false;
}

rc_t KPTrieIndexCheckConsistency_v2 ( const KPTrieIndex_v2 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    const KIndex *outer, bool key2id, bool id2key, bool all_ids, bool convertFromV1 )
{
    rc_t rc = 0;
    KPTrieIndexCCParms_v2 pb;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcParam, rcNull );

    if ( ( key2id || id2key ) && outer == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcSelf, rcNull );

    KPTrieIndexCCParmsInit_v2 ( & pb, self, outer, key2id, id2key, all_ids, convertFromV1 );
    if ( PTrieDoUntil ( self -> key2id, KPTrieIndexCCVisit_v2, & pb ) )
        rc = pb . rc;
    else if ( pb . failed )
        rc = RC ( rcDB, rcIndex, rcValidating, rcSelf, rcCorrupt );

    if ( start_id != NULL )
        * start_id = pb . stats . i_min_id;
    if ( id_range != NULL )
        * id_range = pb . stats . x_max_id - pb . stats . i_min_id;
    if ( num_keys != NULL )
        * num_keys = pb . stats . num_entries;
    if ( num_rows != NULL )
        * num_rows = pb . stats . num_ids;
    if ( num_holes != NULL )
        * num_holes = pb . stats . num_holes;

    KPTrieIndexCCParmsWhack_v2 ( & pb );
    return rc;
}
