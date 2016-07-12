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

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>

#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <string.h> /* strcmp () */

/*)))
 |||
 +++    Tree and all the worst
 |||
(((*/
static const char * _sXFSNode_classname = "XFSNode";

/*)))
 |||
 +++    Currentrly node presents three interfaces
 |||
(((*/

/*))
 ((     Node make/dispose. Note, we do initialize/destroy node name
  ))    here, but Constructor could redefine it.
 ((*/
LIB_EXPORT
rc_t CC
XFSNodeInit (
            const struct XFSNode * self,
            const char * NodeName
)
{
    return XFSNodeInitVT ( self, NodeName, NULL );
}   /* XFSNodeInit () */

static
rc_t CC
_NodeSetName ( const struct XFSNode * self, const char * Name )
{
    struct XFSNode * Node = ( struct XFSNode * ) self;

    if ( Node == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> Name != NULL ) {
        free ( Node -> Name );
        Node -> Name = NULL;
    }

    return XFS_StrDup ( Name, ( const char ** ) & ( Node -> Name ) );
}   /* _NodeSetName () */

LIB_EXPORT
rc_t CC
XFSNodeInitVT (
            const struct XFSNode * self,
            const char * NodeName,
            const union XFSNode_vt * VT
)
{
    rc_t RCt;
    struct XFSNode * Node;

    RCt = 0;
    Node = ( struct XFSNode * ) self;

    if ( Node == NULL || NodeName == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( VT != NULL ) {
        Node -> vt = VT;
    }

    RCt = _NodeSetName ( self, NodeName );
/*
pLogMsg ( klogDebug, " [XFSNodeInit] [$(node)] [$(name)]", "node=%p,name=%s", ( void * ) Node, Node -> Name );
*/
    if ( RCt == 0 ) {
        KRefcountInit (
                    & ( Node -> refcount ),
                    1,
                    _sXFSNode_classname,
                    "XFSNodeInit",
                    "XFSNode"
                    );
    }

    return RCt;
}   /* XFSNodeInitVT () */

LIB_EXPORT
rc_t CC
XFSNodeMake (
            const struct XFSModel * Model,
            const char * NodeName,
            const char * NameAlias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSModelNode * ModelNode;
    const struct XFSTeleport * Teleport;
    struct XFSNode * NewNode;
    const char * NewNodeName;

    RCt = 0;
    ModelNode = NULL;
    Teleport = NULL;
    NewNode = NULL;
    NewNodeName = NULL;

    if ( Model == NULL || Node == NULL || NodeName == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    ModelNode = XFSModelLookupNode ( Model, NodeName );
    if ( ModelNode == NULL ) {
        return XFS_RC ( rcNotFound );
    }

    if ( XFSModelNodeIsRoot ( ModelNode ) ) {
        NewNodeName = "/";
    }
    else {
            /*  I could use here '?:' operator, but i think that
             *  is more useful, cuz shows names priority:
             *     alias -> label -> name_given
             */
        NewNodeName = NameAlias;
        if ( NewNodeName == NULL ) {

            NewNodeName = XFSModelNodeLabel ( ModelNode );
            if ( NewNodeName == NULL ) {

                NewNodeName = NodeName;
            }
        }
    }

    RCt = XFSTeleportLookup ( 
                    XFSModelNodeType ( ModelNode ),
                    & Teleport
                    );
    if ( RCt == 0 ) {
        if ( Teleport == NULL ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
            if ( Teleport -> Constructor == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                RCt = Teleport -> Constructor (
                                    Model,
                                    ModelNode,
                                    NameAlias,
                                    ( const struct XFSNode ** )& NewNode
                                    );
                if ( RCt == 0 ) {
                    RCt = _NodeSetName ( NewNode, NewNodeName );
                    if ( RCt != 0 ) {
                        XFSNodeDispose ( NewNode );
                    }
                    else {
                        * Node = NewNode;
                    }
                }
            }
        }
    }

/*
pLogMsg ( klogDebug, "XFSNodeMake ( \"$(name)\" ) As [$(alias)] ( $(node) ) [$(rc)]", "name=%s,alias=%s,node=%p,rc=%d", NodeName, ( NameAlias == NULL ? "NULL" : NameAlias ), ( void * ) NewNode, RCt );
*/

    return RCt;
}   /* XFSNodeMake () */

LIB_EXPORT
rc_t CC
XFSNodeDispose ( const struct XFSNode * self )
{
    rc_t RCt;
    struct XFSNode * Node;

    RCt = 0;
    Node = ( struct XFSNode * ) self;

/*
pLogMsg ( klogDebug, "XFSNodeDispose ( $(node) )", "node=%p", ( void * ) self );
*/

    if ( Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    KRefcountWhack ( & ( Node -> refcount ), _sXFSNode_classname );

    if ( Node -> Name != NULL ) {
/*
pLogMsg ( klogDebug, " [XFSNodeDispose] [$(node)] [$(name)]", "node=%p,name=%s", ( void * ) Node, Node -> Name );
*/
        free ( Node -> Name );
        Node -> Name = NULL;
    }

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( Node -> vt -> v1.dispose != NULL ) {
                RCt = Node -> vt -> v1.dispose ( Node );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSNodeDispose () */

LIB_EXPORT
rc_t CC
XFSNodeAddRef ( const struct XFSNode * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    if ( self != NULL ) {
/*
pLogMsg ( klogDebug, "XFSNodeAddRef ( $(node) )[$(name)]", "node=%p,name=%s", ( void * ) self, self -> Name );
*/

        RefC = KRefcountAdd (
                        & ( self -> refcount ),
                        _sXFSNode_classname
                        );
        switch ( RefC ) {
            case krefOkay :     RCt = 0; break;
            
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
}   /* XFSNodeAddRef () */

LIB_EXPORT
rc_t CC
XFSNodeRelease ( const struct XFSNode * self )
{
    rc_t RCt;
    int RefC;

    RCt = 0;
    RefC = 0;

    if ( self != NULL ) {
/*
pLogMsg ( klogDebug, "XFSNodeRelease ( $(node) )[$(name)]", "node=%p,name=%s", ( void * ) self, self -> Name );
*/
        RefC = KRefcountDrop (
                            & ( self -> refcount ),
                            _sXFSNode_classname
                            );
        switch ( RefC ) {
            case krefOkay :
            case krefZero :     RCt = 0; break;

            case krefWhack :    RCt = XFSNodeDispose ( self ); break;

            case krefNegative : RCt = XFS_RC ( rcInvalid ); break;
            default :           RCt = XFS_RC ( rcUnknown ); break;
        }
    }

    return RCt;
}   /* XFSNodeRelease () */

LIB_EXPORT
rc_t CC
XFSNodeFindNode (
                const struct XFSNode * self,
                const struct XFSPath * Path,
                uint32_t PathIndex,
                const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || Path == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.findnode != NULL ) {
                RCt = self -> vt -> v1.findnode (
                                                self,
                                                Path,
                                                PathIndex,
                                                Node
                                                );
            }
            else {
/*
pLogMsg ( klogDebug, "XFSNodeAttrEditor ( $(node) ): unimplemented method 'findnode'", "node=%p", ( void * ) self );
*/
                RCt = XFS_RC ( rcUnsupported );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
            break;
    }

    return RCt;
}   /* XFSNodeFindNode () */

LIB_EXPORT
rc_t CC
XFSNodeFindNodeCheckInitStandard (
                        const struct XFSNode * self,
                        const struct XFSPath * Path,
                        uint32_t PathIndex,
                        const struct XFSNode ** Node,
                        const char ** NodeName,
                        uint32_t * PathCount,
                        bool * IsLast
)
{
    const char * XNodeName;
    uint32_t XPathCount;
    bool XIsLast;

    XNodeName = NULL;
    XPathCount = 0;
    XIsLast = false;


    if ( self == NULL || Path == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    if ( NodeName == NULL || PathCount == NULL || IsLast == NULL ) {
        return XFS_RC ( rcUnexpected );
    }

    * NodeName = NULL;
    * PathCount = 0;
    * IsLast = false;

    XPathCount = XFSPathPartCount ( Path );

    if ( XPathCount <= PathIndex ) {
        return XFS_RC ( rcInvalid );
    }

    XIsLast = PathIndex == ( XPathCount - 1 );

    XNodeName = XFSPathPartGet ( Path, PathIndex );

    /*))  Should not happen thou
     ((*/
    if ( XNodeName == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    /*))  Should not happen thou
     ((*/
    if ( strcmp ( XNodeName, XFSNodeName ( self ) ) != 0 ) {
        return XFS_RC ( rcUnexpected );
    }

    * NodeName = XNodeName;
    * PathCount = XPathCount;
    * IsLast = XIsLast;

    return 0;
}   /* XFSNodeFindNodeCheckInitStandard () */

LIB_EXPORT
rc_t CC
XFSNodeDirEditor (
                const struct XFSNode * self,
                const struct XFSDirEditor ** Dir
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || Dir == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Dir = NULL;

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.dir != NULL ) {
                RCt = self -> vt -> v1.dir ( self, Dir );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
    }

    if ( * Dir == NULL ) {
        RCt = XFS_RC ( rcUnsupported );
    }

    return RCt;
}   /* XFSNodeDirEditor () */

LIB_EXPORT
rc_t CC
XFSNodeFileEditor (
                const struct XFSNode * self,
                const struct XFSFileEditor ** File
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    * File = NULL;

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.file != NULL ) {
                RCt = self -> vt -> v1.file ( self, File );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
    }

    if ( * File == NULL ) {
        RCt = XFS_RC ( rcUnsupported );
    }

    return RCt;
}   /* XFSNodeFileEditor () */

LIB_EXPORT
rc_t CC
XFSNodeAttrEditor (
                const struct XFSNode * self,
                const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || Attr == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Attr = NULL;

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.attr != NULL ) {
                RCt = self -> vt -> v1.attr ( self, Attr );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
    }

    if ( * Attr == NULL ) {
        RCt = XFS_RC ( rcUnsupported );
    }

    return RCt;
}   /* XFSNodeAttrEditor () */

LIB_EXPORT
rc_t CC
XFSNodeDescribe (
                const struct XFSNode * self,
                char * Buffer,
                size_t BufferSize
)
{
    rc_t RCt;

    RCt = 0;

    if ( self == NULL || Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    * Buffer = 0;

    switch ( self -> vt -> v1.maj ) {
        case 1 :
            if ( self -> vt -> v1.describe != NULL ) {
                RCt = self -> vt -> v1.describe (
                                                self,
                                                Buffer,
                                                BufferSize
                                                );
            }
            break;
        default :
            RCt = XFS_RC ( rcBadVersion );
    }

    return RCt;
}   /* XFSNodeDescribe () */

LIB_EXPORT
rc_t CC
XFSNodeDump ( const struct XFSNode * self )
{
    rc_t RCt;
    char Buffer [ XFS_SIZE_1024 ];

    RCt = XFSNodeDescribe ( self, Buffer, sizeof ( Buffer ) );

    if ( RCt == 0 ) {
        pLogMsg ( klogDebug, "|||>>> $(buf)\n", "buf=%s", Buffer );
    }

    return RCt;
}   /* XFSNodeDump () */

LIB_EXPORT
const char * CC
XFSNodeName ( const struct XFSNode * self )
{
    return self == NULL ? NULL : ( self -> Name );
}   /* XFSNodeName () */

LIB_EXPORT
uint32_t CC
XFSNodeFlavor ( const struct XFSNode * self )
{
    if ( self != NULL ) {
        switch ( self -> vt -> v1.maj ) {
            case 1 :
                if ( self -> vt -> v1.flavor != NULL ) {
                    return self -> vt -> v1.flavor ( self );
                }
                break;
        }
    }

    return _sFlavorLess;
}   /* XFSNodeFlavor () */

