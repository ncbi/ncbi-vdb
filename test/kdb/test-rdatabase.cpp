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
* Unit tests for read-side KDatabase
*/

#include <ktst/unit_test.hpp>

#include <../libs/kdb/rdatabase.h>
#include <../libs/kdb/dbmgr-priv.h>

#include <klib/rc.h>

#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <kdb/table.h>
#include <kdb/meta.h>


using namespace std;

TEST_SUITE(KRDatabaseTestSuite);

static const string ScratchDir = "./data/";

class KDatabase_Fixture
{
public:
    KDatabase_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeRead ( & m_mgr, m_dir ) );
    }
    ~KDatabase_Fixture()
    {
        KDatabaseRelease( m_db );
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }
    void Setup( const string testName )
    {
        const string ColumnName = ScratchDir + testName;
        THROW_ON_RC( KDatabaseMake( & m_db, m_dir, ColumnName.c_str() ) );
        m_db -> mgr = KDBManagerAttach ( m_mgr );
        KDirectoryAddRef( m_dir); // KDatabaseMake does not call AddRef
    }

    KDirectory * m_dir = nullptr;
    const KDBManager * m_mgr = nullptr;
    KDatabase * m_db = nullptr;
};


//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KRDatabase_AddRelease, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE_EQ( 1, (int)atomic32_read( & m_db -> dad . refcount ) );
    REQUIRE_RC( KDatabaseAddRef( m_db ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_db -> dad . refcount ) );
    REQUIRE_RC( KDatabaseRelease( m_db ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_db -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRDatabase_Locked, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseLocked( m_db ) );
}

FIXTURE_TEST_CASE(KRDatabase_Exists, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseExists( m_db, kptIndex, "%s", GetName() ) );
}

FIXTURE_TEST_CASE(KRDatabase_IsAlias, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseIsAlias( m_db, kptIndex, nullptr, 0, GetName() ) );
}

FIXTURE_TEST_CASE(KRDatabase_Writable, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE_EQ( -1, (int)KDatabaseWritable( m_db, kptIndex, "%s", GetName() ) ); // "TBD" for read side
}

FIXTURE_TEST_CASE(KDatabase_OpenManagerRead, KDatabase_Fixture)
{
    Setup( GetName() );

    struct KDBManager const *mgr;
    REQUIRE_RC( KDatabaseOpenManagerRead( m_db, &mgr ) );
    REQUIRE_EQ( m_mgr, mgr );
    KDBManagerRelease( mgr );
}

FIXTURE_TEST_CASE(KDatabase_OpenParentRead, KDatabase_Fixture)
{
    Setup( GetName() );

    struct KDatabase const *db;
    REQUIRE_RC( KDatabaseOpenParentRead( m_db, &db ) );
    REQUIRE_NULL( db );
}

FIXTURE_TEST_CASE(KRDatabase_OpenDirectoryRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KDirectory * dir = nullptr;
    REQUIRE_RC( KDatabaseOpenDirectoryRead( m_db, &dir ) );
    REQUIRE_EQ( (const KDirectory *)m_dir, dir );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenDBRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KDatabase * subdb = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenDBRead( m_db, & subdb, "%s", "subdb" ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenTableRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KTable * t = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenTableRead( m_db, & t, "%s", "t" ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenMetadataRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KMetadata * meta = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcMetadata,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenMetadataRead( m_db, & meta ) );
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

const char UsageDefaultName[] = "Test_KDB_KRDatabase";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KRDatabaseTestSuite(argc, argv);
    return rc;
}

}
