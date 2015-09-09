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

#include <sysalloc.h>

#include <stdio.h>
#include <string.h>     /* memset */

/*)))
 |||    That is first draft approach for dbGaP project node
 |||    That node contains 3 statically linked nodes :
 |||        Cached ( SRA and FILES )
 |||        Karts
 |||        WorkSpace
 |||    Each node exists in corresponded file
 |||
(((*/

/*)))
 |||
 +++    DbGapProjectNode
 |||
(((*/

struct XFSDbGapProjectNode {
    struct XFSContNode node;

    uint32_t project_id;
    bool read_only;

    const char * kart_files;
};

static
rc_t CC
_DbGapProjectNodeDispose ( struct XFSContNode * self )
{
    struct XFSDbGapProjectNode * Node =
                                ( struct XFSDbGapProjectNode * ) self;

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
}   /* _DbGapProjectNodeDispose () */

static
rc_t CC
_DbGapProjectNodeMake (
            struct XFSDbGapProjectNode ** Node,
            const char * Name,
            uint32_t ProjectId,
            bool ReadOnly,
            const char * KartFiles,  /* Could be NULL */
            const char * Perm
)
{
    rc_t RCt;
    struct XFSDbGapProjectNode * DbGapNode;

    RCt = 0;
    DbGapNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    DbGapNode = calloc ( 1, sizeof ( struct XFSDbGapProjectNode ) );
    if ( DbGapNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSContNodeInit (
                            & ( DbGapNode -> node . node ),
                            Name,
                            Perm,
                            _sFlavorOfDbGapProject,
                            _DbGapProjectNodeDispose
                            );
        if ( RCt == 0 ) {
            if ( KartFiles != NULL ) {
                RCt = XFS_StrDup (
                                KartFiles,
                                & ( DbGapNode -> kart_files )
                                );
            }
            if ( RCt == 0 ) {
                DbGapNode -> project_id = ProjectId;
                DbGapNode -> read_only = ReadOnly;

                * Node = DbGapNode;
            }
        }
    }

    return RCt;
}   /* _DbGapProjectNodeMake () */

/* TODO : DELETE THAT CODE!!!
 */
static
rc_t CC
_HackWorkspace ( char * Buf, size_t BufSize )
{
    rc_t RCt;
    struct KDirectory * NatDir;

    RCt = 0;
    NatDir = NULL;

    RCt = KDirectoryNativeDir ( & NatDir );
    if ( RCt == 0 ) {
        RCt = KDirectoryResolvePath (
                                    NatDir,
                                    true,
                                    Buf,
                                    BufSize,
                                    "Enc"
                                    );
        if ( RCt == 0 ) {
            if ( KDirectoryPathType ( NatDir, Buf ) == kptNotFound ) {
                RCt = KDirectoryCreateDir (
                                        NatDir,
                                        0777,
                                        kcmCreate,
                                        Buf
                                        );
            }
        }

        KDirectoryRelease ( NatDir );
    }

    return RCt;
}   /* _HackWorkspace () */

static
rc_t CC
_DbGapProjectNodeAddChildren ( struct XFSDbGapProjectNode * Node )
{
    rc_t RCt;
    struct XFSNode * TheNode;
    char Bf [ XFS_SIZE_4096 ];

    RCt = 0;
    TheNode = NULL;
    * Bf = 0;

    XFS_CAN ( Node );

        /* There are three children: Cache, Karts and WS ...
           and may be Bublic cache too.
         */

        /* Workspace is here
         * TODO: password, location, etc ...
         */
    RCt = _HackWorkspace ( Bf, sizeof ( Bf ) );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = XFSWorkspaceNodeMake (
                        & TheNode,
                        "workspace",
                        Bf,
                        "aaa",
                        NULL
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( Node -> node ) . node, TheNode );
        if ( RCt != 0 ) {
            XFSNodeDispose ( TheNode );

            return RCt;
        }
    }

        /* Here is cache related to project
         */
    RCt = XFSDbGapCacheNodeMake (
                        & TheNode,
                        Node -> project_id,     /* projectId */
                        true,                   /* ReadOnly */
                        NULL,                   /* name is automatic */
                        NULL                    /* perm is automatic */
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( Node -> node ) . node, TheNode );
        if ( RCt != 0 ) {
            XFSNodeDispose ( TheNode );

            return RCt;
        }
    }

        /* Here is public cache 
         */
    RCt = XFSDbGapCacheNodeMake (
                        & TheNode,
                        0,                      /* projectId */
                        false,                  /* Non ReadOnly */
                        NULL,                   /* name is automatic */
                        NULL                    /* perm is automatic */
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( Node -> node ) . node, TheNode );
        if ( RCt != 0 ) {
            XFSNodeDispose ( TheNode );

            return RCt;
        }
    }

    RCt = XFSDbGapKartsNodeMake (
                        & TheNode,
                        "carts",
                        Node -> kart_files,
                        Node -> project_id,
                        NULL
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild ( & ( Node -> node ) . node, TheNode );
        if ( RCt != 0 ) {
            XFSNodeDispose ( TheNode );

            return RCt;
        }
    }

    return RCt;
}   /* _DbGapProjectNodeAddChildren () */


LIB_EXPORT
rc_t CC
XFSDbGapProjectNodeMake (
            struct XFSNode ** Node,
            const char * Name,
            uint32_t ProjectId,
            bool ReadOnly,
            const char * KartFiles,
            const char * Perm
)
{
    rc_t RCt;
    struct XFSDbGapProjectNode * DbGapNode;

    RCt = 0;
    DbGapNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )
    XFS_CAN ( Name )

    RCt = _DbGapProjectNodeMake (
                                & DbGapNode,
                                Name,
                                ProjectId,
                                ReadOnly,
                                KartFiles,
                                Perm
                                );
    if ( RCt == 0 ) {
        RCt = _DbGapProjectNodeAddChildren ( DbGapNode );
        if ( RCt == 0 ) {
            * Node = & ( DbGapNode -> node . node );
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( DbGapNode != NULL ) {
            XFSNodeDispose ( & ( DbGapNode -> node . node ) );
            DbGapNode = NULL;
        }
    }

/*
printf ( "XFSDbGapNodeMake ND[0x%p] NM[%s] TP[%d]\n", ( void * ) Node, Name, Type );
*/
    return RCt;
}   /* XFSDbGapProjectNodeMake () */

/*)))
 |||
 +++  There DbGapNode Find
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
_DbGapProjectNodeConstructorEx (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        XFSNType Type,
                        const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSDbGapProjectNode * TheNode;
    const char * NodeName;
    const char * KartFiles;
    const char * TempStr;
    uint32_t ProjectId;
    bool ReadOnly;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;
    KartFiles = NULL;
    TempStr = NULL;
    ProjectId = 0;
    ReadOnly = true;

    XFS_CSAN ( Node )
    XFS_CAN ( Model )
    XFS_CAN ( Template )
    XFS_CAN ( Node )

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    KartFiles = XFSModelNodeProperty ( Template, XFS_MODEL_KARTFILES );
    TempStr = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( TempStr == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    ProjectId = atol ( TempStr );
    if ( ProjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    TempStr = XFSModelNodeProperty ( Template, XFS_MODEL_MODE );
    if ( TempStr != NULL ) {
        ReadOnly = strcmp ( TempStr, XFS_MODEL_MODE_RW ) == 0;
    }

    RCt = _DbGapProjectNodeMake (
                                & TheNode,
                                NodeName,
                                ProjectId,
                                ReadOnly,
                                KartFiles,
                                XFSModelNodeSecurity ( Template )
                                );
    if ( RCt == 0 ) {
        RCt = _DbGapProjectNodeAddChildren ( TheNode );
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
}   /* _DbGapProjectNodeConstructorEx () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_DbGapProjectNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt = 0;

    RCt = _DbGapProjectNodeConstructorEx (
                                        Model,
                                        Template,
                                        Alias,
                                        kxfsFile,
                                        Node
                                        );
/*
printf ( "_DbGapProjectNodeConstructor ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/


    return RCt;
}   /* _DbGapProjectNodeConstructor () */

static
rc_t CC
_DbGapProjectNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
printf ( "_DbGapProjectNodeValidator ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _DbGapProjectNodeValidator () */

static const struct XFSTeleport _sDbGapProjectNodeTeleport = {
                                        _DbGapProjectNodeConstructor,
                                        _DbGapProjectNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSDbGapProjectProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sDbGapProjectNodeTeleport;

    return 0;
}   /* XFSDbGapProjectProvider () */
