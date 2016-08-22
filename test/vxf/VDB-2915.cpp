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
#include <vdb/manager.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>

#include <kdb/table.h>

#include <ktst/unit_test.hpp>
#include <klib/out.h>
#include <klib/rc.h>

#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <sysalloc.h>

using namespace std;

TEST_SUITE( VDB_2915_TEST_SUITE );

/* --------------------------------------------------------------------------
There was a problem in the irzip, izip and fzip encoding :

If the data was too random, the library was not able to load the data.

The reason for that was a copy-paste error which has now been fixed.

This test creates a temporary output-tabe, and tries to load random
data into it. This has to succeed!
--------------------------------------------------------------------------*/

const string ScratchDir = "./";

static void fill_random_uint8_buffer( uint8_t * buffer, size_t value_count )
{
    for ( size_t i = 0; i < value_count; ++i )
        buffer[ i ] = rand() & 0xFF;
}


static void fill_random_uint16_buffer( uint16_t * buffer, size_t value_count )
{
    for ( size_t i = 0; i < value_count; ++i )
        buffer[ i ] = rand() & 0xFFFF;
}

static void fill_random_uint32_buffer( uint32_t * buffer, size_t value_count )
{
    for ( size_t i = 0; i < value_count; ++i )
        buffer[ i ] = rand() & 0xFFFFFFFF;
}

static void fill_random_uint64_buffer( uint64_t * buffer, size_t value_count )
{
    for ( size_t i = 0; i < value_count; ++i )
    {
        uint64_t v = rand();
        buffer[ i ] = v * rand();
    }
}

/*
typedef union
{
  float f;
  struct
  {
    unsigned int mantisa : 23;
    unsigned int exponent : 8;
    unsigned int sign : 1;
  } parts;
} double_cast;

static float random_float( void )
{
    double_cast x;
    x.parts.mantisa = rand() & 0x7FFFFF;
    x.parts.exponent = rand() & 0xFF;
    x.parts.sign = rand() & 0x1;
    return x.f;
}

static void fill_random_float_buffer( float * buffer, size_t value_count )
{
    for ( size_t i = 0; i < value_count; ++i )
        buffer[ i ] = random_float();
}
*/

class WVDB_Fixture
{
public:
    WVDB_Fixture() : m_tableName( "RANDOM_DATA" ), m_table( 0 ), m_cursor( 0 ) { }
    
    ~WVDB_Fixture()
    {
        if ( m_cursor ) { VCursorRelease( m_cursor ); }
        if ( m_table ) { VTableRelease( m_table ); }
    }

    void RemoveTable()
    {
        if ( ! m_tableName . empty () )
        {
            KDirectory *wd;
            KDirectoryNativeDir( &wd );
            KDirectoryRemove( wd, true, m_tableName.c_str() );
            KDirectoryRelease( wd );
        }
    }

    void MakeTable( const string& p_schemaText, const string& p_schemaSpec )
    {
        RemoveTable();

        VDBManager * mgr;
        THROW_ON_RC( VDBManagerMakeUpdate ( &mgr, NULL ) );
        VSchema *schema;
        THROW_ON_RC( VDBManagerMakeSchema ( mgr, &schema ) );
        THROW_ON_RC( VSchemaParseText( schema, NULL, p_schemaText.c_str(), p_schemaText.size() ) );
        THROW_ON_RC( VDBManagerCreateTable( mgr,
                                            &m_table,
                                            schema,
                                            p_schemaSpec.c_str(),
                                            kcmInit + kcmMD5,
                                            "%s",
                                            m_tableName.c_str() ) );
        THROW_ON_RC( VTableCreateCursorWrite( m_table, &m_cursor, kcmInsert ) );
        THROW_ON_RC( VSchemaRelease( schema ) );
        THROW_ON_RC( VDBManagerRelease( mgr ) );
    }

    void OpenTable()
    {
        VDBManager * mgr;
        THROW_ON_RC( VDBManagerMakeUpdate ( &mgr, NULL ) );
        THROW_ON_RC( VDBManagerOpenTableRead( mgr,
                                              ( const VTable ** )&m_table,
                                              NULL,
                                              "%s",
                                              m_tableName.c_str() ) );
        THROW_ON_RC( VTableCreateCursorRead( m_table, ( const VCursor ** )&m_cursor ) );
        THROW_ON_RC( VDBManagerRelease( mgr ) );
    }
    
    string m_tableName;
    VTable * m_table;
    VCursor * m_cursor;
};


FIXTURE_TEST_CASE ( LOAD_RANDOM_DATA, WVDB_Fixture )
{
    RemoveTable();
    
    string schemaText = "version 1;"
                        "include 'vdb/vdb.vschema';"
                        "table A_TABLE #1.0"
                        "{"
                        "   column < U8 > izip_encoding C1;"
                        "   column U8 R1;"                        
                        "   column < U16 > izip_encoding C2;"
                        "   column U16 R2;"
                        "   column < U32 > izip_encoding C3;"
                        "   column U32 R3;"
                        "   column < U64 > izip_encoding C4;"
                        "   column U64 R4;"
                        "   column < F32 > fzip_encoding< 23 > F5;"
                        "   column F32 R5;"
                        "   column < F64 > fzip_encoding< 52 > F6;"
                        "   column F64 R6;"
                        "};";

    REQUIRE_RC( KOutMsg( "creating table '%s'\n", m_tableName.c_str() ) );
    
    MakeTable ( schemaText, "A_TABLE" );
    
    uint32_t c1, c2, c3, c4;
    uint32_t r1, r2, r3, r4;

    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c1, "%s", "C1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c2, "%s", "C2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c3, "%s", "C3" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c4, "%s", "C4" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r1, "%s", "R1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r2, "%s", "R2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r3, "%s", "R3" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r4, "%s", "R4" ) );

    REQUIRE_RC ( VCursorOpen ( m_cursor ) );

    srand( 123456 );
    for ( int64_t i = 1; i <= 512; ++i )
    {
        REQUIRE_RC ( VCursorOpenRow ( m_cursor ) );
        
        uint8_t src_1[ 1024 ];
        fill_random_uint8_buffer( src_1, 1024 );
        REQUIRE_RC ( VCursorWrite ( m_cursor, c1, 8, src_1, 0, 1024 ) );
        REQUIRE_RC ( VCursorWrite ( m_cursor, r1, 8, src_1, 0, 1024 ) );        
        
        uint16_t src_2[ 1024 ];
        fill_random_uint16_buffer( src_2, 1024 );
        REQUIRE_RC ( VCursorWrite ( m_cursor, c2, 16, src_2, 0, 1024 ) );
        REQUIRE_RC ( VCursorWrite ( m_cursor, r2, 16, src_2, 0, 1024 ) );
        
        uint32_t src_3[ 1024 ];
        fill_random_uint32_buffer( src_3, 1024 );
        REQUIRE_RC ( VCursorWrite ( m_cursor, c3, 32, src_3, 0, 1024 ) );
        REQUIRE_RC ( VCursorWrite ( m_cursor, r3, 32, src_3, 0, 1024 ) );

        uint64_t src_4[ 1024 ];
        fill_random_uint64_buffer( src_4, 1024 );
        REQUIRE_RC ( VCursorWrite ( m_cursor, c4, 64, src_4, 0, 1024 ) );
        REQUIRE_RC ( VCursorWrite ( m_cursor, r4, 64, src_4, 0, 1024 ) );

        REQUIRE_RC ( VCursorCommitRow ( m_cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( m_cursor ) );
    }
    REQUIRE_RC ( VCursorCommit ( m_cursor ) );
}

FIXTURE_TEST_CASE ( CHECK_RANDOM_DATA, WVDB_Fixture )
{
    OpenTable();

    REQUIRE_RC( KOutMsg( "verify table '%s'\n", m_tableName.c_str() ) );
    
    uint32_t c1, c2, c3, c4;
    uint32_t r1, r2, r3, r4;
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c1, "%s", "C1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c2, "%s", "C2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c3, "%s", "C3" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &c4, "%s", "C4" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r1, "%s", "R1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r2, "%s", "R2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r3, "%s", "R3" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &r4, "%s", "R4" ) );

    REQUIRE_RC ( VCursorOpen ( m_cursor ) );
    
    for ( int64_t i = 1; i <= 512; ++i )
    {
        uint32_t c_bits, r_bits, c_boff, r_boff, c_len, r_len;
        {
            uint8_t * c;
            uint8_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, c1, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, r1, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )8 );
            REQUIRE_EQ ( r_bits, ( uint32_t )8 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }

        {
            uint16_t * c;
            uint16_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, c2, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, r2, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )16 );
            REQUIRE_EQ ( r_bits, ( uint32_t )16 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }
        
        {
            uint32_t * c;
            uint32_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, c3, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, r3, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )32 );
            REQUIRE_EQ ( r_bits, ( uint32_t )32 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }

        {
            uint64_t * c;
            uint64_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, c4, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, r4, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )64 );
            REQUIRE_EQ ( r_bits, ( uint32_t )64 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }
        
    }
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char * progname ) { return 0; }
rc_t CC Usage ( const Args * args ){ return 0; }

const char UsageDefaultName[] = "vdb_2915_testwb-test-vxf";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc = VDB_2915_TEST_SUITE( argc, argv );
    return rc;
}

}
