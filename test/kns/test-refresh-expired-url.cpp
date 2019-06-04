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
* =========================================================================== */

#include "HttpFixture.hpp"

#include <ktst/unit_test.hpp>

#include <kfg/kfg-priv.h>
#include <kns/kns-mgr-priv.h>
#include <kfs/file.h>
#include <klib/time.h>

#include <cassert>
#include <sstream>

using namespace std;

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

static rc_t argsHandler ( int argc, char * argv [] );
TEST_SUITE_WITH_ARGS_HANDLER ( HttpRefreshTestSuite, argsHandler );

FIXTURE_TEST_CASE( HttpRefreshTestSuite, HttpFixture )
{
    string url = MakeURL(GetName()).c_str();
    // Request 7 bytes
    const size_t dataSize = 7;
    {
        ostringstream ostr;
        ostr << "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: " << dataSize << "\r\n";
        TestStream::AddResponse( ostr.str() );
    }
    REQUIRE_RC ( KNSManagerMakeReliableHttpFile( m_mgr, ( const KFile** ) &  m_file, & m_stream, 0x01010000, url . c_str (), true ) );
    REQUIRE_NOT_NULL ( m_file ) ;

    // simulates a response to HEAD from the "signer" service. Pretend this is the object we need
    #define NEW_URL "https://sra-download.ncbi.nlm.nih.gov/traces/sra4/SRR/000000/SRR000123"
    // 65 seconds in the future; the refresh timer will be set to 60 seconds before that
    char expirationStr[100];
    KTimeIso8601 ( KTimeStamp () + 65, expirationStr, sizeof expirationStr );
    string resp = string ( "HTTP/1.1 307 Temporary Redirect\r\n"
                           "Location: " NEW_URL "\r\n"
                           "Expires: " ) + expirationStr + "\r\n";
    TestStream::AddResponse( resp );

    char buf[1024];
    size_t num_read;
    REQUIRE_RC( KFileTimedRead ( m_file, 0, buf, sizeof buf, & num_read, NULL ) );
    REQUIRE_EQ ( dataSize, num_read );

    // make sure expiration time is reflected on the HttpFile object
}

//////////////////////////////////////////// Main

#include <kapp/args.h> // Args
#include <klib/debug.h>
#include <kfg/config.h>

static rc_t argsHandler ( int argc, char * argv [] ) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle ( & args, argc, argv, 0, NULL, 0 );
    ArgsWhack ( args );
    return rc;
}

extern "C" {
    const char UsageDefaultName[] = "test-refresh-expired";
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] )
    {
        if ( 1 ) assert ( ! KDbgSetString ( "KNS-HTTP" ) );
        KConfigDisableUserSettings ();

        KConfig * kfg = NULL;
        rc_t rc = KConfigMakeEmpty ( & kfg );
     // turn off certificate validation to download from storage.googleapis.com
        if ( rc == 0 )
            rc = KConfigWriteString ( kfg, "/tls/allow-all-certs", "true" );

        if ( rc == 0 )
            rc = HttpRefreshTestSuite ( argc, argv );

        RELEASE ( KConfig, kfg );

        return rc;
    }
}

