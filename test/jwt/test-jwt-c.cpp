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
* A test for C interface to JWT library
*/

#include <klib/rc.h>

#include <cfloat>

#include <jwt/jwt-string.hpp>

using namespace ncbi;
// this is compiled by a c++ compiler but will be linked without C++ std library

//TODO: remove this when done removing exceptions from JWT
void *__gxx_personality_v0;

extern "C"
{

#define THROW return RC(rcText,rcString,rcProcessing,rcError,rcUnexpected)

rc_t JWT_Example()
{
    {   // empty
        JwtString str;
        if (! str.empty()) THROW;
        if (str.size() != 0) THROW;
        if (strcmp ( str.data(), "" ) != 0) THROW;
        if (str.c_str() != str.data()) THROW;
    }

    {
        JwtString str("123");
        if (str.empty()) THROW;
        if (str.size() != 3) THROW;
        if (strcmp(str.data(), "123") != 0) THROW;
        if (strcmp(str.c_str(), "123") != 0) THROW;
    }

    {
        JwtString str("123", 3);
        if (str.empty()) THROW;
        if (str.size() != 3) THROW;
        if (strcmp(str.data(), "123") != 0) THROW;
        if (strcmp(str.c_str(), "123") != 0) THROW;
    }

    {   // copy ctor
        JwtString src("123");
        JwtString str(src);
        if (str.empty()) THROW;
        if (str.size() != 3) THROW;
        if (strcmp(str.data(), "123") != 0) THROW;
        if (strcmp(str.c_str(), "123") != 0) THROW;
    }

    {   // find_first_of(char)
        JwtString str("123");
        if ( str.find_first_of('2', 0) != 1) THROW;
        if ( str.find_first_of('2', 3) != JwtString::npos) THROW;
        if ( str.find_first_of('4', 0) != JwtString::npos) THROW;
    }

    {   // find_first_of(char*)
        JwtString str("123");
        if ( str.find_first_of("32", 0) != 1) THROW;
        if ( str.find_first_of("23", 3) != JwtString::npos) THROW;
    }

    {   // substr
        JwtString str("1234");
        if ( strcmp("23", str.substr(1, 2).data()) != 0 ) THROW;
    }
    {   // clear
        JwtString str("1234");
        str.clear();
        if (! str.empty()) THROW;
    }

    {   // compare
        JwtString str("1234");
        if ( str.compare("23") >= 0 ) THROW;
        if ( str.compare("0123") <= 0 ) THROW;
        if ( str.compare("1234") != 0 ) THROW;

        if ( str.compare( JwtString("23") ) >= 0 ) THROW;
        if ( str.compare( JwtString("0123") ) <= 0 ) THROW;
        if ( str.compare( JwtString("1234") ) != 0 ) THROW;

        if ( str.compare( 1, 2, JwtString("01234"), 2, 2 ) != 0 ) THROW;
        if ( str.compare( 1, 2, "23" ) != 0 ) THROW;
        if ( str.compare( 1, 2, "234", 2 ) != 0 ) THROW;
    }

    {   // find
        JwtString str("1234");
        if ( str.find("23") != 1 ) THROW;
        if ( str.find("32") != JwtString::npos ) THROW;
    }

    {   // +=
        JwtString str("1234");
        str += JwtString("56");
        if ( str.compare("123456") != 0 ) THROW;
        str += '7';
        if ( str.compare("1234567") != 0 ) THROW;
    }

    {   // []
        JwtString str("1234");
        if ( str[1] != '2' ) THROW;
    }

    {   // <
        if ( ! ( JwtString("123") < JwtString("23") ) ) THROW;
    }

    {   // +
        if ( strcmp( ( JwtString("123") + "45" ) . data(), JwtString("12345").data() ) != 0 ) THROW;
        if ( strcmp( ( JwtString("123") + JwtString("45") ) . data(), JwtString("12345").data() ) != 0 ) THROW;
        if ( strcmp( ( JwtString("123") + '4' ) . data(), JwtString("1234").data() ) != 0 ) THROW;
    }

    {   // stoi
        if ( stoi ( JwtString("-123"), nullptr, 10 ) != -123 ) THROW;
        size_t idx = 0;
        if ( stoi ( JwtString("123fx"), &idx, 16 ) != 0x123F || idx != 4 ) THROW;
    }

    {   // stoll
        if ( stoll ( JwtString("-123"), nullptr, 10 ) != -123 ) THROW;
        size_t idx = 0;
        if ( stoll ( JwtString("123fx"), &idx, 16 ) != 0x123F || idx != 4 ) THROW;
    }

    {   // stold
        if ( stold ( JwtString("-12.3"), nullptr ) - -12.3L > DBL_EPSILON ) THROW;
        size_t idx = 0;
        if ( stold ( JwtString("1.23fx"), &idx ) - 1.23L > DBL_EPSILON || idx != 4 ) THROW;
    }

    {   // to_string
        if ( strcmp ( to_string ( 123u ) . data (), JwtString ( "123" ) . data () ) != 0 ) THROW;
        if ( strcmp ( to_string ( 123LL ) . data (), JwtString ( "123" ) . data () ) != 0 ) THROW;
    }

    return 0;
}

//////////////////////////////////////////// Main

#include <kapp/args.h>
#include <kfg/config.h>
#include <stdio.h>

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

const char UsageDefaultName[] = "test-jwt-c";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc = JWT_Example();
    if ( rc == 0 )
    {
        printf("No errors detected\n");
    }
    return rc;
}

}
