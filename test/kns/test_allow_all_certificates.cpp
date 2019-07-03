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

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/kfg-priv.h> /* KConfigMakeEmpty */
#include <kfs/file.h> /* KFileRelease */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/out.h>   /* KOutMsg */
#include <kns/http.h> /* KNSManagerMakeHttpFile */
#include <kns/manager.h> /* KNSManagerRelease */
#include <kns/tls.h> /* KNSManagerRelease */
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */

#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )

/*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*
 *  Simple two tests:
 *
 *  Conf ( false )                                = false
 *  Conf ( false ) and SetAllowAllCerts ( true )  = true
 *  Conf ( false ) and SetAllowAllCerts ( false ) = false
 *
 *  Conf ( true )                                 = true
 *  Conf ( true ) and SetAllowAllCerts ( false )  = false
 *  Conf ( true ) and SetAllowAllCerts ( true )   = true
 *
 *+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*/
const char * Bizdrapuda = "https://storage.googleapis.com/yan-blastdb/2018-09-12-08-33-02/fuse.xml";

static rc_t argsHandler ( int argc, char * argv [] )
{   return ArgsMakeAndHandle ( NULL, argc, argv, 0, NULL, 0 ); }

TEST_SUITE_WITH_ARGS_HANDLER ( ALLOW_ALL_CERTS, argsHandler )

TEST_CASE ( Test_ConfAllowCerts ) {

    KConfig * kfg = NULL;
    rc_t rc = KConfigMakeEmpty ( & kfg );
    REQUIRE_RC ( rc );
    REQUIRE_RC ( KConfigWriteString ( kfg, "/tls/allow-all-certs", "false" ) );

    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

        /*  First we shoud fail to read without any setting
         */
KOutMsg ( "##[1] Conf (false )                     = false\n" );
    const KFile * file = NULL;
    rc = KNSManagerMakeHttpFile ( mgr, & file, NULL, 0x01010000, Bizdrapuda );
    CHECK_NE ( rc, ( rc_t ) 0 );
    REQUIRE_NULL ( file );

    rc = 0;

    REQUIRE_RC ( KFileRelease ( file ) );
KOutMsg ( "##[1] OK : Conf (false )                     = false\n" );

        /*  Second, we call KNSManagerSetAllowAllCerts
         */
KOutMsg ( "##[2] Conf (false ) + SetAllow ( true ) = true\n" );
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, true ) );

    file = NULL;

#if GOOGLE_FILE_EXISTS

    REQUIRE_RC ( KNSManagerMakeHttpFile ( mgr, & file, NULL, 0x01010000, Bizdrapuda ) );
    REQUIRE_NOT_NULL ( file );

    uint64_t size = 0;
    REQUIRE_RC ( KFileSize ( file, & size ) );

    char buffer [ 64 ];
    size_t bsize = sizeof ( buffer ) - 1;
    size_t num_read = 0;

    REQUIRE_RC ( KFileRead ( file, 0, buffer, bsize, & num_read ) );
    REQUIRE_EQ ( num_read, bsize );

    REQUIRE_RC ( KFileRelease ( file ) );
KOutMsg ( "##[2] OK : Conf (false ) + SetAllow ( true ) = true\n" );

        /*  Third, we call KNSManagerSetAllowAllCerts again
         */
KOutMsg ( "##[3] Conf (false ) + SetAllow ( false ) = false\n" );
    REQUIRE_RC ( KNSManagerSetAllowAllCerts ( mgr, false ) );

    file = NULL;
    rc = KNSManagerMakeHttpFile ( mgr, & file, NULL, 0x01010000, Bizdrapuda );
    CHECK_NE ( rc, ( rc_t ) 0 );
    REQUIRE_NULL ( file );

    rc = 0;
    REQUIRE_RC ( KFileRelease ( file ) );

KOutMsg ( "##[3] OK : Conf (false ) + SetAllow ( false ) = false\n" );
#endif

    REQUIRE_RC ( KNSManagerRelease ( mgr ) );

    RELEASE ( KConfig, kfg );
}

extern "C" {
    const char UsageDefaultName[] = "test200for-whole-file";
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) { if (
0 ) assert ( ! KDbgSetString ( "KNS-HTTP" ) );
        KConfigDisableUserSettings ();

     // turn off certificate validation to download from storage.googleapis.com

     rc_t rc = ALLOW_ALL_CERTS ( argc, argv );

     return rc;
    }
}
