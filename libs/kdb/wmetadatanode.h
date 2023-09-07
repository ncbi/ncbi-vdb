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

#include <klib/pbstree.h>
#include <klib/container.h>

struct KMetadata;
struct KMDataNode;

#ifdef __cplusplus
extern "C" {
#endif

#define NODE_SIZE_LIMIT ( 25 * 1024 * 1024 )
#define NODE_CHILD_LIMIT ( 100 * 1024 )

typedef struct KMDataNodeInflateData KMDataNodeInflateData;
struct KMDataNodeInflateData
{
    KMetadata *meta;
    KMDataNode *par;
    BSTree *bst;
    size_t node_size_limit;
    uint32_t node_child_limit;
    rc_t rc;
    bool byteswap;
};

/*--------------------------------------------------------------------------
 * KMAttrNode
 */
typedef struct KMAttrNode KMAttrNode;
struct KMAttrNode
{
    BSTNode n;
    void *value;
    size_t vsize;
    char name [ 1 ];
};

rc_t KMAttrNodeMake ( KMAttrNode **np, const char *name, const void *value, size_t vsize );
void CC KMAttrNodeWhack ( BSTNode *n, void *data );
int64_t CC KMAttrNodeCmp ( const void *item, const BSTNode *n );
int64_t CC KMAttrNodeSort ( const BSTNode *item, const BSTNode *n );
rc_t KMAttrNodeRename ( const KMAttrNode *self, KMAttrNode **renamed, const char *name );

/*--------------------------------------------------------------------------
 * KMDataNode
 *  a node with an optional value,
 *  optional attributes, and optional children
 *
 *  nodes are identified by path, relative to a starting node,
 *  where "/" serves as a path separator.
 */
struct KMDataNode
{
    BSTNode n;
    KMDataNode *par;
    KMetadata *meta;
    void *value;
    size_t vsize;
    BSTree attr;
    BSTree child;
    KRefcount refcount;
    uint8_t read_only;
    char name [ 1 ];
};

rc_t KMDataNodeMake ( KMDataNode *self, KMDataNode **np, char *name );
void CC KMDataNodeWhack ( BSTNode *n, void *data );
rc_t KMDataNodeFind ( const KMDataNode *cself, KMDataNode **np, char **path );
bool CC KMDataNodeInflate_v1 ( PBSTNode *n, void *data );
bool CC KMDataNodeInflate ( PBSTNode *n, void *data );

#ifdef __cplusplus
}
#endif

