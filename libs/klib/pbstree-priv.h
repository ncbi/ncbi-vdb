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

#ifndef _h_pbstree_priv_
#define _h_pbstree_priv_

#ifndef _h_klib_pbstree_
#include <klib/pbstree.h>
#endif

#ifndef _h_klib_ptrie_
#include <klib/ptrie.h>
#endif

#include <endian.h>


#define RECORD_HDR_IDX   1
#define RECORD_HDR_DEPTH 1
#define EXTENDED_PTRIE   1


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * P_BSTree
 *  persisted binary search tree
 *
 *  in memory, the tree is built from nodes with 3 pointers. this
 *  arrangement allows parent identification, left & right child
 *  identification, as well as leaf node indication.
 *
 *  when persisted, the tree nodes are identified by index integer.
 *  this integer may be 8, 16 or 32 bits as determined by the node count.
 */
typedef struct P_BSTree P_BSTree;
struct P_BSTree
{
    /* a node count - if zero, then the structure ends */
    uint32_t num_nodes;

    /* the total size of auxiliary data */
    uint32_t data_size;

    /* offsets relative to & data_idx [ num_nodes ] */
    union
    {
        uint8_t v8 [ 4 ];
        uint16_t v16 [ 2 ];
        uint32_t v32 [ 1 ];
    } data_idx;

#if 0
    uint8_t data [ data_size ];
#endif
};


/*--------------------------------------------------------------------------
 * PBSTree_vt
 *  versioned message dispatch table
 */
#ifndef PBSTREE_IMPL
#define PBSTREE_IMPL PBSTree
#endif

typedef struct PBSTree_vt_v1 PBSTree_vt_v1;
struct PBSTree_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    void ( CC * destroy ) ( PBSTREE_IMPL *self );
    uint32_t ( CC * count ) ( const PBSTREE_IMPL *self );
    uint32_t ( CC * depth ) ( const PBSTREE_IMPL *self );
    size_t ( CC * size ) ( const PBSTREE_IMPL *self );
    rc_t ( CC * get_node_data ) ( const PBSTREE_IMPL *self,
        const void **addr, size_t *size, uint32_t id );
    uint32_t ( CC * find ) (  const PBSTREE_IMPL *self, PBSTNode *rtn,
        const void *item, int ( CC * cmp ) ( const void*, const PBSTNode* , void* ), void *data );
    void ( CC * for_each ) ( const PBSTREE_IMPL *self, bool reverse,
        void ( CC * f ) ( PBSTNode*, void* ), void *data );
    bool ( CC * do_until ) ( const PBSTREE_IMPL *self, bool reverse,
        bool ( CC * f ) ( PBSTNode*, void* ), void *data );
    /* end minor version == 0 */
};

/* latest major version */
#define PBSTREE_LATEST 1

typedef union PBSTree_vt PBSTree_vt;
union PBSTree_vt
{
    PBSTree_vt_v1 v1;
};

/*--------------------------------------------------------------------------
 * PBSTree
 *  handle to persisted binary search tree
 */
struct PBSTree
{
    const PBSTree_vt *vt;
    const P_BSTree *pt;
};


/* Make
 *  swapped and native versions
 *  take a pre-allocated but uninitialized PBSTree structure
 */
rc_t PBSTreeMakeNative ( PBSTree *pt, const void *addr, size_t size );
rc_t PBSTreeMakeSwapped ( PBSTree *pt, const void *addr, size_t size );


/* Init
 *  initialize the parent class
 */
rc_t PBSTreeInit ( PBSTree *self, const PBSTree_vt *vt, const P_BSTree *pt );


/* PBSTreeGetNodeData
 *  finds node data boundaries
 */
rc_t PBSTreeGetNodeData ( const PBSTree *self,
    const void **addr, size_t *size, uint32_t id );

/* PBSTreeDump
 *  dumps a persisted binary search tree
 *
 *  "name" [ IN, NULL OKAY ] - optional tree name
 *
 *  "stdio_file" [ IN ] - an output FILE* opaquely typed to avoid
 *  requiring an include of stdio.h
 *
 *  "indent_level" [ IN ] - the indent tabbing amount for output
 *  the implementation uses a tab setting of 2 spaces
 *
 *  "dump_node" [ IN, NULL OKAY ] and "data" [ IN ] - an optional
 *  node dumping function and associated data
 */
void PBSTreeDump ( const PBSTree *self, const char *name,
    void *stdio_file, uint32_t indent_level,
    void ( CC * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );

/* GetIndentString
 *  returns a string of spaces for indentation
 */
const char *GetIndentString ( uint32_t indent_level, uint32_t indent_tab );


/*--------------------------------------------------------------------------
 * P_TTrans
 * PTTrans
 *  the index is represented as a min-sized uint32_t,
 *  the child transitions are represented by two parallel
 *  sequences described below, and data values have an in-line
 *  PTBTree.
 */
enum
{
#if RECORD_HDR_IDX
    /* transition index from parent to here */
    pttHdrIdx,
#endif
#if RECORD_HDR_DEPTH
    /* tree depth of node, i.e. transitions from root */
    pttHdrDepth,
#endif
    /* tcnt from TTrans, meaning the number of non-terminals */
    pttHdrTransCnt,

    /* zero if child array was NULL, or
       the number of indicies in "idx" array
       beyond the header values */
    pttHdrIdxCnt,

    /* the end of "idx" if cnt is zero */
    pttHdrNullEnd,

    /* the number of sequence entries in "idx",
       where an entry is either a single index
       or an index range, as indicated by "child_seq_type" */
    pttHdrSeqLen = pttHdrNullEnd,

    /* the number of entries in "child" array */
    pttHdrChildCnt,

    /* index of the last header */
    pttLastHdr = pttHdrChildCnt,

    /* index of the first entry in "idx" */
    pttFirstIdx
};

typedef struct P_TTrans P_TTrans;
struct P_TTrans
{
    /* the variant selected for idx depends upon the recorded
       character set size, and is typically 8 bit.

       idx [ pttHdrIdx ]      => transition index from parent,
                                 i.e. character code
       idx [ pttHdrDepth ]    => node depth
       idx [ pttHdrTransCnt ] => transition count
       idx [ pttIdxCnt ]      => index count, meaning the
                                 dimension of "idx".

       if idx . variant [ pttIdxCnt ] is zero, then the P_TTrans
       child transition information is skipped, i.e. there is nothing
       corresponding to the "child" array in a TTrans.

       idx [ pttHdrSeqLen ]   => sequence length

       if the child array exists, the index sequence starts
       at offset pttFirstIdx into idx, due to the header information. */

    union
    {
        uint8_t v8 [ 4 ];
        uint16_t v16 [ 2 ];
        uint32_t v32 [ 1 ];

    } idx;

    /* from this point forward, the structure members have calculated offsets */
#if 0

    /* transition array sequence type bits
       indexed as: bits [ i / 8 ] & ( 1 << ( i % 8 ) )

       a "0" indicates a simple { idx, trans } pair, while
       a "1" indicates an index range { lower, upper } and
       a trans [] corresponding to the range. the index range
       is fully closed, i.e. upper is inclusive. */

    uint8_t child_seq_type [ ( idx [ pttHdrSeqLen ] + 7 ) / 8 ];

    /* alignment padding - when finding the location of the
       "child" array ( if it exists ), it is forced to be aligned
       to the size of its variant */

    if ( offsetof align1 % sizeof child [ 0 ] != 0 )
        uint8_t align1 [ sizeof child [ 0 ] - offsetof align1 % sizeof child [ 0 ] ];

    /* if keys are embedded within tree, then record a backtrace
       link from this P_TTrans to its parent */
    if ( tt -> ext_keys == false )
    {
        union
        {
            uint8_t v8;
            uint16_t v16;
            uint32_t v32;

        } dad;
    }

    /* transition ids. the variant selected depends upon
       the number of transition nodes in the tree */

    if ( idx [ pttHdrIdxCnt ] != 0 )
    {
        union
        {
            uint8_t v8 [ idx [ pttHdrChildCnt ] ];
            uint16_t v16 [ idx [ pttHdrChildCnt ] ];
            uint32_t v32 [ idx [ pttHdrChildCnt ] ];
            
        } child;
    }
    
    /* alignment padding */
    if ( offsetof align2 % sizeof uint32_t != 0 )
    {
        /* true or false as to whether to expect vals PBSTree */
        uint8_t has_vals;

        /* padding */
        uint8_t align2 [ sizeof uint32_t - 1 - offsetof align2 % sizeof uint32_t ];
    }

    /* a persisted tree */
    if ( offsetof align2 % sizeof uint32_t == 0 || has_vals != false )
        PBSTree vals;

    /* final alignment */
    if ( offsetof align3 % 4 != 0 )
        uint8_t align3 [ 4 - offsetof align3 % 4 ];
#endif
};

typedef struct PTTrans PTTrans;
struct PTTrans
{
    const PTTrans *back;

    union
    {
        const uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
    } idx;

    const uint8_t *child_seq_type;

    union
    {
        const uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
    } dad;

    union
    {
        const uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
    } child;

    PBSTree _val;
    const PBSTree *vals;

    uint32_t tid;
#if ! RECORD_HDR_IDX
    uint32_t tidx;
#endif
#if ! RECORD_HDR_DEPTH
    uint32_t depth;
#endif
    uint32_t tcnt;
    uint32_t icnt;
    uint32_t slen;
    int32_t refcount;
};

/* PTTransGetIdx
 */
#define PTTransGetIdx( self, tt, zidx ) \
    ( * ( tt ) -> get_idx ) ( self, zidx )

/* PTTransGetIdxAddr
 */
#define PTTransGetIdxAddr( self, tt, zidx ) \
    ( * ( tt ) -> get_idx_addr ) ( self, zidx )

/* PTTransGetDad
 */
#define PTTransGetDad( self, tt ) \
    ( * ( tt ) -> get_dad ) ( self )

/* PTTransAlignDadAddr
 */
#define PTTransAlignDadAddr( self, tt ) \
    ( * ( tt ) -> align_dad_addr ) ( self, ( tt ) -> backtrace )

/* PTTransGetChild
 */
#define PTTransGetChild( self, tt, zidx ) \
    ( * ( tt ) -> get_child ) ( self, zidx )

/* PTTransGetChildAddr
 */
#define PTTransGetChildAddr( self, tt, zidx ) \
    ( * ( tt ) -> get_child_addr ) ( self, zidx )

/* PTTransGetTransCharCode
 *  get char code of parent to self transition
 */
#if RECORD_HDR_IDX
#define PTTransGetTransCharCode( self, tt ) \
    PTTransGetIdx ( self, tt, pttHdrIdx )
#else
#define PTTransGetTransCharCode( self, tt ) \
    ( self ) -> tidx
#endif

/* PTTransGetDepth
 *  get transition depth
 */
#if RECORD_HDR_DEPTH
#define PTTransGetDepth( self, tt ) \
    PTTransGetIdx ( self, tt, pttHdrDepth )
#else
#define PTTransGetDepth( self, tt ) \
    ( self ) -> depth
#endif

/* PTTransForEach
 */
void PTTransForEach ( const PTTrans *self, const PTrie *tt,
    void ( * f ) ( const PTTrans *trans, const PTrie *tt, void *data ), void * data );

/* PTTransDoUntil
 */
bool PTTransDoUntil ( const PTTrans *self, const PTrie *tt,
    bool ( * f ) ( const PTTrans *trans, const PTrie *tt, void *data ), void * data );

/* PTTransWhack
 */
void PTTransWhack ( PTTrans *trans );

/* PTTransDump
 *  takes a FILE* as argument
 *
 *  "tt" [ IN ] - the owning PTrie
 *
 *  "stdio_file" [ IN ] - an output FILE* opaquely typed to avoid
 *  requiring an include of stdio.h
 *
 *  "indent_level" [ IN ] - the indent tabbing amount for output
 *  the implementation uses a tab setting of 2 spaces
 */
void PTTransDump ( const PTTrans *self , const PTrie *tt,
    void *stdio_file, uint32_t indent_level,
    void ( CC * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );


/*--------------------------------------------------------------------------
 * P_Trie
 * PTrie
 *  persisted TTree
 */
typedef struct P_Trie P_Trie;
struct P_Trie
{
    /* the number of transition nodes */
    uint32_t num_trans;

    /* the number of value nodes */
    uint32_t num_nodes;

    /* size of all P_TTrans data */
    uint32_t data_size;

    /* storage characteristics */
#if 0
    /* keys are like this, built always as little-endian, first fields lowest */
    uint16_t ext_keys  : 1;
    uint16_t backtrace : 1;
    uint16_t id_coding : 3;
#if EXTENDED_PTRIE
    uint16_t unused    : 3;
#else
    uint16_t unused    : 11;
#endif
#elif EXTENDED_PTRIE
    uint8_t keys;
#else
    uint16_t keys;
#endif  
#if EXTENDED_PTRIE
    uint8_t ext_data_size;
#endif

    /* character set in indexed order
       N.B. a character value of 0 means
       unmapped, and is unlikely but possible */
    uint16_t width;
    uint32_t rmap [ 1 ];

#if 0
    /* P_TTrans index - consists of 4-byte
       offsets relative to "data" member,
       i.e. an index of 2 means 8 byte offset */
    union
    {
        uint8_t v8 [ num_trans ];
        uint16_t v16 [ num_trans ];
        uint32_t v32 [ num_trans ];

    } trans_idx;

    /* an alignment to keep "data" on 4-byte boundary */
    if ( ( offsetof align1 & 3 ) != 0 )
        uint8_t align1 [ 4 - ( offsetof align1 & 3 ) ];

    /* P_TTrans objects */
    uint32_t data [ ( data_size + 3 ) / 4 ];

#endif
};


/* P_TrieExtKeys
 */
#define P_TrieExtKeys( keys ) \
    ( ( keys ) & 1 )

/* P_TrieBacktrace
 */
#define P_TrieBacktrace( keys ) \
    ( ( keys ) & 2 )

/* P_TrieIdCoding
 */
#define P_TrieIdCoding( keys ) \
    ( ( keys >> 2 ) & 7 )

/* P_TrieSetExtKeys
 */
#define P_TrieSetExtKeys( keys, val ) \
    ( void ) ( ( val ) ? ( ( keys ) |= 1 ) : ( ( keys ) &= ~ 1 ) )

/* P_TrieSetBacktrace
 */
#define P_TrieSetBacktrace( keys, val ) \
    ( void ) ( ( val ) ? ( ( keys ) |= 2 ) : ( ( keys ) &= ~ 2 ) )

/* P_TrieSetIdCoding
 */
#define P_TrieSetIdCoding( keys, val ) \
    ( void ) ( ( keys ) = ( ( keys ) & ~ ( 7 << 2 ) ) | ( ( ( val ) & 7 ) << 2 ) )


struct PTrie
{
    /* virtual functions on PTTrans */
    uint32_t ( * get_idx ) ( const PTTrans*, uint32_t idx );
    const void* ( * get_idx_addr ) ( const PTTrans*, uint32_t idx );
    uint32_t ( * get_dad ) ( const PTTrans* );
    const void* ( * align_dad_addr ) ( PTTrans*, bool backtrace );
    uint32_t ( * get_child ) ( const PTTrans*, uint32_t idx );
    const void* ( * get_child_addr ) ( const PTTrans*, uint32_t tcnt );

    /* virtual functions on PTrie */
    size_t ( * get_node_off ) ( const PTrie*, uint32_t idx );
    uint32_t ( * encode_node_id ) ( const PTrie*,
        uint32_t tid, uint32_t btid );
    rc_t ( * decode_node_id ) ( const PTrie*, uint32_t id,
        uint32_t *tid, uint32_t *btid );
    uint32_t ( * decode_char_id ) ( const PTrie*, uint32_t id );

    /* P_TTrans index to character map */
    const uint32_t *_rmap;

    /* P_TTrans offsets */
    union
    {
        const uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
        const uint64_t *v64;
    } trans_off;

    /* P_TTrans PTrans objects */
    union
    {
        const uint32_t *v32;
        const uint64_t *v64;
    } data;

    /* the number of transition nodes */
    uint32_t num_trans;

    /* the number of value nodes */
    uint32_t num_nodes;

    /* size of all P_TTrans data */
    size_t data_size;

    /* storage characteristics */
    uint8_t ext_keys;
    uint8_t backtrace;
    uint8_t id_coding;
    uint8_t byteswap;

    /* width of character map & trans array */
    uint16_t width;

    /* range of UTF-32 input characters */
    uint32_t first_char;
    uint32_t last_char;

    /* index of an unmapped character */
    uint16_t unmapped_idx;

    /* character to index map */
    uint16_t map [ 1 ];
};



/* PTrieGetNodeOff
 */
#define PTrieGetNodeOff( self, zidx ) \
    ( * ( self ) -> get_node_off ) ( self, zidx )

/* PTrieEncodeNodeId
 */
#define PTrieEncodeNodeId( self, tid, btid ) \
    ( * ( self ) -> encode_node_id ) ( self, tid, btid )

/* PTrieDecodeNodeId
 */
#define PTrieDecodeNodeId( self, id, tid, btid ) \
    ( * ( self ) -> decode_node_id ) ( self, id, tid, btid )

/* PTrieDecodeCharId
 */
#define PTrieDecodeCharId( self, idx ) \
    ( * ( self ) -> decode_char_id ) ( self, idx )


/* PTrieInitNode
 *  takes a one-based index
 */
rc_t PTrieInitNode ( const PTrie *self, PTTrans *trans, uint32_t idx );

/* PTrieMakeNode
 *  takes a one-based index
 */
rc_t PTrieMakeNode ( const PTrie *self, PTTrans **transp, uint32_t idx );

/* PTrieDump
 *  takes a FILE* as argument
 *
 *  "name" [ IN, NULL OKAY ] - optional ttree name
 *
 *  "stdio_file" [ IN ] - an output FILE* opaquely typed to avoid
 *  requiring an include of stdio.h
 *
 *  "indent_level" [ IN ] - the indent tabbing amount for output
 *  the implementation uses a tab setting of 2 spaces
 *
 *  "dump_node" [ IN, NULL OKAY ] and "data" [ IN ] - an optional
 *  node dumping function and associated data
 */
void PTrieDump ( const PTrie *self, const char *name,
    void *stdio_file, uint32_t indent_level,
    void ( * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );

/* PTrieDumpTrans
 *  takes a FILE* as argument
 *
 *  the indent level is used for hierarchy.
 *  2 spaces are emitted at the left margin for each level.
 */
void PTrieDumpTrans ( const PTrie *self , uint32_t id,
    void *stdio_file, uint32_t indent_level,
    void ( * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );


#ifdef __cplusplus
}
#endif

#endif /* _h_pbstree_priv_ */
