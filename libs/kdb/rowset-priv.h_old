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

#ifndef _h_kdb_rowset_priv_
#define _h_kdb_rowset_priv_

#include <klib/container.h>

#ifdef __cplusplus
extern "C" {
#endif

#if _DEBUGGING
#define CHECK_NODE_MARKS 1
#endif

#if CHECK_NODE_MARKS
#define NODE_MARK 55
#define LEAF_MARK 99
#endif

#define LEAF_DEPTH 6 // at which depth leaves are located
#define LEAF_DATA_SZ_BT 8192

// the maximum value that leaf can represent
//  (leaf data only stores lowest bytes of row_id)
#define ROW_LEAF_MAX 0xFFFFLL

/*--------------------------------------------------------------------------
 * KRowSet
 */
typedef enum KRowSetTreeLeafType
{
    LeafType_Bitmap,
    LeafType_ArrayRanges
} KRowSetTreeLeafType;

// Holds bites of row_id followed by current byte (radix trie compression)
typedef struct KRowSetTreeNodeTransition
{
    int8_t size;
    uint8_t data[5];
} KRowSetTreeNodeTransition;

typedef struct KRowSetTreeNode
{
    void* children[256];
    KRowSetTreeNodeTransition transitions[256];

#if CHECK_NODE_MARKS
    uint8_t node_mark;
#endif
} KRowSetTreeNode;

typedef struct KRowSetTreeLeafHeader
{
    DLNode dad;
    int64_t leaf_id; // this is basically a row_id >> 16
    uint64_t leaf_rows; // number of rows set in this leaf
    uint8_t type; // types are defined in KRowSetTreeLeafType

#if CHECK_NODE_MARKS
    uint8_t leaf_mark;
#endif
} KRowSetTreeLeafHeader;


typedef struct KRowSetTreeLeaf
{
    KRowSetTreeLeafHeader header;
    union u
    {
        uint8_t bitmap[LEAF_DATA_SZ_BT];
        struct KRowSetTreeLeafArrayRanges {
            struct KRowSetTreeLeafRowRange {
                uint16_t start;
                uint16_t end;
            } ranges[8];
            uint8_t len;
        } array_ranges;
    } data;
} KRowSetTreeLeaf;

const KRowSetTreeLeaf * KRowSetTreeGetFirstLeaf ( const KRowSet * self );
const KRowSetTreeLeaf * KRowSetTreeGetLastLeaf ( const KRowSet * self );
const KRowSetTreeLeaf * KRowSetTreeLeafGetNext ( const KRowSetTreeLeaf * leaf );
const KRowSetTreeLeaf * KRowSetTreeLeafGetPrev ( const KRowSetTreeLeaf * leaf );

#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_priv_ */
