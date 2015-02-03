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
#include <klib/container.h>
#include <klib/refcount.h>
#include <klib/namelist.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>

#include "mehr.h"
#include "zehr.h"
#include "ncon.h"

#include <sysalloc.h>

#include <stdio.h>

/*)))
 |||
 +++    Collection of node containers
 |||
(((*/

/*))
 ((     Simple Node Container
  ))
 ((*/

static const char * _sXFSNodeContainer_classname = "XFSNodeContainer";

struct XFSNodeContainer {
    BSTree tree;
    KRefcount refcount;
};

struct XFSNodeContainerNode {
    BSTNode node;

    const struct XFSNode * Node;
};


/*))))   Here we are
 ((((*/
static
rc_t CC
_NodeContainerNodeDispose ( const struct XFSNodeContainerNode * self )
{
    rc_t RCt;
    struct XFSNodeContainerNode * Node;

    RCt = 0;

/*
printf ( "_NodeContainerNodeDispose ( 0x%p )\n", ( void * ) self );
*/

    if ( self == NULL ) {
        return 0;
    }

    Node = ( struct XFSNodeContainerNode * ) self;

    if ( Node -> Node != NULL ) {
        XFSNodeRelease ( Node -> Node );
        Node -> Node = NULL;
    }

    free ( Node );

    return RCt;
}   /* _NodeContainerNodeDispose () */

static
rc_t CC
_NodeContainerNodeMake (
                    const struct XFSNode * Node, 
                    const struct XFSNodeContainerNode ** RetNode
)
{
    rc_t RCt;
    struct XFSNodeContainerNode * TheNode;

    RCt = 0;

    if ( Node == NULL || RetNode == NULL ) {
        return XFS_RC ( rcNull );
    }

    * RetNode = NULL;

    TheNode = calloc ( 1, sizeof ( struct XFSNodeContainerNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        TheNode -> Node = Node;

        * RetNode = TheNode;
    }

    if ( RCt != 0 ) {
        _NodeContainerNodeDispose ( TheNode );
        * RetNode = NULL;   /* for any case :lol: */
    }
/*
printf ( "_NodeContainerNodeMake ( 0x%p )\n", ( void * ) * RetNode );
*/

    return RCt;
}   /* _NodeContainerNodeMake () */

/*))))   Here we are. Part II
 ((((*/

LIB_EXPORT
rc_t CC
XFSNodeContainerMake (
            const struct XFSNodeContainer ** Container
)
{
    rc_t RCt;
    struct XFSNodeContainer * NewCont;

    RCt = 0;
    NewCont = NULL;

    if ( Container == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Container = NULL;

    NewCont = calloc ( 1 , sizeof ( struct XFSNodeContainer ) );
    if ( NewCont == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        BSTreeInit ( & ( NewCont -> tree ) );

        KRefcountInit (
                    & ( NewCont -> refcount ),
                    1,
                    _sXFSNodeContainer_classname,
                    "XFSNodeContainerMake",
                    "XFSNodeContainer"
                    );

        * Container = NewCont;
    }

/*
printf ( "XFSNodeContainerMake ( 0x%p )\n", ( void * ) * Container );
*/

    return RCt;
}   /* XFSNodeContainerMake () */

static
void CC
_TreeWhackCallback ( BSTNode * Node, void * Data )
{
    struct XFSNodeContainerNode * CNode =
                            ( struct XFSNodeContainerNode * ) Node;

    if ( CNode != NULL ) {
        _NodeContainerNodeDispose ( CNode );
    }
}   /* _TreeWhackCallback () */

LIB_EXPORT
rc_t CC
XFSNodeContainerDispose ( const struct XFSNodeContainer * self )
{
/*
printf ( "XFSNodeContainerDispose ( 0x%p )\n", ( void * ) self );
*/

    if ( self == NULL ) {
        return 0;
    }

    BSTreeWhack (
                ( BSTree * ) & ( self -> tree ),
                _TreeWhackCallback,
                NULL
                );

    KRefcountWhack (
                ( KRefcount * ) & ( self -> refcount ),
                _sXFSNodeContainer_classname
                );

    free ( ( struct XFSNodeContainer * ) self );

    return 0;
}   /* XFSNodeContainerDispose () */

LIB_EXPORT
rc_t CC
XFSNodeContainerAddRef ( const struct XFSNodeContainer * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self != NULL ) {
        switch ( KRefcountAdd (
                            & ( self -> refcount ),
                            _sXFSNodeContainer_classname
                            )
        ) {
            case krefOkay :     RCt = 0;                    break;

            case krefZero :
            case krefLimit :
            case krefNegative : RCt = XFS_RC ( rcInvalid ); break;

            default :           RCt = XFS_RC ( rcUnknown ); break;
        }
    }
    else {
        RCt = XFS_RC ( rcNull );
    }

    return RCt;
}   /* XFSNodeContainerAddRef () */

LIB_EXPORT
rc_t CC
XFSNodeContainerRelease ( const struct XFSNodeContainer * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self != NULL ) {
        switch ( KRefcountDrop (
                            & ( self -> refcount ),
                            _sXFSNodeContainer_classname
                            )
        ) {
            case krefOkay :
            case krefZero : RCt= 0;                         break;

            case krefWhack :
                    RCt = XFSNodeContainerDispose ( self ); break;

            case krefNegative : RCt = XFS_RC ( rcInvalid ); break;

            default : RCt = XFS_RC ( rcUnknown );           break;
        }
    }

    return RCt;
}   /* XFSNodeContainerRelease () */

LIB_EXPORT
bool CC
XFSNodeContainerHas (
                const struct XFSNodeContainer * self,
                const char * Name
)
{
    const struct XFSNode * Node = NULL;

    if ( XFSNodeContainerGet ( self, Name, & Node ) == 0 ) {
        return ( Node != NULL );
    }

    return false;
}   /* XFSNodeContainerHas () */

static
int CC
_NodeContainerCompare ( const void * Item, const BSTNode * Node )
{
    return XFS_StringCompare4BST_ZHR (
        ( const char * ) Item,
        ( ( const struct XFSNodeContainerNode * ) Node ) -> Node -> Name
        );
}   /* _NodeContainerCompare () */

static
rc_t CC
_NodeContainerNodeGet (
                const struct XFSNodeContainer * self,
                const char * Name,
                const struct XFSNodeContainerNode ** Node
)
{
    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = ( struct XFSNodeContainerNode * ) BSTreeFind (
                                                & ( self -> tree ),
                                                Name,
                                                _NodeContainerCompare
                                                );

    return * Node == NULL ? XFS_RC ( rcNotFound ) : 0;
}   /* _NodeContainerNodeGet () */

LIB_EXPORT
rc_t CC
XFSNodeContainerGet (
                const struct XFSNodeContainer * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSNodeContainerNode * TheNode;

    RCt = 0;

    RCt = _NodeContainerNodeGet ( self, Name, & TheNode );

    if ( RCt == 0 ) {
        * Node = TheNode -> Node;

        if ( * Node == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
    }

    return RCt;
}   /* XFSNodeContainerGet () */

static
int CC
_NodeContainerInsert ( const BSTNode * N1, const BSTNode * N2 )
{
    return XFS_StringCompare4BST_ZHR (
            ( ( struct XFSNodeContainerNode * ) N1 ) -> Node -> Name,
            ( ( struct XFSNodeContainerNode * ) N2 ) -> Node -> Name
            );
}   /* _NodeContainerInsert () */

LIB_EXPORT
rc_t CC
XFSNodeContainerAdd (
            const struct XFSNodeContainer * self,
            const struct XFSNode * Node
)
{
    rc_t RCt;
    struct XFSNodeContainer * Container;
    struct XFSNodeContainerNode * TheNode;

    RCt = 0;
    TheNode = NULL;
    Container = ( struct XFSNodeContainer * ) self;

    if ( Container == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( XFSNodeContainerHas ( Container, Node -> Name ) ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _NodeContainerNodeMake (
                    Node,
                    ( const struct XFSNodeContainerNode ** ) & TheNode
                    );
    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                        & ( Container -> tree ),
                        & ( TheNode -> node ),
                        _NodeContainerInsert
                        );
    }

    return RCt;
}   /* XFSNodeContainerAdd () */

LIB_EXPORT
rc_t CC
XFSNodeContainerDel (
                const struct XFSNodeContainer * self,
                const char * NodeName
)
{
    rc_t RCt;
    struct XFSNodeContainer * Container;
    struct XFSNodeContainerNode * Node;

    RCt = 0;
    Container = ( struct XFSNodeContainer * ) self;

    RCt = _NodeContainerNodeGet (
                    Container,
                    NodeName,
                    ( const struct XFSNodeContainerNode ** ) & Node
                    );
    if ( RCt == 0 ) {
        RCt = BSTreeUnlink (
                        & ( Container -> tree ),
                        & ( Node -> node )
                        );
        if ( RCt == 0 ) {
            RCt = _NodeContainerNodeDispose ( Node );
        }
    }

    return RCt;
}   /* XFSNodeContainerDef () */

static
void CC
_NodeContainerList ( BSTNode * Node, void * Data )
{
    struct XFSNodeContainerNode * TheNode;
    struct VNamelist * TheList;

    if ( Node != NULL && Data != NULL ) {
        TheNode = ( struct XFSNodeContainerNode * ) Node;
        TheList = ( struct VNamelist * ) Data;

        VNamelistAppend ( TheList, TheNode -> Node -> Name );
    }
}   /* _NodeContainerList () */

LIB_EXPORT
rc_t CC
XFSNodeContainerList (
                const struct XFSNodeContainer * self,
                const struct KNamelist ** List
)
{
    rc_t RCt;
    struct VNamelist * TheList;

    RCt = 0;
    TheList = NULL;

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }
    * List = NULL;

    RCt = VNamelistMake ( & TheList, 32 );
    if ( RCt == 0 ) {
        BSTreeForEach ( 
                    & ( self -> tree ),
                    false,
                    _NodeContainerList,
                    TheList
                    );

        RCt = VNamelistToConstNamelist ( TheList, List );

        VNamelistRelease ( TheList );
    }

    return RCt;
}   /* XFSNodeContainerList () */
