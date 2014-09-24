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

#ifndef _h_kproc_impl_
#define _h_kproc_impl_

#ifndef _h_kproc_extern_
#include <kproc/extern.h>
#endif

#ifndef _h_kproc_task_
#include <kproc/task.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
typedef union KTask_vt KTask_vt;


/*--------------------------------------------------------------------------
 * KTask
 *  a deferred task abstraction
 *  specific task objects are constructed with required parameters
 *  and implement the Execute method to perform their operation
 */
struct KTask
{
    const KTask_vt *vt;
    KRefcount refcount;
    uint8_t align [ 4 ];
};

#ifndef KTASK_IMPL
#define KTASK_IMPL struct KTask
#endif

typedef struct KTask_vt_v1 KTask_vt_v1;
struct KTask_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KTASK_IMPL *self );
    rc_t ( CC * execute ) ( KTASK_IMPL * self );
    /* end minor version == 0 */
};

union KTask_vt
{
    KTask_vt_v1 v1;
};

/* Init
 *  initialize a newly allocated task object
 */
KPROC_EXTERN rc_t CC KTaskInit ( KTask *self, const KTask_vt *vt, const char *clsname, const char *name );


/* Destroy
 *  destroy task
 */
KPROC_EXTERN rc_t CC KTaskDestroy ( KTask *self, const char *clsname );


#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_impl_ */
