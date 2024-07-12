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
* Unit tests for read-side KColumnBlob
*/

#include <ktst/unit_test.hpp>

extern "C"
{
    #include "../libs/kdb/rcolumnblob.h"
    #include "../libs/kdb/rcolumn.h"
}

#include <../libs/kdb/ColumnBlob.hpp>

#include <kdb/manager.h>
#include <kdb/table.h>

using namespace std;

TEST_SUITE(RBlobTestSuite);

class RColumnBlobFixture
{
public:
    RColumnBlobFixture()
    {
        THROW_ON_RC( KRColumnBlobMake ( (KRColumnBlob**)& b, false) );
    }
    ~RColumnBlobFixture()
    {
        b -> release();
    }

    KColumnBlobBase * b = nullptr;
};

FIXTURE_TEST_CASE( Test_R_ColumnBlob_AddRelease, RColumnBlobFixture )
{
    REQUIRE_EQ( 1, (int)atomic32_read( & b -> refcount ) );
    REQUIRE_RC( b -> addRef() );
    REQUIRE_EQ( 2, (int)atomic32_read( & b -> refcount ) );
    REQUIRE_RC( b -> release() );
    REQUIRE_EQ( 1, (int)atomic32_read( & b -> refcount ) );
}

FIXTURE_TEST_CASE( Test_R_ColumnBlob_Read, RColumnBlobFixture )
{
    char buffer[1024];
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, b -> read( 0, buffer, sizeof( buffer ), nullptr, nullptr ) );
}

FIXTURE_TEST_CASE( Test_R_ColumnBlob_ReadAll, RColumnBlobFixture )
{
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, b -> readAll ( nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE( Test_R_ColumnBlob_Validate, RColumnBlobFixture )
{
    REQUIRE_RC( b -> validate() );
}

FIXTURE_TEST_CASE( Test_R_ColumnBlob_ValidateBuffer, RColumnBlobFixture )
{
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    REQUIRE_EQ( rc, b -> validateBuffer ( nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE( Test_R_ColumnBlob_IdRange, RColumnBlobFixture )
{
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    REQUIRE_EQ( rc, b -> idRange ( nullptr, nullptr ) );
}

// KColumnBlob

class RColumnBlobFixture_API
{
public:
    RColumnBlobFixture_API()
    {
    }
    ~RColumnBlobFixture_API()
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
    const KColumnBlobBase * getBlob() const { return (const KColumnBlobBase *)m_blob; };

    const KColumnBlob*  m_blob = nullptr;
    size_t m_num_read = 0;
    size_t m_remaining = 0;
};

FIXTURE_TEST_CASE(KRColumnBlob_AddRelease, RColumnBlobFixture_API)
{
    MakeBlob();

    REQUIRE_EQ( 1, (int)atomic32_read( & getBlob() -> refcount ) );
    REQUIRE_RC( KColumnBlobAddRef( m_blob ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & getBlob() -> refcount ) );
    REQUIRE_RC( KColumnBlobRelease( m_blob ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & getBlob() -> refcount ) );

    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRColumnBlob_Read, RColumnBlobFixture_API)
{
    MakeBlob();

    char buffer[1024];
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobRead ( m_blob, 0, buffer, sizeof( buffer ), nullptr, nullptr ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_ReadAll, RColumnBlobFixture_API)
{
    MakeBlob();

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobReadAll ( m_blob, nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_Validate, RColumnBlobFixture_API)
{
    MakeBlob();
    REQUIRE_RC( KColumnBlobValidate ( m_blob ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_ValidateBuffer, RColumnBlobFixture_API)
{
    MakeBlob();

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobValidateBuffer ( m_blob, nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KRColumnBlob_IdRange, RColumnBlobFixture_API)
{
    MakeBlob();

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    REQUIRE_EQ( rc, KColumnBlobIdRange ( m_blob, nullptr, nullptr ) );
}

FIXTURE_TEST_CASE ( ColumnBlobRead_basic, RColumnBlobFixture_API )
{
    OpenBlob();

    const size_t BlobSize = 1882;
    const size_t BufSize = 2024;
    char buffer [ BufSize ];
    REQUIRE_RC ( KColumnBlobRead ( m_blob, 0, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BlobSize, m_num_read );
    REQUIRE_EQ ( (size_t)0, m_remaining );
}

FIXTURE_TEST_CASE ( ColumnBlobRead_insufficient_buffer, RColumnBlobFixture_API )
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

const char UsageDefaultName[] = "Test_KDB_RBlob";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=RBlobTestSuite(argc, argv);
    return rc;
}

}
