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
#include <xfs/tree.h>
#include "xfs-priv.h"

#include <stdlib.h>     /* using malloc() */
#include <string.h>     /* using memset() */

/*  Some platform dependent headers
 */

#include "operations.h"

/*  Some useless pranks
 */
#define MOO(Moo)    \
    OUTMSG( ( "%s: %d\n", Moo, __LINE__ ) )

/*
 *  Virtuhai table and it's methods
 */
static rc_t XFS_FUSE_init_v1 ( struct XFSControl * self );
static rc_t XFS_FUSE_destroy_v1 ( struct XFSControl * self );
static rc_t XFS_FUSE_mount_v1 ( struct XFSControl * self );
static rc_t XFS_FUSE_loop_v1 ( struct XFSControl * self );
static rc_t XFS_FUSE_unmount_v1 ( struct XFSControl * self);

static struct XFSControl_vt_v1 XFSControl_VT_V1 = {
    1,
    1,
    XFS_FUSE_init_v1,
    XFS_FUSE_destroy_v1,
    XFS_FUSE_mount_v1,
    XFS_FUSE_loop_v1,
    XFS_FUSE_unmount_v1
};

static struct fuse_operations TheFuseOperations;

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
}   /* XFSControlPlatformInit () */

/*  Overloadable versions
 */
rc_t
XFS_FUSE_init_v1 ( struct XFSControl * self )
{
    rc_t RCt;

    RCt = 0;

    OUTMSG ( ( "XFS_FUSE_init()\n" ) );

    if ( self -> Control != NULL ) {
        OUTMSG ( ( "XFS_FUSE_init(): control is not empty\n" ) );

        return XFS_RC ( rcUnexpected );
    }

    if ( self -> Arguments == NULL ) {
        OUTMSG ( ( "XFS_FUSE_init(): arguments are empty\n" ) );

        return XFS_RC ( rcUnexpected );
    }

    if ( XFSControlGetLabel ( self ) == NULL ) {
        RCt = XFSControlSetLabel ( self, "FUSE" );
    }

    return RCt;
}   /* XFS_FUSE_init_v1 () */

rc_t
XFS_FUSE_destroy_v1 ( struct XFSControl * self )
{
    OUTMSG ( ( "XFS_FUSE_destroy()\n" ) );

    if ( self == NULL ) {
        OUTMSG ( ( "XFS_FUSE_destroy_v1(): NULL self passed" ) );

        return XFS_RC ( rcNull );
    }

    return 0;
}   /* XFS_FUSE_destroy_v1 () */

rc_t
XFS_FUSE_mount_v1 ( struct XFSControl * self )
{
    rc_t RCt;

    struct fuse_args FuseArgs;
    struct fuse_chan * FuseChannel;
    struct fuse * FuseStruct;
    char * MountPoint;
    int Foreground;
    int Multithreaded;
    int Result;

    RCt = 0;

    OUTMSG ( ( "XFS_FUSE_mount()\n" ) );

    if ( self == NULL ) {
        OUTMSG ( ( "ZERO self passed\n" ) );
        return XFS_RC ( rcNull );
    }

    RCt = XFS_Private_InitOperations ( & TheFuseOperations );
    if ( RCt != 0 ) {
        OUTMSG ( ( "Can not initialize operations\n" ) );
        return XFS_RC ( rcFailed ); 
    }


MOO ( "H" );

    memset ( & FuseArgs, 0, sizeof FuseArgs );
    Result = fuse_opt_add_arg ( & FuseArgs, XFSControlGetLabel ( self ) );
    Result = fuse_opt_add_arg ( & FuseArgs, XFSControlGetMountPoint ( self ) );
#if ! MAC
/* Options MAC does not know about */
    Result = fuse_opt_add_arg ( & FuseArgs, "-o" );
    Result = fuse_opt_add_arg( & FuseArgs, "big_writes" );
#endif /* MAC */

    if ( Result != 0 ) {
        OUTMSG ( ( "Can not mount\n" ) );
        return XFS_RC ( rcFailed ); 
    }

MOO ( "H" );
        /*  Adding the mountpoint
         */
    Result = fuse_parse_cmdline (
                    & FuseArgs,
                    & MountPoint,
                    & Multithreaded,
                    & Foreground
                );
    if ( Result != 0 ) {
        OUTMSG ( ( "Can not parse arguments\n" ) );
        return XFS_RC ( rcFailed ); 
    }

MOO ( "H" );

OUTMSG ( ( "Mnt = %s\nMlt = %d\nFrg = %d\n", MountPoint, Multithreaded, Foreground ) );


    FuseChannel = fuse_mount ( MountPoint, & FuseArgs );
    if ( FuseChannel == NULL ) {
        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not mount\n" ) );
        return XFS_RC ( rcFailed ); 
    }
    self -> ControlAux = FuseChannel;

MOO ( "H" );

        /*  Note passing TreeDepot as private data to fuse_context
         */
    FuseStruct = fuse_new (
                        FuseChannel,
                        & FuseArgs,
                        & TheFuseOperations,
                        sizeof ( struct fuse_operations ),
                        self -> TreeDepot
                    );
    if ( FuseStruct == NULL ) {
        fuse_unmount ( MountPoint, FuseChannel );

        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not fuse_new\n" ) );
        return XFS_RC ( rcFailed ); 
    }
    self -> Control = FuseStruct;


MOO ( "H" );

/* TODO REMOVE */
Foreground = 1;

    Result = fuse_daemonize ( Foreground );
    if ( Result == -1 ) {
        fuse_unmount ( MountPoint, FuseChannel );

        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not daemonize\n" ) );
        return XFS_RC ( rcFailed ); 
    }

MOO ( "H" );

    Result = fuse_set_signal_handlers ( fuse_get_session ( FuseStruct ) );
    if ( Result == -1 ) {
        fuse_unmount ( MountPoint, FuseChannel );

        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not set signal handlers\n" ) );
        return XFS_RC ( rcFailed ); 
    }

/*
MOO ( "H" );
    if ( Multithreaded != 0 ) {
        Result = fuse_loop_mt ( FuseStruct );
    }
    else {
        Result = fuse_loop ( FuseStruct );
    }

MOO ( "H" );

    fuse_remove_signal_handlers ( fuse_get_session ( FuseStruct ) );
*/

    fuse_opt_free_args ( & FuseArgs );

MOO ( "H" );

    return Result == 0 ? 0 : 1;
}   /* XFS_FUSE_mount_v1 () */

static bool bashmalton = false;

rc_t
XFS_FUSE_loop_v1( struct XFSControl * self )
{
    rc_t RCt;

    struct fuse * FuseStruct;
    int Result;

    RCt = 0;

    OUTMSG ( ( "XFS_FUSE_loop()\n" ) );

    if ( self == NULL ) {
        OUTMSG ( ( "XFS_FUSE_loop(): empty control passed\n" ) );
        RCt = 1;
    }
    else {
        FuseStruct = (struct fuse * ) self -> Control;

MOO ( "H" );
        Result = fuse_loop_mt ( FuseStruct );

MOO ( "H" );

        RCt = Result == 0 ? 0 : 1;
    }

	bashmalton = true;

    return RCt;
}   /* XFS_FUSE_loop_v1 () */

rc_t
XFS_FUSE_unmount_v1 ( struct XFSControl * self )
{
    struct fuse * FuseStruct;

    FuseStruct = (struct fuse * ) self -> Control;

    OUTMSG ( ( "XFS_FUSE_unmount()\n" ) );

    if ( self -> Control != NULL ) {

#if ! MAC
        fuse_exit ( FuseStruct );
#endif /* MAC */

/*
OUTMSG ( ( "|o|fuse_unmount()\n" ) );
*/
        fuse_unmount (
                    XFSControlGetMountPoint ( self ),
                    self -> ControlAux
                    );

/*
OUTMSG ( ( "|o|waiting thread()\n" ) );
*/
		KThreadWait ( self -> Thread, 0 );

/*
OUTMSG ( ( "|o|releasing thread()\n" ) );
*/
		KThreadRelease ( self -> Thread );
		self -> Thread = NULL;

/*
OUTMSG ( ( "|o|fuse_remove_signal_handlers()\n" ) );
*/
        fuse_remove_signal_handlers ( fuse_get_session ( FuseStruct ) );

/*
OUTMSG ( ( "|o|fuse_destroy()\n" ) );
*/
        fuse_destroy ( FuseStruct );

        self -> Control = NULL;
        self -> ControlAux = NULL;

/*
OUTMSG ( ( "|o|exiting fuse()\n" ) );
*/
    }
    else {
        OUTMSG ( ( "XFS_FUSE_unmount(): empty control passed\n" ) );
    }

    return 0;
}   /* XFS_FUSE_unmount_v1 () */
