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
#include "xgap.h"

#include <sysalloc.h>

/*)))
 |||    That file contains third approach to project GAP cached
 |||    private and public files
(((*/

/*)))
 |||
 +++    XFSCacheDirNode, and other simple containers
 |||
(((*/
#define XFS_GAP_PUBLIC_CACHE_NAME "public"
#define XFS_GAP_CACHE_NAME "cache"

struct _CacheDirNode {
    struct XFSNode node;

    uint32_t project_id; /* GaP project Id */
    const char * path;   /* Path for object */
    const char * perm;   /* Permissions in format "rwxrwxrwx u:g:o" */
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* CacheDirNode is living here                                   *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    CacheDirNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _CacheDirNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _CacheDirNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _CacheDirNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _CacheDirNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _CacheDirNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _CacheDirNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sCacheDirNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _CacheDirNodeFlavor_v1,
                                        _CacheDirNodeDispose_v1,
                                        _CacheDirNodeFindNode_v1,
                                        _CacheDirNodeDir_v1,
                                        NULL,   /* NO FILE */
                                        _CacheDirNodeAttr_v1,
                                        _CacheDirNodeDescribe_v1
                                        };

static
rc_t CC
_GapCacheGetPath ( const char ** CachePath, uint32_t ProjectId )
{
    rc_t RCt;
    const struct XFSGapProject * Project;
    char * RepositoryRoot;

    RCt = 0;
    Project = NULL;
    RepositoryRoot = NULL;

    XFS_CSAN ( CachePath )
    XFS_CAN ( CachePath )

    RCt = XFSGapFindOrCreate ( ProjectId, & Project );
    if ( RCt == 0 ) {
        RCt = XFSGapProjectRepositoryRoot ( Project, & RepositoryRoot );
        if ( RCt == 0 ) {
            * CachePath = RepositoryRoot;
        }
        XFSGapProjectRelease ( Project );
    }

    return RCt;
}   /* _GapCacheGetPath () */

static
rc_t CC
_CacheDirNodeMake (
            struct _CacheDirNode ** Node,
            const char * Path,
            const char * Name,
            const char * Perm
)
{
    rc_t RCt;
    struct _CacheDirNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )
    XFS_CAN ( Path )

    TheNode = calloc ( 1, sizeof ( struct _CacheDirNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInitVT (
                & ( TheNode -> node),
                Name,
                ( const union XFSNode_vt * ) ( & _sCacheDirNodeVT_v1 )
                );
        if ( RCt == 0 ) {

            RCt = XFS_StrDup ( Path, & ( TheNode -> path ) );
            if ( RCt == 0 ) {
                if ( Perm != NULL ) {
                    RCt = XFS_StrDup ( Perm, & ( TheNode -> perm ) );
                }
                if ( RCt == 0 ) {
                    * Node = TheNode;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Node = TheNode;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( & ( TheNode -> node ) );
            TheNode = NULL;
        }
    }

    return RCt;
}   /* _CacheDirNodeMake () */

uint32_t CC
_CacheDirNodeFlavor_v1 ( const struct XFSNode * self )
{
    if ( self != NULL ) {
        return ( ( struct _CacheDirNode * ) self ) -> project_id == 0
                                            ? _sFlavorOfGapPublicCache
                                            : _sFlavorOfGapProjectCache
                                            ;
    }
    return _sFlavorLess;
}   /* _CacheDirNodeFlavor_v1 () */

static
rc_t CC
_CacheDirNodeDispose ( const struct _CacheDirNode * self )
{
    struct _CacheDirNode * Node = ( struct _CacheDirNode * ) self;

/*
pLogMsg ( klogDebug, "_CacheDirNodeDispose ( $(node) ) [T=$(project_id)]", "node=%p,project_id=%d", ( void * ) Node, ( Node == NULL ? 0 : Node -> project_id ) );
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

        Node -> project_id = 0;

        free ( Node );
    }

    return 0;
}   /* _CacheDirNodeDispose () */

rc_t CC
_CacheDirNodeDispose_v1 ( const struct XFSNode * self )
{
    return _CacheDirNodeDispose ( ( struct _CacheDirNode * ) self );
}   /* _KfsNodeDispose_v1 () */

/*)))
 |||
 +++  Find operation
 |||
(((*/

static
rc_t CC
_CacheDirNodeForPath (
                    const struct XFSNode ** Node,
                    const char * Path,
                    const char * Name,
                    uint32_t ProjectId
)
{
    rc_t RCt;
    const struct XFSPath * xPath;
    struct KDirectory * NativeDir;
    uint32_t Type;
    struct XFSNode * TheNode;

    RCt = 0;
    xPath = NULL;
    NativeDir = NULL;
    Type = kptNotFound;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Path )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    RCt = XFSPathMake ( & xPath, true, "%s/%s", Path, Name );
    if ( RCt == 0 ) {
        RCt = KDirectoryNativeDir ( & NativeDir );
        if ( RCt == 0 ) {
            Type = KDirectoryPathType (
                                    NativeDir,
                                    XFSPathGet ( xPath )
                                    );
            switch ( Type ) {
                case kptFile :
                        RCt = XFSFileNodeMake (
                                            & TheNode,
                                            XFSPathGet ( xPath ),
                                            XFSPathName ( xPath ),
                                            NULL
                                            );
                        break;
                case kptDir :
                        _CacheDirNodeMake (
                                ( struct _CacheDirNode **  ) & TheNode,
                                XFSPathGet ( xPath ),
                                XFSPathName ( xPath ),
                                NULL
                                );
                        ( ( struct _CacheDirNode * ) TheNode ) -> project_id = ProjectId;
                        break;
                default :
                        RCt = XFS_RC ( rcInvalid );
                        break;
            }
            if ( RCt == 0 ) {
                * Node = TheNode;
            }

            KDirectoryRelease ( NativeDir );
        }

        XFSPathRelease ( xPath );
    }

    return RCt;
}   /* _CacheDirNodeForPath () */

rc_t CC
_CacheDirNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    struct _CacheDirNode * CacheNode;
    bool IsLast;
    const struct XFSPath * xPath;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    CacheNode = NULL;
    IsLast = false;
    xPath = NULL;

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

        CacheNode = ( struct _CacheDirNode * ) self;
        if ( CacheNode -> path == NULL ) {
            return XFS_RC ( rcInvalid );
        }

            /*) Here we are trying to create new node
             (*/
        RCt = XFSPathFrom ( Path, PathIndex + 1, & xPath );
        if ( RCt == 0 ) {
            RCt = _CacheDirNodeForPath (
                                        Node,
                                        CacheNode -> path,
                                        XFSPathGet ( xPath ),
                                        CacheNode -> project_id
                                        );
            XFSPathRelease ( xPath );
        }
    }

    return RCt;
}   /* _CacheDirNodeFindNode () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_CacheDir_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_CacheDir_dispose_v1 ( $(editor) )\n", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSDirEditor * ) self );
    }

    return 0;
}   /* _KfsDir_dispose_v1 () */

static
bool CC
_CacheDirListCallback (
                    const struct KDirectory * self,
                    const char * Name,
                    void * Unused
)
{
    if ( XFS_StrEndsWith ( Name, ".cache" ) ) {
        return false;
    }

    if ( XFS_StrEndsWith ( Name, ".lock" ) ) {
        return false;
    }

    if ( strcmp ( Name, "workspace" ) == 0 ) {
        return false;
    }

    return true;
}   /* _CacheDirListCallback () */

static
rc_t CC
_CacheDir_list_v1 (
                const struct XFSDirEditor * self,
                const struct KNamelist ** List
)
{
    KDirectory * NativeDir;
    const struct _CacheDirNode * Node;
    struct KNamelist * TempList;
    rc_t RCt;

    RCt = 0;
    NativeDir = NULL;
    Node = NULL;
    TempList = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    Node = ( const struct _CacheDirNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryList (
                            NativeDir,
                            & TempList,
                            _CacheDirListCallback,
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
}   /* _CacheDir_list_v1 () */

static
rc_t CC
_CacheDir_find_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    const struct _CacheDirNode * CacheNode;
    rc_t RCt;

    RCt = 0;
    CacheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    * Node = NULL;

    CacheNode = ( const struct _CacheDirNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    XFS_CAN ( CacheNode )
    XFS_CAN ( CacheNode -> path )

    RCt = _CacheDirNodeForPath (
                                Node,
                                CacheNode -> path,
                                Name,
                                CacheNode -> project_id
                                );

    return RCt;
}   /* _CacheDir_find_v1 () */

static
rc_t CC
_CacheDir_delete_v1 (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    rc_t RCt;
    KDirectory * NativeDir;
    struct _CacheDirNode * CacheNode;

    RCt = 0;
    NativeDir = NULL;
    CacheNode = NULL;

    XFS_CAN ( self )
    XFS_CAN ( Name )

    CacheNode = ( struct _CacheDirNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( CacheNode )
    XFS_CAN ( CacheNode -> path )

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryRemove (
                                NativeDir,
                                true,
                                "%s/%s",
                                CacheNode -> path,
                                Name
                                );

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _CacheDir_delete_v1 () */

rc_t CC
_CacheDirNodeDir_v1 (
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
                    _CacheDir_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> list = _CacheDir_list_v1;
        Editor -> find = _CacheDir_find_v1;
        Editor -> delete = _CacheDir_delete_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _CacheDirNodeDir_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_CacheAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_CacheAttr_dispose_v1 ( $(editor) )\n", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _CacheAttr_dispose_v1 () */

static
rc_t CC
_CacheAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    const struct _CacheDirNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    XFS_CSAN ( Permissions )
    XFS_CAN ( self )
    XFS_CAN ( Permissions )

    Node = ( const struct _CacheDirNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )

    * Permissions = Node -> perm == NULL
                                    ? XFSPermRWDefContChar ()
                                    : Node -> perm
                                    ;

    return RCt;
}   /* _CacheAttr_permissions_v1 () */

static
rc_t CC
_CacheAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    const struct _CacheDirNode * Node;
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

    Node = ( const struct _CacheDirNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )

    RCt = KDirectoryNativeDir ( & NativeDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryDate ( NativeDir, & TempTime, Node -> path );
        if ( RCt == 0 ) {
            * Time = TempTime;
        }

        KDirectoryRelease ( NativeDir );
    }

    return RCt;
}   /* _CacheAttr_date_v1 () */

static
rc_t CC
_CacheAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    const struct _CacheDirNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    XFS_CSA ( Type, kxfsNotFound )
    XFS_CAN ( self )
    XFS_CAN ( Type )

    Node = ( const struct _CacheDirNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CAN ( Node )
    XFS_CAN ( Node -> path )

    * Type = kxfsDir;

    return RCt;
}   /* _CacheAttr_type_v1 () */

static
rc_t CC
_CacheDirNodeAttr_v1 (
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
                        _CacheAttr_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> permissions = _CacheAttr_permissions_v1;
        Editor -> date = _CacheAttr_date_v1;
        Editor -> type = _CacheAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _CacheDirNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_CacheDirNodeDescribe_v1 (
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

    Abbr = ( ( const struct _CacheDirNode * ) self ) -> project_id == 0
                ? "CACHE PUB"
                : "CACHE PRJ"
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
}   /* _CacheDirNodeDescribe_v1 () */

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_CacheDirNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * TempStr;
    uint32_t ProjectId;

    RCt = 0;
    TheNode = NULL;
    TempStr = NULL;
    ProjectId = 0;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )


    TempStr = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( TempStr == NULL ) {
        return XFS_RC ( rcInvalid );
    }
    ProjectId = atol ( TempStr );

    RCt = XFSGapCacheNodeMake (
                     & TheNode,
                     ProjectId,
                     XFSModelNodeSecurity ( Template )
                     );
    if ( RCt == 0 ) {
        * Node = TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
    }

    return RCt;
}   /* _CacheDirNodeConstructor () */

/*)))
 |||
 +++    Non-Teleport methods to create nodes
 |||
(((*/

LIB_EXPORT
rc_t CC
XFSGapCacheNodeMake (
            struct XFSNode ** Node,
            uint32_t ProjectId,
            const char * Perm
)
{
    rc_t RCt;
    struct _CacheDirNode * TheNode;
    const char * Path;
    const char * Name;

    RCt = 0;
    TheNode = NULL;
    Path = NULL;
    Name = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    RCt = _GapCacheGetPath ( & Path, ProjectId );
    if ( RCt == 0 ) {
        Name = ProjectId == 0
                    ? XFS_GAP_PUBLIC_CACHE_NAME
                    : XFS_GAP_CACHE_NAME
                    ;
        RCt = _CacheDirNodeMake ( & TheNode, Path, Name, Perm );
        if ( RCt == 0 ) {
            TheNode -> project_id = ProjectId;

            * Node = & ( TheNode -> node );
        }

        free ( ( char * ) Path );
    }

    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( & ( TheNode -> node ) );
            TheNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "XFSKfsNodeMake ND[$(node)] NM[$(name)] TP[$(project_id)]", "nnode=%p,name=%s,project_id=%d", ( void * ) TheNode, Name, ProjectId );
*/

    return RCt;
}   /* XFSGapCacheNodeMake () */

/*)))
 |||
 +++    DirNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_GapCacheNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _CacheDirNodeConstructor ( Model, Template, Alias, Node );

/*
pLogMsg ( klogDebug, "_GapCacheNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )\n", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapCacheNodeConstructor () */

static
rc_t CC
_GapCacheNodeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_GapCacheNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )\n", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapCacheNodeValidator () */

static const struct XFSTeleport _sGapCacheNodeTeleport = {
                                            _GapCacheNodeConstructor,
                                            _GapCacheNodeValidator,
                                            false
                                            };


LIB_EXPORT
rc_t CC
XFSGapCacheProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sGapCacheNodeTeleport;

    return 0;
}   /* XFSGapCacheProvider () */
