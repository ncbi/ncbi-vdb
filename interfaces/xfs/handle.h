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

#ifndef _handle_h_
#define _handle_h_

#include <klib/refcount.h>

#include <xfs/xfs-defs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  Forwards and others
 */
struct XFSNode;
struct XFSHandle;

/******************************************************************
 * Very awkward structure which need to be reimplemented
 * To avoid such things like getting list of files under Windows
 * which looks like :
 *    Handle = CreateFile ( file_name, open_existing, other_flags );
 *    GetFileInformation ( Handle, FILEINFO );
 *    FindFiles ( Handle );
 * Apparently it need to keep referrenced Node, and last file handle
 * for XFSFileEditor, which is only used to read/write data.
 * All other editors suppose to work on-place only
 ******************************************************************/
XFS_EXTERN rc_t CC XFSHandleMake (
                            const struct XFSNode * Node,
                            const struct XFSHandle ** Handle
                            );

/*))
 // Here we are deleting Handle
((*/
XFS_EXTERN rc_t CC XFSHandleDestroy (
                            const struct XFSHandle * self
                            );

/*))
 // This method will add referrence to XFSHandle
((*/
XFS_EXTERN rc_t CC XFSHandleAddRef (
                            const struct XFSHandle * self
                            );

/*))
 // This method will release referrence or destroy XFSHandle
((*/
XFS_EXTERN rc_t CC XFSHandleRelease (
                            const struct XFSHandle * self
                            );

/*))
 // That method will return handle on hanlde
((*/
XFS_EXTERN void * CC XFSHandleGet (
                            const struct XFSHandle * self
                            );

/*))
 // That method will set handle on handle
((*/
XFS_EXTERN rc_t CC XFSHandleSet (
                            const struct XFSHandle * self,
                            void * Handle
                            );

/*))
 // That method will return node for handle
((*/
XFS_EXTERN const struct XFSNode * CC XFSHandleNode (
                            const struct XFSHandle * self
                            );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _handle_h_ */
