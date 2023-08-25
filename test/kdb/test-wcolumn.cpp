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
* Unit tests for read-side KColumn
*/

#include <ktst/unit_test.hpp>

#include <../libs/kdb/wcolumn-priv.h>

#include <kdb/manager.h>

#include <klib/rc.h>

using namespace std;

TEST_SUITE(KWColumnTestSuite);

static const string ScratchDir = "./data/";
class KColumn_Fixture
{
public:
    KColumn_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
    }
    ~KColumn_Fixture()
    {
        if ( m_col ) KColumnRelease( m_col );
        KDirectoryRelease( m_dir );
        KDBManagerRelease ( m_mgr );
    }
    void Setup( const string testName )
    {
        THROW_ON_RC( KDBManagerMakeUpdate ( & m_mgr, m_dir ) );
        const string ColumnName = ScratchDir + testName;
        THROW_ON_RC( KDBManagerCreateColumn ( m_mgr, & m_col, kcmOpen, kcsNone, 0, "%s", ColumnName.c_str() ) );
    }

    KDirectory * m_dir = nullptr;
    KColumn * m_col = nullptr;
    KDBManager * m_mgr = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KWColumn_AddRelease, KColumn_Fixture)
{
    Setup( GetName() );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_col -> dad . refcount ) );
    REQUIRE_RC( KColumnAddRef( m_col ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_col -> dad . refcount ) );
    REQUIRE_RC( KColumnRelease( m_col ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_col -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KWColumn_Locked, KColumn_Fixture)
{
    Setup( GetName() );
    REQUIRE( ! KColumnLocked( m_col ) );
}

FIXTURE_TEST_CASE(KWColumn_ByteOrder, KColumn_Fixture)
{
    Setup( GetName() );

    bool reversed = true;
    REQUIRE_RC( KColumnByteOrder( m_col, & reversed ) );
    REQUIRE( ! reversed );
}

FIXTURE_TEST_CASE(KWColumn_IdRange, KColumn_Fixture)
{
    Setup( GetName() );

    int64_t first = 0;
    uint64_t count = 1;
    rc_t rc = SILENT_RC ( rcDB, rcColumn, rcAccessing, rcRange, rcInvalid );
    REQUIRE_EQ( rc, KColumnIdRange( m_col, & first, & count ) );
}

FIXTURE_TEST_CASE(KWColumn_FindFirstRowId, KColumn_Fixture)
{
    Setup( GetName() );

    int64_t found;
    int64_t start = 0;
    rc_t rc = SILENT_RC ( rcDB, rcColumn, rcSelecting,rcRow,rcNotFound );
    REQUIRE_EQ( rc, KColumnFindFirstRowId( m_col, & found, start ) );
}

FIXTURE_TEST_CASE(KWColumn_OpenManagerRead, KColumn_Fixture)
{
    Setup( GetName() );
    const KDBManager * mgr = nullptr;
    REQUIRE_RC( KColumnOpenManagerRead( m_col, & mgr ) );
    REQUIRE_EQ( (const KDBManager *)m_mgr, mgr );

    REQUIRE_RC( KDBManagerRelease( mgr ) );
}

FIXTURE_TEST_CASE(KWColumn_OpenParentRead, KColumn_Fixture)
{
    Setup( GetName() );
    const KTable * tbl = nullptr;
    rc_t rc = SILENT_RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
    REQUIRE_EQ( rc, KColumnOpenParentRead( m_col, & tbl ) );
    REQUIRE_NULL( tbl );
}

// KColumnBlob

TEST_CASE(KWColumnBlob_AddRelease)
{
    KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnBlobMake ( & blob, false ) );

    REQUIRE_EQ( 1, (int)atomic32_read( & blob -> dad . refcount ) );
    REQUIRE_RC( KColumnBlobAddRef( blob ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & blob -> dad . refcount ) );
    REQUIRE_RC( KColumnBlobRelease( blob ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & blob -> dad . refcount ) );

    REQUIRE_RC( KColumnBlobRelease( blob ) );
    // use valgrind to find any leaks
}

TEST_CASE(KWColumnBlob_Read)
{
    KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnBlobMake ( & blob, false ) );

    char buffer[1024];
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobRead ( blob, 0, buffer, sizeof( buffer ), nullptr, nullptr ) );

    REQUIRE_RC( KColumnBlobRelease( blob ) );
}

TEST_CASE(KWColumnBlob_ReadAll)
{
    KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnBlobMake ( & blob, false ) );

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobReadAll ( blob, nullptr, nullptr, 0 ) );

    REQUIRE_RC( KColumnBlobRelease( blob ) );
}

TEST_CASE(KWColumnBlob_Validate)
{
    KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnBlobMake ( & blob, false ) );

    REQUIRE_RC( KColumnBlobValidate ( blob ) );

    REQUIRE_RC( KColumnBlobRelease( blob ) );
}

TEST_CASE(KWColumnBlob_ValidateBuffer)
{
    KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnBlobMake ( & blob, false ) );

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobValidateBuffer ( blob, nullptr, nullptr, 0 ) );

    REQUIRE_RC( KColumnBlobRelease( blob ) );
}

TEST_CASE(KWColumnBlob_IdRange)
{
    KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnBlobMake ( & blob, false ) );

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobIdRange ( blob, nullptr, nullptr ) );

    REQUIRE_RC( KColumnBlobRelease( blob ) );
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

const char UsageDefaultName[] = "Test_KDB_KWColumn";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KWColumnTestSuite(argc, argv);
    return rc;
}

}
