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
* Unit tests for read-side KIndex
*/

#include <ktst/unit_test.hpp>

#include <klib/rc.h>
#include <kdb/index.h>
#include <kdb/database.h>
#include <kdb/kdb-priv.h>

#include <../libs/kdb/index-priv.h>
#include <../libs/kdb/rdbmgr.h>

using namespace std;

TEST_SUITE(KDBRIndexTestSuite);

class KIndex_Fixture
{
public:
    KIndex_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeRead ( & m_mgr, m_dir ) );
    }
    ~KIndex_Fixture()
    {
        KIndexRelease( m_idx );
        KDatabaseRelease( m_db );
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }

    void Open( const char * dbpath, const char * name )
    {
        THROW_ON_RC( KDBManagerOpenDBRead( m_mgr, & m_db, "%s", dbpath ) );
        THROW_ON_RC( KDatabaseOpenIndexRead ( m_db, &m_idx, name ) );
    }

    KDirectory * m_dir = nullptr;
    const KDBManager * m_mgr = nullptr;
    const KDatabase * m_db = nullptr;
    const KIndex * m_idx = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KRIndex_AddRelease, KIndex_Fixture)
{
    Open( "testdb", "index" );

    REQUIRE_EQ( 1, (int)atomic32_read( & m_idx -> dad . refcount ) );
    REQUIRE_RC( KIndexAddRef( m_idx ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_idx -> dad . refcount ) );
    REQUIRE_RC( KIndexRelease( m_idx ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_idx -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRIndex_Locked, KIndex_Fixture)
{
    Open( "testdb", "index" );
    REQUIRE( ! KIndexLocked( m_idx ) );
}

FIXTURE_TEST_CASE(KRIndex_Version, KIndex_Fixture)
{
    Open( "testdb", "index" );

    uint32_t version = 0;
    REQUIRE_RC( KIndexVersion( m_idx, & version ) );
    REQUIRE_EQ( (uint32_t)4, version );
}

FIXTURE_TEST_CASE(KRIndex_Type, KIndex_Fixture)
{
    Open( "testdb", "index" );

    KIdxType type = 1;
    REQUIRE_RC( KIndexType( m_idx, & type ) );
    REQUIRE_EQ( (KIdxType)0, type );
}

FIXTURE_TEST_CASE(KRIndex_ConsistencyCheck, KIndex_Fixture)
{
    Open( "testdb", "index" );

    int64_t start_id = 0;
    uint64_t id_range = 0;
    uint64_t num_keys = 0;
    uint64_t num_rows = 0;
    uint64_t num_holes = 0;
    const uint32_t level = 1; // levels higher than 1 produce RC(libs/kdb/ptrieval-v2.c:381:KPTrieIndexCheckConsistency_v2 rcDB,rcIndex,rcValidating,rcSelf,rcCorrupt)
    REQUIRE_RC( KIndexConsistencyCheck ( m_idx, level, & start_id, & id_range, & num_keys, & num_rows, & num_holes ) );
    REQUIRE_EQ( (int64_t)1, start_id );
    REQUIRE_EQ( (uint64_t)0, id_range );
    REQUIRE_EQ( (uint64_t)1, num_keys );
    REQUIRE_EQ( (uint64_t)0, num_rows );
    REQUIRE_EQ( (uint64_t)0, num_holes );
}

FIXTURE_TEST_CASE(KRIndex_FindText, KIndex_Fixture)
{
    Open( "testdb", "index" );

    int64_t start_id = 0;
    uint64_t id_count = 0;

    REQUIRE_RC( KIndexFindText ( m_idx, "a", & start_id, & id_count, nullptr, nullptr ) );
    REQUIRE_EQ( (int64_t)1, start_id );
    REQUIRE_EQ( (uint64_t)1, id_count );
}


static uint64_t fat_called = 0;
static int64_t fat_start_id = 0;
static uint64_t fat_id_count = 0;
static
rc_t CC
findAllTextCallback (int64_t p_startId, uint64_t p_count, void*)
{
    ++ fat_called;
    fat_start_id = p_startId;
    fat_id_count = p_count;
    return 0;
}

FIXTURE_TEST_CASE(KRIndex_FindAllText, KIndex_Fixture)
{
    Open( "testdb", "index" );

    REQUIRE_RC( KIndexFindAllText ( m_idx, "a", findAllTextCallback, nullptr ) );
    REQUIRE_EQ( (uint64_t)1, fat_called );
    REQUIRE_EQ( (int64_t)1, fat_start_id );
    REQUIRE_EQ( (uint64_t)1, fat_id_count );
}

FIXTURE_TEST_CASE(KRIndex_ProjectText, KIndex_Fixture)
{
    Open( "testdb", "index" );

    int64_t start_id = 0;
    uint64_t id_count = 0;
    char key[1024];
    size_t actsize = 0;
    rc_t rc = SILENT_RC(rcDB,rcIndex,rcProjecting,rcIndex,rcIncorrect);
    REQUIRE_EQ( rc, KIndexProjectText ( m_idx, 1, & start_id, & id_count, key, sizeof key, &actsize ) );
}


FIXTURE_TEST_CASE(KRIndex_ProjectAllText, KIndex_Fixture)
{
    Open( "testdb", "index" );

    auto f = [] (int64_t p_startId, uint64_t p_count, const char *key, void*) -> rc_t { return 0; };

    rc_t rc = SILENT_RC(rcDB,rcIndex,rcProjecting,rcIndex,rcIncorrect);
    REQUIRE_EQ( rc, KIndexProjectAllText ( m_idx, 1, f, nullptr ) );
}

FIXTURE_TEST_CASE(KRIndex_FindU64, KIndex_Fixture)
{
    Open( "testdb", "index" );

    uint64_t key[1024];
    size_t key_size = sizeof( key );
    int64_t start_id = 0;
    uint64_t id_count = 0;

    rc_t rc = SILENT_RC(rcDB, rcIndex, rcSelecting, rcNoObj, rcUnknown);
    REQUIRE_EQ( rc, KIndexFindU64 ( m_idx, 0, key, & key_size, & start_id, & id_count ) );
}

FIXTURE_TEST_CASE(KRIndex_FindAllU64, KIndex_Fixture)
{
    Open( "testdb", "index" );

    auto f = [] ( uint64_t key, uint64_t key_size, int64_t start_id, uint64_t id_count, void *data ) -> rc_t { return 0; };

    rc_t rc = SILENT_RC(rcDB, rcIndex, rcSelecting, rcNoObj, rcUnknown);
    REQUIRE_EQ( rc, KIndexFindAllU64 ( m_idx, 0, f, nullptr ) );
}

FIXTURE_TEST_CASE(KRIndex_SetMaxRowId, KIndex_Fixture)
{
    Open( "testdb", "index" );

    KIndexSetMaxRowId ( m_idx, 100 );
    // no easy way to check the outcome
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

const char UsageDefaultName[] = "Test_KDB_RIndex";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBRIndexTestSuite(argc, argv);
    return rc;
}

}
