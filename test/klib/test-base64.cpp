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

#include <klib/base64.h>
#include <klib/text.h>
#include <klib/data-buffer.h>

TEST_SUITE(KBase64TestSuite);

///////////////////////////////////////////////// text

TEST_CASE ( KBase64_encodeBase64 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "Test for basic Base64 encoding";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "VGVzdCBmb3IgYmFzaWMgQmFzZTY0IGVuY29kaW5n" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "VGVzdCBmb3IgYmFzaWMgQmFzZTY0IGVuY29kaW5n" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "Test for basic Base64 encoding" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc1 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc1 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc2 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "f";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "Zg==" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc2 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "Zg==" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "f" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc3 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "fo";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "Zm8=" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc3 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "Zm8=" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "fo" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc4 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "foo";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "Zm9v" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc4 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "Zm9v" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "foo" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc5 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "foob";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "Zm9vYg==" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc5 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "Zm9vYg==" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "foob" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc6 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "fooba";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "Zm9vYmE=" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc6 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "Zm9vYmE=" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "fooba" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

TEST_CASE ( KBase64_encodeBase64_rfc7 )
{
    //KEncodeBase64 ( "Test for basic Base64 encoding" );
    rc_t rc = 0;
    const String *encoded;
    const char *data = "foobar";
    
    rc = encodeBase64 ( &encoded, data, strlen (data) );
    
    REQUIRE_RC ( rc );
    
    std :: string expected ( "Zm9vYmFy" );
    std :: string result ( encoded -> addr, encoded -> size );
    
    REQUIRE_EQ ( expected, result );
    
    StringWhack ( encoded );
}

TEST_CASE ( KBase64_decodeBase64_rfc7 )
{
    rc_t rc = 0;
    KDataBuffer decoded;
    String str;
    StringInitCString ( &str, "Zm9vYmFy" );
    const String *encoding;
    StringCopy ( &encoding, &str );
    
    rc = decodeBase64 ( &decoded, encoding );
    REQUIRE_RC ( rc );

    std :: string expected ( "foobar" );
    std :: string result ( ( char * ) decoded . base, decoded . elem_count );
    
    REQUIRE_EQ ( expected, result );

    rc = KDataBufferWhack ( &decoded );
    REQUIRE_RC ( rc );
}

//////////////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-base64";

rc_t CC KMain(int argc, char* argv[])
{
    KConfigDisableUserSettings();
    rc_t rc = KBase64TestSuite(argc, argv);
    return rc;
}
}
