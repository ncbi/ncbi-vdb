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

#ifndef _xfs_tree_h_
#define _xfs_tree_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics.
 ++   That file contains interfaces for XFS Tree and TreeDepot
 ||
((*/

/*)))
 ///   XFSModel and other forwards.
(((*/
struct XFSTree;
struct XFSTreeDepot;
struct XFSNode;
struct XFSModel;
struct KNamelist;
struct VPath;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 ((    Really I will introduce separate TreeMake and Verify methods
  ))   I think it is more convinient than checking model on Make stage
 ((*/

/*))   Verification flags, here would be more then
 //    Will think about it later
((*/
#define XFS_CHECK_INTEGRITY      (1 << 0)
#define XFS_CHECK_REMOTE_CONTENT (1 << 1)
#define XFS_CHECK_LOOCAL_CONTENT (1 << 2)

XFS_EXTERN
rc_t CC
XFSTreeVerifyModel ( const struct XFSModel * Model, uint32_t Flags );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
XFS_EXTERN
rc_t CC
XFSTreeMake ( const struct XFSModel * Model, struct XFSTree ** Tree );

XFS_EXTERN
rc_t CC
XFSTreeDispose ( struct XFSTree * self );

XFS_EXTERN
rc_t CC
XFSTreeAddRef ( const struct XFSTree * self );

XFS_EXTERN
rc_t CC
XFSTreeRelease ( const struct XFSTree * self );

XFS_EXTERN
rc_t CC
XFSTreeFindNode (
                const struct XFSTree * self,
                const char * Path,
                const struct XFSNode ** Node
                );

XFS_EXTERN
rc_t CC
XFSTreeFindNodeForPath (
                const struct XFSTree * self,
                const struct VPath * Path,
                const struct XFSNode ** Node
                );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))))
 ((((   Lyrics. That is a place for storeing and retrieving current
  ))))           version of tree, and it is almost thread safe :lol:
 ((((*/

XFS_EXTERN
rc_t CC
XFSTreeDepotMake ( const struct XFSTreeDepot ** Depot );

XFS_EXTERN
rc_t CC
XFSTreeDepotDispose ( const struct XFSTreeDepot * self );

XFS_EXTERN
rc_t CC
XFSTreeDepotGet (
            const struct XFSTreeDepot * self,
            const struct XFSTree ** Tree
            );

XFS_EXTERN
rc_t CC
XFSTreeDepotSet (
            const struct XFSTreeDepot * self,
            const struct XFSTree * Tree
            );

XFS_EXTERN
rc_t CC
XFSTreeDepotVersion (
                    const struct XFSTreeDepot * self,
                    char * Buffer,
                    size_t BufferSize
                    );

XFS_EXTERN
rc_t CC
XFSTreeDepotResource (
                    const struct XFSTreeDepot * self,
                    char * Buffer,
                    size_t BufferSize
                    );

XFS_EXTERN
rc_t CC
XFSTreeDepotFindNode (
                    const struct XFSTreeDepot * self,
                    const char * Path,
                    const struct XFSNode ** Node
                    );

XFS_EXTERN
rc_t CC
XFSTreeDepotFindNodeForPath (
                    const struct XFSTreeDepot * self,
                    const struct VPath * Path,
                    const struct XFSNode ** Node
                    );

/* Enuff for today */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_tree_h_ */
