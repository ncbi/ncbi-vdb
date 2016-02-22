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
#include <kfg/kart.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/perm.h>
#include <xfs/doc.h>

#include "mehr.h"
#include "schwarzschraube.h"
#include "ncon.h"
#include "teleport.h"
#include "common.h"
#include "contnode.h"
#include "xgap.h"
#include "xgapk.h"

#include <sysalloc.h>

#include <string.h>     /* memset */

/*)))
 |||    That is first draft approach for reading Kart files.
 |||    Represents KartNode container with KartItemNodes.
(((*/

/*)))
 |||
 +++    KartNode and KartItemNode
 |||
(((*/

struct XFSKartNode {
    struct XFSContNode node;

    uint32_t project_id;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* KartNode lives here                                           *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

static
rc_t CC
_AddKartItem (
            struct XFSKartNode * Node,
            const struct XFSGapKart * Kart,
            const char * ItemName
)
{
    rc_t RCt;
    struct XFSNode * ItemNode;
    const struct XFSGapKartItem * KartItem;

    RCt = 0;
    ItemNode = NULL;
    KartItem = NULL;

    XFS_CAN ( Node )
    XFS_CAN ( Kart )
    XFS_CAN ( ItemName )

    KartItem = XFSGapKartGet ( Kart, ItemName );
    if ( KartItem == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSGapFileNodeMake ( 
                            & ItemNode,
                            ItemName,
                            XFSGapKartItemProjectId ( KartItem ),
                            XFSGapKartItemAccession ( KartItem ),
                            XFSGapKartItemObjectId ( KartItem )
                            );

    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild (
                                & ( Node -> node . node ),
                                ItemNode
                                );
        if ( GetRCState ( RCt ) == rcExists ) {
            RCt = 0;
        }
        /* We do not dispose node here, but on the caller level */
    }

    if ( RCt != 0 ) {
        if ( ItemNode != NULL ) { 
            XFSNodeDispose ( ItemNode );
            ItemNode = NULL;
        }
    }

    return RCt;
}   /* _AddKartItem () */

static
rc_t CC
_AddSignatureFile ( struct XFSKartNode * Node )
{
    rc_t RCt;
    struct XFSDoc * Doc;
    struct XFSNode * Sign;

    RCt = 0;
    Doc = NULL;
    Sign = NULL;

    XFS_CAN ( Node )

    RCt = XFSTextDocMake ( & Doc );
    if ( RCt == 0 ) {
        RCt = XFSTextDocAppend ( Doc,
                                "%d\n",
                                Node -> project_id
                                );
        if ( RCt == 0 ) {
            RCt = XFSDocNodeMakeWithFlavor (
                                        & Sign,
                                        Doc,
                                        ".#dbgap-mount-tool#",
                                        XFSPermRODefNodeChar (),
                                        _sFlavorOfFoo
                                        );
            if ( RCt == 0 ) {
                RCt = XFSContNodeAddChild (
                                        & ( Node -> node . node ),
                                        Sign
                                        );
            }
        }

        XFSDocRelease ( Doc );
    }

    if ( RCt != 0 ) {
        if ( Sign != NULL ) { 
            XFSNodeDispose ( Sign );
            Sign = NULL;
        }
    }

    return RCt;
}   /* _AddSignatureFile () */

static 
rc_t CC
_LoadKart ( struct XFSKartNode * Node )
{
    rc_t RCt;
    const struct XFSGapKart * Kart;
    struct KNamelist * List;
    uint32_t ListQ, ListI;
    const char * ListN;

    RCt = 0;
    Kart = NULL;
    List = NULL;
    ListQ = ListI = 0;
    ListN = NULL;

    XFS_CAN ( Node )

    RCt = _AddSignatureFile ( Node );
    if ( RCt == 0 ) {
        RCt = XFSGapKartDepotGet (
                            & Kart,
                            XFSNodeName ( & ( Node -> node . node ) )
                            );
        if ( RCt == 0 ) {

            RCt = XFSGapKartList ( Kart, & List, Node -> project_id );
            if ( RCt == 0 ) {
                RCt = KNamelistCount ( List, & ListQ );
                if ( RCt == 0 ) {
                    for ( ListI = 0; ListI < ListQ; ListI ++ ) {
                        RCt = KNamelistGet ( List, ListI, & ListN );
                        if ( RCt != 0 ) {
                            break;
                        }
                        RCt = _AddKartItem ( Node, Kart, ListN );
                        if ( RCt != 0 ) {
                            break;
                        }
                    }
                }

                KNamelistRelease ( List );
            }

            XFSGapKartRelease ( Kart );
        }
    }

    return RCt;
}   /* _LoadKart () */

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_KartNodeDispose ( struct XFSContNode * self )
{
    struct XFSKartNode * Node = ( struct XFSKartNode * ) self;

    if ( Node != NULL ) {
        Node -> project_id = 0;

        free ( Node );
    }

    return 0;
}   /* _KartNodeDispose () */

LIB_EXPORT
rc_t CC
XFSGapKartNodeMake (
            struct XFSNode ** Node,
            const char * Name,
            uint32_t ProjectId,
            const char * Perm
)
{
    rc_t RCt;
    struct XFSKartNode * KartNode;

    RCt = 0;
    KartNode = NULL;


    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    KartNode = calloc ( 1, sizeof ( struct XFSKartNode ) );
    if ( KartNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSContNodeInit (
                            & ( KartNode -> node . node ),
                            Name,
                            Perm,
                            _sFlavorOfGapKart,
                            _KartNodeDispose
                            );
        if ( RCt == 0 ) {
            KartNode -> project_id = ProjectId;

            RCt = _LoadKart ( KartNode );
            if ( RCt == 0 ) {
                * Node = ( struct XFSNode * ) KartNode;
            }
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( KartNode != NULL ) {
            XFSNodeDispose ( & ( KartNode -> node . node ) );
            KartNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "XFSGapKartNodeMake ND[$(node)] NM[$(name)] TP[$(project_id)]", "node=%p,name=%s,project_id=%d", ( void * ) Node, Name, ProjectId );
*/

    return RCt;
}   /* XFSGapKartNodeMake () */

static
rc_t CC
_KartNodeConstructorEx (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            XFSNType Type,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * NodeName;
    uint32_t ProjectId;
    const char * Var;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;
    ProjectId = 0;
    Var = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    Var = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( Var == NULL ) {
        return XFS_RC ( rcInvalid );
    }
    ProjectId = atol ( Var );

    RCt = XFSGapKartNodeMake (
                    & TheNode,
                    NodeName,
                    ProjectId,
                    XFSModelNodeSecurity ( Template )
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
}   /* _KartNodeConstructorEx () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_KartNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _KartNodeConstructorEx (
                            Model,
                            Template,
                            Alias,
                            kxfsFile,
                            Node
                            );

/*
pLogMsg ( klogDebug, "_KartNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _KartNodeConstructor () */

static
rc_t CC
_KartNodeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_KartNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _KartNodeValidator () */

static const struct XFSTeleport _sKartNodeTeleport = {
                                        _KartNodeConstructor,
                                        _KartNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSGapKartProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sKartNodeTeleport;

    return 0;
}   /* XFSGapKartProvider () */
