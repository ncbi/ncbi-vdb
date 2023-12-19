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
* Unit tests of functions potentially implemented using asm
*
* For now, only cover operations with jumps inside (to test transition from relative offsets to labels)
*
*/

#include <ktst/unit_test.hpp>

#include <arch-impl.h>

#include <iostream>

using namespace std;

TEST_SUITE(Int128TestSuite);

// 128 bit operations

// special case, shift by 0
TEST_CASE(shr_0)
{
    uint64_t HI = 0x90109da2e0000;
    uint64_t LO = 0xc0e6b00000ce76e;
    uint128_t acc;
    uint128_sethi( & acc, HI );
    uint128_setlo( & acc, LO);

    uint128_shr ( & acc, 0 );

    REQUIRE_EQ ( HI, uint128_hi ( & acc ) );
    REQUIRE_EQ ( LO, uint128_lo ( & acc ) );
}
TEST_CASE(shl_0)
{
    uint64_t HI = 0x90109da2e0000;
    uint64_t LO = 0xc0e6b00000ce76e;
    uint128_t acc;
    uint128_sethi( & acc, HI );
    uint128_setlo( & acc, LO);

    uint128_shl ( & acc, 0 );

    REQUIRE_EQ ( HI, uint128_hi ( & acc ) );
    REQUIRE_EQ ( LO, uint128_lo ( & acc ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-int128";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=Int128TestSuite(argc, argv);
    return rc;
}

}


