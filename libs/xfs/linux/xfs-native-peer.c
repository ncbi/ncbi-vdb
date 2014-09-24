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
#include <xfs/xfs-peer.h>
#include "xfs-priv.h"

#include <stdlib.h>     /* using malloc() */
#include <string.h>     /* using memset() */

/*  Some platform dependent headers
 */

#include "xfs-native-peer-operations.h"

/*  Some useless pranks
 */
#define MOO(Moo)    \
    OUTMSG( ( "%s: %d\n", Moo, __LINE__ ) )

XFS_EXTERN rc_t CC XFSControlInitVT ( XFSControl * self );

/*
 *  Virtuhai table and it's methods
 */
static rc_t XFS_FUSE_init_v1 ( XFSControl * self );
static rc_t XFS_FUSE_destroy_v1 ( XFSControl * self );
static rc_t XFS_FUSE_mount_v1 ( XFSControl * self );
static rc_t XFS_FUSE_loop_v1 ( XFSControl * self );
static rc_t XFS_FUSE_unmount_v1 ( XFSControl * self);

static XFSControl_vt_v1 XFSControl_VT_V1 = {
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
XFSControlInitVT ( XFSControl * self )
{
    if ( self == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }

    self -> vt = ( XFSControl_vt * )&XFSControl_VT_V1;

    return 0;
}   /* XFSControlInitVT () */

/*  Overloadable versions
 */
rc_t
XFS_FUSE_init_v1 ( XFSControl * self )
{
    XFSControlArgs * Args;

    Args = NULL;

    OUTMSG ( ( "XFS_FUSE_init()\n" ) );

    if ( self -> Control != NULL || self -> ControlOpt != NULL ) {
        OUTMSG ( ( "XFS_FUSE_init(): control is not empty\n" ) );

        return RC ( rcFS, rcNoTarg, rcInitializing, rcSelf, rcUnexpected );
    }

    if ( self -> Arguments != NULL ) {
        OUTMSG ( ( "XFS_FUSE_init(): arguments are not empty\n" ) );

        return RC ( rcFS, rcNoTarg, rcInitializing, rcSelf, rcUnexpected );
    }

    Args = ( XFSControlArgs * ) malloc(
                                sizeof ( struct XFSControlArgs )
                                );
    if ( Args == NULL ) {
        return RC ( rcFS, rcNoTarg, rcAllocating, rcSelf, rcNull );
    }

    * ( Args -> MountPoint ) = 0;
    string_copy_measure (
                    Args -> Label,
                    sizeof ( Args -> Label ),
                    "XFS"
                    );

    self -> Arguments = Args;

    return 0;
}   /* XFS_FUSE_init_v1 () */

rc_t
XFS_FUSE_destroy_v1 ( XFSControl * self )
{
    OUTMSG ( ( "XFS_FUSE_destroy()\n" ) );

    if ( self -> Arguments == NULL ) {
        OUTMSG ( ( "XFS_FUSE_destroy(): arguments are empty\n" ) );

/* Do we really need that ???
        return RC ( rcFS, rcNoTarg, rcDestroying, rcSelf, rcUnexpected );
*/
    }
    else {
        free ( self -> Arguments );
        self -> Arguments = NULL;
    }

    return 0;
}   /* XFS_FUSE_destroy_v1 () */

rc_t
XFS_FUSE_mount_v1 ( XFSControl * self )
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

    RCt = XFS_Private_InitOperations ( & TheFuseOperations );
    if ( RCt != 0 ) {
        OUTMSG ( ( "Can not initialize operations\n" ) );
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
    }


MOO ( "H" );

    memset ( & FuseArgs, 0, sizeof FuseArgs );
    Result = fuse_opt_add_arg ( & FuseArgs, self -> Arguments -> Label );
    Result = fuse_opt_add_arg ( & FuseArgs, self -> Arguments -> MountPoint );
    if ( Result != 0 ) {
        OUTMSG ( ( "Can not mount\n" ) );
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
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
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
    }

MOO ( "H" );

OUTMSG ( ( "Mnt = %s\nMlt = %d\nFrg = %d\n", MountPoint, Multithreaded, Foreground ) );


    FuseChannel = fuse_mount ( MountPoint, & FuseArgs );
    if ( FuseChannel == NULL ) {
        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not mount\n" ) );
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
    }
    self -> ControlOpt = FuseChannel;

MOO ( "H" );

        /*  Note we are passing Peer as private data to fuse_context
         */
    FuseStruct = fuse_new (
                        FuseChannel,
                        & FuseArgs,
                        & TheFuseOperations,
                        sizeof ( struct fuse_operations ),
                        self -> Peer
                    );
    if ( FuseStruct == NULL ) {
        fuse_unmount ( MountPoint, FuseChannel );

        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not fuse_new\n" ) );
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
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
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
    }

MOO ( "H" );

    Result = fuse_set_signal_handlers ( fuse_get_session ( FuseStruct ) );
    if ( Result == -1 ) {
        fuse_unmount ( MountPoint, FuseChannel );

        fuse_opt_free_args ( & FuseArgs );

        OUTMSG ( ( "Can not daemonize\n" ) );
        return RC ( rcFS, rcNoTarg, rcExecuting, rcParam, rcFailed ); 
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

rc_t
XFS_FUSE_loop_v1( XFSControl * self )
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

        fuse_remove_signal_handlers ( fuse_get_session ( FuseStruct ) );

        RCt = Result == 0 ? 0 : 1;
    }

    return RCt;
}   /* XFS_FUSE_loop_v1 () */
rc_t
XFS_FUSE_unmount_v1 ( XFSControl * self )
{
    OUTMSG ( ( "XFS_FUSE_unmount()\n" ) );

    if ( self -> Control != NULL && self -> ControlOpt != NULL ) {
        fuse_unmount (
                    self -> Arguments -> MountPoint,
                    ( struct fuse_chan * ) self -> ControlOpt
                    );

        fuse_destroy ( (struct fuse * ) self -> Control );

        self -> Control = NULL;
        self -> ControlOpt = NULL;
    }
    else {
        OUTMSG ( ( "XFS_FUSE_unmount(): empty control passed\n" ) );
    }

    return 0;
}   /* XFS_FUSE_unmount_v1 () */
