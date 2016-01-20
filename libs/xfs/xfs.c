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

#include <klib/out.h>
#include <klib/text.h>

#include <xfs/xfs.h>
#include <xfs/tree.h>

#include "platform.h"
#include "schwarzschraube.h"
#include "owp.h"
#include "xfs-priv.h"

#include <sysalloc.h>
#include <stdlib.h> /* we are using calloc */
#include <string.h> /* we are using memset */

/*  Some forwards and declarations
 */
static rc_t XFSVeryMainLoop ( void * Data );

/*  The code, which is checking version is quite similar for
 *  destroy/start/stop ... and, possible for other methods,
 *  so we do that then.
 */
static
rc_t
XFSControlStandardSelfCheck( const struct XFSControl * self )
{
        /*  TODO : here and further not sure about rc */

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> vt == NULL ) {
        return XFS_RC ( rcNull );
    }

        /*  Should be extended for switch later
         */
    if ( self -> vt -> v1 . maj != 1 && self -> vt -> v1 . min != 1 ) {
        return XFS_RC ( rcInvalid );
    }

    if (    self -> vt -> v1.init == NULL
        ||  self -> vt -> v1.destroy == NULL 
        ||  self -> vt -> v1.mount == NULL 
        ||  self -> vt -> v1.loop == NULL 
        ||  self -> vt -> v1.unmount == NULL 
        ) {
        return XFS_RC ( rcInvalid );
    }

    if ( self -> Arguments == NULL ) {
        return XFS_RC ( rcNull );
    }

    return 0;
}   /* XFSControlStandardSelfCheck () */

/*  The code will create  and init instance of XFSControl,
 *  it is calling external method
 *  XFSControlInitVT (), which is platform dependent
 */
LIB_EXPORT
rc_t CC
XFSControlMake (
            const struct XFSTree * Tree,
            struct XFSControl ** Control
)
{
    rc_t RCt;
    struct XFSControl * TheControl;

    RCt = 0;
    TheControl = NULL;

    if ( Control == NULL || Tree == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Control = NULL;

    TheControl = calloc ( 1, sizeof ( struct XFSControl ) );
    if ( TheControl == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSOwpMake ( & ( TheControl -> Arguments ) );
    if ( RCt == 0 ) {

        RCt = XFSControlPlatformInit ( TheControl );
        if ( RCt == 0 ) {

            RCt = XFSTreeDepotMake (
                        ( const struct XFSTreeDepot ** )
                                & ( TheControl -> TreeDepot )
                        );
            if ( RCt == 0 ) {

                RCt = XFSTreeDepotSet ( TheControl -> TreeDepot, Tree );
                if ( RCt == 0 ) {

                    RCt = TheControl -> vt -> v1.init ( TheControl );
                    if ( RCt == 0 ) {
                        * Control = TheControl;
                    }
                }
            }
        }
    }


    if ( RCt != 0 ) {
        XFSControlDispose ( TheControl );

        * Control = NULL;
    }

    return RCt;
}   /* XFSControlInit () */

/*  Control destroy.
 */
LIB_EXPORT
rc_t CC
XFSControlDispose ( struct XFSControl * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL ) {
        return 0;
    }

    if ( self -> TreeDepot != NULL ) {
        XFSTreeDepotDispose ( self -> TreeDepot );

        self -> TreeDepot = NULL;
    }

    if ( self -> Arguments != NULL ) {
        XFSOwpDispose ( self -> Arguments );

        self -> Arguments = NULL;
    }

    if ( self -> vt != NULL ) {
        self -> vt -> v1.destroy ( self );

        self -> vt = NULL;
    }

    free ( self );

    return RCt;
}   /* XFSControlDestroy () */

/*  Start file system
 */
LIB_EXPORT
rc_t CC
XFSStart ( struct XFSControl * self )
{
    rc_t RCt;

    RCt = XFSControlStandardSelfCheck ( self );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = self -> vt -> v1.mount ( self );
    XFSVeryMainLoop ( self );

    return RCt;
}   /* XFSStart () */

/*  Stop filesystem.
 */
LIB_EXPORT
rc_t CC
XFSStop ( struct XFSControl * self )
{
    rc_t RCt;

    RCt = XFSControlStandardSelfCheck ( self );
    if ( RCt == 0 ) {

        RCt = self -> vt -> v1.unmount ( self );
    }

    return RCt;
}   /* XFSStop () */

LIB_EXPORT
rc_t CC
XFSControlGetTree (
                struct XFSControl * self,
                const struct XFSTree ** Tree
)
{
    if ( self == NULL || Tree == NULL ) {
        return XFS_RC ( rcNull );
    }

    return XFSTreeDepotGet ( self -> TreeDepot, Tree );
}   /* XFSControlGetTree () */

/*  Here we are goint to implement some Arguments methods
 */
LIB_EXPORT
rc_t CC
XFSControlSetArg (
                struct XFSControl * self,
                const char * ArgName,
                const char * ArgValue
)
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    return XFSOwpSet ( self -> Arguments, ArgName, ArgValue );
}   /* XFSControlSetArg () */

LIB_EXPORT
const char * CC
XFSControlGetArg ( struct XFSControl * self, const char * ArgName )
{
    return self == NULL
                ? NULL
                : XFSOwpGet ( self -> Arguments, ArgName )
                ;
}   /* XFSControlGetArg () */

LIB_EXPORT
bool CC
XFSControlHasArg ( struct XFSControl * self, const char * ArgName )
{
    if ( self != NULL && ArgName != NULL ) {
        return XFSOwpHas ( self -> Arguments, ArgName );
    }
    return false;
}   /* XFSControlHasArg () */


LIB_EXPORT
rc_t CC
XFSControlSetAppName ( struct XFSControl * self, const char * AppName )
{
    return XFSControlSetArg (
                        self,
                        XFS_CONTROL_APPNAME,
                        ( AppName == NULL ? "mount-tool" : AppName )
                        );
}   /* XFSControlSetAppName () */

LIB_EXPORT
const char * CC
XFSControlGetAppName ( struct XFSControl * self )
{
    return XFSControlGetArg ( self, XFS_CONTROL_APPNAME );
}   /* XFSControlGetAppName () */

LIB_EXPORT
rc_t CC
XFSControlDaemonize ( struct XFSControl * self )
{
    return XFSControlSetArg (
                        self,
                        XFS_CONTROL_DAEMONIZE,
                        XFS_CONTROL_DAEMONIZE
                        );
}   /* XFSControlDaemonize () */

LIB_EXPORT
bool CC
XFSControlIsDaemonize ( struct XFSControl * self )
{
    return XFSControlHasArg ( self, XFS_CONTROL_DAEMONIZE );
}   /* XFSControlIsDaemonize () */

LIB_EXPORT
rc_t CC
XFSControlSetMountPoint (
                    struct XFSControl * self,
                    const char * MountPoint
)
{
    return MountPoint == NULL
                ? XFS_RC ( rcNull )
                : XFSControlSetArg (
                                    self,
                                    XFS_CONTROL_MOUNTPOINT,
                                    MountPoint
                                    )
                ;
}   /* XFSControlSetMountPoint () */

LIB_EXPORT
const char * CC
XFSControlGetMountPoint ( struct XFSControl * self )
{
    return XFSControlGetArg ( self, XFS_CONTROL_MOUNTPOINT );
}   /* XFSControlGetMountPoint () */

LIB_EXPORT
rc_t CC
XFSControlSetLogFile ( struct XFSControl * self, const char * Path )
{
    rc_t RCt;
    char BF [ XFS_SIZE_1024 ];

    RCt = 0;
    * BF = 0;

    XFS_CAN ( self )

    RCt = XFS_ResolvePath (
                        true,
                        BF,
                        sizeof ( BF ),
                        ( Path == NULL ? "./mount-tool.log" : Path )
                        );
    if ( RCt == 0 ) {
        RCt = XFSControlSetArg ( self, XFS_CONTROL_LOGFILE, BF );
    }

    return RCt;
}   /* XFSControlSetLogFile () */

LIB_EXPORT
const char * CC
XFSControlGetLogFile ( struct XFSControl * self )
{
    return XFSControlGetArg ( self, XFS_CONTROL_LOGFILE );
}   /* XFSControlGetLogFile () */

LIB_EXPORT
rc_t CC
XFSControlSetLabel ( struct XFSControl * self, const char * Label )
{
    return Label == NULL
                ? XFS_RC ( rcNull )
                : XFSControlSetArg ( self, XFS_CONTROL_LABEL, Label )
                ;
}   /* XFSControlSetLabel () */

LIB_EXPORT
const char * CC
XFSControlGetLabel ( struct XFSControl * self )
{
    return XFSControlGetArg ( self, XFS_CONTROL_LABEL );
}   /* XFSControlGetLabel () */

static
rc_t
XFSVeryMainLoop ( void * Data )
{
    struct XFSControl * TheControl;

    TheControl = ( struct XFSControl * ) Data;

    if ( TheControl == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( TheControl -> vt == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( TheControl -> vt -> v1.loop == NULL ) {
        return XFS_RC ( rcNull );
    }

    return TheControl -> vt -> v1.loop ( TheControl );
}   /* XFSVeryMainLoop () */
