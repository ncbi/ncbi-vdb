#include "../../libs/vfs/services-priv.h" /* KServiceNames3_0StreamTest */
#include "../../libs/vfs/path-priv.h" /* VPathEqual */
#include <vfs/services.h> /* KSrvResponse */
#include <vfs/path.h> /* VPath */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/rc.h>
#include <klib/text.h> /* CONST_STRING */
#include <ktst/unit_test.hpp> /* KMain */

//#include <cstdio> // printf

TEST_SUITE ( Names3_0_TestSuite );

class P {
    String _tick;
    String _id;
    size_t _size;

public:
    P ( const char * tick, const char * id, size_t size ) : _size ( size ) {
        size_t s = string_measure ( tick, NULL );
        StringInit ( & _tick, tick, s, s );

        s = string_measure ( id, NULL );
        StringInit ( & _id, id, s, s );
    }

    const VPath * make ( const char * path ) {
        String url;
        size_t s = string_measure ( path, NULL );
        StringInit ( & url, path, s, s );

        VPath * p = NULL;
        rc_t rc
            = VPathMakeFromUrl ( & p, & url, & _tick, true, & _id, _size, 0 );

        if ( rc == 0 )
            rc = VPathMarkHighReliability ( p, true );

        if ( rc != 0 )
            throw rc;

        return p;
    }
};

static P Path  ( " ticket ", " object-id ", 90 );
static P Path1 ( " ticke1 ", " object-i1 ", 10 );

TEST_CASE ( INCOMPLETE ) {
    const KSrvResponse * response = NULL;

    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( NULL, & response, 0 ) );
    REQUIRE_NULL ( response );

    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( "#3.2\n"
        "0|| object-id |90| date | md5 | ticket |"
"http://url/$fasp://frl/$https://hsl/$file:///p$s3:p|"
"http://vdbcacheUrl/$fasp://fvdbcache/$https://vdbcache/$file:///vdbcache$s3:v|"
            " expiration |200| message\n"
        "1|| object-i1 |10| dat1 | md1 | ticke1 |"
          "http://ur1/|https://vdbcacheUrl1/| expiratio1 |200| messag1\n"
        "$ timestamp\n", NULL, 0 ) );
    REQUIRE_NULL ( response );
}

TEST_CASE ( SINGLE ) {
    const KSrvResponse * response = NULL;

    // incomplete string
    REQUIRE_RC_FAIL ( KServiceNames3_0StreamTest ( "#3.2\n"
        "SRR000001||http://dwnl.ncbi.nlm.nih.gov/srapub/SRR000001|200|ok",
        & response, 0 ) );
    REQUIRE_NULL ( response );
    REQUIRE_RC ( KSrvResponseRelease (response ) );
    response = NULL;

    const VPath * ph = Path . make ( "http://url/" );
    const VPath * vh = Path . make ( "http://vdbcacheUrl/" );
    const VPath * phs= Path . make ( "https://hsl/" );
    const VPath * vhs= Path . make ( "https://vdbcache/" );
    const VPath * pf = Path . make ( "fasp://frl/" );
    const VPath * vf = Path . make ( "fasp://fvdbcache/" );
    const VPath * pfl= Path . make ( "file:///p" );
    const VPath * vfl= Path . make ( "file:///vdbcache" );
    const VPath * p3 = Path . make ( "s3:p" );
    const VPath * v3 = Path . make ( "s3:v" );

    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.2\n"
        "0|| object-id |90| date | md5 | ticket |"
"http://url/$fasp://frl/$https://hsl/$file:///p$s3:p|"
"http://vdbcacheUrl/$fasp://fvdbcache/$https://vdbcache/$file:///vdbcache$s3:v|"
            " expiration |200| message\n"
        "$ timestamp\n", & response, 0 ) );
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

    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.2\n"
        "0|| object-id |90| date | md5 | ticket |"
"http://url/$fasp://frl/$https://hsl/$file:///p$s3:p|"
"http://vdbcacheUrl/$fasp://fvdbcache/$https://vdbcache/$file:///vdbcache$s3:v|"
            " expiration |200| message\n"
        "1|| object-i1 |10| dat1 | md1 | ticke1 |"
          "http://ur1/|https://vdbcacheUrl1/| expiratio1 |200| messag1\n"
        "$ timestamp\n", & response, 0 ) );

    CHECK_NOT_NULL ( response );
    REQUIRE_EQ ( KSrvResponseLength ( response ), 2u );

    const VPath * phs = Path . make ( "https://hsl/" );
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

    const VPath * ph = Path1 . make ( "http://ur1/" );
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

TEST_CASE ( ERROR ) {
    const KSrvResponse * response = NULL;
    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.2\n"
        "0|| object-id |90| date | md5 | ticket |||" " expiration |500| mssg\n",
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
    REQUIRE_RC ( KServiceNames3_0StreamTest ( "#3.2\n"
        "0||object-0|90| dat0 | md50 | tckt0         |||expiratin0|503|e mssg\n"
        "1||object-1|10| dat1 | md51 | tckt1|http://u/||expiraton1|200|messag\n"
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
    String message;
    REQUIRE_RC ( KSrvErrorMessage ( error, & message ) );
    String exp;
    CONST_STRING ( & exp, "e mssg" );
    REQUIRE ( StringEqual ( & message, & exp ) );
/*  printf ( "KSrvErrorMessage: '%.*s'\n", (int)message. size, message. addr );
    printf ( "Expected        : '%.*s'\n", (int)exp    . size, exp    . addr );
    printf ( "KSrvErrorMessage.len: '%d'\n", message. len );
    printf ( "Expected        .len: '%d'\n", exp    . len );
    printf ( "KSrvErrorMessage.sz : '%ld'\n", message. size );
    printf ( "Expected        .sz : '%ld'\n", exp    . size );*/
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

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char * argv [] ) {
        if ( 0 ) assert ( ! KDbgSetString ( "VFS" ) );
        return Names3_0_TestSuite ( argc, argv );
    }
}
