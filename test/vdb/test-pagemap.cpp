// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================


#include <ktst/unit_test.hpp>

extern "C" {
    #include <../libs/vdb/page-map.h>
}

using namespace std;

TEST_SUITE( VdbPageMapTestSuite );

TEST_CASE ( MakeRelease )
{
    PageMap *pm;
    const uint32_t Reserve = 1UL<<31 - 1;

    REQUIRE_RC( PageMapNew( &pm, Reserve) );
    REQUIRE_EQUAL( uint64_t( 32 ), pm->cstorage.elem_bits );
    REQUIRE_EQUAL( uint64_t( Reserve ) * 3, pm->cstorage.elem_count );
    REQUIRE_EQUAL( uint8_t( 0 ), pm->cstorage.bit_offset );

    REQUIRE_EQUAL( false, pm -> random_access );

    REQUIRE_EQUAL( pm_size_t( 0 ), pm -> leng_recs );
    REQUIRE_EQUAL( pm_size_t( Reserve ), pm -> reserve_leng );
    REQUIRE_NOT_NULL( pm -> length ); // always allocated with capacity of at least 1
    REQUIRE_NOT_NULL( pm -> leng_run ); // always allocated with capacity of at least 1

    REQUIRE_EQUAL( pm_size_t( 0 ), pm -> data_recs );
    REQUIRE_NOT_NULL( pm -> data_run ); // always allocated with capacity of at least 1
    REQUIRE_EQUAL( pm_size_t( Reserve ), pm -> reserve_data );
    REQUIRE_NULL( pm -> data_offset ); // only allocated if random_access

    REQUIRE_EQUAL( pm_size_t( 0 ), pm -> start_valid );

    REQUIRE_EQUAL( row_count_t( 0 ), pm -> row_count );
    REQUIRE_EQUAL( row_count_t( 0 ), pm -> pre_exp_row_count );

// //	y->istorage.elem_bits = sizeof(PageMapRegion)*8;
// //	y->dstorage.elem_bits = sizeof(elem_count_t)*8;

//     PageMapRegion		*exp_rgn_last;
//     row_count_t			exp_row_last;   /* last row analyzed for region expansion */
//     row_count_t			exp_lr_used;    /* how much leng_run was used from the */
//     pm_size_t			exp_lr_last;	/* index of last leng_run expanded */
//     pm_size_t			exp_dr_last;	/* index of last data_run expanded */
//     pm_size_t			exp_rgn_cnt;    /* current number of expanded regions */
//     elem_count_t		exp_data_offset_last;/* last offset into data */


//     KDataBuffer			istorage;	/* binary searchable storage for expansion regions */
//     KDataBuffer			dstorage;	/* storage for expanded data */
// /** LAST SEARCH CONTROL *****/
//     pm_size_t			i_rgn_last; 	/* region index found in previous lookup **/
//     PageMapRegion*		rgn_last; 	/* redundant - region found in previous lookup **/

    REQUIRE_RC( PageMapRelease( pm ) );
}
//#endif

TEST_CASE ( PageMap_MaxReserve )
{   // VDB-4986
    PageMap *pm;
    const uint32_t Reserve = 1UL<<31; // too much
    REQUIRE_RC_FAIL( PageMapNew( &pm, Reserve) );
}

TEST_CASE ( PageMap_AppendRows )
{   // VDB-4897
    PageMap *pm;
    const uint32_t Reserve = 1UL<<31 - 1;
    REQUIRE_RC( PageMapNew( &pm, Reserve) );

    const uint64_t row_length = 1UL << 30;
    const uint64_t run_length = 1;
    REQUIRE_RC( PageMapAppendRows( pm, row_length, run_length, false) );

    REQUIRE_RC( PageMapRelease( pm ) );
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

const char UsageDefaultName[] = "test-pagemap";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbPageMapTestSuite(argc, argv);
    return rc;
}

}
