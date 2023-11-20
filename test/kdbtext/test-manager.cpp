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
* Unit tests for KDBManager for reading textual data
*/

#include <ktst/unit_test.hpp>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/table.h>
#include <kdb/kdb-priv.h>

#include <klib/rc.h>

#include <vfs/path.h>
#include <vfs/manager.h>

using namespace std;

TEST_SUITE(KDBTextManagerTestSuite);

class KDBTextManager_Fixture
{
public:
    KDBTextManager_Fixture()
    {
    }
    ~KDBTextManager_Fixture()
    {
        KDBManagerRelease( m_mgr );
    }
    void Setup( const char * input = "{}" )
    {
        THROW_ON_RC( KDBManagerMakeText ( & m_mgr, input, m_error, sizeof m_error ) );
    }
    const KDBManager * m_mgr = nullptr;
    char m_error[1024];
};

FIXTURE_TEST_CASE(KDBTextManager_Make_Null, KDBTextManager_Fixture)
{
    REQUIRE_RC_FAIL( KDBManagerMakeText ( nullptr, "{}", m_error, sizeof m_error  ) );
}

FIXTURE_TEST_CASE(KDBTextManager_Make_BadJson, KDBTextManager_Fixture)
{
    REQUIRE_RC_FAIL( KDBManagerMakeText ( & m_mgr, "notavalidjson", m_error, sizeof m_error  ) );
}

FIXTURE_TEST_CASE(KDBTextManager_Make_WrongJsonRoot, KDBTextManager_Fixture)
{
    REQUIRE_RC_FAIL( KDBManagerMakeText ( & m_mgr, "[]", m_error, sizeof m_error  ) );
}

FIXTURE_TEST_CASE(KDBTextManager_AddRelease, KDBTextManager_Fixture)
{
    Setup();

    REQUIRE_NOT_NULL( m_mgr ) ;
    REQUIRE_RC( KDBManagerAddRef( m_mgr ) );
    REQUIRE_RC( KDBManagerRelease( m_mgr ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KDBTextManager_Version_Null, KDBTextManager_Fixture)
{
    Setup();

    REQUIRE_RC_FAIL( KDBManagerVersion( m_mgr, nullptr ) );
}
FIXTURE_TEST_CASE(KDBTextManager_Version, KDBTextManager_Fixture)
{
    Setup();

    uint32_t version = 0;
    REQUIRE_RC( KDBManagerVersion( m_mgr, & version ) );
    REQUIRE_EQ( (uint32_t)0, version );
}

FIXTURE_TEST_CASE(KDBTextManager_Db_Exists_Not, KDBTextManager_Fixture)
{
    Setup();
    REQUIRE( ! KDBManagerExists( m_mgr, kptDatabase, "%s", "testdb" ) );
}
FIXTURE_TEST_CASE(KDBTextManager_Db_Exists, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE( KDBManagerExists( m_mgr, kptDatabase, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBTextManager_Writable_Found, KDBTextManager_Fixture)
{   // for now, any path will be reported as readonly
    Setup( R"({"type": "database", "name": "testdb"})" );
    rc_t rc = KDBManagerWritable( m_mgr, "%s", "testdb" );
    REQUIRE_EQ( SILENT_RC( rcDB, rcPath, rcAccessing, rcPath, rcReadonly ), rc );
}
//TODO
// FIXTURE_TEST_CASE(KDBTextManager_Writable_NotFound, KDBTextManager_Fixture)
// {
//     Setup( R"({"type": "database", "name": "testdb"})" );
//     rc_t rc = KDBManagerWritable( m_mgr, "%s", "proddb" );
//     REQUIRE_EQ( SILENT_RC( rcDB, rcPath, rcAccessing, rcPath, rcNotFound ), rc );
// }

//TODO: KDBManagerWritable on internal objects, to test interpretation of the path

FIXTURE_TEST_CASE(KDBTextManager_RunPeriodicTasks, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE_RC( KDBManagerRunPeriodicTasks( m_mgr ) );
}

FIXTURE_TEST_CASE(KDBTextManager_PathTypeVP_Db, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );

    VFSManager * vfs;
    REQUIRE_RC( VFSManagerMake ( & vfs ) );

    struct VPath * path;
    REQUIRE_RC( VFSManagerMakePath ( vfs, & path, "%s", "testdb" ) );
    REQUIRE_EQ( (int)kptDatabase, KDBManagerPathTypeVP( m_mgr, path ) );

    REQUIRE_RC( VPathRelease( path ) );
    REQUIRE_RC( VFSManagerRelease( vfs ) );
}
FIXTURE_TEST_CASE(KDBTextManager_PathTypeVP_NotFound, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );

    VFSManager * vfs;
    REQUIRE_RC( VFSManagerMake ( & vfs ) );

    struct VPath * path;
    REQUIRE_RC( VFSManagerMakePath ( vfs, & path, "%s", "proddb" ) );
    REQUIRE_EQ( (int)kptNotFound, KDBManagerPathTypeVP( m_mgr, path ) );

    REQUIRE_RC( VPathRelease( path ) );
    REQUIRE_RC( VFSManagerRelease( vfs ) );
}
//TODO: KDBManagerPathTypeVP on internal objects, to test interpretation of the path

FIXTURE_TEST_CASE(KDBTextManager_PathType, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    REQUIRE_EQ( (int)kptDatabase, KDBManagerPathType( m_mgr, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBTextManager_VPathTypeUnreliable, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    auto fn = [] ( const KDBManager * self, const char *path, ... ) -> int
    {
        va_list args;
        va_start ( args, path );
        int res = KDBManagerVPathTypeUnreliable ( self, path, args );
        va_end (args);
        return res;
    };
    REQUIRE_EQ( (int)kptDatabase, fn( m_mgr, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBTextManager_OpenDBRead, KDBTextManager_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb"})" );
    const KDatabase * db = nullptr;
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & db, "%s", "testdb" ) );
    REQUIRE_NOT_NULL( db );
    REQUIRE_RC( KDatabaseRelease( db ) );
}

FIXTURE_TEST_CASE(KDBTextManager_OpenTableRead, KDBTextManager_Fixture)
{   // root table
    Setup( R"({"type": "table", "name": "tbl"})" );
    const KTable * tbl = nullptr;
    REQUIRE_RC( KDBManagerOpenTableRead( m_mgr, & tbl, "%s", "tbl" ) );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_RC( KTableRelease( tbl ) );
}
//TODO: non-root tables

#if 0
FIXTURE_TEST_CASE(KDBTextManager_OpenTableReadVPath, KDBTextManager_Fixture)
{
    VFSManager * vfs;
    REQUIRE_RC( VFSManagerMake ( & vfs ) );

    struct VPath * path;
    REQUIRE_RC( VFSManagerMakePath ( vfs, & path, "%s", "SRR000123" ) );

    const KTable * tbl = nullptr;
    REQUIRE_RC( KDBManagerOpenTableReadVPath( m_mgr, & tbl, path ) );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_RC( KTableRelease( tbl ) );

    REQUIRE_RC( VPathRelease( path ) );
    REQUIRE_RC( VFSManagerRelease( vfs ) );
}

FIXTURE_TEST_CASE(KDBTextManager_OpenColumnRead, KDBTextManager_Fixture)
{
    const KColumn * col = nullptr;
    rc_t rc = SILENT_RC( rcVFS,rcMgr,rcOpening,rcDirectory,rcNotFound );
    REQUIRE_EQ( rc, KDBManagerOpenColumnRead( m_mgr, & col, "%s", "testdb/tbl/SEQUENCE/col/qq" ) );
    REQUIRE_NULL( col );
}

//KDBManagerVPathOpenLocalDBRead: see remote_open_test.cpp/kdbtest.cpp
//KDBManagerVPathOpenRemoteDBRead
#endif

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

const char UsageDefaultName[] = "Test_KDBText_Manager";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextManagerTestSuite(argc, argv);
    return rc;
}

}

#if 0
    struct Database {
        let name: String
        let metadata: Metadata

        let tables: [Table]
        let indices: [Index]
        let databases: [Database]
    }
        struct Node {
            struct Attribute {
                let name: String
                let value: [UInt8]
            }
            let name: String
            let value: [UInt8]
            let attributes: [Attribute]
            let children: [Node]
        }
        let root: Node
    }
    struct Table {
        let name: String
        let metadata: Metadata

        let columns: [Column]
        let indices: [Index]
    }
    struct Column {
        let name: String
        let metadata: Metadata
        let data: [(row: Int64, value: [UInt8])]
    }
    struct Index {
        struct Value {
            let startId: Int64
            let count: UInt64
        }
        enum IndexType {
            case text([String: Value])
            case rowId([UInt64: Value])
        }
        let name: String
        let text: IndexType
    }

{
    "type": "database",
    "name": "testdb",
    "metadata":
    {
        "name":"",
        "value":"blah",
        "attributes":{"attr0":"value", "attr1":"attr1value"},
        "children":{
            "name":"schema",
            "value":"version 1;....",
            "attributes":{},
            "children":{}
        }
    }
    "tables":[
        {
            "name":"SEQUENCE",
            "metadata":null,
            "columns":[
                {
                    "name":"READ",
                    "metadata":null,
                    "data":
                    [
                        {"row":1,"value":"AGCT"},
                        {"row":2,"value":"AGCT"}
                    ]
                }
            ]
            "indexes":[
                {
                    "name":"qwer",
                    "text":[
                        {"key":"CG", "startId":1, "count":10},
                        {"key":"AT", "startId":11, "count":2},
                    ],
                },
                {
                    "name":"rewq",
                    "projection":[
                        {"value":"CG", "startId":1, "count":10},
                        {"value":"AT", "startId":11, "count":2},
                    ],
                }
            ]
        }
    ]
}

metadata testdb.m
{
    name :
}
#endif