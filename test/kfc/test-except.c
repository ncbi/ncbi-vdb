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
#include <kfc/extern.h>
#include <kfc/tstate.h>
#include <kfc/rsrc.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/rc.h>
#include <kfc/xc.h>
#include <kfc/xcdefs.h>
#include <klib/text.h>

#include <sysalloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static
void immediate_error_no_clean ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcExe, rcFunction, rcValidating );
    UNIMPLEMENTED ();
}

static
void child_error_no_clean ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcExe, rcFunction, rcValidating );
    immediate_error_no_clean ( ctx );
}

static
void immediate_error_clean ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcExe, rcFunction, rcValidating );
    UNIMPLEMENTED ();
    CLEAR ();
}


static
void run_test ( KRsrc * rsrc, ctx_t ctx, void ( * test ) ( ctx_t ctx ) )
{
    rsrc -> thread = KProcMgrMakeThreadState ( ( void * ) 1 );

    ( * test ) ( ctx );

    KThreadStateWhack ( rsrc -> thread );
    rsrc -> thread = NULL;
}

static
void test_except ( KRsrc * rsrc, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcExe, rcFunction, rcValidating );
    run_test ( rsrc, ctx, immediate_error_no_clean );
    run_test ( rsrc, ctx, child_error_no_clean );
    ON_FAIL ( run_test ( rsrc, ctx, immediate_error_clean ) ) return;
}

int main ( int argc, char * argv [] )
{
    int status;
    KRsrc rsrc;
    KCtx local_ctx, * ctx = & local_ctx;
    DECLARE_FUNC_LOC ( rcExe, rcFunction, rcValidating ); /* ! */

    memset ( & rsrc, 0, sizeof rsrc );

    memset ( & local_ctx, 0, sizeof local_ctx );
    local_ctx . rsrc = & rsrc;
    local_ctx . loc = & s_func_loc;

    TRY ( test_except ( & rsrc, ctx ) )
    {
        fprintf ( stderr, "%s - succeeded\n", argv [ 0 ] );
        status = 0;
    }
    CATCH_ALL ()
    {
        fprintf ( stderr, "%s - failed\n", argv [ 0 ] );
        status = 1;
    }

    return status;
}

#if WINDOWS
int CC wmain ( int argc, wchar_t * wargv [] )
{
    int i, status;
    char ** argv = malloc ( argc * sizeof * argv );
    for ( i = 0; i < argc; ++ i )
    {
        size_t src_size, dst_size;
        uint32_t len = wchar_cvt_string_measure ( wargv [ i ], & src_size, & dst_size );
        char * dst = malloc ( dst_size + 1 );
        wchar_cvt_string_copy ( dst, dst_size + 1, wargv [ i ], src_size );
        argv [ i ] = dst;
    }

    status = main ( argc, argv );

    for ( i = 0; i < argc; ++ i )
        free ( argv [ i ] );
    free ( argv );

    return status;
}
#endif
