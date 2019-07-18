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

#ifndef _h_kfs_remove_file_task_
#define _h_kfs_remove_file_task_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KTask;
struct KDirectory;


/*--------------------------------------------------------------------------
 * Make
 *  make a task that will attempt to remove the named entry when run
 *
 *  "task" [ OUT ] - return parameter for task
 *
 *  "dir" [ IN ] - directory that will receive Remove message
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *     character set denoting target object
 */
KFS_EXTERN rc_t CC KRemoveFileTaskMake ( struct KTask ** task,
    struct KDirectory * dir, const char * path, ... );
KFS_EXTERN rc_t CC KRemoveFileTaskVMake ( struct KTask ** task,
    struct KDirectory * dir, const char * path, va_list args );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_remove_file_task_ */
