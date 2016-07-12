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
#include "xgapk.h"

#include <sysalloc.h>

/*)))
 |||    That file contains _GapKartFilesNode, which is really Kfs
 |||    type node without ability to create subdirectories and with
 |||    callback to changeing version ( datestamp )
(((*/

#define XFS_GAP_KART_FILES_NAME "kart-files"

/*)))
 |||
 +++    FileNode, and other simple containers
 |||
(((*/

struct _KartFilesNode {
    struct XFSNode node;

    const char * path;   /* Path for object */
    uint64_t version;

    const char * perm;   /* Permissions in format "rwxrwxrwx u:g:o" */
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* _GapKartFilesNode is living here                              *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    _GapKartFilesNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _KartFilesNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _KartFilesNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _KartFilesNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _KartFilesNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _KartFilesNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _KartFilesNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sKartFilesNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _KartFilesNodeFlavor_v1,
                                        _KartFilesNodeDispose_v1,
                                        _KartFilesNodeFindNode_v1,
                                        _KartFilesNodeDir_v1,
                                        NULL,   /* NO FILE */
                                        _KartFilesNodeAttr_v1,
                                        _KartFilesNodeDescribe_v1
                                        };

static
rc_t CC
_KartFilesNodeMake (
            struct _KartFilesNode ** Node,
            const char * Name
)
{
    rc_t RCt;
    struct _KartFilesNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    TheNode = calloc ( 1, sizeof ( struct _KartFilesNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInitVT (
                & ( TheNode -> node),
                Name,
                ( const union XFSNode_vt * ) & _sKartFilesNodeVT_v1
                );
        if ( RCt == 0 ) {

                /* This is duplicate, but necessary one
                 */
            TheNode -> node . vt = 
                    ( const union XFSNode_vt * ) & _sKartFilesNodeVT_v1
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
pLogMsg ( klogDebug, "_KartFilesNodeMake ND[$(node)] NM[$(name)]", "node=%p,name=%s", ( void * ) TheNode, Name );
*/

    return RCt;
}   /* _KartFilesNodeMake () */

static
rc_t CC
_KartFilesNodeMakeEx (
            struct _KartFilesNode ** Node,
            const char * Path,
            const char * Perm
)
{
    struct _KartFilesNode * TempNode;
    rc_t RCt;

    RCt = 0;
    TempNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Path )

    RCt = _KartFilesNodeMake ( & TempNode, XFS_GAP_KART_FILES_NAME );
    if ( RCt == 0 ) {
        RCt = XFS_StrDup ( Path, & ( TempNode -> path ) );
        if ( RCt == 0 ) {
            if ( Perm != NULL ) {
                RCt = XFS_StrDup ( Perm, & ( TempNode -> perm ) );
            }
            if ( RCt == 0 ) {
                TempNode -> version = 0;
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
}   /* _KartFilesNodeMakeEx () */

uint32_t CC
_KartFilesNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfGapKartFiles;
}   /* _KartFilesNodeFlavor_v1 () */

static
rc_t CC
_KartFilesNodeDispose ( const struct _KartFilesNode * self )
{
    struct _KartFilesNode * Node = ( struct _KartFilesNode * ) self;

/*
pLogMsg ( klogDebug, "_KartFilesNodeDispose ND[$(node)] NM[$(path)]", "node=%p,path=%s", ( void * ) Node, ( Node == NULL ? "" : Node -> path ) );
*/

    if ( Node != 0 ) {

        if ( Node -> perm != NULL ) {
            free ( ( char * ) Node -> perm );
            Node -> perm = NULL;
        }

        if ( Node -> path != NULL ) {
            free ( ( char * ) Node -> path );
            Node -> path = NULL;
        }

        Node -> version = 0;

        free ( Node );
    }

    return 0;
}   /* _KartFilesNodeDispose () */

rc_t CC
_KartFilesNodeDispose_v1 ( const struct XFSNode * self )
{
    return _KartFilesNodeDispose (
                                ( const struct _KartFilesNode * ) self
                                );
}   /* _KartFilesNodeDispose_v1 () */

/*)))
 |||
 +++  Finding node child
 |||
(((*/

rc_t CC
_KartFilesNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    struct _KartFilesNode * KartsNode;
    struct XFSNode * XfsNode;
    bool IsLast;
    KDirectory * NativeDir;
    const struct XFSPath * xPath, * yPath;
    uint32_t PathType;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    KartsNode = NULL;
    XfsNode = NULL;
    IsLast = false;
    NativeDir = NULL;
    xPath = yPath = NULL;
    PathType = kptNotFound;

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

        KartsNode = ( struct _KartFilesNode * ) self;
        if ( KartsNode -> path == NULL ) {
            return XFS_RC ( rcInvalid );
        }

            /*) Here we are trying to create new node
             (*/
        RCt = XFSPathFrom (
                        Path,
                        PathIndex + 1,
                        & xPath
                        );
        if ( RCt == 0 ) {
            RCt = XFSPathMake (
                            & yPath,
                            true,
                            "%s/%s",
                            KartsNode -> path,
                            XFSPathGet ( xPath )
                            );

            if ( RCt == 0 ) {
                RCt = KDirectoryNativeDir ( & NativeDir );
                if ( RCt == 0 ) {
                    PathType = KDirectoryPathType (
                                                NativeDir,
                                                XFSPathGet ( yPath )
                                                );
                    RCt = PathType == kptFile ? 0 : XFS_RC ( rcInvalid );
                    if ( RCt == 0 ) {
                        RCt = XFSFileNodeMake (
                                            & XfsNode,
                                            XFSPathGet ( yPath ),
                                            XFSPathName ( yPath ),
                                            XFSPermRWDefNodeChar ()
                                            );
                        if ( RCt == 0 ) {
                            * Node = XfsNode;
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
}   /* _KartFilesNodeFindNode_v1 () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_KartFilesDir_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_KartFilesDir_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSDirEditor * ) self );
    }

    return 0;
}   /* _KartFilesDir_dispose_v1 () */

static
rc_t CC
_KartFilesDir_list_v1 (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
)
{
    const struct _KartFilesNode * Node;
    struct KNamelist * TempList;
    rc_t RCt;

    RCt = 0;
    Node = NULL;
    TempList = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    Node = ( const struct _KartFilesNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )

    RCt = XFSGapKartDepotRefresh ();
    if ( RCt == 0 ) {
        RCt = XFSGapKartDepotList ( & TempList, 0 );
        if ( RCt == 0 ) {
            * List = TempList;
        }
        else {
                /* impossible situation, but ...
                 */
            if ( TempList != NULL ) {
                KNamelistRelease ( TempList );
            }
        }
    }

    return RCt;
}   /* _KartFilesDir_list_v1 () */

rc_t CC
_KartFilesDir_find_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
)
{
    const struct _KartFilesNode * KartNode;
    struct XFSNode * TempNode;
    const struct XFSGapKart * Kart;
    rc_t RCt;

    RCt = 0;
    KartNode = NULL;
    TempNode = NULL;
    Kart = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    KartNode = ( const struct _KartFilesNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( KartNode )
    XFS_CAN ( KartNode -> path )

    RCt = XFSGapKartDepotRefresh ();
    if ( RCt == 0 ) {
        RCt = XFSGapKartDepotGet ( & Kart, Name );
        if ( RCt == 0 ) {
            RCt = XFSFileNodeMake (
                                & TempNode,
                                XFSGapKartPath ( Kart ),
                                Name,
                                XFSPermRWDefNodeChar ()
                                );
            if ( RCt == 0 ) {
                * Node = ( const struct XFSNode * ) TempNode;
            }

            XFSGapKartRelease ( Kart );
        }
    }

    return RCt;
}   /* _KartFilesDir_find_v1 () */

static
rc_t CC
_KartFilesDir_create_file_v1 (
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
    struct _KartFilesNode * KartFilesNode;
    struct XFSNode * TempNode;
    const struct XFSHandle * TempHandle;
    const struct XFSPath * Path;
    rc_t RCt;

    NativeDir = NULL;
    File = NULL;
    Update = false;
    CreateMode = kcmCreate;
    KartFilesNode = NULL;
    TempNode = NULL;
    TempHandle = NULL;
    Path = NULL;
    RCt = 0;

    XFS_CSAN ( Handle )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Handle )

    KartFilesNode = ( struct _KartFilesNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( KartFilesNode )
    XFS_CAN ( KartFilesNode -> path )

    Update = Mode == kxfsReadWrite;
    CreateMode = kcmCreate;

    RCt = XFSPathMake (
                    & Path,
                    false,
                    "%s/%s",
                    KartFilesNode -> path,
                    Name
                    );
    if ( RCt == 0 ) {
        /* Here we are */
        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            RCt = KDirectoryCreateFile (
                                    NativeDir,
                                    & File, 
                                    Update,
                                    XFSPermRWDefNodeNum (),
                                    CreateMode,
                                    XFSPathGet ( Path )
                                    );
            if ( RCt == 0 ) {
                RCt = XFSFileNodeMake (
                                    & TempNode,
                                    XFSPathGet ( Path ),
                                    Name,
                                    XFSPermRWDefNodeChar ()
                                    );
                if ( RCt == 0 ) {
                    RCt = XFSFileNodeMakeHandle (
                                                & TempHandle,
                                                TempNode,
                                                File
                                                );
                    if ( RCt == 0 ) {
                        * Handle = TempHandle;
                    }
                }
            }

            KDirectoryRelease ( NativeDir );
        }

        XFSPathRelease ( Path );
    }

    if ( RCt != 0 ) {
        * Handle = NULL;

        if ( TempHandle != NULL ) {
            XFSHandleRelease ( TempHandle );
        }

        if ( File != NULL ) {
            KFileRelease ( File );
        }
    }

    return RCt;
}   /* _KartFilesDir_create_file_v1 () */

static
rc_t CC
_KartFilesDir_delete_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name
)
{
    rc_t RCt;
    KDirectory * NativeDir;
    struct _KartFilesNode * KartsNode;
    const struct XFSPath * Path;

    RCt = 0;
    NativeDir = NULL;
    KartsNode = NULL;
    Path = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Name )

    KartsNode = ( struct _KartFilesNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( KartsNode )
    XFS_CAN ( KartsNode -> path )

    RCt = XFSPathMake (
                        & Path,
                        false,
                        "%s/%s",
                        KartsNode -> path,
                        Name
                        );
    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            RCt = KDirectoryRemove (
                                    NativeDir,
                                    true,
                                    XFSPathGet ( Path )
                                    );
            KDirectoryRelease ( NativeDir );
        }
        XFSPathRelease ( Path );
    }

    return RCt;
}   /* _KartFilesDir_delete_v1 () */

rc_t CC
_KartFilesNodeDir_v1 (
                    const struct XFSNode * self,
                    const struct XFSDirEditor ** Dir
)
{
    rc_t RCt;
    struct XFSDirEditor * Editor;

    RCt = 0;
    Editor = NULL;

    XFS_CSAN ( Dir )
    XFS_CAN ( self )
    XFS_CAN ( Dir )

    Editor = calloc ( 1, sizeof ( struct XFSDirEditor ) );
    if ( Editor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                        & ( Editor -> Papahen ),
                        self,
                        _KartFilesDir_dispose_v1
                        );

    if ( RCt == 0 ) {
        Editor -> list = _KartFilesDir_list_v1;
        Editor -> find = _KartFilesDir_find_v1;
        Editor -> create_file = _KartFilesDir_create_file_v1;
        Editor -> delete = _KartFilesDir_delete_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _KartFilesNodeDir_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_KartFilesAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_KartFilesAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _KartFilesAttr_dispose_v1 () */

static
rc_t CC
_KartFilesAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    const struct _KartFilesNode * Node = NULL;

    XFS_CSAN ( Permissions )
    XFS_CAN ( self )
    XFS_CAN ( Permissions )

    Node = ( const struct _KartFilesNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )

    * Permissions = Node -> perm;
    if ( * Permissions == NULL ) {
        * Permissions = XFSPermRWDefContChar ();
    }

    return 0;
}   /* _KartFilesAttr_permissions_v1 () */

static
rc_t CC
_KartFilesAttr_date_v1 ( const struct XFSAttrEditor * self, KTime_t * Time )
{
    const struct _KartFilesNode * Node;
    KDirectory * NativeDir;
    KTime_t TempTime;
    rc_t RCt;

    Node = NULL;
    NativeDir = NULL;
    TempTime = 0;
    RCt = 0;

    XFS_CSA ( Time, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Time )

    Node = ( const struct _KartFilesNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryDate ( NativeDir, & TempTime, Node -> path );
        if ( RCt == 0 ) {
            * Time = TempTime;
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _KartFilesAttr_date_v1 () */

static
rc_t CC
_KartFilesAttr_type_v1 (
                    const struct XFSAttrEditor * self,
                    XFSNType * Type
)
{
    XFS_CSA ( Type, kxfsNotFound )
    XFS_CAN ( self )
    XFS_CAN ( Type )

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    * Type = kxfsDir;

    return 0;
}   /* _KartFilesAttr_type_v1 () */

static
rc_t CC
_KartFilesNodeAttr_v1 (
                        const struct XFSNode * self,
                        const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;

    RCt = 0;
    Editor = NULL;

    XFS_CSAN ( Attr )
    XFS_CAN ( self )
    XFS_CAN ( Attr )

    Editor = calloc ( 1, sizeof ( struct XFSAttrEditor ) );
    if ( Editor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _KartFilesAttr_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> permissions = _KartFilesAttr_permissions_v1;
        Editor -> date = _KartFilesAttr_date_v1;
        Editor -> type = _KartFilesAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _KartFilesNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_KartFilesNodeDescribe_v1 (
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

    XFS_CAN ( Buffer )
    XFS_CA ( BufferSize, 0 )

    * Buffer = 0;

    Abbr = "KART-FILES";

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
}   /* _KartFilesNodeDescribe_v1 () */

/*))
 ((     Node make/dispose
  ))
 ((*/

/*)))
 |||
 +++    Non-Teleport methods to create nodes
 |||
(((*/

LIB_EXPORT
rc_t CC
XFSGapKartFilesNodeMake (
            struct XFSNode ** Node,
            const char * Perm
)
{
    rc_t RCt;
    struct _KartFilesNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    RCt = _KartFilesNodeMakeEx (
                                & TheNode,
                                XFSGapKartDepotPath (),
                                Perm
                                );
    if ( RCt == 0 ) {
        * Node = & ( TheNode -> node );
    }

    return RCt;
}   /* XFSDirNodeMake () */

/*)))
 |||
 +++    KartFilesNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_KartFilesNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct _KartFilesNode * TheNode;
    const char * NodePath;

    RCt = 0;
    TheNode = NULL;
    NodePath = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    NodePath = XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE );
    if ( NodePath == NULL ) {
        NodePath = XFSGapKartDepotPath ();
    }

    RCt = _KartFilesNodeMakeEx (
                                & TheNode,
                                NodePath,
                                XFSModelNodeSecurity ( Template )
                                );
    if ( RCt == 0 ) {
        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            _KartFilesNodeDispose ( TheNode );
        }
    }

/*
pLogMsg ( klogDebug, "_KartFilesNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _KartFilesNodeConstructor () */

static
rc_t CC
_KartFilesNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_KartFilesNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _KartFilesNodeValidator () */


static const struct XFSTeleport _sKartFilesNodeTeleport = {
                                            _KartFilesNodeConstructor,
                                            _KartFilesNodeValidator,
                                            false
                                            };


LIB_EXPORT
rc_t CC
XFSGapKartFilesProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sKartFilesNodeTeleport;

    return 0;
}   /* XFSGapKartFilesProvider () */
