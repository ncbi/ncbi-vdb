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

#ifndef _h_simple_SRAException_
#define _h_simple_SRAException_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifndef _h_simple_SRASourceLoc_
#include "sra-sourceloc.h"
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * SRAException
 *  a shallow, one-shot exception recording block
 */
typedef struct SRAException SRAException;
struct SRAException
{
    /* location information */
    const char *mod;
    const char *file;
    const char *ext;
    const char *func;
    uint32_t lineno;

    /* exception type id, if useful on outside */
    uint32_t xid;

    /* C error code */
    rc_t rc;

    /* message prefix size */
    uint32_t psize;

    /* message text size */
    uint32_t msize;

    /* buffer for recording error text */
    char msg [ 256 + 4096 ];
};

/* Init
 *  initialize the C structure
 *  for internal use - should always be a good pointer
 */
SIMPLE_EXTERN void CC SRAExceptionInit ( SRAException *self );

/* OK
 *  silly macro to test rc
 */
#define SRAExceptionOK( self ) \
    ( ( self ) -> rc == 0 )

/* Error
 *  a function to print textual data to exception
 */
SIMPLE_EXTERN void CC SRAException_Error ( SRAException *self,
    const SRASourceLoc *loc, const char *func, uint32_t lineno,
    uint32_t xid, rc_t rc, const char * fmt, ... );
#define SRAExceptionError( self, xid, rc, fmt, ... ) \
    SRAException_Error ( self, & s_loc, __func__, __LINE__, xid, rc, fmt, ## __VA_ARGS__ )

SIMPLE_EXTERN void CC SRAException_VError ( SRAException *self,
    const SRASourceLoc *loc, const char *func, uint32_t lineno,
    uint32_t xid, rc_t rc, const char * fmt, va_list args );
#define SRAExceptionVError( self, xid, rc, fmt, args ) \
    SRAException_VError ( self, & s_loc, __func__, __LINE__, xid, rc, fmt, args )


/* PrepareMessage
 *  performs final steps before throwing exception
 */
SIMPLE_EXTERN void CC SRAExceptionPrepareMessage ( SRAException *self );


/* canned responses
 */
void SRAException_BadInterface ( SRAException *self,
    const SRASourceLoc *loc, const char *func, uint32_t lineno );
#define SRAExceptionBadInterface( self )                            \
    SRAException_BadInterface ( self, & s_loc, __func__, __LINE__ )

void SRAException_SelfNull ( SRAException *self, rc_t ctx,
    const SRASourceLoc *loc, const char *func, uint32_t lineno );
#define SRAExceptionSelfNull( self, ctx )                           \
    SRAException_SelfNull ( self, ctx, & s_loc, __func__, __LINE__ )

#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRAException_ */
