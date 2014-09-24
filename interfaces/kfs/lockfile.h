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

#ifndef _h_kfs_lockfile_
#define _h_kfs_lockfile_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_kfs_defs_
#include <kfs/defs.h>
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


/*--------------------------------------------------------------------------
 * KLockFile
 *  a mutual exclusion lock on a file
 *  the lock is acquired upon creation
 *  and released upon destruction
 *
 * NB - only guaranteed to work when used from a single host
 */
typedef struct KLockFile KLockFile;


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KLockFileAddRef ( const KLockFile *self );
KFS_EXTERN rc_t CC KLockFileRelease ( const KLockFile *self );



/*--------------------------------------------------------------------------
 * KDirectory
 *  interface extensions
 */


/* CreateLockFile
 *  attempts to create a KLockFile
 *
 *  "lock" [ OUT ] - return parameter for newly created lock file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting lock file
 */
KFS_EXTERN rc_t CC KDirectoryCreateLockFile ( struct KDirectory *self,
    KLockFile **lock, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVCreateLockFile ( struct KDirectory *self,
    KLockFile **lock, const char *path, va_list args );


/* CreateExclusiveAccessFile
 *  opens a file with exclusive write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
KFS_EXTERN rc_t CC KDirectoryCreateExclusiveAccessFile ( struct KDirectory *self, struct KFile **f,
    bool update, uint32_t access, KCreateMode mode, const char *path, ... );
KFS_EXTERN rc_t CC KDirectoryVCreateExclusiveAccessFile ( struct KDirectory *self, struct KFile **f,
    bool update, uint32_t access, KCreateMode mode, const char *path, va_list args );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_lockfile_ */
