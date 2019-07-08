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
* Unit tests for Kdb interface
*/

#include <kapp/args.h>

#include <ktst/unit_test.hpp>

#include <sysalloc.h>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>
#include <kdb/table.h>

#include <vfs/manager.h>

using namespace std;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KdbTestSuite, argsHandler);

#define KDB_MANAGER_MAKE(mgr, wd) KDBManagerMakeRead((const KDBManager **)mgr, (struct KDirectory const *)wd)
#include "remote_open_test.cpp"

TEST_CASE(KDBManagerVPathType)
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


    const KDBManager* mgr;
    REQUIRE_RC(KDBManagerMakeRead(&mgr, NULL));

//cout << path << endl;
    REQUIRE_EQ((int)kptTable, KDBManagerPathType(mgr, path.c_str()));

    REQUIRE_RC(KDBManagerRelease(mgr));

}

// KColumnBlob
// see same tests on the write side, wkdbtest.cpp

class ColumnBlobReadFixture
{
public:
    ColumnBlobReadFixture()
    :   m_num_read ( 0 ),
        m_remaining ( 0 )
    {
        const KDBManager* mgr;
        THROW_ON_RC ( KDBManagerMakeRead ( & mgr, NULL ) );

        const KTable* tbl;
        THROW_ON_RC ( KDBManagerOpenTableRead ( mgr, & tbl, "SRR000123" ) );

        const KColumn* col;
        THROW_ON_RC ( KTableOpenColumnRead ( tbl, & col, "X" ) );

        THROW_ON_RC ( KColumnOpenBlobRead ( col, & m_blob, 1 ) );

        THROW_ON_RC ( KColumnRelease ( col ) );
        THROW_ON_RC ( KTableRelease ( tbl ) );
        THROW_ON_RC ( KDBManagerRelease ( mgr ) );
    }
    ~ColumnBlobReadFixture()
    {
        KColumnBlobRelease ( m_blob );
    }

    const KColumnBlob*  m_blob;
    size_t m_num_read;
    size_t m_remaining;
};

FIXTURE_TEST_CASE ( ColumnBlobRead_basic, ColumnBlobReadFixture )
{
    const size_t BlobSize = 1882;
    const size_t BufSize = 2024;
    char buffer [ BufSize ];
    REQUIRE_RC ( KColumnBlobRead ( m_blob, 0, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BlobSize, m_num_read );
    REQUIRE_EQ ( (size_t)0, m_remaining );
}

FIXTURE_TEST_CASE ( ColumnBlobRead_insufficient_buffer, ColumnBlobReadFixture )
{
    const size_t BlobSize = 1882;
    const size_t BufSize = 1024;
    char buffer [ BufSize ];
    // first read incomplete
    REQUIRE_RC ( KColumnBlobRead ( m_blob, 0, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BufSize, m_num_read );
    REQUIRE_EQ ( BlobSize - BufSize, m_remaining );
    // the rest comes in on the second read
    REQUIRE_RC ( KColumnBlobRead ( m_blob, BufSize, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BlobSize - BufSize, m_num_read );
    REQUIRE_EQ ( (size_t)0, m_remaining );
}

//////////////////////////////////////////// Main
static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h> // KDbgSetString

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

const char UsageDefaultName[] = "test-kdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    //assert(!KDbgSetString("KFG"));
    assert(!KDbgSetString("VFS"));
    //KDbgSetModConds ( DBG_KNS, DBG_FLAG ( DBG_KNS_SOCKET ), DBG_FLAG ( DBG_KNS_SOCKET ) );
    KConfigDisableUserSettings();
    rc_t rc=KdbTestSuite(argc, argv);
    return rc;
}

}
