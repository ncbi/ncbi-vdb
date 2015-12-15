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

#include <xfs/tree.h>
#include <xfs/node.h>
#include <xfs/editors.h>

#include "mehr.h"
#include "zehr.h"

#include <sysalloc.h>

#include <string.h> /* memset () */

/*)))
 |||
 +++    All about Editors, and other simple Editors
 |||
(((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))   Editor ...
 (((*/
LIB_EXPORT
rc_t CC
XFSEditorInit (
            const struct XFSEditor * self,
            const struct XFSNode * Node,
            rc_t ( CC * dispose ) ( const struct XFSEditor * self )
)
{
    rc_t RCt;
    struct XFSEditor * Editor;

    RCt = 0;
    Editor = ( struct XFSEditor * ) self;

    if ( Editor == NULL || Node == NULL || dispose == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSNodeAddRef ( Node );
    if ( RCt == 0 ) {

        memset ( Editor, 0, sizeof ( struct XFSEditor ) );

        Editor -> Node = Node;
        Editor -> dispose = dispose;
        Editor -> Data = NULL;
    }

    return RCt;
}   /* XFSEditorInit () */

LIB_EXPORT
rc_t CC
XFSEditorDispose ( const struct XFSEditor * self )
{
    struct XFSEditor * Editor;

    Editor = ( struct XFSEditor * ) self;

    if ( Editor == NULL ) {
        return 0;
    }

    if ( Editor -> dispose == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( Editor -> Node != NULL ) {
        XFSNodeRelease ( Editor -> Node );

        Editor -> Node = NULL;
    }

    return Editor -> dispose ( self );
}   /* XFSEditorDispose () */

LIB_EXPORT
const struct XFSNode * CC
XFSEditorNode ( const struct XFSEditor * self )
{
    return self == NULL ? NULL : ( self -> Node );
}   /* XFSEditorNode () */

LIB_EXPORT
void * CC
XFSEditorData ( const struct XFSEditor * self )
{
    return self == NULL ? self -> Data : NULL;
}   /* XFSEditorData () */

LIB_EXPORT
rc_t CC
XFSEditorSetData ( const struct XFSEditor * self, void * Data )
{
    XFS_CAN ( self )

    ( ( struct XFSEditor * ) self ) -> Data = Data;

    return 0;
}   /* XFSEditorSetData () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))   EditorF ...
 (((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))   DirEditor ...
 (((*/

LIB_EXPORT
rc_t CC
XFSDirEditorList (
                const struct XFSDirEditor * self,
                const struct KNamelist ** List
)
{
    if ( self == NULL || List == NULL ) {
        return XFS_RC ( rcNull );
    }

    * List = NULL;

    if ( self -> list != NULL ) {
        return self -> list ( self, List );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDirEditorList () */

LIB_EXPORT
rc_t CC
XFSDirEditorFind (
                const struct XFSDirEditor * self,
                const char * Name,
                const struct XFSNode ** Node
)
{
    if ( self == NULL || Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Node = NULL;

    if ( self -> find != NULL ) {
        return self -> find ( self, Name, Node );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDirEditorFind () */

LIB_EXPORT
rc_t CC
XFSDirEditorCreate (
                const struct XFSDirEditor * self,
                const char * Name,
                XFSNMode Mode,
                const struct XFSHandle ** Handle
)
{
    if ( self == NULL || Name == NULL || Handle == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Handle = NULL;

    if ( self -> create_file != NULL ) {
        return self -> create_file ( self, Name, Mode, Handle );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDirEditorCreate () */

LIB_EXPORT
rc_t CC
XFSDirEditorCreateDir (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    if ( self == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> create_dir != NULL ) {
        return self -> create_dir ( self, Name );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDirEditorCreateDir () */

LIB_EXPORT
rc_t CC
XFSDirEditorDelete (
                const struct XFSDirEditor * self,
                const char * Name
)
{
    if ( self == NULL || Name == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> delete != NULL ) {
        return self -> delete ( self, Name );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDirEditorDelete () */

LIB_EXPORT
rc_t CC
XFSDirEditorMove (
                const struct XFSDirEditor * self,
                const char * OldName,
                const struct XFSNode * NewDir,
                const char * NewName
)
{
    if ( self == NULL || OldName == NULL
        || NewDir == NULL || NewName == NULL
    ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> move != NULL ) {
        return self -> move ( self, OldName, NewDir, NewName );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSDirEditorMove () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))   FileEditor ...
 (((*/

LIB_EXPORT
rc_t CC
XFSFileEditorOpen ( const struct XFSFileEditor * self, XFSNMode Mode )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> open != NULL ) {
        return self -> open ( self, Mode );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSFileModeEditorOpen () */

LIB_EXPORT
rc_t CC
XFSFileEditorClose ( const struct XFSFileEditor * self )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> close != NULL ) {
        return self -> close ( self );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSFileEditorClose () */

LIB_EXPORT
rc_t CC
XFSFileEditorRead (
                const struct XFSFileEditor * self,
                uint64_t Offset,
                void * Buffer,
                size_t SizeToRead,
                size_t * NumReaded
)
{
    if ( self == NULL || Buffer == NULL || NumReaded == NULL ) {
        return XFS_RC ( rcNull );
    }

    * NumReaded = 0;

    if ( self -> read != NULL ) {
        return self -> read (
                            self,
                            Offset,
                            Buffer,
                            SizeToRead,
                            NumReaded
                            );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSFileEditorRead () */

LIB_EXPORT
rc_t CC
XFSFileEditorWrite (
                const struct XFSFileEditor * self,
                uint64_t Offset,
                const void * Buffer,
                size_t SizeToWrite,
                size_t * NumWritten
)
{
    if ( self == NULL || Buffer == NULL || NumWritten == NULL ) {
        return XFS_RC ( rcNull );
    }

    * NumWritten = 0;

    if ( self -> write != NULL ) {
        return self -> write (
                            self,
                            Offset,
                            Buffer,
                            SizeToWrite,
                            NumWritten
                            );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSFileEditorWrite () */

LIB_EXPORT
rc_t CC
XFSFileEditorSize ( const struct XFSFileEditor * self, uint64_t * Size )
{
    if ( self == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Size = 0;

    if ( self -> size != NULL ) {
        return self -> size ( self, Size );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSFileEditorSize () */

LIB_EXPORT
rc_t CC
XFSFileEditorSetSize ( const struct XFSFileEditor * self, uint64_t Size )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> set_size != NULL ) {
        return self -> set_size ( self, Size );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSFileEditorSetSize () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*)))   AttrEditor ...
 (((*/

LIB_EXPORT
rc_t CC
XFSAttrEditorPermissions (
                const struct XFSAttrEditor * self,
                const char ** Permissions
)
{
    if ( self == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Permissions = NULL;

    if ( self -> permissions != NULL ) {
        return self -> permissions ( self, Permissions );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSAttrEditorPermissions () */

LIB_EXPORT
rc_t CC
XFSAttrEditorSetPermissions (
                const struct XFSAttrEditor * self,
                const char * Permissions
)
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> set_permissions != NULL ) {
        return self -> set_permissions ( self, Permissions );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSAttrEditorSetPermissions () */

LIB_EXPORT
rc_t CC
XFSAttrEditorDate ( const struct XFSAttrEditor * self, KTime_t * Time )
{
    if ( self == NULL || Time == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Time = 0;

    if ( self -> date != NULL ) {
        return self -> date ( self, Time );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSAttrEditorDate () */

LIB_EXPORT
rc_t CC
XFSAttrEditorSetDate ( const struct XFSAttrEditor * self, KTime_t Time )
{
    if ( self == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( self -> set_date != NULL ) {
        return self -> set_date ( self, Time );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSAttrEditorSetDate () */

LIB_EXPORT
rc_t CC
XFSAttrEditorType ( const struct XFSAttrEditor * self, XFSNType * Type )
{
    if ( self == NULL || Type == NULL ) {
        return XFS_RC ( rcNull );
    }

    * Type = kxfsNotFound;

    if ( self -> type != NULL ) {
        return self -> type ( self, Type );
    }

    return XFS_RC ( rcUnsupported );
}   /* XFSAttrEditorType () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

