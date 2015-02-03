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
 |||    That is first draft approach for reading Kart files.
 |||    Represents KartNode container with KartItemNodes.
(((*/

/*)))
 |||
 +++    KartNode and KartItemNode
 |||
(((*/

struct XFSKartCollectionNode {
    struct XFSContNode node;

    const char * path;
};

static
rc_t CC
_IsPathCart (
        const struct KDirectory * Directory,
        const char * Path,
        const char * Name,
        char * BF,
        size_t BFS,
        bool * IsCart
)
{
    rc_t RCt;
    size_t nwr;
    const struct KFile * File;
    const char * Sg = "ncbikart";
    size_t SgLen;
    char SF [ 64 ];

    RCt = 0;
    nwr = 0;
    File = NULL;
    SgLen = sizeof ( Sg );
    * SF = 0;

    * BF = 0;
    * IsCart = false;

    RCt = string_printf ( BF, BFS, & nwr, "%s/%s", Path, Name );
    if ( RCt == 0 ) {
        RCt = KDirectoryOpenFileRead ( Directory, & File, BF );
        if ( RCt == 0 ) {
            RCt = KFileRead ( File, 0, SF, SgLen, & nwr );
            if ( RCt == 0 ) {
                * IsCart = ! string_cmp ( SF, SgLen, Sg, SgLen, SgLen );
            }

            KFileRelease ( File );
        }
    }

    return RCt;
}   /* _IsPathCart () */

static
rc_t CC
_LoadKartItem (
            struct KDirectory * Directory,
            struct XFSKartCollectionNode * Node,
            const char * Name
)
{
    rc_t RCt;
    char BF [ XFS_SIZE_4096 ];
    bool IsCart;
    struct XFSNode * KartNode;

    RCt = 0;
    * BF = 0;
    IsCart = false;
    KartNode = NULL;

    if ( Directory == NULL || Node == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _IsPathCart (
                    Directory,
                    Node -> path,
                    Name,
                    BF,
                    sizeof ( BF ),
                    & IsCart
                    );
    if ( RCt == 0 ) {
        if ( IsCart ) {
            RCt = XFSKartNodeMake ( Name, BF, NULL, & KartNode );
            if ( RCt == 0 ) {
                RCt = XFSContNodeAddChild ( 
                                        & ( Node -> node . node ),
                                        KartNode
                                        );
            }
            else {
                    /*  Some Karts could be damaged 
                     |  we should think about it later
                     */
                KOutMsg ( "Invalid Kart file [%s]\n", BF );
                RCt = 0;
            }
        }
    }

    return RCt;
}   /* _LoadKartItem () */

static 
rc_t CC
_LoadKartScanPath ( struct XFSKartCollectionNode * Node )
{
    rc_t RCt;
    struct KDirectory * Directory;
    KNamelist * List;
    const char * Name;
    uint32_t Idx;
    uint32_t ListCount;

    RCt = 0;
    Directory = NULL;
    List = NULL;
    Name = NULL;
    Idx = 0;
    ListCount = 0;

    if ( Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> path == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KDirectoryNativeDir ( & Directory );
    if ( RCt == 0 ) {
        RCt = KDirectoryList (
                            Directory,
                            & List,
                            NULL,
                            NULL,
                            Node -> path
                            );
        if ( RCt == 0 ) {
            RCt = KNamelistCount ( List, & ListCount );
            if ( RCt == 0 ) {
                for ( Idx = 0; Idx < ListCount; Idx ++ ) {
                    RCt = KNamelistGet ( List, Idx, & Name );
                    if ( RCt == 0 ) {
                        RCt = _LoadKartItem ( Directory, Node, Name );
                    }
                    if ( RCt != 0 ) {
                        break;
                    }
                }
            }

            KNamelistRelease ( List );
        }

        KDirectoryRelease ( Directory );
    }

    return RCt;
}   /* _LoadKartScanPath () */

static 
rc_t CC
_LoadKartScanChildren (
                    const struct XFSModel * Model,
                    const struct XFSModelNode * Template,
                    struct XFSKartCollectionNode * Node
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

                    RCt = XFSNodeMake (
                                    Model,
                                    ChildName,
                                    ChildAlias,
                                    & TheNode
                                    );
                    if ( RCt == 0 ) {
                        RCt = XFSContNodeAddChild (
                                            & ( Node -> node . node ),
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

    return RCt;
}   /* _LoadKartScanChildren () */

static
rc_t CC
_KartCollectionNodeDispose ( struct XFSContNode * self )
{
    rc_t RCt;
    struct XFSKartCollectionNode * Node;

    RCt = 0;
    Node = ( struct XFSKartCollectionNode * ) self;

    if ( Node == NULL ) {
        return 0;
    }

    if ( Node -> path != NULL ) {
        free ( ( char * ) Node -> path );
        Node -> path = NULL;
    }

    free ( Node );

    return RCt;
}   /* _KartCollectionNodeDispose () */

static
rc_t CC
_KartCollectionNodeMake (
            const char * Name,
            const char * Path,  /* Could be NULL */
            const char * Perm,
            struct XFSKartCollectionNode ** Node
)
{
    rc_t RCt;
    struct XFSKartCollectionNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    if ( Node != NULL ) { 
        * Node = NULL;
    }

    if ( Node == NULL || Path == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    KartNode = calloc ( 1, sizeof ( struct XFSKartCollectionNode ) );
    if ( KartNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSContNodeInit (
                            & ( KartNode -> node . node ),
                            Name,
                            Perm,
                            _sFlavorOfKartCollection,
                            _KartCollectionNodeDispose
                            );
        if ( RCt == 0 ) {
            RCt = XFS_StrDup ( Path, & ( KartNode -> path ) );
            if ( RCt == 0 ) {
                * Node = KartNode;
            }
        }
    }

    return RCt;
}   /* _KartCollectionNodeMake () */

LIB_EXPORT
rc_t CC
XFSKartCollectionNodeMake (
            const char * Name,
            const char * Path,
            const char * Perm,
            struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSKartCollectionNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    if ( Node != NULL ) { 
        * Node = NULL;
    }

    if ( Node == NULL || Path == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _KartCollectionNodeMake ( Name, Path, Perm, & KartNode );
    if ( RCt == 0 ) {
        RCt = _LoadKartScanPath ( KartNode );
        if ( RCt == 0 ) {
            * Node = & ( KartNode -> node . node );
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
printf ( "XFSKartNodeMake ND[0x%p] NM[%s] TP[%d]\n", ( void * ) Node, Name, Type );
*/

    return RCt;
}   /* XFSKartCollectionNodeMake () */

static
rc_t CC
_KartCollectionNodeMakeFromModel (
                    const char * Name,
                    const struct XFSModel * Model,
                    const struct XFSModelNode * Template,
                    struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSKartCollectionNode * KartNode;

    RCt = 0;
    KartNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Name == NULL || Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = _KartCollectionNodeMake (
                                Name,
                                "generic",
                                XFSModelNodeSecurity ( Template ),
                                & KartNode
                                );
    if ( RCt == 0 ) {
        RCt = _LoadKartScanChildren ( Model, Template, KartNode );
        if ( RCt == 0 ) {
            * Node = & ( KartNode -> node . node );
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
printf ( "KartNodeMakeFromModel ND[0x%p] NM[%s] TP[%d]\n", ( void * ) Node, Name, Type );
*/

    return RCt;
}   /* _KartCollectionNodeMakeFromModel () */

/*)))
 |||
 +++  There KartNode Find
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
_KartCollectionNodeConstructorEx (
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
    const char * Path;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;
    Path = NULL;

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    Path = XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE );
    if ( Path != NULL ) {
        RCt = XFSKartCollectionNodeMake (
                                    NodeName,
                                    Path,
                                    XFSModelNodeSecurity ( Template ),
                                    & TheNode
                                    );
    }
    else {
        RCt = _KartCollectionNodeMakeFromModel (
                                            NodeName,
                                            Model,
                                            Template,
                                            & TheNode
                                            );
    }

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
}   /* _KartCollectionNodeConstructorEx () */

/*)))
 |||
 +++    FileNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_KartCollectionNodeConstructor (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        const struct XFSNode ** Node
)
{
    rc_t RCt = 0;

    RCt = _KartCollectionNodeConstructorEx (
                            Model,
                            Template,
                            Alias,
                            kxfsFile,
                            Node
                            );

/*
printf ( "_KartCollectionNodeConstructor ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/


    return RCt;
}   /* _KartCollectionNodeConstructor () */

static
rc_t CC
_KartCollectionNodeValidator (
                        const struct XFSModel * Model,
                        const struct XFSModelNode * Template,
                        const char * Alias,
                        uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
printf ( "_KartCollectionNodeValidator ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _KartCollectionNodeValidator () */

static const struct XFSTeleport _sKartCollectionNodeTeleport = {
                                        _KartCollectionNodeConstructor,
                                        _KartCollectionNodeValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSKartCollectionProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sKartCollectionNodeTeleport;

    return 0;
}   /* XFSKartCollectionProvider () */
