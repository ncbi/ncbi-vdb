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

#ifndef _h_xfs_
#define _h_xfs_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  Lyrics - that part should be used in client programs.
 *  There are no any data handling, just filesystem start and stop.
 *  Client should create instance of XFSControl structure by calling
 *  XFSControlMake method. File system will start and mounted by call
 *  XFSStart method. To stop filesystem, XFSStop method
 *  should be called, and mount should be destroyed by XFSControlDipsose
 *  method.
 *
 *  The typical scheme of usage is :
 *
 *      struct XFSModel * TheModel;
 *      struct XFSTree * TheTree;
 *      XFSControl TheControl;
 *      
 *      XFSModelMake ( & TheModel );
 *      XFSTreeMake ( Model, & TheTree );
 *      XFSControlMake ( TheTree, & TheControl );
 *          ... here some initialisation action, like arg settings
 *      XFSControlSetArg ( TheControl, Arg, Value );
 *      XFSStart ( TheControl, true/false );
 *          ... here some action
 *      XFSStop ( TheControl );
 *      XFSControlDispose ( TheControl );
 *      XFSPeerDestroyDummy(Peer);
 */

/*  Struct XFSControl. Apparently need to have XFS arguments, which
 *  are different on Dokan and Fuse, and may be some handle
 */
struct XFSControl;
struct XFSTree;

/*  Init/Destroy XFSControl structure
 */
XFS_EXTERN rc_t CC XFSControlMake(
                            const struct XFSTree * Tree,
                            struct XFSControl ** Control
                            );
XFS_EXTERN rc_t CC XFSControlDispose( struct XFSControl * self );

/*  Started filesystem, and creating mount point.
 */
XFS_EXTERN rc_t CC XFSStart( struct XFSControl * self );
/*  Unmount and stoping filesystem
 */
XFS_EXTERN rc_t CC XFSStop( struct XFSControl * self ); 

/*  Returns current XFSTree structure from control
 */
XFS_EXTERN rc_t CC XFSControlGetTree (
                                    struct XFSControl * self,
                                    const struct XFSTree ** Tree
                                    );

/*  Very special platform specific metnod which allows to unmount
 *  Fuse/Dokan without calling external program
 */
XFS_EXTERN rc_t CC XFSUnmountAndDestroy ( const char * MountPoint );

/*  Control argumenting, not sure it will be left in the same way
    These methods will not take affect after XFSStart
 */
XFS_EXTERN rc_t CC XFSControlSetArg (
                    struct XFSControl * self,
                    const char * Arg,
                    const char * Value
                    );
XFS_EXTERN const char * CC XFSControlGetArg (
                    struct XFSControl * self,
                    const char * Arg
                    );
XFS_EXTERN bool CC XFSControlHasArg (
                    struct XFSControl * self,
                    const char * Arg
                    );

XFS_EXTERN rc_t CC XFSControlSetAppName(
                    struct XFSControl * self,
                    const char * AppName
                    );
XFS_EXTERN const char * CC XFSControlGetAppName(
                    struct XFSControl * self
                    );

XFS_EXTERN rc_t CC XFSControlSetMountPoint(
                    struct XFSControl * self,
                    const char * MountPoint
                    );
XFS_EXTERN const char * CC XFSControlGetMountPoint(
                    struct XFSControl * self
                    );

/*  If You will call that method with LogFile = NULL, it will
    redirect log to some undisclosured standard place.
 */
XFS_EXTERN rc_t CC XFSControlSetLogFile (
                    struct XFSControl * self,
                    const char * LogFile
                    );
XFS_EXTERN const char * CC XFSControlGetLogFile (
                    struct XFSControl * self
                    );

/*  By default it always run in foreground, but could be set once
 *  as a daemon at the begining of application ... and no way back.
 */
XFS_EXTERN rc_t CC XFSControlDaemonize (
                    struct XFSControl * self
                    );
XFS_EXTERN bool CC XFSControlIsDaemonize (
                    struct XFSControl * self
                    );

/*  That method setup a label which will be shown in /etc/mtab entry
    You may use NULL, and in that case label will be "XFS"
 */
XFS_EXTERN rc_t CC XFSControlSetLabel(
                    struct XFSControl * self,
                    const char * Label
                    );
XFS_EXTERN const char * CC XFSControlGetLabel(
                    struct XFSControl * self
                    );

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _h_xfs_ */
