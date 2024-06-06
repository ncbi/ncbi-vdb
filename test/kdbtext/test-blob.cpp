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
#include <kdb/page-map.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextColumnBlobTestSuite);

class KTextColumnBlob_Fixture
{
public:
    KTextColumnBlob_Fixture()
    {
    }
    ~KTextColumnBlob_Fixture()
    {
        KJsonValueWhack( m_json );
        delete m_blob;
    }

    void Setup( const string & p_json )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, p_json.c_str(), m_error, sizeof( m_error ) ) );
        m_blob = new ColumnBlob( m_json );
    }

    char m_error[1024];
    KJsonValue * m_json = nullptr;
    ColumnBlob * m_blob = nullptr;
};


FIXTURE_TEST_CASE(KTextColumnBlob_inflate_NotObject, KTextColumnBlob_Fixture)
{
    Setup(R"( 1 )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_Empty, KTextColumnBlob_Fixture)
{
    Setup(R"( {} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_RowIdBad, KTextColumnBlob_Fixture)
{
    Setup(R"( {"row":"a","value":"q"} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_StartBad, KTextColumnBlob_Fixture)
{
    Setup(R"( {"start":"a","value":"q"} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_CountBad, KTextColumnBlob_Fixture)
{
    Setup(R"( {"start":1,"count":"a","value":"q"} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_CountLessThan1, KTextColumnBlob_Fixture)
{
    Setup(R"( {"start":1,"count":0,"value":"q"} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_CellMake_ValueMissing, KTextColumnBlob_Fixture)
{
    Setup(R"( {"row":"1"} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_ValueBad, KTextColumnBlob_Fixture)
{
    Setup(R"( {"row":1,"value":null} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_FromString, KTextColumnBlob_Fixture)
{
    Setup(R"( {"row":1,"value":"qq"} )");
    REQUIRE_RC( m_blob -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( 1, (int)m_blob->getIdRange().first );
    REQUIRE_EQ( 1, (int)m_blob->getIdRange().second );
    REQUIRE_EQ( 1, (int)m_blob->getPageMap().row_count );

    const KDataBuffer & buf = m_blob->getData();
    REQUIRE_EQ( 2, (int)buf.elem_count );
    REQUIRE_EQ( string("qq"), string( (const char*)buf.base, 2 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_inflate_Repeat, KTextColumnBlob_Fixture)
{
    Setup(R"( {"row":2,"value":"qq","count":3} )");
    REQUIRE_RC( m_blob -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( 2, (int)m_blob->getIdRange().first );
    REQUIRE_EQ( 3, (int)m_blob->getIdRange().second );
    REQUIRE_EQ( 3, (int)m_blob->getPageMap().row_count );

    const KDataBuffer & buf = m_blob->getData();
    REQUIRE_EQ( 2, (int)buf.elem_count );
    REQUIRE_EQ( string("qq"), string( (const char*)buf.base, 2 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_inflate_FromArray, KTextColumnBlob_Fixture)
{   // count, if specified, has to match the length of the array
    Setup(R"( {"row":1,"count":2,"value":["q","r"]} )");
    REQUIRE_RC( m_blob -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( 1, (int)m_blob->getIdRange().first );
    REQUIRE_EQ( 2, (int)m_blob->getIdRange().second );
    REQUIRE_EQ( 2, (int)m_blob->getPageMap().row_count );

    const KDataBuffer & buf = m_blob->getData();
    REQUIRE_EQ( 2, (int)buf.elem_count );
    REQUIRE_EQ( string("qr"), string( (const char*)buf.base, 2 ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_inflate_FromArray_NoCount, KTextColumnBlob_Fixture)
{   // count can be omitted
    Setup(R"( {"row":1,"value":["q","r"]} )");
    REQUIRE_RC( m_blob -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( 1, (int)m_blob->getIdRange().first );
}

FIXTURE_TEST_CASE(KTextColumnBlob_inflate_FromArray_CountMismatch, KTextColumnBlob_Fixture)
{
    Setup(R"( {"row":1,"count":3,"value":["q","r"]} )");
    REQUIRE_RC_FAIL( m_blob -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

#if 0
//TODO
FIXTURE_TEST_CASE(KTextColumnBlob_AppendRow_TwoUnderbytes, KTextColumnBlob_Fixture)
{
    m_blob = new ColumnBlob( 1 );
    uint8_t b0= 0b10101000; // 5 high bits used
    uint8_t b1= 0b01010000; // 4 high bits used
    REQUIRE_RC( m_blob -> appendRow( &b0, 5, 1 ) );
    REQUIRE_RC( m_blob -> appendRow( &b1, 4, 1 ) );
    REQUIRE_EQ( 1, (int)m_blob->getIdRange().first );
    REQUIRE_EQ( 2, (int)m_blob->getIdRange().second );
    REQUIRE_EQ( 2, (int)m_blob->getPageMap().row_count );
    REQUIRE_EQ( 9, (int)m_blob->getData().elem_count ); // total size in bits
    REQUIRE_EQ( (int)(uint8_t)0b10101010, (int)((uint8_t*)m_blob->getData().base)[0] );
    REQUIRE_EQ( (int)(uint8_t)0b10000000, (int)((uint8_t*)m_blob->getData().base)[1] & 0b10000000 );
}
#endif

class KTextColumnBlob_ApiFixture
{
public:
    KTextColumnBlob_ApiFixture()
    {
    }
    ~KTextColumnBlob_ApiFixture()
    {
        KColumnBlobRelease( m_blob );
        KJsonValueWhack( m_json );
    }
    void Setup( const string & data )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, data.c_str(), m_error, sizeof( m_error ) ) );
        ColumnBlob * cb = new ColumnBlob( m_json );
        THROW_ON_RC( cb -> inflate( m_error, sizeof m_error ) );
        m_blob = (const KColumnBlob*)cb;
    }

    const KColumnBlob * m_blob = nullptr;
    char m_error[1024];

    KJsonValue * m_json = nullptr;
    uint8_t m_buffer[1024];

    size_t num_read = 0;
    size_t remaining = 0;
};

FIXTURE_TEST_CASE(KTextColumnBlob_AddRelease, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":"q"} )");

    REQUIRE_RC( KColumnBlobAddRef( m_blob ) );
    REQUIRE_RC( KColumnBlobRelease( m_blob ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_BufferNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":"q"} )");

    REQUIRE_RC_FAIL( KColumnBlobRead ( m_blob, 0, nullptr, 1, &num_read, &remaining ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_Offset, KTextColumnBlob_ApiFixture)
{   // non-0 offsets are not supported
    Setup(R"( {"row":1,"value":"q"} )");
    REQUIRE_RC_FAIL( KColumnBlobRead ( m_blob, 1, m_buffer, sizeof( m_buffer ), &num_read, &remaining ) );
}

#if 0
//TODO: can we have a 0-length blob?
FIXTURE_TEST_CASE(KTextColumnBlob_BufferNull_SizeNull, KTextColumnBlob_ApiFixture)
{
    Setup( Data );

    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, nullptr, 0, &num_read, &remaining ) );
    REQUIRE_EQ( (size_t)0, num_read );
    REQUIRE_EQ( 2 + Data.size(), remaining );
}
#endif

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Data_RowLength_8bits, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":"q"} )");
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, m_buffer, sizeof( m_buffer ), &num_read, &remaining ) );
    REQUIRE_EQ( 3, (int)num_read );
    REQUIRE_EQ( 0, (int)remaining );

    // fixed row length header:
    // High 7 bits of the 1st byte: length < 0x100 bits (000), no extra bits (000), little endian (10)
    REQUIRE_EQ( 0b00000010, (int)m_buffer[0] );
    // 2nd byte: row length in bits
    REQUIRE_EQ( 0x08, (int)m_buffer[1] );
    // 3d byte: data
    REQUIRE_EQ( (int)'q', (int)m_buffer[2] );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Data_RowLength_4bytes, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":"qwer"} )");
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, m_buffer, sizeof( m_buffer ), &num_read, &remaining ) );
    REQUIRE_EQ( 6, (int)num_read );
    REQUIRE_EQ( 0, (int)remaining );

    // fixed row length header:
    // High 7 bits of the 1st byte: length < 0x100 bits (000), no extra bits (000), little endian (10)
    REQUIRE_EQ( 0b00000010, (int)m_buffer[0] );
    // 2nd byte: row length in bits
    REQUIRE_EQ( 32, (int)m_buffer[1] );
    // bytes 3-6: data
    REQUIRE_EQ( string("qwer"), string( (char*)m_buffer + 2, 4 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Data_RowLength_variable, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, m_buffer, sizeof( m_buffer ), &num_read, &remaining ) );
    REQUIRE_EQ( 21, (int)num_read );
    REQUIRE_EQ( 0, (int)remaining );

    // fixed row length header:
    // High 7 bits of the 0 byte: little endian (1000), no extra bits (0000)
    REQUIRE_EQ( 0b10000000, (int)m_buffer[0] );
    // byte 1: header_size (0)
    REQUIRE_EQ( 0, (int)m_buffer[1] );
    // byte 2: pagemap size (8)
    REQUIRE_EQ( 8, (int)m_buffer[2] );
    // bytes 3-10: pagemap
    // bytes 11-20: data
    REQUIRE_EQ( string("qwerqwerty"), string( (char*)m_buffer + 11, 10 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_NumReadNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer"] } )");
    rc_t rc = KColumnBlobRead ( m_blob, 0, m_buffer, sizeof m_buffer, nullptr, &remaining );
    REQUIRE_EQ( SILENT_RC( rcDB, rcBlob, rcReading, rcParam, rcNull ), rc );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_From_Offset, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    REQUIRE_RC_FAIL( KColumnBlobRead ( m_blob, 1, m_buffer, sizeof m_buffer, &num_read, &remaining ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Read_BufferTooShort, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    // read the first few bytes
    const size_t ToRead = 3;
    REQUIRE_RC( KColumnBlobRead ( m_blob, 0, m_buffer, ToRead, &num_read, &remaining ) );
    REQUIRE_EQ( ToRead, num_read );
    REQUIRE_EQ( (int)(21 - ToRead), (int)remaining );
    REQUIRE_EQ( 0b10000000, (int)m_buffer[0] );
    REQUIRE_EQ( 0, (int)m_buffer[1] );
    REQUIRE_EQ( 8, (int)m_buffer[2] );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ReadAll_BufferNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer"] } )");
    REQUIRE_RC_FAIL( KColumnBlobReadAll ( m_blob, nullptr, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ReadAll_NoCS, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    KDataBuffer b;
    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & b, nullptr, 0 ) );
    REQUIRE_EQ( string("qwerqwerty"), string( (char*)b.base + 11, 10 ) );
    KDataBufferWhack( &b );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ReadAll_WithCS, KTextColumnBlob_ApiFixture)
{   // checksum data is set to all 0s
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    KDataBuffer b;
    KColumnBlobCSData cs0;
    memset ( & cs0, 0, sizeof( KColumnBlobCSData ) );
    KColumnBlobCSData cs;
    memset ( & cs, 1, sizeof( KColumnBlobCSData ) );

    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & b, & cs, sizeof( KColumnBlobCSData ) ) );
    REQUIRE_EQ( string("qwerqwerty"), string( (char*)b.base + 11, 10 ) );
    REQUIRE_EQ( 0, memcmp( & cs0, & cs, sizeof( KColumnBlobCSData ) ) );
    KDataBufferWhack( &b );
}

// always succeeds
FIXTURE_TEST_CASE(KTextColumnBlob_Validate, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    REQUIRE_RC( KColumnBlobValidate ( m_blob ) );
}

// only checks size
FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_BufferNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");

    KColumnBlobCSData cs;
    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, nullptr, &cs, 0 ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_CsNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    KDataBuffer b;
    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, &b, nullptr, 0 ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_SizeUnder, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    KDataBuffer b;
    KColumnBlobCSData cs;
    memset ( & cs, 1, sizeof( KColumnBlobCSData ) );

    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & b, & cs, sizeof( KColumnBlobCSData ) ) );

    REQUIRE_RC( KDataBufferResize( & b, KDataBufferBytes( &b ) - 1 ) );

    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, &b, &cs, 0 ) );

    KDataBufferWhack( &b );
}
FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer_SizeOver, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    KDataBuffer b;
    KColumnBlobCSData cs;
    memset ( & cs, 1, sizeof( KColumnBlobCSData ) );

    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & b, & cs, sizeof( KColumnBlobCSData ) ) );

    REQUIRE_RC( KDataBufferResize( & b, KDataBufferBytes( &b ) + 1 ) );

    REQUIRE_RC_FAIL( KColumnBlobValidateBuffer ( m_blob, &b, &cs, 0 ) );

    KDataBufferWhack( &b );
}

FIXTURE_TEST_CASE(KTextColumnBlob_ValidateBuffer, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    KDataBuffer b;
    KColumnBlobCSData cs;
    memset ( & cs, 1, sizeof( KColumnBlobCSData ) );

    REQUIRE_RC( KColumnBlobReadAll ( m_blob, & b, & cs, sizeof( KColumnBlobCSData ) ) );

    REQUIRE_RC( KColumnBlobValidateBuffer ( m_blob, &b, &cs, 0 ) );

    KDataBufferWhack( &b );
}

FIXTURE_TEST_CASE(KTextColumnBlob_IdRange_FirstNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    uint32_t count;
    REQUIRE_RC_FAIL( KColumnBlobIdRange ( m_blob, nullptr, &count ) );
}
FIXTURE_TEST_CASE(KTextColumnBlob_IdRange_CountNull, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"value":["qwer", "qwerty"] } )");
    int64_t first;
    REQUIRE_RC_FAIL( KColumnBlobIdRange ( m_blob, &first, nullptr ) );
}

FIXTURE_TEST_CASE(KTextColumnBlob_IdRange, KTextColumnBlob_ApiFixture)
{
    Setup(R"( {"row":1,"count":2,"value":["q","r"]} )");
    int64_t first = 0;
    uint32_t count = 0;
    REQUIRE_RC( KColumnBlobIdRange ( m_blob, &first, &count ) );
    REQUIRE_EQ( (int64_t)1, first );
    REQUIRE_EQ( (uint32_t)2, count );
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

const char UsageDefaultName[] = "Test_KDBText_Column";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KTextColumnBlobTestSuite(argc, argv);
    return rc;
}

}
