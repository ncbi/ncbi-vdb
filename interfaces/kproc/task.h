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

#ifndef _h_kproc_task_
#define _h_kproc_task_

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


/*--------------------------------------------------------------------------
 * KTask
 *  a deferred task abstraction
 *  specific task objects are constructed with required parameters
 *  and implement the Execute method to perform their operation
 */
typedef struct KTask KTask;


/* AddRef
 * Release
 */
KPROC_EXTERN rc_t CC KTaskAddRef ( const KTask *self );
KPROC_EXTERN rc_t CC KTaskRelease ( const KTask *self );
KPROC_EXTERN rc_t CC KTaskDestroy ( KTask *self, const char *clsname );

/* Execute
 *  perform deferred operation
 */
KPROC_EXTERN rc_t CC KTaskExecute ( KTask *self );

/*--------------------------------------------------------------------------
 * KTaskTicket
 *  an opaque object holding task identification
 */
typedef struct KTaskTicket KTaskTicket;
struct KTaskTicket
{
    uint64_t info [ 2 ];
};


#ifdef __cplusplus
}
#endif

#endif /* _h_kproc_task_ */
