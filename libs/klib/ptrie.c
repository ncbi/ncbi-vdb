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

#include <klib/extern.h>
#include "pbstree-priv.h"
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <byteswap.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * PTNode
 *  a node within text tree
 *
 *  a TTree will contain TTNodes, which themselves are intrusive but
 *  irrelevant internal tree links plus a key string plus an externally
 *  defined data structure, representing a value, and supporting multiple
 *  nodes per key. the internal links permit navigation from node
 *  to node that is not possible with the PTNode. furthermore, they
 *  provide direct access to the key string that is not normally stored
 *  verbatim within a persisted image.
 *
 *  a PTrie does not contain structured nodes, but stores linkage
 *  and ordering information separately from the externally defined
 *  data, and rather than using pointers, assigns integer ids to the
 *  nodes themselves. navigation by pointers would require allocations,
 *  which are unnecessary and expensive when traversing a read-only
 *  persisted image. navigation is therefore intrusive on an externally
 *  allocated node structure.
 */

/* PTNodeMakeKey
 *  tries to may a key string from node
 *  will fail if key text was not embedded into image when created
 *
 *  "key" [ OUT ] - return parameter for a key string allocation
 *  that must be whacked with StringWhack when no longer needed.
 *
 *  return status codes:
 *    EINVAL => invalid parameter
 *    ENOENT => no key data were written to tree image
 *    ENOMEM => memory failure
 */
LIB_EXPORT rc_t CC PTNodeMakeKey ( const PTNode *self, const String **keyp )
{
    rc_t rc;
    if ( keyp == NULL )
        rc = RC ( rcCont, rcNode, rcAccessing, rcParam, rcNull );
    else
    {
        * keyp = NULL;

        if ( self == NULL )
            rc = RC ( rcCont, rcNode, rcAccessing, rcSelf, rcNull );
        else
        {
            const PTrie *tt = self -> internal;
            if ( tt->ext_keys || ! tt->backtrace )
                rc = RC ( rcCont, rcNode, rcAccessing, rcString, rcNotFound );
            else
            {
                uint32_t tid, btid;

                /* decode the node id into trans and btnode ids */
                rc = PTrieDecodeNodeId ( tt, self -> id, & tid, & btid );
                if ( rc == 0 )
                {
                    /* recover the trans node */
                    PTTrans trans;
                    rc = PTrieInitNode ( tt, & trans, tid );
                    if ( rc == 0 )
                    {
                        size_t size;
                        const void *addr;

                        assert ( trans . vals != NULL );
                        assert ( PBSTreeCount ( trans . vals ) >= btid );

                        /* recover the value node */
                        rc = PBSTreeGetNodeData ( trans . vals, & addr, & size, btid );
                        if ( rc == 0 )
                        {
                            /* the depth of this transition is
                               the length of the key string up to node */
                            uint32_t depth = PTTransGetDepth ( & trans, tt );

                            /* get an adequate UTF-32 buffer */
                            uint32_t tbbuff [ 64 ], *traceback = tbbuff;
                            if ( depth > sizeof tbbuff / sizeof tbbuff [ 0 ] )
                                traceback = malloc ( depth * sizeof traceback [ 0 ] );

                            /* complete the work */
                            if ( traceback != NULL )
                            {
                                String *key;
                                uint32_t nlen;
                                size_t tbsize, nsize;

                                uint32_t i;
                                for ( i = depth; i > 0; )
                                {
                                    uint32_t zidx = PTTransGetTransCharCode ( & trans, tt );
                                    traceback [ -- i ] =
                                        ( zidx + 1 == tt -> unmapped_idx ) ?
                                        ( uint32_t ) '?' : PTrieDecodeCharId ( tt, zidx );
                                    tid = PTTransGetDad ( & trans, tt );
                                    assert ( tid != 0 || i == 0 );
                                    rc = PTrieInitNode ( tt, & trans, tid );
                                    assert ( rc == 0 );
                                }

                                /* determine the size of traceback string */
                                i = utf32_cvt_string_len ( traceback,
                                    depth * sizeof traceback [ 0 ], & tbsize );
                                assert ( i == depth );

                                /* determine the size and length of node string */
                                nlen = string_measure ( addr, & nsize );
                                assert ( nsize < size );

                                /* create the string object */
                                key = malloc ( sizeof * key + tbsize + nsize + 1 );
                                if ( key == NULL )
                                    rc = RC ( rcCont, rcNode, rcAccessing, rcMemory, rcExhausted );
                                else
                                {
                                    /* home free */
                                    char *text = ( char* ) ( key + 1 );
                                    tbsize = utf32_cvt_string_copy ( text, tbsize + 1,
                                        traceback, depth * sizeof traceback [ 0 ] );
                                    memmove ( & text [ tbsize ], addr, nsize + 1 );
                                    StringInit ( key, text, tbsize + nsize, depth + nlen );
                                    * keyp = key;
                                }

                                if ( traceback != tbbuff )
                                    free ( traceback );
                            }
                        }
                    }
                }
            }
        }
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * PTTrans
 */

/* Whack
 */
void PTTransWhack ( PTTrans *trans )
{
    if ( trans != NULL ) while ( 1 )
    {
        PTTrans *doomed = trans;
        trans = ( PTTrans* ) trans -> back;
        if ( doomed -> vals != & doomed -> _val )
            PBSTreeWhack ( ( PBSTree* ) doomed -> vals );
        free ( doomed );
        if ( trans == NULL )
            break;
        if ( -- trans -> refcount > 0 )
            break;
    }
}

/* GetDepth
 */
#if ! RECORD_HDR_DEPTH
LIB_EXPORT uint32_t CC PTTransGetDepth ( const PTTrans *self, const PTrie *tt )
{
    PTTrans trans;
    uint32_t dad, depth;

    assert ( self != NULL );
    if ( self -> depth != 0 )
        return self -> depth;

    for ( depth = 0, dad = PTTransGetDad ( self, tt ); dad != 0;
          ++ depth, dad = PTTransGetDad ( & trans, tt ) )
    {
        int status = PTrieInitNode ( tt, & trans, dad );
        assert ( status == 0 );
    }

    return depth;
}
#endif

/* GetChildChildIdx
 */
static
uint32_t PTTransGetChildChildIdx ( const PTTrans *self,
    const PTrie *tt, uint32_t zidx )
{
    const uint8_t *child_seq_type = self -> child_seq_type;
    if ( self -> child_seq_type != NULL )
    {
        uint32_t i, j, k;

        /* terse variable name explanation:
           "i" will be the index into "child_seq_type" bit array
           "j" is a 1-based index into child trans array
           "k" is an index into the index array */
        for ( i = 0, j = 1, k = pttFirstIdx; i < self -> slen; ++ k, ++ j, ++ i )
        {
            /* get the single char code or
               the left edge of char code range */
            uint32_t left = PTTransGetIdx ( self, tt, k );

            /* detect terminal condition */
            if ( left >= zidx )
            {
                /* if single or range left is too high, there's no transition */
                if ( left > zidx )
                    break;

                /* the single or range left is an exact match
                   "j" is the 1-based index into child array */
                return j;
            }

            /* check entry type: if 0, then the index was a single
               value. if 1, then it's a range */
            if ( ( child_seq_type [ i >> 3 ] & ( 1U << ( i & 7 ) ) ) != 0 )
            {
                /* check for inclusion within the range
                   N.B. ensure that the increment of "k"
                   does not get evaluated more than once */
                uint32_t right = PTTransGetIdx ( self, tt, ++ k );
                if ( right >= zidx )
                {
                    /* found the range - calculate 1-based index */
                    return zidx - left + j;
                }

                /* advance "j" by range width - 1 */
                j += right - left;
            }
        }
    }

    return 0;
}

/* 8 bit index */
static
uint32_t PTTransGetIdx8 ( const PTTrans *self, uint32_t zidx )
{
    return self -> idx . v8 [ zidx ];
}

static
const void * PTTransGetIdxAddr8 ( const PTTrans *self, uint32_t zidx )
{
    return & self -> idx . v8 [ zidx ];
}

/* 16 bit index */
static
uint32_t PTTransGetIdx16 ( const PTTrans *self, uint32_t zidx )
{ 
    return self -> idx . v16 [ zidx ];
}

static
uint32_t PTTransSwapIdx16 ( const PTTrans *self, uint32_t zidx )
{
    return bswap_16 ( self -> idx . v16 [ zidx ] );
}

static
const void * PTTransGetIdxAddr16 ( const PTTrans *self, uint32_t zidx )
{
    return & self -> idx . v16 [ zidx ];
}

/* NULL dad access */
static
uint32_t PTTransGetDadNULL ( const PTTrans *self )
{
    return 0; 
}

/* 8 bit trans id */
static
uint32_t PTTransGetDad8 ( const PTTrans *self )
{
    return self -> dad . v8 [ 0 ];
}

static
const void * PTTransAlignDadAddr8 ( PTTrans *self, bool backtrace )
{
    return & self -> dad . v8 [ backtrace ];
}

static
uint32_t PTTransGetChild8 ( const PTTrans *self, uint32_t zidx )
{
    return self -> child . v8 [ zidx ];
}

static
const void * PTTransGetChildAddr8 ( const PTTrans *self, uint32_t zidx )
{
    return & self -> child . v8 [ zidx ];
}

/* 16 bit trans id */
static
uint32_t PTTransGetDad16 ( const PTTrans *self )
{
    return self -> dad . v16 [ 0 ];
}

static
uint32_t PTTransSwapDad16 ( const PTTrans *self )
{
    return bswap_16 ( self -> dad . v16 [ 0 ] );
}

static
const void * PTTransAlignDadAddr16 ( PTTrans *self, bool backtrace )
{
    /* align dad first */
    size_t i = ( size_t ) self -> dad . v16;
    i = ( i + 1 ) & ~ ( size_t ) 1;
    self -> dad . v16 = ( const uint16_t* ) i;
    if ( backtrace )
        return ( const void* ) ( i + 2 );
    return ( const void* ) i;
}

static
uint32_t PTTransGetChild16 ( const PTTrans *self, uint32_t zidx )
{
    return self -> child . v16 [ zidx ];
}

static
uint32_t PTTransSwapChild16 ( const PTTrans *self, uint32_t zidx )
{
    return bswap_16 ( self -> child . v16 [ zidx ] );
}

static
const void * PTTransGetChildAddr16 ( const PTTrans *self, uint32_t zidx )
{
    return & self -> child . v16 [ zidx ];
}

/* 32 bit trans id */
static
uint32_t PTTransGetDad32 ( const PTTrans *self )
{
    return self -> dad . v32 [ 0 ];
}

static
uint32_t PTTransSwapDad32 ( const PTTrans *self )
{
    return bswap_32 ( self -> dad . v32 [ 0 ] );
}

static
const void * PTTransAlignDadAddr32 ( PTTrans *self, bool backtrace )
{
    /* align dad first */
    size_t i = ( size_t ) self -> dad . v32;
    i = ( i + 3 ) & ~ ( size_t ) 3;
    self -> dad . v32 = ( const uint32_t* ) i;
    if ( backtrace )
        return ( const void* ) ( i + 4 );
    return ( const void* ) i;
}

static
uint32_t PTTransGetChild32 ( const PTTrans *self, uint32_t zidx )
{
    return self -> child . v32 [ zidx ];
}

static
uint32_t PTTransSwapChild32 ( const PTTrans *self, uint32_t zidx )
{
    return bswap_32 ( self -> child . v32 [ zidx ] );
}

static
const void * PTTransGetChildAddr32 ( const PTTrans *self, uint32_t zidx )
{
    return & self -> child . v32 [ zidx ];
}

/* ForEach
 */
void PTTransForEach ( const PTTrans *self, const PTrie *tt,
    void ( * f ) ( const PTTrans *trans, const PTrie *tt, void *data ), void * data )
{
    const uint8_t *child_seq_type;

    assert ( f != NULL );
    ( * f ) ( self, tt, data );

    child_seq_type = self -> child_seq_type;
    if ( self -> child_seq_type != NULL )
    {
        PTTrans *trans = malloc ( sizeof * trans );
        if ( trans != NULL )
        {
            uint32_t i, j, k;

            /* see explanations in PTTransGetChildChildIdx */
            for ( i = j = 0, k = pttFirstIdx; i < self -> slen; ++ k, ++ i )
            {
                uint32_t left, right;

                /* get the single char code or
                   the left edge of char code range */
                left = right = PTTransGetIdx ( self, tt, k );

                /* check entry type: if 0, then the index was a single
                   value. if 1, then it's a range */
                if ( ( child_seq_type [ i >> 3 ] & ( 1U << ( i & 7 ) ) ) != 0 )
                    right = PTTransGetIdx ( self, tt, ++ k );

                /* get child node id */
                for ( ; left <= right; ++ j, ++ left )
                {
                    uint32_t tid = PTTransGetChild ( self, tt, j ) + 1;
                    int status = PTrieInitNode ( tt, trans, tid );
                    if ( status == 0 )
                    {
                        trans -> back = self;
#if ! RECORD_HDR_IDX
                        trans -> tidx = left;
#endif
#if ! RECORD_HDR_DEPTH
                        trans -> depth = self -> depth + 1;
#endif
                        PTTransForEach ( trans, tt, f, data );
                    }
                }
            }

            free ( trans );
        }
    }
}

/* DoUntil
 */
bool PTTransDoUntil ( const PTTrans *self, const PTrie *tt,
    bool ( * f ) ( const PTTrans *trans, const PTrie *tt, void *data ), void * data )
{
    const uint8_t *child_seq_type;

    assert ( f != NULL );
    if ( ( * f ) ( self, tt, data ) )
        return true;

    child_seq_type = self -> child_seq_type;
    if ( self -> child_seq_type != NULL )
    {
        PTTrans *trans = malloc ( sizeof * trans );
        if ( trans != NULL )
        {
            uint32_t i, j, k;

            /* see explanations in PTTransGetChildChildIdx */
            for ( i = j = 0, k = pttFirstIdx; i < self -> slen; ++ k, ++ i )
            {
                uint32_t left, right;

                /* get the single char code or
                   the left edge of char code range */
                left = right = PTTransGetIdx ( self, tt, k );

                /* check entry type: if 0, then the index was a single
                   value. if 1, then it's a range */
                if ( ( child_seq_type [ i >> 3 ] & ( 1U << ( i & 7 ) ) ) != 0 )
                    right = PTTransGetIdx ( self, tt, ++ k );

                /* get child node id */
                for ( ; left <= right; ++ j, ++ left )
                {
                    uint32_t tid = PTTransGetChild ( self, tt, j ) + 1;
                    rc_t rc = PTrieInitNode ( tt, trans, tid );
                    if ( rc == 0 )
                    {
                        trans -> back = self;
#if ! RECORD_HDR_IDX
                        trans -> tidx = left;
#endif
#if ! RECORD_HDR_DEPTH
                        trans -> depth = self -> depth + 1;
#endif
                        if ( PTTransDoUntil ( trans, tt, f, data ) )
                        {
                            free ( trans );
                            return true;
                        }
                    }
                }
            }

            free ( trans );
        }
    }
    return false;
}


/*--------------------------------------------------------------------------
 * PTrie
 *  a persisted tree of text nodes
 *
 *  this is a collection of { key, value } pairs, where a many-value
 *  to one key paradigm is naturally supported, as in the b-tree.
 *
 *  the desired retrieval operations are:
 *   a) key -> id : value
 *   b) key -> { id : value, ... }
 *   c) RE -> id : value
 *   d) RE -> { id : value, ... }
 *   e) iteration across { id : value, ... }
 *   f) id : value -> key
 *
 *  the reverse retrieval operation is:
 *   a) id -> value
 *
 *  indexing of the id is performed externally, and should not be interpreted
 *  as a serial, integer value. it may be a byte offset or a combination of
 *  two integer values, as well as anything else.
 */

/* GetNodeOff
 *  takes a 1-based index
 *  returns an offset into data array
 */
static
size_t PTrieGetNodeOff8 ( const PTrie *self, uint32_t zidx )
{
    return self -> trans_off . v8 [ zidx ];
}

static
size_t PTrieGetNodeOff16 ( const PTrie *self, uint32_t zidx )
{
    return self -> trans_off . v16 [ zidx ];
}

static
size_t PTrieSwapNodeOff16 ( const PTrie *self, uint32_t zidx )
{
    return bswap_16 ( self -> trans_off . v16 [ zidx ] );
}

static
size_t PTrieGetNodeOff32 ( const PTrie *self, uint32_t zidx )
{
    return self -> trans_off . v32 [ zidx ];
}

static
size_t PTrieSwapNodeOff32 ( const PTrie *self, uint32_t zidx )
{
    return bswap_32 ( self -> trans_off . v32 [ zidx ] );
}

#if 0
static
size_t PTrieGetNodeOff64 ( const PTrie *self, uint32_t zidx )
{
    return self -> trans_off . v64 [ zidx ];
}

static
size_t PTrieSwapNodeOff64 ( const PTrie *self, uint32_t zidx )
{
    return bswap_64 ( self -> trans_off . v64 [ zidx ] );
}
#endif


/* EncodeNodeId
 * PTrieDecodeNodeId
 */
static
uint32_t PTrieOrigEncodeNodeId0 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 8 ) | btid; 
}

static
uint32_t PTrieEncodeNodeId0 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 8 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeTestTrieNodeId ( const PTrie *self, uint32_t *tid, uint32_t *btid, uint32_t count )
{
    uint32_t which = 0;

    PTTrans trans;
    rc_t rc = PTrieInitNode ( self, & trans, * tid & ~ 1U );
    if ( rc == 0 )
    {
        assert ( trans . vals != NULL );
        if ( PBSTreeCount ( trans . vals ) == count )
            which |= 1;
    }
    rc = PTrieInitNode ( self, & trans, * tid | 1U );
    if ( rc == 0 )
    {
        assert ( trans . vals != NULL );
        if ( PBSTreeCount ( trans . vals ) == count )
            which |= 2;
    }

    switch ( which )
    {
    case 0:
        return RC ( rcCont, rcTrie, rcAccessing, rcId, rcCorrupt );
    case 1:
        * tid &= ~ 1U;
        * btid = count;
        break;
    case 2:
        * tid |= 1U;
        * btid = count;
        break;
    case 3:
        * btid = count;
        return RC ( rcCont, rcTrie, rcAccessing, rcId, rcAmbiguous );
    }

    return 0;
}

static
rc_t PTrieOrigDecodeNodeId0 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    /* unpack 1-based ids */
    * tid = id >> 8;
    * btid = id & 0xFF;

    /* detect no-overflow cases */
    if ( * btid != 0 )
        return 0;

    /* handle overflow */
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x100 );
}

static
rc_t PTrieDecodeNodeId0 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 8 ) + 1;
    * btid = ( id & 0xFF ) + 1;
    return 0;
}

static
uint32_t PTrieOrigEncodeNodeId1 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 10 ) | btid;
}

static
uint32_t PTrieEncodeNodeId1 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 10 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeNodeId1 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    * tid = id >> 10;
    * btid = id & 0x3FF;
    if ( * btid != 0 )
        return 0;
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x400 );
}

static
rc_t PTrieDecodeNodeId1 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 10 ) + 1;
    * btid = ( id & 0x3FF ) + 1;
    return 0;
}

static
uint32_t PTrieOrigEncodeNodeId2 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 12 ) | btid;
}

static
uint32_t PTrieEncodeNodeId2 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 12 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeNodeId2 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    * tid = id >> 12;
    * btid = id & 0xFFF;
    if ( * btid != 0 )
        return 0;
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x1000 );
}

static
rc_t PTrieDecodeNodeId2 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 12 ) + 1;
    * btid = ( id & 0xFFF ) + 1;
    return 0;
}

static
uint32_t PTrieOrigEncodeNodeId3 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 14 ) | btid;
}

static
uint32_t PTrieEncodeNodeId3 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 14 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeNodeId3 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    * tid = id >> 14;
    * btid = id & 0x3FFF;
    if ( * btid != 0 )
        return 0;
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x4000 );
}

static
rc_t PTrieDecodeNodeId3 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 14 ) + 1;
    * btid = ( id & 0x3FFF ) + 1;
    return 0;
}

static
uint32_t PTrieOrigEncodeNodeId4 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 16 ) | btid;
}

static
uint32_t PTrieEncodeNodeId4 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 16 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeNodeId4 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    * tid = id >> 16;
    * btid = id & 0xFFFF;
    if ( * btid != 0 )
        return 0;
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x10000 );
}

static
rc_t PTrieDecodeNodeId4 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 16 ) + 1;
    * btid = ( id & 0xFFFF ) + 1;
    return 0;
}

static
uint32_t PTrieOrigEncodeNodeId5 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 18 ) | btid;
}

static
uint32_t PTrieEncodeNodeId5 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 18 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeNodeId5 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    * tid = id >> 18;
    * btid = id & 0x3FFFF;
    if ( * btid != 0 )
        return 0;
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x40000 );
}

static
rc_t PTrieDecodeNodeId5 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 18 ) + 1;
    * btid = ( id & 0x3FFFF ) + 1;
    return 0;
}

static
uint32_t PTrieOrigEncodeNodeId6 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( tid << 20 ) | btid;
}

static
uint32_t PTrieEncodeNodeId6 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    return ( ( ( tid - 1 ) << 20 ) + ( btid - 1 ) ) + 1;
}

static
rc_t PTrieOrigDecodeNodeId6 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    * tid = id >> 20;
    * btid = id & 0xFFFFF;
    if ( * btid != 0 )
        return 0;
    return PTrieOrigDecodeTestTrieNodeId ( self, tid, btid, 0x100000 );
}

static
rc_t PTrieDecodeNodeId6 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    -- id;
    * tid = ( id >> 20 ) + 1;
    * btid = ( id & 0xFFFFF ) + 1;
    return 0;
}

static
uint32_t PTrieEncodeNodeId7 ( const PTrie *self,
    uint32_t tid, uint32_t btid )
{
    /* first get the byte offset to trans */
    uint32_t id = PTrieGetNodeOff ( self, tid - 1 );

    /* the trans node is supposed to have at least btid
       nodes underneath, taking > 1 byte each, so by adding
       the count to a trans offset, we get something that
       can be recovered later, and we insist that this
       offset will be somewhere within the trans */

    /* NB - conversion from tid to offset gives a
       zero-based index into an array of uint32_t,
       which is converted to a zero-based byte offset
       and then added to a one-based node id, resulting
       in a one-based composite node id */
    return ( id << 2 ) + btid;
}

static
rc_t PTrieDecodeNodeId7 ( const PTrie *self, uint32_t id,
    uint32_t *tid, uint32_t *btid )
{
    uint32_t left = 0;
    uint32_t right = self -> num_nodes;
    while ( left < right )
    {
        uint32_t i = ( left + right ) >> 1;
        uint32_t off = PTrieGetNodeOff ( self, i ) << 2;
        if ( off > id )
            right = i;
        else
        {
            size_t end;

            /* should never be equal, because ids are 1-based */
            assert ( off != id );

            /* get offset of next or end of data */
            if ( i + 1 == self -> num_nodes )
                end = self -> data_size;
            else
                end = PTrieGetNodeOff ( self, i + 1 ) << 2;

            if ( end > id )
            {
                /* found it */
                * tid = i + 1;
                * btid = id - off;
                return 0;
            }

            left = i;
        }
    }

    return RC ( rcCont, rcTrie, rcSelecting, rcId, rcNotFound );
}


/* DecodeCharId
 */
static
uint32_t PTrieGetRMap ( const PTrie *self, uint32_t idx )
{ 
    return self -> _rmap [ idx ];
}

static
uint32_t PTrieSwapRMap ( const PTrie *self, uint32_t idx )
{
    return bswap_32 ( self -> _rmap [ idx ] );
}


/* MakeInt
 *  make a persisted tree structure
 *  includes code to handle original composite node encoding
 *
 *  "addr" [ IN ] and "size" [ IN ] - constant memory image of
 *  persisted text tree with a lifetime exceeding that of the
 *  PTrie itself
 *
 *  "byteswap" [ IN ] - true if persisted image must be byteswapped
 *
 *  "orig" [ IN ] - true if persisted node ids must be reprocessed
 *   for overflow. In the original implementation, the composite
 *   node ids were formed from one-based trie and bstree node ids,
 *   but the bits allotted for them were calculated from zero-based
 *   ids, causing an overflow when either node reached its maximum.
 *
 *   e.g.: tid_max <= 0x1000000, btid_max <= 0x100: 24:8 packing
 *   when ( one-based ) btid == 256, it requires 9 bits, whereas
 *   zero-based btid == 255, requiring the expected 8 bits.
 */
static int16_t _get_16 ( uint16_t i ) { return ( int16_t ) i; }
static int16_t _swap_16 ( uint16_t i ) { return ( int16_t ) bswap_16 ( i ); }
static int32_t _get_32 ( uint32_t i ) { return ( int32_t ) i; }
static int32_t _swap_32 ( uint32_t i ) { return ( int32_t ) bswap_32 ( i ); }

static uint16_t _get_U16 ( uint16_t i ) { return i; }
static uint16_t _swap_U16 ( uint16_t i ) { return ( uint16_t ) bswap_16 ( i ); }
static uint32_t _get_U32 ( uint32_t i ) { return i; }
static uint32_t _swap_U32 ( uint32_t i ) { return ( uint32_t ) bswap_32 ( i ); }

#define PTOFFSETOF( mbr ) \
    ( ( uint32_t ) ( size_t ) & ( ( ( P_Trie* ) 0 ) -> mbr ) )

static
rc_t PTrieMakeInt ( PTrie **ttp, const void *addr, size_t size, bool byteswap, bool orig )
{
    rc_t rc;

    /* guard against unknown compiler struct packing */
    assert ( PTOFFSETOF ( num_trans )     == 0x00 );
    assert ( PTOFFSETOF ( num_nodes )     == 0x04 );
    assert ( PTOFFSETOF ( data_size )     == 0x08 );
    assert ( PTOFFSETOF ( keys )          == 0x0C );
#if EXTENDED_PTRIE
    assert ( PTOFFSETOF ( ext_data_size ) == 0x0D );
#endif
    assert ( PTOFFSETOF ( width )         == 0x0E );
    assert ( PTOFFSETOF ( rmap )          == 0x10 );

    if ( ttp == NULL )
        rc = RC ( rcCont, rcTrie, rcAllocating, rcParam, rcNull );
    else
    {
        if ( addr == NULL )
            rc = RC ( rcCont, rcTrie, rcAllocating, rcParam, rcNull );
        else if ( size == 0 )
            rc = RC ( rcCont, rcTrie, rcAllocating, rcData, rcInvalid );
        else
        {
            const P_Trie *ptt;

            if ( size < ( sizeof * ptt - sizeof ptt -> rmap ) )
                rc = RC ( rcCont, rcTrie, rcAllocating, rcData, rcCorrupt );
            else
            {
                uint64_t min_size;
                uint64_t data_size;

                /* accessor functions */
                int16_t ( * get_16 ) ( uint16_t ) = byteswap ? _swap_16 : _get_16;
                int32_t ( * get_32 ) ( uint32_t ) = byteswap ? _swap_32 : _get_32;

                uint16_t ( * get_U16 ) ( uint16_t ) = byteswap ? _swap_U16 : _get_U16;
                uint32_t ( * get_U32 ) ( uint32_t ) = byteswap ? _swap_U32 : _get_U32;

                /* everything looks good */
                rc = 0;

                /* structured pointer into const memory image */
                ptt = addr;

                /* extract data_size */
                data_size = get_U32 ( ptt -> data_size )
#if EXTENDED_PTRIE
                    + ( ( uint64_t ) ptt -> ext_data_size << 32 )
#endif
                    ;

                /* minimum known size of the memory image
                   will be adjusted as image is scanned */
                min_size = sizeof * ptt - sizeof ptt -> rmap +
                    get_U16 ( ptt -> width ) * sizeof ptt -> rmap [ 0 ] +
                    data_size;
                if ( ( uint64_t ) size >= min_size )
                {
                    PTrie *tt;
                    uint16_t i, unmapped;
                    uint32_t ch, first, last;

                    /* the accept character set is recovered from the
                       reverse character map where the special character
                       value NUL represents a wildcard.

                       the forward map will be built using one-based
                       character codes, where code 0 means unmapped.

                       first determine the range of accept characters
                       while at the same time capturing the one-based
                       character code of wildcards */
                    for ( last = 0, first = ~ 0, unmapped = i = 0;
                          i < get_U16 ( ptt -> width ); ++ i )
                    {
                        ch = get_32 ( ptt -> rmap [ i ] );
                        if ( ch == 0 )
                            unmapped = i + 1;
                        else
                        {
                            if ( ch < first )
                                first = ch;
                            if ( ch > last )
                                last = ch;
                        }
                    }

                    /* this case occurs when all values are on the root node */
                    if ( last < first )
                        first = 1;
                    
                    /* allocated block with mapped sections */
                    tt = malloc ( sizeof * tt - sizeof tt -> map +
                        ( last - first + 1 ) * sizeof tt -> map [ 0 ] );
                    if ( tt == NULL )
                        rc = RC ( rcCont, rcTrie, rcAllocating, rcMemory, rcExhausted );
                    else
                    {
                        uint16_t ptt_width = get_16 ( ptt -> width );

                        /* fill character to index map with wildcard index
                           usually NULL or unmapped */
                        for ( ch = first; ch <= last; ++ ch )
                            tt -> map [ ch - first ] = unmapped;

                        /* now overwrite mapped characters with one-based char codes */
                        for ( i = 0; i < ptt_width; ++ i )
                        {
                            ch = get_32 ( ptt -> rmap [ i ] );
                            if ( ch != 0 )
                                tt -> map [ ch - first ] = i + 1;
                        }

                        /* fill out the virtual functions */
                        do
                        {
                            uint32_t trans_off_len;
                            uint16_t ptt_keys = get_16 ( ptt -> keys );

                            /* reverse character lookup */
                            tt -> decode_char_id = byteswap ?
                                PTrieSwapRMap : PTrieGetRMap;

                            /* index encoding scheme */
                            if ( orig ) switch ( P_TrieIdCoding ( ptt_keys ) )
                            {
                            case 0:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId0;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId0;
                                break;
                            case 1:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId1;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId1;
                                break;
                            case 2:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId2;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId2;
                                break;
                            case 3:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId3;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId3;
                                break;
                            case 4:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId4;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId4;
                                break;
                            case 5:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId5;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId5;
                                break;
                            case 6:
                                tt -> encode_node_id = PTrieOrigEncodeNodeId6;
                                tt -> decode_node_id = PTrieOrigDecodeNodeId6;
                                break;
                            default:
                                tt -> encode_node_id = PTrieEncodeNodeId7;
                                tt -> decode_node_id = PTrieDecodeNodeId7;
                            }
                            else switch ( P_TrieIdCoding ( ptt_keys ) )
                            {
                            case 0:
                                tt -> encode_node_id = PTrieEncodeNodeId0;
                                tt -> decode_node_id = PTrieDecodeNodeId0;
                                break;
                            case 1:
                                tt -> encode_node_id = PTrieEncodeNodeId1;
                                tt -> decode_node_id = PTrieDecodeNodeId1;
                                break;
                            case 2:
                                tt -> encode_node_id = PTrieEncodeNodeId2;
                                tt -> decode_node_id = PTrieDecodeNodeId2;
                                break;
                            case 3:
                                tt -> encode_node_id = PTrieEncodeNodeId3;
                                tt -> decode_node_id = PTrieDecodeNodeId3;
                                break;
                            case 4:
                                tt -> encode_node_id = PTrieEncodeNodeId4;
                                tt -> decode_node_id = PTrieDecodeNodeId4;
                                break;
                            case 5:
                                tt -> encode_node_id = PTrieEncodeNodeId5;
                                tt -> decode_node_id = PTrieDecodeNodeId5;
                                break;
                            case 6:
                                tt -> encode_node_id = PTrieEncodeNodeId6;
                                tt -> decode_node_id = PTrieDecodeNodeId6;
                                break;
                            default:
                                tt -> encode_node_id = PTrieEncodeNodeId7;
                                tt -> decode_node_id = PTrieDecodeNodeId7;
                            }

                            /* PTTrans index access based on the
                               minimum allowed char code width
                               where the 0 <= code < width */
                            if ( ptt_width <= 256 )
                            {
                                tt -> get_idx = PTTransGetIdx8;
                                tt -> get_idx_addr = PTTransGetIdxAddr8;
                            }
                            else
                            {
                                tt -> get_idx = byteswap ? PTTransSwapIdx16 : PTTransGetIdx16;
                                tt -> get_idx_addr = PTTransGetIdxAddr16;
                            }

                            /* PTTrans child and parent access based
                               ont the minimum allowed trans id width */
                            if ( get_32 ( ptt -> num_trans ) <= 256 )
                            {
                                tt -> get_dad = PTTransGetDad8;
                                tt -> align_dad_addr = PTTransAlignDadAddr8;
                                tt -> get_child = PTTransGetChild8;
                                tt -> get_child_addr = PTTransGetChildAddr8;
                            }
                            else if ( get_32 ( ptt -> num_trans ) <= 65536 )
                            {
                                if ( byteswap )
                                {
                                    tt -> get_dad = PTTransSwapDad16;
                                    tt -> get_child = PTTransSwapChild16;
                                }
                                else
                                {
                                    tt -> get_dad = PTTransGetDad16;
                                    tt -> get_child = PTTransGetChild16;
                                }
                                tt -> align_dad_addr = PTTransAlignDadAddr16;
                                tt -> get_child_addr = PTTransGetChildAddr16;
                            }
                            else
                            {
                                if ( byteswap )
                                {
                                    tt -> get_dad = PTTransSwapDad32;
                                    tt -> get_child = PTTransSwapChild32;
                                }
                                else
                                {
                                    tt -> get_dad = PTTransGetDad32;
                                    tt -> get_child = PTTransGetChild32;
                                }
                                tt -> align_dad_addr = PTTransAlignDadAddr32;
                                tt -> get_child_addr = PTTransGetChildAddr32;
                            }

                            /* wipe out the backtrace function */
                            if ( ! P_TrieBacktrace ( ptt_keys ) )
                                tt -> get_dad = PTTransGetDadNULL;

                            /* the trans offset access is based upon the
                               total data size to be represented in nodes.
                               all entries are 4-byte aligned, making it
                               possible to represent their offsets with
                               the byte value / 4 */
                            if ( data_size <= 256 * 4 )
                            {
                                tt -> get_node_off = PTrieGetNodeOff8;
                                trans_off_len = ( get_32 ( ptt -> num_trans ) + 3 ) >> 2;
                            }
                            else if ( data_size <= 65536 * 4 )
                            {
                                tt -> get_node_off = byteswap ? PTrieSwapNodeOff16 : PTrieGetNodeOff16;
                                trans_off_len = ( get_32 ( ptt -> num_trans ) + 1 ) >> 1;
                            }
                            else
                            {
                                tt -> get_node_off = byteswap ? PTrieSwapNodeOff32 : PTrieGetNodeOff32;
                                trans_off_len = get_32 ( ptt -> num_trans );
                            }

                            /* the minimum size of this image */
                            min_size += trans_off_len << 2;

                            /* initialize a few members -
                               could be here or inside following if
                               placed here only for pipelining */
                            tt -> _rmap = ptt -> rmap;
                            tt -> trans_off . v32 = & ptt -> rmap [ ptt_width ];

                            /* require image size to pass test */
                            if ( min_size <= size )
                            {
                                tt -> data . v32 = & tt -> trans_off . v32 [ trans_off_len ];
                                tt -> num_trans = get_U32 ( ptt -> num_trans );
                                tt -> num_nodes = get_U32 ( ptt -> num_nodes );
                                tt -> data_size = ( size_t ) data_size;
                                tt -> ext_keys = P_TrieExtKeys ( ptt_keys ) != 0;
                                tt -> backtrace = P_TrieBacktrace ( ptt_keys ) != 0;
                                tt -> id_coding = P_TrieIdCoding ( ptt_keys );
                                tt -> byteswap = byteswap;
                                tt -> width = ptt_width;
                                tt -> first_char = first;
                                tt -> last_char = last;
                                tt -> unmapped_idx = unmapped;

                                * ttp = tt;
                                return 0;
                            }
                        }
                        while ( 0 );

                        free ( tt );
                    }
                }
                else {
                    rc = RC ( rcCont, rcTrie, rcAllocating, rcData, rcInsufficient );
                }
            }
        }

        if ( rc == 0 ) {
            rc = RC ( rcCont, rcTrie, rcAllocating, rcData, rcInvalid );
        }

        * ttp = NULL;
    }
    return rc;
}

/* Make
 *  make a persisted tree structure
 *
 *  "addr" [ IN ] and "size" [ IN ] - constant memory image of
 *  persisted text tree with a lifetime exceeding that of the
 *  PTrie itself
 *
 *  "byteswap" [ IN ] - true if persisted image must be byteswapped
 */
LIB_EXPORT rc_t CC PTrieMake ( PTrie **tt, const void *addr, size_t size, bool byteswap )
{
    return PTrieMakeInt ( tt, addr, size, byteswap, false );
}

/* MakeOrig - DEPRECATED
 *  make a persisted tree structure
 *  includes code to handle original composite node encoding
 *
 *  "addr" [ IN ] and "size" [ IN ] - constant memory image of
 *  persisted text tree with a lifetime exceeding that of the
 *  PTrie itself
 *
 *  "byteswap" [ IN ] - true if persisted image must be byteswapped
 */
LIB_EXPORT rc_t CC PTrieMakeOrig ( PTrie **tt, const void *addr, size_t size, bool byteswap )
{
    return PTrieMakeInt ( tt, addr, size, byteswap, true );
}

/* Count
 *  returns number of { id : value } pairs in text tree
 *  not included within the TTree interface itself, but
 *  was included here due to the fact that it is constant
 *
 *  return value:
 *    integer value >= 0
 */
LIB_EXPORT uint32_t CC PTrieCount ( const PTrie *self )
{
    if ( self != NULL )
      return self -> num_nodes;
    return 0;
}

/* Size
 *  returns the size in bytes
 *  of the PTrie image
 */
LIB_EXPORT size_t CC PTrieSize ( const PTrie *self )
{
    const P_Trie *ptt;

    if ( self == NULL )
        return 0;

    /* the original image has some standard set of headers
       plus a bunch of table data from "rmap" to "data", followed
       by "data_size" bytes of data. */
    return ( const char* ) self -> data . v32 - ( const char* ) self -> _rmap +
      self -> data_size + sizeof * ptt - sizeof ptt -> rmap;
}

/* GetNode
 *  gets a PTNode from an id
 *
 *  "node" [ OUT ] - return parameter for node
 *
 *  "id" [ IN ] - a 1-based integer node id
 *
 *  return values:
 *    EINVAL => an invalid parameter was passed
 *    ENOENT => id out of range
 */
LIB_EXPORT rc_t CC PTrieGetNode ( const PTrie *self, PTNode *node, uint32_t id )
{
    rc_t rc;
    if ( node == NULL )
        rc = RC ( rcCont, rcTrie, rcAccessing, rcNode, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcCont, rcTrie, rcAccessing, rcSelf, rcNull );
        else if ( id == 0 )
            rc = RC ( rcCont, rcTrie, rcAccessing, rcId, rcNull );
        else
        {
            uint32_t tid, btid;
            rc = PTrieDecodeNodeId ( self, id, & tid, & btid );
            if ( rc == 0 )
            {
                PTTrans trans;
                rc = PTrieInitNode ( self, & trans, tid );
                if ( rc == 0 )
                {
                    const PBSTree *pbt = trans . vals;
                    if ( pbt == NULL )
                        rc = RC ( rcCont, rcTrie, rcAccessing, rcId, rcNotFound );
                    else
                    {
                        rc = PBSTreeGetNodeData ( pbt,
                            & node -> data . addr, & node -> data . size, btid );
                        if ( rc == 0 )
                        {
                            node -> internal = self;
                            node -> id = id;

                            if ( ! self -> ext_keys )
                            {
                                size_t sz = strlen ( node -> data . addr ) + 1;
                                assert ( sz <= node -> data . size );
                                node -> data . addr =
                                    ( const char* ) node -> data . addr + sz;
                                node -> data . size -= sz;
                            }

                            return 0;
                        }
                    }
                }
            }
        }

        node -> data . addr = node -> internal = NULL;
        node -> data . size = 0;
        node -> id = 0;
    }
    return rc;
}

/* InitNode
 */
rc_t PTrieInitNode ( const PTrie *self, PTTrans *trans, uint32_t idx )
{
    rc_t rc;
    uint32_t tcnt;
    const uint8_t *align2;

    assert ( self != NULL );
    assert ( trans != NULL );

    if ( idx == 0 )
        return RC ( rcCont, rcNode, rcConstructing, rcId, rcNull );
    if ( idx > self -> num_trans )
        return RC ( rcCont, rcNode, rcConstructing, rcId, rcNotFound );

    /* record node id */
    trans -> tid = idx;

    /* no backtrace link yet assigned */
    trans -> back = NULL;

    /* nobody is linked to us yet */
    trans -> refcount = 0;

    /* initialize the trans' index member */
    trans -> idx . v32 =
        & self -> data . v32 [ PTrieGetNodeOff ( self, idx - 1 ) ];
    assert ( ( ( size_t ) trans -> idx . v8 & 3 ) == 0 );

    /* can now access header values within index */
    trans -> tcnt = PTTransGetIdx ( trans, self, pttHdrTransCnt );
    trans -> icnt = PTTransGetIdx ( trans, self, pttHdrIdxCnt );

    /* if the icnt is zero, the original child array was NULL */
    if ( trans -> icnt == 0 )
    {
        trans -> slen = tcnt = 0;
        trans -> child_seq_type = NULL;

        /* get unaligned dad pointer */
        trans -> dad . v8 =
            PTTransGetIdxAddr ( trans, self, pttHdrNullEnd );
   }
    else
    {
        tcnt = PTTransGetIdx ( trans, self, pttHdrChildCnt );
        trans -> slen = PTTransGetIdx ( trans, self, pttHdrSeqLen );

        /* sequence type */
        trans -> child_seq_type =
            PTTransGetIdxAddr ( trans, self, pttFirstIdx + trans -> icnt );

        /* unaligned dad pointer */
        trans -> dad . v8 = & trans -> child_seq_type [ ( trans -> slen + 7 ) >> 3 ];
    }

    /* align dad and get child pointer */
    trans -> child . v8 = PTTransAlignDadAddr ( trans, self );

    /* location of align2 */
    align2 = PTTransGetChildAddr ( trans, self, tcnt );

    /* NULL out dad pointer if no backtrace */
    if ( ! self -> backtrace )
        trans -> dad . v8 = NULL;

    /* NULL out child pointer if no transitions */
    if ( trans -> icnt == 0 )
        trans -> child . v8 = NULL;

    /* finally, look at align for boolean */
    if ( ( ( size_t ) align2 & 3 ) != 0 )
    {
        if ( align2 [ 0 ] == 0 )
        {
            trans -> vals = NULL;
            return 0;
        }

        * ( size_t* ) & align2 |= 3;
        ++ align2;
    }

    /* to avoid an allocation, mimic the older behavior
       that used persisted data directly as a PBSTree
       and had no allocation. we introduced a wrapper
       object in order to have a vtable for byte-swapping,
       and this disturbs the older code. */
    trans -> vals = & trans -> _val;

    rc = self -> byteswap ?
        PBSTreeMakeSwapped ( & trans -> _val, align2, 0 ):
        PBSTreeMakeNative ( & trans -> _val, align2, 0 );
    if ( rc == 0 )
    {
        if ( PBSTreeCount ( trans -> vals ) == 0 )
        {
            trans -> vals = NULL;
        }
    }
    return rc;
}

/* MakeNode
 */
rc_t PTrieMakeNode ( const PTrie *self, PTTrans **transp, uint32_t idx )
{
    rc_t rc;

    PTTrans *trans = malloc ( sizeof * trans );
    if ( trans == NULL )
        return RC ( rcCont, rcNode, rcAllocating, rcMemory, rcExhausted );

    assert ( transp != NULL );
    rc = PTrieInitNode ( self, * transp = trans, idx );
    if ( rc != 0 )
        free ( trans );

    return rc;
}

/* MapChar
 *  maps a character to a one-based character code
 */
static
uint32_t PTrieMapChar ( const PTrie *tt, uint32_t ch )
{
    if ( ch >= tt -> first_char && ch <= tt -> last_char )
        return tt -> map [ ch - tt -> first_char ];
    return tt -> unmapped_idx;
}

/* NextIdx
 *  pops a character from key string
 *  maps it to an index
 */
static
rc_t PTrieNextIdx ( const PTrie *tt, String *key, uint32_t *idx )
{
    uint32_t ch;
    rc_t rc = StringPopHead ( key, & ch );
    if ( rc == 0 )
        * idx = PTrieMapChar ( tt, ch );
    return rc;
}

/* FindTrans
 */
static
rc_t PTrieFindTrans ( const PTrie *self, String *key, PTTrans **transp )
{
    rc_t rc;

    PTTrans *dad;
    uint32_t tid;
    uint32_t key_idx;

    for ( dad = NULL, key_idx = 0, rc = PTrieMakeNode ( self, transp, 1 );
          rc == 0; rc = PTrieMakeNode ( self, transp, tid ) )
    {
        PTTrans *trans = * transp;

        /* link in dad */
        trans -> back = dad;
#if ! RECORD_HDR_IDX
        trans -> tidx = key_idx;
#endif
        if ( dad != NULL )
        {
#if ! RECORD_HDR_DEPTH
            trans -> depth = dad -> depth + 1;
#endif
            ++ dad -> refcount;
        }
#if ! RECORD_HDR_DEPTH
        else
        {
            trans -> depth = 0;
        }
#endif

        /* if there are no child transitions,
           this must be the node */
        if ( trans -> icnt == 0 )
            return 0;

        /* capture dad for next loop
           or for failure cleanup */
        dad = trans;

        /* get transition index for character */
        rc = PTrieNextIdx ( self, key, & key_idx );
        if ( rc != 0 )
        {
            /* end of string */
            if ( GetRCState ( rc ) == rcEmpty )
                return 0;

            /* invalid input */
            break;
        }

        /* either an unrecognized character or
           a NULL transition mean NOT FOUND */
        rc = RC ( rcCont, rcTrie, rcSelecting, rcString, rcNotFound );

        /* check for inability to take transition */
        if ( key_idx == 0 )
            break;

        /* try to take transition */
        tid = PTTransGetChildChildIdx ( trans, self, -- key_idx );
        if ( tid == 0 )
            break;

        /* convert the child transition index into a trans id */
        tid = PTTransGetChild ( trans, self, tid - 1 ) + 1;
    }

    PTTransWhack ( dad );
    return rc;
}

/* Find
 * PTrieFindRE
 *  find a single { id, value } pair  within tree
 *
 *  "item" [ IN ] - an exact match text string
 *
 *  "re" [ IN ] - a regular expression string
 *
 *  "rtn" [ OUT ] - return parameter for node if found.
 *  its value is undefined unless the function returns success.
 *
 *  return value:
 *    0    => not found
 *    1..n => id of found pair
 */
static
int CC PTNodeFindExact ( const void *ignore, const PBSTNode *n , void * data)
{
    assert ( n -> data . addr != NULL );
    assert ( n -> data . size > 0 );

    if ( ( ( const char* ) n -> data . addr ) [ 0 ] == 0 )
        return 0;
    return -1;
}

static
int CC PTNodeFindRem ( const void *item, const PBSTNode *n, void * data )
{
    /* compare the remainder of the key */
    const String *rem = item;
    int diff = string_cmp ( rem -> addr, rem -> size,
        n -> data . addr, n -> data . size, rem -> len );

    /* if they differed within the key length, it could
       be that the node is shorter or that there was a
       character mismatch */
    if ( diff != 0 )
        return diff;

    /* the only thing to ascertain now
       is that the lengths are equal */
    if ( ( ( const char* ) n -> data . addr ) [ rem -> size ] == 0 )
        return 0;

    /* the key is shorter */
    return -1;
}

LIB_EXPORT uint32_t CC PTrieFind ( const PTrie *self, const String *item, PTNode *rtn,
        int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data )
{

    if ( rtn == NULL )
        return 0;

    rtn -> data . addr = rtn -> internal = NULL;
    rtn -> data . size = 0;
    rtn -> id = 0;

    if ( self != NULL && item != NULL && self -> num_trans != 0 && item -> len != 0 )
    {
        String key = * item;

        PTTrans *trans;
        rc_t rc = PTrieFindTrans ( self, & key, & trans );

        if ( rc == 0 )
        {
            /* any values in b-tree? */
            if ( trans -> vals != NULL )
            {
                int ( CC * cmp ) ( const void *item, const PBSTNode *n, void *data );
                
                if ( custom_cmp != NULL)
                    cmp = custom_cmp;
                else if ( self -> ext_keys )
                    cmp = NULL;
                /* for exact match on node */
                else if ( key . len == 0 )
                    cmp = PTNodeFindExact;
                /* for remainder */
                else
                    cmp = PTNodeFindRem;

                /* try to find the node */
                if ( cmp != NULL )
                {
                    PBSTNode btnode;
                    uint32_t btid = PBSTreeFind ( trans -> vals, & btnode, & key, cmp, data );
                    if ( btid != 0 )
                    {
                        /* grab the data */
                        rtn -> data . addr = btnode . data . addr;
                        rtn -> data . size = btnode . data . size;

                        /* record reference to self */
                        rtn -> internal = self;

                        /* set an id */
                        rtn -> id = PTrieEncodeNodeId ( self,
                            trans -> tid, btid );

                        /* adjust the data block for internal keys */
                        if ( ! self -> ext_keys )
                        {
                            const char *ptr = ( const char* ) rtn -> data . addr;
                            for ( ; rtn -> data . size > 1 && * ptr != '\0'; ++ ptr, -- rtn -> data . size )
                                ( void ) 0;
                            rtn -> data . addr = ptr + 1; /* skip terminating NUL byte */
                            -- rtn -> data.size;
                        }
                    }
                }
            }

            PTTransWhack ( trans );
        }
    }

    return rtn -> id;
}

LIB_EXPORT uint32_t CC PTrieFindRE ( const PTrie *self, const String *re, PTNode *rtn );

/* FindAll
 * PTrieFindAllRE
 *  find multiple objects within tree
 *
 *  "key" [ IN ] - an exact match text string
 *
 *  "re" [ IN ] - a regular expression string
 *
 *  "buffer" [ OUT ] and "capacity" [ IN ] - a user-supplied
 *  array of PTNode with a capacity of "capacity" elements.
 *  if successful, the entire found set will be returned unordered within.
 *
 *  "num_found" [ OUT ] - indicates the size of the found set,
 *  regardless of return value, such that if the supplied
 *  buffer were to be too small, the required size is returned.
 *
 *  returns status codes:
 *    EINVAL  => an invalid parameter
 *    ENOENT  => the found set was empty
 *    ENOBUFS => the found set was too large
 */
LIB_EXPORT rc_t CC PTrieFindAll ( const PTrie *self, struct String const *key,
    PTNode buffer [], uint32_t capacity, uint32_t *num_found,
    int ( CC * custom_cmp ) ( const void *item, const PBSTNode *n, void *data ), void *data );

LIB_EXPORT rc_t CC PTrieFindAllRE ( const PTrie *self, const String *re,
    PTNode buffer [], uint32_t capacity, uint32_t *num_found );

/* ForEach
 *  executes a function on each tree element
 *
 *  "f" [ IN ] and "data" [ IN ] - iteration callback function for
 *  examining each TTNode in the tree
 */
typedef struct PTrieForEachData PTrieForEachData;
struct PTrieForEachData
{
    const PTrie *tt;
    void ( CC * f ) ( PTNode *n, void *data );
    void *data;
    uint32_t tid;
};

static
void CC PTrieRunForEachVal ( PBSTNode *btn, void *data )
{
    PTNode ttn;
    PTrieForEachData *pb = data;

    ttn . data . addr = btn -> data . addr;
    ttn . data . size = btn -> data . size;
    ttn . internal = pb -> tt;
    ttn . id = PTrieEncodeNodeId ( pb -> tt, pb -> tid, btn -> id );

    if ( ! pb -> tt -> ext_keys )
    {
        size_t sz = strlen ( ttn . data . addr ) + 1;
        assert ( sz <= ttn . data . size );
        ttn . data . addr =
            ( const char* ) ttn . data . addr + sz;
        ttn . data . size -= sz;
    }

    ( * pb -> f ) ( & ttn, pb -> data );
}

static
void PTrieRunForEachTrans ( const PTTrans *trans, const PTrie *tt, void *data )
{
    if ( trans -> vals != NULL )
    {
        PTrieForEachData *pb = data;
        pb -> tid = trans -> tid;
        PBSTreeForEach ( trans -> vals, 0, PTrieRunForEachVal, pb );
    }
}

LIB_EXPORT void CC PTrieForEach ( const PTrie *self,
    void ( CC * f ) ( PTNode *n, void *data ), void *data )
{
  if ( self != NULL && f != NULL && self -> num_trans != 0 )
    {
        PTTrans trans;
        int status = PTrieInitNode ( self, & trans, 1 );
        if ( status == 0 )
        {
            PTrieForEachData pb;
            pb . tt = self;
            pb . f = f;
            pb . data = data;

            PTTransForEach ( & trans, self, PTrieRunForEachTrans, & pb );
        }
    }
}

/* DoUntil
 *  executes a function on each tree element
 *  until the function returns true
 *
 *  "f" [ IN ] and "data" [ IN ] - iteration callback function for
 *  examining each TTNode in the tree. the function returns "true"
 *  to halt iteration.
 *
 *  return values:
 *    the last value returned by "f" or false if never invoked
 */
typedef struct PTrieDoUntilData PTrieDoUntilData;
struct PTrieDoUntilData
{
    const PTrie *tt;
    bool ( CC * f ) ( PTNode *n, void *data );
    void *data;
    uint32_t tid;
};

static
bool CC PTrieRunDoUntilVal ( PBSTNode *btn, void *data )
{
    PTNode ttn;
    PTrieDoUntilData *pb = data;

    ttn . data . addr = btn -> data . addr;
    ttn . data . size = btn -> data . size;
    ttn . internal = pb -> tt;
    ttn . id = PTrieEncodeNodeId ( pb -> tt, pb -> tid, btn -> id );

    if ( ! pb -> tt -> ext_keys )
    {
        size_t sz = strlen ( ttn . data . addr ) + 1;
        assert ( sz <= ttn . data . size );
        ttn . data . addr =
            ( const char* ) ttn . data . addr + sz;
        ttn . data . size -= sz;
    }

    return ( * pb -> f ) ( & ttn, pb -> data );
}

static
bool PTrieRunDoUntilTrans ( const PTTrans *trans, const PTrie *tt, void *data )
{
    if ( trans -> vals != NULL )
    {
        PTrieDoUntilData *pb = data;
        pb -> tid = trans -> tid;
        return PBSTreeDoUntil ( trans -> vals, 0, PTrieRunDoUntilVal, pb );
    }
    return false;
}

LIB_EXPORT bool CC PTrieDoUntil ( const PTrie *self,
    bool ( CC * f ) ( PTNode *n, void *data ), void *data )
{
  if ( self != NULL && f != NULL && self -> num_trans != 0 )
    {
        PTTrans trans;
        rc_t rc = PTrieInitNode ( self, & trans, 1 );
        if ( rc == 0 )
        {
            PTrieDoUntilData pb;
            pb . tt = self;
            pb . f = f;
            pb . data = data;

            return PTTransDoUntil ( & trans, self, PTrieRunDoUntilTrans, & pb );
        }
    }
    return false;
}

/* Whack
 *  tears down internal structure
 */
LIB_EXPORT void CC PTrieWhack ( PTrie *self )
{
    if ( self != NULL )
        free ( self );
}
