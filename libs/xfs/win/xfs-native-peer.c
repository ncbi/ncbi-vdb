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
#include <xfs/xfs.h>
#include "xfs-priv.h"

#include <sysalloc.h>
#include <stdlib.h>

/*  Some platform dependent headers
 */

#include "xfs-native-peer-operations.h"

/*
 *  Virtuhai table and it's methods
 */
static rc_t XFS_WIN_init_v1( XFSControl * self );
static rc_t XFS_WIN_destroy_v1( XFSControl * self );
static rc_t XFS_WIN_start_v1(
                        const XFSControl * self,
                        bool AsBackgroundProcess
                        );
static rc_t XFS_WIN_stop_v1( const XFSControl * self);

static XFSControl_vt_v1 XFSControl_VT_V1 = {
    1,
    1,
    XFS_WIN_init_v1,
    XFS_WIN_destroy_v1,
    XFS_WIN_start_v1,
    XFS_WIN_stop_v1
};

/*  Control init.
 */
LIB_EXPORT
rc_t CC
XFSControlInit( XFSControl ** self )
{
    rc_t RCt;
    XFSControl *TheControl;

    RCt = 0;
    TheControl = NULL;

    if ( self == NULL ) {
            /*  TODO : not sure about rc */
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcNull );
    }

    TheControl = *self;

    if ( TheControl != NULL ) {
        return RC ( rcFS, rcNoTarg, rcConstructing, rcSelf, rcUnexpected );
    }

    TheControl = calloc ( 1, sizeof ( XFSControl ) );
    if ( TheControl == NULL ) {
        return RC ( rcFS, rcNoTarg, rcAllocating, rcSelf, rcNull );
    }

    TheControl -> vt = ( XFSControl_vt * )&XFSControl_VT_V1;

    RCt = TheControl -> vt -> v1.init ( TheControl );
    if ( RCt != 0 ) {
        TheControl -> vt -> v1.destroy ( TheControl );

        free ( TheControl );
    } else {
        * self = TheControl;
    }

    return RCt;
}   /* XFSControlInit() */

/*  Overloadable versions
 */
rc_t
XFS_WIN_init_v1( XFSControl * self )
{
    OUTMSG ( ( "XFS_WIN_init()\n" ) );
    return 0;
}   /* XFS_WIN_init() */

rc_t
XFS_WIN_destroy_v1( XFSControl * self )
{
    OUTMSG ( ( "XFS_WIN_destroy()\n" ) );
    return 0;
}   /* XFS_WIN_destroy() */

rc_t
XFS_WIN_start_v1( const XFSControl * self, bool AsBackgroundProcess )
{
    OUTMSG ( ( "XFS_WIN_start()\n" ) );
    return 0;
}   /* XFS_WIN_start() */

rc_t
XFS_WIN_stop_v1( const XFSControl * self )
{
    OUTMSG ( ( "XFS_WIN_stop()\n" ) );
    return 0;
}   /* XFS_WIN_stop() */

