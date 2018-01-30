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
* Unit tests for KDirectoryResolve
*/
#include <cstring>
#include <ktst/unit_test.hpp>
#include <kfs/directory.h>
#include <kfs/impl.h>
#include <klib/out.h>
#include <kapp/args.h>

#include <stdexcept>

using namespace std;

TEST_SUITE( ResolveSuite );

class ResolveFixture {
public:
    ResolveFixture()
    : m_dir(0)
    {
        if ( KDirectoryNativeDir ( &m_dir ) != 0 )
           throw logic_error( "ResolveFixture: KDirectoryNativeDir failed");
    }
    ~ResolveFixture()
    {
        if ( m_dir )
        {
            KDirectoryRelease( m_dir );
        }
    }
    
    char m_resolved[ 4 * 1024 ];
    KDirectory *m_dir;
};

// use this in test cases based on ResolveFixture
#define REQUIRE_RESOLVE(input, result) \
    REQUIRE_RC( KDirectoryResolvePath ( m_dir, true, m_resolved, sizeof m_resolved, input ) ); \
    REQUIRE_EQ(string (m_resolved), string (result) );

FIXTURE_TEST_CASE( ResolveTest_Simple, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22/333", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_DoubleLeading, ResolveFixture )
{
    REQUIRE_RC( KDirectoryResolvePath ( m_dir, true, m_resolved, sizeof m_resolved, "//1/22/333" ) );
#ifdef WINDOWS    
    REQUIRE_RESOLVE( "//1/22/333", "//1/22/333");
#else
    REQUIRE_RESOLVE( "//1/22/333", "/1/22/333");
#endif
}

FIXTURE_TEST_CASE( ResolveTest_TrailingSlash, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22/333/", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_TrailingDotSlash, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22/333/./", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_CanonizeDoubleDot, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22/333/../333/", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_CanonizeDoubleSlashDoubleDot, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22//../333/", "/a/333" );
}

FIXTURE_TEST_CASE( ResolveTest_TrailingDoubleDot, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22/333/4444/../", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_DoubleSlash, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22//333", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_TripleSlash, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22///333", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_TrailingTripleSlash, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/333///", "/a/333" );
}

FIXTURE_TEST_CASE( ResolveTest_ShortTrailingTripleSlash, ResolveFixture )
{
#ifdef WINDOWS    
    REQUIRE_RESOLVE( "/a///", "/a/" ); // should not drop the '/' after the drive letter since it is significant
#else
    REQUIRE_RESOLVE( "/a///", "/a" ); 
#endif    
}

FIXTURE_TEST_CASE( ResolveTest_DotsAgogo, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/././22/./././333/././././/", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_DoubleDotsAgogo, ResolveFixture )
{
    REQUIRE_RESOLVE( "/a/22/333/.././../22/333/", "/a/22/333" );
}

FIXTURE_TEST_CASE( ResolveTest_LeadingDoubleSlash, ResolveFixture )
{
#ifdef WINDOWS    
    REQUIRE_RESOLVE( "//pqrst/pon1", "//pqrst/pon1" );
#else
    REQUIRE_RESOLVE( "//pqrst/pon1", "/pqrst/pon1" );
#endif    
}

FIXTURE_TEST_CASE( ResolveTest_LeadingTripleSlash, ResolveFixture )
{
#ifdef WINDOWS    
    REQUIRE_RESOLVE( "///pqrst/pon1", "//pqrst/pon1" );
#else
    REQUIRE_RESOLVE( "///pqrst/pon1", "/pqrst/pon1" );
#endif    
}

#ifndef WINDOWS    
FIXTURE_TEST_CASE( ResolveTest_Relative, ResolveFixture )
{
    REQUIRE_RC( KDirectoryResolvePath ( m_dir, false, m_resolved, sizeof m_resolved, "/" ) );
    REQUIRE_EQ( string (m_resolved).substr(0, 9), string ("../../../") );
}
#endif

//////////////////////////////////////////// Main

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

rc_t CC UsageSummary (const char * prog_name)
{
    return 0;
}

rc_t CC Usage ( const Args * args)
{
    return 0;
}

const char UsageDefaultName[] = "test-resolve";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc = ResolveSuite( argc, argv );
    return rc;
}

}
