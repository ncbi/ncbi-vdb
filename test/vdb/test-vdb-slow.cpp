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

#include "VDB_Fixture.hpp"

#include <ktst/unit_test.hpp> // TEST_CASE
#include <kfg/config.h>

// #include <sysalloc.h>
// #include <cstdlib>
// #include <stdexcept>

using namespace std;

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( VdbSlowTestSuite, argsHandler );

FIXTURE_TEST_CASE(TestReadBitsDirect_vs_CellDataDirect, VDB_Fixture)
{   // VDB-4431
    static char const *columns[] = { "(INSDC:4na:packed)READ", 0 };  //66795559 total
    REQUIRE_RC ( Setup ( "JADQCU01", columns ) );

    uint32_t remaining_count_ReadBits;
    {
        const uint32_t elem_bits = 4;
        uint32_t read_count;
        char buffer[ 1 ];
        REQUIRE_RC (  VCursorReadBitsDirect( curs, 2, col_idx[0],
                                                elem_bits, 0, buffer, 0, 0,
                                                &read_count, &remaining_count_ReadBits) );
    }

    uint32_t remaining_count_CellData;
    {
        uint32_t elem_bits;
        const void * buffer;
        uint32_t boff;
        REQUIRE_RC ( VCursorCellDataDirect ( curs, 2, col_idx[0], &elem_bits, &buffer, &boff, &remaining_count_CellData ) );
    }

    REQUIRE_EQ( remaining_count_ReadBits, remaining_count_CellData );
}
//////////////////////////////////////////// Main
#include <kapp/args.h>

static rc_t argsHandler ( int argc, char * argv [] ) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );
    ArgsWhack ( args );
    return rc;
}

extern "C"
{

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

const char UsageDefaultName[] = "test-vdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbSlowTestSuite(argc, argv);
    return rc;
}

}
