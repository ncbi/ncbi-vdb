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
#include "xgapk.h"

#include <sysalloc.h>

/*)))
  |||
  +++    That is second approach for reading Kart files.
  +++    Represents KartNode container
  |||
  (((*/

#define XFS_GAP_KART_NAME "karts"

struct XFSGapKartsNode {
    struct XFSNode node;

    const struct XFSNodeContainer * container;

    uint32_t project_id;

    uint64_t version;
};

/*)))
 |||
 +++    Simple Container virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _GapKartsNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _GapKartsNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _GapKartsNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _GapKartsNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _GapKartsNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _GapKartsNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sGapKartsNodeVT_v1 = {
                                            1, 1,   /* nin naj */
                                            _GapKartsNodeFlavor_v1,
                                            _GapKartsNodeDispose_v1,
                                            _GapKartsNodeFindNode_v1,
                                            _GapKartsNodeDir_v1,
                                            NULL,
                                            _GapKartsNodeAttr_v1,
                                            _GapKartsNodeDescribe_v1
                                            };

static
rc_t CC
_GapKartsNodeDispose ( const struct XFSGapKartsNode * self )
{
    struct XFSGapKartsNode * Node = ( struct XFSGapKartsNode * ) self;

/*
pLogMsg ( klogDebug, "_GapKartsNodeDispose ( $(node) )", "node=%p", ( void * ) Node );
*/

    if ( Node != 0 ) {
        if ( Node -> container != NULL ) {
            XFSNodeContainerRelease ( Node -> container );
            Node -> container = NULL;
        }

        Node -> project_id = 0;
        Node -> version = 0;

        free ( Node );
    }

    return 0;
}   /* _GapKartsNodeDispose () */

/*)))
 |||
 +++    Here we are creating and disposing that node
 |||
(((*/

uint32_t CC
_GapKartsNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfGapKarts;
}   /* _GapKartsNodeFlavor_v1 () */

rc_t CC
_GapKartsNodeDispose_v1 ( const struct XFSNode * self )
{
    return _GapKartsNodeDispose (
                                ( const struct XFSGapKartsNode * ) self
                                );
}   /* _GapKartsNodeDispose_v1 () */

/*)))
 (((    First check itself and all others after.
  )))
 (((*/
rc_t CC
_GapKartsNodeFindNode_v1 (
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
                    ( ( const struct XFSGapKartsNode * ) self )
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
}   /* _GapKartsNodeFindNode_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Directory
 (((*/

static
rc_t CC
_GapKartsNodeDir_dispose_v1 ( const struct XFSEditor * self )
{
/*
pLogMsg ( klogDebug, "_GapKartsNodeDir_dispose_ ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != 0 ) {
        free ( ( struct XFSDirEditor * ) self );
    }

    return 0;
}   /* _GapKartsNodeDir_dispose_v1 () */

static
rc_t CC
_GapKartsNodeDir_find_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
)
{
    struct XFSGapKartsNode * Cont = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    Cont = ( struct XFSGapKartsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Cont == NULL ) {
        return XFS_RC ( rcInvalid );
    }
    return XFSNodeContainerGet ( Cont -> container, Name, Node );
}   /* _GapKartsNodeDir_find_v1 () */

static rc_t CC _UpdateKarts ( struct XFSGapKartsNode * Node );

static
rc_t CC
_GapKartsNodeDir_list_v1 (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
)
{
    rc_t RCt;
    struct XFSGapKartsNode * Cont;

    RCt = 0;
    Cont = NULL;

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    Cont = ( struct XFSGapKartsNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Cont == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _UpdateKarts ( Cont );
    if ( RCt == 0 ) {
        RCt = XFSNodeContainerList ( Cont -> container, List );
    }

    return RCt;
}   /* _GapKartsNodeDir_list_v1 () */

rc_t CC
_GapKartsNodeDir_v1 (
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
                        _GapKartsNodeDir_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> list = _GapKartsNodeDir_list_v1;
        Editor -> find = _GapKartsNodeDir_find_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _GapKartsNodeDir_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Attributes
 (((*/

static
rc_t CC
_GapKartsNodeAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
pLogMsg ( klogDebug, "_GapKartsNodeAttr_dispose_ ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != 0 ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _GapKartsNodeAttr_dispose_v1 () */

static
rc_t CC
_GapKartsNodeAttr_permissions_v1 (
                            const struct XFSAttrEditor * self,
                            const char ** Permissions
                            )
{
    XFS_CSAN ( Permissions )
    XFS_CAN ( self )
    XFS_CAN ( Permissions )

    XFS_CAN ( XFSEditorNode ( & ( self -> Papahen ) ) )

    * Permissions = XFSPermRODefContChar ();

    return 0;
}   /* _GapKartsNodeAttr_permisiions_v1 () */

static
rc_t CC
_GapKartsNodeAttr_date_v1 (
                            const struct XFSAttrEditor * self,
                            KTime_t * Time
)
{
    XFS_CSA ( Time, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Time )
    XFS_CAN ( XFSEditorNode ( & ( self -> Papahen ) ) );

    XFSGapKartDepotRefresh ();

    return 0;
}   /* _GapKartsNodeAttr_date_v1 () */

static
rc_t CC
_GapKartsNodeAttr_type_v1 (
                            const struct XFSAttrEditor * self,
                            XFSNType * Type
)
{
    XFS_CSA ( Type, kxfsNotFound )
    XFS_CAN ( self )
    XFS_CAN ( Type )
    XFS_CAN ( XFSEditorNode ( & ( self -> Papahen ) ) )

    XFSGapKartDepotRefresh ();

    * Type = kxfsDir;

    return 0;
}   /* _GapKartsNodeAttr_type_v1 () */

rc_t CC
_GapKartsNodeAttr_v1 (
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
                        _GapKartsNodeAttr_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> permissions = _GapKartsNodeAttr_permissions_v1;
        Editor -> date = _GapKartsNodeAttr_date_v1;
        Editor -> type = _GapKartsNodeAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return 0;
}   /* _GapKartsNodeAttr_v1 () */

rc_t CC
_GapKartsNodeDescribe_v1 (
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
                        "NODE (KARTS)[NULL][NULL]"
                        );
    }
    else {
        RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumWrit,
                        "NODE (KARTS)[%s][0x%p]",
                        self -> Name,
                        self
                        );
    }

    return RCt;
}   /* _GapKartsNodeDescribe_v1 () */

/*)))
 |||
 +++    KartNode
 |||
(((*/

static
rc_t CC
_LoadKart ( struct XFSGapKartsNode * Node, const char * KartName )
{
    rc_t RCt;
    struct XFSNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    XFS_CAN ( Node )
    XFS_CAN ( KartName )

    RCt = XFSGapKartNodeMake (
                            & KartNode,
                            KartName,
                            Node -> project_id,
                            NULL
                            );
    if ( RCt == 0 ) {
        RCt = XFSNodeContainerAdd ( Node -> container, KartNode );
    }
    else {
            /*  Some Karts could be damaged 
             |  we should think about it later
             */
        pLogMsg ( klogWarn, "Invalid Kart file [$(name)]", "name=%s", KartName );
        RCt = 0;
    }

    return RCt;
}   /* _LoadKart () */

static 
rc_t CC
_LoadKarts ( struct XFSGapKartsNode * Node )
{
    rc_t RCt;
    struct KNamelist * KartNames;
    uint32_t qty, llp;
    const char * Name;

    RCt = 0;
    KartNames = NULL;
    qty = llp = 0;
    Name = NULL;

    XFS_CAN ( Node )

    RCt = XFSGapKartDepotList ( & KartNames, Node -> project_id );
    if ( RCt == 0 ) {
        RCt = KNamelistCount ( KartNames, & qty );
        if ( RCt == 0 ) {
            for ( llp = 0; llp < qty; llp ++ ) {
                RCt = KNamelistGet ( KartNames, llp, & Name );
                if ( RCt == 0 ) {
                    RCt = _LoadKart ( Node, Name );
                }

                RCt = 0;
            }
        }

        KNamelistRelease ( KartNames );
    }

    if ( RCt != 0 ) {
            /*  Sorry, we just were unable to load any Kart
             */
        RCt = 0;
    }

    return RCt;
}   /* _LoadKarts () */

rc_t CC
_UpdateKarts ( struct XFSGapKartsNode * Node )
{
    rc_t RCt;
    uint64_t Version;

    RCt = 0;
    Version = 0;

    XFS_CAN ( Node );


    RCt = XFSGapKartDepotRefresh ();
    if ( RCt == 0 ) {
        RCt = XFSGapKartDepotVersion ( & Version );
        if ( RCt == 0 ) {
            if ( Version != Node -> version ) {
                RCt = XFSNodeContainerClear ( Node -> container );
                if ( RCt == 0 ) {
                    RCt = _LoadKarts ( Node );
                    if ( RCt == 0 ) {
                        Node -> version = Version;
                    }
                }
            }
        }
    }

    return RCt;
}   /* _UpdateKarts () */

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_GapKartsNodeMake (
                const char * Name,
                uint32_t ProjectId,
                const char * Perm,
                struct XFSGapKartsNode ** Node
)
{
    rc_t RCt;
    struct XFSGapKartsNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Name )
    XFS_CAN ( Node )

    KartNode = calloc ( 1, sizeof ( struct XFSGapKartsNode ) );
    if ( KartNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInitVT (
                    & ( KartNode -> node ),
                    Name,
                    ( const union XFSNode_vt * ) & _sGapKartsNodeVT_v1
                    );
        if ( RCt == 0 ) {

            RCt = XFSNodeContainerMake ( & ( KartNode -> container ) );
            if ( RCt == 0 ) {
                KartNode -> project_id = ProjectId;
                KartNode -> version = 0;
                if ( _UpdateKarts ( KartNode )  == 0 ) {

                    * Node = KartNode;
                }
            }
        }
    }

    return RCt;
}   /* _GapKartsNodeMake () */

LIB_EXPORT
rc_t CC
XFSGapKartsNodeMake (
            struct XFSNode ** Node,
            uint32_t ProjectId,
            const char * Perm
)
{
    rc_t RCt;
    struct XFSGapKartsNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    RCt = _GapKartsNodeMake (
                            XFS_GAP_KART_NAME,
                            ProjectId,
                            Perm,
                            & KartNode
                            );
    if ( RCt == 0 ) {
        * Node = & ( KartNode -> node );
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( KartNode != NULL ) {
            XFSNodeDispose ( & ( KartNode -> node ) );
            KartNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "XFSKartNodeMake ND[$(node)] NM[$(name)] PJ[$(project_id)]", "node=%p,name=%s,project_id=%d", ( void * ) Node, XFS_GAP_KART_NAME, ProjectId );
*/

    return RCt;
}   /* XFSGapKartsNodeMake () */

static
rc_t CC
_GapKartsNodeMakeFromModel (
                    const char * Name,
                    const struct XFSModel * Model,
                    const struct XFSModelNode * Template,
                    struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSGapKartsNode * KartNode;
    const char * TempStr;
    uint32_t ProjectId;

    RCt = 0;
    KartNode = NULL;
    TempStr = NULL;
    ProjectId = 0;

    XFS_CSAN ( Node )
    XFS_CAN ( Name )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    TempStr = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( TempStr == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    ProjectId = atol ( TempStr );
    if ( ProjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _GapKartsNodeMake (
                            XFS_GAP_KART_NAME,
                            ProjectId,
                            XFSModelNodeSecurity ( Template ),
                            & KartNode
                            );
    if ( RCt == 0 ) {
        RCt = _LoadKarts ( KartNode );
        if ( RCt == 0 ) {
            * Node = & ( KartNode -> node );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( KartNode != NULL ) {
            XFSNodeDispose ( & ( KartNode -> node ) );
            KartNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "KartNodeMakeFromModel ND[$(node)] NM[$(name)] TP[$(project_id)]", "node=%p,name=%s,project_id=%d", ( void * ) Node, Name, ProjectId );
*/

    return RCt;
}   /* _GapKartsNodeMakeFromModel () */

static
rc_t CC
_GapKartsNodeConstructorEx (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * NodeName;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = _GapKartsNodeMakeFromModel (
                                    NodeName,
                                    Model,
                                    Template,
                                    & TheNode
                                    );

    if ( RCt == 0 ) {
        * Node = TheNode;
    }
    else {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
    }

    return RCt;
}   /* _GapKartsNodeConstructorEx () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_GapKartsNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt = _GapKartsNodeConstructorEx (
                                            Model,
                                            Template,
                                            Alias,
                                            Node
                                            );

/*
pLogMsg ( klogDebug, "_GapKartsNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/


    return RCt;
}   /* _GapKartsNodeConstructor () */

static
rc_t CC
_GapKartsNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt = 0;

/*
pLogMsg ( klogDebug, "_GapKartsNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapKartsNodeValidator () */

static const struct XFSTeleport _sGapKartsNodeTeleport = {
                                            _GapKartsNodeConstructor,
                                            _GapKartsNodeValidator,
                                            false
                                            };


LIB_EXPORT
rc_t CC
XFSGapKartsProvider ( const struct XFSTeleport ** Teleport )
{
    XFS_CSAN ( Teleport )
    XFS_CAN ( Teleport )

    * Teleport = & _sGapKartsNodeTeleport;

    return 0;
}   /* XFSGapKartsProvider () */
