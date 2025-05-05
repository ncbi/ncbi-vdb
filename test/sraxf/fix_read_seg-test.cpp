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

#include <klib/text.h> /* uint16_t */
#include <assert.h>
#include <math.h>

#include <ktst/unit_test.hpp> /* TEST_SUITE */
#include <kapp/vdbapp.h>

#include <os-native.h> /* lround on Windows, uint16_t */
#include <string.h> /* memset */

#include "../../libs/sraxf/fix_read_seg_impl.h"

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

int main ( int argc, char *argv [] )
{ return FixReadSegTestSuite(argc, argv); }
