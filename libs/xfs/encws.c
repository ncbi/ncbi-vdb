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
#include "ncon.h"
#include "teleport.h"
#include "common.h"
#include "xencws.h"

#include <sysalloc.h>

/*)))
 |||    That file contains 'native' KFile and KDirectory based nodes
 |||    Both nodes are implemented as _EncWsNode
 |||    That kind of node represent real path which exists in system
(((*/

/*)))
 |||
 +++    FileNode, and other simple containers
 |||
(((*/
struct _EncWsNode {
    struct XFSNode node;

    const struct KDirectory * workspace;
    const char * path;      /* Path for object */
    XFSNType type;          /* For optimizing goals only */
};

struct _EncWsFileEditor {
    struct XFSFileEditor Papahen;

    struct KFile * File;
};

struct _EncWsAttrEditor {
    struct XFSAttrEditor Papahen;

    char perm [ 16 ];     /* there we are storing 'const' object */
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* EncWsNode is living here                                      *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    EncWsNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _EncWsNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _EncWsNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _EncWsFileNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _EncWsDirNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _EncWsNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _EncWsNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _EncWsNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _EncWsNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sEncWsFileNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _EncWsNodeFlavor_v1,
                                        _EncWsNodeDispose_v1,
                                        _EncWsFileNodeFindNode_v1,
                                        NULL,   /* NO DIR */
                                        _EncWsNodeFile_v1,
                                        _EncWsNodeAttr_v1,
                                        _EncWsNodeDescribe_v1
                                        };

static const struct XFSNode_vt_v1 _sEncWsDirNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _EncWsNodeFlavor_v1,
                                        _EncWsNodeDispose_v1,
                                        _EncWsDirNodeFindNode_v1,
                                        _EncWsNodeDir_v1,
                                        NULL,   /* NO FILE */
                                        _EncWsNodeAttr_v1,
                                        _EncWsNodeDescribe_v1
                                        };

static
rc_t CC
_EncWsNodeMake (
            struct _EncWsNode ** Node,
            const struct KDirectory * Workspace,
            const char * Name,
            const char * Path
)
{
    rc_t RCt;
    struct _EncWsNode * TheNode;
    uint32_t Type;

    RCt = 0;
    TheNode = NULL;
    Type = kptNotFound;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Workspace )
    XFS_CAN ( Name )

        /* First we should check that object exist and has valid type
         */
    Type = KDirectoryPathType ( Workspace, Path );
    switch ( Type ) {
        case kptFile :
        case kptDir :
            break;
        case kptNotFound :
            return XFS_RC ( rcNotFound );
        default :
            return XFS_RC ( rcInvalid );
    }

    TheNode = calloc ( 1, sizeof ( struct _EncWsNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInitVT (
                & ( TheNode -> node),
                Name,
                ( const union XFSNode_vt * ) ( Type == kptDir
                                        ? ( & _sEncWsDirNodeVT_v1 )
                                        : ( & _sEncWsFileNodeVT_v1 )
                )
                );
        if ( RCt == 0 ) {
            RCt = KDirectoryAddRef ( Workspace );
            if ( RCt == 0 ) {
                TheNode -> workspace = Workspace;

                TheNode -> type = Type == kptDir ? kxfsDir : kxfsFile;

                    /* This is duplicate, but necessary one
                     */
                TheNode -> node . vt = Type == kxfsDir
                        ? ( ( const union XFSNode_vt * ) & _sEncWsDirNodeVT_v1 )
                        : ( ( const union XFSNode_vt * ) & _sEncWsFileNodeVT_v1 )
                        ;

                RCt = XFS_StrDup ( Path, & ( TheNode -> path ) );
                if ( RCt == 0 ) {
                    * Node = TheNode;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( & ( TheNode -> node ) );
            TheNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "_EncWsNodeMake ND[$(node)] NM[$(name)] TP[$(type)]", "node=%p,name=%s,type=%d", ( void * ) TheNode, Name, Type );
*/

    return RCt;
}   /* _EncWsNodeMake () */

uint32_t CC
_EncWsNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfWorkspace;
}   /* _EncWsNodeFlavor_v1 () */

static
rc_t CC
_EncWsNodeDispose ( const struct _EncWsNode * self )
{
    struct _EncWsNode * Node = ( struct _EncWsNode * ) self;

/*
pLogMsg ( klogDebug, "_EncWsNodeDispose ND[$(node)] TP[$(type)]", "node=%p,type=%d", ( void * ) Node, ( Node == NULL ? 0 : Node -> type ) );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> workspace != NULL ) {
        KDirectoryRelease ( Node -> workspace );
        Node -> workspace = NULL;
    }

    if ( Node -> path != NULL ) {
        free ( ( char * ) Node -> path );
        Node -> path = NULL;
    }

    Node -> type = 0;

    free ( Node );

    return 0;
}   /* _EncWsNodeDispose () */

rc_t CC
_EncWsNodeDispose_v1 ( const struct XFSNode * self )
{
    return _EncWsNodeDispose ( ( struct _EncWsNode * ) self );
}   /* _EncWsNodeDispose_v1 () */

/*)))
 |||
 +++  There will be two methods to find node: for KDir and for KFile
 |||
(((*/

/*)) KFile version
 ((*/
rc_t CC
_EncWsFileNodeFindNode_v1 (
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
}   /* _EncWsFileNodeFindNode () */

/*)) KDir version
 ((*/
rc_t CC
_EncWsDirNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    char PathBuf [ XFS_SIZE_4096 ];
    size_t PathBufLen;
    struct _EncWsNode * EncWsNode;
    const struct XFSPath * xPath;
    bool IsLast;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    * PathBuf = 0;
    PathBufLen = 0;
    EncWsNode = NULL;
    xPath = NULL;
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

            return RCt;
        }

        EncWsNode = ( struct _EncWsNode * ) self;
        if ( EncWsNode -> path == NULL ) {
            return XFS_RC ( rcInvalid );
        }

        if ( EncWsNode -> workspace == NULL ) {
            return XFS_RC ( rcInvalid );
        }

        PathBufLen = string_copy_measure (
                                        PathBuf,
                                        sizeof ( PathBuf ),
                                        EncWsNode -> path
                                        );
        * ( PathBuf + PathBufLen ) = '/';
            /*) Here we are trying to create new node
             (*/
        RCt = XFSPathFrom ( Path, PathIndex + 1, & xPath );
        if ( RCt == 0 ) {
            if ( string_copy (
                        PathBuf + PathBufLen + 1,
                        sizeof ( PathBuf ) - PathBufLen,
                        XFSPathGet ( xPath ),
                        string_size ( XFSPathGet ( xPath ) )
                        ) != string_size ( XFSPathGet ( xPath ) ) ) {

                RCt = XFS_RC ( rcInvalid );
            }
            XFSPathRelease ( xPath );
        }
        if ( RCt == 0 ) {
            RCt = _EncWsNodeMake (
                                & EncWsNode,
                                EncWsNode -> workspace,
                                XFSPathName ( Path ),
                                PathBuf
                                );
            if ( RCt == 0 ) {
                * Node = & ( EncWsNode -> node );
            }
        }
    }


    return RCt;
}   /* _EncWsDirNodeFindNode () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_EncWsDir_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_EncWsDir_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSDirEditor * ) self );
    }

    return 0;
}   /* _EncWsDir_dispose_v1 () */

static
rc_t CC
_EncWsDir_list_v1 (
                const struct XFSDirEditor * self,
                const struct KNamelist ** List
)
{
    const struct _EncWsNode * Node;
    struct KNamelist * TempList;
    rc_t RCt;

    RCt = 0;
    Node = NULL;
    TempList = NULL;

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }
    * List = NULL;

    Node = ( const struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> type != kxfsDir ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KDirectoryList (
                        Node -> workspace,
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

    return RCt;
}   /* _EncWsDir_list_v1 () */

static
rc_t CC
_EncWsDir_find_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    const struct _EncWsNode * EncWsNode;
    struct _EncWsNode * TempNode;
    char FullPath [ XFS_SIZE_4096 ];
    size_t NumWrit;
    rc_t RCt;

    RCt = 0;
    EncWsNode = NULL;
    TempNode = NULL;
    NumWrit = 0;
    * FullPath = 0;

    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    EncWsNode = ( const struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( EncWsNode == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( EncWsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( EncWsNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        FullPath,
                        sizeof ( FullPath ),
                        & NumWrit,
                        "%s/%s",
                        EncWsNode -> path,
                        Name
                        );
    if ( RCt == 0 ) {
        RCt = _EncWsNodeMake (
                            & TempNode,
                            EncWsNode -> workspace,
                            Name,
                            FullPath
                            );
        if ( RCt == 0 ) {
            * Node = ( const struct XFSNode * ) TempNode;
        }
    }

    return RCt;
}   /* _EncWsDir_find_v1 () */

static
rc_t CC
_EncWsDir_create_file_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                XFSNMode Mode,
                const struct XFSHandle ** Handle
)
{
    struct KFile * File;
    bool Update;
    KCreateMode CreateMode;
    char Path [ XFS_SIZE_4096 ];
    struct _EncWsNode * EncWsNode, * TempNode;
    struct _EncWsFileEditor * FileEditor;
    const struct XFSHandle * TempHandle;
    size_t NumWritten;
    uint32_t NodeMode;
    rc_t RCt;

    File = NULL;
    Update = false;
    CreateMode = kcmCreate;
    * Path = 0;
    EncWsNode = TempNode = NULL;
    FileEditor = NULL;
    TempHandle = NULL;
    NumWritten = 0;
    NodeMode = 0;
    RCt = 0;

    if ( self == NULL || Name == NULL || Handle == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Handle = NULL;

    EncWsNode = ( struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( EncWsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( EncWsNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        Path,
                        sizeof ( Path ),
                        & NumWritten,
                        "%s/%s",
                        EncWsNode -> path,
                        Name
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    Update = Mode == kxfsReadWrite;
    CreateMode = kcmCreate;

    NodeMode = ( Mode == kxfsReadWrite || Mode == kxfsWrite )
                                                ? XFSPermRWDefNodeNum ()
                                                : XFSPermRODefNodeNum ()
                                                ;

        /* Here we are */
    RCt = KDirectoryCreateFile (
                            ( struct KDirectory * ) EncWsNode -> workspace,
                            & File, 
                            Update,
                            NodeMode,
                            CreateMode,
                            Path
                            );
    if ( RCt == 0 ) {
        RCt = _EncWsNodeMake (
                            & TempNode,
                            EncWsNode -> workspace,
                            Name,
                            Path
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
}   /* _EncWsDir_create_file_v1 () */

static
rc_t CC
_EncWsDir_create_dir_v1 (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    rc_t RCt;
    struct _EncWsNode * EncWsNode;
    char Path [ XFS_SIZE_4096 ];
    size_t NumWritten;

    RCt = 0;
    EncWsNode = NULL;
    * Path = 0;
    NumWritten = 0;

    if ( self == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    EncWsNode = ( struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( EncWsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( EncWsNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        Path,
                        sizeof ( Path ),
                        & NumWritten,
                        "%s/%s",
                        EncWsNode -> path,
                        Name
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    return KDirectoryCreateDir (
                        ( struct KDirectory * ) EncWsNode -> workspace,
                        XFSPermRWDefContNum (),
                        kcmCreate,
                        Path
                        );
}   /* _EncWsDir_create_dir_v1 () */

static
rc_t CC
_EncWsDir_delete_v1 (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    rc_t RCt;
    struct _EncWsNode * EncWsNode;
    char Path [ XFS_SIZE_4096 ];
    size_t NumWritten;

    RCt = 0;
    EncWsNode = NULL;
    * Path = 0;
    NumWritten = 0;

    if ( self == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    EncWsNode = ( struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( EncWsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( EncWsNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        Path,
                        sizeof ( Path ),
                        & NumWritten,
                        "%s/%s",
                        EncWsNode -> path,
                        Name
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    return KDirectoryRemove (
                        ( struct KDirectory * ) EncWsNode -> workspace,
                        true,
                        Path
                        );
}   /* _EncWsDir_delete_v1 () */

rc_t CC
_EncWsDir_move_v1 (
            const struct XFSDirEditor * self,
            const char * OldName,
            const struct XFSNode * NewDir,
            const char * NewName
)
{
    rc_t RCt;
    struct _EncWsNode * EncWsNode;
    struct _EncWsNode * NewEncWsNode;
    char OldPath [ XFS_SIZE_4096 ];
    char NewPath [ XFS_SIZE_4096 ];
    size_t NumWritten;

    RCt = 0;
    EncWsNode = NULL;
    NewEncWsNode = NULL;
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

    EncWsNode = ( struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( EncWsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( EncWsNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    NewEncWsNode = ( struct _EncWsNode * ) NewDir;

    if ( NewEncWsNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( NewEncWsNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = string_printf (
                        OldPath,
                        sizeof ( OldPath ),
                        & NumWritten,
                        "%s/%s",
                        EncWsNode -> path,
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
                        NewEncWsNode -> path,
                        NewName
                        );
    if ( RCt != 0 ) {
        return RCt;
    }

    return KDirectoryRename (
                        ( struct KDirectory * ) EncWsNode -> workspace,
                        true,
                        OldPath,
                        NewPath
                        );
}   /* _EncWsDir_move_v1 () */

rc_t CC
_EncWsNodeDir_v1 (
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

    if ( ( ( struct _EncWsNode * ) self ) -> type != kxfsDir ) {
        return XFS_RC ( rcInvalid );
    }

    Editor = calloc ( 1, sizeof ( struct XFSDirEditor ) );
    if ( Editor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _EncWsDir_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> list = _EncWsDir_list_v1;
        Editor -> find = _EncWsDir_find_v1;
        Editor -> create_file = _EncWsDir_create_file_v1;
        Editor -> create_dir = _EncWsDir_create_dir_v1;
        Editor -> delete = _EncWsDir_delete_v1;
        Editor -> move = _EncWsDir_move_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _EncWsNodeDir_v1 () */

/*)))
 |||
 +++  Unified FileEditor
 |||
(((*/

static
rc_t CC
_EncWsFile_dispose_v1 ( const struct XFSEditor * self )
{
    struct _EncWsFileEditor * Editor = ( struct _EncWsFileEditor * ) self;

/*
    pLogMsg ( klogDebug, "_EncWsFile_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        if ( Editor -> File != NULL ) {
            KFileRelease ( Editor -> File );

            Editor -> File = NULL;
        }

        free ( Editor );
    }

    return 0;
}   /* _EncWsFile_dispose_v1 () */

static
rc_t CC
_EncWsFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    KFile * File;
    const struct _EncWsNode * Node;
    rc_t RCt;

    File = NULL;
    Node = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( const struct _EncWsNode * ) XFSEditorNode (
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

    if ( Node -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Mode == kxfsRead ) {
        RCt = KDirectoryOpenFileRead (
                            ( struct KDirectory * ) Node -> workspace,
                            ( const KFile ** ) & File,
                            Node -> path
                            );
    }
    else {
        RCt = KDirectoryOpenFileWrite (
                            ( struct KDirectory * ) Node -> workspace,
                            & File,
                            ( Mode & kxfsRead ) == kxfsRead,
                            Node -> path
                            );
    }
    if ( RCt == 0 ) {
        ( ( struct _EncWsFileEditor * ) self ) -> File = File;
    }

    return RCt;
}   /* _EncWsFile_open_v1 () */

static
rc_t CC
_EncWsFile_close_v1 ( const struct XFSFileEditor * self )
{
    struct _EncWsFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }


    Editor = ( struct _EncWsFileEditor * ) self;

    if ( Editor -> File != NULL ) {
        RCt = KFileRelease ( Editor -> File );

        Editor -> File = NULL;
    }

    return RCt;
}   /* _EncWsFile_close_v1 () */

static
rc_t CC
_EncWsFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    struct _EncWsFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Editor = ( struct _EncWsFileEditor * ) self;

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
}   /* _EncWsFile_read_v1 () */

static
rc_t CC
_EncWsFile_write_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    const void * Buffer,
                    size_t SizeToWrite,
                    size_t * NumWritten
)
{
    struct _EncWsFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Editor = ( struct _EncWsFileEditor * ) self;

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

    return RCt;
}   /* _EncWsFile_write_v1 () */

static
rc_t CC
_EncWsFile_size_v1 (
                        const struct XFSFileEditor * self,
                        uint64_t * Size
)
{
    const struct _EncWsNode * Node;
    const struct KFile * File;
    uint64_t TempSize;
    rc_t RCt;

    Node = NULL;
    File = NULL;
    TempSize = 0;
    RCt = 0;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    Node = ( const struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )
    XFS_CAN ( Node -> workspace )

    if ( Node -> type == kxfsDir ) {
            /*) Traditionally directory have size ZERO :lol:
             (*/
        * Size = 0;
    }
    else {
        File = ( const struct KFile * )
                        ( ( struct _EncWsFileEditor * ) self ) -> File;
        if ( File == NULL ) {
            RCt = KDirectoryFileSize (
                                Node -> workspace,
                                & TempSize,
                                Node -> path
                                );

        }
        else {
            RCt = KFileSize ( File, & TempSize );
        }

        if ( RCt == 0 ) {
            * Size = TempSize;
        }
    }

    return RCt;
}   /* _EncWsFile_size_v1 () */

static
rc_t CC
_EncWsFile_set_size_v1 (
                const struct XFSFileEditor * self,
                uint64_t Size
)
{
    rc_t RCt;
    const struct _EncWsNode * Node;
    struct KFile * File;

    RCt = 0;
    Node = NULL;

    XFS_CAN ( self );

    Node = ( const struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )
    XFS_CAN ( Node -> workspace )

    if ( Node -> type != kxfsDir ) {
        File = ( struct KFile * )
                        ( ( struct _EncWsFileEditor * ) self ) -> File;
        if ( File == NULL ) {
            RCt = KDirectorySetFileSize (
                            ( struct KDirectory * ) Node -> workspace,
                            Size,
                            Node -> path
                            );
        }
        else {
            RCt = KFileSetSize ( File, Size );
        }
    }

    return RCt;
}   /*  _EncWsFile_set_size_v1 () */

rc_t CC
_EncWsNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct _EncWsFileEditor * FileEditor;
    struct XFSFileEditor * Editor;

    RCt = 0;
    FileEditor = NULL;
    Editor = NULL;

    if ( self == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    * File = NULL;

    if ( ( ( struct _EncWsNode * ) self ) -> type != kxfsFile ) {
        return XFS_RC ( rcInvalid );
    }

    FileEditor = calloc ( 1, sizeof ( struct _EncWsFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _EncWsFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _EncWsFile_open_v1;
        Editor -> close = _EncWsFile_close_v1;
        Editor -> read = _EncWsFile_read_v1;
        Editor -> write = _EncWsFile_write_v1;
        Editor -> size = _EncWsFile_size_v1;
        Editor -> set_size = _EncWsFile_set_size_v1;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _EncWsNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_EncWsAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_EncWsAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct _EncWsAttrEditor * ) self );
    }

    return 0;
}   /* _EncWsAttr_dispose_v1 () */

/*))    Something unusual. Will check-initialize NativeDir and Node
 //     NativeDir and Node could be NULL
((*/
static
rc_t CC
_EncWsAttr_init_check_v1 (
                    const struct XFSAttrEditor * self,
                    const struct _EncWsNode ** Node,
                    struct KDirectory ** Workspace

)
{
    const struct _EncWsNode * RetNode = NULL;

    XFS_CSAN ( Node )
    XFS_CSAN ( Workspace )
    XFS_CAN ( self )

    RetNode = ( const struct _EncWsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( RetNode -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( RetNode -> workspace == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node != NULL ) {
        * Node = RetNode;
    }

    if ( Workspace != NULL ) {
        * Workspace = ( struct KDirectory * ) RetNode -> workspace;
    }

    return 0;
}   /* _EncWsAttr_init_check_v1 () */

static
rc_t CC
_EncWsAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    const struct _EncWsNode * Node;
    KDirectory * Workspace;
    uint32_t Access;
    char * BF;
    rc_t RCt;

    Node = NULL;
    Workspace = NULL;
    BF = NULL;
    RCt = 0;

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Permissions = NULL;

    RCt = _EncWsAttr_init_check_v1 ( self, & Node, & Workspace );
    if ( RCt == 0 ) {
        RCt = KDirectoryAccess ( Workspace, & Access, Node -> path );
        if ( RCt == 0 ) {
            BF = ( ( struct _EncWsAttrEditor * ) self ) -> perm;
            RCt = XFSPermToChar (
                                Access,
                                BF,
                                sizeof ( ( ( struct _EncWsAttrEditor * ) self ) -> perm )
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

    return RCt;
}   /* _EncWsAttr_permissions_v1 () */

static
rc_t CC
_EncWsAttr_set_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char * Permissions
)
{
    rc_t RCt;
    const struct _EncWsNode * Node;
    struct KDirectory * Workspace;
    uint32_t Access;

    RCt = 0;
    Workspace = NULL;
    Node = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Permissions )

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _EncWsAttr_init_check_v1 ( self, & Node, & Workspace );
    if ( RCt == 0 ) {
        RCt = XFSPermToNum ( Permissions, & Access );
        if ( RCt == 0 ) {
            RCt = KDirectorySetAccess (
                                    Workspace,
                                    false,
                                    Access,
                                    Access,
                                    Node -> path
                                    );

        }
    }

    return RCt;
}   /*  _EncWsAttr_set_permissions_v1 () */

static
rc_t CC
_EncWsAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    const struct _EncWsNode * Node;
    struct KDirectory * Workspace;
    KTime_t TempTime;
    rc_t RCt;

    Node = NULL;
    Workspace = NULL;
    TempTime = 0;
    RCt = 0;

    XFS_CSA ( Time, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Time )

    RCt = _EncWsAttr_init_check_v1 ( self, & Node, & Workspace );
    if ( RCt == 0 ) {
        RCt = KDirectoryDate ( Workspace, & TempTime, Node -> path );
        if ( RCt == 0 ) {
            * Time = TempTime;
        }
    }

    return RCt;
}   /* _EncWsAttr_date_v1 () */

static
rc_t CC
_EncWsAttr_set_date_v1 (
                const struct XFSAttrEditor * self,
                KTime_t Time
)
{
    rc_t RCt;
    const struct _EncWsNode * Node;
    struct KDirectory * Workspace;

    RCt = 0;
    Workspace = NULL;
    Node = NULL;

    XFS_CAN ( self )

    RCt = _EncWsAttr_init_check_v1 ( self, & Node, & Workspace );
    if ( RCt == 0 ) {
        RCt = KDirectorySetDate ( Workspace, true, Time, Node -> path );
    }

    return RCt;
}   /*  _EncWsAttr_set_date_v1 () */

static
rc_t CC
_EncWsAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    const struct _EncWsNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    XFS_CSA ( Type, kxfsFile )
    XFS_CAN ( Type )

    RCt = _EncWsAttr_init_check_v1 ( self, & Node, NULL );
    if ( RCt == 0 ) {
        * Type = Node -> type;
    }

    return RCt;
}   /* _EncWsAttr_type_v1 () */

static
rc_t CC
_EncWsNodeAttr_v1 (
            const struct XFSNode * self,
            const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct _EncWsAttrEditor * EncWsEditor;
    struct XFSAttrEditor * Editor;

    RCt = 0;
    EncWsEditor = NULL;
    Editor = NULL;

    XFS_CSAN ( Attr )
    XFS_CAN ( self )
    XFS_CAN ( Attr )

    EncWsEditor = calloc ( 1, sizeof ( struct _EncWsAttrEditor ) );
    if ( EncWsEditor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    Editor = & ( EncWsEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _EncWsAttr_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> permissions = _EncWsAttr_permissions_v1;
        Editor -> set_permissions = _EncWsAttr_set_permissions_v1;
        Editor -> date = _EncWsAttr_date_v1;
        Editor -> set_date = _EncWsAttr_set_date_v1;
        Editor -> type = _EncWsAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( EncWsEditor );
    }

    return RCt;
}   /* _EncWsNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_EncWsNodeDescribe_v1 (
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

    Abbr = ( ( const struct _EncWsNode * ) self ) -> type == kxfsDir
            ? "ENC DIR"
            : "ENC FILE"
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
}   /* _EncWsNodeDescribe_v1 () */

/*)))
 |||
 +++    WorkspaceNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/
static
rc_t CC
_EncWsNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct _EncWsNode * TheNode;
    const struct KDirectory * Workspace;
    const char * NodeName;

    RCt = 0;
    TheNode = NULL;
    Workspace = NULL;
    NodeName = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

        /* First we should find valid Workspace Directory 
         */
    RCt = XFSEncDirectoryOpen (
                & Workspace,
                ! XFSModelNodeReadOnly ( Template ),
                XFSModelNodeProperty ( Template, XFS_MODEL_PASSWD ),
                XFSModelNodeProperty ( Template, XFS_MODEL_ENCTYPE ),
                XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE )
                );

    if ( RCt == 0 ) {
        RCt = _EncWsNodeMake (
                    & TheNode,
                    Workspace,
                    NodeName,
                    XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE )
                    );
        if ( RCt == 0 ) {
            * Node = ( struct XFSNode * ) TheNode;
        }

        KDirectoryRelease ( Workspace );
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            _EncWsNodeDispose ( TheNode );
        }
    }

    return RCt;
}   /* _EncWsNodeConstructor () */

/*)))
 |||
 +++    Non-Teleport methods to create nodes
 |||
(((*/
LIB_EXPORT
rc_t CC
XFSWorkspaceNodeMake (
            struct XFSNode ** Node,
            const char * Name,
            const char * Path,
            const char * Password,
            const char * EncType,
            bool ReadOnly
)
{
    rc_t RCt;
    struct _EncWsNode * TheNode;
    const struct KDirectory * Workspace;

    RCt = 0;
    TheNode = NULL;
    Workspace = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Name )
    XFS_CAN ( Path )
    XFS_CAN ( Password )
    XFS_CAN ( Node )

    RCt = XFSEncDirectoryOpen (
                            & Workspace,
                            ! ReadOnly,
                            Password,
                            EncType,
                            Path
                            );
    if ( RCt == 0 ) {
        RCt = _EncWsNodeMake ( & TheNode, Workspace, Name, Path );
        if ( RCt == 0 ) {
            * Node = & ( TheNode -> node );
        }

        KDirectoryRelease ( Workspace );
    }

    return RCt;
}   /* XFSWorkspaceNodeMake () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_WorkspaceNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _EncWsNodeConstructor ( Model, Template, Alias, Node );

/*
pLogMsg ( klogDebug, "_WorkspaceNodeConstructor ( $(model), $(template) (\"name\"), \"alias\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _WorkspaceNodeConstructor () */

static
rc_t CC
_WorkspaceNodeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_WorkspaceNodeValidator ( $(model), $(template) (\"name\"), \"alias\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _WorkspaceNodeValidator () */

static const struct XFSTeleport _sWorkspaceNodeTeleport = {
                                            _WorkspaceNodeConstructor,
                                            _WorkspaceNodeValidator,
                                            false
                                            };


LIB_EXPORT
rc_t CC
XFSWorkspaceProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sWorkspaceNodeTeleport;

    return 0;
}   /* XFSWorkspaceProvider () */

