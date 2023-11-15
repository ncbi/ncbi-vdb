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

using namespace std;

TEST_SUITE(KDBTextManagerTestSuite);

class KDBManager_Fixture
{
public:
    KDBManager_Fixture()
    {
    }
    ~KDBManager_Fixture()
    {
        KDBManagerRelease( m_mgr );
    }

    const KDBManager * m_mgr = nullptr;
};

FIXTURE_TEST_CASE(KDBRManager_Make_Null, KDBManager_Fixture)
{
    REQUIRE_RC_FAIL( KDBManagerMakeText ( nullptr, "" ) );
}

FIXTURE_TEST_CASE(KDBRManager_AddRelease, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerMakeText ( & m_mgr, "" ) );

    REQUIRE_NOT_NULL( m_mgr ) ;
    REQUIRE_RC( KDBManagerAddRef( m_mgr ) );
    REQUIRE_RC( KDBManagerRelease( m_mgr ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KDBRManager_Version_Null, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerMakeText ( & m_mgr, "" ) );

    REQUIRE_RC_FAIL( KDBManagerVersion( m_mgr, nullptr ) );
}
FIXTURE_TEST_CASE(KDBRManager_Version, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerMakeText ( & m_mgr, "" ) );

    uint32_t version = 0;
    REQUIRE_RC( KDBManagerVersion( m_mgr, & version ) );
    REQUIRE_EQ( (uint32_t)0, version );
}

FIXTURE_TEST_CASE(KDBRManager_Db_Exists_Not, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerMakeText ( & m_mgr, "" ) );
    REQUIRE( ! KDBManagerExists( m_mgr, kptDatabase, "%s", "testdb" ) );
}
FIXTURE_TEST_CASE(KDBRManager_Db_Exists, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerMakeText ( & m_mgr, "database testdb{}" ) );
    REQUIRE( KDBManagerExists( m_mgr, kptDatabase, "%s", "testdb" ) );
}


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


#if 0
FIXTURE_TEST_CASE(KDBRManager_Writable, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerWritable( m_mgr, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBRManager_RunPeriodicTasks, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerRunPeriodicTasks( m_mgr ) );
}

FIXTURE_TEST_CASE(KDBRManager_PathTypeVP, KDBManager_Fixture)
{
    VFSManager * vfs;
    REQUIRE_RC( VFSManagerMake ( & vfs ) );

    struct VPath * path;
    REQUIRE_RC( VFSManagerMakePath ( vfs, & path, "%s", "testdb" ) );
    REQUIRE_EQ( (int)kptDatabase, KDBManagerPathTypeVP( m_mgr, path ) );

    REQUIRE_RC( VPathRelease( path ) );
    REQUIRE_RC( VFSManagerRelease( vfs ) );
}

FIXTURE_TEST_CASE(KDBRManager_PathType, KDBManager_Fixture)
{
    REQUIRE_EQ( (int)kptDatabase, KDBManagerPathType( m_mgr, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBManagerVPathType, KDBManager_Fixture)
{
    string path;
    { // convert accession "SRR000123" into a file system path
        VFSManager* vfsmgr;
        REQUIRE_RC(VFSManagerMake(&vfsmgr));
        {
            VPath * vpath;
            const struct KFile *dummy1;
            const struct VPath *dummy2;
            REQUIRE_RC(VFSManagerResolveSpec ( vfsmgr, "SRR000123", &vpath, &dummy1, &dummy2, true));

            path = ToString(vpath);

            REQUIRE_RC(KFileRelease(dummy1));
            REQUIRE_RC(VPathRelease(dummy2));
            REQUIRE_RC(VPathRelease(vpath));
        }
        REQUIRE_RC(VFSManagerRelease(vfsmgr));
    }

//cout << path << endl;
    REQUIRE_EQ((int)kptTable, KDBManagerPathType(m_mgr, path.c_str()));
}


FIXTURE_TEST_CASE(KDBRManager_VPathTypeUnreliable, KDBManager_Fixture)
{
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

FIXTURE_TEST_CASE(KDBRManager_OpenDBRead, KDBManager_Fixture)
{
    const KDatabase * db = nullptr;
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & db, "%s", "testdb" ) );
    REQUIRE_NOT_NULL( db );
    REQUIRE_RC( KDatabaseRelease( db ) );
}

FIXTURE_TEST_CASE(KDBRManager_OpenTableRead, KDBManager_Fixture)
{
    const KTable * tbl = nullptr;
    REQUIRE_RC( KDBManagerOpenTableRead( m_mgr, & tbl, "%s", "SRR000123" ) );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_RC( KTableRelease( tbl ) );
}

FIXTURE_TEST_CASE(KDBRManager_OpenTableReadVPath, KDBManager_Fixture)
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

FIXTURE_TEST_CASE(KDBRManager_OpenColumnRead, KDBManager_Fixture)
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
