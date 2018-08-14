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

#include <ktst/unit_test.hpp> /* TEST_SUITE */
#include <kapp/main.h> /* KAppVersion */

#include <kfs/directory.h>
#include <vdb/manager.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <insdc/insdc.h>

/*****************************************************************************
    This test 
*****************************************************************************/

ver_t CC KAppVersion( void ) { return 0; }
rc_t CC Usage( const Args * args ) { return 0; }
const char UsageDefaultName[] = "";
rc_t UsageSummary( const char * progname ) { return 0; }

TEST_SUITE( SynQualityTestSuite );

TEST_CASE( total_lt_spotlen_read_2 )
{
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );
 
    const VDBManager * mgr;
    REQUIRE_RC( VDBManagerMakeRead ( &mgr, dir ) );

    const VTable * tbl;
    REQUIRE_RC( VDBManagerOpenTableRead( mgr, &tbl, NULL, "./test_syn_qual/data.test" ) );
    
    const VCursor * cur;
    REQUIRE_RC( VTableCreateCursorRead( tbl, &cur ) );

    uint32_t idx_READ_LEN, idx_READ_FILTER, idx_QUALITY;

    REQUIRE_RC( VCursorAddColumn( cur, & idx_READ_LEN, "READ_LEN" ) );
    REQUIRE_RC( VCursorAddColumn( cur, & idx_READ_FILTER, "READ_FILTER" ) );
    REQUIRE_RC( VCursorAddColumn( cur, & idx_QUALITY, "QUALITY" ) );
    REQUIRE_RC( VCursorOpen ( cur ) );
    
    int64_t first, row_id;
    uint64_t count, i;
    REQUIRE_RC( VCursorIdRange( cur, idx_READ_LEN, &first, &count ) );
    REQUIRE( count > 0 );
    
    uint32_t boff, elem_bits;
    INSDC_coord_len total_read_len;
    
    uint32_t row_len_READ_LEN;
    const INSDC_coord_len * READ_LEN;

    uint32_t row_len_READ_FILTER;
    const INSDC_read_filter * READ_FILTER;

    uint32_t row_len_QUALITY;
    const INSDC_quality_phred * QUALITY;
    
    for( row_id = first, i = 0; i < count; row_id++, i++ )
    {
        REQUIRE_RC( VCursorCellDataDirect( cur, row_id, idx_READ_LEN, &elem_bits,
                        ( const void** )&READ_LEN, &boff, &row_len_READ_LEN ) );
        REQUIRE( boff == 0 );
        REQUIRE( elem_bits == ( sizeof READ_LEN[ 0 ] ) * 8 );
        total_read_len = 0;
        for ( uint32_t idx = 0; idx < row_len_READ_LEN; ++idx )
            total_read_len += READ_LEN[ idx ];
        
        REQUIRE_RC( VCursorCellDataDirect( cur, row_id, idx_READ_FILTER, &elem_bits,
                        ( const void** )&READ_FILTER, &boff, &row_len_READ_FILTER ) );
        REQUIRE( boff == 0 );
        REQUIRE( elem_bits == ( sizeof READ_FILTER[ 0 ] ) * 8 );
        
        REQUIRE_RC( VCursorCellDataDirect( cur, row_id, idx_QUALITY, &elem_bits,
                        ( const void** )&QUALITY, &boff, &row_len_QUALITY ) );
        REQUIRE( boff == 0 );
        REQUIRE( elem_bits == ( sizeof QUALITY[ 0 ] ) * 8 );
        REQUIRE( total_read_len == row_len_QUALITY );
        
        if ( total_read_len > 0 )
        {
            INSDC_quality_phred * computed = ( INSDC_quality_phred * )malloc( total_read_len );
            REQUIRE( computed != NULL );
            
            INSDC_quality_phred * dst = computed;
            if ( row_len_READ_FILTER == 0 )
            {
                memset( dst, 30, total_read_len );
            }
            else
            {
                for ( uint32_t idx = 0; idx < row_len_READ_LEN; ++idx )
                {
                    INSDC_coord_len len = READ_LEN[ idx ];
                    INSDC_quality_phred q = 30;

                    if ( idx < row_len_READ_FILTER )
                    {
                        if ( READ_FILTER[ idx ] == READ_FILTER_REJECT )
                            q = 3;
                    }
                    memset( dst, q, len );
                    dst += len;
                }
            }
            int cmp = memcmp( QUALITY, computed, total_read_len );
            REQUIRE( cmp == 0 );
            free( computed );
        }
    }
    
    REQUIRE_RC( VCursorRelease( cur ) );
    REQUIRE_RC( VTableRelease( tbl ) );
    REQUIRE_RC( VDBManagerRelease( mgr ) );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

rc_t CC KMain ( int argc, char *argv [] )
{
    return SynQualityTestSuite( argc, argv );
}
