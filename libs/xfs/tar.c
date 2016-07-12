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

#include "xtar.h"
#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <string.h>     /* memset */

/*)))
 |||    That file contains 'archive' XFSTar based nodes
(((*/

/*)))
 |||
 +++    TarRootNode, TarNode, and others
 |||
(((*/
struct XFSTarNode {
    struct XFSNode node;

    const struct XFSTarEntry * entry;
};

struct XFSTarRootNode {
    struct XFSTarNode node;

    const struct XFSTar * tar;
};

struct XFSTarFileEditor {
    struct XFSFileEditor Papahen;

    const struct XFSTarEntry * entry;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* TarNode is living here                                       *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    TarNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _TarNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _TarNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _TarRootNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _TarNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _TarNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _TarNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _TarNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _TarNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sTarRootNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _TarNodeFlavor_v1,
                                        _TarRootNodeDispose_v1,
                                        _TarNodeFindNode_v1,
                                        _TarNodeDir_v1,
                                        _TarNodeFile_v1,
                                        _TarNodeAttr_v1,
                                        _TarNodeDescribe_v1
                                        };

static const struct XFSNode_vt_v1 _sTarNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _TarNodeFlavor_v1,
                                        _TarNodeDispose_v1,
                                        NULL,
                                        _TarNodeDir_v1,
                                        _TarNodeFile_v1,
                                        _TarNodeAttr_v1,
                                        _TarNodeDescribe_v1
                                        };


static
rc_t CC
XFSTarNodeMake (
    struct XFSNode ** Node,
    const char * Name,
    const struct XFSTarEntry * Entry
)
{
    rc_t RCt;
    struct XFSTarNode * xNode;

    RCt = 0;
    xNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    xNode = calloc ( 1, sizeof ( struct XFSTarNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSNodeInitVT (
                    & ( xNode -> node ),
                    Name,
                    ( const union XFSNode_vt * ) & _sTarNodeVT_v1
                    );

    if ( RCt == 0 ) {
        RCt = XFSTarEntryAddRef ( Entry );
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
}   /* XFSTarNodeMake () */

static
rc_t CC
XFSTarRootNodeMake (
    struct XFSNode ** Node,
    const char * Name,
    const char * Path
)
{
    rc_t RCt;
    struct XFSTarRootNode * xNode;
    const struct XFSTar * Tar;
    const struct XFSTarEntry * Entry;

    RCt = 0;
    xNode = NULL;
    Tar = NULL;
    Entry = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL || Path == NULL ) {
        return XFS_RC ( rcNull );
    }

    xNode = calloc ( 1, sizeof ( struct XFSTarRootNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSNodeInitVT (
                    & ( xNode -> node . node),
                    Name,
                    ( const union XFSNode_vt * ) & _sTarRootNodeVT_v1
                    );
    if ( RCt == 0 ) {
        RCt = XFSTarFindOrCreate ( Path, & Tar );
        if ( RCt == 0 ) {
            RCt = XFSTarGetEntry ( Tar, "/", & Entry );
            if ( RCt == 0 ) {
                if ( Entry == NULL ) {
                    RCt = XFS_RC ( rcInvalid );
                }
                else {
                    xNode -> node . entry = Entry;
                    xNode -> tar = Tar;

                    * Node = & ( xNode -> node . node );
                }
            }
        }
    }

    if ( RCt != 0 ) {
        if ( xNode != NULL ) {
            XFSNodeDispose ( ( const struct XFSNode * ) & ( xNode -> node ) );
        }
    }

    return RCt;
}   /* XFSTarRootNodeMake () */

uint32_t CC
_TarNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfTar;
}   /* _TarNodeFlavor_v1 () */

static
rc_t CC
XFSTarNodeDispose ( const struct XFSTarNode * self )
{
    struct XFSTarNode * Node = ( struct XFSTarNode * ) self;

/*
pLogMsg ( klogDebug, "XFSTarNodeDispose ( $(node) )", "node=%p", ( void * ) Node );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> entry != NULL ) {
        XFSTarEntryRelease ( Node -> entry );

        Node -> entry = NULL;
    }

    free ( Node );

    return 0;
}   /* XFSTarNodeDispose () */

static
rc_t CC
XFSTarRootNodeDispose ( const struct XFSTarRootNode * self )
{
    struct XFSTarRootNode * Node = ( struct XFSTarRootNode * ) self;

/*
pLogMsg ( klogDebug, "XFSTarRootNodeDispose ( $(node) )", "node=%p", ( void * ) Node );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> tar != NULL ) {
        XFSTarRelease ( ( struct XFSTar * ) Node -> tar );
        Node -> tar = NULL;
    }

    XFSTarNodeDispose ( & ( Node -> node ) );

    return 0;
}   /* XFSTarRootNodeDispose () */

rc_t CC
_TarRootNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSTarRootNodeDispose ( ( const struct XFSTarRootNode * ) self );
}   /* _TarRootNodeDispose_v1 () */

rc_t CC
_TarNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSTarNodeDispose ( ( const struct XFSTarNode * ) self );
}   /* _TarNodeDispose_v1 () */

/*)))
 |||
 +++  There are two methods to find node: for Root and non Root nodes
 |||
(((*/

/*)) KDir version
 ((*/
rc_t CC
_TarNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    struct XFSTarRootNode * RootNode;
    struct XFSNode * RetNode;
    const struct XFSTarEntry * Entry;
    const struct XFSPath * xPath;
    bool IsLast;

    RCt = 0;
    PathCount = 0;
    NodeName = NULL;
    RootNode = NULL;
    RetNode = NULL;
    Entry = NULL;
    xPath = NULL;
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

            return RCt;
        }

        RootNode = ( struct XFSTarRootNode * ) self;
        if ( RootNode -> tar == NULL ) {
            return XFS_RC ( rcInvalid );
        }

        RCt = XFSPathFrom ( Path, PathIndex + 1, & xPath );
        if ( RCt == 0 ) {
            RCt = XFSTarGetEntry (
                                RootNode -> tar,
                                XFSPathGet ( xPath ),
                                & Entry
                                );
            if ( RCt == 0 ) {
                RCt = XFSTarNodeMake (
                                    & RetNode,
                                    XFSPathName ( Path ),/* NodeName, */
                                    Entry
                                    );
                if ( RCt == 0 ) {
                    * Node = RetNode;
                }
            }

            XFSPathRelease ( xPath );
        }
    }

    return RCt;
}   /* _TarNodeFindNode () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_TarDir_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSDirEditor * Editor = ( struct XFSDirEditor * ) self;
/*
    pLogMsg ( klogDebug, "_TarDir_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        free ( Editor );
    }

    return 0;
}   /* _TarDir_dispose_v1 () */

static
rc_t CC
_TarDir_list_v1 (
                const struct XFSDirEditor * self,
                const struct KNamelist ** List
)
{
    rc_t RCt;
    const struct XFSTarNode * Node;
    struct KNamelist * TempList;

    RCt = 0;
    Node = NULL;
    TempList = NULL;

    if ( List != NULL ) {
        * List = NULL;
    }

    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( const struct XFSTarNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSTarEntryList ( Node -> entry, & TempList );
    if ( RCt == 0 ) {
        * List = TempList;
    }

    return RCt;
}   /* _TarDir_list_v1 () */

static
rc_t CC
_TarDir_find_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSTarNode * TarNode;
    struct XFSNode * TempNode;
    const struct XFSTarEntry * TempEntry;

    RCt = 0;
    TarNode = NULL;
    TempNode = NULL;
    TempEntry = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    TarNode = ( const struct XFSTarNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( TarNode == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( TarNode -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSTarEntryGetChild ( TarNode -> entry, Name, & TempEntry );
    if ( RCt == 0 ) {
        RCt = XFSTarNodeMake ( & TempNode, Name, TempEntry );
        if ( RCt == 0 ) {
            * Node = TempNode;
        }
    }

    return RCt;
}   /* _TarDir_find_v1 () */

rc_t CC
_TarNodeDir_v1 (
            const struct XFSNode * self,
            const struct XFSDirEditor ** Dir
)
{
    rc_t RCt;
    struct XFSDirEditor * Editor;
    const struct XFSTarNode * Node;

    RCt = 0;
    Editor = NULL;
    Node = ( const struct XFSTarNode * ) self;

    if ( Dir != NULL ) {
        * Dir = NULL;
    }

    if ( self == NULL || Dir == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ! XFSTarEntryIsFolder ( Node -> entry ) ) {
        return XFS_RC ( rcInvalid );
    }

    Editor = calloc ( 1, sizeof ( struct XFSDirEditor ) );
    if ( Editor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    memset ( Editor, 0, sizeof ( struct XFSDirEditor ) );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _TarDir_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> list = _TarDir_list_v1;
        Editor -> find = _TarDir_find_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _TarNodeDir_v1 () */

/*)))
 |||
 +++  Unified FileEditor
 |||
(((*/

static
rc_t CC
_TarFile_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSTarFileEditor * Editor = ( struct XFSTarFileEditor * ) self;
/*
    pLogMsg ( klogDebug, "_TarNodeFile_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        if ( Editor -> entry != NULL ) {
            XFSTarEntryRelease ( Editor -> entry );

            Editor -> entry = NULL;
        }

        free ( Editor );
    }

    return 0;
}   /* _TarFile_dispose_v1 () */

static
rc_t CC
_TarFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    rc_t RCt;
    const struct XFSTarNode * Node;
    struct XFSTarFileEditor * Editor;

    RCt = 0;
    Node = NULL;
    Editor = ( struct XFSTarFileEditor * ) self;

    if ( Mode != kxfsRead ) {
        return XFS_RC ( rcInvalid );
    }

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( const struct XFSTarNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Editor = ( struct XFSTarFileEditor * ) self;
    if ( Editor -> entry == NULL ) {
        if ( ! XFSTarEntryIsOpen ( Node -> entry ) ) {
            RCt = XFSTarEntryOpen ( Node -> entry );
        }
        if ( RCt == 0 ) {
            Editor -> entry = Node -> entry;
        }
    }

    return RCt;
}   /* _TarFile_open_v1 () */

static
rc_t CC
_TarFile_close_v1 ( const struct XFSFileEditor * self )
{
    rc_t RCt;
    struct XFSTarFileEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSTarFileEditor * ) self;

    if ( Editor == NULL ) {
        return XFS_RC ( rcNull );
    }


    if ( Editor -> entry != NULL ) {
        RCt = XFSTarEntryClose ( Editor -> entry );
        if ( RCt == 0 ) {
            RCt = XFSTarEntryRelease ( Editor -> entry );

            Editor -> entry = NULL;
        }
    }

    return RCt;
}   /* _TarFile_close_v1 () */

static
rc_t CC
_TarFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    rc_t RCt;
    struct XFSTarFileEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSTarFileEditor * ) self;

    if ( Editor == NULL ) {
        return XFS_RC ( rcNull );
    }


    if ( Editor -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSTarEntryRead (
                        Editor -> entry,
                        Offset,
                        Buffer,
                        SizeToRead,
                        NumReaded
                        );

    return RCt;
}   /* _TarFile_read_v1 () */

static
rc_t CC
_TarFile_size_v1 (
                        const struct XFSFileEditor * self,
                        uint64_t * Size
)
{
    rc_t RCt;
    struct XFSTarNode * Node;

    RCt = 0;
    Node = NULL;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    Node = ( struct XFSTarNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );

    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( XFSTarEntryIsFolder ( Node -> entry ) ) {
        * Size = 0;
    }
    else {
        RCt = XFSTarEntrySize ( Node -> entry, Size );
        if ( RCt != 0 ) {
            * Size = 0;
        }
    }

    return RCt;
}   /* _TarFile_size_v1 () */

rc_t CC
_TarNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct XFSTarFileEditor * FileEditor;
    struct XFSFileEditor * Editor;

    RCt = 0;
    FileEditor = NULL;
    Editor = NULL;

    if ( File != NULL ) {
        * File = NULL;
    }

    if ( self == NULL || File == NULL ) {
        return XFS_RC ( rcNull );
    }

    FileEditor = calloc ( 1, sizeof ( struct XFSTarFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    memset ( FileEditor, 0, sizeof ( struct XFSTarFileEditor ) );

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _TarFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _TarFile_open_v1;
        Editor -> close = _TarFile_close_v1;
        Editor -> read = _TarFile_read_v1;
        Editor -> size = _TarFile_size_v1;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _TarNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_TarAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_TarAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _TarAttr_dispose_v1 () */

static
rc_t CC
_TarAttr_init_check_v1 (
                const struct XFSAttrEditor * self,
                const struct XFSTarEntry ** Entry
)
{
    struct XFSTarNode * Node = NULL;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( self == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( struct XFSTarNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    * Entry = Node -> entry;

    return 0;
}   /* _TarAttr_init_check_v1 () */

static
rc_t CC
_TarAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    rc_t RCt;
    const struct XFSTarEntry * Entry;

    RCt = 0;
    Entry = NULL;

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Permissions = NULL;

    RCt = _TarAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        * Permissions = XFSTarEntryIsFolder ( Entry )
                                        ? XFSPermRODefContChar ()
                                        : XFSPermRODefNodeChar ()
                                        ;
    }

    return RCt;
}   /* _TarAttr_permissions_v1 () */


static
rc_t CC
_TarAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    rc_t RCt;
    const struct XFSTarEntry * Entry;

    RCt = 0;
    Entry = NULL;

    if ( Time == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Time = 0;

    RCt = _TarAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        RCt = XFSTarEntryTime ( Entry, Time );
        if ( RCt != 0 ) {
            * Time = 0;
        }
    }

    return RCt;
}   /* _TarAttr_date_v1 () */

static
rc_t CC
_TarAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    rc_t RCt;
    const struct XFSTarEntry * Entry;

    RCt = 0;
    Entry = NULL;

    if ( Type == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Type = kxfsNotFound;

    RCt = _TarAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        * Type = XFSTarEntryIsFolder ( Entry ) ? kxfsDir : kxfsFile ;
    }

    return RCt;
}   /* _TarAttr_type_v1 () */

static
rc_t CC
_TarNodeAttr_v1 (
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
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _TarAttr_dispose_v1
                    );
    if ( RCt == 0 ) {
        Editor -> permissions = _TarAttr_permissions_v1;
        Editor -> date = _TarAttr_date_v1;
        Editor -> type = _TarAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _TarNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_TarNodeDescribe_v1 (
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

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    Abbr = XFSTarEntryIsFolder ( ( ( const struct XFSTarNode * ) self ) -> entry )
            ? "TAR FOLDER"
            : "TAR NODE"
            ;

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
}   /* _TarNodeDescribe_v1 () */

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
 +++    TarRootNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_TarArchiveNodeConstructor (
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

    RCt = XFSTarRootNodeMake (
                    & TheNode,
                    NodeName,
                    XFSModelNodeProperty ( Template, XFS_MODEL_SOURCE )
                    );
    if ( RCt == 0 ) {
        * Node = ( struct XFSNode * ) TheNode;
    }

    if ( RCt != 0 ) {
        * Node = NULL;

        if ( TheNode != NULL ) {
            XFSTarRootNodeDispose ( ( const struct XFSTarRootNode * ) TheNode );
        }
    }

    return RCt;
}   /* _TarArchiveNodeConstructor () */

/*)))
 |||
 +++    TarRootNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_TarArchiveConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _TarArchiveNodeConstructor (
                                        Model,
                                        Template,
                                        Alias,
                                        Node
                                        );

/*
pLogMsg ( klogDebug, "_TarArchiveConstructor ( $(node), $(template) (\"$(name)\"), \"$(alias)\" )", "node=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _TarArchiveConstructor () */

static
rc_t CC
_TarArchiveValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_TarArchiveValidator ( $(node), $(template) (\"$(name)\"), \"$(alias)\" )", "node=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _TarArchiveValidator () */

static const struct XFSTeleport _sTarArchiveTeleport = {
                                        _TarArchiveConstructor,
                                        _TarArchiveValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSTarArchiveProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sTarArchiveTeleport;

    return 0;
}   /* XFSTarArchiveProvider () */

LIB_EXPORT
rc_t CC
XFSTarArchiveNodeMake (
                struct XFSNode ** Node,
                const char * Name,
                const char * Path,
                const char * Perm
)
{
    rc_t RCt;
    struct XFSNode * TheNode;

    RCt = 0;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Name == NULL || Path == NULL || Node == NULL ) {
        return XFS_RC ( rcNull ) ;
    }

    RCt = XFSTarRootNodeMake ( & TheNode, Name, Path );
    if ( RCt == 0 ) {
        * Node = TheNode;
    }
    else {
        if ( TheNode != NULL ) {
            XFSTarRootNodeDispose ( ( const struct XFSTarRootNode * ) TheNode );
        }
    }

    return RCt;
}   /* XFSTarArchiveNodeMake () */
