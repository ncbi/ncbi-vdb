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

#include <../libs/kdb/wdatabase.h>
#include <../libs/kdb/dbmgr.h>

#include <klib/rc.h>
#include <klib/namelist.h>

#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <kdb/table.h>
#include <kdb/meta.h>
#include <kdb/index.h>
#include <kdb/namelist.h>

using namespace std;

TEST_SUITE(KWDatabaseTestSuite);

static const string ScratchDir = "./data/";

class KDatabase_Fixture
{
public:
    KDatabase_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeUpdate ( & m_mgr, m_dir ) );
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
        THROW_ON_RC( KWDatabaseMake( & m_db, m_dir, ColumnName.c_str(), nullptr, false ) );
        m_db -> mgr = KDBManagerAttach ( m_mgr );
        THROW_ON_RC( KDBManagerOpenObjectAdd ( m_mgr, & m_db -> sym) );
        KDirectoryAddRef( m_dir); // KWDatabaseMake does not call AddRef
    }

    KDirectory * m_dir = nullptr;
    KDBManager * m_mgr = nullptr;
    KDatabase * m_db = nullptr;
};


//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KWDatabase_AddRelease, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE_EQ( 1, (int)atomic32_read( & m_db -> dad . refcount ) );
    REQUIRE_RC( KDatabaseAddRef( m_db ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_db -> dad . refcount ) );
    REQUIRE_RC( KDatabaseRelease( m_db ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_db -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KWDatabase_Locked, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseLocked( m_db ) );
}

FIXTURE_TEST_CASE(KWDatabase_Exists, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseExists( m_db, kptIndex, "%s", GetName() ) );
}

FIXTURE_TEST_CASE(KWDatabase_IsAlias, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseIsAlias( m_db, kptIndex, nullptr, 0, GetName() ) );
}

FIXTURE_TEST_CASE(KWDatabase_Writable, KDatabase_Fixture)
{
    Setup( GetName() );
    rc_t rc = SILENT_RC (rcDB, rcMgr, rcAccessing, rcParam, rcInvalid);
    REQUIRE_EQ( rc, KDatabaseWritable( m_db, kptIndex, "%s", GetName() ) );
}

FIXTURE_TEST_CASE(KWDatabase_OpenManagerRead, KDatabase_Fixture)
{
    Setup( GetName() );

    const KDBManager *mgr;
    REQUIRE_RC( KDatabaseOpenManagerRead( m_db, &mgr ) );
    REQUIRE_EQ( (const KDBManager *)m_mgr, mgr );
    KDBManagerRelease( mgr );
}

FIXTURE_TEST_CASE(KWDatabase_OpenParentRead, KDatabase_Fixture)
{
    Setup( GetName() );

    struct KDatabase const *db;
    REQUIRE_RC( KDatabaseOpenParentRead( m_db, &db ) );
    REQUIRE_NULL( db );
}

FIXTURE_TEST_CASE(KWDatabase_OpenDirectoryRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KDirectory * dir = nullptr;
    REQUIRE_RC( KDatabaseOpenDirectoryRead( m_db, &dir ) );
    REQUIRE_EQ( (const KDirectory *)m_dir, dir );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

FIXTURE_TEST_CASE(KWDatabase_OpenDBRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KDatabase * subdb = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenDBRead( m_db, & subdb, "%s", "subdb" ) );
}

FIXTURE_TEST_CASE(KWDatabase_OpenTableRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KTable * t = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenTableRead( m_db, & t, "%s", "t" ) );
}

FIXTURE_TEST_CASE(KWDatabase_OpenMetadataRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KMetadata * meta = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcMetadata,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenMetadataRead( m_db, & meta ) );
}

FIXTURE_TEST_CASE(KWDatabase_OpenIndexRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KIndex * idx = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcIndex,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenIndexRead( m_db, & idx, "%s", "qq" ) );
}

FIXTURE_TEST_CASE(KWDatabase_ListDB, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBUpdate( m_mgr, & m_db, "testdb" ) );

    struct KNamelist * names;
    REQUIRE_RC( KDatabaseListDB( m_db, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KWDatabase_ListTbl, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBUpdate( m_mgr, & m_db, "testdb" ) );

    struct KNamelist * names;
    REQUIRE_RC( KDatabaseListTbl( m_db, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)1, count );
    REQUIRE( KNamelistContains( names, "SEQUENCE" ) );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KWDatabase_ListIdx, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBUpdate( m_mgr, & m_db, "testdb" ) );

    struct KNamelist * names;
    REQUIRE_RC( KDatabaseListIdx( m_db, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KWDatabase_GetPath, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBUpdate( m_mgr, & m_db, "testdb" ) );

    const char * path;
    REQUIRE_RC( KDatabaseGetPath( m_db, & path ) );
}

//TODO: non-virtual write-side only methods

FIXTURE_TEST_CASE(KWDatabase_CreateIndex, KDatabase_Fixture)
{
    KDirectoryRemove(m_dir, true, (ScratchDir + GetName()).c_str() );
    REQUIRE_RC(KDBManagerCreateDB(m_mgr, &m_db, kcmCreate, (ScratchDir + GetName()).c_str() ));

    {
        KIndex *idx;
        REQUIRE_RC(KDatabaseCreateIndex(m_db, &idx, kitText, kcmCreate, "index"));

        REQUIRE_RC(KIndexInsertText(idx, true, "a", 1));
    //    REQUIRE_RC(KIndexInsertText(idx, true, "b", 2)); causes "offset not supported" error fron klib/pack
    //    REQUIRE_RC(KIndexInsertText(idx, true, "c", 3));

        REQUIRE_RC(KIndexCommit ( idx ));

        KIndexRelease(idx);
    }

    // re-open, validate
    {
        const KIndex *idx;
        REQUIRE_RC( KDatabaseOpenIndexRead ( m_db, &idx, "index" ) );
        int64_t start_id = 0;
        uint64_t id_count = 0;
        REQUIRE_RC( KIndexFindText ( idx, "a", &start_id, &id_count, nullptr, nullptr ) );
        REQUIRE_EQ( (int64_t)1, start_id );
        REQUIRE_EQ( (uint64_t)1, id_count );
        KIndexRelease(idx);
    }

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

const char UsageDefaultName[] = "Test_KDB_KWDatabase";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KWDatabaseTestSuite(argc, argv);
    return rc;
}

}
