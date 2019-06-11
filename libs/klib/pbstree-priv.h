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
typedef struct P_BSTree_v1 P_BSTree_v1;
struct P_BSTree_v1
{
    /* a node count - if zero, then the structure ends */
    pbst_count_t_v1 num_nodes;

    /* the total size of auxiliary data */
    pbst_count_t_v1 data_size;

    /* offsets relative to & data_idx [ num_nodes ] */
    union
    {
        uint8_t   v8 [ 4 ];
        uint16_t v16 [ 2 ];
        uint32_t v32 [ 1 ];
    } data_idx;

#if 0
    uint8_t data [ data_size ];
#endif
};

typedef struct P_BSTree_v2 P_BSTree_v2;
struct P_BSTree_v2
{
    /* a node count - if zero, then the structure ends */
    pbst_count_t_v2 num_nodes;

    /* the total size of auxiliary data */
    pbst_count_t_v2 data_size;

    /* offsets relative to & data_idx [ num_nodes ] */
    union
    {
        uint8_t   v8 [ 8 ];
        uint16_t v16 [ 4 ];
        uint32_t v32 [ 2 ];
        uint64_t v64 [ 1 ];
    } data_idx;

#if 0
    uint8_t data [ data_size ];
#endif
};

/*--------------------------------------------------------------------------
 * PBSTree_vt
 *  versioned message dispatch table
 */
#ifndef PBSTREE_V1_IMPL
#define PBSTREE_V1_IMPL PBSTree_v1
#endif

typedef struct PBSTree_v1_vt_v1 PBSTree_v1_vt_v1;
struct PBSTree_v1_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    void ( CC * destroy ) ( PBSTREE_V1_IMPL *self );
    pbst_count_t_v1 ( CC * count ) ( const PBSTREE_V1_IMPL *self );
    pbst_count_t_v1 ( CC * depth ) ( const PBSTREE_V1_IMPL *self );
    size_t ( CC * size ) ( const PBSTREE_V1_IMPL *self );
    rc_t ( CC * get_node_data ) ( const PBSTREE_V1_IMPL *self,
        const void **addr, size_t *size, PBSTNodeId_v1 id );
    PBSTNodeId_v1 ( CC * find ) (  const PBSTREE_V1_IMPL *self, PBSTNode_v1 *rtn,
        const void *item, int ( CC * cmp ) ( const void*, const PBSTNode_v1* , void* ), void *data );
    void ( CC * for_each ) ( const PBSTREE_V1_IMPL *self, bool reverse,
        void ( CC * f ) ( PBSTNode_v1*, void* ), void *data );
    bool ( CC * do_until ) ( const PBSTREE_V1_IMPL *self, bool reverse,
        bool ( CC * f ) ( PBSTNode_v1*, void* ), void *data );
    /* end minor version == 0 */
};

#ifndef PBSTREE_V2_IMPL
#define PBSTREE_V2_IMPL PBSTree_v2
#endif

typedef struct PBSTree_v2_vt_v1 PBSTree_v2_vt_v1;
struct PBSTree_v2_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    void ( CC * destroy ) ( PBSTREE_V2_IMPL *self );
    pbst_count_t_v2 ( CC * count ) ( const PBSTREE_V2_IMPL *self );
    pbst_count_t_v2 ( CC * depth ) ( const PBSTREE_V2_IMPL *self );
    size_t ( CC * size ) ( const PBSTREE_V2_IMPL *self );
    rc_t ( CC * get_node_data ) ( const PBSTREE_V2_IMPL *self,
        const void **addr, size_t *size, PBSTNodeId_v2 id );
    PBSTNodeId_v2 ( CC * find ) (  const PBSTREE_V2_IMPL *self, PBSTNode_v2 *rtn,
        const void *item, int ( CC * cmp ) ( const void*, const PBSTNode_v2* , void* ), void *data );
    void ( CC * for_each ) ( const PBSTREE_V2_IMPL *self, bool reverse,
        void ( CC * f ) ( PBSTNode_v2*, void* ), void *data );
    bool ( CC * do_until ) ( const PBSTREE_V2_IMPL *self, bool reverse,
        bool ( CC * f ) ( PBSTNode_v2*, void* ), void *data );
    /* end minor version == 0 */
};

/* latest major version */
#define PBSTREE_ITF_LATEST 1

typedef union PBSTree_v1_vt PBSTree_v1_vt;
union PBSTree_v1_vt
{
    PBSTree_v1_vt_v1 v1;
};

typedef union PBSTree_v2_vt PBSTree_v2_vt;
union PBSTree_v2_vt
{
    PBSTree_v2_vt_v1 v1;
};

/*--------------------------------------------------------------------------
 * PBSTree
 *  handle to persisted binary search tree
 */
struct PBSTree_v1
{
    const PBSTree_v1_vt *vt;
    const P_BSTree_v1 *pt;
};

struct PBSTree_v2
{
    const PBSTree_v2_vt *vt;
    const P_BSTree_v2 *pt;
};


/* Make
 *  swapped and native versions
 *  take a pre-allocated but uninitialized PBSTree structure
 */
rc_t PBSTreeMakeNative_v1 ( PBSTree_v1 *pt, const void *addr, size_t size );
rc_t PBSTreeMakeNative_v2 ( PBSTree_v2 *pt, const void *addr, size_t size );
rc_t PBSTreeMakeSwapped_v1 ( PBSTree_v1 *pt, const void *addr, size_t size );
rc_t PBSTreeMakeSwapped_v2 ( PBSTree_v2 *pt, const void *addr, size_t size );


/* Init
 *  initialize the parent class
 */
rc_t PBSTreeInit_v1 ( PBSTree_v1 *self, const PBSTree_v1_vt *vt, const P_BSTree_v1 *pt );
rc_t PBSTreeInit_v2 ( PBSTree_v2 *self, const PBSTree_v2_vt *vt, const P_BSTree_v2 *pt );


/* PBSTreeGetNodeData
 *  finds node data boundaries
 */
rc_t PBSTreeGetNodeData_v1 ( const PBSTree_v1 *self,
    const void **addr, size_t *size, PBSTNodeId_v1 id );
rc_t PBSTreeGetNodeData_v2 ( const PBSTree_v2 *self,
    const void **addr, size_t *size, PBSTNodeId_v2 id );

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
void PBSTreeDump_v1 ( const PBSTree_v1 *self, const char *name,
    void *stdio_file, uint32_t indent_level,
    void ( CC * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );
void PBSTreeDump_v2 ( const PBSTree_v2 *self, const char *name,
    void *stdio_file, uint32_t indent_level,
    void ( CC * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );

/* GetIndentString
 *  returns a string of spaces for indentation
 *
 *  TBD - this is likely no faster than just using normal formatting
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

typedef struct P_TTrans_v1 P_TTrans_v1;
struct P_TTrans_v1
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
        uint8_t   v8 [ 4 ];
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
            uint8_t   v8;
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
            uint8_t   v8 [ idx [ pttHdrChildCnt ] ];
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
        PBSTree_v1 vals;

    /* final alignment */
    if ( offsetof align3 % 4 != 0 )
        uint8_t align3 [ 4 - offsetof align3 % 4 ];
#endif
};

typedef struct P_TTrans_v2 P_TTrans_v2;
struct P_TTrans_v2
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
        uint8_t   v8 [ 8 ];
        uint16_t v16 [ 4 ];
        uint32_t v32 [ 2 ];
        uint64_t v64 [ 1 ];

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
            uint8_t   v8;
            uint16_t v16;
            uint32_t v32;
            uint64_t v64;

        } dad;
    }

    /* transition ids. the variant selected depends upon
       the number of transition nodes in the tree */

    if ( idx [ pttHdrIdxCnt ] != 0 )
    {
        union
        {
            uint8_t   v8 [ idx [ pttHdrChildCnt ] ];
            uint16_t v16 [ idx [ pttHdrChildCnt ] ];
            uint32_t v32 [ idx [ pttHdrChildCnt ] ];
            uint64_t v64 [ idx [ pttHdrChildCnt ] ];
            
        } child;
    }
    
    /* alignment padding */
    if ( offsetof align2 % sizeof uint64_t != 0 )
    {
        /* true or false as to whether to expect vals PBSTree */
        uint8_t has_vals;

        /* padding */
        uint8_t align2 [ sizeof uint64_t - 1 - offsetof align2 % sizeof uint64_t ];
    }

    /* a persisted tree */
    if ( offsetof align2 % sizeof uint64_t == 0 || has_vals != false )
        PBSTree_v2 vals;

    /* final alignment */
    if ( offsetof align3 % 8 != 0 )
        uint8_t align3 [ 8 - offsetof align3 % 8 ];
#endif
};

typedef struct PTTrans_v1 PTTrans_v1;
struct PTTrans_v1
{
    const PTTrans_v1 * back;

    union
    {
        const  uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
    } idx;

    const uint8_t *child_seq_type;

    union
    {
        const  uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
    } dad;

    union
    {
        const  uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
    } child;

    PBSTree_v1 _val;
    const PBSTree_v1 *vals;

    pbst_count_t_v1 tid;
#if ! RECORD_HDR_IDX
    pbst_count_t_v1 tidx;
#endif
#if ! RECORD_HDR_DEPTH
    pbst_count_t_v1 depth;
#endif
    pbst_count_t_v1 tcnt;
    pbst_count_t_v1 icnt;
    uint32_t slen;
    int32_t refcount;
};

typedef struct PTTrans_v2 PTTrans_v2;
struct PTTrans_v2
{
    const PTTrans_v2 * back;

    union
    {
        const  uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
        const uint64_t *v64;
    } idx;

    const uint8_t *child_seq_type;

    union
    {
        const  uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
        const uint64_t *v64;
    } dad;

    union
    {
        const  uint8_t *v8;
        const uint16_t *v16;
        const uint32_t *v32;
        const uint64_t *v64;
    } child;

    PBSTree_v2 _val;
    const PBSTree_v2 *vals;

    pbst_count_t_v2 tid;
#if ! RECORD_HDR_IDX
    pbst_count_t_v2 tidx;
#endif
#if ! RECORD_HDR_DEPTH
    pbst_count_t_v2 depth;
#endif
    pbst_count_t_v2 tcnt;
    pbst_count_t_v2 icnt;
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
void PTTransForEach_v1 ( const PTTrans_v1 *self, const PTrie_v1 *tt,
    void ( * f ) ( const PTTrans_v1 *trans, const PTrie_v1 *tt, void *data ), void * data );
void PTTransForEach_v2 ( const PTTrans_v2 *self, const PTrie_v2 *tt,
    void ( * f ) ( const PTTrans_v2 *trans, const PTrie_v2 *tt, void *data ), void * data );

/* PTTransDoUntil
 */
bool PTTransDoUntil_v1 ( const PTTrans_v1 *self, const PTrie_v1 *tt,
    bool ( * f ) ( const PTTrans_v1 *trans, const PTrie_v1 *tt, void *data ), void * data );
bool PTTransDoUntil_v2 ( const PTTrans_v2 *self, const PTrie_v2 *tt,
    bool ( * f ) ( const PTTrans_v2 *trans, const PTrie_v2 *tt, void *data ), void * data );

/* PTTransWhack
 */
void PTTransWhack_v1 ( PTTrans_v1 *trans );
void PTTransWhack_v2 ( PTTrans_v2 *trans );

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
void PTTransDump_v1 ( const PTTrans_v1 *self , const PTrie_v1 *tt,
    void *stdio_file, uint32_t indent_level,
    void ( CC * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );
void PTTransDump_v2 ( const PTTrans_v2 *self , const PTrie_v2 *tt,
    void *stdio_file, uint32_t indent_level,
    void ( CC * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );


/*--------------------------------------------------------------------------
 * P_Trie
 * PTrie
 *  persisted TTree
 */
typedef struct P_Trie_v1 P_Trie_v1;
struct P_Trie_v1
{
    /* the number of transition nodes */
    pbst_count_t_v1 num_trans;

    /* the number of value nodes */
    pbst_count_t_v1 num_nodes;

    /* size of all P_TTrans data */
    pbst_count_t_v1 data_size;

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
        uint8_t   v8 [ num_trans ];
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

typedef struct P_Trie_v2 P_Trie_v2;
struct P_Trie_v2
{
    /* the number of transition nodes */
    pbst_count_t_v2 num_trans;

    /* the number of value nodes */
    pbst_count_t_v2 num_nodes;

    /* size of all P_TTrans data */
    pbst_count_t_v2 data_size;

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
        uint8_t   v8 [ num_trans ];
        uint16_t v16 [ num_trans ];
        uint32_t v32 [ num_trans ];
        uint64_t v64 [ num_trans ];

    } trans_idx;

    /* an alignment to keep "data" on 8-byte boundary */
    if ( ( offsetof align1 & 7 ) != 0 )
        uint8_t align1 [ 8 - ( offsetof align1 & 7 ) ];

    /* P_TTrans objects */
    uint64_t data [ ( data_size + 7 ) / 8 ];

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


struct PTrie_v1
{
    /* virtual functions on PTTrans */
    pbst_count_t_v1 ( * get_idx ) ( const PTTrans_v1*, pbst_count_t_v1 idx );
    const void* ( * get_idx_addr ) ( const PTTrans_v1*, pbst_count_t_v1 idx );
    pbst_count_t_v1 ( * get_dad ) ( const PTTrans_v1* );
    const void* ( * align_dad_addr ) ( PTTrans_v1*, bool backtrace );
    pbst_count_t_v1 ( * get_child ) ( const PTTrans_v1*, pbst_count_t_v1 idx );
    const void* ( * get_child_addr ) ( const PTTrans_v1*, pbst_count_t_v1 tcnt );

    /* virtual functions on PTrie */
    size_t ( * get_node_off ) ( const PTrie_v1*, pbst_count_t_v1 idx );
    pbst_count_t_v1 ( * encode_node_id ) ( const PTrie_v1*,
        pbst_count_t_v1 tid, pbst_count_t_v1 btid );
    rc_t ( * decode_node_id ) ( const PTrie_v1*, pbst_count_t_v1 id,
        pbst_count_t_v1 *tid, pbst_count_t_v1 *btid );

    uint32_t ( * decode_char_id ) ( const PTrie_v1*, uint32_t id );

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
    pbst_count_t_v1 num_trans;

    /* the number of value nodes */
    pbst_count_t_v1 num_nodes;

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

struct PTrie_v2
{
    /* virtual functions on PTTrans */
    pbst_count_t_v2 ( * get_idx ) ( const PTTrans_v2*, pbst_count_t_v2 idx );
    const void* ( * get_idx_addr ) ( const PTTrans_v2*, pbst_count_t_v2 idx );
    pbst_count_t_v2 ( * get_dad ) ( const PTTrans_v2* );
    const void* ( * align_dad_addr ) ( PTTrans_v2*, bool backtrace );
    pbst_count_t_v2 ( * get_child ) ( const PTTrans_v2*, pbst_count_t_v2 idx );
    const void* ( * get_child_addr ) ( const PTTrans_v2*, pbst_count_t_v2 tcnt );

    /* virtual functions on PTrie */
    size_t ( * get_node_off ) ( const PTrie_v2*, pbst_count_t_v2 idx );
    pbst_count_t_v2 ( * encode_node_id ) ( const PTrie_v2*,
        pbst_count_t_v2 tid, pbst_count_t_v2 btid );
    rc_t ( * decode_node_id ) ( const PTrie_v2*, pbst_count_t_v2 id,
        pbst_count_t_v2 *tid, pbst_count_t_v2 *btid );

    uint32_t ( * decode_char_id ) ( const PTrie_v2*, uint32_t id );

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
    pbst_count_t_v2 num_trans;

    /* the number of value nodes */
    pbst_count_t_v2 num_nodes;

    /* size of all P_TTrans data */
    size_t data_size;

    /* storage characteristics */
    uint8_t ext_keys;
    uint8_t backtrace;
    uint8_t id_coding;
    uint8_t byteswap;

    /* range of UTF-32 input characters */
    uint32_t first_char;
    uint32_t last_char;

    /* width of character map & trans array */
    uint16_t width;

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
rc_t PTrieInitNode_v1 ( const PTrie_v1 *self, PTTrans_v1 *trans, pbst_count_t_v1 idx );
rc_t PTrieInitNode_v2 ( const PTrie_v2 *self, PTTrans_v2 *trans, pbst_count_t_v2 idx );

/* PTrieMakeNode
 *  takes a one-based index
 */
rc_t PTrieMakeNode_v1 ( const PTrie_v1 *self, PTTrans_v1 **transp, pbst_count_t_v1 idx );
rc_t PTrieMakeNode_v2 ( const PTrie_v2 *self, PTTrans_v2 **transp, pbst_count_t_v2 idx );

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
void PTrieDump_v1 ( const PTrie_v1 *self, const char *name,
    void *stdio_file, uint32_t indent_level,
    void ( * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );
void PTrieDump_v2 ( const PTrie_v2 *self, const char *name,
    void *stdio_file, uint32_t indent_level,
    void ( * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );

/* PTrieDumpTrans
 *  takes a FILE* as argument
 *
 *  the indent level is used for hierarchy.
 *  2 spaces are emitted at the left margin for each level.
 */
void PTrieDumpTrans_v1 ( const PTrie_v1 *self , pbst_count_t_v1 id,
    void *stdio_file, uint32_t indent_level,
    void ( * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );
void PTrieDumpTrans_v2 ( const PTrie_v2 *self , pbst_count_t_v2 id,
    void *stdio_file, uint32_t indent_level,
    void ( * dump_node ) ( const void *addr, size_t size, void *stdio_file,
        uint32_t indent_level, void *data ), void *data );

/*--------------------------------------------------------------------------
 * remapped names
 */

#define P_BSTree NAME_VERS ( P_BSTree, PBSTREE_VERS )
#define PBSTree_vt NAME_VERS ( PBSTree, t )
#define PBSTree_vt_v1 NAME_VERS ( PBSTree_vt, 1 )
#define PBSTreeMakeNative NAME_VERS ( PBSTreeMakeNative, PBSTREE_VERS )
#define PBSTreeMakeSwapped NAME_VERS ( PBSTreeMakeSwapped, PBSTREE_VERS )
#define PBSTreeInit NAME_VERS ( PBSTreeInit, PBSTREE_VERS )
#define PBSTreeGetNodeData NAME_VERS ( PBSTreeGetNodeData, PBSTREE_VERS )
#define PBSTreeDump NAME_VERS ( PBSTreeDump, PBSTREE_VERS )

#define P_TTrans NAME_VERS ( P_TTrans, PTRIE_VERS )
#define PTTrans NAME_VERS ( PTTrans, PTRIE_VERS )
#define PTTransForEach NAME_VERS ( PTTransForEach, PTRIE_VERS )
#define PTTransDoUntil NAME_VERS ( PTTransDoUntil, PTRIE_VERS )
#define PTTransWhack NAME_VERS ( PTTransWhack, PTRIE_VERS )
#define PTTransDump NAME_VERS ( PTTransDump, PTRIE_VERS )

#define P_Trie NAME_VERS ( P_Trie, PTRIE_VERS )
#define PTrieInitNode NAME_VERS ( PTrieInitNode, PTRIE_VERS )
#define PTrieMakeNode NAME_VERS ( PTrieMakeNode, PTRIE_VERS )
#define PTrieDump NAME_VERS ( PTrieDump, PTRIE_VERS )
#define PTrieDumpTrans NAME_VERS ( PTrieDumpTrans, PTRIE_VERS )

#ifdef __cplusplus
}
#endif

#endif /* _h_pbstree_priv_ */
