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

#include <simple/extern.h>
#define NO_SOURCE_LOC 1
#include "sra-exception.h"
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>

#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SRAException
 *  a shallow, one-shot exception recording block
 */

/* Init
 *  initialize the C structure
 *  for internal use - should always be a good pointer
 */
SIMPLE_EXTERN void CC SRAExceptionInit ( SRAException *self )
{
    assert ( self != NULL );

    self -> mod = self -> file = self -> ext = self -> func = "";
    self -> lineno = self -> xid = self -> rc = self -> msize = 0;
}

/* Error
 *  a function to print textual data to exception
 */
LIB_EXPORT void CC SRAException_VError ( SRAException *self,
    const SRASourceLoc *loc, const char *func, uint32_t lineno,
    uint32_t xid, rc_t xrc, const char * fmt, va_list args )
{
    rc_t rc;
    size_t psize, msize = 0;

    assert ( self != NULL );

    /* record location */
    self -> mod = loc -> mod;
    self -> file = loc -> file;
    self -> ext = loc -> ext;
    self -> func = func;
    self -> lineno = lineno;

    /* record exception codes */
    self -> xid = xid;
    self -> rc = xrc;

    /* create prefix */
#if _DEBUGGING
    rc = string_printf ( self -> msg, sizeof self -> msg, & psize,
                         "%s.%s.%s:%u: %s: "
                         , self -> mod
                         , self -> file
                         , self -> ext
                         , self -> lineno
#else
                         "%s: "
#endif
                         , self -> func
        );

    if ( rc == 0 )
    {
        rc = string_vprintf ( & self -> msg [ psize ],
            sizeof self -> msg - psize, & msize, fmt, args );
    }

    self -> psize = ( uint32_t ) psize;
    self -> msize = ( uint32_t ) ( psize + msize );

    if ( rc != 0 )
    {
        String failure;

        if ( GetRCTarget ( rc ) == rcBuffer && GetRCState ( rc ) == rcInsufficient )
        {
            memcpy ( & self -> msg [ sizeof self -> msg - 4 ], "...", 4 );
            assert ( msize != 0 );
            self -> msize = sizeof self -> msg - 1;
            return;
        }

        CONST_STRING ( & failure, "SRAException: failed to capture message" );
        self -> msize = ( uint32_t )
            string_copy ( & self -> msg [ psize ], sizeof self -> msg - psize, failure . addr, failure . size + 1 );
        self -> msize += ( uint32_t ) psize;
    }
}

LIB_EXPORT void CC SRAException_Error ( SRAException *self,
    const SRASourceLoc *loc, const char *func, uint32_t lineno,
    uint32_t xid, rc_t rc, const char * fmt, ... )
{
    va_list args;
    va_start ( args, fmt );
    SRAException_VError ( self, loc, func, lineno, xid, rc, fmt, args );
    va_end ( args );
}


/* PrepareMessage
 *  performs final steps before throwing exception
 */
LIB_EXPORT void CC SRAExceptionPrepareMessage ( SRAException *self )
{
    rc_t rc;
    size_t msize;

    assert ( self != NULL );

    if ( self -> msize == 0 )
    {
        String failure;
        CONST_STRING ( & failure, "SRAException" );
        memcpy ( self -> msg, failure . addr, failure . size + 1 );
        self -> msize = failure . size;
    }

    if ( self -> xid != 0 )
    {
        assert ( self -> msize < sizeof self -> msg );
        rc = string_printf ( & self -> msg [ self -> msize ],
                             sizeof self -> msg - self -> msize, & msize,
                             ": xid: %u", self -> xid );
        if ( rc == 0 )
            self -> msize += msize;
    }

    assert ( self -> msize < sizeof self -> msg );
    rc = string_printf ( & self -> msg [ self -> msize ],
                         sizeof self -> msg - self -> msize, & msize,
                         ": rc: %u ( %#R )", self -> rc, self -> rc );
    if ( rc == 0 )
        self -> msize += msize;

    assert ( self -> msize < sizeof self -> msg );
    self -> msg [ self -> msize ] = 0;
}


/*--------------------------------------------------------------------------
 * standardized responses
 */
void SRAException_BadInterface ( SRAException *self,
    const SRASourceLoc *loc, const char *func, uint32_t lineno )
{
    rc_t rc = RC ( rcSRA, rcNoTarg, rcResolving, rcInterface, rcBadVersion );
    SRAException_Error ( self, loc, func, lineno, 0, rc, "bad interface version" );
}

void SRAException_SelfNull ( SRAException *self, rc_t ctx,
    const SRASourceLoc *loc, const char *func, uint32_t lineno )
{
    rc_t rc = ResetRCState ( ctx, rcSelf, rcNull );
    SRAException_Error ( self, loc, func, lineno, 0, rc, "bad self reference" );
}
