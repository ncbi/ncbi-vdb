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

#ifndef _h_kfs_buffile_
#define _h_kfs_buffile_

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
 * KFile
 */

/* MakeBufferedRead
 *  make a read-only file buffer
 *
 *  "buf" [ OUT ] - return parameter for new buffered file
 *
 *  "original" [ IN ] - source file to be buffered. must have read access
 *
 *  "bsize" [ IN ] - buffer size
 */
KFS_EXTERN rc_t CC KBufFileMakeRead ( struct KFile const ** buf,
    struct KFile const * original, size_t bsize );


/* MakeBufferedWrite
 *  make a writable file buffer
 *
 *  "buf" [ OUT ] - return parameter for new buffered file
 *
 *  "original" [ IN ] - source file to be buffered. must have read access
 *
 *  "update" [ IN ] - if true, make a read/write buffer
 *
 *  "bsize" [ IN ] - buffer size
 */
KFS_EXTERN rc_t CC KBufFileMakeWrite ( struct KFile ** buf,
    struct KFile * original, bool update, size_t bsize );




KFS_EXTERN rc_t CC KBufReadFileMakeRead ( const struct KFile ** bp,
     const struct KFile * original, size_t bsize );

KFS_EXTERN rc_t CC KBufWriteFileMakeWrite ( struct KFile ** bp,
     struct KFile * original, size_t bsize );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_buffile_ */
