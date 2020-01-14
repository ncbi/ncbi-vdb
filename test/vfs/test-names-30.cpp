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


#include "../../libs/vfs/services-priv.h" /* KServiceNames3_0StreamTest */
#include "../../libs/vfs/path-priv.h" /* VPathEqual */
#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/rc.h>
#include <klib/text.h> /* CONST_STRING */
#include <klib/time.h> /* KTimeMakeTime */
#include <ktst/unit_test.hpp> /* KMain */
#include <vfs/path.h> /* VPath */
#include <vfs/services.h> /* KSrvResponse */
#include <vfs/services-priv.h> /* KServiceTestNamesExecuteExt */

#include "../../libs/vfs/resolver-cgi.h" /* RESOLVER_CGI */

//#include <cstdio> // printf

using std :: string;

static rc_t argsHandler(int argc, char* argv[]) {
    return ArgsMakeAndHandle ( NULL, argc, argv, 0, NULL, 0 );
}

TEST_SUITE_WITH_ARGS_HANDLER ( Names3_0_TestSuite, argsHandler );

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

class P {
    String _tick;
    String _id;
    size_t _size;

static int getDigit ( char c, rc_t * rc ) {
        assert ( rc );
 
        if ( * rc != 0 )
            return 0;
 
        c = tolower ( c );
        if ( ! isdigit ( c ) && c < 'a' && c > 'f' ) {
            * rc = RC ( rcVFS, rcQuery, rcExecuting, rcItem, rcIncorrect );
            return 0;
        }
 
        if ( isdigit ( c ) )
            return c - '0';
 
        return c - 'a' + 10;
    }

public:
    P ( const char * tick, const char * id, size_t size ) : _size ( size ) {
        size_t s = string_measure ( tick, NULL );
        StringInit ( & _tick, tick, s, s );

        s = string_measure ( id, NULL );
        StringInit ( & _id, id, s, s );
    }

    const VPath * make ( const char * path, const string & date = "",
        const string & md5 = "", KTime_t expiration = 0 )
    {
        KTime_t t = 0;
        if ( date . size () > 0 ) {
            KTime kt;
            const KTime * tt = KTimeFromIso8601 ( & kt, date . c_str (),
                date . size () );
            if ( tt != NULL )
                t = KTimeMakeTime ( & kt );
        }
	
        uint8_t ud5 [ 16 ];
        uint8_t * pd5 = NULL;
        if ( md5 . size () == 32 ) {
            rc_t rc = 0;
            for ( int i = 0; i < 16 && rc == 0; ++ i ) {
                ud5 [ i ]  = getDigit ( md5 [ 2 * i     ], & rc ) * 16;
                ud5 [ i ] += getDigit ( md5 [ 2 * i + 1 ], & rc );
            }
            pd5 = rc == 0 ? ud5 : NULL;
        }

        String url;
        size_t s = string_measure ( path, NULL );
        StringInit ( & url, path, s, s );

        VPath * p = NULL;
        rc_t rc = VPathMakeFromUrl ( & p, & url, & _tick, true, & _id, _size, t,
                pd5, expiration, NULL, NULL, NULL, false, false, NULL, -1, 0 );

        if ( rc == 0 )
            rc = VPathMarkHighReliability ( p, true );

        if ( rc != 0 )
            throw rc;

        return p;
    }
};

static P Path  ( "ticket", " object-id ", 90 );
static P Path1 ( " ticke1 ", " object-i1 ", 10 );

#if 1
TEST_CASE ( INCOMPLETE ) {
    const KSrvResponse * response = NULL;

    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( NULL, & response, 0 ) );
    REQUIRE_NULL ( response );

    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( "#3.0\n"
"0|| object-id |90|1930-01-13T13:25:30|0123456789abcdefABCDEF0123456789|ticket|"
"http://url/$fasp://frl/$https://hsl/$file:///p$s3:p||||"
"http://vdbcacheUrl/$fasp://fvdbcache/$https://vdbcache/$file:///vdbcache$s3:v|"
            "1490000000|200| message\n"
        "1|| object-i1 |10| dat1 | md1 | ticke1 |"
          "http://ur1/|https://vdbcacheUrl1/| expiratio1 |200| messag1\n"
        "$1500000000\n", NULL, 0 ) );
    REQUIRE_NULL ( response );
}

TEST_CASE ( SINGLE ) {
    const KSrvResponse * response = NULL;

    // incomplete string
    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( "#3.0\n"
        "SRR000001||http://dwnl.ncbi.nlm.nih.gov/srapub/SRR000001||||200|ok",
        & response, 0 ) );
    REQUIRE_NULL ( response );
    REQUIRE_RC ( KSrvResponseRelease (response ) );
    response = NULL;

    const string date ( "1980-01-13T13:25:30" );
    const KTime_t exp = 2000000000 ;
    const string md5  ( "0123456789abcdefABCDEF012345678a" );
    const VPath * ph = Path . make ( "http://url/"        , date, md5, exp );
    const VPath * vh = Path . make ( "http://vdbcacheUrl/",   "", md5  );
    const VPath * phs= Path . make ( "https://hsl/"       , date, md5, exp );
    const VPath * vhs= Path . make ( "https://vdbcache/"  , ""  , md5 );
    const VPath * pf = Path . make ( "fasp://frl/"        , date, md5, exp );
    const VPath * vf = Path . make ( "fasp://fvdbcache/"  , ""  , md5 );
    const VPath * pfl= Path . make ( "file:///p"          , date, md5, exp );
    const VPath * vfl= Path . make ( "file:///vdbcache"   , ""  , md5 );
    const VPath * p3 = Path . make ( "s3:p"               , date, md5, exp );
    const VPath * v3 = Path . make ( "s3:v"               , ""  , md5 );

    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.0\n"
"0|| object-id |90|1980-01-13T13:25:30|0123456789abcdefABCDEF012345678a|ticket|"
"http://url/$fasp://frl/$https://hsl/$file:///p$s3:p||||"
"http://vdbcacheUrl/$fasp://fvdbcache/$https://vdbcache/$file:///vdbcache$s3:v|"
            "2000000000|200| message\n"
        "$1500000000\n", & response, 0 ) );
    CHECK_NOT_NULL ( response );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );

    const VPath * path = NULL;
    const VPath * vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolDefault,
        & path, & vdbcache, NULL ) );
    int ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, ph, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, vh, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolHttp,
        & path, & vdbcache, NULL ) );
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, ph, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, vh, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolHttpHttps,
        & path, & vdbcache, NULL ) );
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, ph, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, vh, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolFaspHttp,
        & path, & vdbcache, NULL ) );
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, pf, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, vf, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolFileFaspHttpHttps,
        & path, & vdbcache, NULL ) );
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, pfl, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, vfl, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolHttpsHttp,
        & path, & vdbcache, NULL ) );
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, phs, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, vhs, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolS3,
        & path, & vdbcache, NULL ) );
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, p3, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathEqual ( vdbcache, v3, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;

    REQUIRE_RC ( KSrvResponseRelease (response ) );
    response = NULL;

    REQUIRE_RC ( VPathRelease ( ph ) );
    REQUIRE_RC ( VPathRelease ( vh ) );
    REQUIRE_RC ( VPathRelease ( phs) );
    REQUIRE_RC ( VPathRelease ( vhs) );
    REQUIRE_RC ( VPathRelease ( pf ) );
    REQUIRE_RC ( VPathRelease ( vf ) );
    REQUIRE_RC ( VPathRelease ( pfl) );
    REQUIRE_RC ( VPathRelease ( vfl) );
    REQUIRE_RC ( VPathRelease ( p3 ) );
    REQUIRE_RC ( VPathRelease ( v3 ) );
}

TEST_CASE ( DOUBLE ) {
    const KSrvResponse * response = NULL;

    const string date  (  "1981-01-13T13:25:30" );
    const KTime_t exp = 1489700000  ;
    const string date1 (  "1981-01-13T13:25:31" );
    const KTime_t exp1 = 1489710000  ;
    REQUIRE_RC ( KServiceNames3_0StreamTestMany ( "#3.0\n"
        "0|| object-id |90|1981-01-13T13:25:30||ticket|"
"http://url/$fasp://frl/$https://hsl/$file:///p$s3:p||||"
"http://vdbcacheUrl/$fasp://fvdbcache/$https://vdbcache/$file:///vdbcache$s3:v|"
            "1489700000|200| message\n"
        "1|| object-i1 |10|1981-01-13T13:25:31|| ticke1 |"
          "http://ur1/||||https://vdbcacheUrl1/|1489710000|200| messag1\n"
        "$1489690000\n", & response, 0, 2 ) );

    CHECK_NOT_NULL ( response );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 2u );

    const VPath * phs = Path . make ( "https://hsl/", date, "", exp );
    const VPath * path = NULL;
    const VPath * vdbcache = NULL;
    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolHttps,
        & path, NULL, NULL ) );
    int ne = ~0;
    REQUIRE_RC ( VPathEqual ( path, phs, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathRelease (phs ) );

    const VPath * ph = Path1 . make ( "http://ur1/", date1, "", exp1 );
    REQUIRE_RC ( KSrvResponseGetPath ( response, 1, eProtocolHttp,
        & path, & vdbcache, NULL ) );
    REQUIRE_NULL ( vdbcache );
    REQUIRE_RC ( VPathEqual ( path, ph, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (path ) );
    path = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathRelease (ph ) );

    const VPath * vhs = Path1 . make ( "https://vdbcacheUrl1/" );
    REQUIRE_RC ( KSrvResponseGetPath ( response, 1, eProtocolHttps,
        & path, & vdbcache, NULL ) );
    REQUIRE_NULL ( path );
    REQUIRE_RC ( VPathEqual ( vdbcache, vhs, & ne ) );
    REQUIRE_EQ ( ne, 0 );
    REQUIRE_RC ( VPathRelease (vdbcache ) );
    vdbcache = NULL;
    ne = ~0;
    REQUIRE_RC ( VPathRelease (vhs ) );

    REQUIRE_RC ( KSrvResponseRelease (response ) );
    response = NULL;
}

TEST_CASE ( BAD_TYPE ) {
    const KSrvResponse * response = NULL;
    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( "#3.0\n"
        "0|t| object-id |9|1981-01-13T13:25:30|0123456789abcdefABCDEF012345678b"
		"|ticket|||1981-01-13T13:25:30||||200| mssg\n",
        & response, 1 ) );
    REQUIRE_NULL ( response );
}

TEST_CASE ( ERROR ) {
    const KSrvResponse * response = NULL;
    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.0\n"
        "0|| object-id |90|1981-01-13T13:25:30|0123456789abcdefABCDEF012345678c"
		"|ticket||||||1489688000|500| mssg\n",
        & response, 1 ) );
    REQUIRE_NOT_NULL ( response );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );

    const VPath * path = NULL;
    const VPath * vdbcache = NULL;
    const KSrvError * error = NULL;
    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolDefault,
        & path, & vdbcache, & error ) );
    REQUIRE_NULL ( path );
    REQUIRE_NULL ( vdbcache );
    REQUIRE_NOT_NULL ( error );
    REQUIRE_RC ( KSrvErrorRelease ( error ) );

    REQUIRE_RC ( KSrvResponseRelease ( response ) );
    response = NULL;
}

TEST_CASE ( AND_ERROR ) {
    const KSrvResponse * response = NULL;
    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.0\n"
"0|na|object-0|90|1930-01-13T13:25:30|0123456789abcdefABCDEF012345678d|tckt0|||"
          "|||1489687900|503|e mssg\n"
"1||objc1|10|1931-01-13T13:25:31|0123456789abcdefABCDEF012345678e|1|http://u/||"
          "|||1489687200|200|messag\n"
        , & response, 1 ) );
    REQUIRE_NOT_NULL ( response );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 2u );

    const VPath * path = NULL;
    const VPath * vdbcache = NULL;
    const KSrvError * error = NULL;

    REQUIRE_RC ( KSrvResponseGetPath ( response, 0, eProtocolDefault,
        & path, & vdbcache, & error ) );
    REQUIRE_NULL ( path );
    REQUIRE_NULL ( vdbcache );
    REQUIRE_NOT_NULL ( error );
    rc_t rc = 0;
    REQUIRE_RC ( KSrvErrorRc ( error, & rc ) );
    REQUIRE_EQ ( rc,
        RC ( rcVFS, rcQuery, rcResolving, rcDatabase, rcNotAvailable ) );
    uint32_t code = 0;
    REQUIRE_RC ( KSrvErrorCode ( error, & code ) );
    REQUIRE_EQ ( code, 503u );
    String str;
    REQUIRE_RC ( KSrvErrorMessage ( error, & str ) );
    String exp;
    CONST_STRING ( & exp, "e mssg" );
    REQUIRE ( StringEqual ( & str, & exp ) );
/*  printf ( "KSrvErrorMessage: '%.*s'\n", (int)message. size, message. addr );
    printf ( "Expected        : '%.*s'\n", (int)exp    . size, exp    . addr );
    printf ( "KSrvErrorMessage.len: '%d'\n", message. len );
    printf ( "Expected        .len: '%d'\n", exp    . len );
    printf ( "KSrvErrorMessage.sz : '%ld'\n", message. size );
    printf ( "Expected        .sz : '%ld'\n", exp    . size );*/
    EObjectType type = eOT_undefined;
    REQUIRE_RC ( KSrvErrorObject ( error, & str, & type ) );
    REQUIRE_EQ ( type, eOT_na );
    CONST_STRING ( & exp, "object-0" );
    REQUIRE ( StringEqual ( & str, & exp ) );
    REQUIRE_RC ( KSrvErrorRelease ( error ) );

    REQUIRE_RC ( KSrvResponseGetPath ( response, 1, eProtocolDefault,
        & path, & vdbcache, & error ) );
    REQUIRE_NULL ( vdbcache );
    REQUIRE_NULL ( error );
    REQUIRE_NOT_NULL ( path );
    REQUIRE_RC ( VPathRelease ( path ) );

    REQUIRE_RC ( KSrvResponseRelease ( response ) );
    response = NULL;
}
#endif

static KConfig * KFG = NULL;

TEST_CASE ( FULL_TEST_NO_HTTP ) {
//  assert ( ! KDbgSetString ( "VFS" ) );

    REQUIRE_RC_FAIL ( KServiceMake ( NULL ) );

    KService * service = NULL;
    REQUIRE_RC ( KServiceMake ( & service ) );
    REQUIRE_NOT_NULL ( service );

    REQUIRE_RC_FAIL ( KServiceTestNamesExecuteExt ( service, 0, NULL, NULL,
        NULL, NULL ) );

    const KSrvResponse * response = NULL;

if ( 1 )
    REQUIRE_RC_FAIL ( KServiceTestNamesExecuteExt ( service, 0, NULL, NULL,
        & response, "" ) );

    REQUIRE_RC_FAIL ( KServiceAddId ( NULL, "SRR000001" ) );

    REQUIRE_RC ( KServiceAddId ( service, "SRR000001" ) );
#if 0
    REQUIRE_RC_FAIL ( KServiceTestNamesExecuteExt ( service, 0, NULL, "#3.0",
        & response, "" ) );
    REQUIRE_RC ( KServiceTestNamesExecuteExt ( service, 0, NULL, "#1.2",
        & response, NULL ) );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );
    REQUIRE_RC ( KSrvResponseRelease ( response ));

    REQUIRE_RC ( KServiceTestNamesExecuteExt ( service, 0, NULL, "#1.2",
        & response, "#1.2\n"
        "SRR000001||SRR000001|312527083|2015-04-07T21:54:15|"
        "9bde35fefa9d955f457e22d9be52bcd9||"
        "http://sra-download.ncbi.nlm.nih.gov/srapub/SRR000001|200|ok\n" ) );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );
    REQUIRE_RC ( KSrvResponseRelease ( response ));

    REQUIRE_RC ( KServiceTestNamesExecuteExt ( service, 0, NULL, NULL,
        & response, NULL ) );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );
    REQUIRE_RC ( KSrvResponseRelease ( response ));

    REQUIRE_RC ( KServiceTestNamesExecuteExt ( service, 0, NULL, NULL,
        & response, "#3.0\n"
        "srapub|SRR000001|312527083|2015-04-07T21:54:15|"
        "9bde35fefa9d955f457e22d9be52bcd9||"
        "http://sra-download.ncbi.nlm.nih.gov/srapub/SRR000001|200|ok\n" ) );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );
    REQUIRE_RC ( KSrvResponseRelease ( response ));
#endif

    REQUIRE_RC ( KServiceTestNamesExecuteExt ( service, 0, NULL, "#3.0",
        & response, "#3.0\n"
        "0|srapub|SRR000001|312527083|2015-04-07T21:54:15|"
        "9bde35fefa9d955f457e22d9be52bcd9||"
        "http://sra-download.ncbi.nlm.nih.gov/srapub/SRR000001||||||200|ok\n"
    ) );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 1u );
    REQUIRE_RC ( KSrvResponseRelease ( response ));

    REQUIRE_RC ( KServiceRelease ( service ) );

    REQUIRE_RC_FAIL ( KServiceTestNamesExecuteExt ( service, 0, NULL, NULL,
        NULL, NULL ) );
}

extern "C" {
    const char UsageDefaultName[] = "test-names-30";
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
        if ( 0 ) assert ( ! KDbgSetString ( "VFS" ) );
        KConfigDisableUserSettings ();

        rc_t rc = KConfigMake ( & KFG, NULL );
        if ( rc == 0 )
            rc = KConfigWriteString ( KFG,
                "repository/remote/main/CGI/resolver-cgi", RESOLVER_CGI );

        rc = Names3_0_TestSuite ( argc, argv );

        RELEASE ( KConfig, KFG );

        return rc;
    }
}
