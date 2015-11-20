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

#ifndef _ncon_h_
#define _ncon_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   This file contains unterface to different node containers
  ))))  not sure how many we will have
 ((((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSNodeContainer;
struct KNamelist;

XFS_EXTERN rc_t CC XFSNodeContainerMake (
                            const struct XFSNodeContainer ** Container
                            );

XFS_EXTERN rc_t CC XFSNodeContainerDispose (
                            const struct XFSNodeContainer * self
                            );

XFS_EXTERN rc_t CC XFSNodeContainerAddRef (
                            const struct XFSNodeContainer * self
                            );

XFS_EXTERN rc_t CC XFSNodeContainerRelease (
                            const struct XFSNodeContainer * self
                            );

XFS_EXTERN bool CC XFSNodeContainerHas (
                            const struct XFSNodeContainer * self,
                            const char * NodeName
                            );

XFS_EXTERN rc_t CC XFSNodeContainerGet (
                            const struct XFSNodeContainer * self,
                            const char * NodeName,
                            const struct XFSNode ** Node
                            );

XFS_EXTERN rc_t CC XFSNodeContainerAdd (
                            const struct XFSNodeContainer * self,
                            const struct XFSNode * Node
                            );

XFS_EXTERN rc_t CC XFSNodeContainerDel (
                            const struct XFSNodeContainer * self,
                            const char * NodeName
                            );

XFS_EXTERN rc_t CC XFSNodeContainerList (
                            const struct XFSNodeContainer * self,
                            const struct KNamelist ** List
                            );

XFS_EXTERN rc_t CC XFSNodeContainerClear (
                            const struct XFSNodeContainer * self
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _ncon_h_ */
