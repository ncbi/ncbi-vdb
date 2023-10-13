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
* Unit tests for write-side KTable
*/

#include <ktst/unit_test.hpp>

#include <../libs/kdb/wtable.h>
#include <../libs/kdb/dbmgr.h>

#include <klib/rc.h>
#include <klib/namelist.h>

#include <kdb/manager.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <kdb/index.h>
#include <kdb/database.h>
#include <kdb/namelist.h>
#include <kdb/kdb-priv.h>

using namespace std;

TEST_SUITE(KWTableTestSuite);

static const string ScratchDir = "./data/";

class KTable_Fixture
{
public:
    KTable_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeUpdate ( & m_mgr, m_dir ) );
    }
    ~KTable_Fixture()
    {
        KTableRelease( m_tbl );
        KDatabaseRelease( m_db );
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }
    void Setup( const string testName )
    {
        const string ColumnName = ScratchDir + testName;
        THROW_ON_RC( KWTableMake( & m_tbl, m_dir, ColumnName.c_str(), nullptr, true ) );
        KDirectoryAddRef( m_dir); // KWTableMake does not call AddRef

        THROW_ON_RC( KDBManagerOpenObjectAdd ( m_mgr, & m_tbl -> sym) );
        m_tbl -> mgr = KDBManagerAttach ( m_mgr );
    }
    void Open( const char * dbname, const char * tablename )
    {
        THROW_ON_RC( KDBManagerOpenDBUpdate( m_mgr, & m_db, dbname ) );
        THROW_ON_RC( KDatabaseOpenTableRead( m_db, (const KTable**)& m_tbl, tablename ) );
    }
    KDirectory * m_dir = nullptr;
    KDBManager * m_mgr = nullptr;
    KDatabase * m_db = nullptr;
    KTable * m_tbl = nullptr;
};


//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KWTable_AddRelease, KTable_Fixture)
{
    Setup( GetName() );

    REQUIRE_EQ( 1, (int)atomic32_read( & m_tbl -> dad . refcount ) );
    REQUIRE_RC( KTableAddRef( m_tbl ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_tbl -> dad . refcount ) );
    REQUIRE_RC( KTableRelease( m_tbl ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_tbl -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KWTable_Locked, KTable_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KTableLocked( m_tbl ) );
}

FIXTURE_TEST_CASE(KWTable_Exists, KTable_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KTableExists( m_tbl, kptIndex, "%s", GetName() ) );
}

FIXTURE_TEST_CASE(KWTable_IsAlias, KTable_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KTableIsAlias( m_tbl, kptIndex, nullptr, 0, GetName() ) );
}

FIXTURE_TEST_CASE(KWTable_Writable, KTable_Fixture)
{
    Setup( GetName() );

    REQUIRE_RC_FAIL( KTableWritable( m_tbl, kptIndex, "%s", GetName() ) );
}

FIXTURE_TEST_CASE(KWTable_OpenManagerRead, KTable_Fixture)
{
    Setup( GetName() );

    const KDBManager *mgr;
    REQUIRE_RC( KTableOpenManagerRead( m_tbl, &mgr ) );
    REQUIRE_EQ( (const KDBManager *)m_mgr, mgr );
    REQUIRE_RC( KDBManagerRelease( mgr ) );
}

FIXTURE_TEST_CASE(KWTable_OpenParentRead, KTable_Fixture)
{
    Setup( GetName() );

    struct KDatabase const *db;
    REQUIRE_RC( KTableOpenParentRead( m_tbl, &db ) );
    REQUIRE_NULL( db );
}

FIXTURE_TEST_CASE(KWTable_HasRemoteData, KTable_Fixture)
{
    Setup( GetName() );

    REQUIRE( KTableHasRemoteData( m_tbl ) ); // always true for write-side
}

FIXTURE_TEST_CASE(KWTable_OpenDirectoryRead, KTable_Fixture)
{
    Setup( GetName() );
    const KDirectory * dir = nullptr;
    REQUIRE_RC( KTableOpenDirectoryRead( m_tbl, &dir ) );
    REQUIRE_EQ( (const KDirectory *)m_dir, dir );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

FIXTURE_TEST_CASE(KWTable_OpenColumnRead, KTable_Fixture)
{
    Setup( GetName() );
    const KColumn * col = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KTableOpenColumnRead ( m_tbl, &col, "%s", "col" ) );
    REQUIRE_NULL( col );
}

FIXTURE_TEST_CASE(KWTable_OpenMetadataRead, KTable_Fixture)
{
    Setup( GetName() );
    const KMetadata * meta = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcMetadata,rcNotFound );
    REQUIRE_EQ( rc, KTableOpenMetadataRead ( m_tbl, & meta ) );
}

FIXTURE_TEST_CASE(KWTable_OpenIndexRead, KTable_Fixture)
{
    Setup( GetName() );
    const KIndex * idx = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcIndex,rcNotFound );
    REQUIRE_EQ( rc, KTableOpenIndexRead ( m_tbl, & idx, "idx" ) );
}

FIXTURE_TEST_CASE(KWTable_GetPath, KTable_Fixture)
{
    Setup( GetName() );
    const char * path = nullptr;
    REQUIRE_RC( KTableGetPath ( m_tbl, & path ) );
    REQUIRE_EQ( ScratchDir + GetName(), string( path ) );
}

FIXTURE_TEST_CASE(KWTable_GetName, KTable_Fixture)
{
    Setup( GetName() );
    const char * name = nullptr;
    REQUIRE_RC( KTableGetName ( m_tbl, & name ) );
    REQUIRE_EQ( string( GetName() ), string( name ) );
}

FIXTURE_TEST_CASE(KWTable_ListCol, KTable_Fixture)
{
    Open( "testdb", "SEQUENCE" );

    struct KNamelist * names;
    REQUIRE_RC( KTableListCol( m_tbl, & names ) );

    uint32_t count = 1;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KWTable_ListIdx, KTable_Fixture)
{
    Open( "testdb", "SEQUENCE" );

    struct KNamelist * names;
    REQUIRE_RC( KTableListIdx( m_tbl, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)1, count );
    REQUIRE( KNamelistContains( names, "dummy" ) );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KWTable_MetaCompare, KTable_Fixture)
{
    Open( "testdb", "SEQUENCE" );

    bool equal = false;
    REQUIRE_RC( KTableMetaCompare( m_tbl, m_tbl, "", &equal ) );
    REQUIRE( equal );
}

//TODO: non-virtual write-side only methods

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

const char UsageDefaultName[] = "Test_KDB_KWTable";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KWTableTestSuite(argc, argv);
    return rc;
}

}
