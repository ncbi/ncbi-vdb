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
#include <klib/log.h>
#include <kproc/lock.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>

#include "mehr.h"
#include "zehr.h"
#include "schwarzschraube.h"

#include <sysalloc.h>

/*)))
 |||
 +++    Tree and all the worst
 |||
(((*/

/*)))
 |||
 +++    We believe that here could be many trees, so refcounts
 +++    Also, we will not keep model in memory, because it could be
 +++    reloaded from Resource at any time and compared by version
 |||
(((*/
struct XFSNode;

struct XFSTree {
    KRefcount refcount;

        /* Yes, it is pointer for something */
    struct XFSNode * Root;

    const char * Resource;
    const char * Version;
};

/*)
 /      Useful farriables
(*/
static const char * _sXFSTree_classname = "XFSTree";

/*)))
 |||
 +++    Verificaion ... does not know yet what is it, but ...
 |||
(((*/

static
rc_t CC
_VerifyTreeModelNode (
                const struct XFSModel * Model,
                const struct XFSModelNode * Node,
                uint32_t Flags
)
{
    rc_t RCt;
    const char * Prop, * Name;
    bool DoCheck;

    RCt = 0;
    Prop = NULL;
    Name = NULL;

    if ( Model == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    DoCheck = Flags != 0
            && ( Flags & XFS_CHECK_INTEGRITY ) == XFS_CHECK_INTEGRITY;

    if ( DoCheck ) {
            /* First we should check that name exists and not NULL */
        Name = XFSModelNodeName ( Node );
        if ( Name == NULL ) {
/*
LogMsg ( klogDebug, "Node name missed" );
*/
            return XFS_RC ( rcInvalid );
        }

            /* checking noode with AS name */
        Prop = XFSModelNodeAs ( Node );
        if ( Prop != NULL ) {
            if ( XFSModelLookupNode ( Model, Prop ) == NULL ) {
/*
pLogMsg ( klogDebug, "Node [$(name)] can not stat node with 'AS' name [$(name)]", "name=%s,as=%s", Name, Prop );
*/
                return XFS_RC ( rcInvalid );
            }
        }

            /* there should not be node with name as property 'label' */
        Prop = XFSModelNodeLabel ( Node );
        if ( Prop != NULL ) {
            /* TODO .... incorrect check, You should check all labels
                for all modell nodes
             */ 
            if ( XFSModelLookupNode ( Model, Prop ) != NULL ) {
/*
pLogMsg ( klogDebug, "Node [$(name)] with 'LABEL' [$(label)] already exists", "name=%s,label=%s", Name, Prop );
*/
                return XFS_RC ( rcInvalid );
            }
        }
    }

    return RCt;
}   /* _VerifyTreeModelNoode () */

LIB_EXPORT
rc_t CC
XFSTreeVerifyModel ( const struct XFSModel * Model, uint32_t Flags )
{
    rc_t RCt;
    const struct XFSModelNode * Root;
    bool DoCheck;

    RCt = 0;
    Root = NULL;

    if ( Model == NULL ) {
        return XFS_RC ( rcNull );
    }

    DoCheck = Flags != 0
            && ( Flags & XFS_CHECK_INTEGRITY ) == XFS_CHECK_INTEGRITY;

    if ( DoCheck ) {
        Root = XFSModelRootNode ( Model );
        if ( Root == NULL ) {
            return XFS_RC ( rcInvalid );
        }

        RCt = _VerifyTreeModelNode ( Model, Root, Flags );
    }

    return RCt;
}   /* XFSTreeVerifyModel () */

LIB_EXPORT
rc_t CC
XFSTreeMake ( const struct XFSModel * Model, struct XFSTree ** Tree )
{
    rc_t RCt;
    struct XFSTree * tTree;

    RCt = 0;

    if ( Model == NULL || Tree == NULL ) { 
        return XFS_RC ( rcNull );
    }

    * Tree = NULL;

        /* Creating a tree */
    tTree = calloc ( 1, sizeof ( struct XFSTree ) );
/*
pLogMsg ( klogDebug, " |<- TreeMake ( $(tree) )", "tree=%p", ( void * ) tTree );
*/
    if ( tTree == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( XFS_StrDup (
                    XFSModelResource ( Model ),
                    & ( tTree -> Resource )
                    ) != 0
    ) {
        XFSTreeDispose ( tTree );
        return XFS_RC ( rcExhausted );
    }

    if ( XFSModelVersion ( Model ) != NULL ) {
        if ( XFS_StrDup (
                        XFSModelVersion ( Model ),
                        & ( tTree -> Version ) ) != 0
        ) {
            XFSTreeDispose ( tTree );
            return XFS_RC ( rcExhausted );
        }
    }
    else {
        tTree -> Version = NULL;
    }

    RCt = XFSNodeMake (
                    Model,
                    XFSModelNodeName ( XFSModelRootNode ( Model ) ),
                    NULL,
                    ( const struct XFSNode ** ) & ( tTree -> Root )
                    );
    if ( RCt == 0 ) {
        KRefcountInit (
                        & ( tTree -> refcount ),
                        1,
                        _sXFSTree_classname,
                        "XFSTreeMake",
                        "Tree"
                        );
    }

    if ( RCt == 0 ) {
        * Tree = tTree;
    }
    else {
        XFSTreeDispose ( tTree );

        tTree = NULL;
    }

/*
pLogMsg ( klogDebug, " ->| TreeMake ( $(tree) )", "tree=%p", ( void * ) * Tree );
*/

    return RCt;
}   /* XFSTreeMake () */

LIB_EXPORT
rc_t CC
XFSTreeDispose ( struct XFSTree * self )
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, " |<- TreeDispose ( $(tree) )", "tree=%p", ( void * ) self );
*/

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> Root != NULL ) {
        XFSNodeRelease ( self -> Root );

        self -> Root = NULL;
    }

    if ( self -> Resource != NULL ) {
        free ( ( char * ) self -> Resource );

        self -> Resource = NULL;
    }

    if ( self -> Version != NULL ) {
        free ( ( char * ) self -> Version );

        self -> Version = NULL;
    }

    KRefcountWhack ( & ( self -> refcount ), _sXFSTree_classname );

    free ( self );

    return RCt;
}   /* XFSTreeDispose () */

LIB_EXPORT
rc_t CC
XFSTreeAddRef ( const struct XFSTree * self )
{
    rc_t RCt;

    RCt = 0;

    if ( self != NULL ) {
        switch ( KRefcountAdd (
                    & ( self -> refcount ),
                    _sXFSTree_classname
                    )
        ) {
            case krefOkay :
                RCt = 0;
                break;
            case krefZero :
            case krefLimit :
            case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
            default :
                RCt = XFS_RC ( rcUnknown );
                break;
        }
    }
    else {
        RCt = XFS_RC ( rcNull );
    }

    return RCt;
}   /* XFSTreeAddRef () */

LIB_EXPORT
rc_t CC
XFSTreeRelease ( const struct XFSTree * self )
{
    rc_t RCt;

    RCt = 0;
    if ( self != NULL ) {
        switch ( KRefcountDrop (
                        & ( self -> refcount ),
                        _sXFSTree_classname
                        )
        ) {
            case krefOkay :
            case krefZero :
                RCt = 0;
                break;
            case krefWhack :
                RCt = XFSTreeDispose ( ( struct XFSTree * ) self );
                break;
            case krefNegative :
                RCt = XFS_RC ( rcInvalid );
                break;
            default :
                RCt = XFS_RC ( rcUnknown );
                break;
        }
    }

    return RCt;
}   /* XFSTreeRelease () */

LIB_EXPORT
rc_t CC
XFSTreeFindNode (
                const struct XFSTree * self,
                const char * Path,
                const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSPath * XPath;

    RCt = 0;
    XPath = NULL;

    if ( self == NULL || Path == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

    if ( self -> Root == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSPathMake ( & XPath, true, Path );
    if ( RCt == 0 ) {

        RCt = XFSNodeFindNode ( self -> Root, XPath, 0, Node );
        if ( RCt != 0 ) {
            RCt = XFSNodeNotFoundMake ( XFSPathName ( XPath ), Node );
        }

        XFSPathRelease ( XPath );
    }

    return RCt;
}   /* XFSTreeFindNode () */

XFS_EXTERN
rc_t CC
XFSTreeFindNodeForPath (
                const struct XFSTree * self,
                const struct VPath * Path,
                const struct XFSNode ** Node
)
{
    rc_t RCt;
    char Buf [ XFS_SIZE_4096 ];

    RCt = 0;
    * Buf = 0;

    if ( self == NULL || Path == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;


    RCt = XFS_ReadVPath_ZHR ( Path, Buf, sizeof ( Buf ) , "" );
    if ( RCt == 0 ) {
        RCt = XFSTreeFindNode ( self, Buf, Node );
    }

    return RCt;
}   /* XFSTreeFindNodeForPath () */

/*)))
 |||
 +++    Tree Depot and all the worstterrerr
 |||
(((*/

struct XFSTreeDepot {
    KLock * mutabor;

    struct XFSTree * Tree;
};

LIB_EXPORT
rc_t CC
XFSTreeDepotMake ( const struct XFSTreeDepot ** Depot )
{
    rc_t RCt;
    struct XFSTreeDepot * TheDepot;

    RCt = 0;
    TheDepot = NULL;

    if ( Depot == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Depot = NULL;

    TheDepot = calloc ( 1, sizeof ( struct XFSTreeDepot ) );
    if ( TheDepot == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = KLockMake ( & ( TheDepot -> mutabor ) );
    if ( RCt == 0 ) {
        * Depot = TheDepot;
    }
    else {
        free ( TheDepot );

        * Depot = NULL;
    }

/*
pLogMsg ( klogDebug, " |<- TreeDepotMake ( $(tree) )", "tree=%p", ( void * ) * Depot );
*/

    return RCt;
}   /* XFSTreeDepotMake () */

LIB_EXPORT
rc_t CC
XFSTreeDepotDispose ( const struct XFSTreeDepot * self )
{
    rc_t RCt;
    struct XFSTree * Tree;
    KLock * mutabor;

    RCt = 0;
    Tree = NULL;
    mutabor = NULL;

/*
pLogMsg ( klogDebug, " |<- TreeDepotDispose ( $(tree) )", "tree=%p", ( void * ) self );
*/

    if ( self == 0 ) {
        return 0;
    }

    /* That is not thread safe method :lol: */

    Tree = ( struct XFSTree * ) self -> Tree;
    mutabor = self -> mutabor;

    free ( ( struct XFSTreeDepot * ) self );

    XFSTreeRelease ( Tree );
    KLockRelease ( mutabor );

    return RCt;
}   /* XFSTreeDepotDispose () */

LIB_EXPORT
rc_t CC
XFSTreeDepotGet (
            const struct XFSTreeDepot * self,
            const struct XFSTree ** Tree
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || Tree == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Tree = NULL;

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {

        RCt = XFSTreeAddRef ( self -> Tree );
        if ( RCt == 0 ) {
            * Tree = self -> Tree;
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSTreeDepotGet () */

LIB_EXPORT
rc_t CC
XFSTreeDepotSet (
            const struct XFSTreeDepot * self,
            const struct XFSTree * Tree
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || Tree == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {
        RCt = XFSTreeAddRef ( Tree );
        if ( RCt == 0 ) {
            if ( self -> Tree != NULL ) {

                    /*)) I do not check return code here, because we
                     //  are going to drop that tree anyway.
                    ((*/
                XFSTreeRelease ( self -> Tree );
            }

            ( ( struct XFSTreeDepot * ) self ) -> Tree =
                                    ( struct XFSTree * ) Tree;
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSTreeDepotSet () */

LIB_EXPORT
rc_t CC
XFSTreeDepotVersion (
                const struct XFSTreeDepot * self,
                char * Buffer,
                size_t BufferSize
)
{
    rc_t RCt;
    const struct XFSTree * Tree;

    RCt = 0;
    Tree = NULL;

    if ( self == NULL || Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    * Buffer = 0;

    RCt = XFSTreeDepotGet ( self, & Tree );
    if ( RCt == 0 ) { 
        if ( Tree != NULL ) {
            if ( Tree -> Version != NULL ) {
                string_copy_measure (
                                    Buffer,
                                    BufferSize,
                                    Tree -> Version
                                    );
            }

            XFSTreeRelease ( Tree );
        }
        else {
            RCt = XFS_RC ( rcNotFound );
        }
    }

    return RCt;
}   /* XFSTreeDepotVersion () */

LIB_EXPORT
rc_t CC
XFSTreeDepotResource (
                const struct XFSTreeDepot * self,
                char * Buffer,
                size_t BufferSize
)
{
    rc_t RCt;
    const struct XFSTree * Tree;

    RCt = 0;
    Tree = NULL;

    if ( self == NULL || Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    * Buffer = 0;

    RCt = XFSTreeDepotGet ( self, & Tree );
    if ( RCt == 0 ) { 
        if ( Tree != NULL ) {
            if ( Tree -> Resource != NULL ) {
                string_copy_measure (
                                    Buffer,
                                    BufferSize,
                                    Tree -> Resource
                                    );
            }

            XFSTreeRelease ( Tree );
        }
        else {
            RCt = XFS_RC ( rcNotFound );
        }
    }

    return RCt;
}   /* XFSTreeDepotResource () */

LIB_EXPORT
rc_t CC
XFSTreeDepotFindNode (
                    const struct XFSTreeDepot * self,
                    const char * Path,
                    const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSTree * Tree;

    RCt = 0;
    Tree = NULL;

    if ( self == NULL || Path == NULL || Node == 0 ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    RCt = XFSTreeDepotGet ( self, & Tree );
    if ( RCt == 0 ) { 
        if ( Tree != NULL ) {
            RCt = XFSTreeFindNode ( Tree, Path, Node );

            XFSTreeRelease ( Tree );
        }
        else {
            RCt = XFS_RC ( rcNotFound );
        }
    }

    return RCt;
}   /* XFSTreeDepotFindNode () */

LIB_EXPORT
rc_t CC
XFSTreeDepotFindNodeForPath (
                    const struct XFSTreeDepot * self,
                    const struct VPath * Path,
                    const struct XFSNode ** Node
)
{
    rc_t RCt;
    char Buf [ XFS_SIZE_4096 ];

    RCt = 0;

    if ( self == NULL || Path == NULL || Node == 0 ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

    RCt = XFS_ReadVPath_ZHR ( Path, Buf, sizeof ( Buf ) , "" );
    if ( RCt == 0 ) {
        RCt = XFSTreeDepotFindNode ( self, Buf, Node );
    }

    return RCt;
}   /* XFSTreeDepotFindNodeForPath () */

