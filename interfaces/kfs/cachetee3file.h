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

#ifndef _h_kfs_cachetee3file_
#define _h_kfs_cachetee3file_

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

/* MakeKCacheTeeFile_v3
 *  takes a KFile as source
 *
 *  "tee" [ OUT ] - return parameter for tee file
 *
 *  "source" [ IN ] - the source file, generally costly to read which is
 *  the motivation behind caching its output
 *
 *  "page_size" [ IN ] - the cache-file page size
 *                       ( if 0 ... default-value will be 32k )
 *
 *  "cluster_factor" [ IN ] - a blocking factor for accessing "source"
 *  reads are in blocks of size "page_size" * "cluster_factor".
 *
 *  "ram_pages" [ IN ] - a requested number of pages to be held
 *  in RAM cache.
 *
 *  "promote" [ IN ] - whether to promote cache file to complete file once
 *  completed.
 *
 *  "temporary" [ IN ] - cache file should be deleted upon process exit
 *
 *  "path" [ IN ] and "args" [ IN ] - NUL terminated string in directory-native
 *  character set denoting full cache file, i.e. the name of the file
 *  as it would appear after promoting from partial to full status.
 *  a NULL or empty path indicates NO FILE CACHING, but perform RAM caching.
 *
 * the function creates a new ( cached ) KFile in "tee"
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
KFS_EXTERN rc_t CC  KDirectoryMakeKCacheTeeFile_v3 ( struct KDirectory * self,
    struct KFile const ** tee, struct KFile const * source,
    uint32_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    bool try_promote_on_close, bool remove_on_close, const char * path, ... );

KFS_EXTERN rc_t CC  KDirectoryVMakeKCacheTeeFile_v3 ( struct KDirectory * self,
    struct KFile const ** tee, struct KFile const * source,
    uint32_t page_size, uint32_t cluster_factor, uint32_t ram_pages,
    bool try_promote_on_close, bool remove_on_close, const char * path, va_list args );

/* IsComplete
 *  checks if a given file ( has to be a local file )
 *   a) is a backing cache-file for the CacheTee-file
 *   b) all blocks have been filled with the remote content
 *
 *  "is_complete" [ OUT ] - return value: true when file is complete
 */
KFS_EXTERN rc_t CC CacheTee3FileIsComplete ( struct KFile const * self, bool * is_complete );

/* IsValid
 *  checks if a given file ( has to be a local file )
 *  ss a backing cache-file for the CacheTee-file ( right ratio of orig_size / page_size / total_size )
 *
 *  "is_complete" [ OUT ] - return value: true when file is complete
 */
KFS_EXTERN rc_t CC CacheTee3FileIsValid ( struct KFile const * self, bool * is_valid );

/* Finalize
 *  removes the cache-tee file metadata
 */
KFS_EXTERN rc_t CC CacheTee3FileFinalize ( struct KFile * self );


/* GetCompleteness
 *  examines the file, and reports what percentage of blocks are in the cache...
 *
 *  "percent" [ OUT, NULL OKAY ] - returns a percentage of completeness
 *
 *  "bytes_in_cache" [ OUT, NULL OKAY ] - returns number of bytes in cache
 *
 *  NB - either parameter may be NULL, but not both.
 */
KFS_EXTERN rc_t CC CacheTee3FileGetCompleteness ( struct KFile const * self,
    double * percent, uint64_t * bytes_in_cache );

/* GetOriginalSize
 *  examines the file, and reports the size of the original file ( without the cachefile-footer )
 *
 *  "original_size" [ OUT ] - return value for the original file size
 */
KFS_EXTERN rc_t CC CacheTee3FileGetOriginalSize ( struct KFile const * self,
    uint64_t * original_size );

/* IsKCacheTee3File
 *  checks if the given implementation is a CacheTee3File
 */
KFS_EXTERN bool CC KFileIsKCacheTeeFile_v3( struct KFile const * self );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_cachetee3file_ */
