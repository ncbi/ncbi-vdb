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

#ifndef _xfs_node_h_
#define _xfs_node_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics.
 ++   That file contains interfaces for XFS Tree
 ||
((*/

/*)))
 ///   XFSModel and other forwards.
(((*/
struct XFSTree;
struct XFSNode;
struct XFSModel;
struct XFSPath;
struct KRefcount;



/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   Node itself
 ((*/
union XFSNode_vt;

struct XFSDirEditor;
struct XFSFileEditor;
struct XFSAttrEditor;

struct XFSNode {
    const union XFSNode_vt * vt;

    KRefcount refcount;

    char * Name;
};

struct XFSNode_vt_v1 {
        /* version, as is */
    uint32_t maj;
    uint32_t min;

        /* needed for authomatic node recognition */
    uint32_t ( CC * flavor ) ( const struct XFSNode * self );

        /* needed for authomatic node dipsosal */
    rc_t ( CC * dispose ) ( const struct XFSNode * self );

        /* will return node for path */
    rc_t ( CC * findnode ) (
                    const struct XFSNode * self, 
                    const struct XFSPath * Path,
                    uint32_t PathIndex,
                    const struct XFSNode ** Node
                    );

        /* returns interface to node as a directory, if exists */
    rc_t ( CC * dir ) (
                    const struct XFSNode * self,
                    const struct XFSDirEditor ** Dir
                    );

        /* returns interface to node as a file, if exists */
    rc_t ( CC * file ) (
                    const struct XFSNode * self,
                    const struct XFSFileEditor ** File
                    );

        /* returns interface which allows access node attributes */
    rc_t ( CC * attr ) (
                    const struct XFSNode * self,
                    const struct XFSAttrEditor ** Attr
                    );

        /* quite stupid method which will print some info about node */
    rc_t ( CC * describe ) (
                    const struct XFSNode * self,
                    char * Buffer,
                    size_t BufferSize
                    );
};

union XFSNode_vt {
    struct XFSNode_vt_v1 v1;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
XFS_EXTERN rc_t CC XFSNodeInit (
                            const struct XFSNode * self,
                            const char * NodeName
                            );

XFS_EXTERN rc_t CC XFSNodeInitVT (
                            const struct XFSNode * self,
                            const char * NodeName,
                            const union XFSNode_vt * VT /*  Could be
                                                            NULL */
                            );

XFS_EXTERN rc_t CC XFSNodeMake (
                            const struct XFSModel * ModelNode,
                            const char * NodeName,
                            const char * NameAlias,   /* NULL is OK */
                            const struct XFSNode ** Node
                            );

XFS_EXTERN rc_t CC XFSNodeDispose ( const struct XFSNode * self );

XFS_EXTERN rc_t CC XFSNodeAddRef ( const struct XFSNode * self );

XFS_EXTERN rc_t CC XFSNodeRelease ( const struct XFSNode * self );

XFS_EXTERN rc_t CC XFSNodeFindNode (
                            const struct XFSNode * self, 
                            const struct XFSPath * Path,
                            uint32_t PathIndex,
                            const struct XFSNode ** Node
                            );

    /*  That method checks values and inits useful variables
     */
XFS_EXTERN rc_t CC XFSNodeFindNodeCheckInitStandard (
                            const struct XFSNode * self, 
                            const struct XFSPath * Path,
                            uint32_t PathIndex,
                            const struct XFSNode ** Node,
                            const char ** NodeName,
                            uint32_t * PathCount,
                            bool * IsLast
                            );

        /* returns interface to node as a directory, if exists */
XFS_EXTERN rc_t CC XFSNodeDirEditor (
                            const struct XFSNode * self,
                            const struct XFSDirEditor ** Dir
                            );

        /* returns interface to node as a file, if exists */
XFS_EXTERN rc_t CC XFSNodeFileEditor (
                            const struct XFSNode * self,
                            const struct XFSFileEditor ** File
                            );

        /* returns interface which allows access node attributes */
XFS_EXTERN rc_t CC XFSNodeAttrEditor (
                            const struct XFSNode * self,
                            const struct XFSAttrEditor ** Attr
                            );

        /* describes node content */
XFS_EXTERN rc_t CC XFSNodeDescribe (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

XFS_EXTERN rc_t CC XFSNodeDump ( const struct XFSNode * self );

XFS_EXTERN const char * CC XFSNodeName ( const struct XFSNode * self );

XFS_EXTERN uint32_t CC XFSNodeFlavor ( const struct XFSNode * self );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
    /*)))   Way to make special Bogus nodes
     (((*/
XFS_EXTERN rc_t CC XFSNodeNotFoundMake (
                            const char * NodeName,
                            const struct XFSNode ** Node
                            );

XFS_EXTERN rc_t CC XFSNodeBadPathMake (
                            const char * NodeName,
                            const struct XFSNode ** Node
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_node_h_ */
