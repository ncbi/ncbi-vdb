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
#include <../libs/kdb/dbmgr-priv.h>

#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <kdb/table.h>

#include <klib/rc.h>

using namespace std;

TEST_SUITE(KDBRManagerTestSuite);

static const string ScratchDir = "./data/";

class KDBManager_Fixture
{
public:
    KDBManager_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeRead ( & m_mgr, m_dir ) );
    }
    ~KDBManager_Fixture()
    {
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }

    KDirectory * m_dir = nullptr;
    const KDBManager * m_mgr = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KDBRManager_AddRelease, KDBManager_Fixture)
{
    REQUIRE_EQ( 1, (int)atomic32_read( & m_mgr -> dad . refcount ) );
    REQUIRE_RC( KDBManagerAddRef( m_mgr ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_mgr -> dad . refcount ) );
    REQUIRE_RC( KDBManagerRelease( m_mgr ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_mgr -> dad . refcount ) );
    // use valgrind to find any leaks
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

const char UsageDefaultName[] = "Test_KDB_RManager";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBRManagerTestSuite(argc, argv);
    return rc;
}

}
