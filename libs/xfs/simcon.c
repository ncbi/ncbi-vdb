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
#include <klib/namelist.h>
#include <klib/refcount.h>
#include <klib/printf.h>
#include <klib/log.h>

#include <xfs/model.h>
#include <xfs/node.h>

#include "teleport.h"
#include "common.h"
#include "contnode.h"

#include <sysalloc.h>

/*)))
 |||
 +++    Simple Container lives here
 |||
(((*/

/*)))
 |||
 +++    Simple Container has a Teleport, and it is HERE
 |||
(((*/

static
rc_t CC
_SimpleContainerCreateChildren (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            struct XFSNode * Container
)
{
    rc_t RCt;
    const struct KNamelist * Children;
    uint32_t ChildrenQty, llp;
    const char * ChildName, * ChildAlias;
    const struct XFSNode * TheNode;

    RCt = 0;
    Children = NULL;
    ChildrenQty = 0;
    ChildName = ChildAlias = NULL;
    TheNode = NULL;


    RCt = XFSModelNodeChildrenNames ( Template, & Children );
    if ( RCt == 0 ) {

        RCt = KNamelistCount ( Children, & ChildrenQty );
        if ( RCt == 0 ) {

            for ( llp = 0; llp < ChildrenQty; llp ++ ) {
                RCt = KNamelistGet ( Children, llp, & ChildName );
                if ( RCt == 0 ) {

                    ChildAlias = XFSModelNodeChildAlias (
                                                    Template,
                                                    ChildName
                                                    );

/*
pLogMsg ( klogDebug, " ||== Creating child [$(name)] alias [$(alias)]", "name=%s,alias=%s", ChildName, ( ChildAlias == NULL ? "NULL" : ChildAlias ) );
*/

                    RCt = XFSNodeMake (
                                    Model,
                                    ChildName,
                                    ChildAlias,
                                    & TheNode
                                    );
                    if ( RCt == 0 ) {
                        RCt = XFSContNodeAddChild ( Container, TheNode );
                        /* Don't know what to do here */
                    }

                    if ( RCt != 0 ) {
                        break;
                    }
                }
            }
        }
        KNamelistRelease ( Children );
    }

    return RCt;
}   /* _SimpleContainerCreateChildren () */

static
rc_t CC
_SimpleContainerConstructor (
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

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Model == NULL || Template == NULL ) {
        return XFS_RC ( rcNull );
    }

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = XFSContNodeMakeWithFlavor (
                                NodeName,
                                XFSModelNodeSecurity ( Template ),
                                _sFlavorOfSimpleContainer,
                                & TheNode
                                );
    if ( RCt == 0 ) {
        RCt = _SimpleContainerCreateChildren (
                                            Model,
                                            Template,
                                            TheNode
                                            );
        if ( RCt == 0 ) {
            * Node = TheNode;
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        XFSNodeDispose ( TheNode );
    }

/*
pLogMsg ( klogDebug, "_SimpleContainerConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _SimpleContainerConstructor () */

static
rc_t CC
_SimpleContainerValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_SimpleContainerValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _SimpleContainerValidator () */

static const struct XFSTeleport _sSimpleContainerTeleport = {
                                        _SimpleContainerConstructor,
                                        _SimpleContainerValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSSimpleContainerProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sSimpleContainerTeleport;

    return 0;
}   /* XFSSimpleContainerProvider () */


