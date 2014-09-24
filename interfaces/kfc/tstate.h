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

#ifndef _h_kfc_tstate_
#define _h_kfc_tstate_

#ifndef _h_kfc_extern_
#include <kfc/extern.h>
#endif

#ifndef _h_kfc_except_
#include <kfc/except.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KProcMgr;


/*--------------------------------------------------------------------------
 * KThreadState
 *  exection state belonging to the current thread
 */
typedef struct KThreadState KThreadState;


/* MakeThreadState
 *  creates state for a newly created thread
 *  called from the new thread
 */
KThreadState * KProcMgrMakeThreadState ( struct KProcMgr const * self );


/* Whack
 */
void KThreadStateWhack ( KThreadState * self );


/* CaptureEvent
 *  records an event from the exception mechanism
 */
void KThreadStateEvent ( KThreadState * self, ctx_t ctx,
    uint32_t lineno, xc_sev_t severity, xc_org_t origin,
    xc_t xc, const char * msg, va_list args );


/* ClearEvents
 *  clears events from a particular point in the callchain
 *  down toward lower points.
 */
void KThreadStateClearEvents ( KThreadState * self, ctx_t ctx );


/* GetMessage
 *  retrieve current event message
 */
const char * KThreadStateGetMessage ( const KThreadState * self, ctx_t ctx );


/* IsXCErr
 */
bool KThreadStateIsXCErr ( const KThreadState * self, ctx_t ctx, xc_t xc );

/* IsXCObj
 */
bool KThreadStateIsXCObj ( const KThreadState * self, ctx_t ctx, xobj_t xo );

/* IsXCState
 */
bool KThreadStateIsXCState ( const KThreadState * self, ctx_t ctx, xstate_t xs );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_tstate_ */
