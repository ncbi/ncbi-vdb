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

#ifndef _h_kfs_crc_
#define _h_kfs_crc_

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
 * KCRC32SumFmt
 *  a formatter for reading or writing an crc32sum-style checksum file
 */
typedef struct KCRC32SumFmt KCRC32SumFmt;


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KCRC32SumFmtAddRef ( const KCRC32SumFmt *self );
KFS_EXTERN rc_t CC KCRC32SumFmtRelease ( const KCRC32SumFmt *self );


/* Make
 *  creates a formatted accessor to textual checksum file
 *  takes over ownership of "in"/"out" file reference
 *
 *  "in" [ IN ] - readable checksum input file
 *  -OR-
 *  "out" [ IN ] - writable checksum file. if readable as well,
 *  its contents will be preserved.
 */
KFS_EXTERN rc_t CC KCRC32SumFmtMakeRead ( const KCRC32SumFmt **f, struct KFile const *in );
KFS_EXTERN rc_t CC KCRC32SumFmtMakeUpdate ( KCRC32SumFmt **f, struct KFile *out );


/* Count
 *  return the number of checksums in file
 */
KFS_EXTERN rc_t CC KCRC32SumFmtCount ( const KCRC32SumFmt *self, uint32_t *count );


/* Get
 *  retrieve an indexed checksum line
 *
 *  "idx" [ IN ] - zero-based index of checksum to access
 *
 *  "path" [ OUT ] and "size" [ IN ] - return buffer for file path
 *
 *  "crc32" [ OUT ] - return parameter for the checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
KFS_EXTERN rc_t CC KCRC32SumFmtGet ( const KCRC32SumFmt *self, uint32_t idx,
    char *path, size_t size, uint32_t *crc32, bool *bin );


/* Find
 *  retrieve checksum line by path
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  "crc32" [ OUT ] - return parameter for the checksum
 *
 *  "bin" [ OUT, NULL OKAY ] - true if fopen mode used to
 *  calculate "digest" was binary, false if text. pertinent if
 *  digest was calculated using DOS text mode.
 */
KFS_EXTERN rc_t CC KCRC32SumFmtFind ( const KCRC32SumFmt *self,
    const char *path, uint32_t *crc32, bool *bin );


/* Delete
 *  remove an existing line
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  returns rcNotFound if not present
 */
KFS_EXTERN rc_t CC KCRC32SumFmtDelete ( KCRC32SumFmt *self, const char *path );


/* Update
 *  add a checksum line to file or update an existing line
 *
 *  "path" [ IN ] - NUL terminated string in OS-native char set
 *  denoting path to file
 *
 *  "crc32" [ IN ] - calculated checksum
 *
 *  "bin" [ IN ] - true if checksum was calculated on all bytes
 *  in file, false if fopen mode used to read file was text.
 *
 * NB - the crc32sum tool was designed to behave like md5sum.
 *  the md5sum tool operates with the assumption that files
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
KFS_EXTERN rc_t CC KCRC32SumFmtUpdate ( KCRC32SumFmt *self, const char *path,
    uint32_t crc32, bool bin );

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
KFS_EXTERN rc_t CC KCRC32SumFmtRename ( KCRC32SumFmt *self, const char *oldpath, const char *newpath );


/* Flush
 *  write data to disk, but stay open
 */
KFS_EXTERN rc_t CC KCRC32SumFmtFlush ( KCRC32SumFmt *self );


/*--------------------------------------------------------------------------
 * CRC32 KFile extensions
 */

/* MakeCRC32Read
 *  creates an adapter to check CRC32 checksum for a source file
 *
 *  "f" [ OUT ] - return parameter for file with crc check
 *
 *  "src" [ IN ] - source file with read permission
 *
 *  "crc32" [ IN ] - checksum to match
 *
 * NB - creates a read-only file that does NOT support random access
 *  i.e. must be consumed serially starting from offset 0
 */
KFS_EXTERN rc_t CC KFileMakeCRC32Read ( struct KFile const **f,
    struct KFile const *src, uint32_t crc32 );




/*--------------------------------------------------------------------------
 * KCRC32File
 *  subclass of KFile
 */
typedef struct KCRC32File KCRC32File;


/* MakeWrite
 * MakeAppend
 *  creates an adapter to calculate CRC checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with crc check
 *
 *  "out" [ IN ] - output file with write permission
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "fmt" [ IN ] - checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "fmt".
 *
 *  "path" [ IN ] - checksum output file path, used to
 *  create standard md5sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
KFS_EXTERN rc_t CC KCRC32FileMakeWrite ( KCRC32File **f,
    struct KFile *out, KCRC32SumFmt *fmt, const char *path );
KFS_EXTERN rc_t CC KCRC32FileMakeAppend ( KCRC32File **f,
    struct KFile *out, KCRC32SumFmt *fmt, const char *path );

/* MakeReadNew
 *  creates an adapter to calculate CRC checksum for a file
 *
 *  "f" [ OUT ] - return parameter for file with crc check
 *
 *  "in" [ IN ] - input file 
 *  since this file is being wrapped by "f", ownership of
 *  the provided reference is transferred to "f"
 *
 *  "fmt" [ IN ] - checksum output formatter.
 *  since this may be used for multiple checksums, "f" will
 *  attach its own reference to "fmt".
 *
 *  "path" [ IN ] - checksum output file path, used to
 *  create standard md5sum file entry.
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
KFS_EXTERN rc_t CC KFileMakeNewCRC32Read ( const struct KFile **fp,
    const struct KFile *in, KCRC32SumFmt *fmt, const char *path );


/* ToKFile
 *  a CRC32 file is a subclass of KFile
 *  this is an explicit cast operator
 */
KFS_EXTERN struct KFile* CC KCRC32FileToKFile ( KCRC32File *self );
KFS_EXTERN struct KFile const* CC KCRC32FileToKFileConst ( const KCRC32File *self );

/* BeginTransaction
 *  preserve current CRC state and file marker
 *  in preparation for rollback
 */
KFS_EXTERN rc_t CC KCRC32FileBeginTransaction ( KCRC32File *self );


/* Commit
 *  accept current CRC state as correct
 */
KFS_EXTERN rc_t CC KCRC32FileCommit ( KCRC32File *self );


/* Revert
 *  reset state to previous transaction boundary conditions
 */
KFS_EXTERN rc_t CC KCRC32FileRevert ( KCRC32File *self );

/* Reset
 *  reset state to that of an empty file
 *  aborts any open transaction
 */
KFS_EXTERN rc_t CC KCRC32FileReset ( KCRC32File *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_crc_ */
