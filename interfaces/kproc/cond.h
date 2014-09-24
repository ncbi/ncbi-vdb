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

#ifndef _h_kproc_cond_
#define _h_kproc_cond_

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
 * KCondition
 *  a POSIX-style condition object
 *  ( requires an external lock object )
 *
 *  usage: the user first acquires an external lock. then, depending upon
 *  the operation, will either test for a condition or establish it, where
 *  the former involves the potential to wait for a signal and the latter
 *  to generate a signal using the external lock for blocking.
 */
typedef struct KCondition KCondition;


/* Make
 *  create a condition
 */
KPROC_EXTERN rc_t CC KConditionMake ( KCondition **cond );


/* AddRef
 * Release
 */
KPROC_EXTERN rc_t CC KConditionAddRef ( const KCondition *self );
KPROC_EXTERN rc_t CC KConditionRelease ( const KCondition *self );


/* Wait
 *  block on external lock until signalled
 */
KPROC_EXTERN rc_t CC KConditionWait ( KCondition *self, struct KLock *lock );
KPROC_EXTERN rc_t CC KConditionTimedWait ( KCondition *self, struct KLock *lock, struct timeout_t *tm );


/* Signal
 *  signal waiting threads
 *  awaken at most a single thread
 *
 * NB - external lock used for synchronization must be locked by current thread
 */
KPROC_EXTERN rc_t CC KConditionSignal ( KCondition *self );


/* Broadcast
 *  signal waiting threads
 *  awaken all waiting thread
 *
 * NB - external lock used for synchronization must be locked by current thread
 */
KPROC_EXTERN rc_t CC KConditionBroadcast ( KCondition *self );

#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_cond_ */
