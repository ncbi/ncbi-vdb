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
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/log.h>

#include <kfs/file.h>
#include <kfs/directory.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/handle.h>
#include <xfs/perm.h>

#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

/*)))
 |||    That file contains 'native' KFile and KDirectory based nodes
 |||    Both nodes are implemented as XFSKfsNode
 |||    That kind of node represent real path which exists in system
(((*/

/*)))
 |||
 +++    FileNode, and other simple containers
 |||
(((*/
struct XFSKfsNode {
    struct XFSNode node;

    XFSNType type;       /* possible Dir and File */
    const char * path;   /* Path for object */
    const char * perm;   /* Permissions in format "rwxrwxrwx u:g:o" */
};

struct XFSKfsFileEditor {
    struct XFSFileEditor Papahen;

    struct KFile * File;
};

struct XFSKfsAttrEditor {
    struct XFSAttrEditor Papahen;

    char perm [ 16 ];     /* there we are storing 'const' object */
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* KfsNode is living here                                        *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    KfsNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _KfsNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _KfsNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _KfsFileNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _KfsDirNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _KfsNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _KfsNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _KfsNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _KfsNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sKfsFileNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _KfsNodeFlavor_v1,
                                        _KfsNodeDispose_v1,
                                        _KfsFileNodeFindNode_v1,
                                        NULL,   /* NO DIR */
                                        _KfsNodeFile_v1,
                                        _KfsNodeAttr_v1,
                                        _KfsNodeDescribe_v1
                                        };

static const struct XFSNode_vt_v1 _sKfsDirNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _KfsNodeFlavor_v1,
                                        _KfsNodeDispose_v1,
                                        _KfsDirNodeFindNode_v1,
                                        _KfsNodeDir_v1,
                                        NULL,   /* NO FILE */
                                        _KfsNodeAttr_v1,
                                        _KfsNodeDescribe_v1
                                        };

static
rc_t CC
XFSKfsNodeMake (
            struct XFSKfsNode ** Node,
            XFSNType Type,
            const char * Name
)
{
    rc_t RCt;
    struct XFSKfsNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    if ( Node == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    if ( Type != kxfsFile && Type != kxfsDir ) {
        return XFS_RC ( rcUnsupported );
    }

    TheNode = calloc ( 1, sizeof ( struct XFSKfsNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInitVT (
                & ( TheNode -> node),
                Name,
                ( const union XFSNode_vt * ) ( Type == kxfsDir
                                        ? ( & _sKfsDirNodeVT_v1 )
                                        : ( & _sKfsFileNodeVT_v1 )
                )
                );
        if ( RCt == 0 ) {

            TheNode -> type = Type;

                /* This is duplicate, but necessary one
                 */
            TheNode -> node . vt = Type == kxfsDir
                    ? ( ( const union XFSNode_vt * ) & _sKfsDirNodeVT_v1 )
                    : ( ( const union XFSNode_vt * ) & _sKfsFileNodeVT_v1 )
                    ;

            * Node = TheNode;
        }
    }

    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( & ( TheNode -> node ) );
            TheNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "XFSKfsNodeMake ND[$(node)] NM[$(name)] TP[$(type)]", "node=%p,name=%s,type=%d", ( void * ) TheNode, Name, Type );
*/

    return RCt;
}   /* XFSKfsNodeMake () */

static
rc_t CC
XFSKfsNodeMakeEx (
            struct XFSKfsNode ** Node,
            XFSNType Type,
            const char * Name,
            const char * Path,
            const char * Perm
)
{
    struct XFSKfsNode * TempNode;
    rc_t RCt;

    RCt = 0;
    TempNode = NULL;

    if ( Node == NULL || Name == NULL || Path == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    RCt = XFSKfsNodeMake ( & TempNode, Type, Name );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( Path, & ( TempNode -> path ) );
        if ( RCt == 0 ) {
            if ( Perm != NULL ) {
                RCt = XFS_StrDup ( Perm, & ( TempNode -> perm ) );
            }
            if ( RCt == 0 ) {
                * Node = TempNode;
            }
        }
    }

    if ( RCt != 0 ) {
        if ( TempNode != NULL ) {
            XFSNodeDispose ( & ( TempNode -> node ) );
            TempNode = NULL;
        }
    }

    return RCt;
}   /* XFSKfsNodeMakeEx () */

uint32_t CC
_KfsNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfKfs;
}   /* _KfsNodeFlavor_v1 () */

static
rc_t CC
XFSKfsNodeDispose ( const struct XFSKfsNode * self )
{
    struct XFSKfsNode * Node = ( struct XFSKfsNode * ) self;

/*
pLogMsg ( klogDebug, "XFSKfsNodeDispose ( $(node) ) TP[$(type)]", "node=%p,type=%d", ( void * ) Node, ( Node == NULL ? 0 : Node -> type ) );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> perm != NULL ) {
        free ( ( char * ) Node -> perm );
        Node -> perm = NULL;
    }

    if ( Node -> path != NULL ) {
        free ( ( char * ) Node -> path );
        Node -> path = NULL;
    }

    Node -> type = 0;

    free ( Node );

    return 0;
}   /* XFSKfsNodeDispose () */

rc_t CC
_KfsNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSKfsNodeDispose ( ( struct XFSKfsNode * ) self );
}   /* _KfsNodeDispose_v1 () */

/*)))
 |||
 +++  There will be two methods to find node: for KDir and for KFile
 |||
(((*/

/*)) KFile version
 ((*/
rc_t CC
_KfsFileNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    bool IsLast;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    IsLast = false;

    RCt = XFSNodeFindNodeCheckInitStandard (
                                            self,
                                            Path,
                                            PathIndex,
                                            Node,
                                            & NodeName,
                                            & PathCount,
                                            & IsLast
                                            );
    if ( RCt == 0 ) {
        if ( IsLast ) {
            RCt = XFSNodeAddRef ( self );

            * Node = self;
        }
    }

    return RCt;
}   /* _KfsFileNodeFindNode () */

/*)) KDir version
 ((*/
rc_t CC
_KfsDirNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    struct XFSKfsNode * KfsNode;
    bool IsLast;
    KDirectory * NativeDir;
    XFSNType Type;
    const struct XFSPath * xPath, * yPath;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    KfsNode = NULL;
    IsLast = false;
    NativeDir = NULL;
    Type = kxfsNotFound;
    xPath = yPath = NULL;

    RCt = XFSNodeFindNodeCheckInitStandard (
                                            self,
                                            Path,
                                            PathIndex,
                                            Node,
                                            & NodeName,
                                            & PathCount,
                                            & IsLast
                                            );
    if ( RCt == 0 ) {
        if ( IsLast ) {
            RCt = XFSNodeAddRef ( self );

            * Node = self;

            return RCt;
        }

        KfsNode = ( struct XFSKfsNode * ) self;
        if ( KfsNode -> path == NULL ) {
            return XFS_RC ( rcInvalid );
        }

            /*) Here we are trying to create new node
             (*/
        RCt = XFSPathFrom ( Path, PathIndex + 1, & xPath );
        if ( RCt == 0 ) {
            RCt = XFSPathMake (
                            & yPath,
                            true,
                            "%s/%s",
                            KfsNode -> path,
                            XFSPathGet ( xPath )
                            );
            if ( RCt == 0 ) {
                RCt = KDirectoryNativeDir ( & NativeDir );
                if ( RCt == 0 ) {
                    switch ( KDirectoryPathType ( NativeDir, XFSPathGet ( yPath ) ) ) {
                        case kptFile :
                                Type = kxfsFile;
                                break;
                        case kptDir :
                                Type = kxfsDir;
                                break;
                        default :
                                RCt = XFS_RC ( rcInvalid );
                                break;
                    }
                    if ( RCt == 0 ) {
                        RCt = XFSKfsNodeMakeEx (
                                            & KfsNode,
                                            Type,
                                            XFSPathName ( yPath ),
                                            XFSPathGet ( yPath ),
                                            NULL
                                            );
                        if ( RCt == 0 ) {
                            * Node = & ( KfsNode -> node );
                        }
                    }

                    KDirectoryRelease ( NativeDir );
                }

                XFSPathRelease ( yPath );
            }

            XFSPathRelease ( xPath );
        }
    }

    return RCt;
}   /* _KfsDirNodeFindNode () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_KfsDir_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_KfsDir_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSDirEditor * ) self );
    }

    return 0;
}   /* _KfsDir_dispose_v1 () */

static
rc_t CC
_KfsDir_list_v1 (
                const struct XFSDirEditor * self,
                const struct KNamelist ** List
)
{
    KDirectory * NativeDir;
    const struct XFSKfsNode * Node;
    struct KNamelist * TempList;
    rc_t RCt;

    RCt = 0;
    NativeDir = NULL;
    Node = NULL;
    TempList = NULL;

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }
    * List = NULL;

    Node = ( const struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> type != kxfsDir ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryList (
                            NativeDir,
                            & TempList,
                            NULL, /* Filter Func */
                            NULL, /* Filter Data */
                            Node -> path
                            );
        if ( RCt == 0 ) {
            * List = TempList;
        }
        else {
            if ( TempList != NULL ) {
                KNamelistRelease ( TempList );
            }
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsDir_list_v1 () */

static
rc_t CC
_KfsDir_find_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    KDirectory * NativeDir;
    const struct XFSKfsNode * KfsNode;
    struct XFSKfsNode * TempNode;
    uint32_t FileType;
    XFSNType Type;
    char FullPath [ XFS_SIZE_4096 ];
    size_t NumWrit;
    rc_t RCt;

    RCt = 0;
    NativeDir = NULL;
    KfsNode = NULL;
    TempNode = NULL;
    FileType = kptNotFound;
    Type = kxfsNotFound;
    NumWrit = 0;
    * FullPath = 0;

    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    KfsNode = ( const struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( KfsNode == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( KfsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        FullPath,
                        sizeof ( FullPath ),
                        & NumWrit,
                        "%s/%s",
                        KfsNode -> path,
                        Name
                        );
    if ( RCt == 0 ) {

        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            FileType = KDirectoryPathType ( NativeDir, FullPath );
            switch ( FileType ) {
                case kptFile :
                    Type = kxfsFile;
                    break;
                case kptDir :
                    Type = kxfsDir;
                    break;
                default :
                    RCt = XFS_RC ( rcUnsupported );
                    break;
            }
            if ( RCt == 0 ) {
                RCt = XFSKfsNodeMakeEx (
                                    & TempNode,
                                    Type,
                                    Name,
                                    FullPath,
                                    KfsNode -> perm
                                    );
                if ( RCt == 0 ) {
                    * Node = ( const struct XFSNode * ) TempNode;
                }
            }

            KDirectoryRelease ( NativeDir );
        }
    }

    return RCt;
}   /* _KfsDir_find_v1 () */

static
rc_t CC
_KfsDir_create_file_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                XFSNMode Mode,
                const struct XFSHandle ** Handle
)
{
    struct KDirectory * NativeDir;
    struct KFile * File;
    bool Update;
    KCreateMode CreateMode;
    char Path [ XFS_SIZE_4096 ];
    struct XFSKfsNode * KfsNode, * TempNode;
    struct XFSKfsFileEditor * FileEditor;
    const struct XFSHandle * TempHandle;
    size_t NumWritten;
    rc_t RCt;

    NativeDir = NULL;
    File = NULL;
    Update = false;
    CreateMode = kcmCreate;
    * Path = 0;
    KfsNode = TempNode = NULL;
    FileEditor = NULL;
    TempHandle = NULL;
    NumWritten = 0;
    RCt = 0;

    if ( self == NULL || Name == NULL || Handle == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Handle = NULL;

    KfsNode = ( struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( KfsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        Path,
                        sizeof ( Path ),
                        & NumWritten,
                        "%s/%s",
                        KfsNode -> path,
                        Name
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    Update = Mode == kxfsReadWrite;
    CreateMode = kcmCreate;

        /* Here we are */
    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryCreateFile (
                                NativeDir,
                                & File, 
                                Update,
                                XFSPermRODefNodeNum (),
                                CreateMode,
                                Path
                                );
        if ( RCt == 0 ) {
            RCt = XFSKfsNodeMakeEx (
                                & TempNode,
                                kxfsFile,
                                Name,
                                Path,
                                KfsNode -> perm
                                );
            if ( RCt == 0 ) {
                RCt = XFSNodeFileEditor (
                                    & ( TempNode -> node ),
                                    ( const struct XFSFileEditor ** ) & FileEditor
                                    );
                if ( RCt == 0 ) {
                    FileEditor -> File = File;
                    RCt = XFSHandleMake (
                                    & ( TempNode -> node ),
                                    & TempHandle
                                    );
                    if ( RCt == 0 ) {
                        XFSHandleSet ( TempHandle, FileEditor );

                        * Handle = TempHandle;
                    }
                }
            }
        }

        KDirectoryRelease ( NativeDir );
    }

    if ( RCt != 0 ) {
        * Handle = NULL;

        if ( TempHandle != NULL ) {
            XFSHandleRelease ( TempHandle );
        }
        else {
            if ( FileEditor != NULL ) {
                XFSEditorDispose ( ( const struct XFSEditor * ) FileEditor );
            }
        }

        if ( File != NULL ) {
            KFileRelease ( File );
        }
    }

    return RCt;
}   /* _KfsDir_create_file_v1 () */

static
rc_t CC
_KfsDir_create_dir_v1 (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    rc_t RCt;
    KDirectory * NativeDir;
    struct XFSKfsNode * KfsNode;
    char Path [ XFS_SIZE_4096 ];
    size_t NumWritten;

    RCt = 0;
    NativeDir = NULL;
    KfsNode = NULL;
    * Path = 0;
    NumWritten = 0;

    if ( self == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    KfsNode = ( struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( KfsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        Path,
                        sizeof ( Path ),
                        & NumWritten,
                        "%s/%s",
                        KfsNode -> path,
                        Name
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryCreateDir (
                                NativeDir,
                                XFSPermRODefContNum (),
                                kcmCreate,
                                Path
                                );

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsDir_create_dir_v1 () */

static
rc_t CC
_KfsDir_delete_v1 (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    rc_t RCt;
    KDirectory * NativeDir;
    struct XFSKfsNode * KfsNode;
    char Path [ XFS_SIZE_4096 ];
    size_t NumWritten;

    RCt = 0;
    NativeDir = NULL;
    KfsNode = NULL;
    * Path = 0;
    NumWritten = 0;

    if ( self == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    KfsNode = ( struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( KfsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        Path,
                        sizeof ( Path ),
                        & NumWritten,
                        "%s/%s",
                        KfsNode -> path,
                        Name
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryRemove ( NativeDir, true, Path );

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsDir_delete_v1 () */

rc_t CC
_KfsDir_move_v1 (
            const struct XFSDirEditor * self,
            const char * OldName,
            const struct XFSNode * NewDir,
            const char * NewName
)
{
    rc_t RCt;
    struct XFSKfsNode * KfsNode;
    struct XFSKfsNode * NewKfsNode;
    KDirectory * NativeDir;
    char OldPath [ XFS_SIZE_4096 ];
    char NewPath [ XFS_SIZE_4096 ];
    size_t NumWritten;

    RCt = 0;
    KfsNode = NULL;
    NewKfsNode = NULL;
    NativeDir = NULL;
    * OldPath = * NewPath = 0;
    NumWritten = 0;

    if ( self == NULL || OldName == NULL || NewDir == NULL
        || NewName == NULL
    ) {
        return XFS_RC ( rcNull );
    }

        /*) TODO it is temporary, until we will do
         (*/
    if ( XFSNodeFlavor ( NewDir ) !=
            XFSNodeFlavor ( ( self -> Papahen ) . Node ) ) {
        return XFS_RC ( rcInvalid );
    }

    KfsNode = ( struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( KfsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    NewKfsNode = ( struct XFSKfsNode * ) NewDir;
    if ( NewKfsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        OldPath,
                        sizeof ( OldPath ),
                        & NumWritten,
                        "%s/%s",
                        KfsNode -> path,
                        OldName
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = string_printf (
                        NewPath,
                        sizeof ( NewPath ),
                        & NumWritten,
                        "%s/%s",
                        NewKfsNode -> path,
                        NewName
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryRename ( NativeDir, true, OldPath, NewPath );

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsDir_move_v1 () */

rc_t CC
_KfsNodeDir_v1 (
            const struct XFSNode * self,
            const struct XFSDirEditor ** Dir
)
{
    rc_t RCt;
    struct XFSDirEditor * Editor;

    RCt = 0;
    Editor = NULL;

    if ( self == NULL || Dir == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Dir = NULL;

    if ( ( ( struct XFSKfsNode * ) self ) -> type != kxfsDir ) {
        return XFS_RC ( rcInvalid );
    }

    Editor = calloc ( 1, sizeof ( struct XFSDirEditor ) );
    if ( Editor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _KfsDir_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> list = _KfsDir_list_v1;
        Editor -> find = _KfsDir_find_v1;
        Editor -> create_file = _KfsDir_create_file_v1;
        Editor -> create_dir = _KfsDir_create_dir_v1;
        Editor -> delete = _KfsDir_delete_v1;
        Editor -> move = _KfsDir_move_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _KfsNodeDir_v1 () */

/*)))
 |||
 +++  Unified FileEditor
 |||
(((*/

static
rc_t CC
_KfsFile_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSKfsFileEditor * Editor = ( struct XFSKfsFileEditor * ) self;

/*
    pLogMsg ( klogDebug, "_KfsFile_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        if ( Editor -> File != NULL ) {
            KFileRelease ( Editor -> File );

            Editor -> File = NULL;
        }

        free ( Editor );
    }

    return 0;
}   /* _KfsFile_dispose_v1 () */

static
rc_t CC
_KfsFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    KFile * File;
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;
    rc_t RCt;

    File = NULL;
    Node = NULL;
    NativeDir = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( const struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> type != kxfsFile ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        if ( Mode == kxfsRead ) {
            RCt = KDirectoryOpenFileRead (
                                        NativeDir,
                                        ( const KFile ** ) & File,
                                        Node -> path
                                        );
        }
        else {
            RCt = KDirectoryOpenFileWrite (
                                        NativeDir,
                                        & File,
                                        ( Mode & kxfsRead ) == kxfsRead,
                                        Node -> path
                                        );
        }
        if ( RCt == 0 ) {
            ( ( struct XFSKfsFileEditor * ) self ) -> File = File;
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsFile_open_v1 () */

static
rc_t CC
_KfsFile_close_v1 ( const struct XFSFileEditor * self )
{
    struct XFSKfsFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }


    Editor = ( struct XFSKfsFileEditor * ) self;

    if ( Editor -> File != NULL ) {
        RCt = KFileRelease ( Editor -> File );

        Editor -> File = NULL;
    }

    return RCt;
}   /* _KfsFile_close_v1 () */

static
rc_t CC
_KfsFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    struct XFSKfsFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Editor = ( struct XFSKfsFileEditor * ) self;

    if ( Editor -> File == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KFileReadAll (
                    Editor -> File,
                    Offset,
                    Buffer,
                    SizeToRead,
                    NumReaded
                    );

/* here may be debutt */

    return RCt;
}   /* _KfsFile_read_v1 () */

static
rc_t CC
_KfsFile_write_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    const void * Buffer,
                    size_t SizeToWrite,
                    size_t * NumWritten
)
{
    struct XFSKfsFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Editor = ( struct XFSKfsFileEditor * ) self;

    if ( Editor -> File == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KFileWriteAll (
                    Editor -> File,
                    Offset,
                    Buffer,
                    SizeToWrite,
                    NumWritten
                    );


/* here may be debutt */

    return RCt;
}   /* _KfsFile_write_v1 () */

static
rc_t CC
_KfsFile_size_v1 (
                        const struct XFSFileEditor * self,
                        uint64_t * Size
)
{
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;
    const struct KFile * File;
    uint64_t TempSize;
    rc_t RCt;

    Node = NULL;
    NativeDir = NULL;
    File = NULL;
    TempSize = 0;
    RCt = 0;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    Node = ( const struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> type != kxfsFile ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    File = ( ( struct XFSKfsFileEditor * ) self ) -> File;
    if ( File == NULL ) {
        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            RCt = KDirectoryFileSize (
                                NativeDir,
                                & TempSize,
                                Node -> path
                                );

            KDirectoryRelease ( NativeDir );
        }
    }
    else {
        RCt = KFileSize ( File, & TempSize );
    }
    if ( RCt == 0 ) {
        * Size = TempSize;
    }

    return RCt;
}   /* _KfsFile_size_v1 () */

static
rc_t CC
_KfsFile_set_size_v1 (
                const struct XFSFileEditor * self,
                uint64_t Size
)
{
    rc_t RCt;
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;
    struct KFile * File;

    RCt = 0;
    Node = NULL;
    NativeDir = NULL;
    File = NULL;

    XFS_CAN ( self )

    Node = ( const struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> type != kxfsFile ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    File = ( ( struct XFSKfsFileEditor * ) self ) -> File;
    if ( File == NULL ) {

        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            RCt = KDirectorySetFileSize ( NativeDir, Size, Node -> path);

            KDirectoryRelease ( NativeDir );
        }
    }
    else {
        RCt = KFileSetSize ( File, Size );
    }

    return RCt;
}   /*  _KfsFile_set_size_v1 () */

rc_t CC
_KfsNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct XFSKfsFileEditor * FileEditor;
    struct XFSFileEditor * Editor;

    RCt = 0;
    FileEditor = NULL;
    Editor = NULL;

    if ( self == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    * File = NULL;

    if ( ( ( struct XFSKfsNode * ) self ) -> type != kxfsFile ) {
        return XFS_RC ( rcInvalid );
    }

    FileEditor = calloc ( 1, sizeof ( struct XFSKfsFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _KfsFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _KfsFile_open_v1;
        Editor -> close = _KfsFile_close_v1;
        Editor -> read = _KfsFile_read_v1;
        Editor -> write = _KfsFile_write_v1;
        Editor -> size = _KfsFile_size_v1;
        Editor -> set_size = _KfsFile_set_size_v1;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _KfsNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_KfsAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_KfsAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSKfsAttrEditor * ) self );
    }

    return 0;
}   /* _KfsAttr_dispose_v1 () */

/*))    Something unusual. Will check-initialize NativeDir and Node
 //     NativeDir and Node could be NULL
((*/
static
rc_t CC
_KfsAttr_init_check_v1 (
                    const struct XFSAttrEditor * self,
                    const struct XFSKfsNode ** Node,
                    KDirectory ** NativeDir

)
{
    rc_t RCt;
    const struct XFSKfsNode * RetNode;
    KDirectory * Dir;

    RCt = 0;
    RetNode = NULL;
    Dir = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( NativeDir != NULL ) {
        * NativeDir = NULL;
    }

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetNode = ( const struct XFSKfsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( RetNode == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( RetNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( NativeDir != NULL ) {
        RCt = KDirectoryNativeDir ( & Dir );
        if ( RCt == 0 ) {
            * NativeDir = Dir;
        }
    }

    if ( RCt == 0 ) {
        if ( Node != NULL ) {
            * Node = RetNode;
        }
    }

    return RCt;
}   /* _KfsAttr_init_check_v1 () */

static
rc_t CC
_KfsAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;
    uint32_t Access;
    char * BF;
    rc_t RCt;

    Node = NULL;
    NativeDir = NULL;
    BF = NULL;
    RCt = 0;

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Permissions = NULL;

    RCt = _KfsAttr_init_check_v1 ( self, & Node, & NativeDir );
    if ( RCt == 0 ) {
        if ( Node -> perm != NULL ) {
            * Permissions = Node -> perm;
        }
        else {
            RCt = KDirectoryAccess ( NativeDir, & Access, Node -> path );
            if ( RCt == 0 ) {
                BF = ( ( struct XFSKfsAttrEditor * ) self ) -> perm;
                RCt = XFSPermToChar (
                                    Access,
                                    BF,
                                    sizeof ( ( ( struct XFSKfsAttrEditor * ) self ) -> perm )
                                    );
                if ( RCt == 0 ) {
                    if ( Node -> type == kxfsDir ) {
                        BF [ 0 ] = 'r';
                        BF [ 2 ] = 'x';
                    }

                    * Permissions = ( const char * ) BF;
                }
            }
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsAttr_permissions_v1 () */

static
rc_t CC
_KfsAttr_set_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char * Permissions
)
{
    rc_t RCt;
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;
    uint32_t Access;

    RCt = 0;
    NativeDir = NULL;
    Node = NULL;

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _KfsAttr_init_check_v1 ( self, & Node, & NativeDir );
    if ( RCt == 0 ) {
        if ( Node -> perm != NULL ) {
            RCt = XFSPermToNum ( Node -> perm, & Access );
        }
        else {
            RCt = XFSPermToNum ( Permissions, & Access );
        }
        if ( RCt == 0 ) {
            RCt = KDirectorySetAccess (
                                    NativeDir,
                                    false,
                                    Access,
                                    Access,
                                    Node -> path
                                    );
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /*  _KfsAttr_set_permissions_v1 () */

static
rc_t CC
_KfsAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;
    KTime_t TempTime;
    rc_t RCt;

    Node = NULL;
    NativeDir = NULL;
    TempTime = 0;
    RCt = 0;

    if ( Time == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Time = 0;

    RCt = _KfsAttr_init_check_v1 ( self, & Node, & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryDate ( NativeDir, & TempTime, Node -> path );
        if ( RCt == 0 ) {
            * Time = TempTime;
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KfsAttr_date_v1 () */

static
rc_t CC
_KfsAttr_set_date_v1 (
                const struct XFSAttrEditor * self,
                KTime_t Time
)
{
    rc_t RCt;
    const struct XFSKfsNode * Node;
    KDirectory * NativeDir;

    RCt = 0;
    NativeDir = NULL;
    Node = NULL;

    RCt = _KfsAttr_init_check_v1 ( self, & Node, & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectorySetDate ( NativeDir, true, Time, Node -> path );
        
        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /*  _KfsAttr_set_date_v1 () */

static
rc_t CC
_KfsAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    const struct XFSKfsNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    if ( Type == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Type = kxfsFile;

    RCt = _KfsAttr_init_check_v1 ( self, & Node, NULL );
    if ( RCt == 0 ) {
        * Type = Node -> type;
    }

    return RCt;
}   /* _KfsAttr_type_v1 () */

static
rc_t CC
_KfsNodeAttr_v1 (
            const struct XFSNode * self,
            const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSKfsAttrEditor * KfsEditor;
    struct XFSAttrEditor * Editor;

    RCt = 0;
    KfsEditor = NULL;
    Editor = NULL;

    if ( self == NULL || Attr == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Attr = NULL;

    KfsEditor = calloc ( 1, sizeof ( struct XFSKfsAttrEditor ) );
    if ( KfsEditor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    Editor = & ( KfsEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _KfsAttr_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> permissions = _KfsAttr_permissions_v1;
        Editor -> set_permissions = _KfsAttr_set_permissions_v1;
        Editor -> date = _KfsAttr_date_v1;
        Editor -> set_date = _KfsAttr_set_date_v1;
        Editor -> type = _KfsAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( KfsEditor );
    }

    return RCt;
}   /* _KfsNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_KfsNodeDescribe_v1 (
            const struct XFSNode * self,
            char * Buffer,
            size_t BufferSize
)
{
    rc_t RCt;
    size_t NumWrit;
    const char * Abbr;

    RCt = 0;
    NumWrit = 0;
    Abbr = NULL;

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    Abbr = ( ( const struct XFSKfsNode * ) self ) -> type == kxfsDir
            ? "DIR"
            : "FILE"
            ;

    if ( self == NULL ) {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (%s)[NULL][NULL]",
                    Abbr
                    );
    }
    else {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (%s)[%s][0x%p]",
                    Abbr,
                    self -> Name,
                    self
                    );
    }

    return RCt;
}   /* _KfsNodeDescribe_v1 () */

/*)))
 |||
 +++    FileNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_KfsNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            XFSNType Type,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSKfsNode * TheNode;
    const char * NodeName;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = XFSKfsNodeMakeEx (
                    & TheNode,
                    Type,
                    NodeName,
                    XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE ),
                    XFSModelNodeSecurity ( Template )
                    );
    if ( RCt == 0 ) {
        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSKfsNodeDispose ( TheNode );
        }
    }

    return RCt;
}   /* _KfsNodeConstructor () */

/*)))
 |||
 +++    Non-Teleport methods to create nodes
 |||
(((*/
LIB_EXPORT
rc_t CC
XFSFileNodeMakeHandle (
                    const struct XFSHandle ** Handle,
                    struct XFSNode * FileNode,
                    struct KFile * File
)
{
    rc_t RCt;
    const struct XFSHandle * TheHandle;
    struct XFSKfsFileEditor * Editor;

    RCt = 0;
    TheHandle = NULL;
    Editor = NULL;

    XFS_CSAN ( Handle )
    XFS_CAN ( Handle )
    XFS_CAN ( FileNode )
    XFS_CAN ( File )

    RCt = XFSNodeFileEditor (
                        FileNode,
                        ( const struct XFSFileEditor ** ) & Editor
                        );
    if ( RCt == 0 ) {
        Editor -> File = File;

        RCt = XFSHandleMake ( FileNode,  & TheHandle );
        if ( RCt == 0 ) {
            XFSHandleSet ( TheHandle, Editor );

            * Handle = TheHandle;
        }
    }

    if ( RCt != 0 ) {
        * Handle = NULL;

        if ( TheHandle != NULL ) {
            XFSHandleRelease ( TheHandle );
        }
        else {
            if ( Editor != NULL ) {
                XFSEditorDispose ( ( const struct XFSEditor * ) Editor );
            }
        }
    }

    return RCt;
}   /* XFSFileNodeMakeHandle () */

LIB_EXPORT
rc_t CC
XFSFileNodeMake (
            struct XFSNode ** Node,
            const char * Path,
            const char * Name,
            const char * Perm
)
{
    rc_t RCt;
    struct XFSKfsNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Path == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSKfsNodeMakeEx ( & TheNode, kxfsFile, Name, Path, Perm );
    if ( RCt == 0 ) {
        * Node = & ( TheNode -> node );
    }

    return RCt;
}   /* XFSFileNodeMake () */

LIB_EXPORT
rc_t CC
XFSDirNodeMake (
            struct XFSNode ** Node,
            const char * Path,
            const char * Name,
            const char * Perm
)
{
    rc_t RCt;
    struct XFSKfsNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Path == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSKfsNodeMakeEx ( & TheNode, kxfsDir, Name, Path, Perm );
    if ( RCt == 0 ) {
        * Node = & ( TheNode -> node );
    }

    return RCt;
}   /* XFSDirNodeMake () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_FileNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _KfsNodeConstructor (
                            Model,
                            Template,
                            Alias,
                            kxfsFile,
                            Node
                            );

/*
pLogMsg ( klogDebug, "_FileNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,templat=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _FileNodeConstructor () */

static
rc_t CC
_FileNodeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_FileNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,templat=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _FileNodeValidator () */

static const struct XFSTeleport _sFileNodeTeleport = {
                                        _FileNodeConstructor,
                                        _FileNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSFileProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sFileNodeTeleport;

    return 0;
}   /* XFSFileProvider () */

/*)))
 |||
 +++    DirNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_DirNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _KfsNodeConstructor (
                            Model,
                            Template,
                            Alias,
                            kxfsDir,
                            Node
                            );

/*
pLogMsg ( klogDebug, "_DirNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,templat=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _DirNodeConstructor () */

static
rc_t CC
_DirNodeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_DirNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,templat=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _DirNodeValidator () */

static const struct XFSTeleport _sDirNodeTeleport = {
                                            _DirNodeConstructor,
                                            _DirNodeValidator,
                                            false
                                            };


LIB_EXPORT
rc_t CC
XFSDirectoryProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sDirNodeTeleport;

    return 0;
}   /* XFSDirectoryProvider () */

