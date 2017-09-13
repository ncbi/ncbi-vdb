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
#include <klib/time.h>

#include <kfs/file.h>
#include <kfs/directory.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/handle.h>
#include <xfs/perm.h>
#include <xfs/access.h>
#include <xfs/doc.h>

#include "node-dpf.h"
#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"
#include "xgapk.h"

#include <sysalloc.h>

/*)))
 |||    That file contains _AccessPanelNode, which is really view
 |||    of ~/.ncbi/dbGaP-XXX.acl file
 |||    Also AccessAgent is also living here :LOL:
(((*/

/*)))
 |||
 +++    FileNode, and other simple containers
 |||
(((*/

#define XFS_ACCESS_PANEL_NAME "access-panel"

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* _AccessAgent is living here                                   *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_AccessAgentNodeMake (
                const struct XFSNode ** Node,
                const char * AgentName

)
{
    rc_t RCt;
    struct XFSNode * xNode;
    struct XFSDoc * xDoc;

    RCt = 0;
    xNode = NULL;
    xDoc = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( AgentName )

    RCt = XFSTextDocMake ( & xDoc );
    if ( RCt == 0 ) {
        RCt = XFSTextDocAppend ( xDoc, "USER: %s\n", AgentName );
        if ( RCt == 0 ) {
            RCt = XFSDocNodeMakeWithFlavor (
                                    & xNode,
                                    xDoc,
                                    AgentName,
                                    XFSPermRODefNodeChar (),
                                    _sFlavorOfAccessAgent
                                    );
        }

        XFSDocRelease ( xDoc );
    }

    if ( RCt == 0 ) {
        * Node = xNode;
    }
    else {
        if ( xNode != 0 ) {
            XFSNodeDispose ( xNode );
        }
    }

    return RCt;
}   /* _AccessAgentNodeMake () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* _AccessPanelNode is living here                               *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    FileNode, and other simple containers
 |||
(((*/

struct _AccessPanelNode {
    struct XFSNodeDPF node;
};

/*)))
 |||
 +++    _AccessPanelNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _AccessPanelNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _AccessPanelNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _AccessPanelNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _AccessPanelNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _AccessPanelNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sAccessPanelNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        XFSNodeDPF_vt_flavor,
                                        _AccessPanelNodeDispose_v1,
                                        _AccessPanelNodeFindNode_v1,
                                        _AccessPanelNodeDir_v1,
                                        NULL,   /* NO FILE */
                                        _AccessPanelNodeAttr_v1,
                                        _AccessPanelNodeDescribe_v1
                                        };


static
rc_t CC
_AccessPanelNodeMake ( struct _AccessPanelNode ** Node )
{
    rc_t RCt;
    struct _AccessPanelNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    TheNode = calloc ( 1, sizeof ( struct _AccessPanelNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeDPF_Init (
                ( struct XFSNode * ) TheNode,
                XFSPermRODefContChar (),
                KTimeStamp(),
                _sFlavorOfAccessPanel
                );
        if ( RCt == 0 ) {
            if ( RCt == 0 ) {
                RCt = XFSNodeInitVT (
                        ( struct XFSNode * ) TheNode,
                        XFS_ACCESS_PANEL_NAME,
                        ( const union XFSNode_vt * ) & _sAccessPanelNodeVT_v1
                        );
                if ( RCt == 0 ) {

                        /* This is duplicate, but necessary one
                         */
                    ( ( struct XFSNode * ) TheNode ) -> vt = 
                            ( const union XFSNode_vt * ) & _sAccessPanelNodeVT_v1
                            ;
                    * Node = TheNode;
                }
            }
        }
    }

    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( ( struct XFSNode * ) TheNode );
            TheNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "_AccessPanelNodeMake ND[$(node)] NM[$(name)]", "node=%p,name=%s", ( void * ) TheNode, XFS_ACCESS_PANEL_NAME );
*/

    return RCt;
}   /* _AccessPanelNodeMake () */

static
rc_t CC
_AccessPanelNodeDispose ( const struct _AccessPanelNode * self )
{
    struct _AccessPanelNode * Node = ( struct _AccessPanelNode * ) self;

/*
pLogMsg ( klogDebug, "_AccessPanelNodeDispose ND[$(node)] NM[$(path)]", "node=%p,name=%s", ( void * ) Node, ( Node == NULL ? "" : Node -> Name ) );
*/

    XFSNodeDPF_Dispose ( ( const struct XFSNode * ) self );

    if ( Node != 0 ) {
        free ( Node );
    }

    return 0;
}   /* _AccessPanelNodeDispose () */

rc_t CC
_AccessPanelNodeDispose_v1 ( const struct XFSNode * self )
{
    return _AccessPanelNodeDispose (
                                ( const struct _AccessPanelNode * ) self
                                );
}   /* _AccessPanelNodeDispose_v1 () */

/*)))
 |||
 +++  Finding node child
 |||
(((*/

rc_t CC
_AccessPanelNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    const char * NewNodeName;
    bool IsLast;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    NewNodeName = NULL;
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

            /*) First of all, next node suppose to be last
             (*/
        if ( PathIndex + 2 != XFSPathPartCount ( Path ) ) {
            return XFS_RC ( rcInvalid );
        }

        NewNodeName = XFSPathPartGet ( Path, PathIndex + 1 );

        if ( ! XFSAccessHas ( NewNodeName ) ) {
            return XFS_RC ( rcNotFound );
        }

            /*) Here we are trying to create new node
             (*/
            RCt = _AccessAgentNodeMake ( Node, NewNodeName );
    }

    return RCt;
}   /* _AccessPanelNodeFindNode_v1 () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_AccessPanelDir_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_AccessPanelDir_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSDirEditor * ) self );
    }

    return 0;
}   /* _AccessPanelDir_dispose_v1 () */

static
rc_t CC
_AccessPanelDir_list_v1 (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
)
{
    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSAccessList ( ( struct KNamelist ** ) List );
}   /* _AccessPanelDir_list_v1 () */

rc_t CC
_AccessPanelDir_find_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
)
{
    rc_t RCt = 0;

    XFS_CSAN ( Node )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ! XFSAccessHas ( Name ) ) {
        RCt = XFS_RC ( rcNotFound );
    }
    else {
        RCt = _AccessAgentNodeMake ( Node, Name );
    }

    return RCt;
}   /* _AccessPanelDir_find_v1 () */

static
rc_t CC
_AccessPanelDir_create_file_v1 (
                                const struct XFSDirEditor * self,
                                const char * Name,
                                XFSNMode Mode,
                                const struct XFSHandle ** Handle
)
{
    const struct XFSNode * TempNode;
    const struct XFSHandle * TempHandle;
    rc_t RCt;

    TempNode = NULL;
    TempHandle = NULL;
    RCt = 0;

    XFS_CSAN ( Handle )
    XFS_CAN ( self )
    XFS_CAN ( Name )
    XFS_CAN ( Handle )

        /*  First we check if such node exists
         */
    if ( XFSAccessHas ( Name ) ) {
        return XFS_RC ( rcExists );
    }

        /*  Second we check if such user exists
         */
    if ( ! XFSDoesUserExist ( Name ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

        /*  First we are trying to add another Agent to Access
         */
    RCt = XFSAccessAdd ( Name, NULL );
    if ( RCt == 0 ) {
            /*  Second we should create a node
             */
        RCt = _AccessAgentNodeMake ( & TempNode, Name );
        if ( RCt == 0 ) {
            RCt = XFSHandleMake ( TempNode, & TempHandle );
            if ( RCt == 0 ) {
                * Handle = TempHandle;
            }
        }
    }

    if ( RCt != 0 ) {
        * Handle = NULL;

        if ( TempHandle != NULL ) {
            XFSHandleRelease ( TempHandle );
        }
    }

    return RCt;
}   /* _AccessPanelDir_create_file_v1 () */

static
rc_t CC
_AccessPanelDir_delete_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name
)
{
    XFS_CAN ( self )
    XFS_CAN ( Name )

    if ( ! XFSAccessHas ( Name ) ) {
        return XFS_RC ( rcNotFound );
    }

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSAccessDel ( Name );
}   /* _AccessPanelDir_delete_v1 () */

rc_t CC
_AccessPanelNodeDir_v1 (
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
                        _AccessPanelDir_dispose_v1
                        );

    if ( RCt == 0 ) {
        Editor -> list = _AccessPanelDir_list_v1;
        Editor -> find = _AccessPanelDir_find_v1;
        Editor -> create_file = _AccessPanelDir_create_file_v1;
        Editor -> delete = _AccessPanelDir_delete_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _AccessPanelNodeDir_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_AccessPanelAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_AccessPanelAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _AccessPanelAttr_dispose_v1 () */

static
rc_t CC
_AccessPanelAttr_type_v1 (
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
}   /* _AccessPanelAttr_type_v1 () */

static
rc_t CC
_AccessPanelNodeAttr_v1 (
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
                    _AccessPanelAttr_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> permissions = XFSNodeDPF_vt_permissions;
        Editor -> date = XFSNodeDPF_vt_date;
        Editor -> set_date = XFSNodeDPF_vt_set_date;
        Editor -> type = _AccessPanelAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _AccessPanelNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_AccessPanelNodeDescribe_v1 (
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

    Abbr = "ACCESS-PANEL";

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
}   /* _AccessPanelNodeDescribe_v1 () */

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
XFSAccessPanelNodeMake ( struct XFSNode ** Node )
{
    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    return _AccessPanelNodeMake ( ( struct _AccessPanelNode ** ) Node );
}   /* XFSDirNodeMake () */

/*)))
 |||
 +++    AccessPanelNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_AccessPanelNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct _AccessPanelNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    RCt = _AccessPanelNodeMake ( & TheNode );
    if ( RCt == 0 ) {
        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            _AccessPanelNodeDispose ( TheNode );
        }
    }

/*
pLogMsg ( klogDebug, "_AccessPanelNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _AccessPanelNodeConstructor () */

static
rc_t CC
_AccessPanelNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_AccessPanelNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _AccessPanelNodeValidator () */

static const struct XFSTeleport _sAccessPanelNodeTeleport = {
                                            _AccessPanelNodeConstructor,
                                            _AccessPanelNodeValidator,
                                            false
                                            };

LIB_EXPORT
rc_t CC
XFSAccessPanelProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sAccessPanelNodeTeleport;

    return 0;
}   /* XFSAccessPanelProvider () */
