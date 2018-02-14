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

#ifndef _h_kfs_cachetee2file_
#define _h_kfs_cachetee2file_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
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
 * KDirectory
 */

/* MakeCacheTee
 *  takes a KFile as source
 *
 *  "tee" [ OUT ] - return parameter for tee file
 *
 *  "remote" [ IN ] - the source file, generally costly to read which is
 *  the motivation behind caching its output
 *
 *  "logger" [ IN, NULL OKAY ] - optional debugging output for observing
 *  access patterns of "remote"
 *
 *  "blocksize" [ IN ] - the cache-file page size
 *                       ( if 0 ... default-value will be 32k )
 *
 *  "cluster" [ IN ] - a blocking factor for accessing "remote"
 *  reads are in blocks of size "blocksize" * "cluster"
 *  ( 1...4 makes most sense )
 *
 *  "report" [ IN ] - when true, provides more verbose debugging output
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting full cache file, i.e. the name of the file
 *  as it would appear after promoting from partial to full status.
 *
 * the function creates a new ( cached ) KFile in **tee
 *
 * the caller can access this file to read and query the filesize, these are the only
 * supported messages to this object
 *
 * when the caller requests data that is not already cached in the local file,
 * the cached file will read from the remote-file, write into the local-file and then
 * return the requested data in the read-buffer
 *
 * when the caller requests data that is already cached, the data will be read from
 * the local file only, no remote request will be made
 *
 * when the caller requests data this is partially cached, the file will return the
 * part that comes first either from the remote or from the local file
 * in this case the file will return less data than requested
 *
 * when the caller opens an existing local file that contains a full copy of the
 * remote file, the local KFile will be returned in self
 *
 * this function will check if the remote-file and the local file ( if it already exists )
 * do have the same size and blocksize
 *
 */
KFS_EXTERN rc_t CC KDirectoryMakeCacheTee2 ( struct KDirectory * self,
                                             struct KFile const ** tee,
                                             struct KFile const * to_wrap,
                                             uint32_t blocksize,
                                             const char * path,
                                             ... );
                                             
KFS_EXTERN rc_t CC KDirectoryVMakeCacheTee2 ( struct KDirectory * self,
                                              struct KFile const ** tee,
                                              struct KFile const * to_wrap,
                                              uint32_t blocksize,
                                              const char * path,
                                              va_list args );

/* -----
 * checks if a given file ( has to be a local file )
 *
 * a ... is a backing cache-file for the CacheTee-file
 * b ... all blocks have been filled with the remote content
 *
 * it returns the result of the check in *is_complete
 */
KFS_EXTERN rc_t CC IsCacheTee2FileComplete( const struct KFile * self, bool * is_complete );


/* -----
 * truncates a cache-file for a CacheTee-filec by removing the bitmap/content-lenght/block-size
 *
 * it returns the result of the check in *is_complete
 */
KFS_EXTERN rc_t CC TruncateCacheTee2File( struct KFile * self );


/* -----
 * examens the file, and reports what percentage of blocks are in the cache...
 *
 */
KFS_EXTERN rc_t CC GetCacheTee2FileCompleteness( const struct KFile * self, float * percent, uint64_t * bytes_in_cache );


/* -----
 * examens the file, and reports the size of the original file ( without the cachefile-footer )
 *
 */
KFS_EXTERN rc_t CC GetCacheTee2FileTruncatedSize( const struct KFile * self, uint64_t * truncated_size );


/* -----
 * checks if the given file is internally a CacheTee2File
 *
 */
KFS_EXTERN bool CC KFileIsKCacheTee2File( const struct KFile * self );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_teefile_ */
