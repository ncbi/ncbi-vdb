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

#include "xhttp.h"
#include "mehr.h"
#include "schwarzschraube.h"
#include "teleport.h"
#include "common.h"

#include <sysalloc.h>

#include <string.h>     /* memset */

/*)))
 |||    That file contains 'remote' XFSHttp based nodes
(((*/

/*)))
 |||
 +++    RemoteRepositoryNode, HttpNode, and others
 |||
(((*/
struct XFSHttpNode {
    struct XFSNode node;

    const struct XFSHttpEntry * entry;
};

struct XFSHttpRootNode {
    struct XFSHttpNode node;

    const struct XFSHttp * http;
};

struct XFSHttpFileEditor {
    struct XFSFileEditor Papahen;

    const struct XFSHttpReader * reader;
};

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*                                                               *_*/
/*_* HttpNode is living here                                       *_*/
/*_*                                                               *_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)))
 |||
 +++    HttpNode virtual table is Living here :lol:
 |||
(((*/
static rc_t CC _HttpNodeFlavor_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _HttpNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _HttpRootNodeDispose_v1 (
                                const struct XFSNode * self
                                );
static rc_t CC _HttpNodeFindNode_v1 (
                                const struct XFSNode * self,
                                const struct XFSPath * Path,
                                uint32_t PathIndex,
                                const struct XFSNode ** Node
                                );
static rc_t CC _HttpNodeDir_v1 (
                                const struct XFSNode * self,
                                const struct XFSDirEditor ** Dir
                                );
static rc_t CC _HttpNodeFile_v1 (
                                const struct XFSNode * self,
                                const struct XFSFileEditor ** File
                                );
static rc_t CC _HttpNodeAttr_v1 (
                                const struct XFSNode * self,
                                const struct XFSAttrEditor ** Attr
                                );
static rc_t CC _HttpNodeDescribe_v1 (
                                const struct XFSNode * self,
                                char * Buffer,
                                size_t BufferSize
                                );

static const struct XFSNode_vt_v1 _sHttpRootNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _HttpNodeFlavor_v1,
                                        _HttpRootNodeDispose_v1,
                                        _HttpNodeFindNode_v1,
                                        _HttpNodeDir_v1,
                                        _HttpNodeFile_v1,
                                        _HttpNodeAttr_v1,
                                        _HttpNodeDescribe_v1
                                        };

static const struct XFSNode_vt_v1 _sHttpNodeVT_v1 = {
                                        1, 1,   /* nin naj */
                                        _HttpNodeFlavor_v1,
                                        _HttpNodeDispose_v1,
                                        NULL,
                                        _HttpNodeDir_v1,
                                        _HttpNodeFile_v1,
                                        _HttpNodeAttr_v1,
                                        _HttpNodeDescribe_v1
                                        };


static
rc_t CC
XFSHttpNodeMake (
    struct XFSNode ** Node,
    const char * Name,
    const struct XFSHttpEntry * Entry
)
{
    rc_t RCt;
    struct XFSHttpNode * xNode;

    RCt = 0;
    xNode = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    xNode = calloc ( 1, sizeof ( struct XFSHttpNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSNodeInitVT (
                    & ( xNode -> node ),
                    Name,
                    ( const union XFSNode_vt * ) & _sHttpNodeVT_v1
                    );

    if ( RCt == 0 ) {
        RCt = XFSHttpEntryAddRef ( Entry );
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
}   /* XFSHttpNodeMake () */

static
rc_t CC
XFSHttpRootNodeMake (
    struct XFSNode ** Node,
    const char * Name,
    const char * BaseUrl
)
{
    rc_t RCt;
    struct XFSHttpRootNode * xNode;
    const struct XFSHttp * Http;
    const struct XFSHttpEntry * Entry;

    RCt = 0;
    xNode = NULL;
    Http = NULL;
    Entry = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( Node == NULL || Name == NULL || BaseUrl == NULL ) {
        return XFS_RC ( rcNull );
    }

    xNode = calloc ( 1, sizeof ( struct XFSHttpRootNode ) );
    if ( xNode == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    RCt = XFSNodeInitVT (
                    & ( xNode -> node . node),
                    Name,
                    ( const union XFSNode_vt * ) & _sHttpRootNodeVT_v1
                    );
    if ( RCt == 0 ) {
        RCt = XFSHttpMake ( BaseUrl, & Http );
        if ( RCt == 0 ) {
            Entry = XFSHttpGetEntry ( Http, "/" );
            if ( Entry == NULL ) {
                RCt = XFS_RC ( rcInvalid );
            }
            else {
                xNode -> node . entry = Entry;
                xNode -> http = Http;

                * Node = & ( xNode -> node . node );
            }
        }
    }

    if ( RCt != 0 ) {
        if ( xNode != NULL ) {
            XFSNodeDispose ( ( const struct XFSNode * ) & ( xNode -> node ) );
        }
    }

    return RCt;
}   /* XFSHttpRootNodeMake () */

uint32_t CC
_HttpNodeFlavor_v1 ( const struct XFSNode * self )
{
    return _sFlavorOfHttp;
}   /* _HttpNodeFlavor_v1 () */

static
rc_t CC
XFSHttpNodeDispose ( const struct XFSHttpNode * self )
{
    struct XFSHttpNode * Node = ( struct XFSHttpNode * ) self;

/*
pLogMsg ( klogDebug, "XFSHttpNodeDispose ( $(node) )]", "node=%p", ( void * ) Node );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> entry != NULL ) {
        XFSHttpEntryRelease ( Node -> entry );
        Node -> entry = NULL;
    }

    free ( Node );

    return 0;
}   /* XFSHttpNodeDispose () */

static
rc_t CC
XFSHttpRootNodeDispose ( const struct XFSHttpRootNode * self )
{
    struct XFSHttpRootNode * Node = ( struct XFSHttpRootNode * ) self;

/*
pLogMsg ( klogDebug, "XFSHttpRootNodeDispose ( $(node) )]", "node=%p", ( void * ) Node );
*/

    if ( Node == 0 ) {
        return 0;
    }

    if ( Node -> http != NULL ) {
        XFSHttpRelease ( ( struct XFSHttp * ) Node -> http );
        Node -> http = NULL;
    }

    XFSHttpNodeDispose ( & ( Node -> node ) );

    return 0;
}   /* XFSHttpRootNodeDispose () */

rc_t CC
_HttpRootNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSHttpRootNodeDispose ( ( const struct XFSHttpRootNode * ) self );
}   /* _HttpRootNodeDispose_v1 () */

rc_t CC
_HttpNodeDispose_v1 ( const struct XFSNode * self )
{
    return XFSHttpNodeDispose ( ( const struct XFSHttpNode * ) self );
}   /* _HttpNodeDispose_v1 () */

/*)))
 |||
 +++  There are two methods to find node: for Root and non Root nodes
 |||
(((*/

/*)) KDir version
 ((*/
rc_t CC
_HttpNodeFindNode_v1 (
            const struct XFSNode * self,
            const struct XFSPath * Path,
            uint32_t PathIndex,
            const struct XFSNode ** Node
)
{
    rc_t RCt;
    uint32_t PathCount;
    const char * NodeName;
    struct XFSHttpRootNode * RootNode;
    struct XFSNode * RetNode;
    const struct XFSHttpEntry * Entry;
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

        RootNode = ( struct XFSHttpRootNode * ) self;
        if ( RootNode -> http == NULL ) {
            return XFS_RC ( rcInvalid );
        }

        RCt = XFSPathFrom ( Path, PathIndex + 1, & xPath );
        if ( RCt == 0 ) {
            RCt = XFSHttpGetOrCreateEntry (
                                        RootNode -> http,
                                        XFSPathGet ( xPath ),
                                        & Entry
                                        );
            if ( RCt == 0 ) {
                RCt = XFSHttpNodeMake (
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
}   /* _HttpNodeFindNode () */

/*)))
 |||
 +++  Unified DirEditor
 |||
(((*/
static
rc_t CC
_HttpDir_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSDirEditor * Editor = ( struct XFSDirEditor * ) self;
/*
    pLogMsg ( klogDebug, "_HttpDir_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        free ( Editor );
    }

    return 0;
}   /* _HttpDir_dispose_v1 () */

static
rc_t CC
_HttpDir_list_v1 (
                const struct XFSDirEditor * self,
                const struct KNamelist ** List
)
{
    rc_t RCt;
    const struct XFSHttpNode * Node;
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

    Node = ( const struct XFSHttpNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSHttpEntryList ( Node -> entry, & TempList );
    if ( RCt == 0 ) {
        * List = TempList;
    }

    return RCt;
}   /* _HttpDir_list_v1 () */

static
rc_t CC
_HttpDir_find_v1 (
                const struct XFSDirEditor * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    rc_t RCt;
    const struct XFSHttpNode * HttpNode;
    struct XFSNode * TempNode;
    const struct XFSHttpEntry * TempEntry;

    RCt = 0;
    HttpNode = NULL;
    TempNode = NULL;
    TempEntry = NULL;

    if ( Node != NULL ) {
        * Node = NULL;
    }

    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    HttpNode = ( const struct XFSHttpNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( HttpNode == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( HttpNode -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSHttpEntryGet ( HttpNode -> entry, Name, & TempEntry );
    if ( RCt == 0 ) {
        RCt = XFSHttpNodeMake ( & TempNode, Name, TempEntry );
        if ( RCt == 0 ) {
            * Node = TempNode;
        }
    }

    return RCt;
}   /* _HttpDir_find_v1 () */

rc_t CC
_HttpNodeDir_v1 (
            const struct XFSNode * self,
            const struct XFSDirEditor ** Dir
)
{
    rc_t RCt;
    struct XFSDirEditor * Editor;
    const struct XFSHttpNode * Node;

    RCt = 0;
    Editor = NULL;
    Node = ( const struct XFSHttpNode * ) self;

    if ( Dir != NULL ) {
        * Dir = NULL;
    }

    if ( self == NULL || Dir == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ! XFSHttpEntryIsFolder ( Node -> entry ) ) {
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
                    _HttpDir_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> list = _HttpDir_list_v1;
        Editor -> find = _HttpDir_find_v1;

        * Dir = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _HttpNodeDir_v1 () */

/*)))
 |||
 +++  Unified FileEditor
 |||
(((*/

static
rc_t CC
_HttpFile_dispose_v1 ( const struct XFSEditor * self )
{
    struct XFSHttpFileEditor * Editor = ( struct XFSHttpFileEditor * ) self;
/*
    pLogMsg ( klogDebug, "_HttpFile_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( Editor != NULL ) {
        if ( Editor -> reader != NULL ) {
            XFSHttpReaderRelease ( Editor -> reader );

            Editor -> reader = NULL;
        }

        free ( Editor );
    }

    return 0;
}   /* _HttpFile_dispose_v1 () */

static
rc_t CC
_HttpFile_open_v1 (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
)
{
    rc_t RCt;
    const struct XFSHttpNode * Node;
    struct XFSHttpFileEditor * Editor;
    const struct XFSHttpReader * Reader;

    RCt = 0;
    Node = NULL;
    Editor = ( struct XFSHttpFileEditor * ) self;
    Reader = NULL;

    if ( Mode != kxfsRead ) {
        return XFS_RC ( rcInvalid );
    }

    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( const struct XFSHttpNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Editor = ( struct XFSHttpFileEditor * ) self;
    if ( Editor -> reader == NULL ) {
        RCt = XFSHttpReaderMake ( Node -> entry, & Reader );
        if ( RCt == 0 ) {
            Editor -> reader = Reader;
        }
    }

    return RCt;
}   /* _HttpFile_open_v1 () */

static
rc_t CC
_HttpFile_close_v1 ( const struct XFSFileEditor * self )
{
    rc_t RCt;
    struct XFSHttpFileEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSHttpFileEditor * ) self;

    if ( Editor == NULL ) {
        return XFS_RC ( rcNull );
    }


    if ( Editor -> reader != NULL ) {
        RCt = XFSHttpReaderRelease ( Editor -> reader );

        Editor -> reader = NULL;
    }

    return RCt;
}   /* _HttpFile_close_v1 () */

static
rc_t CC
_HttpFile_read_v1 (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
)
{
    rc_t RCt;
    struct XFSHttpFileEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSHttpFileEditor * ) self;

    if ( Editor == NULL ) {
        return XFS_RC ( rcNull );
    }


    if ( Editor -> reader == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = XFSHttpReaderRead (
                        Editor -> reader,
                        Offset,
                        Buffer,
                        SizeToRead,
                        NumReaded
                        );

    return RCt;
}   /* _HttpFile_read_v1 () */

static
rc_t CC
_HttpFile_size_v1 (
                        const struct XFSFileEditor * self,
                        uint64_t * Size
)
{
    rc_t RCt;
    struct XFSHttpNode * Node;

    RCt = 0;
    Node = NULL;

    XFS_CSA ( Size, 0 )
    XFS_CAN ( self )
    XFS_CAN ( Size )

    Node = ( struct XFSHttpNode * ) XFSEditorNode (
                                                & ( self -> Papahen )
                                                );
    if ( Node == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Node -> entry == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( XFSHttpEntryIsFolder ( Node -> entry ) ) {
        * Size = 0;
    }
    else {
        RCt = XFSHttpEntrySize ( Node -> entry, Size );
        if ( RCt != 0 ) {
            * Size = 0;
        }
    }

    return RCt;
}   /* _HttpFile_size_v1 () */

rc_t CC
_HttpNodeFile_v1 (
            const struct XFSNode * self,
            const struct XFSFileEditor ** File
)
{
    rc_t RCt;
    struct XFSHttpFileEditor * FileEditor;
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

    FileEditor = calloc ( 1, sizeof ( struct XFSHttpFileEditor ) );
    if ( FileEditor == NULL ) { 
        return XFS_RC ( rcExhausted );
    }

    memset ( FileEditor, 0, sizeof ( struct XFSHttpFileEditor ) );

    Editor = & ( FileEditor -> Papahen );

    RCt = XFSEditorInit (
                    & ( Editor -> Papahen ),
                    self,
                    _HttpFile_dispose_v1
                    );

    if ( RCt == 0 ) {
        Editor -> open = _HttpFile_open_v1;
        Editor -> close = _HttpFile_close_v1;
        Editor -> read = _HttpFile_read_v1;
        Editor -> size = _HttpFile_size_v1;
        Editor -> set_size = NULL;

        * File = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _HttpNodeFile_v1 () */

/*)))
 |||
 +++  Unified Attr
 |||
(((*/

static
rc_t CC
_HttpAttr_dispose_v1 ( const struct XFSEditor * self )
{
/*
    pLogMsg ( klogDebug, "_HttpAttr_dispose_v1 ( $(editor) )", "editor=%p", ( void * ) self );
*/

    if ( self != NULL ) {
        free ( ( struct XFSAttrEditor * ) self );
    }

    return 0;
}   /* _HttpAttr_dispose_v1 () */

static
rc_t CC
_HttpAttr_init_check_v1 (
                const struct XFSAttrEditor * self,
                const struct XFSHttpEntry ** Entry
)
{
    struct XFSHttpNode * Node = NULL;

    if ( Entry != NULL ) {
        * Entry = NULL;
    }

    if ( self == NULL || Entry == NULL ) {
        return XFS_RC ( rcNull );
    }

    Node = ( struct XFSHttpNode * ) XFSEditorNode (
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
}   /* _HttpAttr_init_check_v1 () */

static
rc_t CC
_HttpAttr_permissions_v1 (
                        const struct XFSAttrEditor * self,
                        const char ** Permissions
)
{
    rc_t RCt;
    const struct XFSHttpEntry * Entry;

    RCt = 0;
    Entry = NULL;

    if ( Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Permissions = NULL;

    RCt = _HttpAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        * Permissions = XFSHttpEntryIsFolder ( Entry )
                                        ? XFSPermRODefContChar ()
                                        : XFSPermRODefNodeChar ()
                                        ;
    }

    return RCt;
}   /* _HttpAttr_permissions_v1 () */

static
rc_t CC
_HttpAttr_date_v1 (
                        const struct XFSAttrEditor * self,
                        KTime_t * Time
)
{
    rc_t RCt;
    const struct XFSHttpEntry * Entry;

    RCt = 0;
    Entry = NULL;

    if ( Time == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Time = 0;

    RCt = _HttpAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        RCt = XFSHttpEntryTime ( Entry, Time );
        if ( RCt != 0 ) {
            * Time = 0;
        }
    }

    return RCt;
}   /* _HttpAttr_date_v1 () */

static
rc_t CC
_HttpAttr_type_v1 (
                        const struct XFSAttrEditor * self,
                        XFSNType * Type
)
{
    rc_t RCt;
    const struct XFSHttpEntry * Entry;

    RCt = 0;
    Entry = NULL;

    if ( Type == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Type = kxfsNotFound;

    RCt = _HttpAttr_init_check_v1 ( self, & Entry );
    if ( RCt == 0 ) {
        * Type = XFSHttpEntryIsFolder ( Entry ) ? kxfsDir : kxfsFile ;
    }

    return RCt;
}   /* _HttpAttr_type_v1 () */

static
rc_t CC
_HttpNodeAttr_v1 (
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
                    _HttpAttr_dispose_v1
                    );
    if ( RCt == 0 ) {
        Editor -> permissions = _HttpAttr_permissions_v1;
        Editor -> date = _HttpAttr_date_v1;
        Editor -> type = _HttpAttr_type_v1;

        * Attr = Editor;
    }
    else {
        free ( Editor );
    }

    return RCt;
}   /* _HttpNodeAttr_v1 () */

/*)))
 |||
 +++  Unified Discribe
 |||
(((*/

rc_t CC
_HttpNodeDescribe_v1 (
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

    Abbr = XFSHttpEntryIsFolder ( ( ( const struct XFSHttpNode * ) self ) -> entry )
            ? "HTTP FOLDER"
            : "HTTP NODE"
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
}   /* _HttpNodeDescribe_v1 () */

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
_RemoteRepositoryNodeConstructor (
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

    RCt = XFSHttpRootNodeMake (
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
            XFSHttpRootNodeDispose ( ( const struct XFSHttpRootNode * ) TheNode );
        }
    }

    return RCt;
}   /* _RemoteRepositoryNodeConstructor () */

/*)))
 |||
 +++    Non-Teleport methods to create nodes
 |||
(((*/

/*)))
 |||
 +++    HttpRootNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_RemoteRepositoryConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _RemoteRepositoryNodeConstructor (
                                        Model,
                                        Template,
                                        Alias,
                                        Node
                                        );

/*
pLogMsg ( klogDebug, "_RemoteRepositoryConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _RemoteRepositoryConstructor () */

static
rc_t CC
_RemoteRepositoryValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_RemoteRepositoryValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _RemoteRepositoryValidator () */

static const struct XFSTeleport _sRemoteRepositoryTeleport = {
                                        _RemoteRepositoryConstructor,
                                        _RemoteRepositoryValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSRemoteRepositoryProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sRemoteRepositoryTeleport;

    return 0;
}   /* XFSRemoteRepositoryProvider () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/* ALL BELOW TODO!!!                                               */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))
 |||
 +++    HttpRootNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_RemoteFileNodeConstructor (
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

    RCt = XFSHttpRootNodeMake (
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
            XFSHttpRootNodeDispose ( ( const struct XFSHttpRootNode * ) TheNode );
        }
    }

    return RCt;
}   /* _RemoteFileNodeConstructor () */

/*)))
 |||
 +++    HttpRootNode has a Teleport, and it is HERE
 |||
(((*/
static
rc_t CC
_RemoteFileConstructor (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            const struct XFSNode ** Node
)
{
    rc_t RCt;

    RCt = _RemoteFileNodeConstructor (
                                        Model,
                                        Template,
                                        Alias,
                                        Node
                                        );

/*
pLogMsg ( klogDebug, "_RemoteFileConstructor ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _RemoteFileConstructor () */

static
rc_t CC
_RemoteFileValidator (
            const struct XFSModel * Model,
            const struct XFSModelNode * Template,
            const char * Alias,
            uint32_t Flags
)
{
    rc_t RCt;

    RCt = 0;

/*
pLogMsg ( klogDebug, "_RemoteFileValidator ( $(model), $(template) (\"$(name)\"), \"$(alias)\" )", "model=%p,template=%p,name=%s,alias=%s", ( void * ) Model, ( void * ) Template, XFSModelNodeName ( Template ), ( Alias == NULL ? "NULL" : Alias ) );
*/

    return RCt;
}   /* _RemoteFileValidator () */

static const struct XFSTeleport _sRemoteFileTeleport = {
                                        _RemoteFileConstructor,
                                        _RemoteFileValidator,
                                        false
                                        };


LIB_EXPORT
rc_t CC
XFSRemoteFileProvider ( const struct XFSTeleport ** Teleport )
{
    if ( Teleport == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Teleport = & _sRemoteFileTeleport;

    return 0;
}   /* XFSRemoteFileProvider () */
