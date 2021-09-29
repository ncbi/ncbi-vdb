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
#include <kns/http.h> /* KNSManagerMakeHttpFile */
#include <kns/manager.h> /* KNSManagerRelease */
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */

#define RELEASE( type, obj ) do { rc_t rc2 = type##Release ( obj ); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while ( false )

static rc_t argsHandler ( int argc, char * argv [] )
{   return ArgsMakeAndHandle ( NULL, argc, argv, 0, NULL, 0 ); }

TEST_SUITE_WITH_ARGS_HANDLER ( T200FOR_WHOLE_FILE, argsHandler )

TEST_CASE ( Test_206) {
    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

    const KFile * file = NULL;

#if GOOGLE_FILE_EXISTS
    REQUIRE_RC ( KNSManagerMakeHttpFile ( mgr, & file, NULL, 0x01010000,
       "https://sra-download.ncbi.nlm.nih.gov/traces/refseq/KC702174.1" ) );

    uint64_t size = 0;
    REQUIRE_RC ( KFileSize ( file, & size ) );

    void * buffer = malloc ( size );
    REQUIRE_NOT_NULL ( buffer );

    size_t num_read = 0;

    // read incomplete file: expect 206 response code
    size_t bsize = size - 1;
    REQUIRE_RC ( KFileRead ( file, 0, buffer, bsize, & num_read ) );
    REQUIRE_EQ ( num_read, bsize );

    // read the whole: storage.googleapis.com returns 200
    bsize = size;
    REQUIRE_RC ( KFileRead ( file, 0, buffer, bsize, & num_read ) );
    REQUIRE_EQ ( num_read, bsize );

    // request more that file size: expect exact file size
    REQUIRE_RC ( KFileRead ( file, 0, buffer, size * 2, & num_read ) );
    REQUIRE_EQ ( num_read, static_cast < size_t > ( size ) );

    free ( buffer );
#endif

    REQUIRE_RC ( KFileRelease ( file ) );

    REQUIRE_RC ( KNSManagerRelease ( mgr ) );
}

TEST_CASE ( Test_200 ) {
    KNSManager * mgr = NULL;
    REQUIRE_RC ( KNSManagerMake ( & mgr ) );

    const KFile * file = NULL;

#if GOOGLE_FILE_EXISTS
    REQUIRE_RC ( KNSManagerMakeHttpFile ( mgr, & file, NULL, 0x01010000,
       "https://storage.googleapis.com/yan-blastdb/2018-09-12-08-33-02/fuse.xml"
      ) );

    uint64_t size = 0;
    REQUIRE_RC ( KFileSize ( file, & size ) );

    void * buffer = malloc ( size );
    REQUIRE_NOT_NULL ( buffer );

    size_t num_read = 0;

    // read incomplete file: expect 206 response code
    size_t bsize = size - 1;
    REQUIRE_RC ( KFileRead ( file, 0, buffer, bsize, & num_read ) );
    REQUIRE_EQ ( num_read, bsize );

    // read the whole: storage.googleapis.com returns 200
    bsize = size;
    REQUIRE_RC ( KFileRead ( file, 0, buffer, bsize, & num_read ) );
    REQUIRE_EQ ( num_read, bsize );

    // request more that file size: expect exact file size
    REQUIRE_RC ( KFileRead ( file, 0, buffer, size * 2, & num_read ) );
    REQUIRE_EQ ( num_read, static_cast < size_t > ( size ) );

    free ( buffer );
#endif

    REQUIRE_RC ( KFileRelease ( file ) );

    REQUIRE_RC ( KNSManagerRelease ( mgr ) );
}

extern "C" {
    const char UsageDefaultName[] = "test200for-whole-file";
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) { if (
0 ) assert ( ! KDbgSetString ( "KNS-HTTP" ) );
        KConfigDisableUserSettings ();

        KConfig * kfg = NULL;
        rc_t rc = KConfigMakeEmpty ( & kfg );
     // turn off certificate validation to download from storage.googleapis.com
        if ( rc == 0 )
            rc = KConfigWriteString ( kfg, "/tls/allow-all-certs", "true" );

        if ( rc == 0 )
            rc = T200FOR_WHOLE_FILE ( argc, argv );

        RELEASE ( KConfig, kfg );

        return rc;
    }
}
