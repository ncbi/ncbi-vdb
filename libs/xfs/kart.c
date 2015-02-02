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
#include "xkart.h"

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

struct XFSKartNode {
    struct XFSContNode node;

    struct XFSKart * kart;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* KartItemNode lives here                                       *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    KartItemNode virtual table is Living here :lol:
 |||
(((*/

static
rc_t CC
_AddKartItem ( struct XFSKartNode * Node, const char * ItemName )
{
    rc_t RCt;
    struct XFSDoc * ItemDoc;
    struct XFSNode * ItemNode;
    const struct XFSKartItem * KartItem;
    const char * Str;

    RCt = 0;
    ItemDoc = NULL;
    ItemNode = NULL;
    KartItem = NULL;
    Str = NULL;

    if ( Node == NULL || ItemName == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> kart == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    KartItem = XFSKartGet ( Node -> kart, ItemName );
    if ( KartItem == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSTextDocMake ( & ItemDoc );

    if ( RCt != 0 ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSTextDocAppend ( ItemDoc, "KART ITEM\n====\n" );

    if ( RCt == 0 ) {
        RCt = XFSTextDocAppend (
                            ItemDoc,
                            "Project: %s\n",
                            XFSKartItemProject ( KartItem )
                            );
    }

    if ( RCt == 0 ) {
        RCt = XFSTextDocAppend (
                            ItemDoc,
                            "Id: %s\n",
                            XFSKartItemId ( KartItem )
                            );
    }

    if ( RCt == 0 ) {
        Str = XFSKartItemAccession ( KartItem );
        if ( Str != NULL ) {
            RCt = XFSTextDocAppend ( ItemDoc, "Accession: %s\n", Str );
        }
    }

    if ( RCt == 0 ) {
        Str = XFSKartItemName ( KartItem );
        if ( Str != NULL ) {
            RCt = XFSTextDocAppend ( ItemDoc, "Name: %s\n", Str );
        }
    }

    if ( RCt == 0 ) {
        Str = XFSKartItemDescription ( KartItem );
        if ( Str != NULL ) {
            RCt = XFSTextDocAppend ( ItemDoc, "Description: %s\n", Str );
        }
    }

    if ( RCt == 0 ) {
        RCt = XFSDocNodeMakeWithFlavor (
                                        ItemDoc,
                                        ItemName,
                                        NULL,
                                        _sFlavorOfKartItem,
                                        & ItemNode
                                        );
        if ( RCt == 0 ) {
            RCt = XFSContNodeAddChild (
                                    & ( Node -> node . node ),
                                    ItemNode
                                    );
            /* We do not dispose node here, but on the caller level */
        }
    }

    XFSDocRelease ( ItemDoc );
    ItemDoc = NULL;

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
_AddReadMe ( struct XFSKartNode * Node, struct KNamelist * List )
{
    rc_t RCt;
    struct XFSDoc * Doc;
    struct XFSNode * Readme;
    const char * Name, * Accession, * Description, * DisplayName;
    uint32_t ListQ, ListI;
    const struct XFSKartItem * TheItem;

    RCt = 0;
    Readme = NULL;
    Doc = NULL;
    Name = Accession = Description = DisplayName = NULL;
    ListQ = ListI = 0;
    TheItem = NULL;

    RCt = XFSTextDocMake ( & Doc );
    if ( RCt != 0 ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSTextDocAppend ( Doc, "KART\n====\nSource : %s\n\nITEMS\n====\n", XFSKartPath ( Node -> kart ) );
    if ( RCt == 0 ) {
        RCt = KNamelistCount ( List, & ListQ );
        if ( RCt == 0 ) {
            for ( ListI = 0; ListI < ListQ; ListI ++ ) {
                RCt = KNamelistGet ( List, ListI, & DisplayName );
                if ( RCt != 0 ) { 
                    break;
                }

                TheItem = XFSKartGet ( Node -> kart, DisplayName );
                if ( TheItem == NULL ) {
                    RCt = XFS_RC ( rcInvalid );
                    break;
                } 

                Name = XFSKartItemName ( TheItem );
                Accession = XFSKartItemAccession ( TheItem );
                Description = XFSKartItemDescription ( TheItem );

                RCt = XFSTextDocAppend (
                                Doc,
                                "%s|%s|%s|%s|%s|%s\n",
                                DisplayName,
                                XFSKartItemProject ( TheItem ),
                                XFSKartItemId ( TheItem ),
                                ( Accession == NULL ? "" : Accession),
                                ( Name == NULL ? "" : Name),
                                ( Description == NULL ? "" : Description)
                                );
                if ( RCt != 0 ) {
                    break;
                }
            }
        }

        if ( RCt == 0 ) {
            RCt = XFSDocNodeMakeWithFlavor (
                                            Doc,
                                            "README.txt",
                                            NULL,
                                            _sFlavorOfReadMe,
                                            & Readme
                                            );
            if ( RCt == 0 ) {
                RCt = XFSContNodeAddChild (
                                        & ( Node -> node . node ),
                                        Readme
                                        );
            }
        }
    }

    XFSDocRelease ( Doc );

    return RCt;
}   /* _AddReadMe () */

static
rc_t CC
_AddOriginal ( struct XFSKartNode * Node )
{
    rc_t RCt;
    struct XFSNode * FileNode;

    RCt = 0;

    if ( Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSFileNodeMake (
                        XFSKartPath ( Node -> kart ),
                        "KART.krt",
                        NULL,
                        & FileNode
                        );
    if ( RCt == 0 ) {
        RCt = XFSContNodeAddChild (
                                & ( Node -> node . node ),
                                ( struct XFSNode * ) FileNode
                                );
        if ( RCt != 0 ) {
            XFSNodeRelease ( FileNode );
        }
    }

    return RCt;
}   /* _AddOriginal () */

static 
rc_t CC
_LoadKart ( struct XFSKartNode * Node )
{
    rc_t RCt;
    struct KNamelist * List;
    uint32_t ListQ, ListI;
    const char * ListN;

    RCt = 0;
    List = NULL;
    ListQ = ListI = 0;
    ListN = NULL;

    if ( Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> kart == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _AddOriginal ( Node );
    if ( RCt == 0 ) {

        RCt = XFSKartList ( Node -> kart, & List );
        if ( RCt == 0 ) {
            RCt = KNamelistCount ( List, & ListQ );
            if ( RCt == 0 ) {
                for ( ListI = 0; ListI < ListQ; ListI ++ ) {
                    RCt = KNamelistGet ( List, ListI, & ListN );
                    if ( RCt != 0 ) {
                        break;
                    }
                    RCt = _AddKartItem ( Node, ListN );
                    if ( RCt != 0 ) {
                        break;
                    }
                }
                if ( RCt == 0 ) {
                    RCt = _AddReadMe ( Node, List );
                }
            }

            KNamelistRelease ( List );
        }
    }

    return RCt;
}   /* _LoadKart () */

static
rc_t CC
_KartNodeDispose ( struct XFSContNode * self )
{
    rc_t RCt;
    struct XFSKartNode * Node;

    RCt = 0;
    Node = ( struct XFSKartNode * ) self;

    if ( Node == NULL ) {
        return 0;
    }

    if ( Node -> kart != NULL ) {
        XFSKartRelease ( Node -> kart );
        Node -> kart = NULL;
    }

    free ( Node );

    return RCt;
}   /* _KartNodeDispose () */

LIB_EXPORT
rc_t CC
XFSKartNodeMake (
            const char * Name,
            const char * Path,
            const char * Perm,
            struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSKartNode * KartNode;
    struct XFSKart * TheKart;
    char NameR [ XFS_SIZE_128 ];
    size_t NameS;

    RCt = 0;
    KartNode = NULL;
    TheKart = NULL;
    * NameR = 0;
    NameS = 0;

    if ( Node != NULL ) { 
        * Node = NULL;
    }

    if ( Node == NULL || Path == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSKartMake ( Path, & TheKart );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*) Here we are trimming extention '.krt' by brutal force.
         /  will beautify it later
        (*/
    NameS = string_size ( Name );
    if ( NameS != string_copy_measure ( NameR, sizeof ( NameR ), Name ) ) {
        return XFS_RC ( rcExhausted );
    }
    if ( NameR [ NameS - 1 ] == 't' && NameR [ NameS - 2 ] == 'r'
        && NameR [ NameS - 3 ] == 'k' && NameR [ NameS - 4 ] == '.' ) {
        NameR [ NameS - 4 ] = 0;
    }

    KartNode = calloc ( 1, sizeof ( struct XFSKartNode ) );
    if ( KartNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSContNodeInit (
                            & ( KartNode -> node . node ),
                            NameR,
                            Perm,
                            _sFlavorOfKart,
                            _KartNodeDispose
                            );
        if ( RCt == 0 ) {
            KartNode -> kart = TheKart;

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
printf ( "XFSKartNodeMake ND[0x%p] NM[%s] TP[%d]\n", ( void * ) Node, Name, Type );
*/

    return RCt;
}   /* XFSKartNodeMake () */

/*)))
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

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = XFSKartNodeMake (
                    NodeName,
                    XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE ),
                    XFSModelNodeSecurity ( Template ),
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
printf ( "_KartNodeConstructor ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
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
printf ( "_KartNodeValidator ( 0x%p, 0x%p (\"%s\"), \"%s\" )\n", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
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
XFSKartProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sKartNodeTeleport;

    return 0;
}   /* XFSKartProvider () */
