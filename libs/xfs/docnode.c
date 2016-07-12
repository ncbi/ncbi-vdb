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
#include <klib/printf.h>
#include <klib/log.h>

#include <kfs/file.h>

#include <xfs/node.h>
#include <xfs/editors.h>
#include <xfs/perm.h>
#include <xfs/doc.h>

#include "schwarzschraube.h"
#include "common.h"

#include <sysalloc.h>

/*)))
 |||    That file contains DocFile based node as XFSDocNode
(((*/

/*)))
 |||
 +++    FileNode, and other simple containers
 |||
(((*/
struct XFSDocNode {
    struct XFSNode node;

    const struct XFSDoc * doc;
    const char * perm;
    uint32_t flavor;
};

struct XFSDocFileEditor {
    struct XFSFileEditor Papahen;

    struct KFile * file;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* DocNode is living here                                        *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    DocNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _DocNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _DocNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _DocNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _DocNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _DocNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _DocNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sDocNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _DocNodeFlavor_v1,
                                        _DocNodeDispose_v1,
                                        _DocNodeFindNode_v1,
                                        NULL,   /* NO DIR */
                                        _DocNodeFile_v1,
                                        _DocNodeAttr_v1,
                                        _DocNodeDescribe_v1
                                        };

LIB_EXPORT
rc_t CC
XFSDocNodeMake (
            struct XFSNode ** Node,
            const struct XFSDoc * Doc,
            const char * Name,
            const char * Perm          /* Could be NULL */
)
{
    return XFSDocNodeMakeWithFlavor (
                                    Node,
                                    Doc,
                                    Name,
                                    Perm,
                                    _sFlavorLess
                                    );
}   /* XFSDocNodeMake () */

LIB_EXPORT
rc_t CC
XFSDocNodeMakeWithFlavor (
            struct XFSNode ** Node,
            const struct XFSDoc * Doc,
            const char * Name,
            const char * Perm,          /* Could be NULL */
            uint32_t Flavor
)
{
    rc_t RCt;
    struct XFSDocNode * TheNode;

    RCt = 0;
    TheNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL || Doc == NULL ) {
        return XFS_RC ( rcNull );
    }

    TheNode = calloc ( 1, sizeof ( struct XFSDocNode ) );
    if ( TheNode == NULL ) {
        RCt = XFS_RC ( rcExhausted );
    }
    else {
        RCt = XFSNodeInitVT (
                        & ( TheNode -> node),
                        Name,
                        ( const union XFSNode_vt * ) & _sDocNodeVT_v1
                        );
        if ( RCt == 0 ) {
            if ( Perm != NULL ) {
                RCt = XFS_StrDup ( Perm, & ( TheNode -> perm ) );
            }

            if ( RCt == 0 ) {
                RCt = XFSDocAddRef ( Doc );
                if ( RCt == 0 ) {
                    TheNode -> doc = Doc;

                    TheNode -> flavor = Flavor;

                    * Node = & ( TheNode -> node );
                }
            }
        }
    }

    if ( RCt != 0 ) {
        if ( TheNode != NULL ) {
            XFSNodeDispose ( & ( TheNode -> node ) );
            TheNode = NULL;
        }
    }

/*
pLogMsg ( klogDebug, "XFSDocNodeMake ND[$(node)] NM[$(name)] Doc[$(doc)]", "node=%p,name=%s,doc=%s", ( void * ) TheNode, Name, Doc );
*/

    return RCt;
}   /* XFSDocNodeMakeWithFlavor () */

uint32_t CC
_DocNodeFlavor_v1 ( const struct XFSNode * self )
{
    const struct XFSDocNode * Node = ( const struct XFSDocNode * ) self;

    if ( Node != NULL ) {
        return Node -> flavor;
    }

    return _sFlavorLess;
}   /* _DocNodeFlavor_v1 () */

static
rc_t CC
XFSDocNodeDispose ( const struct XFSDocNode * self )
{
    struct XFSDocNode * Node = ( struct XFSDocNode * ) self;

/*
pLogMsg ( klogDebug, "XFSDocNodeDispose ( $(node) ) [$(doc)]", "node=%p,doc=%p", ( void * ) Node, ( void * ) Node -> doc );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> perm != NULL ) {
        free ( ( char * ) Node -> perm );
        Node -> perm = NULL;
    }

    if ( Node -> doc != NULL ) {
        XFSDocRelease ( Node -> doc );
        Node -> doc = NULL;
    }

    free ( Node );

    return 0;
}   /* XFSDocNodeDispose () */

rc_t CC
_DocNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSDocNodeDispose ( ( struct XFSDocNode * ) self );
}   /* _DocNodeDispose_v1 () */

/*)))
 |||
 +++  Node lookup ... never do it seriously
 |||
(((*/

rc_t CC
_DocNodeFindNode_v1 (
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
}   /* _DocNodeFindNode () */

/*)))
 |||
 +++  Unified FileEditor
 |||
(((*/

static
rc_t CC
_DocFile_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_DocNodeFile_dispose_v1 ( $(node) )", "node=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSDocFileEditor * ) self );
    }

    return 0;
}   /* _DocFile_dispose_v1 () */

static
rc_t CC
_DocFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    const struct XFSDocNode * Node;
    struct KFile * File;
    rc_t RCt;

    Node = NULL;
    File = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( const struct XFSDocNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Mode != kxfsRead ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSDocFileMake (
                        XFSNodeName ( & ( Node -> node ) ),
                        Node -> doc,
                        & File
                        );
    if ( RCt == 0 ) {
        ( ( struct XFSDocFileEditor * ) self ) -> file = File;
    }

    return RCt;
}   /* _DocFile_open_v1 () */

static
rc_t CC
_DocFile_close_v1 ( const struct XFSFileEditor * self )
{
    struct XFSDocFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }


    Editor = ( struct XFSDocFileEditor * ) self;

    if ( Editor -> file != NULL ) {
        RCt = KFileRelease ( Editor -> file );

        Editor -> file = NULL;
    }

    return RCt;
}   /* _DocFile_close_v1 () */

static
rc_t CC
_DocFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    struct XFSDocFileEditor * Editor;
    rc_t RCt;

    Editor = NULL;
    RCt = 0;

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Editor = ( struct XFSDocFileEditor * ) self;

    if ( Editor -> file == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KFileRead (
                    Editor -> file,
                    Offset,
                    Buffer,
                    SizeToRead,
                    NumReaded
                    );

/* here may be debutt */

    return RCt;
}   /* _DocFile_read_v1 () */

static
rc_t CC
_DocFile_size_v1 (
                        const struct XFSFileEditor * self,
                        uint64_t * Size
)
{
    const struct XFSDocNode * Node;
    uint64_t TempSize;
    rc_t RCt;

    Node = NULL;
    TempSize = 0;
    RCt = 0;

    if ( Size == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    Node = ( const struct XFSDocNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> doc == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSDocSize ( Node -> doc, & TempSize );
    if ( RCt == 0 ) {
        * Size = TempSize;
    }

    return RCt;
}   /* _DocFile_size_v1 () */

rc_t CC
_DocNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct XFSDocFileEditor * FileEditor;
    struct XFSFileEditor * Editor;

    RCt = 0;
    FileEditor = NULL;
    Editor = NULL;

    if ( self == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    * File = NULL;

    FileEditor = calloc ( 1, sizeof ( struct XFSDocFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _DocFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _DocFile_open_v1;
        Editor -> close = _DocFile_close_v1;
        Editor -> read = _DocFile_read_v1;
        Editor -> write = NULL;
        Editor -> size = _DocFile_size_v1;
        Editor -> set_size = NULL;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _DocNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_DocAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_DocAttr_dispose_v1 ( node )", "node=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _DocAttr_dispose_v1 () */

/*))    Something unusual. Will check-initialize NativeDir and Node
 //     NativeDir and Node could be NULL
((*/
static
rc_t CC
_DocAttr_init_check_v1 (
                    const struct XFSAttrEditor * self,
                    const struct XFSDocNode ** Node

)
{
    rc_t RCt;
    const struct XFSDocNode * RetNode;

    RCt = 0;
    RetNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    RetNode = ( const struct XFSDocNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( RetNode == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( RetNode -> doc == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( RCt == 0 ) {
        if ( Node != NULL ) {
            * Node = RetNode;
        }
    }

    return RCt;
}   /* _DocAttr_init_check_v1 () */

static
rc_t CC
_DocAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    const struct XFSDocNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Permissions = NULL;

    RCt = _DocAttr_init_check_v1 ( self, & Node );
    if ( RCt == 0 ) {
        if ( Node -> perm != NULL ) {
            * Permissions = Node -> perm;
        }
        else {
            * Permissions = XFSPermRODefNodeChar ();
        }
    }

    return RCt;
}   /* _DocAttr_permissions_v1 () */

static
rc_t CC
_DocAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    const struct XFSDocNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    if ( Time == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Time = 0;

    RCt = _DocAttr_init_check_v1 ( self, & Node );
    if ( RCt == 0 ) {
            /* TODO IMPORTANT - make time !!! */
        * Time = 0;
    }

    return RCt;
}   /* _DocAttr_date_v1 () */

static
rc_t CC
_DocAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    if ( Type == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Type = kxfsFile;

    return 0;
}   /* _DocAttr_type_v1 () */

static
rc_t CC
_DocNodeAttr_v1 (
            const struct XFSNode * self,
            const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;

    RCt = 0;
    Editor = NULL;

    if ( Attr != NULL ) {
        * Attr = NULL;
    }

    if ( self == NULL || Attr == NULL ) {
        return XFS_RC ( rcNull );
    }

    Editor = calloc ( 1, sizeof ( struct XFSAttrEditor ) );
    if ( Editor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _DocAttr_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> permissions = _DocAttr_permissions_v1;
        Editor -> set_permissions = NULL;
        Editor -> date = _DocAttr_date_v1;
        Editor -> set_date = NULL;
        Editor -> type = _DocAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _DocNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_DocNodeDescribe_v1 (
            const struct XFSNode * self,
            char * Buffer,
            size_t BufferSize
)
{
    rc_t RCt;
    size_t NumWrit;
    const char * Abbr;

    RCt = 0;
    NumWrit = 0;
    Abbr = "DOC FILE";

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    if ( self == NULL ) {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "DOCNODE (%s)[NULL][NULL]",
                    Abbr
                    );
    }
    else {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "DOCNODE (%s)[%s][0x%p]",
                    Abbr,
                    self -> Name,
                    self
                    );
    }

    return RCt;
}   /* _DocNodeDescribe_v1 () */

