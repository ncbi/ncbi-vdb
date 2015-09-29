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
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/perm.h>

#include "mehr.h"
#include "schwarzschraube.h"
#include "xcache.h"
#include "common.h"

#include <sysalloc.h>

#include <stdio.h>

/*)))
  |||
  +++    GapCacheNode - represent special node which shows content
  ***    of cached DbGap files by project, or, if project is not set,
  +++    it shows content of public cache.
  |||
  (((*/

/*)))
 |||
 +++    GapCache structure and virtual table are Living here :lol:
 |||
(((*/
struct _CacheNode {
    struct XFSNode node;

    struct XFSGapCache * cache;

    const char * perm;
    uint32_t flavor;
};

static rc_t CC _CacheNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _CacheNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _CacheNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _CacheNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _CacheNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _CacheNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sCacheNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _CacheNodeFlavor_v1,
                                        _CacheNodeDispose_v1,
                                        _CacheNodeFindNode_v1,
                                        _CacheNodeDir_v1,
                                        NULL,
                                        _CacheNodeAttr_v1,
                                        _CacheNodeDescribe_v1
                                        };

static
rc_t CC
_CacheNodeDispose ( const struct _CacheNode * self )
{
    struct _CacheNode * Node =
                                ( struct _CacheNode * ) self;

/*
printf ( "_CacheNodeDispose ( 0x%p )\n", ( void * ) Node );
*/

    if ( Node != NULL ) {
        if ( Node -> cache != NULL ) {
            XFSGapCacheDispose ( Node -> cache );
            Node -> cache = NULL;
        }

        if ( Node -> perm != NULL ) {
            free ( ( char * ) Node -> perm );
            Node -> perm = NULL;
        }

        Node -> flavor = _sFlavorLess;

        free ( Node );
    }

    return 0;
}   /* _CacheNodeDispose () */

/*)))
 |||
 +++    Here we are creating and disposing that node
 |||
(((*/

uint32_t CC
_CacheNodeFlavor_v1 ( const struct XFSNode * self )
{
    return ( ( const struct _CacheNode * ) self ) -> flavor;
}   /* _CacheNodeFlavor_v1 () */

rc_t CC
_CacheNodeDispose_v1 ( const struct XFSNode * self )
{
    return _CacheNodeDispose ( ( const struct _CacheNode * ) self );
}   /* _CacheNodeDispose_v1 () */

/*)))
 (((    First check itself and all others after.
  )))
 (((*/
rc_t CC
_CacheNodeFindNode_v1 (
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
    const struct XFSNode * NextNode;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    IsLast = false;
    NextNode = NULL;

        /* Standard check-initialization */
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

            /* If node is last - that is */
        if ( IsLast ) {
            RCt = XFSNodeAddRef ( self );

            * Node = self;

            return RCt;
        }

            /*  Here we aren't last and looking forward; */
        NodeName = XFSPathPartGet ( Path, PathIndex + 1 );

            /*))  Should not happen thou
             ((*/
        if ( NodeName == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {

            RCt = XFSGapCacheFind (
                    ( ( const struct _CacheNode * ) self ) -> cache,
                    & NextNode,
                    NodeName
                    );
            if ( RCt == 0 ) {

                RCt = XFSNodeFindNode (
                                    NextNode,
                                    Path,
                                    PathIndex + 1,
                                    Node
                                    );
            }
        }
    }

    return RCt;
}   /* _CacheNodeFindNode_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Directory
 (((*/

static
rc_t CC
_CacheNodeDir_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSDirEditor * Editor = ( struct XFSDirEditor * ) self;
/*
printf ( "_CacheNodeDir_dispose_ ( 0x%p )\n", ( void * ) Editor );
*/

    if ( Editor != NULL ) {
        free ( Editor );
    }

    return 0;
}   /* _CacheNodeDir_dispose_v1 () */

static
rc_t CC
_CacheNodeDir_find_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
)
{
    struct _CacheNode * Cache = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    Cache = ( struct _CacheNode * ) ( self -> Papahen ) . Node;
    if ( Cache == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSGapCacheFind ( Cache -> cache, Node, Name );
}   /* _CacheNodeDir_find_v1 () */

static
rc_t CC
_CacheNodeDir_list_v1 (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
)
{
    struct _CacheNode * Cache = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    Cache = ( struct _CacheNode * ) ( self -> Papahen ) . Node;
    if ( Cache == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSGapCacheList ( Cache -> cache, List );
}   /* _CacheNodeDir_list_v1 () */

static
rc_t CC
_CacheNodeDir_delete_v1 (
                        const struct XFSDirEditor * self,
                        const char * NodeName
)
{
    struct _CacheNode * Cache;
    bool ReadOnly;

    Cache = NULL;
    ReadOnly = true;

    XFS_CAN ( self )
    XFS_CAN ( NodeName )

    Cache = ( struct _CacheNode * ) ( self -> Papahen ) . Node;
    if ( Cache == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    XFSGapCacheReadOnly ( Cache -> cache, & ReadOnly );
    if ( ReadOnly ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSGapCacheDeleteNode ( Cache -> cache, NodeName );
}   /* _CacheNodeDir_delete_v1 () */

rc_t CC
_CacheNodeDir_v1 (
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
                        _CacheNodeDir_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> list = _CacheNodeDir_list_v1;
        Editor -> find = _CacheNodeDir_find_v1;
        Editor -> delete = _CacheNodeDir_delete_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _CacheNodeDir_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  And, Attributes
 (((*/

static
rc_t CC
_CacheNodeAttr_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSAttrEditor * Editor = ( struct XFSAttrEditor * ) self;
/*
printf ( "_CacheNodeAttr_dispose_ ( 0x%p )\n", ( void * ) Editor );
*/

    if ( Editor != 0 ) {
        free ( Editor );
    }

    return 0;
}   /* _CacheNodeAttr_dispose_v1 () */

static
rc_t CC
_CacheNodeAttr_permissions_v1 (
                            const struct XFSAttrEditor * self,
                            const char ** Permissions
                            )
{
    const struct XFSNode * Node = NULL;

    XFS_CSAN ( Permissions )
    XFS_CAN ( self )
    XFS_CAN ( Permissions )

    Node = XFSEditorNode ( & ( self -> Papahen ) );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    * Permissions = ( ( struct _CacheNode * ) Node ) -> perm;

	if ( * Permissions == NULL ) {
		* Permissions = XFSPermRODefContChar ();
	}

    return 0;
}   /* _CacheNodeAttr_permisiions_v1 () */

static
rc_t CC
_CacheNodeAttr_date_v1 (
                            const struct XFSAttrEditor * self,
                            KTime_t * Time
)
{
    XFS_CSA ( Time, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Time )

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }
/* TODO - return process start time */

    return 0;
}   /* _CacheNodeAttr_date_v1 () */

static
rc_t CC
_CacheNodeAttr_type_v1 (
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
}   /* _CacheNodeAttr_type_v1 () */

rc_t CC
_CacheNodeAttr_v1 (
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
                        _CacheNodeAttr_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> permissions = _CacheNodeAttr_permissions_v1;
        Editor -> date = _CacheNodeAttr_date_v1;
        Editor -> type = _CacheNodeAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return 0;
}   /* _CacheNodeAttr_v1 () */

rc_t CC
_CacheNodeDescribe_v1 (
                        const struct XFSNode * self,
                        char * Buffer,
                        size_t BufferSize
)
{
    rc_t RCt;
    size_t NumWrit;

    RCt = 0;
    NumWrit = 0;

    XFS_CAN ( Buffer )
    XFS_CA ( BufferSize, 0 )

    * Buffer = 0;

    if ( self == NULL ) {
        RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumWrit,
                        "NODE (GAP CACHE)[NULL][NULL]"
                        );
    }
    else {
        RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumWrit,
                        "NODE (GAP CACHE)[%s][0x%p]",
                        self -> Name,
                        self
                        );
    }

    return RCt;
}   /* _CacheNodeDescribe_v1 () */

/*)))
 |||
 +++    Simple Container lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

LIB_EXPORT
rc_t CC
_CacheNodeMake (
                struct _CacheNode ** Node,
                uint32_t ProjectId,
                bool ReadOnly,
                const char * Name,
                const char * Perm
)
{
    rc_t RCt;
    struct _CacheNode * Cache;
    const char * TheName;

    RCt = 0;
    Cache = NULL;
    TheName = Name;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    Cache = calloc ( 1, sizeof ( struct _CacheNode ) );
    if ( Cache == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        if ( TheName == NULL ) {
            TheName = ProjectId == 0 ? "public" : "cache";
        }

        RCt = XFSNodeInitVT (
                        & ( Cache -> node ),
                        TheName,
                        ( const union XFSNode_vt * ) & _sCacheNodeVT_v1
                        );
        if ( RCt == 0 ) {
            Cache -> flavor = ProjectId == 0 
                                    ? _sFlavorOfGapProjectCache
                                    : _sFlavorOfGapPublicCache
                                    ;

            RCt = XFS_StrDup ( Perm, & ( Cache -> perm ) );
            if ( RCt == 0 ) {
                RCt = XFSGapCacheMake (
                                    & ( Cache -> cache ),
                                    ProjectId,
                                    ReadOnly
                                    );
                if ( RCt == 0 ) {
                    * Node = Cache;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( Cache != NULL ) {
            XFSNodeDispose ( & ( Cache -> node ) );
        }
    }

    return RCt;
}   /* _CacheNodeMake () */

LIB_EXPORT
rc_t CC
XFSGapCacheNodeMake (
                    struct XFSNode ** Node,
                    uint32_t ProjectId,
                    bool ReadOnly,
                    const char * Name,
                    const char * Perm
)
{
    rc_t RCt;
    struct _CacheNode * Cache;

    RCt = 0;
    Cache = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    RCt = _CacheNodeMake ( & Cache, ProjectId, ReadOnly, Name, Perm );
    if ( RCt == 0 ) {
        * Node = & ( Cache -> node );
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( Cache != NULL ) {
            XFSNodeDispose ( & ( Cache -> node ) );
        }
    }

    return RCt;
}   /* XFSGapCacheNodeMake () */

