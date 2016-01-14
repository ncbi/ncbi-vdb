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

#include <sysalloc.h>

#include <string.h>     /* memset */

/*)))
 |||    That is second approach for dbGaP project node
 |||    That node contains 3 statically linked nodes :
 |||        Cached ( SRA and FILES )
 |||        Karts
 |||        WorkSpace
 |||
(((*/

/*)))
 |||
 +++    _GapProjectNode
 |||
(((*/

struct _GapProjectNode {
    struct XFSContNode node;

    uint32_t project_id;
    bool read_only;

    const char * kart_files;
};

static
rc_t CC
_GapProjectNodeDispose ( struct XFSContNode * self )
{
    struct _GapProjectNode * Node =
                                ( struct _GapProjectNode * ) self;

    if ( Node != NULL ) {
        if ( Node -> kart_files != NULL ) {
            free ( ( char * ) Node -> kart_files );
            Node -> kart_files = NULL;
        }

        Node -> project_id = 0;
        Node -> read_only = true;

        free ( Node );
    }

    return 0;
}   /* _GapProjectNodeDispose () */

static
rc_t CC
_GapProjectNodeMake (
            struct _GapProjectNode ** Node,
            const char * Name,
            uint32_t ProjectId,
            bool ReadOnly,
            const char * Perm
)
{
    rc_t RCt;
    struct _GapProjectNode * GapNode;

    RCt = 0;
    GapNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    GapNode = calloc ( 1, sizeof ( struct _GapProjectNode ) );
    if ( GapNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSContNodeInit (
                            & ( GapNode -> node . node ),
                            Name,
                            Perm,
                            _sFlavorOfGapProject,
                            _GapProjectNodeDispose
                            );
        if ( RCt == 0 ) {
            if ( RCt == 0 ) {
                GapNode -> project_id = ProjectId;
                GapNode -> read_only = ReadOnly;

                * Node = GapNode;
            }
        }
    }

    return RCt;
}   /* _GapProjectNodeMake () */

static
rc_t CC
_PrepareWorkspace (
                uint32_t ProjectId,
                char ** Workspace,
                const char ** Password
)
{
    rc_t RCt;
    const struct XFSGapProject * Project;
    char * WorkspaceDir;
    const char * ThePass;
    size_t ThePassSize;
    struct KDirectory * NatDir;
    uint32_t PathType;

    RCt = 0;
    Project = NULL;
    WorkspaceDir = NULL;
    ThePass = NULL;
    ThePassSize = 0;
    NatDir = NULL;
    PathType = kptNotFound;

    RCt = XFSGapFindOrCreate ( ProjectId, & Project );
    if ( RCt == 0 ) {

        RCt = XFSGapProjectWorkspace ( Project, & WorkspaceDir );
        if ( RCt == 0 ) {
            RCt = XFSGapProjectPassword (
                                        Project,
                                        & ThePass,
                                        & ThePassSize
                                        );

            if ( RCt == 0 ) {
                RCt = KDirectoryNativeDir ( & NatDir );
                if ( RCt == 0 ) {
                    PathType = KDirectoryPathType ( NatDir, WorkspaceDir );

                    switch ( PathType ) {
                        case kptNotFound :
                            RCt = KDirectoryCreateDir (
                                                    NatDir,
                                                    0777,
                                                    kcmCreate,
                                                    WorkspaceDir
                                                    );
                            if ( RCt == 0 ) {
                                * Workspace = WorkspaceDir;
                                * Password = ThePass;
                            }
                            break;
                        case kptDir :
                            * Workspace = WorkspaceDir;
                            * Password = ThePass;
                            break;
                        default :
                            RCt = XFS_RC ( rcInvalid );
                            break;
                    }

                    KDirectoryRelease ( NatDir );
                }
            }
        }

        XFSGapProjectRelease ( Project );
    }

    if ( RCt != 0 ) {
        * Workspace = NULL;
        * Password = NULL;

        if ( WorkspaceDir != NULL ) {
            free ( WorkspaceDir );
        }
    }

    return RCt;
}   /* _PrepareWorkspace () */

static
rc_t CC
_GapProjectNodeAddChildren ( struct _GapProjectNode * self )
{
    rc_t RCt;
    struct XFSNode * TheNode;
    char * Workspace;
    const char * Password;

    RCt = 0;
    TheNode = NULL;
    Workspace = NULL;
    Password = NULL;

    XFS_CAN ( self );

        /* There are three children: Cache, Karts and WS ...
           and may be Bublic cache too.
         */

        /* Workspace is here
         * TODO: password, location, etc ...
         */
    RCt = _PrepareWorkspace (
                            self -> project_id,
                            & Workspace,
                            & Password
                            );
    if ( RCt == 0 ) {

        RCt = XFSWorkspaceNodeMake (
                                & TheNode,
                                "workspace",
                                Workspace,
                                Password,
                                NULL,
                                self -> read_only
                                );
        if ( RCt == 0 ) {
            RCt = XFSContNodeAddChild (
                                    & ( self -> node ) . node,
                                    TheNode
                                    );
        }

        free ( Workspace );
    }
    else {
/* TODO - use approved output method
*/
pLogErr ( klogErr, RCt, "ERROR: Can not find definition of Project $(project_id) in config file", "project_id=%d", self -> project_id );
    }

    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
        return RCt;
    }

        /* Here is cache related to project
         */
    RCt = XFSGapCacheNodeMake (
                        & TheNode,
                        self -> project_id,     /* projectId */
                        NULL                    /* perm is automatic */
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( self -> node ) . node, TheNode );
    }
    else {
/* TODO - use approved output method
*/
pLogErr ( klogErr, RCt, "ERROR: Can not find definition of Project $(project_id) in config file", "project_id=%d", self -> project_id );
    }
    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
        return RCt;
    }

        /* Here is public cache 
         */
    RCt = XFSGapCacheNodeMake (
                        & TheNode,
                        XFS_PUBLIC_PROJECT_ID,  /* projectId */
                        NULL                    /* perm is automatic */
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( self -> node ) . node, TheNode );
    }
    else {
        RCt = 0;
/* TODO - use approved output method
*/
LogMsg ( klogWarn, "Can not find definition for 'public' area in config file" );
    }

    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
        return RCt;
    }

    RCt = XFSGapKartFilesNodeMake ( & TheNode, NULL );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( self -> node ) . node, TheNode );
    }
    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
        return RCt;
    }

    RCt = XFSGapKartsNodeMake (
                            & TheNode,
                            self -> project_id,
                            NULL
                            );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( self -> node ) . node, TheNode );
    }
    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
        return RCt;
    }

    return RCt;
}   /* _GapProjectNodeAddChildren () */


LIB_EXPORT
rc_t CC
XFSGapProjectNodeMake (
            struct XFSNode ** Node,
            const char * Name,
            uint32_t ProjectId,
            bool ReadOnly,
            const char * Perm
)
{
    rc_t RCt;
    struct _GapProjectNode * GapNode;

    RCt = 0;
    GapNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    RCt = _GapProjectNodeMake (
                                & GapNode,
                                Name,
                                ProjectId,
                                ReadOnly,
                                Perm
                                );
    if ( RCt == 0 ) {
        RCt = _GapProjectNodeAddChildren ( GapNode );
        if ( RCt == 0 ) {
            * Node = & ( GapNode -> node . node );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( GapNode != NULL ) {
            XFSNodeDispose ( & ( GapNode -> node . node ) );
            GapNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "_GapNodeMake ND[$(node)] NM[$(name)] PJ[$(project_id)]", "node=%p,name=%s,project_id=%d", ( void * ) Node, Name, ( GapNode == NULL ? 0 : GapNode -> project_id ) );
*/
    return RCt;
}   /* XFSGapProjectNodeMake () */

/*)))
 |||
 +++  There GapNode Find
 |||
(((*/

/*)))
 |||
 +++  There KartItemNode Find
 |||
(((*/

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

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
_GapProjectNodeConstructorEx (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        XFSNType Type,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct _GapProjectNode * TheNode;
    const char * NodeName;
    const char * TempStr;
    uint32_t ProjectId;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;
    TempStr = NULL;
    ProjectId = 0;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    TempStr = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( TempStr == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    ProjectId = atol ( TempStr );
    if ( ProjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _GapProjectNodeMake (
                                & TheNode,
                                NodeName,
                                ProjectId,
                                XFSModelNodeReadOnly ( Template ),
                                XFSModelNodeSecurity ( Template )
                                );
    if ( RCt == 0 ) {
        RCt = _GapProjectNodeAddChildren ( TheNode );
        if ( RCt == 0 ) {
            * Node = & ( TheNode -> node . node );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( & ( TheNode -> node . node ) );

            TheNode = NULL;
        }
    }

    return RCt;
}   /* _GapProjectNodeConstructorEx () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_GapProjectNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt = 0;

    RCt = _GapProjectNodeConstructorEx (
                                        Model,
                                        Template,
                                        Alias,
                                        kxfsFile,
                                        Node
                                        );
/*
pLogMsg ( klogDebug, "_GapProjectNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/


    return RCt;
}   /* _GapProjectNodeConstructor () */

static
rc_t CC
_GapProjectNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_GapProjectNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapProjectNodeValidator () */

static const struct XFSTeleport _sGapProjectNodeTeleport = {
                                        _GapProjectNodeConstructor,
                                        _GapProjectNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSGapProjectProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sGapProjectNodeTeleport;

    return 0;
}   /* XFSGapProjectProvider () */
