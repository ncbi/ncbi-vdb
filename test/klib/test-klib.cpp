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
* Unit tests for klib interface
*/

#include <ktst/unit_test.hpp>

#include <klib/data-buffer.h>
#include <klib/log.h>
#include <klib/misc.h> /* is_user_admin() */
#include <klib/num-gen.h>
#include <klib/printf.h>
#include <klib/sort.h>
#include <klib/text.h>
#include <klib/vector.h>
#include <klib/time.h>

#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <utility>

using namespace std;


TEST_SUITE(KlibTestSuite);

///////////////////////////////////////////////// text

TEST_CASE(Klib_text_string_len)
{
    // this is ASCII with a character count of 37, byte count of 37
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    uint32_t length = string_len(str, size);
    REQUIRE_EQ(length, (uint32_t)size);

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    length = string_len(str, size);
    REQUIRE_EQ(length, (uint32_t)(size - 4));
}

TEST_CASE(Klib_text_string_len_nonUtf8)
{
    const char* str = "12\3774567";
    size_t size = strlen ( str );
    // stops counting at the first non-UTF8 character
    REQUIRE_EQ ( 2u, string_len ( str, size ) );
}

TEST_CASE(Klib_text_string_measure)
{
    // this is ASCII with a character count of 37, byte count of 37
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    size_t measure;
    uint32_t length = string_measure(str, &measure);
    REQUIRE_EQ(measure, size);
    REQUIRE_EQ(length, (uint32_t)size);

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    length = string_measure(str, &measure);
    REQUIRE_EQ(measure, size);
    REQUIRE_EQ(length, (uint32_t)(size - 4));
}

TEST_CASE(Klib_text_string_measure_nonUtf8)
{
    const char* str = "12\3774567";
    size_t size;
    // stops counting at the first non-UTF8 character
    REQUIRE_EQ ( 2u, string_measure ( str, & size ) );
    REQUIRE_EQ ( 2, (int)size );
}

TEST_CASE(Klib_text_string_copy)
{
    char buff64[64];
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    size_t copied = string_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    char buff32[32];
    copied = string_copy(buff32, sizeof buff32, str, size);
    REQUIRE_EQ(copied, (size_t)(sizeof buff32));

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    copied = string_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    // this is the same string with a split UTF-8 character
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303";
    size = strlen(str);
    copied = string_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size - 1);
    REQUIRE_EQ((char)0, buff64[copied]);

    // test a copy with an embedded NUL
    const char EN[] = "This is the case of an\000embedded NUL byte";
    size = sizeof EN - 1;
    REQUIRE_EQ(size, (size_t)40);
    size = strlen(EN);
    REQUIRE_EQ(size, (size_t)22);
    copied = string_copy(buff64, sizeof buff64, EN, sizeof EN - 1);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
}

TEST_CASE(Klib_text_string_copy_measure)
{
    char buff64[64];
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    size_t copied = string_copy_measure(buff64, sizeof buff64, str);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    char buff32[32];
    copied = string_copy_measure(buff32, sizeof buff32, str);
    REQUIRE_EQ(copied, (size_t)(sizeof buff32));

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    copied = string_copy_measure(buff64, sizeof buff64, str);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    // this is the same string with a split UTF-8 character
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303";
    size = strlen(str);
    copied = string_copy_measure(buff64, sizeof buff64, str);
    REQUIRE_EQ(copied, size - 1);
    REQUIRE_EQ((char)0, buff64[copied]);

    // test a copy with an embedded NUL
    const char EN[] = "This is the case of an\000embedded NUL byte";
    size = sizeof EN - 1;
    REQUIRE_EQ(size, (size_t)40);
    size = strlen(EN);
    REQUIRE_EQ(size, (size_t)22);
    copied = string_copy_measure(buff64, sizeof buff64, EN);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
}

TEST_CASE(Klib_text_tolower_copy)
{
    char buff64[64];
    const char* str = "Tu Estas Probando Este Hoy, No Manana";
    size_t size = strlen(str);
    size_t copied = tolower_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
    int diff = strcmp(buff64, "tu estas probando este hoy, no manana");
    REQUIRE_EQ(diff, 0);

    char buff32[32];
    copied = tolower_copy(buff32, sizeof buff32, str, size);
    REQUIRE_EQ(copied, (size_t)(sizeof buff32));
    diff = memcmp(buff64, "tu estas probando este hoy, no manana", copied);
    REQUIRE_EQ(diff, 0);

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 Est\303\241s Probando \303\211ste Hoy, No Ma\303\261ana";
    size = strlen(str);
    copied = tolower_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
#if 0
    std::cout
        << buff64
        << '\n'
        ;
    diff = strcmp ( buff64, "t\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana" );
    REQUIRE_EQ ( diff, 0 );
#endif

    // this is the same string with a split UTF-8 character
    str = "T\303\272 Est\303\241s Probando \303\211ste Hoy, No Ma\303";
    size = strlen(str);
    copied = tolower_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size - 1);
    REQUIRE_EQ((char)0, buff64[copied]);
#if 0
    diff = memcmp ( buff64, "t\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana", copied );
    REQUIRE_EQ ( diff, 0 );
#endif

    // test a copy with an embedded NUL
    const char EN[] = "This Is The Case of an\000embedded NUL byte";
    size = sizeof EN - 1;
    REQUIRE_EQ(size, (size_t)40);
    size = strlen(EN);
    REQUIRE_EQ(size, (size_t)22);
    copied = tolower_copy(buff64, sizeof buff64, EN, sizeof EN - 1);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
    diff = memcmp(buff64, "this is the case of an\000embedded nul byte",
                  copied);
    REQUIRE_EQ(diff, 0);
}

TEST_CASE(KLib_text_StringToI64)
{
    rc_t rc;
    String str;
    int64_t val;

    CONST_STRING(&str, "12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   ++12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   ++12345678 ");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcTransfer);
    REQUIRE_EQ(GetRCState(rc), rcIncomplete);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   ++c12345678 ");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), (RCObject)rcData);
    REQUIRE_EQ(GetRCState(rc), rcInsufficient);
    REQUIRE_EQ(val, (int64_t)0);

    CONST_STRING(&str, "   -++-12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   -++12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)-12345678);

    CONST_STRING(&str, "");
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, (int64_t)0);

    CONST_STRING(&str, "0");
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, (int64_t)0);

    CONST_STRING(&str, "1");
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, (int64_t)1);

    CONST_STRING(&str, "9223372036854775805");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(9223372036854775805));

    CONST_STRING(&str, "9223372036854775807");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));

    CONST_STRING(&str, "9223372036854775808");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));

    CONST_STRING(&str, "92233720368547758071");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));

    CONST_STRING(&str, "-9223372036854775805");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(-9223372036854775805));

    CONST_STRING(&str, "-9223372036854775807");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807));

    // the value -9223372036854775808 causes complaint
    // because it is first parsed as signed, which overflows
    // and then is negated after the complaint
    // use -9223372036854775807 - 1
    // WHY AREN'T INT64_MIN/MAX WORKING?

    CONST_STRING(&str, "-9223372036854775808");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807) - INT64_C(1));

    CONST_STRING(&str, "-9223372036854775809");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807) - INT64_C(1));
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807) - INT64_C(1));
}

TEST_CASE(KLib_text_StringToU64)
{
    rc_t rc;
    String str;
    uint64_t val;

    CONST_STRING(&str, "12345678");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (uint64_t)12345678);

    CONST_STRING(&str, "   12345678");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (uint64_t)12345678);

    CONST_STRING(&str, "   ++12345678");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), (RCObject)rcData);
    REQUIRE_EQ(GetRCState(rc), rcInsufficient);
    REQUIRE_EQ(val, (uint64_t)0);

    CONST_STRING(&str, "   12345678 ");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcTransfer);
    REQUIRE_EQ(GetRCState(rc), rcIncomplete);
    REQUIRE_EQ(val, (uint64_t)12345678);

    CONST_STRING(&str, "");
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, (uint64_t)0);

    CONST_STRING(&str, "0");
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, (uint64_t)0);

    CONST_STRING(&str, "1");
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, (uint64_t)1);

    CONST_STRING(&str, "18446744073709551610");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, UINT64_C(18446744073709551610));

    CONST_STRING(&str, "18446744073709551615");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));

    CONST_STRING(&str, "18446744073709551616");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));

    CONST_STRING(&str, "184467440737095516151");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));
}

///////////////////////////////////////////////// ksort

static char UserData[] = "User data";

// this is used by qsort and (indirectly) ksort
int CC baseCompare(const void* a, const void* b)
{
    const char* pa = (const char*)a;
    const char* pb = (const char*)b;
    if (*pa < *pb) {
        return -1;
    }
    if (*pa > *pb) {
        return 1;
    }
    return 0;
}
// this is used by ksort
int64_t CC compare(const void* a, const void* b, void* data)
{
    // if data is not pointing where we waint it to, do not sort
    const char* pdata = (const char*)data;
    if (pdata != 0
        && string_cmp(pdata, string_size(pdata), UserData,
                      string_size(UserData), (uint32_t)string_size(UserData))
            != 0) {
        return 0;
    }
    return baseCompare(a, b);
}

TEST_CASE(KLib_ksort_no_elements)
{
    char arr[1] = {13};
    ksort(arr, 0, sizeof(char), compare, UserData);
    // do not crash or change the target
    REQUIRE_EQ(arr[0], (char)13);
}
TEST_CASE(KLib_ksort_one_element)
{
    char arr[1] = {13};
    ksort(arr, 1, sizeof(char), compare, UserData);
    // do not crash or change the target
    REQUIRE_EQ(arr[0], (char)13);
}

TEST_CASE(KLib_ksort_simple)
{
    const int Size = 5;
    char arr[Size] = {2, 4, 1, 0, 3};
    ksort(arr, Size, sizeof(char), compare, UserData);
    REQUIRE_EQ(arr[0], (char)0);
    REQUIRE_EQ(arr[1], (char)1);
    REQUIRE_EQ(arr[2], (char)2);
    REQUIRE_EQ(arr[3], (char)3);
    REQUIRE_EQ(arr[4], (char)4);
}
TEST_CASE(KLib_ksort_vs_qsort)
{
    const int Size = 5;
    char karr[Size] = {2, 4, 1, 0, 3};
    char qarr[Size];
    memmove(qarr, karr, sizeof(karr));

    ksort(karr, Size, sizeof(char), compare, 0); // do not pass any user data
    qsort(qarr, Size, sizeof(char), baseCompare);
    REQUIRE_EQ(memcmp(karr, qarr, sizeof(karr)), 0);
}

// an example of a bad function (compating pointers not values, in reverse
// order) that causes an implementaion of ksort to crash.
int64_t CC badCompare(const void* a, const void* b, void* data)
{
    const char* pa = (const char*)a;
    const char* pb = (const char*)b;
    if (pa < pb) {
        return 1;
    }
    if (pa > pb) {
        return -1;
    }
    return 0;
}
TEST_CASE(KLib_ksort_problem)
{
    const int Size = 5;
    {
        char arr[Size] = {2, 4, 1, 0, 3};
        ksort(arr, Size, sizeof(char), badCompare, 0);
        // we just do not want this to crash since the compare function is not
        // working properly and the eventual sort order is undefined
    }
}

///////////////////////////////////////////////// macro based ksort

TEST_CASE(KSORT_simple)
{
    char karr[] = {2, 4, 1, 0, 3};
    const int ElemSize = sizeof(karr[0]);
    const int Size = sizeof(karr) / ElemSize;
    char qarr[Size];
    memmove(qarr, karr, sizeof(karr));

#define CMP(a, b) (*(char*)(a) < *(char*)(b) ? -1 : *(char*)(a) > *(char*)(b))
#define SWAP(a, b, offset, size)
    KSORT(karr, Size, ElemSize, 0, ElemSize);
    ksort(qarr, Size, ElemSize, compare, 0);
    REQUIRE_EQ(memcmp(karr, qarr, sizeof(karr)), 0);
#undef CMP
#undef SWAP
}

static int64_t CC cmp_int64_t(const void* a, const void* b, void* data)
{
    const int64_t* ap = (const int64_t*)a;
    const int64_t* bp = (const int64_t*)b;

    if (*ap < *bp) return -1;
    return *ap > *bp;
}

TEST_CASE(KSORT_int64)
{
    int64_t karr[] = {2, 4, 1};
    const int ElemSize = sizeof(karr[0]);
    const int Size = sizeof(karr) / ElemSize;
    int64_t qarr[Size];
    memmove(qarr, karr, sizeof(karr));

    ksort_int64_t(karr, Size);
    ksort(qarr, Size, ElemSize, cmp_int64_t, 0);
    REQUIRE_EQ(memcmp(karr, qarr, sizeof(karr)), 0);
}

///////////////////////////////////////////////// string_printf
TEST_CASE(KLib_print_uint64)
{
    char dst[1024];
    size_t num_writ;
    uint64_t val = UINT64_C(1152921504606846976);
    REQUIRE_RC(string_printf(dst, sizeof(dst), &num_writ, "%lu", val));
    REQUIRE_EQ(string(dst, num_writ), string("1152921504606846976"));
}

#ifndef WINDOWS
TEST_CASE(KLib_print_problem)
{
    char dst[1024];
    size_t num_writ;
    double d = 1.0;
    REQUIRE_RC(string_printf(dst, sizeof(dst), &num_writ, "%.6f", d));
    REQUIRE_EQ(num_writ, strlen(dst));
    REQUIRE_EQ(string("1.000000"), string(dst, num_writ));
}
#endif

TEST_CASE(string_printf_args) {
    char buf [ 2 ] = "";
    size_t num_writ = 0;

    REQUIRE_RC_FAIL ( string_printf ( NULL, 0, NULL, NULL ) );

    REQUIRE_RC_FAIL ( string_printf ( buf , 0, NULL, NULL ) );

    REQUIRE_RC_FAIL ( string_printf ( NULL, 0, NULL, ""   ) );

    REQUIRE_RC_FAIL ( string_printf ( NULL, 0, & num_writ, "%s", "X" ) );

    REQUIRE_RC_FAIL ( string_printf ( buf , 0, & num_writ, "%s", "X" ) );

    REQUIRE_RC    ( string_printf ( buf, sizeof buf, NULL, "%s", "X" ) );

    REQUIRE_RC_FAIL(string_printf ( buf, sizeof buf, & num_writ, NULL ) );
    REQUIRE_RC    ( string_printf ( buf, sizeof buf, & num_writ, "" ) );

    REQUIRE_RC    ( string_printf ( buf, sizeof buf, & num_writ, "%s", "X" ) );
}

///////////////////////////////////////////////// KDataBuffer

TEST_CASE(KDataBuffer_Make)
{
    KDataBuffer src;

    REQUIRE_RC(KDataBufferMake(&src, 13, 9876));

    REQUIRE(src.ignore != 0);
    REQUIRE(src.base != 0);
    REQUIRE_EQ((uint64_t)13, src.elem_bits);
    REQUIRE_EQ((uint64_t)9876, src.elem_count);
    REQUIRE_EQ((bitsz_t)(13 * 9876), KDataBufferBits(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_MakeBytes)
{
    KDataBuffer src;

    REQUIRE_RC(KDataBufferMakeBytes(&src, 12));

    REQUIRE_EQ((uint64_t)8, src.elem_bits);
    REQUIRE_EQ((uint64_t)12, src.elem_count);
    REQUIRE_EQ((bitsz_t)(12 * 8), KDataBufferBits(&src));
    REQUIRE_EQ((size_t)12, KDataBufferBytes(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_MakeBits)
{
    KDataBuffer src;

    REQUIRE_RC(KDataBufferMakeBits(&src, 8));

    REQUIRE_EQ((uint64_t)1, src.elem_bits);
    REQUIRE_EQ((uint64_t)8, src.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * 8), KDataBufferBits(&src));
    REQUIRE_EQ((size_t)1, KDataBufferBytes(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_MakeBits1)
{
    KDataBuffer src;
    const size_t BIT_SZ = 7896;
    REQUIRE_RC(KDataBufferMakeBits(&src, BIT_SZ));

    REQUIRE_EQ((uint64_t)1, src.elem_bits);
    REQUIRE_EQ((uint64_t)BIT_SZ, src.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * BIT_SZ), KDataBufferBits(&src));
    REQUIRE_EQ((size_t)((1 * BIT_SZ + 7) / 8), KDataBufferBytes(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_Sub)
{
    KDataBuffer src;
    KDataBuffer sub;

    const size_t BIT_SZ = 7896;
    REQUIRE_RC(KDataBufferMakeBits(&src, BIT_SZ));

    REQUIRE_RC(KDataBufferSub(&src, &sub, 800, 900));

    REQUIRE_EQ(src.elem_bits, sub.elem_bits);
    REQUIRE_EQ((uint64_t)900, sub.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * 900), KDataBufferBits(&sub));
    REQUIRE_EQ((uint8_t*)sub.base, (uint8_t*)src.base + (800 >> 3));
    REQUIRE_EQ((uint64_t)sub.bit_offset, (uint64_t)0);

    KDataBufferWhack(&src);
    KDataBufferWhack(&sub);
}

TEST_CASE(KDataBuffer_Sub1)
{
    KDataBuffer src;
    KDataBuffer sub;

    const size_t BIT_SZ = 7896;
    REQUIRE_RC(KDataBufferMakeBits(&src, BIT_SZ));

    REQUIRE_RC(KDataBufferSub(&src, &sub, 801, 900));

    REQUIRE_EQ(src.elem_bits, sub.elem_bits);
    REQUIRE_EQ((uint64_t)900, sub.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * 900), KDataBufferBits(&sub));
    REQUIRE_EQ((uint8_t*)sub.base, (uint8_t*)src.base + (800 >> 3));
    REQUIRE_EQ((uint64_t)sub.bit_offset, (uint64_t)1);

    KDataBufferWhack(&src);
    KDataBufferWhack(&sub);
}

TEST_CASE(KDataBuffer_MakeWritable)
{
    KDataBuffer src;
    KDataBuffer copy;

    REQUIRE_RC(KDataBufferMakeBytes(&src, 12));

    REQUIRE_RC(KDataBufferMakeWritable(&src, &copy));

    REQUIRE_EQ(src.elem_bits, copy.elem_bits);
    REQUIRE_EQ((uint64_t)12, copy.elem_count);
    REQUIRE_EQ(copy.base, src.base);
    REQUIRE_EQ((uint64_t)copy.bit_offset, (uint64_t)0);

    KDataBufferWhack(&src);
    KDataBufferWhack(&copy);
}

TEST_CASE(KDataBuffer_MakeWritable1)
{
    KDataBuffer src;
    KDataBuffer sub;
    KDataBuffer copy;

    REQUIRE_RC(KDataBufferMakeBytes(&src, 256));

    REQUIRE_RC(KDataBufferSub(&src, &sub, 8, 12));

    REQUIRE_RC(KDataBufferMakeWritable(&sub, &copy));

    REQUIRE_EQ(src.elem_bits, copy.elem_bits);
    REQUIRE_EQ((uint64_t)12, sub.elem_count);
    REQUIRE_EQ((uint64_t)12, copy.elem_count);
    REQUIRE_NE(sub.base, copy.base);
    REQUIRE_EQ((uint64_t)copy.bit_offset, (uint64_t)0);

    KDataBufferWhack(&src);
    KDataBufferWhack(&sub);
    KDataBufferWhack(&copy);
}

TEST_CASE(KDataBuffer_EmptyBufferIsWriteable)
{
    KDataBuffer src;
    REQUIRE_RC ( KDataBufferMake( & src, 8, 0 ) );
    REQUIRE ( KDataBufferWritable( & src ) );

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_ZeroCountMake_DoesNotAllocate)
{
    KDataBuffer src;
    REQUIRE_RC ( KDataBufferMake( & src, 8, 0) );

    REQUIRE_NULL ( src.base );
    REQUIRE_NULL ( src.ignore );

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_Resize)
{
    KDataBuffer src;
    uint32_t blob_size = 4096;
    REQUIRE_RC(KDataBufferMake(&src, 8, blob_size));

    /* make sub-buffer from input */
    KDataBuffer dst;
    uint32_t hdr_size = 7;
    REQUIRE_RC(KDataBufferSub(&src, &dst, hdr_size, blob_size));
    /* cast from 8 into 2 bits */
    REQUIRE_RC(KDataBufferCast(&dst, &dst, 2, true));
    /* resize to 4 times the original number of elements */
    REQUIRE_RC(KDataBufferResize(&dst, (blob_size - hdr_size) * 4));

    KDataBufferWhack(&dst);
    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_ResizeEmpty)
{
    KDataBuffer buffer;
    memset ( & buffer, 0, sizeof buffer );
    REQUIRE_RC ( KDataBufferCheckIntegrity ( & buffer ) );

    /* Not allowed to KDataBufferResize when buffer's elem_bits == 0 */
    REQUIRE_RC_FAIL ( KDataBufferResize ( & buffer, 4096 ) );

    KDataBuffer empty;
    memset ( & empty, 0, sizeof empty );
    REQUIRE ( ! memcmp ( & buffer, & empty, sizeof empty ) );
}

TEST_CASE(KDataBuffer_Cast_W32Assert)
{
    KDataBuffer src;
    REQUIRE_RC(KDataBufferMake(&src, 64, 1));
    REQUIRE_RC(KDataBufferCast(&src, &src, 64,
                               true)); /* used to throw an assert on Win32 */
    KDataBufferWhack(&src);
}

//////////////////////////////////////////// Log
TEST_CASE(KLog_Formatting)
{
    unsigned long status = 161;
    REQUIRE_RC(pLogErr(klogInfo, 0, "$(E) - $(C)", "E=%!,C=%u", status,
                       status)); // fails on Windows
}

TEST_CASE(KLog_LevelExplain)
{
    char buf[5];
    size_t num_writ = 0;
    REQUIRE_RC(KLogLevelExplain(klogInfo, buf, sizeof(buf), &num_writ));
    REQUIRE_EQ(string("info"), string(buf));
    REQUIRE_EQ(num_writ, strlen(buf));
}
TEST_CASE(KLog_LevelExplainNoRoomforNul)
{
    char buf[4];
    size_t num_writ = 0;
    REQUIRE_RC(KLogLevelExplain(klogInfo, buf, sizeof(buf), &num_writ));
    REQUIRE_EQ(string("info"), string(buf, 4));
    REQUIRE_EQ(num_writ, (size_t)4);
}
TEST_CASE(KLog_LevelExplainInsufficientBuffer)
{
    char buf[3];
    size_t num_writ = 0;
    REQUIRE_RC_FAIL(KLogLevelExplain(klogInfo, buf, sizeof(buf), &num_writ));
    REQUIRE_EQ(num_writ, (size_t)0);
}

// this has been observed to fail under TeamCity
//
// TEST_CASE(IsUserAnAdminTest)
// {
//     // TeamCity agents run as admin on some systems but not the others
// #if defined (WINDOWS)
//     // if ( getenv ( "TEAMCITY_VERSION" ) != 0 )
//     // {   // always an admin under TC
//     //     REQUIRE ( is_iser_an_admin() );
//     // }
//     // otherwise, we do not really know
// #else
//     // Linux or not under TeamCity
//     REQUIRE(!is_iser_an_admin());
// #endif
// }

static const size_t BufSize = 1024;
// implementation of KWrtWriter for testing purposes
rc_t CC TestWrtWriter(void* data, const char* buffer, size_t bufsize,
                      size_t* num_writ)
{
    size_t nw = string_copy((char*)data, BufSize, buffer, bufsize);
    if (num_writ != 0) *num_writ = nw;
    return 0;
}

TEST_CASE(KLog_DefaultFormatterNull)
{
    REQUIRE_RC(KLogFmtWriterGet()(NULL, NULL, 0, NULL, 0, NULL));
}
TEST_CASE(KLog_DefaultFormatterBasic)
{
    char buffer[BufSize];
    KWrtHandler testHandler = {TestWrtWriter, (void*)buffer};
    wrt_nvp_t args[] = {// ignored
                        {"", ""}};
    wrt_nvp_t envs[] = {
        // has to be sorted by key, apparently...
        {"app", "test"},     {"message", "msg"},    {"reason", "rc"},
        {"severity", "bad"}, {"timestamp", "time"}, {"version", "v1"},
    };
    REQUIRE_RC(KLogFmtWriterGet()(NULL, &testHandler,
                                  sizeof(args) / sizeof(args[0]), args,
                                  sizeof(envs) / sizeof(envs[0]), envs));
    REQUIRE_EQ(string("time test.v1 bad: rc - msg\n"),
               string((const char*)testHandler.data));
}

// implementation of KFmtWriter for testing purposes
rc_t CC TestFmtWriter(void* self, KWrtHandler* writer, size_t argc,
                      const wrt_nvp_t args[], size_t envc,
                      const wrt_nvp_t envs[])
{
    char buffer[BufSize];
    size_t num_writ;
    string_printf(buffer, sizeof(buffer), &num_writ, "%s",
                  wrt_nvp_find_value(envc, envs, "message"));

    size_t size;
    return LogFlush(writer, buffer, string_measure(buffer, &size));
}
TEST_CASE(KLog_ParamFormatting)
{
    char buffer[BufSize];
    REQUIRE_RC(KLogHandlerSet(TestWrtWriter, buffer));
    REQUIRE_RC(KLogFmtHandlerSet(TestFmtWriter, 0, buffer));
    REQUIRE_RC(pLogMsg(klogErr, "message with $(PARAM1) $(P2) etc.",
                       "PARAM1=%s,P2=%s", "parameter1", "param2"));
    REQUIRE_EQ(string("message with parameter1 param2 etc."), string(buffer));
}

//////////////////////////////////////////// num-gen
#define SHOW_UNIMPLEMENTED 0
class NumGenFixture
{
  public:
    NumGenFixture() : m_ng(0), m_it(0) {}
    ~NumGenFixture()
    {
        if (m_it && num_gen_iterator_destroy(m_it))
        {
            std :: cerr
                << "NumGenFixture: num_gen_iterator_destroy failed"
                << std :: endl
                ;
        }

        else if (m_ng && num_gen_destroy(m_ng) != 0)
        {
            std :: cerr
                << "NumGenFixture: num_gen_destroy failed"
                << std :: endl
                ;
        }
    }

    struct num_gen* m_ng;
    const struct num_gen_iter* m_it;
};

FIXTURE_TEST_CASE(num_gen_Make, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE_NOT_NULL(m_ng);
}
FIXTURE_TEST_CASE(num_gen_MakeFromStr, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_str(&m_ng, "1"));
    REQUIRE_NOT_NULL(m_ng);
}

FIXTURE_TEST_CASE(num_gen_MakeFromEmptyStr, NumGenFixture)
{
    REQUIRE_RC_FAIL(num_gen_make_from_str(&m_ng, ""));
    REQUIRE_NULL(m_ng);
}

FIXTURE_TEST_CASE(num_gen_MakeFromRange, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE_NOT_NULL(m_ng);
}
FIXTURE_TEST_CASE(num_gen_MakeFromEmptyRange, NumGenFixture)
{
    REQUIRE_RC_FAIL(num_gen_make_from_range(&m_ng, 1, 0));
    REQUIRE_NULL(m_ng);
}

FIXTURE_TEST_CASE(num_gen_Empty, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE(num_gen_empty(m_ng));
}

FIXTURE_TEST_CASE(num_gen_Clear, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE(!num_gen_empty(m_ng));

    REQUIRE_RC(num_gen_clear(m_ng));

    REQUIRE(num_gen_empty(m_ng));
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(num_gen_EmptyRange, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 0));
    REQUIRE_NOT_NULL(m_ng);
    REQUIRE(num_gen_empty(m_ng));
}
#endif

FIXTURE_TEST_CASE(num_gen_IteratorMake, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));
}

FIXTURE_TEST_CASE(num_gen_IteratorCount, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));

    uint64_t count;
    REQUIRE_RC(num_gen_iterator_count(m_it, &count));
    REQUIRE_EQ((uint64_t)2, count);
}
FIXTURE_TEST_CASE(num_gen_IteratorCountEmpty, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));

    uint64_t count;
    REQUIRE_RC(num_gen_iterator_count(m_it, &count));
    REQUIRE_EQ((uint64_t)0, count);
}

FIXTURE_TEST_CASE(num_gen_IteratorNext, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));

    int64_t value;
    rc_t rc = 0;
    REQUIRE(num_gen_iterator_next(m_it, &value, &rc));
    REQUIRE_EQ((int64_t)1, value);
    REQUIRE_RC(rc);

    REQUIRE(num_gen_iterator_next(m_it, &value, &rc));
    REQUIRE_EQ((int64_t)2, value);
    REQUIRE_RC(rc);

    REQUIRE(!num_gen_iterator_next(m_it, &value, &rc));
    REQUIRE_RC(rc);
}

// TODO:
// rc_t num_gen_parse( struct num_gen * self, const char * src );
// rc_t num_gen_add( struct num_gen * self, const uint64_t first, const
// uint64_t count );
// rc_t num_gen_trim( struct num_gen * self, const int64_t first, const
// uint64_t count );
// rc_t num_gen_as_string( const struct num_gen * self, char * buffer, size_t
// buffsize, size_t * written, bool full_info );
// rc_t num_gen_contains_value( const struct num_gen * self, const uint64_t
// value );
// rc_t num_gen_range_check( struct num_gen * self, const int64_t first, const
// uint64_t count );
// rc_t num_gen_iterator_percent( const struct num_gen_iter * self, uint8_t
// fract_digits, uint32_t * value );

// Error reporting
#if _DEBUGGING
TEST_CASE(GetUnreadRCInfo_LogRC)
{ // bug report: only 1 RC is reported at the end of KMane in _DEBUGGING mode

    // create a couple of RCs
    RC(rcApp, rcFile, rcConstructing, rcFile, rcNull);
    RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);

    rc_t rc;
    uint32_t lineno;
    const char *filename, *function;
    REQUIRE(GetUnreadRCInfo(&rc, &filename, &function, &lineno));
    // bug: call to pLogErr invokes GetRCFunction (and others alike),
    // which as a side effect changes "last read RC" to equal "last written
    // RC"
    // causing the subsequent call to GetUnreadRCInfo to return "no more
    // unread RCs"
    pLogErr(klogWarn, rc, "$(filename):$(lineno) within $(function)",
            "filename=%s,lineno=%u,function=%s", filename, lineno, function);
    REQUIRE(GetUnreadRCInfo(&rc, &filename, &function, &lineno));
}
#endif

TEST_CASE(TimeRoundTrip)
{ 
    KTime_t t1 = KTimeStamp(); // UTC
    char str1[100];
    KTimeIso8601(t1, str1, sizeof str1);
    KTime time;
    KTimeFromIso8601(&time, str1, string_size(str1));
    KTime_t t2 = KTimeMakeTime(&time);
    char str2[100];
    KTimeIso8601(t2, str2, sizeof str2);
    REQUIRE_EQ( t1, t2 );
    REQUIRE_EQ( string(str1), string(str2) );
}

//////////////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-klib";

rc_t CC KMain(int argc, char* argv[])
{
    KConfigDisableUserSettings();
    rc_t rc = KlibTestSuite(argc, argv);
    return rc;
}
}
