// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#include "VDB_Fixture.hpp"

#include <vdb/vdb-priv.h>
#include <kdb/manager.h>

#include <ktst/unit_test.hpp>

using namespace std;

TEST_SUITE( VdbTextSuite );

class TextVdbFicture
{
public:
    TextVdbFicture() {}
    ~TextVdbFicture()
    {
        VDBManagerRelease( m_mgr );
    }

    void Setup( const char * input )
    {
        const KDBManager * kdb = nullptr;
        THROW_ON_RC( KDBManagerMakeText( & kdb, input, m_error, sizeof( m_error ) ) );
        THROW_ON_RC( VDBManagerMakeReadWithKDBManager( & m_mgr, kdb ));
        THROW_ON_RC( KDBManagerRelease( kdb ));
    }

    char m_error[1024];
    const VDBManager * m_mgr = nullptr;
};

FIXTURE_TEST_CASE( VdbMgr_NullSelf, TextVdbFicture )
{
    const char * input = "{}";
    const KDBManager * kdb = nullptr;
    REQUIRE_RC( KDBManagerMakeText( & kdb, input, m_error, sizeof( m_error ) ) );
    REQUIRE_RC_FAIL( VDBManagerMakeReadWithKDBManager( nullptr, kdb ));
    REQUIRE_RC( KDBManagerRelease( kdb ));
}
FIXTURE_TEST_CASE( VdbMgr_NullKdb, TextVdbFicture )
{
    REQUIRE_RC_FAIL( VDBManagerMakeReadWithKDBManager( & m_mgr, nullptr ));
}

FIXTURE_TEST_CASE( VdbMgr, TextVdbFicture )
{
    const KDBManager * kdb = nullptr;
    const char * input = "{}";
    REQUIRE_RC( KDBManagerMakeText( & kdb, input, m_error, sizeof( m_error ) ) );
    REQUIRE_RC( VDBManagerMakeReadWithKDBManager( & m_mgr, kdb ));
    REQUIRE_NOT_NULL( m_mgr );
}

FIXTURE_TEST_CASE( VdbMgr_OpenDB_Empty, TextVdbFicture )
{
    Setup("{}");
    const VDatabase * db = nullptr;
    REQUIRE_RC_FAIL( VDBManagerOpenDBRead ( m_mgr, &db, nullptr, "db" ) );
}

FIXTURE_TEST_CASE( VdbMgr_OpenDB, TextVdbFicture )
{
    Setup(R"({
        "type": "database",
        "name": "testdb",
        "metadata": {"name":""}
    })");

    const VDatabase * db = nullptr;
    REQUIRE_RC( VDBManagerOpenDBRead ( m_mgr, &db, nullptr, "testdb" ) );
    REQUIRE_NOT_NULL( db );
    REQUIRE_RC( VDatabaseRelease( db ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-vdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=VdbTextSuite(argc, argv);
    return rc;
}

}
