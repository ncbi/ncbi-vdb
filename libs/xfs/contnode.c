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
#include <klib/log.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/perm.h>

#include "mehr.h"
#include "schwarzschraube.h"
#include "ncon.h"
#include "teleport.h"
#include "common.h"
#include "contnode.h"

#include <sysalloc.h>

/*)))
  |||
  +++    ContNode, and other simple containers
  |||
  (((*/

/*)))
 |||
 +++    Simple Container virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _ContNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _ContNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _ContNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _ContNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _ContNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sContNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        XFSNodeDPF_vt_flavor,
                                        _ContNodeDispose_v1,
                                        _ContNodeFindNode_v1,
                                        _ContNodeDir_v1,
                                        NULL,
                                        _ContNodeAttr_v1,
                                        _ContNodeDescribe_v1
                                        };

static
rc_t CC
_ContNodeDispose ( const struct XFSContNode * self )
{
    struct XFSContNode * Container =
                                ( struct XFSContNode * ) self;

/*
pLogMsg ( klogDebug, "_ContNodeDispose ( $(container) )", "container=0x%p", ( void * ) Container );
*/
    if ( Container == 0 ) {
        return 0;
    }

    if ( Container -> container != NULL ) {
        XFSNodeContainerRelease ( Container -> container );
        Container -> container = NULL;
    }

    XFSNodeDPF_Dispose ( ( const struct XFSNode * ) self );

    if ( Container -> disposer != NULL ) {
        Container -> disposer ( Container );
    }
    else {
        free ( Container );
    }

    return 0;
}   /* _ContNodeDispose () */

/*)))
 |||
 +++    Here we are creating and disposing that node
 |||
(((*/

rc_t CC
_ContNodeDispose_v1 ( const struct XFSNode * self )
{
    return _ContNodeDispose ( ( const struct XFSContNode * ) self );
}   /* _ContNodeDispose_v1 () */

/*)))
 (((    First check itself and all others after.
  )))
 (((*/
rc_t CC
_ContNodeFindNode_v1 (
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

            RCt = XFSNodeContainerGet (
                    ( ( const struct XFSContNode * ) self )
                                                        -> container,
                    NodeName,
                    & NextNode
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
}   /* _ContNodeFindNode_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Directory
 (((*/

static
rc_t CC
_ContNodeDir_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSDirEditor * Editor;

    Editor = ( struct XFSDirEditor * ) self;
/*
pLogMsg ( klogDebug, "_ContNodeDir_dispose_ ( $(editor) )", "editor=0x%p", ( void * ) Editor );
*/

    if ( Editor == 0 ) {
        return 0;
    }

    free ( Editor );

    return 0;
}   /* _ContNodeDir_dispose_v1 () */

static
rc_t CC
_ContNodeDir_find_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
)
{
    struct XFSContNode * Cont = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    Cont = ( struct XFSContNode * ) ( self -> Papahen ) . Node;
    if ( Cont == NULL ) {
        return XFS_RC ( rcInvalid );
    }
    return XFSNodeContainerGet ( Cont -> container, Name, Node );
}   /* _ContNodeDir_find_v1 () */

static
rc_t CC
_ContNodeDir_list_v1 (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
)
{
    struct XFSContNode * Cont = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    Cont = ( struct XFSContNode * ) ( self -> Papahen ) . Node;
    if ( Cont == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSNodeContainerList ( Cont -> container, List );
}   /* _ContNodeDir_list_v1 () */

rc_t CC
_ContNodeDir_v1 (
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
                        _ContNodeDir_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> list = _ContNodeDir_list_v1;
        Editor -> find = _ContNodeDir_find_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _ContNodeDir_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Attributes
 (((*/

static
rc_t CC
_ContNodeAttr_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSAttrEditor * Editor;

    Editor = ( struct XFSAttrEditor * ) self;
/*
pLogMsg ( klogDebug, "_ContNodeAttr_dispose_ ( $(editor) )", "editor=0x%p", ( void * ) Editor );
*/

    if ( Editor == 0 ) {
        return 0;
    }

    free ( Editor );

    return 0;
}   /* _ContNodeAttr_dispose_v1 () */

static
rc_t CC
_ContNodeAttr_permissions_v1 (
                            const struct XFSAttrEditor * self,
                            const char ** Permissions
                            )
{
    rc_t RCt = 0;

    XFS_CSAN ( Permissions )
    XFS_CAN ( self )
    XFS_CAN ( Permissions )

    RCt = XFSNodeDPF_Permissions (
                            XFSEditorNode ( & ( self -> Papahen ) ),
                            Permissions
                            );
    if ( RCt == 0 ) {
	    if ( * Permissions == NULL ) {
		    * Permissions = XFSPermRODefContChar ();
        }
    }

    return RCt;
}   /* _ContNodeAttr_permisiions_v1 () */

static
rc_t CC
_ContNodeAttr_type_v1 (
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
}   /* _ContNodeAttr_type_v1 () */

rc_t CC
_ContNodeAttr_v1 (
                    const struct XFSNode * self,
                    const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;

    RCt = 0;

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
                        _ContNodeAttr_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> permissions = _ContNodeAttr_permissions_v1;
        Editor -> date = XFSNodeDPF_vt_date;
        Editor -> set_date = XFSNodeDPF_vt_set_date;
        Editor -> type = _ContNodeAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return 0;
}   /* _ContNodeAttr_v1 () */

rc_t CC
_ContNodeDescribe_v1 (
                        const struct XFSNode * self,
                        char * Buffer,
                        size_t BufferSize
)
{
    rc_t RCt;
    size_t NumWrit;

    RCt = 0;
    NumWrit = 0;

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }
    * Buffer = 0;

    if ( self == NULL ) {
        RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumWrit,
                        "NODE (BASIC CONT)[NULL][NULL]"
                        );
    }
    else {
        RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumWrit,
                        "NODE (BASIC CONT)[%s][0x%p]",
                        self -> Name,
                        self
                        );
    }

    return RCt;
}   /* _ContNodeDescribe_v1 () */

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
XFSContNodeInit (
                struct XFSNode * self,
                const char * Name,
                const char * Perm,
                uint32_t Flavor,
                _tContNodeDisposer Disposer
)
{
    rc_t RCt;
    struct XFSContNode * Cont;

    RCt = 0;
    Cont = ( struct XFSContNode * ) self;

    XFS_CAN ( Cont )

    RCt = XFSNodeInitVT (
                    self,
                    Name,
                    ( const union XFSNode_vt * ) & _sContNodeVT_v1
                    );
    if ( RCt == 0 ) {
        RCt = XFSNodeContainerMake ( ( const struct XFSNodeContainer ** ) & Cont -> container );
        if ( RCt == 0 ) {
            RCt = XFSNodeDPF_Init ( self, Perm, 0, Flavor );
            if ( RCt == 0 ) {
                Cont -> disposer = Disposer;
            }
        }
    }

    return RCt;
}   /* XFSContNodeInit () */

LIB_EXPORT
rc_t CC
XFSContNodeMake (
                const char * Name,
                const char * Perm,
                struct XFSNode ** Node
)
{
    return XFSContNodeMakeWithFlavor (
                                    Name,
                                    Perm,
                                    _sFlavorOfContNode,
                                    Node
                                    );
}   /* XFSContNodeMake () */

LIB_EXPORT
rc_t CC
XFSContNodeMakeWithFlavor (
                const char * Name,
                const char * Perm,
                uint32_t Flavor,
                struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * xNode;

    RCt = 0;
    xNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    xNode = calloc ( 1, sizeof ( struct XFSContNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSContNodeInit ( xNode, Name, Perm, Flavor, NULL );
    if ( RCt == 0 ) {
        * Node = xNode;
    }
    else {
        * Node = NULL;
        if ( xNode != NULL ) {
            XFSNodeDispose ( xNode );
        }
    }
/*
pLogMsg ( klogDebug, "_ContNodeMake ND[$(container)] NM[$(name)]", "container=0x%p,name=%s", ( void * ) Cont, Name );
*/

    return RCt;
}   /* XFSContNodeMakeWithFlavor () */

LIB_EXPORT
rc_t CC
XFSContNodeAddChild (
                struct XFSNode * self,
                const struct XFSNode * Child
)
{
    struct XFSContNode * Cont = ( struct XFSContNode * ) self;

    if ( Cont == NULL || Child == NULL ) {
        return XFS_RC ( rcNull );
    }

    return XFSNodeContainerAdd ( Cont -> container, Child );
}   /* XFSContNodeAddChild () */

LIB_EXPORT
rc_t CC
XFSContNodeDelChild ( struct XFSNode * self, const char * ChildName )
{
    struct XFSContNode * Cont = ( struct XFSContNode * ) self;

    if ( Cont == NULL || ChildName == NULL ) {
        return XFS_RC ( rcNull );
    }

    return XFSNodeContainerDel ( Cont -> container, ChildName );
}   /* XFSContNodeDelChild () */

LIB_EXPORT
rc_t CC
XFSContNodeClear ( struct XFSNode * self )
{
    struct XFSContNode * Cont = ( struct XFSContNode * ) self;

    if ( Cont == NULL ) {
        return XFS_RC ( rcNull );
    }

    return XFSNodeContainerClear ( Cont -> container );
}   /* XFSContNodeClear () */
