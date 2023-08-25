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

#include <../libs/kdb/table-priv.h>

#include <klib/rc.h>

#include <kdb/manager.h>
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
    }
    ~KTable_Fixture()
    {
        KTableRelease( m_tbl );
        KDirectoryRelease( m_dir );
    }
    void Setup( const string testName )
    {
        const string ColumnName = ScratchDir + testName;
        THROW_ON_RC( KTableMake( & m_tbl, m_dir, ColumnName.c_str() ) );
        KDirectoryAddRef( m_dir); // KTableMake does not call AddRef
    }

    KDirectory * m_dir = nullptr;
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

FIXTURE_TEST_CASE(KTable_OpenManagerRead, KTable_Fixture)
{
    Setup( GetName() );

    struct KDBManager const *mgr;
    REQUIRE_RC( KTableOpenManagerRead( m_tbl, &mgr ) );
    REQUIRE_NULL( mgr );
}

FIXTURE_TEST_CASE(KTable_OpenParentRead, KTable_Fixture)
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

const char UsageDefaultName[] = "Test_KDB_KWTable";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KWTableTestSuite(argc, argv);
    return rc;
}

}
