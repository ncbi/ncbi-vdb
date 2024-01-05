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
* Unit tests for KDBDatabase for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/database.hpp"
#include "../../libs/kdbtext/table.hpp"
#include "../../libs/kdbtext/metadata.hpp"

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/table.h>
#include <kdb/meta.h>
#include <kdb/index.h>
#include <kdb/namelist.h>
#include <kdb/kdb-priv.h>

#include <klib/rc.h>
#include <klib/json.h>
#include <klib/namelist.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextDatabaseTestSuite);

const char * NestedDb = R"({
    "type": "database",
    "name": "testdb",
    "databases":[
        {"type": "database", "name":"subdb1"},
        {"type": "database", "name":"subdb2","tables":[
                {"type": "table", "name": "tbl2-1"},
                {"type": "table", "name": "tbl2-2"}
            ]
        }
    ],
    "tables":[
        {"type": "table", "name": "tbl0-1"},
        {"type": "table", "name": "tbl0-2"}
    ]
})";

const char * DbWithMeta = R"({
    "type": "database",
    "name": "testdb",
    "metadata": { "name":"root", "value":"blah" }
})";

class KTextDatabase_Fixture
{
public:
    KTextDatabase_Fixture()
    {
    }
    ~KTextDatabase_Fixture()
    {
        delete m_db;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_db = new Database( json );
    }
    void SetupAndInflate( const char * input )
    {
        Setup( input );
        THROW_ON_RC( m_db -> inflate( m_error, sizeof m_error ) );
    }

    KJsonValue * m_json = nullptr;
    Database * m_db = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KTextDatabase_Make_Empty, KTextDatabase_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_NoName, KTextDatabase_Fixture)
{
    Setup(R"({"type": "database"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_NoType, KTextDatabase_Fixture)
{
    Setup(R"({"name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_InvalidType, KTextDatabase_Fixture)
{
    Setup(R"({"type": [], "name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_WrongType, KTextDatabase_Fixture)
{
    Setup(R"({"type": "table", "name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextDatabase_Make_Flat, KTextDatabase_Fixture)
{
    SetupAndInflate(R"({"type": "database", "name": "testdb"})");
    REQUIRE_EQ( string("testdb"), m_db -> getName() );
}

FIXTURE_TEST_CASE(KTextDatabase_Make_Nested_NotArray, KTextDatabase_Fixture)
{
    Setup(R"({"type": "database", "name": "testdb",
        "databases":{"type": "database", "name":"subdb1"}
    })");

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_Nested_ElementNull, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb","databases":[ null ]})" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_Nested_ElementBad, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb",
        "databases":[ {"type": "NOTAdatabase", "name":"subdb1"} ]
    })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_Nested_Duplicate, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb",
        "databases":[
            {"type": "database", "name":"subdb1"} ,
            {"type": "database", "name":"subdb1"}
        ]
    })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextDatabase_Make_Tables_NotArray, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb", "tables":{"type":"table","name":"tbl"} })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_Tables_ElementNull, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb", "tables":[null] })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_Tables_ElementBad, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb", "tables":[{"type":"NOTAtable","name":"tbl"}] })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextDatabase_Make_Tables_ElementDuplicate, KTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb",
        "tables":[
            {"type":"table","name":"tbl"},
            {"type":"table","name":"tbl"}
        ]
    })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextDatabase_Make_Nested, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );

    { Path p( "notthere" ); REQUIRE_NULL( m_db -> openDatabase( p ) ); }
    {
        Path p( "testdb/db/subdb1" );
        const Database * d = m_db -> openDatabase( p );
        REQUIRE_NOT_NULL( d );
        delete d;
    }
    {
        Path p( "testdb/db/subdb2" );
        const Database * d = m_db -> openDatabase( p );
        REQUIRE_NOT_NULL( d );
        delete d;
    }

    { Path p( "notthere"); REQUIRE_NULL( m_db -> openTable( p ) ); }
    {
        Path p( "testdb/tbl/tbl0-1");
        const Table * t = m_db -> openTable( p );
        REQUIRE_NOT_NULL( t );
        delete t;
    }
    {
        Path p( "testdb/tbl/tbl0-2");
        const Table * t = m_db -> openTable( p );
        REQUIRE_NOT_NULL( t );
        delete t;
    }
}
FIXTURE_TEST_CASE(KTextDatabase_openDatabase, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    const Database * db = m_db -> openDatabase( p );
    REQUIRE_NOT_NULL( db );
    REQUIRE_EQ( string("testdb"), db->getName() );
    Database::release( db );
}
FIXTURE_TEST_CASE(KTextDatabase_openDatabase_Nested, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2" );
    const Database * db = m_db -> openDatabase( p );
    REQUIRE_NOT_NULL( db );
    REQUIRE_EQ( string("subdb2"), db->getName() );
    Database::release( db );
}
FIXTURE_TEST_CASE(KTextDatabase_openSubDatabase, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    const Database * db = m_db -> openSubDatabase( "subdb2" );
    REQUIRE_NOT_NULL( db );
    REQUIRE_EQ( string("subdb2"), db->getName() );
    Database::release( db );
}

FIXTURE_TEST_CASE(KTextDatabase_openTable, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/tbl/tbl0-1" );
    const Table * tbl = m_db -> openTable( p );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_EQ( string("tbl0-1"), tbl->getName() );
    delete tbl;
}
FIXTURE_TEST_CASE(KTextDatabase_openTable_Nested, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2/tbl/tbl2-2" );
    const Table * tbl = m_db -> openTable( p );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_EQ( string("tbl2-2"), tbl->getName() );
    delete tbl;
}

FIXTURE_TEST_CASE(KTextDatabase_pathType_empty, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "" );
    REQUIRE_EQ( (int)kptNotFound, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_pathType_miss, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "qq" );
    REQUIRE_EQ( (int)kptNotFound, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_pathType_self, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    REQUIRE_EQ( (int)kptDatabase, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_pathType_nestedDb, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2" );
    REQUIRE_EQ( (int)kptDatabase, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_pathType_nestedTable, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2/tbl/tbl2-2" );
    REQUIRE_EQ( (int)kptTable, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_pathType_metadata, KTextDatabase_Fixture)
{
    SetupAndInflate( DbWithMeta );
    Path p( "testdb/md" );
    REQUIRE_EQ( (int)kptMetadata, m_db -> pathType( p ) );
}

FIXTURE_TEST_CASE(KTextDatabase_exists_empty, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "" );
    REQUIRE( ! m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_exists_Database_Not, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "notadb" );
    REQUIRE( ! m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_exists_Database_WrongType, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    REQUIRE( ! m_db -> exists( kptTable, p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_exists_Database_Root, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    REQUIRE( m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_exists_Database_Nested, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2" );
    REQUIRE( m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_exists_Table, KTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2/tbl/tbl2-1" );
    REQUIRE( m_db -> exists( kptTable, p ) );
}
FIXTURE_TEST_CASE(KTextDatabase_exists_metadata, KTextDatabase_Fixture)
{
    SetupAndInflate( DbWithMeta );
    Path p( "testdb/md" );
    REQUIRE( m_db -> exists( kptMetadata, p ) );
}

FIXTURE_TEST_CASE(KTextDatabase_openMetadata, KTextDatabase_Fixture)
{
    SetupAndInflate( DbWithMeta );
    const Metadata * m = m_db -> openMetadata();
    REQUIRE_NOT_NULL( m );
    REQUIRE_EQ( string("root"), m -> getName() );
}

// API

class KTextDatabase_ApiFixture
{
public:
    KTextDatabase_ApiFixture()
    {
    }
    ~KTextDatabase_ApiFixture()
    {
        KDatabaseRelease( m_db );
    }
    void Setup( const char * input )
    {
        try
        {
            const KDBManager * mgr = nullptr;
            THROW_ON_RC( KDBManagerMakeText ( & mgr, input, m_error, sizeof m_error ) );
            THROW_ON_RC( KDBManagerOpenDBRead( mgr, & m_db, "%s", "testdb" ) );
            KDBManagerRelease( mgr );
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
        }

    }

    const KDatabase * m_db = nullptr;
    char m_error[1024];
};

FIXTURE_TEST_CASE(KTextDatabase_AddRelease, KTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );

    REQUIRE_NOT_NULL( m_db ) ;
    REQUIRE_RC( KDatabaseAddRef( m_db ) );
    REQUIRE_RC( KDatabaseRelease( m_db ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KTextDatabase_Locked, KTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( ! KDatabaseLocked( m_db ) );
}

FIXTURE_TEST_CASE(KTextDatabase_Exists, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    REQUIRE( KDatabaseExists( m_db, kptTable, "%s", "testdb/db/subdb2/tbl/tbl2-2" ) );
}

FIXTURE_TEST_CASE(KTextDatabase_Alias, KTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( ! KDatabaseIsAlias( m_db, kptDatabase, nullptr, 0, "testdb" ) );
}

FIXTURE_TEST_CASE(KTextDatabase_Writable, KTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( ! KDatabaseWritable( m_db, kptDatabase, 0, "testdb" ) );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenManagerRead, KTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    const KDBManager * mgr;
    REQUIRE_RC( KDatabaseOpenManagerRead( m_db, & mgr ) );
    REQUIRE_NOT_NULL( mgr );
    KDBManagerRelease( mgr );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenDbRead, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const KDatabase * subdb = nullptr;
    REQUIRE_RC( KDatabaseOpenDBRead( m_db, & subdb, "subdb1" ) );
    REQUIRE_NOT_NULL( subdb );
    KDatabaseRelease( subdb );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenParentRead, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const KDatabase * subdb = nullptr;
    REQUIRE_RC( KDatabaseOpenDBRead( m_db, & subdb, "subdb1" ) );
    REQUIRE_NOT_NULL( subdb );

    const KDatabase * parent = nullptr;
    REQUIRE_RC( KDatabaseOpenParentRead( subdb, & parent ) );
    REQUIRE_EQ( m_db, parent );
    KDatabaseRelease( parent );
    KDatabaseRelease( subdb );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenDirectoryRead, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const KDirectory * dir;
    rc_t rc = KDatabaseOpenDirectoryRead( m_db, & dir );
    REQUIRE_EQ( SILENT_RC( rcDB, rcDatabase, rcAccessing, rcDirectory, rcUnsupported ), rc );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenTableRead, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const KTable * tbl = nullptr;
    REQUIRE_RC( KDatabaseOpenTableRead ( m_db, &tbl, "%s", "tbl0-2" ) );
    REQUIRE_NOT_NULL( tbl );
    KTableRelease( tbl );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenMetadataRead, KTextDatabase_ApiFixture)
{
    Setup( DbWithMeta );
    const KMetadata * m = nullptr;
    REQUIRE_RC( KDatabaseOpenMetadataRead ( m_db, &m ) );
    REQUIRE_NOT_NULL( m );
    KMetadataRelease( m );
}

FIXTURE_TEST_CASE(KTextDatabase_OpenIndexRead, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const KIndex * idx;
    rc_t rc = KDatabaseOpenIndexRead( m_db, & idx, "idx" );
    REQUIRE_EQ( SILENT_RC( rcDB, rcDatabase, rcAccessing, rcIndex, rcUnsupported ), rc );
}

FIXTURE_TEST_CASE(KTextDatabase_ListDB, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    KNamelist * names;
    REQUIRE_RC( KDatabaseListDB( m_db, & names ) );
    REQUIRE_NOT_NULL( names );
    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)2, count );
    REQUIRE( KNamelistContains( names, "subdb1" ) );
    REQUIRE( KNamelistContains( names, "subdb2" ) );
    KNamelistRelease( names );
}

FIXTURE_TEST_CASE(KTextDatabase_ListTbl, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    KNamelist * names;
    REQUIRE_RC( KDatabaseListTbl( m_db, & names ) );
    REQUIRE_NOT_NULL( names );
    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)2, count );
    REQUIRE( KNamelistContains( names, "tbl0-1" ) );
    REQUIRE( KNamelistContains( names, "tbl0-2" ) );
    KNamelistRelease( names );
}

FIXTURE_TEST_CASE(KTextDatabase_ListIdx, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    KNamelist * names;

    REQUIRE_RC( KDatabaseListIdx( m_db, & names ) );
    REQUIRE_NOT_NULL( names );
    uint32_t count = 1;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    KNamelistRelease( names );
}

FIXTURE_TEST_CASE(KTextDatabase_GetPath, KTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const char * path;
    rc_t rc = KDatabaseGetPath( m_db, & path );
    REQUIRE_EQ( SILENT_RC( rcDB, rcDatabase, rcAccessing, rcPath, rcUnsupported ), rc );
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

const char UsageDefaultName[] = "Test_KDBText_Database";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KTextDatabaseTestSuite(argc, argv);
    return rc;
}

}
