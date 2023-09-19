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
 * Unit tests for SHA hash functions
 * Test values from https://www.nist.gov/itl/ssd/software-quality-group/nsrl-test-data
 */

#include <ktst/unit_test.hpp>
#include <array>
#include <klib/checksum.h>

using namespace std;

TEST_SUITE(SHATestSuite);

static std::string convertDigest(uint8_t const *digest, size_t size)
{
    auto result = std::string();
    result.reserve(size * 2 + (size * 2 - 1) / 8);
    for (size_t i = 0; i < size; ++i) {
        char buf[16];
        static char const *fmt = "%02X";
        int n = snprintf(buf, 16, fmt, (int)digest[i]);
        result.append(buf, n);
    }
    return result;
}

TEST_CASE(SHA1_abc_NIST)
{
    SHA1State state;
    uint8_t digest[20];

    SHA1StateInit(&state);
    SHA1StateAppend(&state, "abc", 3);
    SHA1StateFinish(&state, digest);

    std::string const expect{"A9993E364706816ABA3E25717850C26C9CD0D89D"};
    auto const got = convertDigest(digest, 20);
    REQUIRE_EQ(got, expect);
}

TEST_CASE(SHA224_abc_openssl)
{
    SHA224State state;
    uint8_t digest[28];

    SHA224StateInit(&state);
    SHA224StateAppend(&state, "abc", 3);
    SHA224StateFinish(&state, digest);

    /// from `echo -n "abc" | openssl dgst -sha224`
    std::string const expect{"23097D223405D8228642A477BDA255B32AADBCE4BDA0B3F7E36C9DA7"};
    auto const got = convertDigest(digest, sizeof(digest));
    REQUIRE_EQ(got, expect);
}

TEST_CASE(SHA256_abc_NIST)
{
    SHA256State state;
    uint8_t digest[32];

    SHA256StateInit(&state);
    SHA256StateAppend(&state, "abc", 3);
    SHA256StateFinish(&state, digest);

    std::string const expect{"BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD"};
    auto const got = convertDigest(digest, sizeof(digest));
    REQUIRE_EQ(got, expect);
}

//////////////////////////////////////////////////// Main
extern "C"
{
#ifdef WINDOWS
#define main wmain
#endif
int main ( int argc, char *argv [] )
{
    rc_t rc=SHATestSuite(argc, argv);
    return rc;
}

}
