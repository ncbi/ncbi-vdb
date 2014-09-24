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

#ifndef _h_kfs_gzip_
#define _h_kfs_gzip_

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

/* MakeGzipForRead
 *  creates an adapter to gunzip a source file
 *
 *  "gz" [ OUT ] - return parameter for decompressed file
 *
 *  "src" [ IN ] - compressed source file with read permission
 *
 * NB - creates a read-only file that does NOT support random access
 *  i.e. must be consumed serially starting from offset 0
 */
KFS_EXTERN rc_t CC KFileMakeGzipForRead ( struct KFile const **gz, struct KFile const *src );


/* MakeGzipForWrite
 *  creates an adapter to gzip a source file
 *
 *  "gz" [ OUT ] - return parameter for compressed file
 *
 *  "src" [ IN ] - uncompressed source file with write permission
 *
 * NB - creates a write-only file that does NOT suppport random access
 *  i.e. must be written serially from offset 0
 */
KFS_EXTERN rc_t CC KFileMakeGzipForWrite ( struct KFile **gz, struct KFile *file );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_gzip_ */
