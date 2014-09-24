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
 *  Client should create instance of XFSControl structure and init
 *  it by XFSControlInit. File system will start and mounted by call
 *  XFSStart method. To stop filesystem, XFSStop method should be 
 *  called, and control should be destroyed by XFSControlDestroy
 *  method.
 *
 *  The typical scheme of usage is :
 *
 *      XFSPeer * ThePeer;
 *      XFSControl TheControl;
 *      XFSPeerCreateDummy(&Peer);
 *      XFSControlInit(&TheControl, Peer);
 *          ... here some initialisation action, like arg settings
 *      XFSControlSetArg(&TheControl, Arg, Value);
 *      XFSStart(&TheControl, true/false);
 *          ... here some action
 *      XFSStop(&TheControl);
 *      XFSControlDestroy(&TheControl);
 *      XFSPeerDestroyDummy(Peer);
 */

/*  Struct XFSControl. Apparently need to have XFS arguments, which
 *  are different on Dokan and Fuse, and may be some handle
 */
typedef struct XFSControl XFSControl;
struct XFSPeer;

/*  Init/Destroy XFSControl structure
 */
XFS_EXTERN rc_t CC XFSControlInit(
                            XFSControl ** self,
                            struct XFSPeer * Peer
                            );
XFS_EXTERN rc_t CC XFSControlDestroy( XFSControl * self );

/*  Started filesystem, and creating mount point.
 */
XFS_EXTERN rc_t CC XFSStart( XFSControl * self );
/*  Unmount and stoping filesystem
 */
XFS_EXTERN rc_t CC XFSStop( XFSControl * self ); 

/*  Control argumenting, not sure it will be left in the same way
    These methods will not take affect after XFSStart
 */
XFS_EXTERN rc_t CC XFSControlSetMountPoint(
                    XFSControl * self,
                    const char * MountPoint
                    );
XFS_EXTERN rc_t CC XFSControlGetMountPoint(
                    const XFSControl * self,
                    char * Buffer,
                    size_t BufferSize
                    );

/*  That method setup a label which will be shown in /etc/mtab entry
    You may use NULL, and in that case label will be "XFS"
 */
XFS_EXTERN rc_t CC XFSControlSetLabel(
                    XFSControl * self,
                    const char * Label
                    );
XFS_EXTERN rc_t CC XFSControlGetLabel(
                    const XFSControl * self,
                    char * Buffer,
                    size_t BufferSize
                    );

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _h_xfs_ */
