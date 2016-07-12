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

#include "xencro.h"
#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <string.h>     /* memset */

/*)))
 |||    That file contains XFSEncEntry based READ ONLY node
(((*/

/*)))
 |||
 +++    EncNode, and others
 |||
(((*/
struct XFSEncNode {
    struct XFSNode node;

    const struct XFSEncEntry * entry;
};

struct XFSEncFileEditor {
    struct XFSFileEditor Papahen;

    const struct XFSEncEntry * entry;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* EncNode is living here                                       *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    EncNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _EncNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _EncNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _EncNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _EncNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _EncNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _EncNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sEncNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _EncNodeFlavor_v1,
                                        _EncNodeDispose_v1,
                                        _EncNodeFindNode_v1,
                                        NULL,   /* No DIR */
                                        _EncNodeFile_v1,
                                        _EncNodeAttr_v1,
                                        _EncNodeDescribe_v1
                                        };


static
rc_t CC
XFSEncNodeMake (
    struct XFSNode ** Node,
    const char * Name,
    const struct XFSEncEntry * Entry
)
{
    rc_t RCt;
    struct XFSEncNode * xNode;

    RCt = 0;
    xNode = NULL;

    XFS_CSAN ( Node )

    XFS_CAN ( Node )
    XFS_CAN ( Name )
    XFS_CAN ( Entry )

    xNode = calloc ( 1, sizeof ( struct XFSEncNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSNodeInitVT (
                    & ( xNode -> node ),
                    Name,
                    ( const union XFSNode_vt * ) & _sEncNodeVT_v1
                    );

    if ( RCt == 0 ) {
        RCt = XFSEncEntryAddRef ( Entry );
        if ( RCt == 0 ) {
            xNode -> entry = Entry;

            * Node = & ( xNode -> node );
        }
    }

    if ( RCt != 0 ) {
        if ( xNode != NULL ) {
            RCt = XFSNodeDispose ( & ( xNode -> node ) );
        }
    }

    return RCt;
}   /* XFSEncNodeMake () */

LIB_EXPORT
rc_t CC
XFSEncryptedFileNodeMake (
                    struct XFSNode ** Node,
                    const char * Name,
                    const char * Path,
                    const char * Passwd,
                    const char * EncType
)
{
    rc_t RCt;
    const struct XFSEncEntry * Entry;
    struct XFSNode * TheNode;

    RCt = 0;
    Entry = NULL;
    TheNode = NULL;

    XFS_CSAN ( Node )

    XFS_CAN ( Name )
    XFS_CAN ( Path )
    XFS_CAN ( Passwd )
    XFS_CAN ( Node )

    RCt = XFSEncEntryFindOrCreate ( Path, Passwd, EncType, & Entry );
    if ( RCt == 0 ) {
        RCt = XFSEncNodeMake ( & TheNode, Name, Entry );
        if ( RCt == 0 ) {
            * Node = TheNode;
        }
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSNodeDispose ( TheNode );
        }
        else {
            if ( Entry != NULL ) {
                XFSEncEntryDispose ( Entry );
            }
        }
    }

    return RCt;
}   /* XFSEncryptedFileNodeMake () */

uint32_t CC
_EncNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfEncode;
}   /* _EncNodeFlavor_v1 () */

static
rc_t CC
XFSEncNodeDispose ( const struct XFSEncNode * self )
{
    struct XFSEncNode * Node = ( struct XFSEncNode * ) self;

/*
pLogMsg ( klogDebug, "XFSEncNodeDispose ( $(node) ) EN[$(entry)]", "node=%p,entry=%d", ( void * ) Node, ( Node == NULL ? 0 : Node -> entry ) );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> entry != NULL ) {
        XFSEncEntryRelease ( Node -> entry );

        Node -> entry = NULL;
    }

    free ( Node );

    return 0;
}   /* XFSEncNodeDispose () */

rc_t CC
_EncNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSEncNodeDispose ( ( const struct XFSEncNode * ) self );
}   /* _EncNodeDispose_v1 () */

/*)))
 |||
 +++  There are no find methods for encrypted files, cuz FILE
 |||
(((*/
rc_t CC
_EncNodeFindNode_v1 (
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
}   /* _EncNodeFindNode () */

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
_EncFile_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSEncFileEditor * Editor = ( struct XFSEncFileEditor * ) self;
/*
    pLogMsg ( klogDebug, "_EncNodeFile_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        if ( Editor -> entry != NULL ) {
            XFSEncEntryRelease ( Editor -> entry );

            Editor -> entry = NULL;
        }

        free ( Editor );
    }

    return 0;
}   /* _EncFile_dispose_v1 () */

static
rc_t CC
_EncFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    rc_t RCt;
    const struct XFSEncNode * Node;
    struct XFSEncFileEditor * Editor;

    RCt = 0;
    Node = NULL;
    Editor = ( struct XFSEncFileEditor * ) self;

    if ( Mode != kxfsRead ) {
        return XFS_RC ( rcInvalid );
    }

    XFS_CAN ( self )

    Node = ( const struct XFSEncNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    XFS_CA ( Node, NULL )
    XFS_CA ( Node -> entry, NULL )

    Editor = ( struct XFSEncFileEditor * ) self;
    if ( Editor -> entry == NULL ) {
        if ( ! XFSEncEntryIsOpen ( Node -> entry ) ) {
            RCt = XFSEncEntryOpen ( Node -> entry );
        }
        if ( RCt == 0 ) {
            Editor -> entry = Node -> entry;
        }
    }

    return RCt;
}   /* _EncFile_open_v1 () */

static
rc_t CC
_EncFile_close_v1 ( const struct XFSFileEditor * self )
{
    rc_t RCt;
    struct XFSEncFileEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSEncFileEditor * ) self;

    XFS_CAN ( Editor )

    if ( Editor -> entry != NULL ) {
        RCt = XFSEncEntryClose ( Editor -> entry );
        if ( RCt == 0 ) {
            RCt = XFSEncEntryRelease ( Editor -> entry );

            Editor -> entry = NULL;
        }
    }

    return RCt;
}   /* _EncFile_close_v1 () */

static
rc_t CC
_EncFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    rc_t RCt;
    struct XFSEncFileEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSEncFileEditor * ) self;

    XFS_CAN ( Editor )
    XFS_CA ( Editor -> entry, NULL )


    RCt = XFSEncEntryRead (
                        Editor -> entry,
                        Offset,
                        Buffer,
                        SizeToRead,
                        NumReaded
                        );

    return RCt;
}   /* _EncFile_read_v1 () */

static
rc_t CC
_EncFile_size_v1 (
                        const struct XFSFileEditor * self,
                        uint64_t * Size
)
{
    rc_t RCt;
    struct XFSEncNode * Node;

    RCt = 0;
    Node = NULL;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( Size )
    XFS_CAN ( self )

    Node = ( struct XFSEncNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    XFS_CAN ( Node )
    XFS_CAN ( Node -> entry )

    RCt = XFSEncEntrySize ( Node -> entry, Size );
    if ( RCt != 0 ) {
        * Size = 0;
    }

    return RCt;
}   /* _EncFile_size_v1 () */

rc_t CC
_EncNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct XFSEncFileEditor * FileEditor;
    struct XFSFileEditor * Editor;

    RCt = 0;
    FileEditor = NULL;
    Editor = NULL;

    XFS_CSAN ( File )

    XFS_CAN ( self )
    XFS_CAN ( File )

    FileEditor = calloc ( 1, sizeof ( struct XFSEncFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    memset ( FileEditor, 0, sizeof ( struct XFSEncFileEditor ) );

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _EncFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _EncFile_open_v1;
        Editor -> close = _EncFile_close_v1;
        Editor -> read = _EncFile_read_v1;
        Editor -> size = _EncFile_size_v1;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _EncNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_EncAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_EncAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _EncAttr_dispose_v1 () */

static
rc_t CC
_EncAttr_init_check_v1 (
                const struct XFSAttrEditor * self,
                const struct XFSEncEntry ** Entry
)
{
    struct XFSEncNode * Node = NULL;

    XFS_CSAN ( Entry )

    XFS_CAN ( self )
    XFS_CAN ( Entry )


    Node = ( struct XFSEncNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    XFS_CA ( Node, NULL )
    XFS_CA ( Node -> entry, NULL )

    * Entry = Node -> entry;

    return 0;
}   /* _EncAttr_init_check_v1 () */

static
rc_t CC
_EncAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    rc_t RCt;
    const struct XFSEncEntry * Entry;

    RCt = 0;
    Entry = NULL;

    XFS_CSAN ( Permissions )
    XFS_CAN ( Permissions )

    RCt = _EncAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        * Permissions = XFSPermRODefNodeChar ();
    }

    return RCt;
}   /* _EncAttr_permissions_v1 () */


static
rc_t CC
_EncAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    rc_t RCt;
    const struct XFSEncEntry * Entry;

    RCt = 0;
    Entry = NULL;

    XFS_CSA ( Time, 0 )
    XFS_CAN ( Time )

    RCt = _EncAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        RCt = XFSEncEntryTime ( Entry, Time );
        if ( RCt != 0 ) {
            * Time = 0;
        }
    }

    return RCt;
}   /* _EncAttr_date_v1 () */

static
rc_t CC
_EncAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    rc_t RCt;
    const struct XFSEncEntry * Entry;

    RCt = 0;
    Entry = NULL;

    XFS_CSA ( Type, kxfsNotFound )
    XFS_CAN ( Type )

    RCt = _EncAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        * Type = kxfsFile ;
    }

    return RCt;
}   /* _EncAttr_type_v1 () */

static
rc_t CC
_EncNodeAttr_v1 (
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
                    _EncAttr_dispose_v1
                    );
    if ( RCt == 0 ) {
        Editor -> permissions = _EncAttr_permissions_v1;
        Editor -> date = _EncAttr_date_v1;
        Editor -> type = _EncAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _EncNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_EncNodeDescribe_v1 (
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

    Abbr = "ENC NODE";

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
}   /* _EncNodeDescribe_v1 () */

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
_EncryptedFileNodeConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    struct XFSNode * TheNode;
    const char * NodeName;

    RCt = 0;
    TheNode = NULL;
    NodeName = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Model == NULL || Template == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    NodeName = Alias == NULL ? XFSModelNodeName ( Template ) : Alias;

    RCt = XFSEncryptedFileNodeMake (
                & TheNode,
                NodeName,
                XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE ),
                XFSModelNodeProperty ( Template, XFS_MODEL_PASSWD ),
                XFSModelNodeProperty ( Template, XFS_MODEL_ENCTYPE )
                );
    if ( RCt == 0 ) {
        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSEncNodeDispose ( ( const struct XFSEncNode * ) TheNode );
        }
    }

    return RCt;
}   /* _EncryptedFileNodeConstructor () */

/*)))
 |||
 +++    EncNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_EncryptedFileConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _EncryptedFileNodeConstructor (
                                        Model,
                                        Template,
                                        Alias,
                                        Node
                                        );

/*
pLogMsg ( klogDebug, "_EncryptedFileConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _EncryptedFileConstructor () */

static
rc_t CC
_EncryptedFileValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_EncryptedFileValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _EncryptedFileValidator () */

static const struct XFSTeleport _sEncryptedFileTeleport = {
                                        _EncryptedFileConstructor,
                                        _EncryptedFileValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSEncryptedFileProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sEncryptedFileTeleport;

    return 0;
}   /* XFSEncryptedFileProvider () */

