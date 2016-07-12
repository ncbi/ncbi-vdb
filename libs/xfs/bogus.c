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

#include "mehr.h"
#include "zehr.h"
#include "ncon.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

/*)))
 |||
 +++    BogusNode: apparently there are two types of node : NotFound
 +++    and BadPath node. Needed to comply for all other methods
 +++    It is purely not Teleport node :P
 |||
(((*/

struct XFSBogusNode {
    struct XFSNode Node;

    bool NotFoundType;
};

/*)))
 |||
 +++    BogusNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _BogusNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _BogusNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _BogusNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _BogusNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _BogusNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sBogusNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _BogusNodeFlavor_v1,
                                        _BogusNodeDispose_v1,
                                        _BogusNodeFindNode_v1,
                                        NULL,
                                        NULL,
                                        _BogusNodeAttr_v1,
                                        _BogusNodeDescribe_v1
                                        };

static rc_t CC _BogusNodeDispose ( const struct XFSBogusNode * self );

uint32_t CC
_BogusNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfBogus;
}   /* _BogusNodeFlavor_v1 () */

rc_t CC
_BogusNodeDispose_v1 ( const struct XFSNode * self )
{
    return _BogusNodeDispose ( ( const struct XFSBogusNode * ) self );
}   /* _BogusNodeDispose_v1 () */

rc_t CC
_BogusNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    return XFS_RC ( rcInvalid );
}   /* _BogusNodeFindNode () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))   Attrs
 (((*/
static
rc_t CC
_BogusNodeAttr_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSAttrEditor * Editor;

    Editor = ( struct XFSAttrEditor * ) self;

/*
pLogMsg ( klogDebug, " _BogusNodeAttr_dispose_ ( $(editor) )", "editor=0x%p", ( void * ) Editor );
*/

    if ( Editor == 0 ) {
        return 0;
    }

    if ( ( Editor -> Papahen ) . Node != NULL ) {
        XFSNodeRelease ( ( Editor -> Papahen ) . Node );

        ( Editor -> Papahen ) . Node = NULL;
    }

    free ( Editor );

    return 0;
}   /* _BogusNodeAttr_dispose_v1 () */

static
rc_t CC
_BogusNodeAttr_type_v1 (
            const struct XFSAttrEditor * self,
            XFSNType * Type
)
{
    struct XFSBogusNode * Bogus = NULL;

    if ( self == NULL || Type == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Type = kxfsNotFound;

    if ( ( self -> Papahen ) . Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Bogus = ( struct XFSBogusNode * ) ( self -> Papahen ) . Node;
    * Type = ( Bogus -> NotFoundType == true )
                                            ? kxfsNotFound
                                            : kxfsBadPath
                                            ;

    return 0;
}   /* _BogusNodeAttr_type_v1 () */

static
rc_t CC
_BogusNodeAttr_v1 (
            const struct XFSNode * self,
            const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;

    RCt = 0;
    Editor = NULL;

    if ( self == NULL || Attr == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Attr = NULL;

    Editor = calloc ( 1, sizeof ( struct XFSAttrEditor ) );
    if ( Editor == NULL ) {
        XFSNodeRelease ( self );
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                        & ( Editor -> Papahen ),
                        self,
                        _BogusNodeAttr_dispose_v1
                        );

    if ( RCt == 0 ) {
        Editor -> type = _BogusNodeAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }
/*
pLogMsg ( klogDebug, " _BogusNodeAttr_ ( $(editor) )", "editor=0x%p", ( void * ) Editor );
*/

    return RCt;
}   /* _BogusNodeAttr_v1 () */

rc_t CC
_BogusNodeDescribe_v1 (
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

    if ( self == NULL ) {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (BOGUS)[NULL][NULL]"
                    );
    }
    else {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (%s)[%s][0x%p]",
                    ( ( struct XFSBogusNode * ) self ) -> NotFoundType
                            ? "NotFound"
                            : "BadPath"
                            ,
                    self -> Name,
                    self
                    );
    }

    return RCt;
}   /* _BogusNodeDescribe_v1 () */

/*)))
 |||
 +++    BogusNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

static
rc_t CC
_BogusNodeDispose ( const struct XFSBogusNode * self )
{
    struct XFSBogusNode * Node = ( struct XFSBogusNode * ) self;

/*
pLogMsg ( klogDebug, " _BogusNodeDispose ( $(node) )", "node=0x%p", ( void * ) Node );
*/
    if ( Node == NULL ) {
        return 0;
    }

    free ( Node );

    return 0;
}   /* _BogusNodeDispose () */

static
rc_t CC
_BogusNodeMake ( 
            const char * NodeName,
            bool NotFoundType,
            const struct XFSBogusNode ** Node )
{
    rc_t RCt;
    struct XFSBogusNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    if ( NodeName == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

    TheNode = calloc ( 1, sizeof ( struct XFSBogusNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        TheNode -> NotFoundType = NotFoundType;

        RCt = XFSNodeInitVT (
                        & ( TheNode -> Node ),
                        NodeName,
                        ( const union XFSNode_vt * ) & _sBogusNodeVT_v1
                        );

        if ( RCt == 0 ) {
            * Node = TheNode;
        }
        else {
            XFSNodeDispose ( ( const struct XFSNode * ) TheNode );
        }
    }
/*
pLogMsg ( klogDebug, " _BogusNodeDispose ( $(name), $(node) )", "name=%s,node=0x%p", NodeName, ( void * ) TheNode );
*/

    return RCt;
}   /* _BogusNodeMake () */

/*))))
 ////   Make methods
((((*/

LIB_EXPORT
rc_t CC
XFSNodeNotFoundMake (
                const char * NodeName,
                const struct XFSNode ** Node
)
{
    return _BogusNodeMake (
                        NodeName,
                        true,
                        ( const struct XFSBogusNode ** ) Node
                        );
}   /* XFSNodeNotFoundMake () */

LIB_EXPORT
rc_t CC
XFSNodeBadPathMake (
                const char * NodeName,
                const struct XFSNode ** Node
)
{
    return _BogusNodeMake (
                        NodeName,
                        false,
                        ( const struct XFSBogusNode ** ) Node
                        );
}   /* XFSNodeBadPathMake () */
