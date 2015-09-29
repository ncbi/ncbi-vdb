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

#ifndef _xcache_h_
#define _xcache_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

/*))))
 ((((   There are methods which allows to render DbGap Project cached
  ))))  files into nodes
 ((((*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

struct XFSNode;
struct XFSGapCache;
struct KNamelist;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* XFSGapCache                                                       */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*
 *  That method will create GapCache object for ProjectId, and, if
 *  ProjectId is zero - it will create object for public cache.
 *  If ReadOnly set into false, that means, user could delete cached
 *  files from cache - the only editing operation allowed.
 */
XFS_EXTERN rc_t CC XFSGapCacheMake (
                            struct XFSGapCache ** Cache,
                            uint32_t ProjectId,
                            bool ReadOnly
                            );

XFS_EXTERN rc_t CC XFSGapCacheDispose (
                            struct XFSGapCache * self
                            );

XFS_EXTERN rc_t CC XFSGapCacheList (
                            const struct XFSGapCache * self,
                            const struct KNamelist ** Names
                            );

XFS_EXTERN rc_t CC XFSGapCacheFind (
                            const struct XFSGapCache * self,
                            const struct XFSNode ** Node,
                            const char * NodeName
                            );

XFS_EXTERN rc_t CC XFSGapCacheDeleteNode (
                            const struct XFSGapCache * self,
                            const char * NodeName
                            );

XFS_EXTERN rc_t CC XFSGapCacheProjectId (
                            const struct XFSGapCache * self,
                            uint32_t * ProjectId
                            );

XFS_EXTERN rc_t CC XFSGapCacheReadOnly (
                            const struct XFSGapCache * self,
                            bool * ReadOnly
                            );

#ifdef __cplusplus 
}
#endif /* __cplusplus */

#endif /* _xcache_h_ */
