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

#ifndef _h_kproc_queue_
#define _h_kproc_queue_

#include <kproc/q-extern.h>

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct timeout_t;


/*--------------------------------------------------------------------------
 * KQueue
 *  a simple thread-safe queue structure supporting push/pop operation
 *  makes use of semaphore objects for synchronization
 */
typedef struct KQueue KQueue;

/* AddRef
 * Release
 *  ignores NULL references
 */
KQ_EXTERN rc_t CC KQueueAddRef ( const KQueue *self );
KQ_EXTERN rc_t CC KQueueRelease ( const KQueue *self );

/* Make
 * create an empty queue object
 *
 *  "capacity" [ IN ] - minimum queue length
 *  always expands to a power of 2, i.e. providing
 *  a length of 10 will result in a length of 16.
 */
KQ_EXTERN rc_t CC KQueueMake ( KQueue **q, uint32_t capacity );

/* Push
 *  add an object to the queue
 *
 *  "item" [ IN, OPAQUE ] - pointer to item being queued
 *
 *  "tm" [ IN, NULL OKAY ] - pointer to system specific timeout
 *  structure. if the queue is full, wait for indicated period
 *  of time for space to become available, or return status
 *  code indicating a timeout. when NULL and queue is full,
 *  Push will wait indefinitely. When not-NULL and value is 0,
 *  Push will time out immediately and return status code.
 */
KQ_EXTERN rc_t CC KQueuePush ( KQueue *self, const void *item, struct timeout_t *tm );

/* Pop
 *  pop an object from queue
 *
 *  "item" [ OUT, OPAQUE* ] - return parameter for popped item
 *
 *  "tm" [ IN, NULL OKAY ] - pointer to system specific timeout
 *  structure. if the queue is empty, wait for indicated period
 *  of time for an object to become available, or return status
 *  code indicating a timeout. when NULL and queue is empty,
 *  Pop will wait indefinitely. When not-NULL and value is 0,
 *  Pop will time out immediately and return status code.
 */
KQ_EXTERN rc_t CC KQueuePop ( KQueue *self, void **item, struct timeout_t *tm );

/* Sealed
 *  ask if the queue has been closed off
 *  meaning there will be no further push operations
 *
 *  NB - if "self" is NULL, the return value is "true"
 *  since a NULL queue cannot accept items via push
 */
KQ_EXTERN bool CC KQueueSealed ( const KQueue *self );

/* Seal
 *  indicate that the queue has been closed off
 *  meaning there will be no further push operations
 */
KQ_EXTERN rc_t CC KQueueSeal ( KQueue *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_queue_ */
