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

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/editors.h>

#include "mehr.h"
#include "zehr.h"
#include "ncon.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <stdio.h>


static const char * _sDefPerm = "r--r--r--";

/*)))
 |||
 +++    FooNode, and other vicitims
 |||
(((*/

struct XFSFooNode {
    struct XFSNode Node;

    const char * Security;
};

/*)))
 |||
 +++    FooNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _FooNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _FooNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _FooNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _FooNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _FooNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _FooNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _FooNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sFooNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _FooNodeFlavor_v1,
                                        _FooNodeDispose_v1,
                                        _FooNodeFindNode_v1,
                                        _FooNodeDir_v1,
                                        _FooNodeFile_v1,
                                        _FooNodeAttr_v1,
                                        _FooNodeDescribe_v1
                                        };

static rc_t CC _FooNodeDispose ( const struct XFSFooNode * self );

uint32_t CC
_FooNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfFoo;
}   /* _FooNodeFlavor_v1 () */

rc_t CC
_FooNodeDispose_v1 ( const struct XFSNode * self )
{
    return _FooNodeDispose ( ( struct XFSFooNode * ) self );
}   /* _FooNodeDispose_v1 () */

rc_t CC
_FooNodeFindNode_v1 (
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

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
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
        }
    }

    return RCt;
}   /* _FooNodeFindNode () */

rc_t CC
_FooNodeDir_v1 (
            const struct XFSNode * self,
            const struct XFSDirEditor ** Dir
)
{
    if ( self == NULL || Dir == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Dir = NULL;

    return XFS_RC ( rcInvalid );
}   /* _FooNodeDir_v1 () */

rc_t CC
_FooNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    return XFS_RC ( rcInvalid );
}   /* _FooNodeFile_v1 () */

static
rc_t CC
_FooNodeAttr_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSAttrEditor * Editor;

    Editor = ( struct XFSAttrEditor * ) self;
/*
printf ( "_FooNodeAttr_dispose_v1 ( 0x%p )\n", ( void * ) Editor );
*/

    if ( Editor != NULL ) {
        free ( Editor );
    }

    return 0;
}   /* _FooNodeAttr_dispose_v1 () */

static
rc_t CC
_FooNodeAttr_permissions_v1 (
            const struct XFSAttrEditor * self,
            const char ** Permissions
)
{
    if ( self == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Permissions = NULL;

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    * Permissions = _sDefPerm;

    return 0;
}   /* _FooNodeAttr_permisiions_v1 () */

static
rc_t CC
_FooNodeAttr_size_v1 (
            const struct XFSAttrEditor * self,
            uint64_t * Size
)
{
    if ( self == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Size = 0;

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _FooNodeAttr_size_v1 () */

static
rc_t CC
_FooNodeAttr_date_v1 (
            const struct XFSAttrEditor * self,
            KTime_t * Time
)
{
    if ( self == NULL || Time == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Time = 0;

    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _FooNodeAttr_date_v1 () */

static
rc_t CC
_FooNodeAttr_type_v1 (
            const struct XFSAttrEditor * self,
            XFSNType * Type
)
{
    if ( self == NULL || Type == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Type = kxfsNotFound;
    
    if ( XFSEditorNode ( & ( self -> Papahen ) ) == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    * Type = kxfsFile;

    return 0;
}   /* _FooNodeAttr_type_v1 () */

rc_t CC
_FooNodeAttr_v1 (
            const struct XFSNode * self,
            const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;

    RCt = 0;

    if ( self == NULL || Attr == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Attr = NULL;

    Editor = calloc ( 1, sizeof ( struct XFSAttrEditor ) );
    if ( Editor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _FooNodeAttr_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> permissions = _FooNodeAttr_permissions_v1;
        Editor -> size = _FooNodeAttr_size_v1;
        Editor -> date = _FooNodeAttr_date_v1;
        Editor -> type = _FooNodeAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _FooNodeAttr_v1 () */

rc_t CC
_FooNodeDescribe_v1 (
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
                    "NODE (FOO)[NULL][NULL]"
                    );
    }
    else {
        RCt = string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (FOO)[%s][0x%p]",
                    self -> Name,
                    self
                    );
    }

    return RCt;
}   /* _FooNodeDescribe_v1 () */

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
_FooNodeMake ( struct XFSFooNode ** Foo, const char * NodeName )
{
    rc_t RCt;
    struct XFSFooNode * TheFoo;

    RCt = 0;
    TheFoo = NULL;

    if ( Foo == NULL || NodeName == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Foo = NULL;

    TheFoo = calloc ( 1, sizeof ( struct XFSFooNode ) );
    if ( TheFoo == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInit ( & ( TheFoo -> Node ), NodeName );
        if ( RCt == 0 ) {
            ( & ( TheFoo -> Node ) ) -> vt =
                    ( const union XFSNode_vt * ) & _sFooNodeVT_v1;

            TheFoo -> Security = NULL;

            * Foo = TheFoo;
        }
    }
/*
printf ( "_FooNodeMake ND[0x%p] NM[%s]\n", ( void * ) TheFoo, NodeName );
*/

    return RCt;
}   /* _FooNodeMake () */

static
rc_t CC
_FooNodeDispose ( const struct XFSFooNode * self )
{
    struct XFSFooNode * Foo = ( struct XFSFooNode * ) self;

/*
printf ( "_FooNodeDispose ( 0x%p )\n", ( void * ) Foo );
*/

    if ( Foo == NULL ) {
        return 0;
    }

    if ( Foo -> Security != NULL ) {
        free ( ( char * ) Foo -> Security );
        Foo -> Security = NULL;
    }

    free ( Foo );

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
    struct XFSFooNode * TheNode;
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
printf ( "_FooNodeConstructor ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
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
printf ( "_FooNodeValidator ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _FooNodeValidator () */

static const struct XFSTeleport _sFooNodeTeleport = {
                                        _FooNodeConstructor,
                                        _FooNodeValidator
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
XFSEncryptedFileProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

LIB_EXPORT
rc_t CC
XFSHelpProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

LIB_EXPORT
rc_t CC
XFSKartCollectionProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

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

LIB_EXPORT
rc_t CC
XFSRemoteRepositoryProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

LIB_EXPORT
rc_t CC
XFSWorkspaceProvider ( const struct XFSTeleport ** Teleport )
{
    return XFSFooNodeProvider ( Teleport );
}

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
