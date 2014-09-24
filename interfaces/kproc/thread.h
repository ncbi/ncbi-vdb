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

#ifndef _h_kproc_thread_
#define _h_kproc_thread_

#ifndef _h_kproc_extern_
#include <kproc/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KThread
 *  a CPU execution thread
 */
typedef struct KThread KThread;

/* Make
 *  create and run a thread
 *
 *  "run_thread" [ IN ] - thread entrypoint
 *
 *  "data" [ IN, OPAQUE ] - user-supplied thread data
 */
KPROC_EXTERN rc_t CC KThreadMake ( KThread **t,
    rc_t ( CC * run_thread ) ( const KThread *self, void *data ), void *data );


/* AddRef
 * Release
 */
KPROC_EXTERN rc_t CC KThreadAddRef ( const KThread *self );
KPROC_EXTERN rc_t CC KThreadRelease ( const KThread *self );


/* Cancel
 *  signal the thread to finish
 */
KPROC_EXTERN rc_t CC KThreadCancel ( KThread *self );


/* Wait
 *  waits for a thread to exit
 *
 *  "status" [ OUT, NULL OKAY ] - return parameter for thread's exit code
 */
KPROC_EXTERN rc_t CC KThreadWait ( KThread *self, rc_t *status );


/* Detach
 *  allow thread to run independently of group
 */
KPROC_EXTERN rc_t CC KThreadDetach ( KThread *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_thread_ */
