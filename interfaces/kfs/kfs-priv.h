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

#ifndef _h_kfs_priv_
#define _h_kfs_priv_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * struct/class forwards
 */
struct KArrayFile;
struct KDirectory;
struct KDlset;
struct KFile;
struct KFile_v2;
struct KNamelist;
struct KPath;

/* MakeFDFile
 *  creates a file from a file-descriptor
 *  not supported under Windows
 */
KFS_EXTERN rc_t CC KFileMakeFDFileRead ( struct KFile const **f, int fd );
KFS_EXTERN rc_t CC KFileMakeFDFileWrite ( struct KFile **f, bool update, int fd );

KFS_EXTERN struct KFile_v2 const * CC KFileMakeFDFileRead_v2 ( ctx_t ctx, int fd );
KFS_EXTERN struct KFile_v2 * CC KFileMakeFDFileWrite_v2 ( ctx_t ctx, bool update, int fd );

/* GetMeta
 *  extracts metadata into a string-vector
 *
 *  "key"   [ IN ]  - the key which part of the metadata to retrieve
 *
 *  "list"  [ OUT ] - the metadata will be filled into this list
 *
 */
KFS_EXTERN rc_t CC KArrayFileGetMeta ( struct KArrayFile const *self, const char *key,
    const struct KNamelist **list );

/* List
 *  list the paths to the libraries in the set
 */
KFS_EXTERN rc_t CC KDlsetList ( struct KDlset const *self, struct KNamelist **list );


#if WINDOWS

/* PosixStringToSystemString
 * converts posix path string to system path
 *  "buffer" [ OUT ] - NUL terminated system path string
 *  "bsize" [ IN ] - buffer length
 *  "path" [ IN ] - NUL terminated posix path string
 */
KFS_EXTERN rc_t CC KDirectoryPosixStringToSystemString (
    const struct KDirectory *self,
    char *buffer, size_t bsize, const char *path, ... );

#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_priv_ */
