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
* Unit tests for GUID generation
*/

#include <ktst/unit_test.hpp>

#include <klib/guid.h>

using namespace std;

TEST_SUITE(GUIDTestSuite);

TEST_CASE(Make_NullBuf)
{
    REQUIRE_RC_FAIL ( KGUIDMake( nullptr, 37 ) );
}

TEST_CASE(Make_TooShort)
{
    char buf[37];
    REQUIRE_RC_FAIL ( KGUIDMake( buf, 36 ) );
}

static uint8_t fromxdigit( unsigned char ch )
{
    if ( ch < '0' )
        return '?';
    if ( ch <= '9' )
        return ch - '0';
    if ( ch < 'a' )
        return '?';
    if ( ch <= 'f' )
        return ch - 'a' + 10;
    return '?';
}

TEST_CASE(Make_Success)
{
    char buf[ 37 ];
    memset( buf, 0, sizeof buf );
    REQUIRE_RC ( KGUIDMake( buf, sizeof buf ) );

//cout<< buf << endl;

    REQUIRE ( isxdigit ( buf[0] ) );
    REQUIRE ( isxdigit ( buf[1] ) );
    REQUIRE ( isxdigit ( buf[2] ) );
    REQUIRE ( isxdigit ( buf[3] ) );
    REQUIRE ( isxdigit ( buf[4] ) );
    REQUIRE ( isxdigit ( buf[5] ) );
    REQUIRE ( isxdigit ( buf[7] ) );
    REQUIRE_EQ ( '-', buf[8] );

    REQUIRE ( isxdigit ( buf[9] ) );
    REQUIRE ( isxdigit ( buf[10] ) );
    REQUIRE ( isxdigit ( buf[11] ) );
    REQUIRE ( isxdigit ( buf[12] ) );
    REQUIRE_EQ ( '-', buf[13] );

    REQUIRE_EQ ( '4', buf[14] ); // version
    REQUIRE ( isxdigit ( buf[15] ) );
    REQUIRE ( isxdigit ( buf[16] ) );
    REQUIRE ( isxdigit ( buf[17] ) );
    REQUIRE_EQ ( '-', buf[18] );

    // 0b10xx, variant 1 in the 2 msb
    REQUIRE ( isxdigit ( buf[19] ) );
    REQUIRE_EQ ( 2, fromxdigit( buf[ 19 ] ) >> 2 );

    REQUIRE ( isxdigit ( buf[20] ) );
    REQUIRE ( isxdigit ( buf[21] ) );
    REQUIRE ( isxdigit ( buf[22] ) );
    REQUIRE_EQ ( '-', buf[23] );

    REQUIRE ( isxdigit ( buf[24] ) );
    REQUIRE ( isxdigit ( buf[25] ) );
    REQUIRE ( isxdigit ( buf[26] ) );
    REQUIRE ( isxdigit ( buf[27] ) );
    REQUIRE ( isxdigit ( buf[28] ) );
    REQUIRE ( isxdigit ( buf[29] ) );
    REQUIRE ( isxdigit ( buf[30] ) );
    REQUIRE ( isxdigit ( buf[31] ) );
    REQUIRE ( isxdigit ( buf[32] ) );
    REQUIRE ( isxdigit ( buf[33] ) );
    REQUIRE ( isxdigit ( buf[34] ) );
    REQUIRE ( isxdigit ( buf[35] ) );
    REQUIRE_EQ ( 0, (int) buf[ 36 ] );
}

//////////////////////////////////////////////////// Main
extern "C"
{
#ifdef WINDOWS
#define main wmain
#endif
int main ( int argc, char *argv [] )
{
    rc_t rc=GUIDTestSuite(argc, argv);
    return rc;
}

}
