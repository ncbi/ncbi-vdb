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

#pragma once

#include "metanode-base.h"

#include <klib/pbstree.h>

struct KWMetadata;

typedef struct KWMDataNode KWMDataNode;

#ifdef __cplusplus
extern "C" {
#endif

#define NODE_SIZE_LIMIT ( 25 * 1024 * 1024 )
#define NODE_CHILD_LIMIT ( 100 * 1024 )

typedef struct KWMDataNodeInflateData KWMDataNodeInflateData;
struct KWMDataNodeInflateData
{
    KWMetadata *meta;
    struct KWMDataNode *par;
    BSTree *bst;
    size_t node_size_limit;
    uint32_t node_child_limit;
    rc_t rc;
    bool byteswap;
};

/*--------------------------------------------------------------------------
 * KWMAttrNode
 */
typedef struct KWMAttrNode KWMAttrNode;
struct KWMAttrNode
{
    BSTNode n;
    void *value;
    size_t vsize;
    char name [ 1 ];
};

rc_t KWMAttrNodeMake ( KWMAttrNode **np, const char *name, const void *value, size_t vsize );
void CC KWMAttrNodeWhack ( BSTNode *n, void *data );
int64_t CC KWMAttrNodeCmp ( const void *item, const BSTNode *n );
int64_t CC KWMAttrNodeSort ( const BSTNode *item, const BSTNode *n );
rc_t KWMAttrNodeRename ( const KWMAttrNode *self, KWMAttrNode **renamed, const char *name );

/*--------------------------------------------------------------------------
 * KMDataNode
 *  a node with an optional value,
 *  optional attributes, and optional children
 *
 *  nodes are identified by path, relative to a starting node,
 *  where "/" serves as a path separator.
 */
struct KWMDataNode
{
    KMDataNode dad;

    KWMDataNode *par;
    KWMetadata *meta;
    void *value;
    size_t vsize;
    BSTree attr;
    BSTree child;
    uint8_t read_only;
    char name [ 1 ];
};

rc_t KWMDataNodeMakeRoot( KWMDataNode ** node, KWMetadata *meta );
rc_t KWMDataNodeFind ( const KWMDataNode *cself, KWMDataNode **np, char **path );
bool CC KWMDataNodeInflate_v1 ( PBSTNode *n, void *data );
bool CC KWMDataNodeInflate ( PBSTNode *n, void *data );

#ifdef __cplusplus
}
#endif

