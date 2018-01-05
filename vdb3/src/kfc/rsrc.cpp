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

#include <kfc/rsrc.hpp>
#include <kfc/callstk.hpp>
#include <kfc/except.hpp>
#include <kfc/caps.hpp>
#include <kfc/array.hpp>
#include <kfc/string.hpp>
#include "pmemmgr.hpp"
#include "ptimemgr.hpp"
#include "pfdmgr.hpp"
#include "plogger.hpp"

#if UNIX
#include <kfc/fd.hpp>
#else
#error "unsupported target platform"
#endif

#include <stdio.h>

namespace vdb3
{

    __thread Rsrc const * rsrc;

    RsrcBase :: ~ RsrcBase ()
    {
        if ( rsrc == ( const Rsrc * ) this )
            rsrc = 0;
    }

    Rsrc :: Rsrc ( rcaps_t mgrs )
    {
        if ( rsrc == 0 )
        {
            FUNC_ENTRY ();
            CONST_THROW ( xc_program_state_violation, "at top of call stack" );
        }

        mmgr = rsrc -> mmgr;

        if ( ( mgrs & RCAP_TMMGR ) != 0 )
            tmmgr = rsrc -> tmmgr;

        if ( ( mgrs & RCAP_FDMGR ) != 0 )
            fdmgr = rsrc -> fdmgr;

        if ( ( mgrs & RCAP_LOG ) != 0 )
            log = rsrc -> log;

        if ( ( mgrs & RCAP_ERR ) != 0 )
            err = rsrc -> err;
    }


    Rsrc :: Rsrc ( const Rsrc & rsrc )
        : mmgr ( rsrc . mmgr )
        , tmmgr ( rsrc . tmmgr )
        , fdmgr ( rsrc . fdmgr )
        , log ( rsrc . log )
        , err ( rsrc . err )
    {
    }

    void Rsrc :: operator = ( const Rsrc & rsrc )
    {
        mmgr = rsrc . mmgr;
        tmmgr = rsrc . tmmgr;
        fdmgr = rsrc . fdmgr;
        log = rsrc . log;
        err = rsrc . err;
    }

    Rsrc :: ~ Rsrc ()
    {
    }

    Rsrc :: Rsrc ( const MemMgr & pmmgr, const char * ident )
        : mmgr ( pmmgr )
    {
        // one-shot latch
        if ( rsrc != 0 )
        {
            assert ( callstk != 0 );
            FUNC_ENTRY ();
            CONST_THROW ( xc_program_state_violation, "not at top of call stack" );
        }
        rsrc = this;

        try
        {
            // create obligatory resources
            tmmgr = PrimordTimeMgr :: make_primordial ();
            fdmgr = PrimordFDMgr :: make_primordial ();
            log = plogger_t :: make ( ident );
#if UNIX
            FileDesc fd2 = fdmgr . make ( 2, CAP_WRITE );
            err = Stream ( fd2 );
#endif
        }
        catch ( exception & x )
        {
            NULTermString what = x . what ();
            fprintf ( stderr, "ERROR: %s:\n", ( const char * ) what );
#if _DEBUGGING
            NULTermString stk = x . stack_trace ();
            fprintf ( stderr, "%s\n", ( const char * ) stk );
#endif
        }
    }

    TopRsrc :: TopRsrc ( const char * ident )
        : Rsrc ( PrimordMemMgr :: make_primordial (), ident )
    {
    }

}
