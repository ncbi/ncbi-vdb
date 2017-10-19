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

/**
* Unit tests for klib progressbar
*/

#include <ktst/unit_test.hpp>

#include <klib/progressbar.h>
#include <klib/time.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <stdint.h>

using namespace std;

TEST_SUITE( ProgressbarTestSuite );

///////////////////////////////////////////////// text

TEST_CASE ( Progress1 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 0 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress2 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 0 ) );
    REQUIRE_RC( update_progressbar( pb, 75 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress3 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 1 ) );
    REQUIRE_RC( update_progressbar( pb, 780 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress4 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 2 ) );
    REQUIRE_RC( update_progressbar( pb, 7950 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress5 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar_stderr( &pb, 0 ) );
    REQUIRE_RC( update_progressbar( pb, 70 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress6 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar_stderr( &pb, 1 ) );
    REQUIRE_RC( update_progressbar( pb, 720 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress7 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar_stderr( &pb, 2 ) );
    REQUIRE_RC( update_progressbar( pb, 7300 ) );
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress8 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 0 ) );
    uint32_t x;
    for ( x = 0; x <= 100; ++x )
    {
        REQUIRE_RC( update_progressbar( pb, x ) );
        KSleepMs( 50 );
    }
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress9 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 1 ) );
    uint32_t x;
    for ( x = 0; x <= 1000; ++x )
    {
        REQUIRE_RC( update_progressbar( pb, x ) );
        KSleepMs( 5 );
    }
    REQUIRE_RC( destroy_progressbar( pb ) );
}

TEST_CASE ( Progress10 )
{
    progressbar * pb;
    
    REQUIRE_RC( make_progressbar( &pb, 2 ) );
    uint32_t x;
    for ( x = 0; x <= 10000; ++x )
    {
        REQUIRE_RC( update_progressbar( pb, x ) );
        KSleepMs( 1 );
    }
    REQUIRE_RC( destroy_progressbar( pb ) );
}

//////////////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-progressbar";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc = ProgressbarTestSuite( argc, argv );
    return rc;
}

}
