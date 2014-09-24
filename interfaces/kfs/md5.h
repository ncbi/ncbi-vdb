/*=======================================================================================
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

#ifndef _h_kfs_md5_
#define _h_kfs_md5_

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
 * forwards
 */
struct KFile;


/*--------------------------------------------------------------------------
 * KMD5SumFmt
 *  a formatter for reading or writing an md5sum-style checksum file
 */
typedef struct KMD5SumFmt KMD5SumFmt;


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KMD5SumFmtAddRef ( const KMD5SumFmt *self );
KFS_EXTERN rc_t CC KMD5SumFmtRelease ( const KMD5SumFmt *self );


/* Make
 *  creates a formatted accessor to textual checksum file
 *  takes over ownership of "in"/"out" file reference
 *
 *  "in" [ IN ] - readable checksum input file
 *  -OR-
 *  "out" [ IN ] - writable checksum file. if readable as well,
 *  its contents will be preserved.
 */
KFS_EXTERN rc_t CC KMD5SumFmtMakeRead ( const KMD5SumFmt **f, struct KFile const *in );
KFS_EXTERN rc_t CC KMD5SumFmtMakeUpdate ( KMD5SumFmt **f, struct KFile *out );


/* Count
 *  return the number of checksums in file
 */
KFS_EXTERN rc_t CC KMD5SumFmtCount ( const KMD5SumFmt *self, uint32_t *count );


/* Get
 *  retrieve an indexed checksum line
 *
 *  "idx" [ IN ] - zero-based index of checksum to access
 *
 *  "path" [ OUT ] and "size" [ IN ] - return buffer for file path
 *
 *  "digest" [ OUT ] - return parameter for the MD5 checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
KFS_EXTERN rc_t CC KMD5SumFmtGet ( const KMD5SumFmt *self, uint32_t idx,
    char *path, size_t size, uint8_t digest [ 16 ], bool *bin );


/* Find
 *  retrieve checksum line by path
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  "digest" [ OUT ] - return parameter for the MD5 checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
KFS_EXTERN rc_t CC KMD5SumFmtFind ( const KMD5SumFmt *self,
    const char *path, uint8_t digest [ 16 ], bool *bin );


/* Delete
 *  remove an existing line
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  returns rcNotFound if not present
 */
KFS_EXTERN rc_t CC KMD5SumFmtDelete ( KMD5SumFmt *self, const char *path );


/* Update
 *  add a checksum line to file or update an existing line
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  "digest" [ IN ] - calculated MD5 checksum
 *
 *  "bin" [ IN ] - true if checksum was calculated on all bytes
 *  in file, false if fopen mode used to read file was text.
 *
 * NB - the md5sum tool operates with the assumption that files
 *  are opened using "fopen". it records the mode used by prepending
 *  '*' to indicate binary and ' ' to indicate text mode within the
 *  checksum line.
 *
 *  unfortunately, this creates a situation where text-mode files
 *  checksummed on DOS cannot be properly checked on any other
 *  platform without the aid of some filter to drop out '\r' from
 *  a '\r\n' sequence.
 *
 *  fortunately, most users are unaware of the binary/text mode
 *  switches available with md5sum and use the default mode, which
 *  is binary on DOS and text on Unix, which happens to be equivalent
 *  to binary ( making one wonder why binary is not instead the
 *  universal default ).
 *
 *  the end result is that we need to indicate whether CR stripping
 *  was performed while at the same time behaving like md5sum.
 */
KFS_EXTERN rc_t CC KMD5SumFmtUpdate ( KMD5SumFmt *self, const char *path,
    const uint8_t digest [ 16 ], bool bin );

/* Rename
 * Rename the checksum line to reflect a file name change but no content change.
 *
 *  "newpath" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file's new name
 *
 *  "oldpath" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file's old name
 *
 * Parameter order matches KDirectoryRename()
 */
KFS_EXTERN rc_t CC KMD5SumFmtRename ( KMD5SumFmt *self, const char *oldpath, const char *newpath );


/* Flush
 *  write data to disk, but stay open
 */
KFS_EXTERN rc_t CC KMD5SumFmtFlush ( KMD5SumFmt *self );


/*--------------------------------------------------------------------------
 * MD5 KFile extensions
 */

/* MakeMD5Read
 *  creates an adapter to check MD5 checksum for a source file
 *
 *  "f" [ OUT ] - return parameter for file with md5 check
 *
 *  "src" [ IN ] - source file with read permission
 *
 *  "digest" [ IN ] - MD5 checksum to match
 *
 * NB - creates a read-only file that does NOT support random access
 *  i.e. must be consumed serially starting from offset 0
 */
KFS_EXTERN rc_t CC KFileMakeMD5Read ( struct KFile const **f,
    struct KFile const *src, const uint8_t digest [ 16 ] );

    
/*--------------------------------------------------------------------------
 * KMD5File
 *  subclass of KFile
 */
typedef struct KMD5File KMD5File;


/* MakeWrite
 * MakeAppend
 *  creates an adapter to create MD5 checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with md5 check
 *
 *  "out" [ IN ] - output file with write permission
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "md5" [ IN ] - md5 checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "md5".
 *
 *  "path" [ IN ] - md5 checksum output file path, used to
 *  create standard md5sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
KFS_EXTERN rc_t CC KMD5FileMakeWrite ( KMD5File **f,
    struct KFile *out, KMD5SumFmt *md5, const char *path );
KFS_EXTERN rc_t CC KMD5FileMakeAppend ( KMD5File **f,
    struct KFile *out, KMD5SumFmt *md5, const char *path );

/* MakeReadNew
 *  creates an adapter to create MD5 checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with md5 check
 *
 *  "in" [ IN ] - input file 
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "md5" [ IN ] - md5 checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "md5".
 *
 *  "path" [ IN ] - md5 checksum output file path, used to
 *  create standard md5sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
KFS_EXTERN rc_t CC KFileMakeNewMD5Read ( const struct KFile **fp,
    const struct KFile *in, KMD5SumFmt *md5, const char *path );


/* ToKFile
 *  an MD5 file is a subclass of KFile
 *  this is an explicit cast operator
 */
KFS_EXTERN struct KFile* CC KMD5FileToKFile ( KMD5File *self );
KFS_EXTERN struct KFile const* CC KMD5FileToKFileConst ( const KMD5File *self );

/* BeginTransaction
 *  preserve current MD5 state and file marker
 *  in preparation for rollback
 */
KFS_EXTERN rc_t CC KMD5FileBeginTransaction ( KMD5File *self );


/* Commit
 *  accept current MD5 state as correct
 */
KFS_EXTERN rc_t CC KMD5FileCommit ( KMD5File *self );


/* Revert
 *  reset state to previous transaction boundary conditions
 */
KFS_EXTERN rc_t CC KMD5FileRevert ( KMD5File *self );

/* Reset
 *  reset state to that of an empty file
 *  aborts any open transaction
 */
KFS_EXTERN rc_t CC KMD5FileReset ( KMD5File *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_md5_ */
