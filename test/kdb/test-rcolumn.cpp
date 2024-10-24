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
* Unit tests for read-side KColumn and KColumnBlob
*/

#include <ktst/unit_test.hpp>

extern "C"
{
    #include <../libs/kdb/rcolumnblob.h>
    #include <../libs/kdb/rcolumn.h>
}

#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/meta.h>

#include <klib/rc.h>

using namespace std;

TEST_SUITE(KRColumnTestSuite);

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
        KColumnRelease( & m_col -> dad );
        KDirectoryRelease( m_dir );
    }
    void Setup( const string testName )
    {
        const string ColumnName = ScratchDir + testName;
        THROW_ON_RC( KRColumnMake( & m_col, m_dir, ColumnName.c_str() ) );
        KDirectoryAddRef( m_dir); // KRColumnMake does not call AddRef
    }

    KDirectory * m_dir = nullptr;
    KRColumn * m_col = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KRColumn_AddRelease, KColumn_Fixture)
{
    Setup( GetName() );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_col -> dad . refcount ) );
    REQUIRE_RC( KColumnAddRef( & m_col -> dad ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_col -> dad . refcount ) );
    REQUIRE_RC( KColumnRelease( & m_col -> dad ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_col -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRColumn_Locked, KColumn_Fixture)
{
    Setup( GetName() );
    REQUIRE( ! KColumnLocked( & m_col -> dad ) );
}

FIXTURE_TEST_CASE(KRColumn_ByteOrder, KColumn_Fixture)
{
    Setup( GetName() );

    bool reversed = true;
    REQUIRE_RC( KColumnByteOrder( & m_col -> dad, & reversed ) );
    REQUIRE( ! reversed );
}

FIXTURE_TEST_CASE(KRColumn_IdRange, KColumn_Fixture)
{
    Setup( GetName() );

    int64_t first = 0;
    uint64_t count = 1;
    rc_t rc = SILENT_RC ( rcDB, rcColumn, rcAccessing, rcRange, rcInvalid );
    REQUIRE_EQ( rc, KColumnIdRange( & m_col -> dad, & first, & count ) );
}

FIXTURE_TEST_CASE(KRColumn_FindFirstRowId, KColumn_Fixture)
{
    Setup( GetName() );

    int64_t found;
    int64_t start = 0;
    rc_t rc = SILENT_RC ( rcDB, rcColumn, rcSelecting,rcRow,rcNotFound );
    REQUIRE_EQ( rc, KColumnFindFirstRowId( & m_col -> dad, & found, start ) );
}

FIXTURE_TEST_CASE(KRColumn_OpenManagerRead, KColumn_Fixture)
{
    Setup( GetName() );
    const KDBManager * mgr = nullptr;
    REQUIRE_RC( KColumnOpenManagerRead( & m_col -> dad, & mgr ) );
    REQUIRE_NULL( mgr );
}

FIXTURE_TEST_CASE(KRColumn_OpenParentRead, KColumn_Fixture)
{
    Setup( GetName() );
    const KTable * tbl = nullptr;
    rc_t rc = SILENT_RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull ); // no parent set
    REQUIRE_EQ( rc, KColumnOpenParentRead( & m_col -> dad, & tbl ) );
    REQUIRE_NULL( tbl );
}

FIXTURE_TEST_CASE(KRColumn_OpenMetadataRead, KColumn_Fixture)
{
    Setup( GetName() );
    const KMetadata * meta = nullptr;
    rc_t rc = SILENT_RC ( rcDB,rcMgr,rcOpening,rcMetadata,rcNotFound );
    REQUIRE_EQ( rc, KColumnOpenMetadataRead( & m_col -> dad, & meta ) );
}

FIXTURE_TEST_CASE(KRColumn_OpenBlobRead, KColumn_Fixture)
{
    Setup( GetName() );
    const KColumnBlob * blob = nullptr;
    rc_t rc = SILENT_RC ( rcDB,rcColumn,rcSelecting,rcBlob,rcNotFound );
    REQUIRE_EQ( rc, KColumnOpenBlobRead( & m_col -> dad, & blob, 1 ) );
}

// KColumnBlob

class RColumnBlobFixture
{
public:
    RColumnBlobFixture()
    {
    }
    ~RColumnBlobFixture()
    {
        KColumnBlobRelease ( m_blob );
    }

    void OpenBlob()
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

    void MakeBlob()
    {
        THROW_ON_RC( KRColumnBlobMake ( (KRColumnBlob**) & m_blob, false ) );
    }

    const KColumnBlob*  m_blob = nullptr;
    size_t m_num_read = 0;
    size_t m_remaining = 0;
};

FIXTURE_TEST_CASE(KRColumnBlob_AddRelease, RColumnBlobFixture)
{
    MakeBlob();

    REQUIRE_EQ( 1, (int)atomic32_read( & m_blob -> refcount ) );
    REQUIRE_RC( KColumnBlobAddRef( m_blob ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_blob -> refcount ) );
    REQUIRE_RC( KColumnBlobRelease( m_blob ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_blob -> refcount ) );

    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRColumnBlob_Read, RColumnBlobFixture)
{
    MakeBlob();

    char buffer[1024];
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobRead ( m_blob, 0, buffer, sizeof( buffer ), nullptr, nullptr ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_ReadAll, RColumnBlobFixture)
{
    MakeBlob();

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobReadAll ( m_blob, nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_Validate, RColumnBlobFixture)
{
    MakeBlob();
    REQUIRE_RC( KColumnBlobValidate ( m_blob ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_ValidateBuffer, RColumnBlobFixture)
{
    MakeBlob();

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobValidateBuffer ( m_blob, nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_IdRange, RColumnBlobFixture)
{
    MakeBlob();

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobIdRange ( m_blob, nullptr, nullptr ) );
}

FIXTURE_TEST_CASE ( ColumnBlobRead_basic, RColumnBlobFixture )
{
    OpenBlob();

    const size_t BlobSize = 1882;
    const size_t BufSize = 2024;
    char buffer [ BufSize ];
    REQUIRE_RC ( KColumnBlobRead ( m_blob, 0, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BlobSize, m_num_read );
    REQUIRE_EQ ( (size_t)0, m_remaining );
}

FIXTURE_TEST_CASE ( ColumnBlobRead_insufficient_buffer, RColumnBlobFixture )
{
    OpenBlob();

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

const char UsageDefaultName[] = "Test_KDB_KRColumn";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KRColumnTestSuite(argc, argv);
    return rc;
}

}
