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
* Unit tests for read-side KMetadata
*/

#include <ktst/unit_test.hpp>

#include <kdb/manager.h>

#include <../libs/kdb/rmeta.h>
#include <../libs/kdb/rdbmgr.h>

using namespace std;

TEST_SUITE(KDBRMetadataTestSuite);

static const string ScratchDir = "./data/";

class KMetadata_Fixture
{
public:
    KMetadata_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeRead ( & m_mgr, m_dir ) );
    }
    ~KMetadata_Fixture()
    {
        KMetadataRelease( m_meta );
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }

    KDirectory * m_dir = nullptr;
    const KDBManager * m_mgr = nullptr;
    KMetadata * m_meta = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KRMetadata_AddRelease, KMetadata_Fixture)
{
    const KDirectory *subdir;
    REQUIRE_RC(  KDirectoryOpenDirRead( m_dir, &subdir, false, "%s", "testdb/tbl/SEQUENCE" ) );
    REQUIRE_RC( KDBManagerOpenMetadataReadInt ( m_mgr, & m_meta, subdir, 0, false ) );
    KDirectoryRelease( subdir );

    REQUIRE_EQ( 1, (int)atomic32_read( & m_meta -> dad . refcount ) );
    REQUIRE_RC( KMetadataAddRef( m_meta ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_meta -> dad . refcount ) );
    REQUIRE_RC( KMetadataRelease( m_meta ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_meta -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRMetadata_Version, KMetadata_Fixture)
{
    const KDirectory *subdir;
    REQUIRE_RC(  KDirectoryOpenDirRead( m_dir, &subdir, false, "%s", "testdb/tbl/SEQUENCE" ) );
    REQUIRE_RC( KDBManagerOpenMetadataReadInt ( m_mgr, & m_meta, subdir, 0, false ) );
    KDirectoryRelease( subdir );

    uint32_t version = 0;
    REQUIRE_RC( KMetadataVersion( m_meta, & version ) );
    REQUIRE_EQ( (uint32_t)2, version );
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

const char UsageDefaultName[] = "Test_KDB_RMetadata";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBRMetadataTestSuite(argc, argv);
    return rc;
}

}
