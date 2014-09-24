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

#define SRC_LOC_DEFINED 1

#include <kfc/extern.h>
#include <kfc/rsrc.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/rc.h>
#include <kfc/xcdefs.h>
#include <kfc/xc.h>
#include <kfc/tstate.h>

#include <klib/text.h>
#include <klib/printf.h>
#include <kproc/thread.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sysalloc.h>


/* event
 *  create a thread event
 *
 *  "lineno" [ IN ] - the source line where event was created
 *  "severity" [ IN ] - severity of the event
 *  "origin" [ IN ] - origin of the event
 *  "xc" [ IN ] - type of the event
 *  "msg" [ IN, VARARG ] - instance data to be recorded on the event
 */
void ctx_event ( ctx_t ctx, uint32_t lineno,
    xc_sev_t severity, xc_org_t origin,
    xc_t xc, const char *msg, ... )
{
    va_list args;
    va_start ( args, msg );

    if ( ctx != NULL )
        KThreadStateEvent ( ctx -> rsrc -> thread, ctx, lineno, severity, origin, xc, msg, args );

    va_end ( args );
}

void ctx_vevent ( ctx_t ctx, uint32_t lineno,
    xc_sev_t severity, xc_org_t origin,
    xc_t xc, const char *msg, va_list args )
{
    if ( ctx != NULL )
        KThreadStateEvent ( ctx -> rsrc -> thread, ctx, lineno, severity, origin, xc, msg, args );
}


/* CATCH
 *  attempts to catch rc on certain types
 */
bool ctx_xc_isa ( ctx_t ctx, xc_t xc )
{
    if ( ctx != NULL && ctx -> evt != NULL )
        return KThreadStateIsXCErr ( ctx -> rsrc -> thread, ctx, xc );
    return false;
}

bool ctx_xobj_isa ( ctx_t ctx, xobj_t xo )
{
    if ( ctx != NULL && ctx -> evt != NULL )
        return KThreadStateIsXCObj ( ctx -> rsrc -> thread, ctx, xo );
    return false;
}

bool ctx_xstate_isa ( ctx_t ctx, xstate_t xs )
{
    if ( ctx != NULL && ctx -> evt != NULL )
        return KThreadStateIsXCState ( ctx -> rsrc -> thread, ctx, xs );
    return false;
}


/* clear
 *  clears annotation and error
 *  used from within CATCH handler
 */
void ctx_clear ( ctx_t ctx )
{
    if ( ctx != NULL && ctx -> evt != NULL )
        KThreadStateClearEvents ( ctx -> rsrc -> thread, ctx );
}


/* what
 *  retrieves current message
 *  if no annotation exists, returns empty string
 */
const char * ctx_what ( ctx_t ctx )
{
    if ( ctx != NULL && ctx -> evt != NULL )
        return KThreadStateGetMessage ( ctx -> rsrc -> thread, ctx );

    return "";
}
