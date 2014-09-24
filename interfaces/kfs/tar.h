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

#ifndef _h_kfs_tar_
#define _h_kfs_tar_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;
struct KDirectory;
struct KToc;

/*--------------------------------------------------------------------------
 * KDirectory
 *  TAR-specific extensions
 */

/* ParseTAR
 *  A parse function for tar files suitable for KDirectoryOpenArcDirRead
 *
 * [INOUT] KToc*    self    The TOC (table of contents) to fill in with the parse
 * [IN]    const KFile* kfile   The opened (as KFile) archive to parse.
 */
KFS_EXTERN rc_t CC KArcParseTAR( struct KToc * self, 
                                 const void * kfile,
                                 bool(CC*f)(const struct KDirectory*,const char *, void*),
                                 void *filterparam);

/* KDirectoryOpenTarArchive
 *  opens a named TAR archive
 *
 *  "tar_dir" [ OUT ] - return parameter for archive
 *
 *  "chroot" [ IN ] - if non-zero, the new directory becomes
 *  chroot'd and will interpret paths beginning with '/'
 *  relative to itself.
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target archive
 */
KFS_EXTERN int CC KDirectoryOpenTarArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *path, ... );
KFS_EXTERN int CC KDirectoryVOpenTarArchiveRead ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *path, va_list args );

KFS_EXTERN int CC KDirectoryOpenTarArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *path, ... );
KFS_EXTERN int CC KDirectoryVOpenTarArchiveRead_silent ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const char *path, va_list args );

KFS_EXTERN int CC KDirectoryOpenTarArchiveRead_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const struct KFile * f, const char *path, ... );
KFS_EXTERN int CC KDirectoryVOpenTarArchiveRead_silent_preopened ( struct KDirectory const *self,
    struct KDirectory const **tar_dir, int chroot, const struct KFile * f, const char *path, va_list args );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_tar_ */
