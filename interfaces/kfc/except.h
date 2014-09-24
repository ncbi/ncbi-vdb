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

#ifndef _h_kfc_except_
#define _h_kfc_except_

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

/* some systems ( Windows ) like to define their own versions
   of these macros. since we write to our own runtime and not
   to the OS, include any OS headers ( below ) and then undef */

#ifndef _h_os_oserror_
#include <oserror.h>
#endif

#include <stdarg.h>

#undef ERROR
#undef FAILED

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * exception-related macros
 */

/* xc_sev_t
 *  severity
 */
typedef enum xc_sev_t
{
    xc_sev_note,            /* used to create some sort of annotation    */
    xc_sev_warn,            /* used to call attention to a bad condition */
    xc_sev_fail,            /* indicates uncorrected failure             */
    xc_sev_fatal            /* indicates an unrecoverable failure        */
} xc_sev_t;

/* xc_org_t
 *  origin
 */
typedef enum xc_org_t
{
    xc_org_system,          /* event originated in OS or C libraries     */
    xc_org_internal,        /* event originated within vdb               */
    xc_org_user             /* event caused by user or application       */
} xc_org_t;


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
    xc_t xc, const char *msg, ... );
void ctx_vevent ( ctx_t ctx, uint32_t lineno,
    xc_sev_t severity, xc_org_t origin,
    xc_t xc, const char *msg, va_list args );


/* ANNOTATE
 *  make some annotation
 *  but not an error
 */
#define SYSTEM_ANNOTATE( xc, ... )                                                \
    ctx_event ( ctx, __LINE__, xc_sev_note, xc_org_system,   xc, __VA_ARGS__ )
#define INTERNAL_ANNOTATE( xc, ... )                                              \
    ctx_event ( ctx, __LINE__, xc_sev_note, xc_org_internal, xc, __VA_ARGS__ )
#define USER_ANNOTATE( xc, ... )                                                  \
    ctx_event ( ctx, __LINE__, xc_sev_note, xc_org_user,     xc, __VA_ARGS__ )


/* WARNING
 *  make an annotation
 *  record a warning as an xc_t
 */
#define SYSTEM_WARNING( xc, ... )                                                \
    ctx_event ( ctx, __LINE__, xc_sev_warn, xc_org_system,   xc, __VA_ARGS__ )
#define INTERNAL_WARNING( xc, ... )                                              \
    ctx_event ( ctx, __LINE__, xc_sev_warn, xc_org_internal, xc, __VA_ARGS__ )
#define USER_WARNING( xc, ... )                                                  \
    ctx_event ( ctx, __LINE__, xc_sev_warn, xc_org_user,     xc, __VA_ARGS__ )


/* ERROR
 *  make an annotation
 *  record an error as an xc_t
 */
#define SYSTEM_ERROR( xc, ... )                                                  \
    ctx_event ( ctx, __LINE__, xc_sev_fail, xc_org_system,   xc, __VA_ARGS__ )
#define INTERNAL_ERROR( xc, ... )                                                \
    ctx_event ( ctx, __LINE__, xc_sev_fail, xc_org_internal, xc, __VA_ARGS__ )
#define USER_ERROR( xc, ... )                                                    \
    ctx_event ( ctx, __LINE__, xc_sev_fail, xc_org_user,     xc, __VA_ARGS__ )


/* ABORT
 *  make an annotation
 *  record a fatal error as an xc_t
 */
#define SYSTEM_ABORT( xc, ... )                                                  \
    ctx_event ( ctx, __LINE__, xc_sev_fatal, xc_org_system,   xc, __VA_ARGS__ )
#define INTERNAL_ABORT( xc, ... )                                                \
    ctx_event ( ctx, __LINE__, xc_sev_fatal, xc_org_internal, xc, __VA_ARGS__ )
#define USER_ABORT( xc, ... )                                                    \
    ctx_event ( ctx, __LINE__, xc_sev_fatal, xc_org_user,     xc, __VA_ARGS__ )


/* UNIMPLEMENTED
 *  marks an error when the code is simply not finished
 */
#define UNIMPLEMENTED()                                 \
    INTERNAL_ERROR ( xcUnimplemented, "" )


/* FAILED
 *  a test of rc within ctx_t
 */
#define FAILED()                                        \
    ( ctx -> rc != 0 )


/* TRY
 *  another C language "try" macro
 */
#define TRY( expr )                                     \
    expr;                                               \
    if ( ! FAILED () )


/* CATCH
 *  attempts to catch rc on certain types
 */
bool ctx_xc_isa ( ctx_t ctx, xc_t xc );
bool ctx_xobj_isa ( ctx_t ctx, xobj_t xo );
bool ctx_xstate_isa ( ctx_t ctx, xstate_t xs );

#define CATCH( xc )                                     \
    else if ( ctx_xc_isa ( ctx, xc ) )
#define CATCH_OBJ( xo )                                 \
    else if ( ctx_xobj_isa ( ctx, xo ) )
#define CATCH_STATE( xs )                               \
    else if ( ctx_xstate_isa ( ctx, xs ) )
#define CATCH_ALL()                                     \
    else


/* ON_FAIL
 *  reverses TRY logic
 *  generally used for less-structured code,
 *  e.g. ON_FAIL ( x ) return y;
 */
#define ON_FAIL( expr )                                 \
    expr;                                               \
    if ( FAILED () )


/* CLEAR
 *  clears annotation and error
 *  used from within CATCH handler
 */
void ctx_clear ( ctx_t ctx );
#define CLEAR()                                         \
    ctx_clear ( ctx )


/* WHAT
 *  retrieves current message
 *  if no annotation exists, returns empty string
 */
const char * ctx_what ( ctx_t ctx );
#define WHAT()                                          \
    ctx_what ( ctx )


#ifdef __cplusplus
}
#endif


#endif /* _h_kfc_except_ */
