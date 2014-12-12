#include <klib/text.h> /* uint16_t */
#include <assert.h>
#include <math.h>

#include <ktst/unit_test.hpp> /* TEST_SUITE */
#include <kapp/main.h> /* KAppVersion */

#include <os-native.h> /* lround on Windows, uint16_t */
#include <string.h> /* memset */

#include "../../libs/sraxf/fix_read_seg_impl.h"

ver_t CC KAppVersion ( void ) { return 0; }
rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName[] = "";
rc_t UsageSummary (const char * progname) { return 0; }

TEST_SUITE(FixReadSegTestSuite);

TEST_CASE(_0_reads) {
    uint16_t rs_src[] = { 0 };
    uint32_t nreads = 0;
    uint32_t rs_dst[] = { 0 };
    fix_read_seg_impl(rs_src, nreads, 0, rs_dst);
}

TEST_CASE(_1_empty_read) {
    uint16_t rs_src[] = { 0, 0 };
    uint32_t nreads = 1;
    uint32_t rs_dst[] = { 0, 0 };
    fix_read_seg_impl(rs_src, nreads, 0, rs_dst);
}

TEST_CASE(_1_read) {
    uint16_t rs_src[] = { 2, 0 };
    uint32_t nreads = 1;
    uint32_t spot_len = 1;
    uint32_t dst[] = { 1, 0 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
}

TEST_CASE(total_0_read_2) {
    uint16_t rs_src[] = { 9, 0, 8, 0 };
    uint32_t nreads = 2;
    uint32_t spot_len = 1;
    uint32_t dst[] = { 2, 2, 2, 2 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
    CHECK_EQ(dst[2], (uint32_t)1);
    CHECK_EQ(dst[3], (uint32_t)0);

    spot_len = 2;
    memset(dst, 2, sizeof dst);
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
    CHECK_EQ(dst[2], (uint32_t)1);
    CHECK_EQ(dst[3], (uint32_t)1);
}

TEST_CASE(total_eq_spotlen_read_1) {
    uint16_t rs_src[] = { 9, 1 };
    uint32_t nreads = 1;
    uint32_t spot_len = 1;
    uint32_t dst[] = { 2, 2, 2, 2 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
}

TEST_CASE(total_eq_spotlen_read_2) {
    uint16_t rs_src[] = { 9, 1, 8, 2 };
    uint32_t nreads = 2;
    uint32_t spot_len = 3;
    uint32_t dst[] = { 7, 7, 7, 7 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
    CHECK_EQ(dst[2], (uint32_t)1);
    CHECK_EQ(dst[3], (uint32_t)2);
}

TEST_CASE(total_gt_spotlen_read_1) {
    uint16_t rs_src[] = { 9, 9 };
    uint32_t nreads = 1;
    uint32_t spot_len = 1;
    uint32_t dst[] = { 7, 7, 7, 7 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
}

TEST_CASE(total_gt_spotlen_read_2) {
    uint16_t rs_src[] = { 9, 9, 8, 9 };
    uint32_t nreads = 2;
    uint32_t spot_len = 1;
    uint32_t dst[] = { 7, 7, 7, 7 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
    CHECK_EQ(dst[2], (uint32_t)1);
    CHECK_EQ(dst[3], (uint32_t)0);
}

TEST_CASE(total_lt_spotlen_read_1) {
    uint16_t rs_src[] = { 9, 1 };
    uint32_t nreads = 1;
    uint32_t spot_len = 9;
    uint32_t dst[] = { 7, 7, 7, 7 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)9);
}

TEST_CASE(total_lt_spotlen_read_2) {
    uint16_t rs_src[] = { 9, 1, 8, 1 };
    uint32_t nreads = 2;
    uint32_t spot_len = 9;
    uint32_t dst[] = { 7, 7, 7, 7 };
    fix_read_seg_impl(rs_src, nreads, spot_len, dst);
    CHECK_EQ(dst[0], (uint32_t)0);
    CHECK_EQ(dst[1], (uint32_t)1);
    CHECK_EQ(dst[2], (uint32_t)1);
    CHECK_EQ(dst[3], (uint32_t)8);
}

rc_t CC KMain ( int argc, char *argv [] )
{ return FixReadSegTestSuite(argc, argv); }
