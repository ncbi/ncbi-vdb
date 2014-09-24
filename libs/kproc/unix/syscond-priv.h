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

#ifndef _h_syscond_priv_
#define _h_syscond_priv_

#include <pthread.h>
#include <atomic32.h>

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
struct KCondition
{
    pthread_cond_t cond;
    atomic32_t refcount;
};

/* Init
 *  initialize an inline KCondition
 */
rc_t KConditionInit ( struct KCondition *self );

/* Destroy
 *  run destructor on inline KCondition
 */
rc_t KConditionDestroy ( struct KCondition *self );

/* DropRef
 *  manipulates reference counter
 *  returns true if last ref
 */
int KConditionDropRef ( struct KCondition const *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_syscond_priv_ */
