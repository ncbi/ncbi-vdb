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
#include <klib/log.h>
#include <kproc/thread.h>
#include <xfs/xfs.h>
#include <xfs/xlog.h>

#include "xfs-priv.h"
#include "platform.h"

#include <sysalloc.h>
#include <stdlib.h>
#include <stdio.h>

#include <Shlwapi.h>

/*  Some platform dependent headers
 */

#include "operations.h"


/*  Some useless pranks
 */

XFS_EXTERN rc_t CC XFSSecurityInit ();
XFS_EXTERN rc_t CC XFSSecurityDeinit ();
XFS_EXTERN rc_t CC wLogMsg ( KLogLevel Level, LPCWSTR Format, ... );

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

    LogMsg ( klogDebug, "XFS_DOKAN_init()" );

        /*) Standard checks
         (*/
    if ( self -> Control != NULL ) {
        LogMsg ( klogDebug, "XFS_DOKAN_init(): control is not empty" );
        return XFS_RC ( rcUnexpected );
    }

    if ( self -> Arguments == NULL ) {
        LogMsg ( klogDebug, "XFS_DOKAN_init(): arguments are empty" );
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

    LogMsg ( klogDebug, "XFS_DOKAN_destroy()" );

    if ( self == NULL ) { 
        LogMsg ( klogDebug, "XFS_DOKAN_destroy(): NULL self passed" );

        return XFS_RC ( rcNull );
    }

    Options = ( PDOKAN_OPTIONS ) self -> Control;

    if ( Options == NULL ) {
        LogMsg ( klogDebug, "XFS_DOKAN_destroy(): options are empty" );
    }
    else {
        if ( Options -> MountPoint != NULL ) {
            free ( ( char * ) Options -> MountPoint );
            Options -> MountPoint = NULL;
        }

        free ( Options );
        self -> Control = NULL;
    }

    return 0;
}   /* XFS_DOKAN_destroy() */

static
rc_t CC
_InitDOKAN_OPERATIONS ( DOKAN_OPERATIONS ** Operations )
{
    rc_t RCt;
    DOKAN_OPERATIONS * RetOp;

    RCt = 0;
    RetOp = NULL;

    XFS_CSAN ( Operations )
    XFS_CAN ( Operations )

    RetOp = calloc ( 1, sizeof ( DOKAN_OPERATIONS ) );
    if ( RetOp == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFS_Private_InitOperations ( RetOp );
        if ( RCt == 0 ) {
            * Operations = RetOp;
        }
    }

    if ( RCt != 0 ) {
        * Operations = NULL;

        if ( RetOp != NULL ) {
            free ( RetOp );
        }
    }

    return RCt;
}   /* _InitDOKAN_OPERATIONS () */

XFS_EXTERN rc_t CC XFSPathInnerToNative (
                                WCHAR * NativePathBuffer,
                                size_t NativePathBufferSize,
                                const char * InnerPath,
                                ...
                                );

static
rc_t CC
_MakeMountPath ( const char * Inner, const WCHAR ** MountPath )
{
    rc_t RCt;
    WCHAR BF [ XFS_SIZE_64 ];
    WCHAR * Path;
    size_t SZ;

    RCt = 0;
    * BF = 0;
    Path = NULL;
    SZ = 0;

    XFS_CSAN ( MountPath )
    XFS_CAN ( Inner )
    XFS_CAN ( MountPath )

    RCt = XFSPathInnerToNative ( BF, sizeof ( BF ), Inner );
    if ( RCt == 0 ) {
        SZ = wcslen ( BF );
        if ( BF [ SZ - 1 ] == L'\\' ) {
            BF [ SZ - 1 ] = 0;
            SZ --;
        }

        Path = calloc ( SZ + 1, sizeof ( WCHAR ) );
        if ( Path == NULL ) {
            RCt = XFS_RC ( rcExhausted );
        }
        else {
            wcscpy ( Path, BF );

            * MountPath = Path;
        }
    }

    if ( RCt != 0 ) {
        * MountPath = NULL;

        if ( Path != NULL ) {
            free ( Path );
        }
    }

    return RCt;
}   /* _MakeMountPath () */

rc_t
XFS_DOKAN_mount_v1( struct XFSControl * self )
{
    rc_t RCt;
    DOKAN_OPTIONS * Options;

    RCt = 0;
    Options = NULL;

    LogMsg ( klogDebug, "XFS_DOKAN_mount()" );

    if ( self == NULL ) {
        LogMsg ( klogDebug, "ZERO self passed" );
        return XFS_RC ( rcNull );
    }

    if ( ( RCt = XFSSecurityInit () ) != 0 ) {
        LogMsg ( klogDebug, "Can not initialize DOKAN security" );
        return RCt;
    }

        /*) Here we are allocating DOKAN options and it's global context
         (*/
    Options = calloc ( 1, sizeof ( DOKAN_OPTIONS ) );
    if ( Options == NULL ) {
        RCt = XFS_RC ( rcNull );
    }
    else {

        Options -> Version = DOKAN_VERSION;
        Options -> ThreadCount = 0; /* Default Value */
        Options -> Options = 0L;
        Options -> Options |= DOKAN_OPTION_KEEP_ALIVE;
        Options -> Options |= DOKAN_OPTION_DEBUG;
            /*) using Peer as GlobalContext as for FUSE implementation
             (*/
        Options -> GlobalContext = ( ULONG64 )( self -> TreeDepot );

        RCt = _MakeMountPath (
                            XFSControlGetMountPoint ( self ),
                            & ( Options -> MountPoint )
                            );
        if ( RCt == 0 ) {
            if ( PathFileExistsW ( Options -> MountPoint ) == TRUE ) {
                wLogMsg ( klogFatal, L"Mount point in use [%s]\n", Options -> MountPoint );
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                self -> Control = Options;
            }
        }
    }

    if ( RCt != 0 ) {
        self -> Control = NULL;

        if ( Options != NULL ) {
            if ( Options -> MountPoint != NULL ) {\
                free ( ( char * ) Options -> MountPoint );
                Options -> MountPoint = NULL;
            }
            free ( Options );
        }
    }

    return RCt;
}   /* XFS_DOKAN_mount() */

rc_t
XFS_DOKAN_loop_v1( struct XFSControl * self )
{
    rc_t RCt;
    DOKAN_OPTIONS * Options;
    DOKAN_OPERATIONS * Operations;
    const struct XFSTree * Tree;

    RCt = 0;
    Operations = NULL;
    Options = NULL;
    Tree = NULL;

    LogMsg ( klogDebug, "XFS_DOKAN_loop()" );

    if ( self == NULL ) {
        LogMsg ( klogDebug, "XFSControl: ZERO self passed" );
        return XFS_RC ( rcNull );
    }

    if ( self -> TreeDepot == NULL ) {
        LogMsg ( klogDebug, "XFSControl: ZERO passed" );
        return XFS_RC ( rcNull );
    }

    RCt = XFSControlGetTree ( self, & Tree );
    if ( RCt != 0 || Tree == NULL ) {
        LogMsg ( klogDebug, "XFSControl: ZERO Tree DATA passed" );
        return XFS_RC ( rcNull );
    }

    Options = ( DOKAN_OPTIONS * ) self -> Control;
    if ( Options == NULL ) {
        LogMsg ( klogDebug, "XFSControl: ZERO options passed" );
        return XFS_RC ( rcNull );
    }

pLogMsg ( klogDebug, "XFS_DOKAN_loop(): Tree [$(tree)] Data [$(data)]\n",  "tree=%p,data=%p", self -> TreeDepot, Tree );


/*  We will split mount method for mount'n'loop later, so there is 
    usual routine stuff
*/

    RCt = _InitDOKAN_OPERATIONS ( & Operations );
    if ( RCt == 0 ) {
            /*)
             /  There we are running DokanMain
            (*/
        switch ( DokanMain ( Options, Operations ) ) {
            case DOKAN_SUCCESS :
                LogMsg ( klogDebug, "DokanMain() : general success" );
                break;
            case DOKAN_ERROR :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : general error" );
                break;
            case DOKAN_DRIVE_LETTER_ERROR :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : bad drive letter" );
                break;
            case DOKAN_DRIVER_INSTALL_ERROR :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : can't install driver" );
                break;
            case DOKAN_START_ERROR :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : can't start, something wrong" );
                break;
            case DOKAN_MOUNT_ERROR :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : can't assigh a drive letter or mount point" );
                break;
            case DOKAN_MOUNT_POINT_ERROR :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : mount point is invalid" );
                break;
            default :
                RCt = XFS_RC ( rcError );
                LogErr ( klogDebug, RCt, "DokanMain() : something wrong happens" );
                break;
        }

        free ( Operations );
    }

pLogMsg ( klogDebug, "XFS_DOKAN_loop(): Exited Tree [$(tree)]", "tree=%p", self -> TreeDepot );

    return RCt;
}   /* XFS_DOKAN_loop() */

rc_t
XFS_DOKAN_unmount_v1( struct XFSControl * self )
{
    rc_t RCt = 0;

    if ( self == NULL ) {
        LogMsg ( klogDebug, "ZERO self passed" );
        /*
        return XFS_RC ( rcNull );
        */
        return 0;
    }

    if ( self -> Control == NULL ) {
        LogMsg ( klogDebug, "ZERO self passed" );
        /*
        return XFS_RC ( rcNull );
        */
        return 0;
    }

    XFSSecurityDeinit ();

    return 0;
}   /* XFS_DOKAN_unmount() */

/********************
 * Something extra
 *************/
static
rc_t CC
_GetProgPath ( WCHAR * Path, DWORD PathSize)
{
    const WCHAR * cP = L"\\Dokan\\DokanLibrary\\dokanctl.exe";

        /* First we are trying %ProgramFiles(x86)%
         */
    if ( GetEnvironmentVariableW ( L"%ProgramFiles(x86)%", Path, PathSize ) == 0 ) {
        wcscat ( Path, cP );
        if ( PathFileExistsW ( Path ) == TRUE ) {
            return 0;
        }
    }

        /* First we are trying %ProgramFiles%
         */
    if ( GetEnvironmentVariableW ( L"%ProgramFiles%", Path, PathSize ) == 0 ) {
        wcscat ( Path, cP );
        if ( PathFileExistsW ( Path ) == TRUE ) {
            return 0;
        }
    }

    wcscpy_s (
        Path,
        PathSize,
        L"C:\\Program Files (x86)\\Dokan\\DokanLibrary\\dokanctl.exe"
        );
    return PathFileExistsW ( Path ) == TRUE ? 0 : XFS_RC ( rcNotFound );
}   /* _GetProgPath () */

/*))    Special platform dependent method
  ||    very specific method. It is looking for
  ||        %ProgramFiles(x86)%\Dokan\DokanLibrary\dokanctl.exe
  ||    or
  ||        %ProgramFiles%\Dokan\DokanLibrary\dokanctl.exe
  ||    or
  ||        C:\Program Files (x86)\Dokan\DokanLibrary\dokanctl.exe
  ((*/
LIB_EXPORT
rc_t CC
XFSUnmountAndDestroy ( const char * MountPoint )
{
    rc_t RCt;
    WCHAR Path [ XFS_SIZE_4096 ];
    WCHAR Comm [ XFS_SIZE_4096 ];
    WCHAR * MPath;
    BOOL Ret;
    STARTUPINFO StartInfo;
    PROCESS_INFORMATION Process;
    int Err;

    RCt = 0;
    * Path = 0;
    * Comm = 0;
    MPath = NULL;
    Ret = FALSE;
    ZeroMemory ( & StartInfo, sizeof( StartInfo ) );
    ZeroMemory ( & Process, sizeof( Process ) );
    Err = 0;

    RCt = _MakeMountPath ( MountPoint, & MPath );
    if ( RCt == 0 ) {
        if ( PathFileExistsW ( MPath ) == TRUE ) {
            wLogMsg ( klogInfo, L"Unmounting volume [%s]\n", MPath );

            RCt = _GetProgPath ( Path, sizeof ( Path ) / sizeof ( WCHAR ) );
            if ( RCt == 0 ) {
                swprintf (
                        Comm,
                        sizeof ( Comm ) / sizeof ( WCHAR ),
                        L"\"%s\" /u %s",
                        Path,
                        MPath
                        );
                wLogMsg ( klogInfo, L"Executing [%s]\n", Comm );
                Ret = CreateProcessW (
                                    NULL,
                                    Comm,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    DETACHED_PROCESS,
                                    NULL,
                                    NULL,
                                    & StartInfo,
                                    & Process
                                    );
                if ( Ret == 0 ) {
                    wLogMsg ( klogErr, L"Failed [%s] ErrNo [%d]\n", Comm, GetLastError () );
                }
            }
            else {
                wLogMsg ( klogErr, L"CRITICAL: Can not find 'dokanctl.exe' utility.\n" );
                wLogMsg ( klogErr, L"          Please ask administrator about it location. \n" );
                wLogMsg ( klogErr, L"          Please use command 'dokanctl.exe /u %s'. \n", MPath );
            }
        }
        else {
            wLogMsg ( klogErr, L"Can not find volume [%s]\n", MPath );
        }

        free ( MPath );
    }


    return RCt;
}   /* XFSUnmountAndDestroy () */
