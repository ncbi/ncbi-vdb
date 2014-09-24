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

#ifndef _h_kfs_ramfile_
#define _h_kfs_ramfile_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KFile;

typedef struct KRamFile KRamFile;

/* -----
 * fakes file reads and writes into a buffer
 *
 * writes never fail. Acts like a sliding window from previous writes of possible
 * reads only succeed of they are from what was contained in the last write
 *
 * zero fills are put in where possible. If you write a buffer, slide the window with 
 * a write that doesn't fill the buffer, the rest will be zero not what was previously written
 *
 * The Read only form doesn;t allow writes but will reflect what was put in
 * the buffer outside of the KFile
 *
 * The write only will accept writes but can only be accessed outside of the KFile.
 *
 * The update form tries its best to give what was asked for [see above]
 */
KFS_EXTERN rc_t CC KRamFileMakeRead (const struct KFile ** self,
                                     char * buffer, size_t buffer_size);

KFS_EXTERN rc_t CC KRamFileMakeWrite (struct KFile ** self,
                                      char * buffer, size_t buffer_size);

KFS_EXTERN rc_t CC KRamFileMakeUpdate (struct KFile ** self,
                                       char * buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_ramfile_ */
