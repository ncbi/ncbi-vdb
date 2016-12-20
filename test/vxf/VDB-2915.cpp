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
#include <vdb/vdb-priv.h>

#include <kfg/config.h>
#include <vfs/manager.h>
#include <vfs/manager-priv.h>

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

static int32_t rand_int32()
{
    int32_t res = rand();
    if ( rand() & 1 ) res *= -1;
    return res;
}

/*
static int64_t rand_int64()
{
    int64_t res = rand();
    res *= rand();
    if ( rand() & 1 ) res *= -1;
    return res;
}

static uint64_t rand_uint64()
{
    uint64_t res = rand();
    res *= rand();
    res &= 0x0FFFFFFFFFFFFFFF; // JOJOBA: kinda strange :)
    // res &= 0x7FFFFFFFFFFFFFFF;
    return res;
}
*/

class WVDB_Fixture
{
    KDirectory * _wd;
    KConfig * _cfg;
public:
    WVDB_Fixture() : _wd ( NULL ), _cfg ( NULL ), m_remove( false )
        , m_tableName( "RANDOM_DATA" ), m_table( 0 ), m_cursor( 0 )
    {
        THROW_ON_RC( KDirectoryNativeDir( &_wd ) );
        
        const KDirectory * dir = NULL;
        THROW_ON_RC
            ( KDirectoryOpenDirRead ( _wd, &dir, false, "local_config" ) );
        
        THROW_ON_RC( KConfigMake ( &_cfg, dir ) );
        
        KConfigPrint( _cfg, 0 ); /* for test */

        THROW_ON_RC( KDirectoryRelease( dir ) );
    }
    
    ~WVDB_Fixture()
    {
        if ( m_cursor ) { VCursorRelease( m_cursor ); }
        if ( m_table ) { VTableRelease( m_table ); }
        if ( m_remove ) { RemoveTable(); }
        KConfigRelease( _cfg );
        KDirectoryRelease( _wd );        
    }

    void RemoveTable()
    {
        if ( ! m_tableName . empty () )
        {
            KDirectoryRemove( _wd, true, m_tableName.c_str() );
        }
    }

    void MakeTable( const string& p_schemaText, const string& p_schemaSpec )
    {
        RemoveTable();

        struct VFSManager * vfs_mgr;
        THROW_ON_RC( VFSManagerMakeFromKfg ( &vfs_mgr, _cfg ) );
        
        VDBManager * mgr;
        THROW_ON_RC
            ( VDBManagerMakeUpdateWithVFSManager ( &mgr, _wd, vfs_mgr ) );
        
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
        THROW_ON_RC( VFSManagerRelease( vfs_mgr ) );
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
    
    bool m_remove;
    string m_tableName;
    VTable * m_table;
    VCursor * m_cursor;
};

/*
        we have to exclude 64-bit tests, because the write-lib is broken for irzip
        if feed with random data
*/

FIXTURE_TEST_CASE ( LOAD_RANDOM_DATA, WVDB_Fixture )
{
    RemoveTable();
    
    string schemaText = "version 1;"
                        "include 'vdb/vdb.vschema';"
                        "table A_TABLE #1.0"
                        "{"
                        "   column < U8 > izip_encoding CU1;"
                        "   column U8 RU1;"
                        "   column < U16 > izip_encoding CU2;"
                        "   column U16 RU2;"
                        "   column < U32 > izip_encoding CU3;"
                        "   column U32 RU3;"
                        "   column < U64 > izip_encoding CU4;"
                        "   column U64 RU4;"
                        
                        "   column < I8 > izip_encoding CI1;"
                        "   column I8 RI1;"                        
                        "   column < I16 > izip_encoding CI2;"
                        "   column I16 RI2;"
                        "   column < I32 > izip_encoding CI3;"
                        "   column I32 RI3;"
                        "   column < I64 > izip_encoding CI4;"
                        "   column I64 RI4;"
                        "};";

    REQUIRE_RC( KOutMsg( "creating table '%s'\n", m_tableName.c_str() ) );
    
    MakeTable ( schemaText, "A_TABLE" );
    
    uint32_t cu1, cu2, cu3 /*, cu4 */;
    uint32_t ru1, ru2, ru3 /*, ru4 */;

    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu1, "%s", "CU1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu2, "%s", "CU2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu3, "%s", "CU3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu4, "%s", "CU4" ) ); */
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru1, "%s", "RU1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru2, "%s", "RU2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru3, "%s", "RU3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru4, "%s", "RU4" ) ); */

    uint32_t ci1, ci2, ci3 /*, ci4 */;
    uint32_t ri1, ri2, ri3 /*, ri4 */;

    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci1, "%s", "CI1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci2, "%s", "CI2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci3, "%s", "CI3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci4, "%s", "CI4" ) ); */
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri1, "%s", "RI1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri2, "%s", "RI2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri3, "%s", "RI3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri4, "%s", "RI4" ) ); */

    REQUIRE_RC ( VCursorOpen ( m_cursor ) );

    srand( 123456 );
    
    size_t i, j;
    for ( i = 1; i <= 512; ++i )
    {
        REQUIRE_RC ( VCursorOpenRow ( m_cursor ) );
        
        {
            uint8_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand() & 0xFF;
            REQUIRE_RC ( VCursorWrite ( m_cursor, cu1, 8, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ru1, 8, b, 0, 1024 ) );
        }
        
        {
            int8_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand() & 0xFF;
            REQUIRE_RC ( VCursorWrite ( m_cursor, ci1, 8, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ri1, 8, b, 0, 1024 ) );
        }

        {
            uint16_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand() & 0xFFFF;
            REQUIRE_RC ( VCursorWrite ( m_cursor, cu2, 16, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ru2, 16, b, 0, 1024 ) );
        }

        {
            int16_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand() & 0xFFFF;
            REQUIRE_RC ( VCursorWrite ( m_cursor, ci2, 16, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ri2, 16, b, 0, 1024 ) );
        }

        {
            uint32_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand();
            REQUIRE_RC ( VCursorWrite ( m_cursor, cu3, 32, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ru3, 32, b, 0, 1024 ) );
        }
        
        {
            int32_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand_int32();
            REQUIRE_RC ( VCursorWrite ( m_cursor, ci3, 32, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ri3, 32, b, 0, 1024 ) );
        }

        /*
        {
            uint64_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand_uint64();
            REQUIRE_RC ( VCursorWrite ( m_cursor, cu4, 64, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ru4, 64, b, 0, 1024 ) );
        }

        {
            int64_t b[ 1024 ];
            for ( j = 0; j < 1024; ++j ) b[ j ] = rand_int64();
            REQUIRE_RC ( VCursorWrite ( m_cursor, ci4, 64, b, 0, 1024 ) );
            REQUIRE_RC ( VCursorWrite ( m_cursor, ri4, 64, b, 0, 1024 ) );
        }
        */
        
        REQUIRE_RC ( VCursorCommitRow ( m_cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( m_cursor ) );
    }
    REQUIRE_RC ( VCursorCommit ( m_cursor ) );
}

FIXTURE_TEST_CASE ( CHECK_RANDOM_DATA, WVDB_Fixture )
{
    OpenTable();

    REQUIRE_RC( KOutMsg( "verify table '%s'\n", m_tableName.c_str() ) );
    
    uint32_t cu1, cu2, cu3 /*, cu4 */;
    uint32_t ru1, ru2, ru3 /*, ru4 */;
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu1, "%s", "CU1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu2, "%s", "CU2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu3, "%s", "CU3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &cu4, "%s", "CU4" ) ); */
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru1, "%s", "RU1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru2, "%s", "RU2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru3, "%s", "RU3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ru4, "%s", "RU4" ) ); */

    uint32_t ci1, ci2, ci3 /*, ci4 */;
    uint32_t ri1, ri2, ri3 /*, ri4 */;
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci1, "%s", "CI1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci2, "%s", "CI2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci3, "%s", "CI3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ci4, "%s", "CI4" ) ); */
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri1, "%s", "RI1" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri2, "%s", "RI2" ) );
    REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri3, "%s", "RI3" ) );
    /* REQUIRE_RC ( VCursorAddColumn ( m_cursor, &ri4, "%s", "RI4" ) ); */

    REQUIRE_RC ( VCursorOpen ( m_cursor ) );
    
    for ( int64_t i = 1; i <= 512; ++i )
    {
        uint32_t c_bits, r_bits, c_boff, r_boff, c_len, r_len;
        {
            uint8_t * c;
            uint8_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, cu1, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ru1, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )8 );
            REQUIRE_EQ ( r_bits, ( uint32_t )8 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }

        {
            int8_t * c;
            int8_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ci1, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ri1, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
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
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, cu2, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ru2, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )16 );
            REQUIRE_EQ ( r_bits, ( uint32_t )16 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }
        
        {
            int16_t * c;
            int16_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ci2, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ri2, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
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
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, cu3, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ru3, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )32 );
            REQUIRE_EQ ( r_bits, ( uint32_t )32 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }

        {
            int32_t * c;
            int32_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ci3, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ri3, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )32 );
            REQUIRE_EQ ( r_bits, ( uint32_t )32 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }

        /*
        {
            uint64_t * c;
            uint64_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, cu4, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ru4, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )64 );
            REQUIRE_EQ ( r_bits, ( uint32_t )64 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }

        {
            int64_t * c;
            int64_t * r;
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ci4, &c_bits, ( const void** )&c, &c_boff, &c_len ) );
            REQUIRE_RC ( VCursorCellDataDirect ( m_cursor, i, ri4, &r_bits, ( const void** )&r, &r_boff, &r_len ) );
            REQUIRE_EQ ( c_bits, ( uint32_t )64 );
            REQUIRE_EQ ( r_bits, ( uint32_t )64 );
            REQUIRE_EQ ( c_boff, ( uint32_t )0 );
            REQUIRE_EQ ( r_boff, ( uint32_t )0 );
            REQUIRE_EQ ( c_len, ( uint32_t )1024 );
            REQUIRE_EQ ( r_len, ( uint32_t )1024 );
            REQUIRE ( std::equal( c, c + 1024, r ) );
        }
        */
    }
    
    m_remove = true;
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
