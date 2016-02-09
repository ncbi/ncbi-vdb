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

 /* Something unusual: Lyrics
  *
  * That file contains only one useful function:
  *
  *                       XFS_Private_InitOperations
  *
  * it needed to fill _DOKAN_OPERATIONS structure.
  *
  * I put here all possible stubs to _DOKAN_OPERATIONS
  * functions. However, I will use only several of them while 
  * structure initialisation. So, if You want to extend functionality,
  * please edit already ready stub and add new function to structure
  * initialized .
  *
  */

#include <windows.h>

#include <klib/out.h>
#include <klib/log.h>
#include <klib/namelist.h>
#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfs/impl.h>               /* KDirectoryGetSysDir() */
#include <vfs/path.h>
#include <vfs/manager.h>

#include <xfs/node.h>
#include <xfs/tree.h>
#include <xfs/editors.h>
#include <xfs/handle.h>
#include <xfs/perm.h>
#include <xfs/path.h>

#include <sysalloc.h>

#include <WinBase.h>
#include <WinNT.h>
#include <wchar.h>

#include "operations.h"
#include "zehr.h"
#include "schwarzschraube.h"

/****************************************************************
 * Song over babalula
 ****************************************************************/
#define USE_XFS_DOKAN_CREATEFILE              1   /* - */  /* */
#define USE_XFS_DOKAN_OPENDIRECTORY           1   /* - */  /* */
#define USE_XFS_DOKAN_CREATEDIRECTORY         1   /* - */  /* */
#define USE_XFS_DOKAN_CLEANUP                 1   /* - */  /* */
#define USE_XFS_DOKAN_CLOSEFILE               1   /* - */  /* */
#define USE_XFS_DOKAN_READFILE                1   /* - */  /* */
#define USE_XFS_DOKAN_WRITEFILE               1   /* - */  /* */
#define USE_XFS_DOKAN_FLUSHFILEBUFFERS        0   /* - */  /* */
#define USE_XFS_DOKAN_GETFILEINFORMATION      1   /* - */  /* */
#define USE_XFS_DOKAN_FINDFILES               1   /* - */  /* */
#define USE_XFS_DOKAN_FINDFILESWITHPATTERN    0   /* - */  /* NO NEED */
#define USE_XFS_DOKAN_SETFILEATTRIBUTES       0   /* - */  /* */
#define USE_XFS_DOKAN_SETFILETIME             0   /* - */  /* */
#define USE_XFS_DOKAN_DELETEFILE              1   /* - */  /* */
#define USE_XFS_DOKAN_DELETEDIRECTORY         1   /* - */  /* */
#define USE_XFS_DOKAN_MOVEFILE                1   /* - */  /* */
#define USE_XFS_DOKAN_SETENDOFFILE            0   /* - */  /* */
#define USE_XFS_DOKAN_SETALLOCATIONSIZE       0   /* - */  /* */
#define USE_XFS_DOKAN_LOCKFILE                0   /* - */  /* */
#define USE_XFS_DOKAN_UNLOCKFILE              0   /* - */  /* */
#define USE_XFS_DOKAN_GETDISKFREESPACE        0   /* - */  /* NO NEED */
#define USE_XFS_DOKAN_GETVOLUMEINFORMATION    1   /* - */  /* */
#define USE_XFS_DOKAN_UNMOUNT                 1   /* - */  /* */
#define USE_XFS_DOKAN_GETFILESECURITY         1   /* - */  /* */
#define USE_XFS_DOKAN_SETFILESECURITY         1   /* - */  /* */

/*)))
  |||   Operations
  (((*/

/*))
 //
|| is using _DOKAN_OPTIONS structure for that goal. So, there
|| is different way to access for XFSPeer for both libraries
 \\
  (*/

/*))    KLog does not work with WCHAR :D
 ((*/
XFS_EXTERN rc_t CC wLogMsg ( KLogLevel Level, LPCWSTR Format, ... );

/*\
|*| importante protopute
\*/
struct KSysDir;
rc_t KSysDirOSPath (
                const struct KSysDir * self,
                wchar_t * real,
                size_t bsize,
                const char * path,
                va_list args
                );

/*)))
 /// Some common things are here
(((*/
LIB_EXPORT
rc_t CC
XFSPathInnerToNative (
                WCHAR * NativePathBuffer,
                size_t NativePathBufferSize,
                const char * InnerPath,
                ...
)
{
    rc_t RCt;
    KDirectory * Dir;
    struct KSysDir * SysDir;
    va_list VaLsd;

    RCt = 0;
    Dir = NULL;

    if ( InnerPath == NULL || NativePathBuffer == NULL
        || NativePathBufferSize == 0 )
    { 
        return XFS_RC ( rcNull );
    }

    RCt = KDirectoryNativeDir ( & Dir );
    if ( RCt == 0 ) {
        SysDir = KDirectoryGetSysDir(Dir);

        va_start ( VaLsd, InnerPath );
        RCt = KSysDirOSPath (
                                SysDir,
                                NativePathBuffer,
                                NativePathBufferSize,
                                InnerPath,
                                VaLsd
                                );
        va_end ( VaLsd );
    }

    return RCt;
}   /* XFSPathInnerToNative () */

static
rc_t CC
_DOKAN_make_v_path ( LPCWSTR File, VPath ** Path )
{
    rc_t RCt;
    char Buffer [ XFS_SIZE_4096 ];
    char * pChar;
    size_t Size;

    RCt = 0;
    Size = 0;
    pChar = NULL;
    * Buffer = 0;

    if ( File == NULL || Path == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Path = NULL;

        /* Quite stupid, but I don't know what to do
         */
    RCt = wcstombs_s (
                    & Size,
                    Buffer,
                    sizeof ( Buffer ),
                    File,
                    wcslen ( File )
                    );

    if ( RCt == 0 ) {
            /* That is kinda stupid, and I should think about it
             */
        pChar = Buffer;
        while ( * pChar != 0 ) {
            if ( * pChar == '\\' ) {
                * pChar = '/';
            }
            pChar ++;
        }

        RCt = VFSManagerMakePath ( XFS_VfsManager (), Path, Buffer );
    }

    return RCt;
}   /* _DOKAN_make_v_path () */

static
rc_t CC
_DOKAN_get_node (
                const PDOKAN_FILE_INFO FileInfo,
                const struct VPath * Path,
                const struct XFSNode ** TheNode
)
{
    struct XFSTreeDepot * Depot;
    const struct XFSNode * Node;
    rc_t RCt;

    Depot = NULL;
    RCt = 0;

    if ( FileInfo == NULL || TheNode == NULL ) {
        return XFS_RC ( rcNull );
    } 

    * TheNode = NULL;

    Depot = ( struct XFSTreeDepot * ) FileInfo -> DokanOptions -> GlobalContext;
    if ( Depot == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSTreeDepotFindNodeForPath ( Depot, Path, & Node );

    if ( RCt == 0 ) {
        * TheNode = Node;
    }

/*
//TT wLogMsg ( klogDebug, L"_DOKAB_get_node () TreeDepot [0x%p] Node [0x%p] Opts [0x%p]\n", TreeDepot, Node, FileInfo -> DokanOptions );
*/

    return RCt;
}   /* _DOKAN_get_node () */

static
rc_t CC
_DOKAN_get_path_and_node (
                        LPCWSTR File,
                        const PDOKAN_FILE_INFO Info,
                        const struct VPath ** Path,
                        const struct XFSNode ** Node,
                        XFSNType * NodeType
)
{
    rc_t RCt;
    struct XFSNode * RNode;
    struct VPath * RPath;
    XFSNType Type;
    const struct XFSAttrEditor * Ediotr;

    RCt = 0;
    RNode = NULL;
    RPath = NULL;
    Type = kxfsBadPath;
    Ediotr = NULL;

    if ( File == NULL || Info == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _DOKAN_make_v_path ( File, & RPath );
    if ( RCt == 0 ) {

        RCt = _DOKAN_get_node ( Info, RPath, & RNode );
        if ( RCt == 0 ) {
            if ( NodeType != NULL ) {
                RCt = XFSNodeAttrEditor ( RNode, & Ediotr );
                if ( RCt == 0 ) {
                    RCt = XFSAttrEditorType ( Ediotr, & Type );

                    XFSEditorDispose ( & ( Ediotr -> Papahen ) );
                }
            }
        }
    }

    if ( RCt == 0 ) {
        if ( Path != NULL ) {
            * Path = RPath;
        }
        else {
            VPathRelease ( RPath );
        }

        if ( Node != NULL ) {
            * Node = RNode;
        }
        else {
            XFSNodeRelease ( RNode );
        }

        if ( NodeType != NULL ) {
            * NodeType = Type;
        }
    }
    else {
        if ( RPath != NULL ) {
            VPathRelease ( RPath );
        }

        if ( RNode != NULL ) {
            XFSNodeRelease ( RNode );
        }
    }

    return RCt;
}   /* _DOKAN_get_path_and_node () */

static
rc_t
_DOKAN_get_parent_node (
                const struct VPath * Path,
                DOKAN_FILE_INFO * TheFileInfo,  /* Depot source */
                struct XFSNode ** Node,         /* Ret node */
                XFSNType * Type,                /* could be NULL */
                char ** NodeName                /* could be NULL */
)
{
    rc_t RCt;
    struct XFSTreeDepot * Depot;
    char BB [ XFS_SIZE_4096 ];
    struct XFSPath * xPath;
    uint32_t xPathQ;
    struct XFSNode * xNode;
    struct XFSAttrEditor * xEditor;
    char * xName;
    const struct XFSPath * xParent;

    RCt = 0;
    Depot = NULL;
    * BB = 0;
    xPath = NULL;
    xPathQ = 0;
    xNode = NULL;
    xEditor = NULL;
    xName = NULL;
    xParent = NULL;

    if ( Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    if ( Path == NULL || TheFileInfo == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Type != NULL ) {
        * Type = kxfsNotFound;
    }

    if ( NodeName != NULL ) {
        * NodeName = NULL;
    }

        /*) First we should retrieve fresh instance of depot
         (*/
    Depot = ( struct XFSTreeDepot * ) TheFileInfo -> DokanOptions -> GlobalContext;
    if ( Depot == NULL ) {
        return XFS_RC ( rcInvalid );
    }

        /*) Reading VPath
         (*/
    RCt = XFS_ReadVPath_ZHR ( Path, BB, sizeof ( BB ), "" );
    if ( RCt == 0 ) {
            /*) Making XFSPath
             (*/
        RCt = XFSPathMake ( & xPath, true, BB );
        if ( RCt == 0 ) {
            xPathQ = XFSPathPartCount ( xPath );
            if ( xPathQ < 2 ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                    /*) Here we are composing parent path
                     (*/
                RCt = XFSPathParent ( xPath, & xParent );
                if ( RCt == 0 ) {
                        /*) Here we are looking for NODE
                         (*/
                    RCt = XFSTreeDepotFindNode (
                                                Depot,
                                                XFSPathGet ( xParent ),
                                                & xNode
                                                );
                    if ( RCt == 0 ) {
                        if ( Type != NULL ) {
                            RCt = XFSNodeAttrEditor ( xNode, & xEditor );
                            if ( RCt == 0 ) {
                                RCt = XFSAttrEditorType ( xEditor, Type );
                                XFSEditorDispose ( & ( xEditor -> Papahen ) );
                            }
                        }

                        if ( RCt == 0 ) {
                            if ( NodeName != NULL ) {
                                RCt = XFS_StrDup ( 
                                                XFSPathName ( xPath ),
                                                & xName
                                                );
                                if ( RCt == 0 ) {
                                    * NodeName = xName;
                                }
                            }
                        }

                        if ( RCt == 0 ) {
                            * Node = xNode;
                        }
                    }
                }

                XFSPathRelease ( xParent );
            }

            XFSPathRelease ( xPath );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;
        if ( Type != NULL ) {
            * Type = kxfsNotFound;
        }
        if ( NodeName != NULL ) {
            * NodeName = NULL;
        }
        if ( xNode != NULL ) {
            XFSNodeRelease ( xNode );
        }
        if ( xName != NULL ) {
            free ( xName );
        }
    }

    return RCt;
}   /* _DOKAN_get_parent_node () */

static
rc_t
_DOKAN_get_parent_node_from_char (
                LPCWSTR ThePath,
                DOKAN_FILE_INFO * TheFileInfo,  /* Depot source */
                struct XFSNode ** Node,         /* Ret node */
                XFSNType * Type,                /* could be NULL */
                char ** NodeName                /* could be NULL */
)
{
    rc_t RCt;
    struct VPath * Path;

    RCt = 0;
    Path = NULL;

    if ( ThePath == NULL || TheFileInfo == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    RCt = _DOKAN_make_v_path ( ThePath, & Path );
    if ( RCt == 0 ) {
        RCt = _DOKAN_get_parent_node (
                                    Path,
                                    TheFileInfo,
                                    Node,
                                    Type,
                                    NodeName
                                    );

        VPathRelease ( Path );
    }

    return RCt;
}   /* _DOKAN_get_parent_node_from_char() */

static
rc_t
_DOKAN_delete_file_dir (
                    LPCWSTR FileName,
                    PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSNode * Node;
    XFSNType Type;
    char * Name;
    struct XFSDirEditor * Editor;

    RCt = 0;
    Node = NULL;
    Type = kxfsNotFound;
    Name = NULL;
    Editor = NULL;

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _DOKAN_get_parent_node_from_char (
                                        FileName,
                                        TheFileInfo,
                                        & Node,
                                        & Type,
                                        & Name
                                        );
    if ( RCt == 0 ) {
        RCt = XFSNodeDirEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSDirEditorDelete ( Editor, Name );

            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }

        XFSNodeRelease ( Node );
        free ( Name );
    }

    return 0;
}   /* DOKAN_delete_file_dir () */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_CREATEFILE == 1

static
void
__PrintDisposition__ ( LPCWSTR FileName, DWORD CreationDisposition )
{
    WCHAR BF [ XFS_SIZE_4096 ];

    swprintf (
            BF,
            sizeof ( BF ) / sizeof ( WCHAR ),
            L"Disposition [%08x][%s]:",
            CreationDisposition ,
            FileName
            );

    switch ( CreationDisposition ) {
        case CREATE_NEW :        wcscat ( BF, L" CREATE_NEW" ); break;
        case OPEN_ALWAYS :       wcscat ( BF, L" OPEN_ALWAYS" ); break;
        case CREATE_ALWAYS :     wcscat ( BF, L" CREATE_ALWAYS" ); break;
        case OPEN_EXISTING :     wcscat ( BF, L" OPEN_EXISTING" ); break;
        case TRUNCATE_EXISTING : wcscat ( BF, L" TRUNCATE_EXISTING" ); break;
        default:                 wcscat ( BF, L" UNKNOWN" ); break;
    }

    wLogMsg ( klogDebug, L"%s (%d)\n", BF, CreationDisposition );
}   /* __PrintDisposition__ () */

static
void 
__PrintAccessMode__ ( LPCWSTR FileName, DWORD AccessMode )
{
    int llp, klf;
    WCHAR BF [ XFS_SIZE_4096 ];

    swprintf ( BF, sizeof ( BF ) / sizeof ( WCHAR ), L"AccessMode [%08x][%s]:", AccessMode, FileName );

    for ( llp = 31; 0 <= llp; llp -- ) {
        klf = 1 << llp;
        if ( llp % 8 == 0 ) {
            wcscat ( BF, L" " );
        }

        if ( ( AccessMode & klf ) == klf ) {
            wcscat ( BF, L"1" );
        }
        else {
            wcscat ( BF, L"0" );
        }
    }
    wLogMsg ( klogDebug, L"%s\n", BF );

    swprintf ( BF, sizeof ( BF ) / sizeof ( WCHAR ), L"AccessMode [%08xl][%s]:", AccessMode, FileName );
    if ( ( AccessMode & FILE_READ_DATA ) == FILE_READ_DATA ) wcscat ( BF, L" FILE_READ_DATA" );
    if ( ( AccessMode & FILE_WRITE_DATA ) == FILE_WRITE_DATA ) wcscat ( BF, L" FILE_WRITE_DATA" );
    if ( ( AccessMode & FILE_APPEND_DATA ) == FILE_APPEND_DATA ) wcscat ( BF, L" FILE_APPEND_DATA" );
    if ( ( AccessMode & FILE_READ_EA ) == FILE_READ_EA ) wcscat ( BF, L" FILE_READ_EA" );
    if ( ( AccessMode & FILE_WRITE_EA ) == FILE_WRITE_EA ) wcscat ( BF, L" FILE_WRITE_EA" );
    if ( ( AccessMode & FILE_EXECUTE ) == FILE_EXECUTE ) wcscat ( BF, L" FILE_EXECUTE" );
    if ( ( AccessMode & FILE_DELETE_CHILD ) == FILE_DELETE_CHILD ) wcscat ( BF, L" FILE_DELETE_CHILD" );
    if ( ( AccessMode & FILE_READ_ATTRIBUTES ) == FILE_READ_ATTRIBUTES ) wcscat ( BF, L" FILE_READ_ATTRIBUTES" );
    if ( ( AccessMode & FILE_WRITE_ATTRIBUTES ) == FILE_WRITE_ATTRIBUTES ) wcscat ( BF, L" FILE_WRITE_ATTRIBUTES" );
    if ( ( AccessMode & FILE_ALL_ACCESS ) == FILE_ALL_ACCESS ) wcscat ( BF, L" FILE_ALL_ACCESS" );
    if ( ( AccessMode & READ_CONTROL ) == READ_CONTROL ) wcscat ( BF, L" READ_CONTROL" );
    if ( ( AccessMode & DELETE ) == DELETE ) wcscat ( BF, L" DELETE" );

    wLogMsg ( klogDebug, L"%s\n", BF );
}   /* __PrintAccessMode__ () */

static
int
_HandleForNode (
            const struct XFSNode * Node,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    int RetVal;
    struct XFSHandle * Handle;

    RetVal = 0;

    if ( Node == NULL ) {
        return ERROR_INVALID_DATA;
    }

    if ( TheFileInfo == NULL ) {
        RetVal = ERROR_INVALID_DATA;
    } else {
        if ( XFSHandleMake ( Node, & Handle ) != 0 ) {
            RetVal = ERROR_INVALID_DATA;
        }
        else {
            TheFileInfo -> Context = ( ULONG ) Handle;

            RetVal = 0;
        }
    }

    XFSNodeRelease ( Node );

    return RetVal;
}   /* _HandleForNode () */

static
int
_HandleOpenExistingFileEdit (
                    const struct XFSNode * Node,
                    PDOKAN_FILE_INFO TheFileInfo,
                    bool Write,
                    bool Read
)
{
    struct XFSFileEditor * Editor;
    rc_t RCt;
    XFSNMode Mode;
    struct XFSHandle * Handle;
    int RetVal;

    Editor = NULL;
    Mode = 0;
    Handle = NULL;
    RCt= 0;
    RetVal = 0;

    if ( Node == NULL ) {
        return ERROR_INVALID_DATA;
    }

    if ( TheFileInfo == NULL ) {
        XFSNodeRelease ( Node );

        return ERROR_INVALID_DATA;
    }

    if ( Write ) {
        Mode |= kxfsWrite;
    }

    if ( Read ) {
        Mode |= kxfsRead;
    }

    RCt = XFSNodeFileEditor ( Node, & Editor );
    if ( RCt == 0 ) {
        RCt = XFSFileEditorOpen ( Editor, Mode );
        if ( RCt == 0 ) {
            RCt = XFSHandleMake ( Node, & Handle );
            if ( RCt == 0 ) {
                XFSHandleSet ( Handle, Editor );

                TheFileInfo -> Context = ( ULONG ) Handle;
            }
        }
    }

    if ( RCt != 0 ) {
        if ( Editor != NULL ) {
            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }
    }

    XFSNodeRelease ( Node );

    if ( RCt == 0 ) {
        RetVal = 0;
    }
    else {
        if ( RCt == XFS_RC ( rcBusy ) ) {
            RetVal = ERROR_PATH_BUSY;
        }
        else {
            RetVal = ERROR_ACCESS_DENIED;
        }
    }

    return RetVal;
}   /* _HandleOpenExistingFileEdit () */

static
int
_HandleCreateNewFileEdit (
                        const struct VPath * Path,
                        DOKAN_FILE_INFO * TheFileInfo,
                        bool Write,
                        bool Read
)
{
    rc_t RCt;
    struct XFSNode * Node;
    struct XFSDirEditor * DirEditor;
    struct XFSHandle * Handle;
    char * NodeName;
    XFSNType Type;
    XFSNMode Mode;

    RCt = 0;
    Node = NULL;
    DirEditor = NULL;
    Handle = NULL;
    NodeName = NULL;
    Type = kxfsNotFound;
    Mode = kxfsNone;

    if ( TheFileInfo == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheFileInfo -> Context = 0;

    if ( Write ) { Mode |= kxfsWrite; }
    if ( Read ) { Mode |= kxfsRead; }

    RCt = _DOKAN_get_parent_node (
                                Path,
                                TheFileInfo,
                                & Node,
                                & Type,
                                & NodeName
                                );
    if ( RCt == 0 ) {
            /* I do not check if node exists or not,
             * cuz Bogus node will fail on retrieving
             * DirEditor
             */
        RCt = XFSNodeDirEditor ( Node, & DirEditor );
        if ( RCt == 0 ) {
            RCt = XFSDirEditorCreate (
                                    DirEditor,
                                    NodeName,
                                    Mode,
                                    & Handle
                                    );
            if ( RCt == 0 ) {
                TheFileInfo -> Context = ( ULONG ) Handle;
            }
        }

        XFSNodeRelease ( Node );
        free ( NodeName );
    }

    if ( RCt != 0 ) {
        if ( DirEditor != NULL ) {
            XFSEditorDispose ( & ( DirEditor -> Papahen ) );
        }
    }

    return RCt == 0 ? 0 : ERROR_ACCESS_DENIED;
}   /* _HandleCreateNewFileEdit () */

/*))
 ((     Really strange method, long and stupid.
  ))
 ((*/
static
int DOKAN_CALLBACK
XFS_DOKAN_CreateFile (
            LPCWSTR FileName,
            DWORD AccessMode,
            DWORD ShareMode,
            DWORD CreationDisposition,
            DWORD FlagsAndAttributes,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    int RetValue;
    struct XFSNode * Node;
    XFSNType Type;
    bool Read, Write;
    VPath * Path;

    RCt = 0;
    RetValue = 0;
    Node = NULL;
    Type = kxfsBadPath;
    Read = Write = false;
    Path = NULL;

#ifdef I_AM_AN_IMBECILE
{
if ( wcsstr ( FileName, klogDebug, L"CVS" ) != NULL
    || wcsstr ( FileName, klogDebug, L".svn" ) != NULL
    || wcsstr ( FileName, klogDebug, L"desktop.ini" ) != NULL
    || wcsstr ( FileName, klogDebug, L".gs" ) != NULL
    || wcsstr ( FileName, klogDebug, L"Authorun.inf" ) != NULL
    ) {
wLogMsg ( klogDebug, L" CREATE File [%s][VYHUHOL]\n", FileName );
    return ERROR_PATH_NOT_FOUND * - 1;
}
}
#endif /* I_AM_AN_IMBECILE */

wLogMsg ( klogDebug, L" CREATE File [%s][I=0x%p][H=0x%p]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return 1 * - 1; /* TODO !!! */
    }
    TheFileInfo -> Context = 0L;

__PrintAccessMode__ ( FileName, AccessMode );
__PrintDisposition__ ( FileName, CreationDisposition );

        /* Awkward attempt to reduce filesystem abuse from
            CVS agen and all other services, like gzip
         */

        /* First we should know what kind of file object do we have
         */
    RCt = _DOKAN_get_path_and_node (
                                FileName,
                                TheFileInfo,
                                & Path,
                                & Node,
                                & Type
                                );
        /* Something really wrong did happen
         */
    if ( RCt != 0 ) {
        return 1 * - 1;
    }

        /* Reading/Writing file
         */
    Read = ( AccessMode & FILE_READ_DATA ) == FILE_READ_DATA;
    Write = ( AccessMode & FILE_WRITE_DATA ) == FILE_WRITE_DATA;

        /* We need only existing file
         */
    if ( Type == kxfsNotFound && CreationDisposition == OPEN_EXISTING ) {
        TheFileInfo -> Context = 0L;

        XFSNodeRelease ( Node );
        VPathRelease ( Path );

        RetValue = ERROR_FILE_NOT_FOUND;

wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=NULL][I=0x%p][%d]\n", FileName, RetValue, TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

        /* We are trying to open new file
         */
    if ( Type == kxfsNotFound && CreationDisposition != OPEN_EXISTING
        && ( Read || Write )
    ) {
        RetValue = _HandleCreateNewFileEdit ( Path, TheFileInfo, Write, Read );

        VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

        /* Reading directory content
         */
    if ( Type == kxfsDir ) {
            /* Reading Directory Listing
             */
        if ( CreationDisposition == OPEN_EXISTING && Read ) {
            RetValue = _HandleForNode ( Node, TheFileInfo );

            VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
            return RetValue * - 1;
        }
    }

        /* Reading/Writin existing file
         */
    if ( ( Read || Write ) && Type != kxfsDir &&  Type != kxfsNotFound && Type != kxfsBadPath ) {
        RetValue = _HandleOpenExistingFileEdit ( Node, TheFileInfo, Write, Read );

        VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

        /* Reading/Writin file attributes
         */
    if ( ( AccessMode & FILE_GENERIC_READ ) == FILE_WRITE_ATTRIBUTES && CreationDisposition == OPEN_EXISTING ) {
        RetValue = _HandleOpenExistingFileEdit ( Node, TheFileInfo, Write, Read );

        VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

        /* Security read ... SACL DACL
         */
    if ( ( AccessMode & READ_CONTROL ) == READ_CONTROL
            && ( ! Read )
            && ( ! Write )
    ) {
        RetValue = _HandleForNode ( Node, TheFileInfo );

        VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

        /* We are reading file attributes, and all other requests
         */
    if ( ( AccessMode & FILE_GENERIC_READ ) == FILE_READ_ATTRIBUTES ) {
        RetValue = _HandleForNode ( Node, TheFileInfo );

        VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

        /* We are deleting file ...
         */
    if ( ( AccessMode & DELETE ) == DELETE ) {
        RetValue = _HandleForNode ( Node, TheFileInfo );

        VPathRelease ( Path );
wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][H=0x%p][I=0x%p][%d]\n", FileName, RetValue, ( void * ) ( TheFileInfo -> Context ), TheFileInfo, __LINE__ );
        return RetValue * - 1;
    }

    /* Something else is here */
    RetValue = 1; // TODO

    XFSNodeRelease ( Node );
    VPathRelease ( Path );

wLogMsg ( klogDebug, L"     RETURNS [%s][RC=%d][I=0x%p][%d]\n", FileName, RetValue, TheFileInfo, __LINE__ );

    return RetValue * - 1;
}   /* CreateFile() */

#endif /* USE_XFS_DOKAN_CREATEFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_OPENDIRECTORY == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_OpenDirectory (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    const struct XFSNode * TheNode;
    XFSNType Type;
    int RetVal;

    RCt = 0;
    TheNode = NULL;
    Type = kxfsNotFound;
    RetVal = 0;

//TT wLogMsg ( klogDebug, L" OPEN directory [%s][I=0x%p][C=0x%p]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return 1 * - 1;
    }
    TheFileInfo -> Context = 0L;

    RCt = _DOKAN_get_path_and_node (
                                FileName,
                                TheFileInfo,
                                NULL,
                                & TheNode,
                                & Type
                                );
    if ( RCt == 0 ) {
        if ( Type == kxfsDir ) {
            RetVal = _HandleForNode ( TheNode, TheFileInfo );
        }
        else {
            RetVal = ERROR_PATH_NOT_FOUND;

            XFSNodeRelease ( TheNode );
        }
    }
    else {
        RetVal = ERROR_INVALID_DATA;
    }

//TT wLogMsg ( klogDebug, L" OPEN directory,cont [%s][I=0x%p][C=0x%p][RC=%lu]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, RetVal );
    return RetVal * - 1;
}   /* OpenDirectory() */

#endif /* USE_XFS_DOKAN_OPENDIRECTORY == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_CREATEDIRECTORY == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_CreateDirectory (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSNode * Node;
    char * Name;
    struct XFSDirEditor * Editor;

    RCt = 0;
    Node = NULL;
    Name = NULL;
    Editor = NULL;

//TT wLogMsg ( klogDebug, L"CREATE Directory [%s][FI=0x%p][H=0x%p]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

    RCt = _DOKAN_get_parent_node_from_char (
                                        FileName,
                                        TheFileInfo,
                                        & Node,
                                        NULL,
                                        & Name
                                        );
    if ( RCt == 0 ) {
        RCt = XFSNodeDirEditor ( Node, & Editor );
        if ( RCt == 0 ) {
            RCt = XFSDirEditorCreateDir ( Editor, Name );

            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }

        XFSNodeRelease ( Node );
        free ( Name );
    }

//TT wLogMsg ( klogDebug, L"   CREATE Directory [%s][FI=0x%p][H=0x%p][RC=%d]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, RCt );

    return RCt == 0 ? 0 : - 1;
}   /* CreateDirectory() */

#endif /* USE_XFS_DOKAN_CREATEDIRECTORY == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_CLEANUP == 1

    /*))
     // We should remember, all files are closing and deleting here
    ((*/
static
int DOKAN_CALLBACK
XFS_DOKAN_Cleanup (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    struct XFSHandle * Handle;
    struct XFSFileEditor * Editor;

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;

    if ( Handle == NULL )  {
        return ERROR_INVALID_HANDLE * - 1;
    }

//TT wLogMsg ( klogDebug, L" CLEANUP File [%s][I=0x%p][H=0x%p][Del=%d]\n", FileName, TheFileInfo, Handle, TheFileInfo -> DeleteOnClose );

    TheFileInfo -> Context = 0L;

    Editor = ( struct XFSFileEditor * ) XFSHandleGet ( Handle );
    if ( Editor != NULL ) {
        /*)) I believe that if ... if here is non-NULL handle, 
         //  it could be only handle for KFile, will change if ...
        ((*/
        XFSFileEditorClose ( Editor );
        XFSEditorDispose ( & ( Editor -> Papahen ) );
        XFSHandleSet ( Handle, NULL );
    }
    XFSHandleRelease ( Handle );

    if ( TheFileInfo -> DeleteOnClose ) {
//TT wLogMsg ( klogDebug, L"     CLEANUP File : DELETE ON CLOSE [%s][I=0x%p][H=0x%p]\n", FileName, TheFileInfo, Handle );
        _DOKAN_delete_file_dir ( FileName, TheFileInfo );

    }

    return 0;
}   /* Cleanup() */

#endif /* USE_XFS_DOKAN_CLEANUP == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_CLOSEFILE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_CloseFile (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSHandle * Handle;
    struct XFSFileEditor * Editor;

    RCt = 0; Handle = NULL;
    Editor = NULL;

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return 1 * - 1;
    }

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;

//TT wLogMsg ( klogDebug, L" CLOSE File [%s][I=0x%p][H=0x%p]\n", FileName, TheFileInfo, Handle );

    if ( Handle == NULL )  {
        /* That's is OK */
        return 0;
    }

//TT wLogMsg ( klogDebug, L" CLOSE File: Cleanup method wasn't called [%s][E=0x%p]\n", FileName, Handle );

    TheFileInfo -> Context = 0L;

    Editor = ( struct XFSFileEditor * ) XFSHandleGet ( Handle );
    if ( Editor != NULL ) {

        XFSFileEditorClose ( Editor );
        XFSEditorDispose ( & ( Editor -> Papahen ) );
        XFSHandleSet ( Handle, NULL );
    }
    XFSHandleRelease ( Handle );

    if ( TheFileInfo -> DeleteOnClose ) {
        RCt = _DOKAN_delete_file_dir ( FileName, TheFileInfo );
    }

//TT wLogMsg ( klogDebug, L" CLOSE File,cont [%s][I=0x%p][H=0x%p][RC=%d]\n", FileName, TheFileInfo, Handle, RCt );

    return ( RCt == 0 ? 0 : ERROR_INVALID_DATA ) * - 1;
}   /* CloseFile() */

#endif /* USE_XFS_DOKAN_CLOSEFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_READFILE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_ReadFile (
            LPCWSTR FileName,
            LPVOID Buffer,
            DWORD NumberOfBytesToRead,
            LPDWORD NumberOfBytesRead,
            LONGLONG Offset,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    struct XFSHandle * Handle;
    bool LocallyOpened;
    struct XFSFileEditor * Editor;
    const struct XFSNode * Node;
    rc_t RCt;
    int RetVal;
    size_t n2r, nRd;

    Handle = NULL;
    LocallyOpened = false;
    Editor = NULL;
    Node = NULL;
    RCt = 0;
    RetVal = 0;
    n2r = nRd = 0;

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return 1 * - 1; /* TODO !!! */
    }

//TT wLogMsg ( klogDebug, L" READ File [%s][I=0x%p][H=0x%p] - [N=%lu][O=%lu]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, NumberOfBytesToRead, Offset );

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;
    n2r = ( size_t ) NumberOfBytesToRead;

        /*)) That's could happen, and we need to reopen fiel
         ((*/
    if ( Handle == NULL ) {
//TT wLogMsg ( klogDebug, L" READ File [%s][I=0x%p][H=0x%p] - REOPENING!\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

            /*)) First we are looking for a node
             ((*/

        RCt = _DOKAN_get_path_and_node (
                                    FileName,
                                    TheFileInfo,
                                    NULL,   /* VPath, no need */
                                    & Node,
                                    NULL    /* NodeType */
                                    );
        if ( RCt == 0 ) {
            RCt = XFSNodeFileEditor ( Node, & Editor );
            if ( RCt == 0 ) {
                RCt = XFSFileEditorOpen ( Editor, kxfsRead );
                if ( RCt == 0 ) {
                    LocallyOpened = true;
                }
            }
        }
    }
    else {
        Editor = ( struct XFSFileEditor * ) XFSHandleGet ( Handle );
        if ( Editor == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            Node = XFSHandleNode ( Handle );
            if ( Node == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }
        }
    }

    if ( RCt == 0 ) {
        /*) Here we are reading info
         (*/
        RCt = XFSFileEditorRead (
                            Editor,
                            Offset,
                            Buffer,
                            n2r,
                            & nRd
                            );
        * NumberOfBytesRead = nRd;
    }

    if ( LocallyOpened ) {
//TT wLogMsg ( klogDebug, L" READ File [%s][I=0x%p][H=0x%p] - RECLOSING!\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );
        if ( Editor != NULL ) {
            XFSFileEditorClose ( Editor );

            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }

        Editor = NULL;

        if ( Node != NULL ) {
            XFSNodeRelease ( Node );

            Node = NULL;
        }
    }

    RetVal =  RCt == 0 ? 0 : ERROR_INVALID_DATA;

//TT wLogMsg ( klogDebug, L" READ File,cont [%s][I=0x%p][H=0x%p] - [Read=%lu][RC=%d]!\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, * NumberOfBytesRead, RetVal );

    return RetVal * - 1;
}   /* ReadFile() */

#endif /* USE_XFS_DOKAN_READFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_WRITEFILE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_WriteFile (
            LPCWSTR FileName,
            LPCVOID Buffer,
            DWORD NumBytesWrite,
            LPDWORD NumBytesWritten,
            LONGLONG Offset,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    struct XFSHandle * Handle;
    bool LocallyOpened;
    struct XFSFileEditor * Editor;
    const struct XFSNode * Node;
    rc_t RCt;
    int RetVal;
    size_t n2w, nWr;

    Handle = NULL;
    LocallyOpened = false;
    Editor = NULL;
    Node = NULL;
    RCt = 0;
    RetVal = 0;
    n2w = nWr = 0;

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return 1 * - 1; /* TODO !!! */
    }

//TT wLogMsg ( klogDebug, L" WRITE File [%s][I=0x%p][H=0x%p] - [O=%d][N=%d]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, (int)Offset, (int)NumBytesWrite );

    n2w = ( size_t ) NumBytesWrite;

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;

        /*)) That's could happen, and we need to reopen fiel
         ((*/
    if ( Handle == NULL ) {
//TT wLogMsg ( klogDebug, L" WRITE File [%s][I=0x%p][H=0x%p] - REOPENING!\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

            /*)) First we are looking for a node
             ((*/

        RCt = _DOKAN_get_path_and_node (
                                    FileName,
                                    TheFileInfo,
                                    NULL,   /* VPath, no need */
                                    & Node,
                                    NULL    /* NodeType */
                                    );
        if ( RCt == 0 ) {
            RCt = XFSNodeFileEditor ( Node, & Editor );
            if ( RCt == 0 ) {
                RCt = XFSFileEditorOpen ( Editor, kxfsWrite );
                if ( RCt == 0 ) {
                    LocallyOpened = true;
                }
            }
        }
    }
    else {
        Editor = ( struct XFSFileEditor * ) XFSHandleGet ( Handle );
        if ( Editor == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            Node = XFSHandleNode ( Handle );
            if ( Node == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }
        }
    }

    if ( RCt == 0 ) {
        /*) Here we are reading info
         (*/
        RCt = XFSFileEditorWrite (
                            Editor,
                            Offset,
                            Buffer,
                            n2w,
                            & nWr
                            );
        * NumBytesWritten = nWr;
    }

    if ( LocallyOpened ) {
//TT wLogMsg ( klogDebug, L" WRITE File [%s][I=0x%p][H=0x%p] - RECLOSING!\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );
        if ( Editor != NULL ) {
            XFSFileEditorClose ( Editor );

            XFSEditorDispose ( & ( Editor -> Papahen ) );
        }

        Editor = NULL;

        if ( Node != NULL ) {
            XFSNodeRelease ( Node );

            Node = NULL;
        }
    }

    RetVal =  RCt == 0 ? 0 : ERROR_INVALID_DATA;

//TT wLogMsg ( klogDebug, L" WRITE File,cont [%s][I=0x%p][H=0x%p] - [Wrote=%d][RC=%d]!\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, (int)* NumBytesWritten, (int)RetVal );

    return RetVal * - 1;
}   /* WriteFile() */

#endif /* USE_XFS_DOKAN_WRITEFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_FLUSHFILEBUFFERS == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_FlushFileBuffers (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"FLUSHFILEBUFFERS(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* FlushFileBuffers() */

#endif /* USE_XFS_DOKAN_FLUSHFILEBUFFERS == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_GETFILEINFORMATION == 1

/*))
 // TODO : introduce global handle class, which will represent all
((*/

static
int
_Read_HANDLE_FILE_INFORMATION (
                        const struct XFSHandle * Handle,
                        LPBY_HANDLE_FILE_INFORMATION HandleFileInfo
)
{
    rc_t RCt;
    int RetVal;
    const struct XFSNode * Node;
    struct XFSAttrEditor * Editor;
    struct XFSFileEditor * FileEditor;
    struct XFSPerm * Perm;
    const char * PermStr;
    XFSNType Type;
    ULONG64 Time;
    KTime_t TheTime;
    DWORD TimeHigh, TimeLow;
    LARGE_INTEGER FileSize;
    uint64_t Size;

    RCt = 0;
    RetVal = 0;
    Node = NULL;
    Editor = NULL;
    Perm = NULL;
    PermStr = NULL;
    Type = kxfsNotFound;

    if ( Handle == NULL || HandleFileInfo == NULL ) {
        return 1;
    }

    Node = XFSHandleNode ( Handle );
    if ( Node == NULL ) {
        return 1;
    }

    RCt = XFSNodeAttrEditor ( Node, & Editor );
    if ( RCt != 0 || Editor == NULL ) {
        return 1;
    }

    while ( true ) {
        ZeroMemory (
                HandleFileInfo,
                sizeof ( BY_HANDLE_FILE_INFORMATION )
                );

            /* File Attributes */
        if ( XFSAttrEditorType ( Editor, & Type ) != 0 ) {
            RetVal = 1;
            break;
        }
        HandleFileInfo -> dwFileAttributes
                                        = FILE_ATTRIBUTE_NORMAL;
        switch ( Type ) {
            case kxfsFile:
                break;
            case kxfsDir:
                HandleFileInfo -> dwFileAttributes
                                        |= FILE_ATTRIBUTE_DIRECTORY;
                break;
            case kxfsLink:
            default:
                HandleFileInfo -> dwFileAttributes
                                        = INVALID_FILE_ATTRIBUTES;
                break;
        }

        if ( HandleFileInfo -> dwFileAttributes != INVALID_FILE_ATTRIBUTES && Type != kxfsDir ) {
/* ### Check permissions */
            if ( XFSAttrEditorPermissions ( Editor, & PermStr ) == 0 ) {
                if ( XFSPermMake ( PermStr, & Perm ) == 0 ) {
                    if ( XFSPermAuth ( Perm, kxfsUser ) != NULL ) {
                        if ( ! XFSAuthCanRead (
                                         XFSPermAuth ( Perm, kxfsUser )
                                         ) ) {
                            HandleFileInfo -> dwFileAttributes
                                            |= FILE_ATTRIBUTE_READONLY;
                        }
                    }
                    free ( Perm );
                }
            }
        }

            /* Times */
        if ( XFSAttrEditorDate ( Editor, & TheTime ) != 0 ) {
            RetVal = 1;
            break;
        }
        Time = ( TheTime * 10000000 ) + 116444736000000000;;
        TimeLow = ( DWORD ) Time;
        TimeHigh = Time >> 32;
        HandleFileInfo -> ftCreationTime.dwLowDateTime = TimeLow;
        HandleFileInfo -> ftCreationTime.dwHighDateTime = TimeHigh;
        HandleFileInfo -> ftLastAccessTime.dwLowDateTime = TimeLow;
        HandleFileInfo -> ftLastAccessTime.dwHighDateTime = TimeHigh;
        HandleFileInfo -> ftLastWriteTime.dwLowDateTime = TimeLow;
        HandleFileInfo -> ftLastWriteTime.dwHighDateTime = TimeHigh;

            /* Sizes: set default value and try to get some */
        FileSize.QuadPart = 0;
        HandleFileInfo -> nFileSizeHigh = FileSize.HighPart;
        HandleFileInfo -> nFileSizeLow = FileSize.LowPart;

        FileEditor = ( struct XFSFileEditor * ) XFSHandleGet ( Handle );
        if ( FileEditor == NULL ) {
            RCt = XFSNodeFileEditor ( Node, & FileEditor );
            if ( RCt == 0 ) {
                if ( FileEditor != NULL ) {
                    if ( XFSFileEditorSize ( FileEditor, & Size ) == 0 ) {
                        FileSize.QuadPart = Size;
                        HandleFileInfo -> nFileSizeHigh = FileSize.HighPart;
                        HandleFileInfo -> nFileSizeLow = FileSize.LowPart;
                    }

                    XFSEditorDispose ( & ( FileEditor -> Papahen ) );
                }
            }
        } else {
            if ( XFSFileEditorSize ( FileEditor, & Size ) == 0 ) {
                FileSize.QuadPart = Size;
                HandleFileInfo -> nFileSizeHigh = FileSize.HighPart;
                HandleFileInfo -> nFileSizeLow = FileSize.LowPart;
            }
        }

        break;
    }

    XFSEditorDispose ( & ( Editor -> Papahen ) );

    return RetVal;
}   /* _Read_HANDLE_FILE_INFORMATION () */

static
int DOKAN_CALLBACK
XFS_DOKAN_GetFileInformation (
            LPCWSTR FileName,
            LPBY_HANDLE_FILE_INFORMATION HandleFileInfo,
            PDOKAN_FILE_INFO FileInfo
)
{
    struct XFSHandle * Handle;
    int RetValue;

    RetValue = 0;
    Handle = NULL;

//TT wLogMsg ( klogDebug, L" INFO file [%s][I=0x%p][H=0x%p]\n", FileName, FileInfo, (void * ) FileInfo -> Context );

    if ( FileName == NULL || HandleFileInfo == NULL || FileInfo == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }

    Handle = ( struct XFSHandle * ) FileInfo -> Context;

    if ( Handle == NULL )  {
        return ERROR_INVALID_HANDLE * - 1;
    }

    RetValue = _Read_HANDLE_FILE_INFORMATION ( Handle, HandleFileInfo );

//TT wLogMsg ( klogDebug, L" INFO File,cont [%s][0x%p][RV=%d]\n", FileName, FileInfo, RetValue );

    return RetValue * - 1;
}   /* GetFileInformation() */

#endif /* USE_XFS_DOKAN_GETFILEINFORMATION == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_FINDFILES == 1

static
rc_t CC
_Read_PWIN32_FIND_DATA (
                const struct XFSNode * Node,
                LPWIN32_FIND_DATAW FindData
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;
    struct XFSFileEditor * FileEditor;
    struct XFSPerm * Perm;
    const char * PermStr;
    ULONG64 Time;
    XFSNType Type;
    KTime_t TheTime;
    DWORD TimeHigh, TimeLow;
    LARGE_INTEGER FileSize;
    uint64_t Size;
    size_t CopyNum;

    RCt = 0;
    Editor = NULL;
    Perm = NULL;
    PermStr = NULL;
    Type = kxfsNotFound;
    CopyNum = 0;

    if ( Node == NULL || FindData == NULL ) {
        return XFS_RC ( rcNull );
    }

    ZeroMemory ( FindData, sizeof ( WIN32_FIND_DATAW ) );


    RCt = XFSNodeAttrEditor ( Node, & Editor );
    if ( RCt != 0 || Editor == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( XFSAttrEditorType ( Editor, & Type ) != 0 ) {
        XFSEditorDispose ( & ( Editor -> Papahen ) );

        return XFS_RC ( rcInvalid );
    }

    FindData -> dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

    switch ( Type ) {
        case kxfsFile:
            break;
        case kxfsDir:
            FindData -> dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
            break;
        case kxfsLink:
        default:
            FindData -> dwFileAttributes = INVALID_FILE_ATTRIBUTES;
            break;
    }

    if ( FindData -> dwFileAttributes != INVALID_FILE_ATTRIBUTES && Type != kxfsDir ) {
/* ### Check permissions */
        RCt = XFSAttrEditorPermissions ( Editor, & PermStr );
        if ( RCt == 0 ) {
            RCt = XFSPermMake ( PermStr, & Perm );
            if ( RCt == 0 ) {
                if ( XFSPermAuth ( Perm, kxfsUser ) == NULL ) {
                    RCt = XFS_RC ( rcInvalid );
                }
                free ( Perm );
            }
        }
    }

    if ( RCt != 0 ) {
        XFSEditorDispose ( & ( Editor -> Papahen ) );
        return RCt;
    }

    RCt = XFSAttrEditorDate ( Editor, & TheTime );
    XFSEditorDispose ( & ( Editor -> Papahen ) );
    if ( RCt != 0 ) {
        return RCt;
    }
    Time = ( TheTime * 10000000 ) + 116444736000000000;;
    TimeLow = ( DWORD ) Time;
    TimeHigh = Time >> 32;

    FindData -> ftCreationTime.dwLowDateTime = TimeLow;
    FindData -> ftCreationTime.dwHighDateTime = TimeHigh;
    FindData -> ftLastAccessTime.dwLowDateTime = TimeLow;
    FindData -> ftLastAccessTime.dwHighDateTime = TimeHigh;
    FindData -> ftLastWriteTime.dwLowDateTime = TimeLow;
    FindData -> ftLastWriteTime.dwHighDateTime = TimeHigh;

    FileSize.QuadPart = 0;
    RCt = XFSNodeFileEditor ( Node, & FileEditor );
    if ( RCt == 0 ) {
        if ( FileEditor != NULL ) {
            RCt = XFSFileEditorSize ( FileEditor, & Size );
            if ( RCt == 0 ) {
                FileSize.QuadPart = Size;
            }
            XFSEditorDispose ( & ( FileEditor -> Papahen ) );
        }
    }
    if ( RCt != 0 ) {
        RCt = 0;
    }
    FindData -> nFileSizeHigh = FileSize.HighPart;
    FindData -> nFileSizeLow = FileSize.LowPart;

        /*))
         // And here is it ... dances with schimpanami
        ((*/
    if ( mbstowcs_s (
                    & CopyNum,
                    FindData -> cFileName,
                    MAX_PATH,
                    Node -> Name,
                    string_size ( Node -> Name )
                    ) != 0 ) {
        RCt = XFS_RC ( rcInvalid );
    }

    if ( RCt == 0 ) {
        GetShortPathNameW (
                        FindData -> cFileName,
                        FindData -> cAlternateFileName,
                        sizeof ( FindData -> cAlternateFileName )
                                                    / sizeof ( WCHAR )
                        );
    }

    return RCt;
}   /* _Read_PWIN32_FIND_DATA () */
       
static
rc_t CC
_FindDataForFile (
                const struct XFSDirEditor * Editor,
                const char * FileName,
                LPWIN32_FIND_DATAW FindData
)
{
    rc_t RCt;
    struct XFSNode * Child;

    RCt = 0;
    Child = NULL;

    if ( Editor == NULL || FileName == NULL || FindData == NULL ) {
        return XFS_RC ( rcNull );
    }

        /*) First we are looking for child node
         (*/
    RCt = XFSDirEditorFind ( Editor, FileName, & Child );
    if ( RCt == 0 ) {
            /*) Second we are reading data for child node
             (*/
        RCt = _Read_PWIN32_FIND_DATA ( Child, FindData );
    }

    return RCt;
}   /* _FindDataForFile () */

static
int DOKAN_CALLBACK
XFS_DOKAN_FindFiles (
            LPCWSTR PathName,
            PFillFindData FindDataCallback,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSHandle * Handle;
    const struct XFSDirEditor * Editor;
    const struct XFSNode * Node;
    struct KNamelist * List;
    uint32_t ListQty, llp;
    const char * Name;
    WIN32_FIND_DATAW FindData;
    int RetVal;

    RCt = 0;
    Handle = NULL;
    Editor = NULL;
    Node = NULL;
    List = NULL;
    ListQty = llp = 0;
    Name = NULL;
    RetVal = 0;

    if ( PathName == NULL || FindDataCallback == NULL
        || TheFileInfo == NULL
    ) {
        return ERROR_INVALID_DATA * - 1;
    }

//TT wLogMsg ( klogDebug, L" FIND Files [%s][0x%p]\n", PathName, TheFileInfo );

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;
    if ( Handle == NULL ) {
        return ERROR_INVALID_HANDLE * - 1;
    }

    Node = XFSHandleNode ( Handle );
    if ( Node == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }

    if ( XFSNodeDirEditor ( Node, & Editor ) != 0 ) {
        return ERROR_INVALID_DATA * - 1;
    }

    if ( Editor == 0 ) {
        return ERROR_INVALID_FUNCTION * - 1;
    }

    if ( XFSDirEditorList ( Editor, & List ) == 0 ) {
        if ( KNamelistCount ( List, & ListQty ) == 0 ) {
            for ( llp = 0; llp < ListQty; llp ++ ) {
                RCt = KNamelistGet ( List, llp, & Name );
                if ( RCt == 0 ) {
                    RCt = _FindDataForFile ( Editor, Name, & FindData );
                    if ( RCt == 0 ) {
                        FindDataCallback ( & FindData, TheFileInfo );
                    }
                }

                if ( RCt != 0 ) {
/* Do we need that? TODO!!!
                    RetVal = ERROR_INVALID_DATA;
                    break;
*/
                    RCt = 0; /* Right ? */
                }
            }
        }
        else {
            RetVal = ERROR_INVALID_DATA;
        }

        KNamelistRelease ( List );
    }
    else {
        RetVal = ERROR_INVALID_DATA;
    }

    XFSEditorDispose ( & ( Editor -> Papahen ) );

//TT wLogMsg ( klogDebug, L" FIND Files [%s][0x%p][H=0x%p][V=%d]\n", PathName, TheFileInfo, Handle, RetVal );

    return RetVal * - 1;
}   /* FindFiles() */

#endif /* USE_XFS_DOKAN_FINDFILES == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_FINDFILESWITHPATTERN == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_FindFilesWithPattern (
            LPCWSTR PathName,
            LPCWSTR SearchPattern,
            PFillFindData FindDataCallback,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"FINDFILESWITHPATTERN(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* FindFilesWithPattern() */

#endif /* USE_XFS_DOKAN_FINDFILESWITHPATTERN == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_SETFILEATTRIBUTES == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_SetFileAttributes (
            LPCWSTR FileName,
            DWORD FileAttributes,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"SETFILEATTRIBUTES(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* SetFileAttributes() */

#endif /* USE_XFS_DOKAN_SETFILEATTRIBUTES == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_SETFILETIME == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_SetFileTime (
            LPCWSTR FileName,
            CONST FILETIME * CreationTime,
            CONST FILETIME * LastAccessTime,
            CONST FILETIME * LastWriteTime,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;
    const struct XFSNode * Node;
    KTime_t Time;
    ULONG64 xTime;

    RCt = 0;
    Editor = NULL;
    Node = NULL;
    Time = 0;
    xTime = 0;

//TT wLogMsg ( klogDebug, L" SET file time: [%s][FI=0x%p]\n", FileName, TheFileInfo );

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }

        /*\ First we should convert time to time
        \*/
    xTime = LastWriteTime -> dwHighDateTime << 32;
    xTime += LastWriteTime -> dwLowDateTime;
    xTime -= 116444736000000000;
    xTime /= 10000000;

    RCt = _DOKAN_get_path_and_node (
                                FileName,
                                TheFileInfo,
                                NULL,   /* VPath, no need */
                                & Node,
                                NULL    /* NodeType */
                                );
    if ( RCt == 0 ) {
        RCt = XFSNodeAttrEditor ( Node, & Editor );

        if ( RCt == 0 ) {
            RCt = XFSAttrEditorSetDate ( Editor, xTime );
        }

        XFSNodeRelease ( Node );
    }

//TT wLogMsg ( klogDebug, L" SET file time,cont: [%s][FI=0x%p][RC=%d]\n", FileName, TheFileInfo, RCt );

    return ( RCt == 0 ? 0 : ERROR_INVALID_DATA ) * - 1;
}   /* SetFileTime() */

#endif /* USE_XFS_DOKAN_SETFILETIME == 1 */

/************************************************************/
/************************************************************/
#if USE_XFS_DOKAN_DELETEFILE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_DeleteFile (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;

    RCt = 0;

//TT wLogMsg ( klogDebug, L"DELETE File [%s][FI=0x%p][H=0x%p]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

    RCt = _DOKAN_delete_file_dir ( FileName, TheFileInfo );

//TT wLogMsg ( klogDebug, L"   DELETE File,cont [%s][FI=0x%p][H=0x%p][RC=%d]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, RCt );

    return ( RCt == 0 ? 0 : ERROR_INVALID_DATA ) * - 1;
}   /* DeleteFile() */

#endif /* USE_XFS_DOKAN_DELETEFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_DELETEDIRECTORY == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_DeleteDirectory (
            LPCWSTR FileName,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;

    RCt = 0;

//TT wLogMsg ( klogDebug, L"DELETE Directory [%s][FI=0x%p][H=0x%p]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context );

    RCt = _DOKAN_delete_file_dir ( FileName, TheFileInfo );

//TT wLogMsg ( klogDebug, L"   DELETE Directory [%s][FI=0x%p][H=0x%p][RC=%d]\n", FileName, TheFileInfo, (void * ) TheFileInfo -> Context, RCt );

    return ( RCt == 0 ? 0 : ERROR_INVALID_DATA ) * - 1;
}   /* DeleteDirectory() */

#endif /* USE_XFS_DOKAN_DELETEDIRECTORY == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_MOVEFILE == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_MoveFile (
            LPCWSTR OldFile,
            LPCWSTR NewFile,
            BOOL ReplaceExisting,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSNode * OldDir, * NewDir;
    char * OldName, * NewName;
    struct XFSDirEditor * Editor;

    RCt = 0;
    OldDir = NewDir = NULL;
    OldName = NewName = NULL;
    Editor = NULL;

//TT wLogMsg ( klogDebug, L"MOVE File FR[%s]TO[%s][FI=0x%p][H=0x%p]\n", OldFile, NewFile, TheFileInfo, (void * ) TheFileInfo -> Context );

    RCt = _DOKAN_get_parent_node_from_char (
                                        OldFile,
                                        TheFileInfo,
                                        & OldDir,
                                        NULL,
                                        & OldName
                                        );
    if ( RCt == 0 ) {
        RCt = _DOKAN_get_parent_node_from_char (
                                            NewFile,
                                            TheFileInfo,
                                            & NewDir,
                                            NULL,
                                            & NewName
                                            );
        if ( RCt == 0 ) {
            RCt = XFSNodeDirEditor ( OldDir, & Editor );
            if ( RCt == 0 ) {
                RCt = XFSDirEditorMove (
                                    Editor,
                                    OldName,
                                    NewDir,
                                    NewName
                                    );
                XFSEditorDispose ( & ( Editor -> Papahen ) );
            }

            XFSNodeRelease ( NewDir );
            free ( NewName );
        }

        XFSNodeRelease ( OldDir );
        free ( OldName );
    }

//TT wLogMsg ( klogDebug, L"   MOVE File FR[%s]TO[%s][FI=0x%p][H=0x%p][RC=%d]\n", OldFile, NewFile, TheFileInfo, (void * ) TheFileInfo -> Context, RCt );

    return ( RCt == 0 ? 0 : ERROR_INVALID_DATA ) * - 1;
}   /* MoveFile() */

#endif /* USE_XFS_DOKAN_MOVEFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_SETENDOFFILE == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_SetEndOfFile (
            LPCWSTR FileName,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    struct XFSHandle * Handle;
    struct XFSFileEditor * FileEditor;
    const struct XFSNode * Node;

    RCt = 0;
    Handle = NULL;
    Editor = NULL;
    Node = NULL;

//TT wLogMsg ( klogDebug, L" SET end of file: [%s][FI=0x%p]\n", FileName, TheFileInfo );

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;

    Editor = Handle != NULL
                ? ( struct XFSFileEditor * ) XFSHandleGet ( Handle )
                : NULL
                ;
    if ( Editor == NULL ) {
        RCt = _DOKAN_get_path_and_node (
                                    FileName,
                                    TheFileInfo,
                                    NULL,   /* VPath, no need */
                                    & Node,
                                    NULL    /* NodeType */
                                    );
        if ( RCt == 0 ) {
            RCt = XFSNodeFileEditor ( Node, & Editor );
            if ( RCt == 0 ) {
                RCt = XFSFileEditorSetSize ( Editor, Length );

                XFSEditorDispose ( & ( Editor -> Papahen ) );
            }

            XFSNodeRelease ( Node );
        }
    }
    else {
        RCt = XFSFileEditorSetSize ( Editor, Length );
    }

    return ( RCt == 0 ? 0 : ERROR_INVALID_DATA ) * - 1;
}   /* SetEndOfFile() */

#endif /* USE_XFS_DOKAN_SETENDOFFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_SETALLOCATIONSIZE == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_SetAllocationSize (
            LPCWSTR FileName,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"SETALLOCATIONSIZE(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* SetAllocationSize() */

#endif /* USE_XFS_DOKAN_SETALLOCATIONSIZE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_LOCKFILE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_LockFile (
            LPCWSTR FileName,
            LONGLONG ByteOffset,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"LOCKFILE(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* LockFile() */

#endif /* USE_XFS_DOKAN_LOCKFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_UNLOCKFILE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_UnlockFile (
            LPCWSTR FileName,
            LONGLONG ByteOffset,
            LONGLONG Length,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"UNLOCKFILE(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* UnlockFile() */

#endif /* USE_XFS_DOKAN_UNLOCKFILE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_GETDISKFREESPACE == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_GetDiskFreeSpace (
            PULONGLONG FreeBytesAvailable,
            PULONGLONG TotalNumberOfBytes,
            PULONGLONG TotalNumberOfFreeBytes,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"GETDISKFREESPACE(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* GetDiskFreeSpace() */

#endif /* USE_XFS_DOKAN_GETDISKFREESPACE == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_GETVOLUMEINFORMATION == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_GetVolumeInformation (
            LPWSTR VolumeNameBuffer,
            DWORD VolumeNameSize,
            LPDWORD VolumeSerialNumber,
            LPDWORD MaximumComponentLength,
            LPDWORD FileSystemFlags,
            LPWSTR FileSystemNameBuffer,
            DWORD FileSystemNameSize,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"GETVOLUMEINFORMATION(DOKAN): [FI=0x%p]\n", TheFileInfo );

    wcscpy_s(
            VolumeNameBuffer,
            VolumeNameSize / sizeof(WCHAR),
            // L"NCBI&CO"
            L"dbGaP"
            );

    * VolumeSerialNumber = 0x19450509;
    * MaximumComponentLength = 256;
    * FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH
                        | FILE_CASE_PRESERVED_NAMES
                        | FILE_SUPPORTS_REMOTE_STORAGE
                        | FILE_UNICODE_ON_DISK
                        | FILE_PERSISTENT_ACLS  /*  comment if ACL and
                                                    security does not
                                                    needed
                                                */
                        ;

    wcscpy_s(
            FileSystemNameBuffer,
            FileSystemNameSize / sizeof(WCHAR),
            L"NCBI&CO"
            );

    return 0;
}   /* GetVolumeInformation() */

#endif /* USE_XFS_DOKAN_GETVOLUMEINFORMATION == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_UNMOUNT == 1


static
int DOKAN_CALLBACK
XFS_DOKAN_Unmount (
            PDOKAN_FILE_INFO TheFileInfo
)
{
wLogMsg ( klogDebug, L"UNMOUNT(DOKAN): [FI=0x%p]\n", TheFileInfo );
    return - 0;
}   /* Unmount() */

#endif /* USE_XFS_DOKAN_UNMOUNT == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_GETFILESECURITY == 1

/************************************************************
 *  Luriks : see file security.c
 ***********************************************************/

static const char * _sDefaultPermissions = "rwxr-xr-x";

XFS_EXTERN void CC _SI_Dump (
                        SECURITY_INFORMATION Inf,
                        char * Buff,
                        size_t BuffSize
                        );

XFS_EXTERN rc_t CC XFSSecurityDescriptor (
                                    SECURITY_INFORMATION SecInfo,
                                    const char * Permissions,
                                    XFSNType NodeType,
                                    PSECURITY_DESCRIPTOR Descriptor,
                                    ULONG DescriptorLength
                                    );

XFS_EXTERN rc_t CC XFSSecurityDescriptorSize (
                                    SECURITY_INFORMATION SecInfo,
                                    const char * Permissions,
                                    XFSNType NodeType,
                                    ULONG * DescSize
                                    );

static
int DOKAN_CALLBACK
XFS_DOKAN_GetFileSecurity (
                        LPCWSTR FileName,
                        PSECURITY_INFORMATION SecInfo,
                        PSECURITY_DESCRIPTOR SecDsc,
                        ULONG SecDscLen,
                        PULONG SecDscLenNeeded,
                        PDOKAN_FILE_INFO TheFileInfo
)
{
    rc_t RCt;
    int RetVal;
    SECURITY_INFORMATION Sinfo;
    const struct XFSHandle * Handle;
    const struct XFSNode * Node;
    const struct XFSAttrEditor * Editor;
    XFSNType Type;
    const char * Permissions;

    RetVal = 0;
    RCt = 0;
    Sinfo = 0;
    Handle = NULL;
    Node = NULL;
    Editor = NULL;
    Type = kxfsNotFound;
    Permissions = NULL;

    if ( FileName == NULL || TheFileInfo == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }

    if ( SecDscLenNeeded == NULL || SecDsc == NULL || SecInfo == NULL ) {
        return ERROR_INVALID_DATA * - 1;
    }
    * SecDscLenNeeded = 0;
    Sinfo = * SecInfo;

    Handle = ( struct XFSHandle * ) TheFileInfo -> Context;

    if ( Handle == NULL )  {
        return ERROR_INVALID_HANDLE * - 1;
    }

    Node = XFSHandleNode ( Handle );
    if ( Node == NULL ) {
        return ERROR_INVALID_HANDLE * - 1;
    }

    if ( XFSNodeAttrEditor ( Node, & Editor ) != 0 ) {
        return ERROR_INVALID_HANDLE * - 1;
    }

//TT wLogMsg ( klogDebug, L" SECURITY File [%s][0x%p][E=0x%p]\n", FileName, TheFileInfo, Editor );
/*
{
char FU [ 64 ];
_SI_Dump ( Sinfo, FU, sizeof ( FU ) );
printf ( "SECURITY File [%s][0x%p]\n", FU, TheFileInfo );
}
*/

    RCt = XFSAttrEditorType ( Editor, & Type );
    if ( RCt == 0 ) {
        if ( Type == kxfsFile || Type == kxfsDir || Type == kxfsLink ) {
            RCt = XFSAttrEditorPermissions ( Editor, & Permissions );
            if ( RCt == 0 ) {

                if ( Permissions == NULL ) {
                    Permissions = _sDefaultPermissions;
                }

                /*   First we are going to check if here is enough space
                 */
                RCt = XFSSecurityDescriptorSize (
                                            Sinfo,
                                            Permissions,
                                            Type,
                                            SecDscLenNeeded
                                            );
                if ( RCt == 0 && 0 < * SecDscLenNeeded ) {
                    if ( SecDscLen == 0 || SecDscLen < * SecDscLenNeeded ) {
//TT wLogMsg ( klogDebug, L" SECURITY File ( SIZE Requested ) [%s][NS=%d][BS=%d]\n", FileName, * SecDscLenNeeded, SecDscLen );
                        RetVal = ERROR_INSUFFICIENT_BUFFER;
                    }
                    else {
                        RCt = XFSSecurityDescriptor (
                                                Sinfo,
                                                Permissions,
                                                Type,
                                                SecDsc,
                                                SecDscLen
                                                );
                    }
                }
            }
        }
        else {
            RCt = XFS_RC ( rcInvalid );
        }
    }

    if ( RCt != 0 ) {
        if ( RetVal == 0 ) {
            RetVal = ERROR_INVALID_HANDLE;
        }
    }

    XFSEditorDispose ( & ( Editor -> Papahen ) );
//TT wLogMsg ( klogDebug, L" SECURITY File,cont [%s][0x%p][R=%d]\n", FileName, TheFileInfo, RetVal );

    return RetVal * - 1;
}   /* GetFileSecurity() */

#endif /* USE_XFS_DOKAN_GETFILESECURITY == 1 */

/************************************************************/
/************************************************************/

#if USE_XFS_DOKAN_SETFILESECURITY == 1

static
int DOKAN_CALLBACK
XFS_DOKAN_SetFileSecurity (
            LPCWSTR FileName,
            PSECURITY_INFORMATION SecurityInformation,
            PSECURITY_DESCRIPTOR SecurityDescriptor,
            ULONG SecurityDescriptorLength,
            PDOKAN_FILE_INFO TheFileInfo
)
{
//TT wLogMsg ( klogDebug, L"SETFILESECURITY(DOKAN): [%s][FI=0x%p]\n", FileName, TheFileInfo );
    return - 0;
}   /* SetFileSecurity() */

#endif /* USE_XFS_DOKAN_SETFILESECURITY == 1 */

/************************************************************/
/************************************************************/

/*))
  || Old good necessary method
  (*/
LIB_EXPORT
rc_t CC
XFS_Private_InitOperations ( DOKAN_OPERATIONS * Operations )
{
    if ( Operations == NULL ) {
        return XFS_RC ( rcNull );
    }

    ZeroMemory ( Operations, sizeof( struct _DOKAN_OPERATIONS ) );


#if USE_XFS_DOKAN_CREATEFILE == 1
    Operations -> CreateFile = XFS_DOKAN_CreateFile;
#endif /* USE_XFS_DOKAN_CREATEFILE == 1 */

#if USE_XFS_DOKAN_OPENDIRECTORY == 1
    Operations -> OpenDirectory = XFS_DOKAN_OpenDirectory;
#endif /* USE_XFS_DOKAN_OPENDIRECTORY == 1 */

#if USE_XFS_DOKAN_CREATEDIRECTORY == 1
    Operations -> CreateDirectory = XFS_DOKAN_CreateDirectory;
#endif /* USE_XFS_DOKAN_CREATEDIRECTORY == 1 */

#if USE_XFS_DOKAN_CLEANUP == 1
    Operations -> Cleanup = XFS_DOKAN_Cleanup;
#endif /* USE_XFS_DOKAN_CLEANUP == 1 */

#if USE_XFS_DOKAN_CLOSEFILE == 1
    Operations -> CloseFile = XFS_DOKAN_CloseFile;
#endif /* USE_XFS_DOKAN_CLOSEFILE == 1 */

#if USE_XFS_DOKAN_READFILE == 1
    Operations -> ReadFile = XFS_DOKAN_ReadFile;
#endif /* USE_XFS_DOKAN_READFILE == 1 */

#if USE_XFS_DOKAN_WRITEFILE == 1
    Operations -> WriteFile = XFS_DOKAN_WriteFile;
#endif /* USE_XFS_DOKAN_WRITEFILE == 1 */

#if USE_XFS_DOKAN_FLUSHFILEBUFFERS == 1
    Operations -> FlushFileBuffers = XFS_DOKAN_FlushFileBuffers;
#endif /* USE_XFS_DOKAN_FLUSHFILEBUFFERS == 1 */

#if USE_XFS_DOKAN_GETFILEINFORMATION == 1
    Operations -> GetFileInformation = XFS_DOKAN_GetFileInformation;
#endif /* USE_XFS_DOKAN_GETFILEINFORMATION == 1 */

#if USE_XFS_DOKAN_FINDFILES == 1
    Operations -> FindFiles = XFS_DOKAN_FindFiles;
#endif /* USE_XFS_DOKAN_FINDFILES == 1 */

#if USE_XFS_DOKAN_FINDFILESWITHPATTERN == 1
    Operations -> FindFilesWithPattern = XFS_DOKAN_FindFilesWithPattern;
#endif /* USE_XFS_DOKAN_FINDFILESWITHPATTERN == 1 */

#if USE_XFS_DOKAN_SETFILEATTRIBUTES == 1
    Operations -> SetFileAttributes = XFS_DOKAN_SetFileAttributes;
#endif /* USE_XFS_DOKAN_SETFILEATTRIBUTES == 1 */

#if USE_XFS_DOKAN_SETFILETIME == 1
    Operations -> SetFileTime = XFS_DOKAN_SetFileTime;
#endif /* USE_XFS_DOKAN_SETFILETIME == 1 */

#if USE_XFS_DOKAN_DELETEFILE == 1
    Operations -> DeleteFile = XFS_DOKAN_DeleteFile;
#endif /* USE_XFS_DOKAN_DELETEFILE == 1 */

#if USE_XFS_DOKAN_DELETEDIRECTORY == 1
    Operations -> DeleteDirectory = XFS_DOKAN_DeleteDirectory;
#endif /* USE_XFS_DOKAN_DELETEDIRECTORY == 1 */

#if USE_XFS_DOKAN_MOVEFILE == 1
    Operations -> MoveFile = XFS_DOKAN_MoveFile;
#endif /* USE_XFS_DOKAN_MOVEFILE == 1 */

#if USE_XFS_DOKAN_SETENDOFFILE == 1
    Operations -> SetEndOfFile = XFS_DOKAN_SetEndOfFile;
#endif /* USE_XFS_DOKAN_SETENDOFFILE == 1 */

#if USE_XFS_DOKAN_SETALLOCATIONSIZE == 1
    Operations -> SetAllocationSize = XFS_DOKAN_SetAllocationSize;
#endif /* USE_XFS_DOKAN_SETALLOCATIONSIZE == 1 */

#if USE_XFS_DOKAN_LOCKFILE == 1
    Operations -> LockFile = XFS_DOKAN_LockFile;
#endif /* USE_XFS_DOKAN_LOCKFILE == 1 */

#if USE_XFS_DOKAN_UNLOCKFILE == 1
    Operations -> UnlockFile = XFS_DOKAN_UnlockFile;
#endif /* USE_XFS_DOKAN_UNLOCKFILE == 1 */

#if USE_XFS_DOKAN_GETDISKFREESPACE == 1
    Operations -> GetDiskFreeSpace = XFS_DOKAN_GetDiskFreeSpace;
#endif /* USE_XFS_DOKAN_GETDISKFREESPACE == 1 */

#if USE_XFS_DOKAN_GETVOLUMEINFORMATION == 1
    Operations -> GetVolumeInformation = XFS_DOKAN_GetVolumeInformation;
#endif /* USE_XFS_DOKAN_GETVOLUMEINFORMATION == 1 */

#if USE_XFS_DOKAN_UNMOUNT == 1
    Operations -> Unmount = XFS_DOKAN_Unmount;
#endif /* USE_XFS_DOKAN_UNMOUNT == 1 */

#if USE_XFS_DOKAN_GETFILESECURITY == 1
    Operations -> GetFileSecurity = XFS_DOKAN_GetFileSecurity;
#endif /* USE_XFS_DOKAN_GETFILESECURITY == 1 */

#if USE_XFS_DOKAN_SETFILESECURITY == 1
    Operations -> SetFileSecurity = XFS_DOKAN_SetFileSecurity;
#endif /* USE_XFS_DOKAN_SETFILESECURITY == 1 */

    return 0;
}   /* XFS_Private_InitOperations() */


/****************************************************************
 * LogMsg as pLogMsg does not work with WCHAR ... so ... julep
 ****************************************************************/
LIB_EXPORT
rc_t CC
wLogMsg ( KLogLevel Level, LPCWSTR Format, ... )
{
    rc_t RCt;
    WCHAR BF [ XFS_SIZE_4096 ];
    char BFF [ XFS_SIZE_4096 ];
    va_list Args;
    size_t Size;

    RCt = 0;
    * BF = 0;
    * BFF = 0;
    Size = 0;

    if ( Level <= KLogLevelGet () ) {
        va_start ( Args, Format );

        if ( vswprintf ( BF, sizeof ( BF ) / sizeof ( WCHAR ), Format, Args ) == - 1 ) {
            RCt = XFS_RC ( rcInvalid );
        }

        va_end ( Args );

        if ( RCt == 0 ) {
            if ( wcstombs_s ( & Size, BFF, sizeof ( BFF ), BF, wcslen ( BF ) ) == 0 ) {
                RCt = LogMsg ( Level, BFF );
            }
            else {
                RCt = XFS_RC ( rcInvalid );
            }
        }
    }

    return RCt;
}   /* wLogMsg () */
