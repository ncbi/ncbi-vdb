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

#ifndef _h_kfs_recorder_
#define _h_kfs_recorder_

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
struct KDirectory;
struct Recorder;

KFS_EXTERN rc_t CC MakeVRecorder ( struct KDirectory * self,
                                   struct Recorder ** recorder,
                                   size_t buflen,
                                   bool append,
                                   const char * path,
                                   va_list args );

KFS_EXTERN rc_t CC MakeRecorder ( struct KDirectory * self,
                                  struct Recorder ** recorder,
                                  size_t buflen,
                                  bool append,
                                  const char * path,
                                  ... );

KFS_EXTERN rc_t CC ReleaseRecorder ( struct Recorder * self );

KFS_EXTERN rc_t CC WriteToVRecorder ( struct Recorder * self, const char * fmt, va_list args );
KFS_EXTERN rc_t CC WriteToRecorder ( struct Recorder * self, const char * fmt, ... );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_recorder_ */
