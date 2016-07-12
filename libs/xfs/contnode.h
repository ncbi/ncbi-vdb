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

#ifndef _h_contnode_
#define _h_contnode_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   This file contains general container node
  ))))
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* ContNode - implemented in contnode.c                              */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSNodeContainer;
struct XFSContNode;

typedef rc_t ( CC * _tContNodeDisposer ) ( struct XFSContNode * self );

struct XFSContNode {
    struct XFSNode node;

    _tContNodeDisposer disposer;

    struct XFSNodeContainer * container;

    const char * security;
    uint32_t flavor;
};

XFS_EXTERN rc_t CC XFSContNodeInit (
                            struct XFSNode * self,
                            const char * Name,      /* Could be NULL */
                            const char * Perm,
                            uint32_t Flavor,
                            _tContNodeDisposer disposer /* NULL is OK */
                            );

XFS_EXTERN rc_t CC XFSContNodeMake (
                            const char * Name,
                            const char * Perm,      /* Could be NULL */
                            struct XFSNode ** Node
                            );

XFS_EXTERN rc_t CC XFSContNodeMakeWithFlavor (
                            const char * Name,
                            const char * Perm,      /* Could be NULL */
                            uint32_t Flavor,
                            struct XFSNode ** Node
                            );

XFS_EXTERN rc_t CC XFSContNodeAddChild (
                            struct XFSNode * self,
                            const struct XFSNode * Child
                            );

XFS_EXTERN rc_t CC XFSContNodeDelChild (
                            struct XFSNode * self,
                            const char * ChildName
                            );

XFS_EXTERN rc_t CC XFSContNodeClear ( struct XFSNode * self );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _h_contnode_ */
