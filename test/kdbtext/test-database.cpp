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

#include <kdb/manager.h>
#include <kdb/database.h>

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextDatabaseTestSuite);

class KDBTextDatabase_Fixture
{
public:
    KDBTextDatabase_Fixture()
    {
    }
    ~KDBTextDatabase_Fixture()
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

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Empty, KDBTextDatabase_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_NoName, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "database"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_NoType, KDBTextDatabase_Fixture)
{
    Setup(R"({"name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_InvalidType, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": [], "name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_WrongType, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "table", "name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Flat, KDBTextDatabase_Fixture)
{
    SetupAndInflate(R"({"type": "database", "name": "testdb"})");
    REQUIRE_EQ( string("testdb"), m_db -> getName() );
}

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_NotArray, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "database", "name": "testdb",
        "databases":{"type": "database", "name":"subdb1"}
    })");

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_ElementNull, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb","databases":[ null ]})" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_ElementBad, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb",
        "databases":[ {"type": "NOTAdatabase", "name":"subdb1"} ]
    })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_Duplicate, KDBTextDatabase_Fixture)
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

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Tables_NotArray, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb", "tables":{"type":"table","name":"tbl"} })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Tables_ElementNull, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb", "tables":[null] })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Tables_ElementBad, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb", "tables":[{"type":"NOTAtable","name":"tbl"}] })" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Tables_ElementDuplicate, KDBTextDatabase_Fixture)
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

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );

    REQUIRE_NULL( m_db -> findDatabase( "notthere") );
    REQUIRE_NOT_NULL( m_db -> findDatabase( "subdb1") );
    REQUIRE_NOT_NULL( m_db -> findDatabase( "subdb2") );

    REQUIRE_NULL( m_db -> findTable( "notthere") );
    REQUIRE_NOT_NULL( m_db -> findTable( "tbl0-1") );
    REQUIRE_NOT_NULL( m_db -> findTable( "tbl0-2") );
}
FIXTURE_TEST_CASE(KDBTextDatabase_getDatabase, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    const Database * db = m_db -> getDatabase( p );
    REQUIRE_NOT_NULL( db );
    REQUIRE_EQ( string("testdb"), db->getName() );
}
FIXTURE_TEST_CASE(KDBTextDatabase_getDatabase_Nested, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2" );
    const Database * db = m_db -> getDatabase( p );
    REQUIRE_NOT_NULL( db );
    REQUIRE_EQ( string("subdb2"), db->getName() );
}
FIXTURE_TEST_CASE(KDBTextDatabase_getTable, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/tbl/tbl0-1" );
    const Table * tbl = m_db -> getTable( p );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_EQ( string("tbl0-1"), tbl->getName() );
}
FIXTURE_TEST_CASE(KDBTextDatabase_getTable_Nested, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2/tbl/tbl2-2" );
    const Table * tbl = m_db -> getTable( p );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_EQ( string("tbl2-2"), tbl->getName() );
}

FIXTURE_TEST_CASE(KDBTextDatabase_pathType_empty, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "" );
    REQUIRE_EQ( (int)kptNotFound, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_pathType_miss, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "qq" );
    REQUIRE_EQ( (int)kptNotFound, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_pathType_self, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    REQUIRE_EQ( (int)kptDatabase, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_pathType_nestedDb, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2" );
    REQUIRE_EQ( (int)kptDatabase, m_db -> pathType( p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_pathType_nestedTable, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2/tbl/tbl2-2" );
    REQUIRE_EQ( (int)kptTable, m_db -> pathType( p ) );
}
//TODO: metadata (db/table), index (db/table)

FIXTURE_TEST_CASE(KDBTextDatabase_exists_empty, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "" );
    REQUIRE( ! m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_exists_Database_Not, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "notadb" );
    REQUIRE( ! m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_exists_Database_WrongType, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    REQUIRE( ! m_db -> exists( kptTable, p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_exists_Database_Root, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb" );
    REQUIRE( m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_exists_Database_Nested, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2" );
    REQUIRE( m_db -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KDBTextDatabase_exists_Table, KDBTextDatabase_Fixture)
{
    SetupAndInflate( NestedDb );
    Path p( "testdb/db/subdb2/tbl/tbl2-1" );
    REQUIRE( m_db -> exists( kptTable, p ) );
}
//TODO: KDBTextDatabase_exists_Metadata (db/table)

// API

class KDBTextDatabase_ApiFixture
{
public:
    KDBTextDatabase_ApiFixture()
    {
    }
    ~KDBTextDatabase_ApiFixture()
    {
        KDatabaseRelease( m_db );
        KDBManagerRelease( m_mgr );
    }
    void Setup( const char * input )
    {
//    const KDBManager * mgr = nullptr;
        THROW_ON_RC( KDBManagerMakeText ( & m_mgr, input, m_error, sizeof m_error ) );
        THROW_ON_RC( KDBManagerOpenDBRead( m_mgr, & m_db, "%s", "testdb" ) );
//    Release(mgr); hold on to the parsed Json, not the top level db/table
    }

    const KDBManager * m_mgr = nullptr;
    const KDatabase * m_db = nullptr;
    char m_error[1024];
};

FIXTURE_TEST_CASE(KDBTextDatabase_AddRelease, KDBTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );

    REQUIRE_NOT_NULL( m_db ) ;
    REQUIRE_RC( KDatabaseAddRef( m_db ) );
    REQUIRE_RC( KDatabaseRelease( m_db ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KDBTextDatabase_Locked, KDBTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( ! KDatabaseLocked( m_db ) );
}

FIXTURE_TEST_CASE(KDBTextDatabase_Exists, KDBTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    REQUIRE( KDatabaseExists( m_db, kptTable, "%s", "testdb/db/subdb2/tbl/tbl2-2" ) );
}

FIXTURE_TEST_CASE(KDBTextDatabase_Alias, KDBTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( ! KDatabaseIsAlias( m_db, kptDatabase, nullptr, 0, "testdb" ) );
}

FIXTURE_TEST_CASE(KDBTextDatabase_Writable, KDBTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( ! KDatabaseWritable( m_db, kptDatabase, 0, "testdb" ) );
}

FIXTURE_TEST_CASE(KDBTextDatabase_OpenManagerRead, KDBTextDatabase_ApiFixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    const KDBManager * mgr;
    REQUIRE_RC( KDatabaseOpenManagerRead( m_db, & mgr ) );
    REQUIRE_EQ( m_mgr, mgr );
    KDBManagerRelease( mgr );
}

FIXTURE_TEST_CASE(KDBTextDatabase_OpenParentRead, KDBTextDatabase_ApiFixture)
{
    Setup( NestedDb );
    const KDatabase * subdb = nullptr;
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & subdb, "testdb/db/subdb1" ) );
    REQUIRE_NOT_NULL( subdb );
    const KDatabase * parent = nullptr;
    REQUIRE_RC( KDatabaseOpenParentRead( subdb, & parent ) );
    REQUIRE_EQ( m_db, parent );
    KDatabaseRelease( parent );
    KDatabaseRelease( subdb );
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
    rc_t rc=KDBTextDatabaseTestSuite(argc, argv);
    return rc;
}

}
