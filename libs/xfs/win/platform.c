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

#include <klib/rc.h>
#include <klib/out.h>
#include <klib/text.h>
#include <kproc/thread.h>
#include <xfs/xfs.h>

#include "xfs-priv.h"
#include "platform.h"

#include <sysalloc.h>
#include <stdlib.h>

/*  Some platform dependent headers
 */

#include "operations.h"


/*  Some useless pranks
 */
#define MOO(Moo)    \
    OUTMSG( ( "%s: %d\n", Moo, __LINE__ ) )

XFS_EXTERN rc_t CC XFSSecurityInit ();
XFS_EXTERN rc_t CC XFSSecurityDeinit ();

/*
 *  Virtuhai table and it's methods
 */
static rc_t XFS_DOKAN_init_v1( struct XFSControl * self );
static rc_t XFS_DOKAN_destroy_v1( struct XFSControl * self );
static rc_t XFS_DOKAN_mount_v1( struct XFSControl * self );
static rc_t XFS_DOKAN_loop_v1( struct XFSControl * self);
static rc_t XFS_DOKAN_unmount_v1( struct XFSControl * self);

static struct XFSControl_vt_v1 XFSControl_VT_V1 = {
    1,
    1,
    XFS_DOKAN_init_v1,
    XFS_DOKAN_destroy_v1,
    XFS_DOKAN_mount_v1,
    XFS_DOKAN_loop_v1,
    XFS_DOKAN_unmount_v1
};

static DOKAN_OPERATIONS TheDokanOperations;

/*  Control init.
 */
LIB_EXPORT
rc_t CC
XFSControlPlatformInit ( struct XFSControl * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    self -> vt = ( union XFSControl_vt * ) & XFSControl_VT_V1;

    return 0;
}   /* XFSControlInit () */

/*  Overloadable versions
 */
rc_t
XFS_DOKAN_init_v1( struct XFSControl * self )
{
    rc_t RCt;

    RCt = 0;

    OUTMSG ( ( "XFS_DOKAN_init()\n" ) );

        /*) Standard checks
         (*/
    if ( self -> Control != NULL ) {
        OUTMSG ( ( "XFS_DOKAN_init(): control is not empty\n" ) );
        return XFS_RC ( rcUnexpected );
    }

    if ( self -> Arguments == NULL ) {
        OUTMSG ( ( "XFS_DOKAN_init(): arguments are empty\n" ) );
        return XFS_RC ( rcUnexpected );
    }

    if ( XFSControlGetLabel ( self ) == NULL ) {
        RCt = XFSControlSetLabel ( self, "DOKAN" );
    }

    return RCt;
}   /* XFS_DOKAN_init() */

rc_t
XFS_DOKAN_destroy_v1( struct XFSControl * self )
{
    PDOKAN_OPTIONS Options;

    Options = NULL;

    OUTMSG ( ( "XFS_DOKAN_destroy()\n" ) );

    if ( self == NULL ) { 
        OUTMSG ( ( "XFS_DOKAN_destroy(): NULL self passed" ) );

        return XFS_RC ( rcNull );
    }

    Options = ( PDOKAN_OPTIONS ) self -> Control;

    if ( Options == NULL ) {
        OUTMSG ( ( "XFS_DOKAN_destroy(): options are empty\n" ) );
    }
    else {
        Options -> MountPoint = NULL;

        free ( Options );
        self -> Control = NULL;
    }

    return 0;
}   /* XFS_DOKAN_destroy() */

static WCHAR wRealMountPoint [ 3000 ];
XFS_EXTERN rc_t CC XFSPathInnerToNative (
                                WCHAR * NativePathBuffer,
                                size_t NativePathBufferSize,
                                const char * InnerPath
                                );

rc_t
XFS_DOKAN_mount_v1( struct XFSControl * self )
{
    rc_t RCt;
    DOKAN_OPTIONS * Options;

    RCt = 0;
    Options = NULL;

    OUTMSG ( ( "XFS_DOKAN_mount()\n" ) );

    if ( self == NULL ) {
        OUTMSG ( ( "ZERO self passed\n" ) );
        return XFS_RC ( rcNull );
    }

    if ( ( RCt = XFSSecurityInit () ) != 0 ) {
        OUTMSG ( ( "Can not initialize DOKAN security\n" ) );
        return RCt;
    }

    RCt = XFS_Private_InitOperations ( & TheDokanOperations );
    if ( RCt != 0 ) {
        OUTMSG ( ( "Can not initialize DOKAN operations\n" ) );
        return XFS_RC ( rcFailed );
    }

        /*) Here we are allocating DOKAN options and it's global context
         (*/
    Options = calloc ( 1, sizeof ( DOKAN_OPTIONS ) );
    if ( Options == NULL ) {
        return XFS_RC ( rcNull );
    }

    Options -> Version = DOKAN_VERSION;
    Options -> ThreadCount = 0; /* Default Value */
    Options -> Options = 0L;
    Options -> Options |= DOKAN_OPTION_KEEP_ALIVE | DOKAN_OPTION_DEBUG;
        /*) using Peer as GlobalContext as for FUSE implementation
         (*/
OUTMSG ( ( "Control [0x%p] Peer [0x%p]\n", self, self -> TreeDepot ) );
    Options -> GlobalContext = ( ULONG64 )( self -> TreeDepot );

    XFSPathInnerToNative (
                        wRealMountPoint,
                        sizeof ( wRealMountPoint ),
                        XFSControlGetMountPoint ( self )
                        );
    Options -> MountPoint = wRealMountPoint;

    self -> Control = Options;

/*  TODO !!!!
    We will split mount method for mount'n'loop later, so there is 
    usual routine stuff

    Currently mount and loop are called from DokanMain function from 
    loop()
*/

    return 0;
}   /* XFS_DOKAN_mount() */

rc_t
XFS_DOKAN_loop_v1( struct XFSControl * self )
{
    rc_t RCt;
    DOKAN_OPTIONS * Options;
    const struct XFSTree * Tree;

    RCt = 0;
    Options = NULL;
    Tree = NULL;

    OUTMSG ( ( "XFS_DOKAN_loop()\n" ) );

    if ( self == NULL ) {
        OUTMSG ( ( "XFSControl: ZERO self passed\n" ) );
        return XFS_RC ( rcNull );
    }

    if ( self -> TreeDepot == NULL ) {
        OUTMSG ( ( "XFSControl: ZERO passed\n" ) );
        return XFS_RC ( rcNull );
    }

    RCt = XFSControlGetTree ( self, & Tree );
    if ( RCt != 0 || Tree == NULL ) {
        OUTMSG ( ( "XFSControl: ZERO Tree DATA passed\n" ) );
        return XFS_RC ( rcNull );
    }

    Options = ( DOKAN_OPTIONS * ) self -> Control;
    if ( Options == NULL ) {
        OUTMSG ( ( "XFSControl: ZERO options passed\n" ) );
        return XFS_RC ( rcNull );
    }

OUTMSG ( ( "XFS_DOKAN_loop(): Tree [0x%p] Data [0x%p]\n",  self -> TreeDepot, Tree ) );


/*  We will split mount method for mount'n'loop later, so there is 
    usual routine stuff
*/
        /*)
         /  There we are running DokanMain
        (*/
    switch ( DokanMain ( Options, & TheDokanOperations ) ) {
        case DOKAN_SUCCESS :
            OUTMSG ( ( "DokanMain() : general success\n" ) );
            break;
        case DOKAN_ERROR :
            OUTMSG ( ( "DokanMain() : general error\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
        case DOKAN_DRIVE_LETTER_ERROR :
            OUTMSG ( ( "DokanMain() : bad drive letter\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
        case DOKAN_DRIVER_INSTALL_ERROR :
            OUTMSG ( ( "DokanMain() : can't install driver\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
        case DOKAN_START_ERROR :
            OUTMSG ( ( "DokanMain() : can't start, something wrong\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
        case DOKAN_MOUNT_ERROR :
            OUTMSG ( ( "DokanMain() : can't assigh a drive letter or mount point\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
        case DOKAN_MOUNT_POINT_ERROR :
            OUTMSG ( ( "DokanMain() : mount point is invalid\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
        default :
            OUTMSG ( ( "DokanMain() : something wrong happens\n" ) );
            return RC ( rcFS, rcNoTarg, rcExecuting, rcNoObj, rcError );
    }

OUTMSG ( ( "XFS_DOKAN_loop(): NOO Tree [0x%p]\n",  self -> TreeDepot ) );

    return 0;
}   /* XFS_DOKAN_loop() */

rc_t
XFS_DOKAN_unmount_v1( struct XFSControl * self )
{
    rc_t RCt;
    DOKAN_OPTIONS * Options;

    RCt = 0;
    Options = NULL;

    if ( self == NULL ) {
        OUTMSG ( ( "ZERO self passed\n" ) );
        /*
        return XFS_RC ( rcNull );
        */
        return 0;
    }

    if ( self -> Control == NULL ) {
        OUTMSG ( ( "ZERO self passed\n" ) );
        /*
        return XFS_RC ( rcNull );
        */
        return 0;
    }

    Options = ( DOKAN_OPTIONS * ) self -> Control;
    if ( ! DokanRemoveMountPoint ( Options -> MountPoint ) ) {
        OUTMSG ( ( "Can not unmount [%s]\n", Options -> MountPoint ) );
        return XFS_RC ( rcError );
    }

    XFSSecurityDeinit ();

    return 0;
}   /* XFS_DOKAN_unmount() */

