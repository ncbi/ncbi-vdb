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
* Unit tests for a file-system-type path represented as queue of strings
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/path.hpp"

#include <vfs/manager.h>
#include <vfs/path.h>

#include <cstdarg>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextPathTestSuite);

TEST_CASE(KDBTextPath_fromEmptyString)
{
    Path path( string("") );
    REQUIRE( path.empty() );
}
TEST_CASE(KDBTextPath_fromString)
{
    Path path( string("a/bb/cccc") );
    REQUIRE( ! path.empty() );
    REQUIRE_EQ( string("a"), path.front() );
    path.pop();
    REQUIRE_EQ( string("bb"), path.front() );
    path.pop();
    REQUIRE_EQ( string("cccc"), path.front() );
    path.pop();
    REQUIRE( path.empty() );
}
TEST_CASE(KDBTextPath_fromFmtArgs)
{
    auto fn = [] ( const char *fmt, ... ) -> Path
    {
        va_list args;
        va_start ( args, fmt );
        return Path( fmt, args );
    };
    Path path = fn( "%s", "a/bb/cccc" );
    REQUIRE( ! path.empty() );
    REQUIRE_EQ( string("a"), path.front() );
    path.pop();
    REQUIRE_EQ( string("bb"), path.front() );
    path.pop();
    REQUIRE_EQ( string("cccc"), path.front() );
    path.pop();
    REQUIRE( path.empty() );
}

TEST_CASE(KDBTextPath_fromVPath)
{
    VPath * vpath;
    VFSManager * vfs;
    REQUIRE_RC( VFSManagerMake ( & vfs ) );
    REQUIRE_RC( VFSManagerMakePath ( vfs, & vpath, "%s", "a/bb/cccc" ) );
    REQUIRE_RC( VFSManagerRelease( vfs ) );

    Path path ( vpath );
    REQUIRE_RC( VPathRelease( vpath ) );

    REQUIRE( ! path.empty() );
    REQUIRE_EQ( string("a"), path.front() );
    path.pop();
    REQUIRE_EQ( string("bb"), path.front() );
    path.pop();
    REQUIRE_EQ( string("cccc"), path.front() );
    path.pop();
    REQUIRE( path.empty() );
}
//////////////////////////////////////////// Main
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

const char UsageDefaultName[] = "Test_KDBText_Path";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextPathTestSuite(argc, argv);
    return rc;
}

}
