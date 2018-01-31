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
#include <klib/namelist.h>
#include <klib/log.h>
#include <kproc/lock.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>

#include "mehr.h"
#include "zehr.h"
#include "ncon.h"
#include "hdict.h"

#include <sysalloc.h>

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
    const struct XFSHashDict * nodes;

    KRefcount refcount;

    struct KLock * mutabor;
};

/*))))   Here we are. Part II
 ((((*/

static
void CC
_NodeContainerWhacker ( const void * Value )
{
    if ( Value != NULL ) {
        XFSNodeRelease ( ( const struct XFSNode * ) Value );
    }
}   /* _NodeContainerWhacker () */

LIB_EXPORT
rc_t CC
XFSNodeContainerMake ( const struct XFSNodeContainer ** Container )
{
    rc_t RCt;
    struct XFSNodeContainer * NewCont;

    RCt = 0;
    NewCont = NULL;

    XFS_CSAN ( Container )
    XFS_CAN ( Container )

    NewCont = calloc ( 1 , sizeof ( struct XFSNodeContainer ) );
    if ( NewCont == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSHashDictMake (
                        & ( NewCont -> nodes ),
                        _NodeContainerWhacker
                        );
        if ( RCt == 0 ) {
            KRefcountInit (
                        & ( NewCont -> refcount ),
                        1,
                        _sXFSNodeContainer_classname,
                        "XFSNodeContainerMake",
                        "XFSNodeContainer"
                        );

            RCt = KLockMake ( & ( NewCont -> mutabor ) );
            if ( RCt == 0 ) {
                * Container = NewCont;
            }
        }
    }

    if ( RCt != 0 ) {
        * Container = NULL;

        if ( NewCont != NULL ) {
            XFSNodeContainerDispose ( NewCont );
        }
    }

/*
pLogMsg ( klogDebug, "XFSNodeContainerMake ( $(node) )\n", "node=%p", ( void * ) * Container );
*/

    return RCt;
}   /* XFSNodeContainerMake () */

LIB_EXPORT
rc_t CC
XFSNodeContainerDispose ( const struct XFSNodeContainer * self )
{
/*
pLogMsg ( klogDebug, "XFSNodeContainerDispose ( $(node) )\n", "node=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        XFSHashDictDispose ( self -> nodes );

        KRefcountWhack (
                    ( KRefcount * ) & ( self -> refcount ),
                    _sXFSNodeContainer_classname
                    );

        if ( self -> mutabor != NULL ) {
            KLockRelease ( ( struct KLock * ) self -> mutabor );
            ( ( struct XFSNodeContainer * ) self ) -> mutabor = NULL;
        }

        free ( ( struct XFSNodeContainer * ) self );

    }

    return 0;
}   /* XFSNodeContainerDispose () */

LIB_EXPORT
rc_t CC
XFSNodeContainerClear ( const struct XFSNodeContainer * self )
{
    rc_t RCt;
    struct XFSNodeContainer * Container;

    RCt = 0;
    Container = ( struct XFSNodeContainer * ) self;

    if ( Container != NULL ) {
        RCt = KLockAcquire ( Container -> mutabor );
        if ( RCt == 0 ) {
            RCt = XFSHashDictDispose ( Container -> nodes );
            if ( RCt == 0 ) {
                RCt = XFSHashDictMake (
                                    & ( Container -> nodes ),
                                    _NodeContainerWhacker
                                    );
            }

            KLockUnlock ( Container -> mutabor );
        }
    }

    return RCt;
}   /* XFSNodeContainerClear () */

LIB_EXPORT
rc_t CC
XFSNodeContainerAddRef ( const struct XFSNodeContainer * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

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

    return RCt;
}   /* XFSNodeContainerAddRef () */

LIB_EXPORT
rc_t CC
XFSNodeContainerRelease ( const struct XFSNodeContainer * self )
{
    rc_t RCt = 0;

    XFS_CAN ( self )

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

    return RCt;
}   /* XFSNodeContainerRelease () */

LIB_EXPORT
bool CC
XFSNodeContainerHas (
                    const struct XFSNodeContainer * self,
                    const char * Name
)
{
    return XFSHashDictHas ( self -> nodes, Name );
}   /* XFSNodeContainerHas () */

static
rc_t CC
_NodeContainerNodeGet_NoLock (
                    const struct XFSNodeContainer * self,
                    const char * Name,
                    const struct XFSNode ** Node
)
{
    XFS_CAN ( self )
    XFS_CAN ( self -> nodes )

    return XFSHashDictGet ( self -> nodes, ( const void ** ) Node, Name );
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
    const struct XFSNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    RCt = KLockAcquire ( self -> mutabor );
    if ( RCt == 0 ) {

        RCt = _NodeContainerNodeGet_NoLock ( self, Name, & TheNode );
        if ( RCt == 0 ) {
            * Node = TheNode;
            if ( * Node == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }
        }

        KLockUnlock ( self -> mutabor );
    }

    return RCt;
}   /* XFSNodeContainerGet () */

LIB_EXPORT
rc_t CC
XFSNodeContainerAdd (
            const struct XFSNodeContainer * self,
            const struct XFSNode * Node
)
{
    rc_t RCt;
    struct XFSNodeContainer * Container;

    RCt = 0;
    Container = ( struct XFSNodeContainer * ) self;

    XFS_CAN ( Container )
    XFS_CAN ( Node )

    if ( XFSNodeContainerHas ( Container, Node -> Name ) ) {
        return XFS_RC ( rcExists );
    }

    RCt = KLockAcquire ( Container -> mutabor );
    if ( RCt == 0 ) {
        RCt = XFSHashDictAdd ( Container -> nodes, Node, Node -> Name );

        KLockUnlock ( Container -> mutabor );
    }

    return RCt;
}   /* XFSNodeContainerAdd () */

LIB_EXPORT
rc_t CC
XFSNodeContainerDel (
                const struct XFSNodeContainer * self,
                const char * Name
)
{
    rc_t RCt;
    struct XFSNodeContainer * Container;

    RCt = 0;
    Container = ( struct XFSNodeContainer * ) self;

    XFS_CAN ( Container )

    RCt = KLockAcquire ( Container -> mutabor );
    if ( RCt == 0 ) {
        RCt = XFSHashDictDel ( Container -> nodes, Name );

        KLockUnlock ( Container -> mutabor );
    }

    return RCt;
}   /* XFSNodeContainerDef () */

static
void CC
_NodeContainerListEacher (
                        const char * Key,
                        const void * Value,
                        const void * Data
)
{
    struct XFSNode * Node;
    struct VNamelist * List;

    Node = ( struct XFSNode * ) Value;
    List = ( struct VNamelist * ) Data;

    if ( Node != NULL && List != NULL ) {
        VNamelistAppend ( List, Node -> Name );
    }
}   /* _NodeContainerListEacher () */

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

    XFS_CSAN ( List )
    XFS_CAN ( self )
    XFS_CAN ( List )

    RCt = VNamelistMake ( & TheList, 32 );
    if ( RCt == 0 ) {
        RCt = KLockAcquire ( self -> mutabor );
        if ( RCt == 0 ) {
            RCt = XFSHashDictForEach ( 
                                    self -> nodes,
                                    _NodeContainerListEacher,
                                    TheList
                                    );

            if ( RCt == 0 ) {
                RCt = VNamelistToConstNamelist ( TheList, List );
            }

            KLockUnlock ( self -> mutabor );
        }

        VNamelistRelease ( TheList );
    }

    return RCt;
}   /* XFSNodeContainerList () */
