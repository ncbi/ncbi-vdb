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
* Unit tests for read-side KDBManager
*/

#include <ktst/unit_test.hpp>

#include <../libs/kdb/rdatabase.h>
#include <../libs/kdb/wdbmgr.h>
#include <../libs/kdb/libkdb.vers.h>

#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <kdb/table.h>
#include <kdb/index.h>

#include <kfs/file.h>

#include <vfs/manager.h>
#include <vfs/path.h>

#include <klib/rc.h>

using namespace std;

TEST_SUITE(KDBWManagerTestSuite);

static const string ScratchDir = "./data/";

static string ToString(const VPath* path)
{
    const String * s;
    if ( VPathMakeString (path, &s) != 0 )
        throw logic_error ( "ToString(VPath) failed" );
    string ret = string(s->addr, s->size);
    free((void*)s);
    return ret;
}

class KDBManager_Fixture
{
public:
    KDBManager_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeUpdate( & m_mgr, m_dir ) );
    }
    ~KDBManager_Fixture()
    {
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }

    KDirectory * m_dir = nullptr;
    KDBManager * m_mgr = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KDBWManager_AddRelease, KDBManager_Fixture)
{
    REQUIRE_EQ( 1, (int)atomic32_read( & m_mgr -> dad . refcount ) );
    REQUIRE_RC( KDBManagerAddRef( m_mgr ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_mgr -> dad . refcount ) );
    REQUIRE_RC( KDBManagerRelease( m_mgr ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_mgr -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KDBWManager_Version, KDBManager_Fixture)
{
    uint32_t version = 0;
    REQUIRE_RC( KDBManagerVersion( m_mgr, & version ) );
    REQUIRE_EQ( (uint32_t)LIBKDB_VERS, version );
}

FIXTURE_TEST_CASE(KDBWManager_Exists, KDBManager_Fixture)
{
    REQUIRE( KDBManagerExists( m_mgr, kptDatabase, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBWManager_Writable, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerWritable( m_mgr, "%s", "testdb" ) );
}

FIXTURE_TEST_CASE(KDBWManager_RunPeriodicTasks, KDBManager_Fixture)
{
    REQUIRE_RC( KDBManagerRunPeriodicTasks( m_mgr ) );
}

FIXTURE_TEST_CASE(KDBWManager_PathTypeVP, KDBManager_Fixture)
{
    VFSManager * vfs;
    REQUIRE_RC( VFSManagerMake ( & vfs ) );

    struct VPath * path;
    REQUIRE_RC( VFSManagerMakePath ( vfs, & path, "%s", "testdb" ) );
    REQUIRE_EQ( (int)kptDatabase, KDBManagerPathTypeVP( m_mgr, path ) );

    REQUIRE_RC( VPathRelease( path ) );
    REQUIRE_RC( VFSManagerRelease( vfs ) );
}

FIXTURE_TEST_CASE(KDBWManager_PathType_Column, KDBManager_Fixture)
{
    auto const pathType = KDBManagerPathType( m_mgr, "%s/tbl/%s/col/%s", "testdb", "SEQUENCE", "READ" );
    REQUIRE_EQ( (int)kptColumn, pathType );
}

FIXTURE_TEST_CASE(KDBWManager_PathType_Table, KDBManager_Fixture)
{
    auto const pathType = KDBManagerPathType( m_mgr, "%s/tbl/%s", "testdb", "SEQUENCE" );
    REQUIRE_EQ( (int)kptTable, pathType );
}

FIXTURE_TEST_CASE(KDBWManager_PathType_Database, KDBManager_Fixture)
{
    auto const pathType = KDBManagerPathType( m_mgr, "%s", "testdb" );
    REQUIRE_EQ( (int)kptDatabase, pathType );
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


FIXTURE_TEST_CASE(KDBWManager_VPathTypeUnreliable, KDBManager_Fixture)
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

FIXTURE_TEST_CASE(KDBWManager_OpenDBRead, KDBManager_Fixture)
{
    const KDatabase * db = nullptr;
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & db, "%s", "testdb" ) );
    REQUIRE_NOT_NULL( db );
    REQUIRE_RC( KDatabaseRelease( db ) );
}

FIXTURE_TEST_CASE(KDBWManager_OpenTableRead, KDBManager_Fixture)
{
    const KTable * tbl = nullptr;
    REQUIRE_RC( KDBManagerOpenTableRead( m_mgr, & tbl, "%s", "SRR000123" ) );
    REQUIRE_NOT_NULL( tbl );
    REQUIRE_RC( KTableRelease( tbl ) );
}

FIXTURE_TEST_CASE(KDBWManager_OpenTableReadVPath, KDBManager_Fixture)
{   // TODO: this test leaks a VPath. make a ticket.
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

FIXTURE_TEST_CASE(KDBWManager_OpenColumnRead, KDBManager_Fixture)
{
    const KColumn * col = nullptr;
    rc_t rc = SILENT_RC( rcVFS,rcMgr,rcOpening,rcDirectory,rcNotFound );
    REQUIRE_EQ( rc, KDBManagerOpenColumnRead( m_mgr, & col, "%s", "testdb/tbl/SEQUENCE/col/qq" ) );
    REQUIRE_NULL( col );
}

//KDBManagerVPathOpenLocalDBRead: see remote_open_test.cpp/wkdbtest.cpp
//KDBManagerVPathOpenRemoteDBRead

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

const char UsageDefaultName[] = "Test_KDB_RManager";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBWManagerTestSuite(argc, argv);
    return rc;
}

}
