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

#ifndef _h_kfs_cacheteefile_wm_
#define _h_kfs_cacheteefile_wm_

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

/* KDirectoryMakeCacheTeeWM / KDirectoryVMakeCacheTeeWM
 *
 *  takes a KFile as source
 *
 *  "self" [ IN ] - KDirectory for handling the internal cache-file
 *
 *  "tee" [ OUT ] - return parameter for tee file
 *
 *  "to_wrap" [ IN ] - the source file, generally costly to read which is
 *  the motivation behind caching its output
 *
 *  "blocksize" [ IN ] - the cache-file page size
 *                       ( if 0 ... default-value will be 32M )
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
KFS_EXTERN rc_t CC KDirectoryMakeCacheTeeWM ( struct KDirectory * self,
                                             struct KFile const ** tee,
                                             struct KFile const * to_wrap,
                                             uint32_t blocksize,
                                             const char * location );
                                             
/* -----
 * checks if the given file is internally a CacheTee2File
 *
 */
KFS_EXTERN bool CC KFileIsKCacheTeeWM( const struct KFile * self );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_teefile_ */
