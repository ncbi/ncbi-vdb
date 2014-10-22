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
#include "ncon.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <stdio.h>

/*)))
 |||
 +++    SimpleContainer, and other simple containers
 |||
(((*/

struct XFSSimpleContainer {
    struct XFSNode Node;

    struct XFSNodeContainer * Container;

    const char * Security;
};

/*)))
 |||
 +++    Simple Container virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _SimpleContainerFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _SimpleContainerDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _SimpleContainerFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _SimpleContainerDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _SimpleContainerFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _SimpleContainerAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _SimpleContainerDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sSimpleContainerVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _SimpleContainerFlavor_v1,
                                        _SimpleContainerDispose_v1,
                                        _SimpleContainerFindNode_v1,
                                        _SimpleContainerDir_v1,
                                        _SimpleContainerFile_v1,
                                        _SimpleContainerAttr_v1,
                                        _SimpleContainerDescribe_v1
                                        };

static rc_t CC _SimpleContainerDispose (
                                const struct XFSSimpleContainer * self
                                );
uint32_t CC
_SimpleContainerFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfSimpleContainer;
}   /* _SimpleContainerFlavor_v1 () */

rc_t CC
_SimpleContainerDispose_v1 ( const struct XFSNode * self )
{
    return _SimpleContainerDispose (
                            ( const struct XFSSimpleContainer * ) self
                            );
}   /* _SimpleContainerDispose_v1 () */

/*)))
 (((    First check itself and all others after.
  )))
 (((*/
rc_t CC
_SimpleContainerFindNode_v1 (
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
        NodeName = XFSPathGet ( Path, PathIndex + 1 );

            /*))  Should not happen thou
             ((*/
        if ( NodeName == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {

            RCt = XFSNodeContainerGet (
                    ( ( const struct XFSSimpleContainer * ) self )
                                                        -> Container,
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
}   /* _SimpleContainerFindNode_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Directory
 (((*/

static
rc_t CC
_SimpleContainerDir_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSDirEditor * Editor;

    Editor = ( struct XFSDirEditor * ) self;
/*
printf ( "_SimpleContainerDir_dispose_ ( 0x%p )\n", ( void * ) Editor );
*/

    if ( Editor == 0 ) {
        return 0;
    }

    free ( Editor );

    return 0;
}   /* _SimpleContainerDir_dispose_v1 () */

static
rc_t CC
_SimpleContainerDir_find_v1 (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
)
{
    struct XFSSimpleContainer * Cont;

    Cont = NULL;

    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

    Cont = ( struct XFSSimpleContainer * ) ( self -> Papahen ) . Node;
    if ( Cont == NULL ) {
        return XFS_RC ( rcInvalid );
    }
    return XFSNodeContainerGet ( Cont -> Container, Name, Node );
}   /* _SimpleContainerDir_find_v1 () */

static
rc_t CC
_SimpleContainerDir_list_v1 (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
)
{
    struct XFSSimpleContainer * Cont;

    Cont = NULL;

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }
    * List = NULL;

    Cont = ( struct XFSSimpleContainer * ) ( self -> Papahen ) . Node;
    if ( Cont == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    return XFSNodeContainerList ( Cont -> Container, List );
}   /* _SimpleContainerDir_list_v1 () */

rc_t CC
_SimpleContainerDir_v1 (
                    const struct XFSNode * self,
                    const struct XFSDirEditor ** Dir
)
{
    rc_t RCt;
    struct XFSDirEditor * Editor;

    RCt = 0;
    Editor = NULL;

    if ( self == NULL || Dir == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Dir = NULL;

    Editor = calloc ( 1, sizeof ( struct XFSDirEditor ) );
    if ( Editor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                        & ( Editor -> Papahen ),
                        self,
                        _SimpleContainerDir_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> list = _SimpleContainerDir_list_v1;
        Editor -> find = _SimpleContainerDir_find_v1;
        Editor -> create_file = NULL;
        Editor -> create_dir = NULL;
        Editor -> delete = NULL;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _SimpleContainerDir_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, File
 (((*/

rc_t CC
_SimpleContainerFile_v1 (
                    const struct XFSNode * self,
                    const struct XFSFileEditor ** File
)
{
    rc_t RCt;

    RCt = 0;

    return RCt;
}   /* _SimpleContainerFile_v1 () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))  So, Attributes
 (((*/

static
rc_t CC
_SimpleContainerAttr_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSAttrEditor * Editor;

    Editor = ( struct XFSAttrEditor * ) self;
/*
printf ( "_SimpleContainerAttr_dispose_ ( 0x%p )\n", ( void * ) Editor );
*/

    if ( Editor == 0 ) {
        return 0;
    }

    free ( Editor );

    return 0;
}   /* _SimpleContainerAttr_dispose_v1 () */

static
rc_t CC
_SimpleContainerAttr_permissions_v1 (
                            const struct XFSAttrEditor * self,
                            const char ** Permissions
                            )
{
    const struct XFSNode * Node;

    if ( self == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Permissions = NULL;

    Node = XFSEditorNode ( & ( self -> Papahen ) );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    * Permissions = ( ( struct XFSSimpleContainer * ) Node ) -> Security;

	if ( * Permissions == NULL ) {
		* Permissions = XFSPermForContainerDefault ();
	}

    return 0;
}   /* _SimpleContainerAttr_permisiions_v1 () */

static
rc_t CC
_SimpleContainerAttr_size_v1 (
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
}   /* _SimpleContainerAttr_size_v1 () */

static
rc_t CC
_SimpleContainerAttr_date_v1 (
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
}   /* _SimpleContainerAttr_date_v1 () */

static
rc_t CC
_SimpleContainerAttr_type_v1 (
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

    * Type = kxfsDir;

    return 0;
}   /* _SimpleContainerAttr_type_v1 () */

rc_t CC
_SimpleContainerAttr_v1 (
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
                        _SimpleContainerAttr_dispose_v1
                        );
    if ( RCt == 0 ) {
        Editor -> permissions = _SimpleContainerAttr_permissions_v1;
        Editor -> size = _SimpleContainerAttr_size_v1;
        Editor -> date = _SimpleContainerAttr_date_v1;
        Editor -> type = _SimpleContainerAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return 0;
}   /* _SimpleContainerAttr_v1 () */

rc_t CC
_SimpleContainerDescribe_v1 (
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
                        "NODE (SIMPLE CONT)[NULL][NULL]"
                        );
    }
    else {
        RCt = string_printf (
                        Buffer,
                        BufferSize,
                        & NumWrit,
                        "NODE (SIMPLE CONT)[%s][0x%p]",
                        self -> Name,
                        self
                        );
    }

    return RCt;
}   /* _SimpleContainerDescribe_v1 () */

/*)))
 |||
 +++    Simple Container lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_SimpleContainerMake (
                struct XFSSimpleContainer ** Container,
                const char * Name
)
{
    rc_t RCt;
    struct XFSSimpleContainer * TheContainer;

    RCt = 0;
    TheContainer = NULL;

    if ( Container == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Container = NULL;

    TheContainer = calloc ( 1, sizeof ( struct XFSSimpleContainer ) );
    if ( TheContainer == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInit ( & ( TheContainer -> Node ), Name );
        if ( RCt == 0 ) {
            ( & ( TheContainer -> Node ) ) -> vt =
                ( const union XFSNode_vt * ) & _sSimpleContainerVT_v1;

            RCt = XFSNodeContainerMake ( ( const struct XFSNodeContainer ** ) & TheContainer -> Container );
            if ( RCt == 0 ) {
                * Container = TheContainer;
            }
        }
    }
/*
printf ( "_SimpleContainerMake ND[0x%p] NM[%s]\n", ( void * ) TheContainer, Name );
*/

    return RCt;
}   /* _SimpleContainerMake () */

static
rc_t CC
_SimpleContainerDispose ( const struct XFSSimpleContainer * self )
{
    struct XFSSimpleContainer * Container =
                                ( struct XFSSimpleContainer * ) self;

/*
printf ( "_SimpleContainerDispose ( 0x%p )\n", ( void * ) Container );
*/
    if ( Container == 0 ) {
        return 0;
    }

    if ( Container -> Container != NULL ) {
        XFSNodeContainerRelease ( Container -> Container );
        Container -> Container = NULL;
    }

    if ( Container -> Security != NULL ) {
        free ( ( char * ) Container -> Security );
        Container -> Security = NULL;
    }

    free ( Container );

    return 0;
}   /* _SimpleContainerDispose () */

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
            const struct XFSSimpleContainer * Container
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

    XFSNodeContainerSetEditable ( Container -> Container, true );

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
printf ( " ||== Creating child [%s] alias [%s]\n", ChildName, ( ChildAlias == NULL ? "NULL" : ChildAlias ) );
*/

                    RCt = XFSNodeMake (
                                    Model,
                                    ChildName,
                                    ChildAlias,
                                    & TheNode
                                    );
                    if ( RCt == 0 ) {
                        RCt = XFSNodeContainerAdd (
                                            Container -> Container,
                                            TheNode
                                            );
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

    XFSNodeContainerSetEditable ( Container -> Container, false );


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
    struct XFSSimpleContainer * TheNode;
    const char * NodeName;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;

    if ( Model == NULL || Template == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = _SimpleContainerMake ( & TheNode, NodeName );
    if ( RCt == 0 ) {

        RCt = _SimpleContainerCreateChildren (
                                            Model,
                                            Template,
                                            TheNode
                                            );
        if ( RCt == 0 ) {

            RCt = XFS_StrDup (
                            XFSModelNodeSecurity ( Template ),
                            & ( TheNode -> Security )
                            );
                /* Adding permissions */
            if ( RCt == 0 ) { 
                * Node = ( const struct XFSNode * )
                                        & ( TheNode -> Node );
            }
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        _SimpleContainerDispose ( TheNode );
    }


/*
printf ( "_SimpleContainerConstructor ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
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
printf ( "_SimpleContainerValidator ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _SimpleContainerValidator () */

static const struct XFSTeleport _sSimpleContainerTeleport = {
                                        _SimpleContainerConstructor,
                                        _SimpleContainerValidator
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


