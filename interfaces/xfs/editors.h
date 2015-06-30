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

#ifndef _xfs_editors_h_
#define _xfs_editors_h_

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*))
 ||   Lyrics.
 ++   That file contains editor interfaces for XFSNode: File, Dir, Attr
 ||
((*/

/*)))
 ///   And other forwards.
(((*/
struct XFSHandle;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))  Base editor for all types of editors
 ((*/
struct XFSEditor {

        /* Victim node for editor
         */
    const struct XFSNode * Node;

        /* Data - some editor related data
         */
    void * Data;

        /* Mandatary method to dispose editor
         */
    rc_t ( CC * dispose ) ( const struct XFSEditor * self );
};

XFS_EXTERN rc_t CC XFSEditorInit (
                            const struct XFSEditor * self,
                            const struct XFSNode * Node,
                            rc_t ( CC * dispose )
                                    ( const struct XFSEditor * self )
                            );

XFS_EXTERN rc_t CC XFSEditorDispose ( const struct XFSEditor * self );
XFS_EXTERN const struct XFSNode * CC XFSEditorNode (
                            const struct XFSEditor * self
                            );
XFS_EXTERN void * CC XFSEditorData ( const struct XFSEditor * self );
XFS_EXTERN rc_t CC XFSEditorSetData (
                            const struct XFSEditor * self,
                            void * Data
                            );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))   Editors, which are Dir, File and Attr
 ((*/

/*)))
 (((    Directory editor, and relative methods
  )))
 (((*/
struct XFSDirEditor {
    struct XFSEditor Papahen;

    rc_t ( CC * list ) (
                        const struct XFSDirEditor * self,
                        const struct KNamelist ** List
                        );
    rc_t ( CC * find ) (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        const struct XFSNode ** Node
                        );
        /*) Creates and opens file with write access
         (*/
    rc_t ( CC * create_file ) (
                        const struct XFSDirEditor * self,
                        const char * Name,
                        XFSNMode Mode,
                        const struct XFSHandle ** Handle
                        );
        /*) Creates and directory no mode or access involved
         (*/
    rc_t ( CC * create_dir ) (
                        const struct XFSDirEditor * self,
                        const char * Name
                        );
        /*) Deletes both file or directory
         (*/
    rc_t ( CC * delete ) (
                        const struct XFSDirEditor * self,
                        const char * Name
                        );
        /*) Move is always happens within filesystem
         (*/
    rc_t ( CC * move ) (
                        const struct XFSDirEditor * self,
                        const char * OldName,
                        const struct XFSNode * NewDir,
                        const char * NewName
                        );
};

XFS_EXTERN rc_t CC XFSDirEditorList (
                                const struct XFSDirEditor * self,
                                const struct KNamelist ** List
                                );
XFS_EXTERN rc_t CC XFSDirEditorFind (
                                const struct XFSDirEditor * self,
                                const char * Name,
                                const struct XFSNode ** Node
                                );
XFS_EXTERN rc_t CC XFSDirEditorCreate (
                                const struct XFSDirEditor * self,
                                const char * Name,
                                XFSNMode Mode,
                                const struct XFSHandle ** Handle
                                );
XFS_EXTERN rc_t CC XFSDirEditorCreateDir (
                                const struct XFSDirEditor * self,
                                const char * Name
                                );
XFS_EXTERN rc_t CC XFSDirEditorDelete (
                                const struct XFSDirEditor * self,
                                const char * Name
                                );
XFS_EXTERN rc_t CC XFSDirEditorMove (
                                const struct XFSDirEditor * self,
                                const char * OldName,
                                const struct XFSNode * NewDir,
                                const char * NewName
                                );

/*)))
 (((    File editor, and relative methods
  )))
 (((*/
struct XFSFileEditor {
    struct XFSEditor Papahen;

    rc_t ( CC * open ) (
                    const struct XFSFileEditor * self,
                    XFSNMode Mode
                    );

    rc_t ( CC * close ) (
                    const struct XFSFileEditor * self
                    );

    rc_t ( CC * read ) (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    void * Buffer,
                    size_t SizeToRead,
                    size_t * NumReaded
                    );

    rc_t ( CC * write ) (
                    const struct XFSFileEditor * self,
                    uint64_t Offset,
                    const void * Buffer,
                    size_t SizeToWrite,
                    size_t * NumWritten
                    );

    rc_t ( CC * size ) (
                    const struct XFSFileEditor * self,
                    uint64_t * Size
                    );
    rc_t ( CC * set_size ) (
                    const struct XFSFileEditor * self,
                    uint64_t Size
                    );

};

XFS_EXTERN rc_t CC XFSFileEditorOpen (
                                const struct XFSFileEditor * self,
                                XFSNMode Mode
                                );

XFS_EXTERN rc_t CC XFSFileEditorClose (
                                const struct XFSFileEditor * self
                                );

XFS_EXTERN rc_t CC XFSFileEditorRead (
                                const struct XFSFileEditor * self,
                                uint64_t Offset,
                                void * Buffer,
                                size_t SizeToRead,
                                size_t * NumReaded
                                );

XFS_EXTERN rc_t CC XFSFileEditorWrite (
                                const struct XFSFileEditor * self,
                                uint64_t Offset,
                                const void * Buffer,
                                size_t SizeToWrite,
                                size_t * NumWritten
                                );

XFS_EXTERN rc_t CC XFSFileEditorSize (
                                    const struct XFSFileEditor * self,
                                    uint64_t * Size
                                    );

XFS_EXTERN rc_t CC XFSFileEditorSetSize (
                                    const struct XFSFileEditor * self,
                                    uint64_t Size
                                    );

/*)))
 (((    Attribute editor, and relative methods
  )))
 (((*/

struct XFSAttrEditor {
    struct XFSEditor Papahen;

    rc_t ( CC * permissions ) (
                    const struct XFSAttrEditor * self,
                    const char ** Permissions
                    );
    rc_t ( CC * set_permissions ) (
                    const struct XFSAttrEditor * self,
                    const char * Permissions
                    );

    rc_t ( CC * date ) (
                    const struct XFSAttrEditor * self,
                    KTime_t * Time
                    );
    rc_t ( CC * set_date ) (
                    const struct XFSAttrEditor * self,
                    KTime_t Time
                    );

    rc_t ( CC * type ) (
                    const struct XFSAttrEditor * self,
                    XFSNType * Type
                    );
};

XFS_EXTERN rc_t CC XFSAttrEditorPermissions (
                                    const struct XFSAttrEditor * self,
                                    const char ** Permissions
                                    );

XFS_EXTERN rc_t CC XFSAttrEditorDate (
                                    const struct XFSAttrEditor * self,
                                    KTime_t * Time
                                    );

XFS_EXTERN rc_t CC XFSAttrEditorType (
                                    const struct XFSAttrEditor * self,
                                    XFSNType * Type
                                    );

XFS_EXTERN rc_t CC XFSAttrEditorSetPermissions (
                                    const struct XFSAttrEditor * self,
                                    const char * Permissions
                                    );

XFS_EXTERN rc_t CC XFSAttrEditorSetDate (
                                    const struct XFSAttrEditor * self,
                                    KTime_t Time
                                    );

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _xfs_editors_h_ */
