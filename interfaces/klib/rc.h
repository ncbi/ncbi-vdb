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

#ifndef _h_klib_rc_
#define _h_klib_rc_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#if _DEBUGGING

#ifndef _h_compiler_
#include <compiler.h>
#endif

#include <assert.h>
#endif


/*--------------------------------------------------------------------------
 * RC
 *  upon success, all functions will return code 0
 *  other codes indicate failure or additional status information
 */

#if _DEBUGGING && ! defined RECORD_RC_FILE_LINE
#define RECORD_RC_FILE_LINE 1
#elif ! defined RECORD_RC_FILE_LINE
#define RECORD_RC_FILE_LINE 0
#endif

/* actual code declarations are in <kfc/rc.h> */
#include <kfc/rc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if RC_EMIT

KLIB_EXTERN const char * CC GetRCFilename ( void );
KLIB_EXTERN const char * CC GetRCFunction ( void );
KLIB_EXTERN uint32_t CC GetRCLineno ( void );
KLIB_EXTERN rc_t CC SetRCFileFuncLine ( rc_t rc, const char *filename, const char *funcname, uint32_t lineno );
KLIB_EXTERN bool CC GetUnreadRCInfo ( rc_t *rc, const char **filename, const char **funcname, uint32_t *lineno );

#if RECORD_RC_FILE_LINE

    #if defined(__SUNPRO_CC)  &&  __SUNPRO_CC <= 0x590  &&  defined(__cplusplus)
    
        #define SET_RC_FILE_FUNC_LINE( rc ) \
            SetRCFileFuncLine ( ( rc ), __FILE__, "(N/A)", __LINE__ )
    
    #else
    
        #define SET_RC_FILE_FUNC_LINE( rc ) \
            SetRCFileFuncLine ( ( rc ), __FILE__, __func__, __LINE__ )
    
    #endif

#else

    #define SET_RC_FILE_FUNC_LINE( rc ) \
        ( rc_t ) ( rc )

#endif

/* CTX
 *  form a context from parts
 */
#define CTX( mod, targ, ctx )                                \
    ( rc_t ) ( RAW_CTX ( mod, targ, ctx ) )

/* RC
 *  form a complete return code from parts
 */
#define SILENT_RC( mod, targ, ctx, obj, state )              \
    ( rc_t ) (                                               \
        CTX ( mod, targ, ctx )    | /* 18 bits */            \
        ( ( rc_t ) ( obj ) << 6 ) | /*  8 bits */            \
        ( ( rc_t ) ( state ) ) )    /*  6 bits */

#define RC( mod, targ, ctx, obj, state )                     \
    ( rc_t ) (                                               \
    SET_RC_FILE_FUNC_LINE (                                  \
        CTX ( mod, targ, ctx )    | /* 18 bits */            \
        ( ( rc_t ) ( obj ) << 6 ) | /*  8 bits */            \
        ( ( rc_t ) ( state ) ) ) )  /*  6 bits */

/* RC_FROM_CTX
 *  form an rc but take input from existing CTX()
 */
#define RC_FROM_CTX( ctx, obj, state )                       \
    ( rc_t ) (                                               \
    SET_RC_FILE_FUNC_LINE (                                  \
        ( ctx )                   | /* 18 bits */            \
        ( ( rc_t ) ( obj ) << 6 ) | /*  8 bits */            \
        ( ( rc_t ) ( state ) ) ) )  /*  6 bits */

/* RC_EXITCODE
 *  form an rc from the desired process exit code
 */
#define RC_EXITCODE( exitcode )                             \
    ( ( rc_t )( CTX ( rcExitCode, rcProcess, rcClosing ) |  \
    ( ( rc_t )( ( ( unsigned ) (exitcode) ) & 0x7F ) ) ) )

#define IF_EXITCODE( rc, default_exit_code )                \
    ( (int) ( (int) GetRCModule(rc) == (int) rcExitCode ) ? \
      ( ((unsigned)rc) & 0x7F ) : ( default_exit_code ) )

/* ResetRCContext
 *  rewrite rc to reflect different context
 *  typically used to pass out return codes
 */
#define ResetRCContext( rc, mod, targ, ctx ) \
    ( ( ( rc ) & 0x3FFF ) | CTX ( mod, targ, ctx ) )

/* ResetRCState
 *  rewrite rc to reflect different state
 *  typically used to pass out return codes
 */
#define ResetRCState( rc, obj, state ) \
    ( ( ( rc ) & 0xFFFFFFC0 ) | ( rc_t ) ( state ) )

#endif /* __cplusplus */

/* GetRCModule
 *  extract the module portion
 */
#define GetRCModule( rc ) \
    ( enum RCModule ) ( ( ( rc ) >> 27 ) & 0x1F )

/* GetRCTarget
 *  extract the target portion
 */
#define GetRCTarget( rc ) \
    ( enum RCTarget ) ( ( ( rc ) >> 21 ) & 0x3F )

/* GetRCContext
 *  extract the context portion
 */
#define GetRCContext( rc ) \
    ( enum RCContext ) ( ( ( rc ) >> 14 ) & 0x7F )

/* GetRCObject
 *  extract the target object portion
 */
#define GetRCObject( rc ) \
    ( enum RCObject ) ( ( ( rc ) >> 6 ) & 0xFF )

/* GetRCState
 *  extract the state portion
 */
#define GetRCState( rc ) \
    ( enum RCState ) ( ( rc ) & 0x3F )


    enum RC_String_Field {
        rcf_module,
        rcf_target,
        rcf_context,
        rcf_object,
        rcf_state
    };
    struct RC_String {
        char const *text;
        size_t size;
        int value;
        int field;
    };

    KLIB_EXTERN void CC Get_RC_Strings(rc_t rc, struct RC_String [5]);

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_rc_ */
