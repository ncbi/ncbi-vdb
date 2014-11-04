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

#include <ktst/unit_test.hpp>

TEST_SUITE(VxfTestSuite);

#include "wb-irzip-impl.h"

////////////////////////////////////////// IZIP encoding tests

// test fixture for scanner tests
class EncoderFixture
{
public:
    EncoderFixture()
    : dSize(BufSize), used(0), series_count(0), planes(0)
    {
        min[0] = min[1] = 0;
        slope[0] = slope[1] = 0;
    }

    static const int BufSize = 102400;
    uint8_t dst[BufSize];
    size_t dSize;
    size_t used;
    int64_t min[2];
    int64_t slope[2];
    uint8_t series_count;
    uint8_t planes;
};

#define ARR_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define REQUIRE_EQ_ARR(a, b, size)  \
    for (size_t i=0; i < size; ++i) \
    {                               \
        REQUIRE_EQUAL(a[i], b[i]);  \
    }

// exposes an assert failure when encoding unsigned 64 bit integers (#VDB-539), case 1
FIXTURE_TEST_CASE(IRZIP_u64_assert1, EncoderFixture)
{
    uint64_t y[]={
        UINT64_C(353878216), 
        UINT64_C(353878152), 
        UINT64_C(353877873), 
        UINT64_C(353877162), 
        UINT64_C(353876785), 
        UINT64_C(353875727), 
        UINT64_C(353874605), 
        UINT64_C(353873979), 
        UINT64_C(353873604), 
        UINT64_C(353872503)
    };
    uint64_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_u64(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_u64(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}

// exposes an assert failure when encoding unsigned 64 bit integers (#VDB-539), case 2
FIXTURE_TEST_CASE(IRZIP_u64_assert2, EncoderFixture)
{
    uint64_t y[]={
        UINT64_C(353878216), 
        UINT64_C(353878152), 
        UINT64_C(        0), 
        UINT64_C(        0), 
        UINT64_C(        0), 
        UINT64_C(        0), 
        UINT64_C(        0), 
        UINT64_C(        0), 
        UINT64_C(        0), 
        UINT64_C(        0)
    };
    uint64_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_u64(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_u64(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}

// tries to expose the same assert failure as above (#VDB-539), using 32 bit integers (#VDB-539), case 1
FIXTURE_TEST_CASE(IRZIP_u32_assert1, EncoderFixture)
{
    uint32_t y[]={
        78216, 
        78152, 
        77873, 
        77162, 
        76785, 
        75727, 
        74605, 
        73979, 
        73604, 
        72503
    };
    uint32_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_u32(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_u32(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}

// tries to expose the same assert failure as above (#VDB-539), using 32 bit integers (#VDB-539), case 2
FIXTURE_TEST_CASE(IRZIP_u32_assert2, EncoderFixture)
{
    uint32_t y[]={
        78216, 
        78152, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0
    };
    uint32_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_u32(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_u32(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}

// exposes an assert failure when encoding unsigned 64 bit integers, case 3
FIXTURE_TEST_CASE(IRZIP_u64_assert3, EncoderFixture)
{
    uint64_t y[]={
        UINT64_C( 388750),
        UINT64_C( 490295),
        UINT64_C( 813277),
        UINT64_C( 725540),
        UINT64_C(  85294),
        UINT64_C( 178363),
        UINT64_C(1607062),
        UINT64_C( 825545),
        UINT64_C( 474451),
        UINT64_C( 745337),
    };
    uint64_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_u64(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_u64(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}

// exposes an assert failure when encoding unsigned 64 bit integers, case 4
FIXTURE_TEST_CASE(IRZIP_u64_assert4, EncoderFixture)
{
    uint64_t y[]={
        UINT64_C(    2800167),
        UINT64_C(47247557592),
        UINT64_C(64427423880),
        UINT64_C(30067744899),
        UINT64_C(64427423881),
        UINT64_C(21477638667),
        UINT64_C( 8592622326),
        UINT64_C(12887443839),
        UINT64_C( 8592622323),
        UINT64_C(12887443837),
        UINT64_C(     773768),
        UINT64_C(60132272802),
        UINT64_C(25772592326),
        UINT64_C( 4297746473),
        UINT64_C(21477638660),
    };
    uint64_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_u64(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_u64(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}


// encoding/decoding of signed 64 bit integers
FIXTURE_TEST_CASE(IRZIP_i64_assert1, EncoderFixture)
{
    int64_t y[]={
        INT64_C( 353878216), 
        INT64_C(-353878152), 
        INT64_C( 353877873), 
        INT64_C(-353877162), 
        INT64_C(-353876785), 
        INT64_C( 353875727), 
        INT64_C(-353874605), 
        INT64_C( 353873979), 
        INT64_C(-353873604), 
        INT64_C( 353872503)
    };
    int64_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_i64(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_i64(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
}

// encoding/decoding of signed 32 bit integers
FIXTURE_TEST_CASE(IRZIP_i32_assert1, EncoderFixture)
{
    int32_t y[]={
        78216, 
        78152, 
        -77873, 
        -77162, 
        -76785, 
        75727, 
        74605, 
        -73979, 
        -73604, 
        72503
    };
    int32_t decoded[ARR_SIZE(y)];
    REQUIRE_RC(doEncode_i32(dst, dSize, &used, min, slope, &series_count, &planes, y, ARR_SIZE(y)));
    REQUIRE_RC(doDecode_i32(decoded, ARR_SIZE(y), min, slope, series_count, planes, dst, used));
    REQUIRE_EQ_ARR(y, decoded, ARR_SIZE(y));
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

const char UsageDefaultName[] = "wb-test-vxf";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VxfTestSuite(argc, argv);
    return rc;
}

}
