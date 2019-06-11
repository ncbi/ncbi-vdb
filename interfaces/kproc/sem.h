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

#ifndef _h_kproc_sem_
#define _h_kproc_sem_

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
 * forwards
 */
struct KLock;
struct timeout_t;


/*--------------------------------------------------------------------------
 * KSemaphore
 *  a metering device
 */
typedef struct KSemaphore KSemaphore;


/* Make
 *
 *  "count" [ IN ] - initial count value
 */
KPROC_EXTERN rc_t CC KSemaphoreMake ( KSemaphore **sem, uint64_t count );


/* AddRef
 * Release
 */
KPROC_EXTERN rc_t CC KSemaphoreAddRef ( const KSemaphore *self );
KPROC_EXTERN rc_t CC KSemaphoreRelease ( const KSemaphore *self );


/* Wait
 *  block until a count becomes available
 *
 *  "lock" [ IN ] - externally acquired lock
 *
 *  "tm" [ IN, NULL OKAY ] - optional timeout where
 *  NULL means infinite timeout. a non-NULL timeout
 *  pointer with a value of 0 means non-blocking.
 */
KPROC_EXTERN rc_t CC KSemaphoreWait ( KSemaphore *self, struct KLock *lock );
KPROC_EXTERN rc_t CC KSemaphoreTimedWait ( KSemaphore *self,
    struct KLock *lock, struct timeout_t *tm );


/* Cancel
 *  signal that the count will never increase
 *
 * NB - external lock used for synchronization must be locked by current thread
 */
KPROC_EXTERN rc_t CC KSemaphoreCancel ( KSemaphore *self );


/* Signal
 *  signal that a count has become available
 *
 * NB - external lock used for synchronization must be locked by current thread
 */
KPROC_EXTERN rc_t CC KSemaphoreSignal ( KSemaphore *self );


/* Alloc
 *  allocate a count
 *  used for resource metering
 *
 *  "lock" [ IN ] - externally acquired lock
 *
 *  "count" [ IN ] - the resource count
 *
 *  "tm" [ IN, NULL OKAY ] - optional timeout where
 *  NULL means infinite timeout. a non-NULL timeout
 *  pointer with a value of 0 means non-blocking.
 */
KPROC_EXTERN rc_t CC KSemaphoreAlloc ( KSemaphore *self,
    struct KLock *lock, uint64_t count );
KPROC_EXTERN rc_t CC KSemaphoreTimedAlloc ( KSemaphore *self,
    struct KLock *lock, uint64_t count, struct timeout_t *tm );


/* Free
 *  signal that resources have become available
 *
 * NB - external lock used for synchronization must be locked by current thread
 */
KPROC_EXTERN rc_t CC KSemaphoreFree ( KSemaphore *self, uint64_t count );


/* Count
 *  request the current resource usage
 *
 *  "count" [ OUT ] - return parameter for current count
 *
 * NB - external lock used for synchronization must be locked by current thread
 */
KPROC_EXTERN rc_t CC KSemaphoreCount ( const KSemaphore *self, uint64_t *count );


#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_sem_ */
