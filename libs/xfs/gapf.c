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

#include <kfs/file.h>
#include <kfs/directory.h>

#include <xfs/model.h>
#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/path.h>
#include <xfs/editors.h>
#include <xfs/handle.h>
#include <xfs/perm.h>

#include "xgapf.h"
#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <string.h>     /* memset */

/*)))
 |||
 +++    _GapNode, and others
 |||
(((*/
struct _GapNode {
    struct XFSNode node;

    uint32_t project_id;

    const char * accession;
    uint32_t object_id;
};

struct _GapFileEditor {
    struct XFSFileEditor Papahen;

    const struct KFile * file;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* _GapNode is living here                                       *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    _GapNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _GapNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _GapNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _GapNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _GapNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _GapNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _GapNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sGapNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _GapNodeFlavor_v1,
                                        _GapNodeDispose_v1,
                                        _GapNodeFindNode_v1,
                                        NULL,   /* No DIR */
                                        _GapNodeFile_v1,
                                        _GapNodeAttr_v1,
                                        _GapNodeDescribe_v1
                                        };


LIB_EXPORT
rc_t CC
XFSGapFileNodeMake (
    struct XFSNode ** Node,
    const char * Name,
    uint32_t ProjectId,
    const char * Accession,
    uint32_t ObjectId
)
{
    rc_t RCt;
    struct _GapNode * xNode;

    RCt = 0;
    xNode = NULL;

    XFS_CSAN ( Node )
    XFS_CAN ( Node )

    if ( Name == NULL ) {
        if ( Accession != NULL ) {
            Name = Accession;
        }
    }

    if ( Accession == NULL && ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    xNode = calloc ( 1, sizeof ( struct _GapNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSNodeInitVT (
                    & ( xNode -> node ),
                    Name,
                    ( const union XFSNode_vt * ) & _sGapNodeVT_v1
                    );
    if ( RCt == 0 ) {
        xNode -> project_id = ProjectId;

        if ( Accession != NULL ) {
            RCt = XFS_StrDup ( Accession, & ( xNode -> accession ) );
            xNode -> object_id = 0;
        }
        else {
            xNode -> accession = NULL;
            xNode -> object_id = ObjectId;
        }

        if ( RCt == 0 ) {
            * Node = & ( xNode -> node );
        }
    }

    if ( RCt != 0 ) {
        if ( xNode != NULL ) {
            RCt = XFSNodeDispose ( & ( xNode -> node ) );
        }
    }

    return RCt;
}   /* XFSGapFileNodeMake () */

uint32_t CC
_GapNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfGapFile;
}   /* _GapNodeFlavor_v1 () */

static
rc_t CC
XFSGapNodeDispose ( const struct _GapNode * self )
{
    struct _GapNode * Node = ( struct _GapNode * ) self;

/*
pLogMsg ( klogDebug, "XFSGapNodeDispose ( $(node) ) TP[$(project_id)]", "node=%p,project_id=%d", ( void * ) Node, ( Node == NULL ? 0 : Node -> project_id ) );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> accession != NULL ) {
        free ( ( char * ) Node -> accession );

        Node -> accession = NULL;
    }

    Node -> project_id = 0;
    Node -> object_id = 0;

    free ( Node );

    return 0;
}   /* XFSGapNodeDispose () */

rc_t CC
_GapNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSGapNodeDispose ( ( const struct _GapNode * ) self );
}   /* _GapNodeDispose_v1 () */

/*)))
 |||
 +++  There are no find methods for encrypted files, cuz FILE
 |||
(((*/
rc_t CC
_GapNodeFindNode_v1 (
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
}   /* _GapNodeFindNode () */

/*)))
 |||
 +++  No Dir editor 
 |||
(((*/

/*)))
 |||
 +++  Unified FileEditor
 |||
(((*/

static
rc_t CC
_GapFile_dispose_v1 ( const struct XFSEditor * self )
{
    struct _GapFileEditor * Editor = ( struct _GapFileEditor * ) self;
/*
pLogMsg ( klogDebug, "_GapFile_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        if ( Editor -> file != NULL ) {
            KFileRelease ( Editor -> file );

            Editor -> file = NULL;
        }

        free ( Editor );
    }

    return 0;
}   /* _GapFile_dispose_v1 () */

static
rc_t CC
_GapFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    rc_t RCt;
    const struct _GapNode * Node;
    struct _GapFileEditor * Editor;
    const struct KFile * File;

    RCt = 0;
    Node = NULL;
    Editor = NULL;
    File = NULL;

    if ( Mode != kxfsRead ) {
        return XFS_RC ( rcInvalid );
    }

    XFS_CAN ( self )

    Node = ( const struct _GapNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CA ( Node, NULL )
    if ( Node -> accession == NULL && Node -> object_id == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    Editor = ( struct _GapFileEditor * ) self;
    if ( Editor -> file == NULL ) {
        if ( Node -> accession != NULL ) {
            RCt = XFSGapFilesOpenAccession (
                                        & File,
                                        Node -> project_id,
                                        Node -> accession
                                        );
        }
        else {
            RCt = XFSGapFilesOpenObjectId (
                                        & File,
                                        Node -> project_id,
                                        Node -> object_id
                                        );
        }

        if ( RCt == 0 ) {
            Editor -> file = File;
        }
    }

    return RCt;
}   /* _GapFile_open_v1 () */

static
rc_t CC
_GapFile_close_v1 ( const struct XFSFileEditor * self )
{
    rc_t RCt;
    struct _GapFileEditor * Editor;

    RCt = 0;
    Editor = ( struct _GapFileEditor * ) self;

    XFS_CAN ( Editor )

    if ( Editor -> file != NULL ) {
        RCt = KFileRelease ( Editor -> file );
        Editor -> file = NULL;
    }

    return RCt;
}   /* _GapFile_close_v1 () */

static
rc_t CC
_GapFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    rc_t RCt;
    struct _GapFileEditor * Editor;

    RCt = 0;
    Editor = ( struct _GapFileEditor * ) self;

    XFS_CAN ( Editor )
    XFS_CA ( Editor -> file, NULL )


    RCt = KFileRead (
                    Editor -> file,
                    Offset,
                    Buffer,
                    SizeToRead,
                    NumReaded
                    );

    return RCt;
}   /* _GapFile_read_v1 () */

static
rc_t CC
_GapFile_size_v1 (
                const struct XFSFileEditor * self,
                uint64_t * Size
)
{
    rc_t RCt;
    struct _GapNode * Node;

    RCt = 0;
    Node = NULL;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( Size )
    XFS_CAN ( self )

    Node = ( struct _GapNode * ) XFSEditorNode ( & ( self -> Papahen ) );

    XFS_CAN ( Node )

    if ( Node -> accession == NULL && Node -> object_id == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> accession != NULL ) {
        RCt = XFSGapFilesAccessionSize (
                                        Size,
                                        Node -> project_id,
                                        Node -> accession
                                        );
    }
    else {
        RCt = XFSGapFilesObjectIdSize (
                                        Size,
                                        Node -> project_id,
                                        Node -> object_id
                                        );
    }

    if ( RCt != 0 ) {
        * Size = 0;
    }

    return RCt;
}   /* _GapFile_size_v1 () */

rc_t CC
_GapNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct _GapFileEditor * FileEditor;
    struct XFSFileEditor * Editor;

    RCt = 0;
    FileEditor = NULL;
    Editor = NULL;

    XFS_CSAN ( File )

    XFS_CAN ( self )
    XFS_CAN ( File )

    FileEditor = calloc ( 1, sizeof ( struct _GapFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _GapFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _GapFile_open_v1;
        Editor -> close = _GapFile_close_v1;
        Editor -> read = _GapFile_read_v1;
        Editor -> size = _GapFile_size_v1;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _GapNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_GapAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
pLogMsg ( klogDebug, "_GapAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSEditor * ) self );
    }

    return 0;
}   /* _GapAttr_dispose_v1 () */

static
rc_t CC
_GapAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    const struct _GapNode * Node;

    Node = ( struct _GapNode * ) XFSEditorNode ( & ( self -> Papahen ) );

    XFS_CSAN ( Permissions )
    XFS_CAN ( Permissions )
    XFS_CAN ( Node )

    if ( Node -> accession == NULL && Node -> object_id == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    * Permissions = XFSPermRODefNodeChar ();

    return 0;
}   /* _GapAttr_permissions_v1 () */


static
rc_t CC
_GapAttr_date_v1 ( const struct XFSAttrEditor * self, KTime_t * Time )
{
    rc_t RCt;
    const struct _GapNode * Node;

    RCt = 0;
    Node = ( struct _GapNode * ) XFSEditorNode ( & ( self -> Papahen ) );

    XFS_CSA ( Time, 0 )
    XFS_CAN ( Time )
    XFS_CAN ( Node )

    if ( Node -> accession == NULL && Node -> object_id == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> accession != NULL ) {
        RCt = XFSGapFilesAccessionDate (
                                        Time, 
                                        Node -> project_id,
                                        Node -> accession
                                        );
    }
    else {
        RCt = XFSGapFilesObjectIdDate (
                                        Time, 
                                        Node -> project_id,
                                        Node -> object_id
                                        );
    }

    if ( RCt != 0 ) {
        * Time = 0;
    }

    return RCt;
}   /* _GapAttr_date_v1 () */

static
rc_t CC
_GapAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    const struct _GapNode * Node;

    Node = ( struct _GapNode * ) XFSEditorNode ( & ( self -> Papahen ) );

    XFS_CSA ( Type, kxfsNotFound )
    XFS_CAN ( Type )
    XFS_CAN ( Node )

    if ( Node -> accession == NULL && Node -> object_id == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    * Type = kxfsFile ;

    return 0;
}   /* _GapAttr_type_v1 () */

static
rc_t CC
_GapNodeAttr_v1 (
            const struct XFSNode * self,
            const struct XFSAttrEditor ** Attr
)
{
    rc_t RCt;
    struct XFSAttrEditor * Editor;

    RCt = 0;
    Editor = NULL;

    XFS_CSAN ( Attr )

    XFS_CAN ( self )
    XFS_CAN ( Attr )

    Editor = calloc ( 1, sizeof ( struct XFSAttrEditor ) );
    if ( Editor == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _GapAttr_dispose_v1
                    );
    if ( RCt == 0 ) {
        Editor -> permissions = _GapAttr_permissions_v1;
        Editor -> date = _GapAttr_date_v1;
        Editor -> type = _GapAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _GapNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_GapNodeDescribe_v1 (
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
    Abbr = NULL;

    XFS_CAN ( Buffer )
    XFS_CA ( BufferSize, 0 )

    Abbr = "GAP FILE";

    if ( self == NULL ) {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (%s)[NULL][NULL]",
                    Abbr
                    );
    }
    else {
        string_printf (
                    Buffer,
                    BufferSize,
                    & NumWrit,
                    "NODE (%s)[%s][0x%p]",
                    Abbr,
                    self -> Name,
                    self
                    );
    }

    return RCt;
}   /* _GapNodeDescribe_v1 () */

/*)))
 |||
 +++    FileNode lives here
 |||
(((*/

/*))
 ((     Node make/dispose
  ))
 ((*/

/*)))
 |||
 +++    Non-Teleport methods to create nodes
 |||
(((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* ALL BELOW TODO!!!                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))
 |||
 +++    EncNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_GapFileNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * NodeName;
    uint32_t ProjectId;
    uint32_t ObjectId;
    const char * Accession;
    const char * Var;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;
    ProjectId = 0;
    ObjectId = 0;
    Accession = NULL;
    Var = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    Var = XFSModelNodeProperty ( Template, XFS_MODEL_PROJECTID );
    if ( Var == NULL ) {
        return XFS_RC ( rcNull );
    }
    ProjectId = atol ( Var );

    Accession = XFSModelNodeProperty ( Template, XFS_MODEL_ACCESSION );

    Var = XFSModelNodeProperty ( Template, XFS_MODEL_OBJECTID );
    if ( Var != NULL ) {
        ObjectId = atol ( Var );
    }

    if ( Accession == NULL && ObjectId == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSGapFileNodeMake (
                            & TheNode,
                            NodeName,
                            ProjectId,
                            Accession,
                            ObjectId
                            );
    if ( RCt == 0 ) {
        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSGapNodeDispose ( ( const struct _GapNode * ) TheNode );
        }
    }

    return RCt;
}   /* _GapFileNodeConstructor () */

/*)))
 |||
 +++    EncNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_GapFileConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _GapFileNodeConstructor (
                                        Model,
                                        Template,
                                        Alias,
                                        Node
                                        );

/*
pLogMsg ( klogDebug, "_GapFileConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapFileConstructor () */

static
rc_t CC
_GapFileValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_GapFileValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _GapFileValidator () */

static const struct XFSTeleport _sGapFileTeleport = {
                                        _GapFileConstructor,
                                        _GapFileValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSGapFileProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sGapFileTeleport;

    return 0;
}   /* XFSGapFileProvider () */
