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
* Unit tests for KDBColumn for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/columnblob.hpp"

#include <kdb/column.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextColumnBlobTestSuite);

const string Data = "0123456789";

class KTextColumnBlob_Fixture
{
public:
    KTextColumnBlob_Fixture()
    {
    }
    ~KTextColumnBlob_Fixture()
    {
        delete m_blob;
    }

    void Setup( const string & data = Data )
    {
        m_blob = new ColumnBlob( data.c_str(), data.size(), nullptr, 1, 2 );
    }

    ColumnBlob * m_blob = nullptr;
};

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Empty, KTextColumnBlob_Fixture)
{
    Setup("");
    REQUIRE_EQ( string(), string( (const char*) (m_blob -> getData()), m_blob -> getSize() ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Data, KTextColumnBlob_Fixture)
{
    Setup( Data );
    REQUIRE_EQ( Data, string( (const char*) (m_blob -> getData()), m_blob -> getSize() ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_GetIdRange, KTextColumnBlob_Fixture)
{
    Setup( Data );
    REQUIRE_EQ( Data, string( (const char*) (m_blob -> getData()), m_blob -> getSize() ) );
    REQUIRE_EQ( (int64_t)1, m_blob -> getIdRange().first );
    REQUIRE_EQ( (uint32_t)2, m_blob -> getIdRange().second );
}

class KTextColumnBlob_ApiFixture
{
public:
    KTextColumnBlob_ApiFixture()
    {
        KDataBufferMakeBytes( & m_buffer, 0 );
    }
    ~KTextColumnBlob_ApiFixture()
    {
        KDataBufferWhack( & m_buffer );
        KColumnBlobRelease( m_blob );
    }
    void Setup( const string & data = "abcdef" )
    {
        m_blob = (const KColumnBlob*)new ColumnBlob( data.c_str(), data.size(), nullptr, 1, 2 );
    }

    const KColumnBlob * m_blob = nullptr;

    KDataBuffer m_buffer;
};

FIXTURE_TEST_CASE(KTextColumnBlob_AddRelease, KTextColumnBlob_ApiFixture)
{
    Setup();

    REQUIRE_RC( KColumnBlobAddRef( m_blob ) );
    REQUIRE_RC( KColumnBlobRelease( m_blob ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KTextColumnBlob_BufferNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    size_t num_read = 0;
    size_t remaining = 0;
    REQUIRE_RC_FAIL( KColumnBlobRead ( m_blob, 0, nullptr, 1, &num_read, &remaining ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_BufferNull_SizeNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    size_t num_read = 0;
    size_t remaining = 0;
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, nullptr, 0, &num_read, &remaining ) );
    REQUIRE_EQ( (size_t)0, num_read );
    REQUIRE_EQ( Data.size(), remaining );
}
FIXTURE_TEST_CASE(KTextColumnBlob_NumReadNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    char data[1024] = {0};
    size_t remaining = 0;
    rc_t rc = KColumnBlobRead ( m_blob, 0, data, sizeof data, nullptr, &remaining );
    REQUIRE_EQ( SILENT_RC( rcDB, rcBlob, rcReading, rcParam, rcNull ), rc );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_From_0, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    char data[1024] = {0};
    size_t num_read = 0;
    size_t remaining = 0;
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, data, sizeof data, &num_read, &remaining ) );
    REQUIRE_EQ( Data, string(data, num_read ) );
    REQUIRE_EQ( (size_t)0, remaining );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_From_Offset, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    char data[1024] = {0};
    size_t num_read = 0;
    size_t remaining = 0;
    const size_t Offset = 3;
    REQUIRE_RC( KColumnBlobRead ( m_blob, Offset, data, sizeof data, &num_read, &remaining ) );
    REQUIRE_EQ( Data.substr( Offset ), string(data, num_read ) );
    REQUIRE_EQ( (size_t)0, remaining );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_Offset_BeyondData, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    char data[1024] = {0};
    size_t num_read = 0;
    REQUIRE_RC_FAIL( KColumnBlobRead ( m_blob, Data.size() + 1, data, sizeof data, &num_read, nullptr ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_BufferTooShort, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    char data[4] = {0};
    size_t num_read = 0;
    size_t remaining = 0;
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, data, sizeof data, &num_read, &remaining ) );
    REQUIRE_EQ( sizeof data, num_read );
    REQUIRE_EQ( Data.size() - num_read, remaining );
    REQUIRE_EQ( Data.substr( 0, num_read ), string(data, num_read ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ReadAll_BufferNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    REQUIRE_RC_FAIL( KColumnBlobReadAll ( m_blob, nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ReadAll_NoCS, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & m_buffer, nullptr, 0 ) );
    REQUIRE_EQ( Data, string( (const char*)m_buffer.base, m_buffer.elem_count ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ReadAll_WithCS, KTextColumnBlob_ApiFixture)
{   // checksum data is set to all 0s
    Setup( Data );
    KColumnBlobCSData cs0;
    memset ( & cs0, 0, sizeof( KColumnBlobCSData ) );
    KColumnBlobCSData cs;
    memset ( & cs, 1, sizeof( KColumnBlobCSData ) );

    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & m_buffer, & cs, sizeof( KColumnBlobCSData ) ) );
    REQUIRE_EQ( Data, string( (const char*)m_buffer.base, m_buffer.elem_count ) );
    REQUIRE_EQ( 0, memcmp( & cs0, & cs, sizeof( KColumnBlobCSData ) ) );
}

// always succeeds
FIXTURE_TEST_CASE(KTextColumnBlob_Validate, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    REQUIRE_RC( KColumnBlobValidate ( m_blob ) );
}

// only checks size
FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_BufferNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    KColumnBlobCSData cs;
    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, nullptr, &cs, 0 ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_CsNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, &m_buffer, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_SizeUnder, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    KDataBufferMakeBytes( & m_buffer, Data.size() - 1 ) ;
    KColumnBlobCSData cs;

    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, &m_buffer, &cs, 0 ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_SizeOver, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    KDataBufferMakeBytes( & m_buffer, Data.size() + 1 ) ;
    KColumnBlobCSData cs;

    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, &m_buffer, &cs, 0 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    KDataBufferMakeBytes( & m_buffer, Data.size() ) ;
    KColumnBlobCSData cs;

    REQUIRE_RC( KColumnBlobValidateBuffer ( m_blob, &m_buffer, &cs, 0 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_IdRange_FirstNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    uint32_t count;
    REQUIRE_RC_FAIL( KColumnBlobIdRange ( m_blob, nullptr, &count ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_IdRange_CountNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    int64_t first;
    REQUIRE_RC_FAIL( KColumnBlobIdRange ( m_blob, &first, nullptr ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_IdRange, KTextColumnBlob_ApiFixture)
{
    Setup( Data );
    int64_t first = 0;
    uint32_t count = 0;
    REQUIRE_RC( KColumnBlobIdRange ( m_blob, &first, &count ) );
    REQUIRE_EQ( (int64_t)1, first );
    REQUIRE_EQ( (uint32_t)2, count );
}

//KTextColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

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

const char UsageDefaultName[] = "Test_KDBText_Column";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KTextColumnBlobTestSuite(argc, argv);
    return rc;
}

}
