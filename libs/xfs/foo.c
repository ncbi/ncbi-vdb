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

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/editors.h>
#include <xfs/perm.h>
#include <xfs/doc.h>

#include "mehr.h"
#include "zehr.h"
#include "ncon.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

/*)))
 |||
 +++    FooNode, and other vicitims
 |||
(((*/

/*)))
 |||
 +++    FooNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_FooNodeMake ( struct XFSNode ** Foo, const char * NodeName )
{
    rc_t RCt;
    struct XFSDoc * Doc;
    struct XFSNode * TheFoo;

    RCt = 0;
    TheFoo = NULL;

    if ( Foo != NULL ) {
        * Foo = NULL;
    }

    if ( Foo == NULL || NodeName == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSTextDocMake ( & Doc );
    if ( RCt == 0 ) {
        RCt = XFSTextDocAppend ( Doc, "FOO FILE: placeholder for not implemented node type\n\n" );
        if ( RCt  == 0 ) {
            RCt = XFSDocNodeMakeWithFlavor (
                                        & TheFoo,
                                        Doc,
                                        NodeName,
                                        XFSPermRODefNodeChar (),
                                        _sFlavorOfFoo
                                        );
            if ( RCt == 0 ) {
                * Foo = TheFoo;
            }
            else {
                if ( TheFoo != NULL ) {
                    XFSNodeDispose ( TheFoo );
                }
            }
        }

        XFSDocRelease ( Doc );
    }

/*
pLogMsg ( klogDebug, "_FooNodeMake ND[$(node)] NM[$(name)]", "node=%p,name=%s", ( void * ) TheFoo, NodeName );
*/

    return RCt;
}   /* _FooNodeMake () */

static
rc_t CC
_FooNodeDispose ( const struct XFSNode * self )
{
    struct XFSNode * Foo = ( struct XFSNode * ) self;

/*
pLogMsg ( klogDebug, "_FooNodeDispose ( $(node) )", "node=%p", ( void * ) Foo );
*/

    if ( Foo == NULL ) {
        return 0;
    }

    XFSNodeDispose ( Foo );

    return 0;
}   /* _FooNodeDispose () */

/*)))
 |||
 +++    FooNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_FooNodeConstructor (
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

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = _FooNodeMake ( & TheNode, NodeName );
    if ( RCt == 0 ) {
        /* TODO Copy Security */

        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        _FooNodeDispose ( TheNode );
    }

/*
pLogMsg ( klogDebug, "_FooNodeConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )\n", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _FooNodeConstructor () */

static
rc_t CC
_FooNodeValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_FooNodeValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )\n", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _FooNodeValidator () */

static const struct XFSTeleport _sFooNodeTeleport = {
                                        _FooNodeConstructor,
                                        _FooNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSFooNodeProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sFooNodeTeleport;

    return 0;
}   /* XFSFooNodeProvider () */


/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 ///    And there are dummies for TeleportProviders
(((*/
LIB_EXPORT
rc_t CC
XFSCacheProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}   /* XFSCacheProvider () */

LIB_EXPORT
rc_t CC
XFSLinkProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

LIB_EXPORT
rc_t CC
XFSLocalRepositoryProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
