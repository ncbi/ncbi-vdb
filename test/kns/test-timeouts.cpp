/*==============================================================================
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

#include <kns/http.h> /* KNSManagerMakeHttpFile */
#include <kns/http-priv.h> /* HttpFileGetReadTimeouts */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeConfig */
#include <kns/manager.h> /* KNSManagerRelease */

#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */

#include "../../libs/kns/http-priv.h" /* MAX_HTTP_READ_LIMIT */
#include "../../libs/kns/mgr-priv.h" /* KNSManager */

static rc_t argsHandler ( int argc, char * argv [] )
{   return ArgsMakeAndHandle ( NULL, argc, argv, 0, NULL, 0 ); }

TEST_SUITE_WITH_ARGS_HANDLER ( TEST_TIMEOUTS, argsHandler )

TEST_CASE ( TestTimeouts ) {
    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeEmpty(&kfg));
    REQUIRE_RC(KConfigWriteString(kfg, "/http/reliable/retryFile", "false"));
    REQUIRE_RC(KConfigWriteString(kfg, "/http/reliable/wait", "-1"));
//  REQUIRE_RC(KConfigPrint(kfg, 0));
    KNSManager * mgr = NULL;
    REQUIRE_RC(KNSManagerMakeConfig(&mgr, kfg));
    const KFile * file = NULL;
    const ver_t vers(0x01010000);
    const char url[] =
        "https://sra-download.ncbi.nlm.nih.gov/traces/refseq/KC702174.1";
    int32_t millis = 0, totalMillis = 0;

    { //////////////////////////////////////////////////////////////////////////
        REQUIRE(!mgr->retryFile);

        /* don't use StableHttpFile; default values;
           no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, MAX_HTTP_READ_LIMIT);
        REQUIRE_EQ(totalMillis, MAX_HTTP_READ_LIMIT);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, MAX_HTTP_READ_LIMIT);
        REQUIRE_EQ(totalMillis, MAX_HTTP_READ_LIMIT);
        REQUIRE_RC(KFileRelease(file));

        /* don't use StableHttpFile; default values;
           no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, MAX_HTTP_READ_LIMIT);
        REQUIRE_EQ(totalMillis, MAX_HTTP_READ_LIMIT);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = true;

        /* use StableHttpFile: there is retry loop; default values;
          no retry for unreliable URL-s */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, MAX_HTTP_READ_LIMIT);
        REQUIRE_EQ(totalMillis, MAX_HTTP_READ_LIMIT);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, MAX_HTTP_READ_LIMIT);
        REQUIRE_EQ(totalMillis, MAX_HTTP_READ_LIMIT);
        REQUIRE_RC(KFileRelease(file));

        /* use StableHttpFile: there is retry loop; default values;
          infinite timeout loop for reliable URLs is disallowed */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, MAX_HTTP_READ_LIMIT);
        REQUIRE_EQ(totalMillis, MAX_HTTP_TOTAL_READ_LIMIT);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    // set http_read_timeout and maxTotalWaitForReliableURLs_ms
    const int max_http_read_limit(123456);
    mgr->http_read_timeout = max_http_read_limit;
    const int max_http_total_read_limit(234567);
    mgr->maxTotalWaitForReliableURLs_ms = max_http_total_read_limit;

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = false;

        /* don't use StableHttpFile; no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        /* don't use StableHttpFile;  no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = true;

        /* use StableHttpFile: there is retry loop;
          no retry for unreliable URL-s */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        /* use StableHttpFile: there is retry loop */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_total_read_limit);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    // set http_read_timeout to -1 (infinite)
    mgr->http_read_timeout = -1;
    mgr->maxTotalWaitForReliableURLs_ms = max_http_total_read_limit;

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = false;

        /* don't use StableHttpFile; no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        /* don't use StableHttpFile;  no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = true;

        /* use StableHttpFile: there is retry loop;
          no retry for unreliable URL-s */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        /* use StableHttpFile: there is retry loop */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    // set maxTotalWaitForReliableURLs_ms to -1 (infinite)
    mgr->http_read_timeout = max_http_read_limit;
    mgr->maxTotalWaitForReliableURLs_ms = -1;

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = false;

        /* don't use StableHttpFile; no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        /* don't use StableHttpFile;  no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = true;

        /* use StableHttpFile: there is retry loop;
          no retry for unreliable URL-s */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, max_http_read_limit);
        REQUIRE_RC(KFileRelease(file));

        /* use StableHttpFile: there is retry loop */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, max_http_read_limit);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    // set both timeouts to -1 (infinite)
    mgr->http_read_timeout = mgr->maxTotalWaitForReliableURLs_ms = -1;

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = false;

        /* don't use StableHttpFile; no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        /* don't use StableHttpFile;  no retry loop => totalMillis == millis */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    { //////////////////////////////////////////////////////////////////////////
        mgr->retryFile = true;

        /* use StableHttpFile: there is retry loop;
          no retry for unreliable URL-s */
        REQUIRE_RC(KNSManagerMakeHttpFile(mgr, &file, NULL, vers, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, false,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));

        /* use StableHttpFile: there is retry loop */
        REQUIRE_RC(KNSManagerMakeReliableHttpFile(mgr, &file, NULL, vers, true,
            false, false, url));
        REQUIRE(KFileIsKHttpFile(file));
        REQUIRE_RC(HttpFileGetReadTimeouts(file, &millis, &totalMillis));
        REQUIRE_EQ(millis, -1);
        REQUIRE_EQ(totalMillis, -1);
        REQUIRE_RC(KFileRelease(file));
    } //////////////////////////////////////////////////////////////////////////

    REQUIRE_RC(KNSManagerRelease(mgr));
    REQUIRE_RC(KConfigRelease(kfg));
}

extern "C" {
    const char UsageDefaultName[] = "test-timeouts";
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) {
        KConfigDisableUserSettings ();
        return TEST_TIMEOUTS( argc, argv );
    }
}
