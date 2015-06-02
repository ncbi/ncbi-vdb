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
#include "trie-priv.h"
#include "pbstree-priv.h"
#include <klib/container.h>
#include <klib/sort.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#if _DEBUGGING
#define DEBUGPRINT 0
#endif

/*--------------------------------------------------------------------------
 * private declarations
 */
typedef struct IdxMap IdxMap;
struct IdxMap
{
    uint32_t freq;
    uint32_t idx;
    uint32_t ch;
};

static
int64_t CC IdxMapSort ( const void *left, const void *right, void *ignore )
{
#define a ( ( const IdxMap* ) left )
#define b ( ( const IdxMap* ) right )

    if ( b -> freq != a -> freq )
        return ( int64_t ) b -> freq - ( int64_t ) a -> freq;
    return ( int64_t ) a -> idx - ( int64_t ) b -> idx;

#undef a
#undef b
}

typedef struct PTChildNode PTChildNode;
struct PTChildNode
{
    SLNode n;
    const TTrans *trans;
    uint32_t dad;
    uint16_t idx;
};

static
void CC PTChildNodeWhack ( SLNode *n, void *ignore )
{
    free ( n );
}

typedef struct PTriePersistData PTriePersistData;
struct PTriePersistData
{
    PTWriteFunc write;
    void *write_param;

    PTAuxFunc aux;
    void *aux_param;

    PTAuxFunc null_write;
    PTAuxFunc live_write;

    IdxMap *idx_map;
    size_t *trans_map;

    void ( CC * record_idx ) ( void*, uint32_t, uint32_t );
    void ( CC * record_trans ) ( void*, uint32_t, uint32_t );

    union
    {
        uint8_t *v8;
        uint16_t *v16;
        uint32_t *v32;

    } idx_seq;

    uint8_t *child_seq_type;

    union
    {
        uint8_t *v8;
        uint16_t *v16;
        uint32_t *v32;

    } backtrace;

    union
    {
        uint8_t *v8;
        uint16_t *v16;
        uint32_t *v32;

    } trans_seq;

    size_t data_size;

    uint32_t num_nodes;
    uint32_t max_nodes;

    uint32_t num_trans;
    uint32_t trans_id;
    uint32_t child_id;

    uint32_t min_width;
    uint32_t depth;

    rc_t rc;

    uint8_t idx_size;
    uint8_t trans_size;
    uint8_t off_size;
};

static
rc_t CC PTAlign ( PTriePersistData *pb, size_t *offset,
    uint32_t size, uint32_t first_byte )
{
    rc_t rc;

    union
    {
        uint8_t u8 [ 4 ];
        uint32_t u32;
    } u;
    size_t to_write, num_writ;

    int align = (int)( offset [ 0 ] & ( size - 1 ) );
    if ( align == 0 )
        return 0;

#if DEBUGPRINT
    fprintf(stderr, "PTAlign called.\n");
#endif


    to_write = size - align;

    u . u32 = 0;
    u . u8 [ 0 ] = ( uint8_t ) first_byte;

    num_writ = 0;
    rc = ( * pb -> write ) ( pb -> write_param, u . u8, to_write, & num_writ );

    * offset += num_writ;

    if ( rc != 0 )
        return rc;
    if ( num_writ != to_write )
        return RC ( rcCont, rcTrie, rcPersisting, rcTransfer, rcIncomplete );
    return 0;
}

static
rc_t CC TNodeDataSize ( void *param, const void *n, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    PTriePersistData *pb = param;
    rc_t rc = ( * pb -> aux ) ( pb -> aux_param, n, num_writ, write, write_param );
    if ( rc == 0 )
    {
        String str;
        const TNode *keynode = n;
        if ( StringSubstr ( & keynode -> key, & str, pb -> depth, 0 ) == NULL )
            * num_writ += 1;
        else
            * num_writ += str . size + 1;
    }
    return rc;
}

static
rc_t CC TNodeWrite ( void *param, const void *n, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    PTriePersistData *pb = param;
    const TNode *keynode = n;

    rc_t rc;
    String str;

#if DEBUGPRINT
    fprintf(stderr, "TNodeWrite called.\n");
#endif


    if ( write == NULL )
        return TNodeDataSize ( param, n, num_writ, write, write_param );

    /* TBD - find out why string is written with pb->write and NUL is written with write
       they should be the same, and so the entire NUL terminated string should be done at once */
    if ( StringSubstr ( & keynode -> key, & str, pb -> depth, 0 ) != NULL )
        rc = ( * pb -> write ) ( pb -> write_param, str . addr, str . size, num_writ );
    else
    {
        rc = 0;
        * num_writ = 0;
    }
    if ( rc == 0 )
    {
        size_t writ;
        char term [ 1 ];

        term [ 0 ] = 0;
        rc = ( * write ) ( write_param, term, 1, & writ );
        if ( rc == 0 )
        {
            * num_writ += writ;
            if ( writ != 1 )
                return -1; /*AK: TODO is it correct?*/

            writ = 0;
            rc = ( * pb -> aux ) ( pb -> aux_param, n, & writ, write, write_param );
            * num_writ += writ;
        }
    }

    return rc;
}

static
rc_t CC NullWrite ( void *param, const void *buffer, size_t bytes, size_t *num_writ )
{
    * num_writ = bytes;
    return 0;
}


/*--------------------------------------------------------------------------
 * TTrans
 */
static
void CC TTransCountNodes ( BSTNode *n, void *data )
{
    /* count node */
    ++ ( * ( uint32_t* ) data );
}

static
void CC TTransCountTrans ( const TTransBacktrace *bt, uint32_t width, void *data )
{
    PTriePersistData *pb = data;
    const TTrans *trans = bt -> trans;

    /* count transition node */
    ++ pb -> num_trans;

    /* count value nodes */
    if ( trans -> vals . root != NULL )
    {
        uint32_t num_nodes = 0;
        BSTreeForEach ( & trans -> vals, 0, TTransCountNodes, & num_nodes );

        /* accumulate into overall counter */
        pb -> num_nodes += num_nodes;

        /* record max */
        if ( num_nodes > pb -> max_nodes )
            pb -> max_nodes = num_nodes;
    }

    /* analyze child transition array and character set */
    if ( trans -> child != NULL )
    {
        uint32_t i;
        for ( i = 0; i < width; ++ i )
        {
            if ( trans -> child [ i ] != NULL )
                ++ pb -> idx_map [ i ] . freq;
        }
    }
}

static
void CC TTransRecordU8 ( void *buffer, uint32_t idx, uint32_t val )
{
    ( ( uint8_t* ) buffer ) [ idx ] = ( uint8_t ) val;
}

static
void CC TTransRecordU16 ( void *buffer, uint32_t idx, uint32_t val )
{
    ( ( uint16_t* ) buffer ) [ idx ] = ( uint16_t ) val;
}

static
void CC TTransRecordU32 ( void *buffer, uint32_t idx, uint32_t val )
{
    ( ( uint32_t* ) buffer ) [ idx ] = ( uint32_t ) val;
}

static
bool CC TTransPersist1 ( const TTrans *trans, PTriePersistData *pb,
    uint32_t tid, uint32_t dad, uint16_t idx, SLList *sl,
    uint32_t *icntp, uint32_t *slenp, uint32_t *tcntp )
{
    uint32_t icnt, slen, tcnt;

    void ( CC * record_idx ) ( void*, uint32_t, uint32_t ) = pb -> record_idx;
    void ( CC * record_trans ) ( void*, uint32_t, uint32_t ) = pb -> record_trans;

#if DEBUGPRINT
    fprintf(stderr, "TTransPersist1 called.\n");
#endif


    /* set up header */
#if RECORD_HDR_IDX
#if DEBUGPRINT
    fprintf(stderr, "Recording header index %d, %d\n", pttHdrIdx, idx);
#endif
    /* zero-based character code of transition */
    ( * record_idx ) ( pb -> idx_seq . v8, pttHdrIdx, idx );
#endif
#if RECORD_HDR_DEPTH
#if DEBUGPRINT
    fprintf(stderr, "Recording header depth %d, %d\n", pttHdrDepth, trans->depth);
#endif
    /* node depth */
    ( * record_idx ) ( pb -> idx_seq . v8, pttHdrDepth, trans -> depth );
#endif
    /* the number of transitions; in child array or in vals btree */
    ( * record_idx ) ( pb -> idx_seq . v8, pttHdrTransCnt, trans -> tcnt );

    /* record one-based parent backtrace */
    if ( pb -> backtrace . v8 != NULL )
        ( * record_trans ) ( pb -> backtrace . v8, 0, dad );

    /* record child transition table */
    if ( trans -> child == NULL )
    {
        /* no sequence or indices beyond header */
        icnt = slen = tcnt = 0;
        ( * record_idx ) ( pb -> idx_seq . v8, pttHdrIdxCnt, 0 );
    }
    else
    {
        uint32_t i;

        /* set all index entries to simple type
           the working array is large enough to hold
           one bit per character code */
        memset ( pb -> child_seq_type, 0, ( pb -> min_width + 7 ) >> 3 );

        /* walk child array:
           i    => zero-based character code and
                   indirect index into trans.child
           tcnt => persisted child index and counter
           icnt => idx index and counter
           slen => child_seq_type index and counter */
        for ( tcnt = icnt = slen = i = 0; i < pb -> min_width; ++ i )
        {
            PTChildNode *n;

            /* look only at valid transitions
               notice that i, which represents the new character code,
               is first mapped to an old character code */
            if ( trans -> child [ pb -> idx_map [ i ] . idx ] != NULL )
            {
                /* record trans into child slot and allocate id */
                ( * record_trans ) ( pb -> trans_seq . v8, tcnt ++, ++ pb -> child_id );

                /* record index, either single or lower part of pair
                   index value is the new character code for transition */
                ( * record_idx ) ( pb -> idx_seq . v8, ++ icnt + pttLastHdr, i );

                /* create a node for next trans to visit */
                n = malloc ( sizeof * n );
                if ( n == NULL )
                    return true;
                n -> trans = trans -> child [ pb -> idx_map [ i ] . idx ];
                n -> dad = tid;
                n -> idx = i;
                SLListPushTail ( sl, & n -> n );

                /* determine type
                   notice that "i" is incremented, even though it will be
                   incremented by for loop. the effect of doing so, as well
                   as the effect of the following compound statement, is to
                   leave "i" such that child [ i ] is NULL, in which case
                   we're going to skip it anyway */
                if ( ++ i < pb -> min_width &&
                     trans -> child [ pb -> idx_map [ i ] . idx ] != NULL )
                {
                    /* it's a range */
                    pb -> child_seq_type [ slen >> 3 ] |=
                        ( uint8_t ) ( 1U << ( slen & 7 ) );

                    /* record each trans, allocating a new child id */
                    ( * record_trans ) ( pb -> trans_seq . v8, tcnt ++, ++ pb -> child_id );
                    n = malloc ( sizeof * n );
                    if ( n == NULL )
                        return true;
                    n -> trans = trans -> child [ pb -> idx_map [ i ] . idx ];
                    n -> dad = tid;
                    n -> idx = i;
                    SLListPushTail ( sl, & n -> n );

                    /* continue until end */
                    for ( ++ i; i < pb -> min_width; ++ i )
                    {
                        /* detect end of range */
                        if ( trans -> child [ pb -> idx_map [ i ] . idx ] == NULL )
                            break;
                        ( * record_trans ) ( pb -> trans_seq . v8, tcnt ++, ++ pb -> child_id );
                        n = malloc ( sizeof * n );
                        if ( n == NULL )
                            return true;
                        n -> trans = trans -> child [ pb -> idx_map [ i ] . idx ];
                        n -> dad = tid;
                        n -> idx = i;
                        SLListPushTail ( sl, & n -> n );
                    }

                    /* i is one beyond end of index range, but
                       we store the range as fully closed */
                    ( * record_idx ) ( pb -> idx_seq . v8, ++ icnt + pttLastHdr, i - 1 );
                }

                /* count the index sequence bit */
                ++ slen;
            }
        }

        /* complete the header for the case where child array was present */
        ( * record_idx ) ( pb -> idx_seq . v8, pttHdrIdxCnt, icnt );
        ( * record_idx ) ( pb -> idx_seq . v8, pttHdrSeqLen, slen );
        ( * record_idx ) ( pb -> idx_seq . v8, pttHdrChildCnt, tcnt );
    }

    * icntp = icnt;
    * slenp = slen;
    * tcntp = tcnt;

    return false;
}

static
bool CC TTransPersist ( const TTrans *trans, PTriePersistData *pb,
     uint32_t dad, uint16_t idx, SLList *sl )
{
    uint32_t ztid;
    uint32_t icnt, slen, tcnt;
    size_t to_write, num_writ, entry_size;

#if DEBUGPRINT
    fprintf(stderr, "TTransPersist called.\n");
#endif


    /* grab my zero-based trans id */
    ztid = pb -> trans_id ++;
    assert ( ztid < pb -> num_trans );

    /* grab entry size */
    entry_size = pb -> data_size;

    /* assign initial size as zero */
    pb -> trans_map [ ztid ] = 0;

    /* populate tables and record child iteration nodes */
    if ( TTransPersist1 ( trans, pb, ztid + 1, dad, idx, sl, & icnt, & slen, & tcnt ) )
        return true;

    /* write TTrans header data */
    if ( icnt == 0 )
    {
        /* simple header for TTrans with no child array */
        to_write = pttHdrNullEnd * pb -> idx_size;
        pb -> rc = ( * pb -> write ) ( pb -> write_param,
            pb -> idx_seq . v8, to_write, & num_writ );

        /* write parent backtrace if needed */
        if ( pb -> backtrace . v8 != NULL && pb -> rc == 0 && num_writ == to_write )
        {
            pb -> data_size += num_writ;
            pb -> rc = PTAlign ( pb, & pb -> data_size, pb -> trans_size, 0 );
            if ( pb -> rc == 0 )
            {
                to_write = pb -> trans_size;
                pb -> rc = ( * pb -> write ) ( pb -> write_param,
                    pb -> backtrace . v8, to_write, & num_writ );
            }
        }
    }
    else
    {
        /* write index array */
        to_write = ( icnt + pttFirstIdx ) * pb -> idx_size;
        pb -> rc = ( * pb -> write ) ( pb -> write_param,
            pb -> idx_seq . v8, to_write, & num_writ );

        /* proceed if no error and completely written
           the incomplete write will be detected below */
        if ( pb -> rc == 0 && num_writ == to_write )
        {
            pb -> data_size += num_writ;

            /* write index type array */
            assert ( slen != 0 );
            to_write = ( slen + 7 ) >> 3;
            pb -> rc = ( * pb -> write ) ( pb -> write_param,
                pb -> child_seq_type, to_write, & num_writ );
            if ( pb -> rc == 0 && num_writ == to_write )
            {
                /* align for transition size */
                pb -> data_size += num_writ;
                pb -> rc = PTAlign ( pb, & pb -> data_size, pb -> trans_size, 0 );
                if ( pb -> rc == 0 )
                {
                    /* write parent backtrace if needed */
                    if ( pb -> backtrace . v8 != NULL )
                    {
                        to_write = pb -> trans_size;
                        pb -> rc = ( * pb -> write ) ( pb -> write_param,
                            pb -> backtrace . v8, to_write, & num_writ );
                        if ( pb -> rc != 0 )
                            return true;
                        if ( num_writ != to_write )
                        {
                            pb -> rc = RC ( rcCont, rcTrie, rcPersisting, rcTransfer, rcIncomplete );
                            return true;
                        }

                        pb -> data_size += num_writ;
                    }

                    /* write transition array */
                    to_write = tcnt * pb -> trans_size;
                    pb -> rc = ( * pb -> write ) ( pb -> write_param,
                        pb -> trans_seq . v8, to_write, & num_writ );
                }
            }
        }
    }

    if ( pb -> rc != 0 )
        return true;

    pb -> data_size += num_writ;
    if ( num_writ != to_write )
    {
        pb -> rc = RC ( rcCont, rcTrie, rcPersisting, rcTransfer, rcIncomplete );
        return true;
    }

    /* record trans' depth for internal key storage */
    pb -> depth = trans -> depth;

    /* detect need to align for data */
    if ( ( pb -> data_size & 3 ) != 0 )
    {
        /* may be able to bail before writing b-tree */
        if ( trans -> vals . root == NULL )
        {
            pb -> rc = PTAlign ( pb, & pb -> data_size, 4, 0 );
            if ( pb -> rc != 0 )
                return true;

            /* record the size */
            pb -> trans_map [ ztid ] = pb -> data_size - entry_size;
            return false;
        }

        /* perform alignment */
        pb -> rc = PTAlign ( pb, & pb -> data_size, 4, 1 );
        if ( pb -> rc != 0 )
            return true;
    }

    /* detect fake pass */
    if ( pb -> write == NullWrite )
    {
        /* gather sizes */
        pb -> rc = BSTreePersist ( & trans -> vals, & num_writ,
            NULL, NULL, pb -> null_write, pb );
    }
    else
    {
        /* write BSTree for values */
        pb -> rc = BSTreePersist ( & trans -> vals, & num_writ,
            pb -> write, pb -> write_param, pb -> live_write, pb );
    }

    pb -> data_size += num_writ;

    /* align3 */
    if ( pb -> rc == 0 )
        pb -> rc = PTAlign ( pb, & pb -> data_size, 4, 0 );

    if ( pb -> rc != 0 )
        return true;

    pb -> trans_map [ ztid ] = pb -> data_size - entry_size;
    return false;
}

/* TTransPIterate
 *  walks TTrans nodes in a predictive order
 *  using a queue
 */
static
bool CC TTransPIterate ( const TTrans *self, PTriePersistData *pb )
{
    PTChildNode *n;

    SLList sl;
    SLListInit ( & sl );

#if DEBUGPRINT
    fprintf(stderr, "TTransPIterate called.\n");
#endif


    pb -> trans_id = pb -> child_id = 0;

    /* prime list with root node */
    n = malloc ( sizeof * n );
    if ( n == NULL )
        return true;

    n -> trans = self;
    n -> dad = 0;
    n -> idx = 0;

    /* now walk the list */
    for ( ; n != NULL; n = ( PTChildNode* ) SLListPopHead ( & sl ) )
    {
        const TTrans *trans = n -> trans;
        uint32_t dad = n -> dad;
        uint16_t idx = n -> idx;

        free ( n );

        if ( TTransPersist ( trans, pb, dad, idx, & sl ) )
        {
            SLListWhack ( & sl, PTChildNodeWhack, NULL );
            return true;
        }
    }

    return false;
}


/*--------------------------------------------------------------------------
 * Trie
 */

/* TriePersist
 *  much like BSTreePersist but operates on a Trie
 *
 *  "num_writ" [ OUT, NULL OKAY ] -  returns parameter for the number
 *  of bytes written as a result of persisting the b-tree. this will
 *  be the actual bytes written regardless of return status.
 *
 *  "ext_keys" [ IN ] - if true, does not store key data internally.
 *  key text gets encoded into the tree structure in all cases. when
 *  stored internally, any key text not represented by the tree will
 *  be explicitly written by this function. otherwise, the caller will
 *  be expected to store the text as desired. while internal storage
 *  will probably be more efficient, it will cause this function to
 *  fail if the Trie was built with ambiguous key transitions, i.e.
 *  if initialized with "cs_expand" false and keys were added having
 *  characters not included within the "accept" character set. this
 *  is because the tree channels all unrecognized characters through
 *  a single code, making their recovery impossible without being
 *  stored externally.
 *
 *  "write" [ IN, NULL OKAY ] and "write_param" [ IN ] -  a generic
 *  output streaming function used for all operations. if NULL, then
 *  the function will exit after its first pass with the number of
 *  bytes required in "num_writ".
 *
 *  "aux" [ IN ] and "aux_param" [ IN ] - a specialized function for
 *  streaming auxiliary node data to output using the supplied "write"
 *  function. it is invoked during the first pass with a NULL write
 *  function for gathering size data, and during the third pass with
 *  a non-NULL write function.
 */
static
rc_t CC TriePersist0 ( size_t *num_writ, int ext_keys,
   PTWriteFunc write, void *write_param )
{
    rc_t rc;
    P_Trie ptt;

    assert ( num_writ != NULL );

#if DEBUGPRINT
    fprintf( stderr, "TriePersist0 called.\n" );
#endif

    ptt . num_trans = 0;
    ptt . num_nodes = 0;
    ptt . data_size = 0;
    ptt . keys = 0;
#if EXTENDED_PTRIE
    ptt . ext_data_size = 0;
#endif
    ptt . width = 0;

    P_TrieSetExtKeys ( ptt . keys, ext_keys );
    P_TrieSetBacktrace ( ptt . keys, ! ext_keys );

    rc = ( * write ) ( write_param, & ptt, 16, num_writ );
    if ( rc == 0 && * num_writ != 16 )
        return RC ( rcCont, rcTrie, rcPersisting, rcTransfer, rcIncomplete );
    return rc;
}

static
void CC TriePersist1 ( const Trie *tt, PTriePersistData *pb )
{
    uint16_t i;

    assert ( tt != NULL );
    assert ( pb != NULL );
    assert ( tt -> root != NULL );
    assert ( pb -> idx_map != NULL );

#if DEBUGPRINT
    fprintf(stderr, "TriePersist1 called.\n");
#endif

    /* initialize statistics */
    for ( i = 0; i < tt -> width; ++ i )
    {
        pb -> idx_map [ i ] . freq = 0;
        pb -> idx_map [ i ] . idx = i;
        pb -> idx_map [ i ] . ch = tt -> rmap [ i ];
    }

    /* count nodes and transition frequencies */
    pb -> num_trans = 0;
    pb -> num_nodes = 0;
    pb -> max_nodes = 0;
    TTransForEach ( tt -> root, tt -> width, TTransCountTrans, pb );

    /* rearrange the indices by frequency */
    ksort ( pb -> idx_map, tt -> width, sizeof pb -> idx_map [ 0 ], IdxMapSort, NULL );

    /* determine real charset width by searching for
       the first slot with zero transitions */
    for ( i = 0; i < tt -> width; ++ i )
    {
        if ( pb -> idx_map [ i ] . freq == 0 )
            break;
    }
    pb -> min_width = i;

    /* translate character set width and node count */
    if ( pb -> min_width <= 256 )
    {
        pb -> record_idx = TTransRecordU8;
        pb -> idx_size = 1;
    }
    else if ( pb -> min_width <= 65536 )
    {
        pb -> record_idx = TTransRecordU16;
        pb -> idx_size = 2;
    }
    else
    {
        pb -> record_idx = TTransRecordU32;
        pb -> idx_size = 4;
    }

    if ( pb -> num_trans <= 256 )
    {
        pb -> record_trans = TTransRecordU8;
        pb -> trans_size = 1;
    }
    else if ( pb -> num_trans <= 65536 )
    {
        pb -> record_trans = TTransRecordU16;
        pb -> trans_size = 2;
    }
    else
    {
        pb -> record_trans = TTransRecordU32;
        pb -> trans_size = 4;
    }
}

static
rc_t CC TriePersist2 ( const Trie *tt, PTriePersistData *pb )
{
    assert ( tt != NULL );
    assert ( pb != NULL );
    assert ( pb -> write == NullWrite );

#if DEBUGPRINT
    fprintf(stderr, "TriePersist2 called.\n");
#endif

    pb -> trans_id = 0;
    pb -> data_size = 0;
    TTransPIterate ( tt -> root, pb );
    if ( pb -> rc == 0 )
    {
        /* we should have visited every TTrans node */
        assert ( pb -> trans_id == pb -> num_trans );

        /* size of trans offset array slot
           since the persisted TTrans objects
           are 4-byte aligned, we may be able to
           get some savings */
        if ( pb -> data_size <= 256 * 4 )
            pb -> off_size = 1;
        else if ( pb -> data_size <= 65536 * 4 )
            pb -> off_size = 2;
        else
            pb -> off_size = 4;
    }

    return pb -> rc;
}

static
rc_t CC TriePersist3 ( const Trie *tt, PTriePersistData *pb, int ext_keys,
    PTWriteFunc write, void *write_param, size_t *num_writ )
{
    P_Trie *pt;
    size_t trans_offset;

#if DEBUGPRINT
    fprintf(stderr, "TriePersist3 called.\n");
#endif

    assert ( tt != NULL );
    assert ( pb != NULL );
    assert ( write != NULL );
    assert ( num_writ != NULL );

    /* detect size overflow */
    if ( sizeof pb -> data_size > 4 )
    {
#if EXTENDED_PTRIE
        if ( ( ( uint64_t ) ( pb -> data_size - 1 ) >> 34 ) != 0 )
            return pb -> rc = RC ( rcCont, rcTrie, rcPersisting, rcData, rcExcessive );
#else
        if ( ( ( uint64_t ) pb -> data_size >> 32 ) != 0 )
            return pb -> rc = RC ( rcCont, rcTrie, rcPersisting, rcData, rcExcessive );
#endif
    }

    /* figure trans_offset up to trans member */
    trans_offset = sizeof * pt - sizeof pt -> rmap +
        pb -> min_width * sizeof pt -> rmap [ 0 ];

    /* create the real deal */
    pt = malloc ( trans_offset + pb -> num_trans * pb -> off_size );
    if ( pt == NULL )
        pb -> rc = RC ( rcCont, rcTrie, rcPersisting, rcMemory, rcExhausted );
    else
    {
        uint32_t i;
        size_t data_offset;
        size_t pt_data_size = pb -> data_size;

        /* initialize header */
        pt -> num_trans = pb -> num_trans;
        pt -> num_nodes = pb -> num_nodes;
        pt -> data_size = ( uint32_t ) pt_data_size;
#if EXTENDED_PTRIE
        if ( sizeof pb -> data_size == 4 )
            pt -> ext_data_size = 0;
        else
            pt -> ext_data_size = ( uint8_t ) ( ( uint64_t ) pt_data_size >> 32 );
#endif
        pt -> keys = 0;
        P_TrieSetExtKeys ( pt -> keys, ext_keys );
        P_TrieSetBacktrace ( pt -> keys, ! ext_keys );
        pt -> width = pb -> min_width;

        /* decide upon id encoding type:
           0 => 24 : 8, i.e. 24 bits for trans id, 8 bits for local btree id
           1 => 22 : 10
           2 => 20 : 12
           3 => 18 : 14
           4 => 16 : 16
           5 => 14 : 18
           6 => 12 : 20
           7 => by offset into data section
        */
#define VALID_IDCODE( pb, trans, bt ) \
    ( ( pb ) -> num_trans <= ( 1U << ( trans ) ) && \
      ( pb ) -> max_nodes <= ( 1U << ( bt ) ) )

        if ( VALID_IDCODE ( pb, 24, 8 ) )
            P_TrieSetIdCoding ( pt -> keys, 0 );
        else if ( VALID_IDCODE ( pb, 22, 10 ) )
            P_TrieSetIdCoding ( pt -> keys, 1 );
        else if ( VALID_IDCODE ( pb, 20, 12 ) )
            P_TrieSetIdCoding ( pt -> keys, 2 );
        else if ( VALID_IDCODE ( pb, 18, 14 ) )
            P_TrieSetIdCoding ( pt -> keys, 3 );
        else if ( VALID_IDCODE ( pb, 16, 16 ) )
            P_TrieSetIdCoding ( pt -> keys, 4 );
        else if ( VALID_IDCODE ( pb, 14, 18 ) )
            P_TrieSetIdCoding ( pt -> keys, 5 );
        else if ( VALID_IDCODE ( pb, 12, 20 ) )
            P_TrieSetIdCoding ( pt -> keys, 6 );
        else
            P_TrieSetIdCoding ( pt -> keys, 7 );

#undef VALID_IDCODE

        /* fill out rmap */
        for ( i = 0; i < ( uint32_t ) pb -> min_width; ++ i )
            pt -> rmap [ i ] = pb -> idx_map [ i ] . ch;

        /* fill out transition offset table */
        if ( pb -> off_size == 1 )
        {
            uint8_t *trans = ( void* ) ( ( char* ) pt + trans_offset );
            data_offset = trans_offset + pb -> num_trans;

            for ( trans_offset = 0, i = 0; i < pb -> num_trans; ++ i )
            {
                assert ( ( trans_offset & 3 ) == 0 );
                trans [ i ] = ( uint8_t ) ( trans_offset >> 2 );
                trans_offset += pb -> trans_map [ i ];
            }
        }
        else if ( pb -> off_size == 2 )
        {
            uint16_t *trans = ( void* ) ( ( char* ) pt + trans_offset );
            data_offset = trans_offset + ( pb -> num_trans << 1 );

            for ( trans_offset = 0, i = 0; i < pb -> num_trans; ++ i )
            {
                assert ( ( trans_offset & 3 ) == 0 );
                trans [ i ] = ( uint16_t ) ( trans_offset >> 2 );
                trans_offset += pb -> trans_map [ i ];
            }
        }
        else
        {
            uint32_t *trans = ( void* ) ( ( char* ) pt + trans_offset );
            data_offset = trans_offset + ( pb -> num_trans << 2 );

            for ( trans_offset = 0, i = 0; i < pb -> num_trans; ++ i )
            {
                assert ( ( trans_offset & 3 ) == 0 );
                trans [ i ] = ( uint32_t ) ( trans_offset >> 2 );
                trans_offset += pb -> trans_map [ i ];
            }
        }

        /* the last offset should equal total data bytes */
        assert ( trans_offset == pt_data_size );

        /* write the tree header */
        pb -> rc = ( * write ) ( write_param, pt, data_offset, num_writ );
        if ( pb -> rc == 0 && * num_writ != data_offset )
            pb -> rc = RC ( rcCont, rcTrie, rcPersisting, rcTransfer, rcIncomplete );

        free ( pt );
    }

    return pb -> rc;
}

LIB_EXPORT rc_t CC TriePersist ( const Trie *tt, size_t *num_writ, bool ext_keys,
    PTWriteFunc write, void *write_param, PTAuxFunc aux, void *aux_param )
{
    PTriePersistData pb;
    size_t num_writ_buffer;

#if DEBUGPRINT
    fprintf(stderr, "TriePersist called\n");
#endif

    if ( num_writ == NULL )
        num_writ = & num_writ_buffer;

    if ( write == NULL )
    {
        write = NullWrite;
        write_param = NULL;
    }

    * num_writ = 0;

    if ( tt == NULL )
        return RC ( rcCont, rcTrie, rcPersisting, rcSelf, rcNull );
    if ( aux == NULL )
        return RC ( rcCont, rcTrie, rcPersisting, rcFunction, rcNull );

    /* handle empty tree */
    if ( tt -> root == NULL )
        return TriePersist0 ( num_writ, ext_keys, write, write_param );

    pb . write = NullWrite;
    pb . write_param = NULL;
    pb . aux = aux;
    pb . aux_param = aux_param;
    pb . rc = 0;

    if ( ext_keys )
    {
        pb . null_write = aux;
        pb . live_write = aux;
    }
    else
    {
        pb . null_write = TNodeDataSize;
        pb . live_write = TNodeWrite;
    }

    /* prepare a statistical array for character transition analysis */
    pb . idx_map = malloc ( tt -> width * sizeof pb . idx_map [ 0 ] );
    if ( pb . idx_map == NULL )
        pb . rc = RC ( rcCont, rcTrie, rcPersisting, rcMemory, rcExhausted );
    else
    {
        /* analyze table dimensions based upon counts */
        TriePersist1 ( tt, & pb );

        /* time to allocate some memory
           allocate trans_map based upon the number of TTrans objects
           and idx_seq for persisting a single child array */
        pb . trans_map = malloc (
            /* the TTrans node size/offset array */
            pb . num_trans * sizeof pb . trans_map [ 0 ] +
            /* maximum space for all indices, all transitions */
            ( pb . idx_size + pb . trans_size ) * pb . min_width +
            /* space for parent backtrace transition */
            pb . trans_size +
            /* maximum space for sequence entry type bits */
            ( ( pb . min_width + 7 ) >> 3 ) +
            /* space for header entries in index array */
            ( pb . idx_size * pttFirstIdx ) +
            /* ample space for align1 */
            pb . trans_size );
        if ( pb . trans_map == NULL )
            pb . rc = RC ( rcCont, rcTrie, rcPersisting, rcMemory, rcExhausted );
        else
        {
            /* set up idx_seq from trans_map */
            pb . idx_seq . v8 = ( uint8_t* ) pb . trans_map +
                pb . num_trans * sizeof pb . trans_map [ 0 ];

            /* assign pointers to parts */
            pb . child_seq_type =
                & pb . idx_seq . v8 [ pb . idx_size * ( pttFirstIdx + pb . min_width ) ];
            pb . backtrace . v8 = NULL;
            pb . trans_seq . v8 =
                pb . child_seq_type + ( ( pb . min_width + 7 ) >> 3 );
            if ( ! ext_keys )
            {
                pb . backtrace . v8 = pb . trans_seq . v8;
                pb . trans_seq . v8 += pb . trans_size;
            }

            /* perform a dry-run to obtain overall size */
            if ( TriePersist2 ( tt, & pb ) == 0 )
            {
                /* create, initialize and write the P_Trie structure */
                if ( TriePersist3 ( tt, & pb, ext_keys, write, write_param, num_writ ) == 0 )
                {
                    /* use the real write function */
                    pb . write = write;
                    pb . write_param = write_param;

                    /* align to 4 byte boundary */
                    pb . rc = PTAlign ( & pb, num_writ, 4, 0 );
                    if ( pb . rc == 0 )
                    {
                        /* write the nodes and data */
                        pb . trans_id = 0;
                        pb . data_size = 0;
                        TTransPIterate ( tt -> root, & pb );
                        * num_writ += pb . data_size;
                    }
                }
            }

            free ( pb . trans_map );
        }

        free ( pb . idx_map );
    }

    return pb . rc;
}
