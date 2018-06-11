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
* ==============================================================================
*
*/

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kfg/config.h> /* KConfig */
#include <kfs/directory.h> /* KDirectory */
#include <kfs/file.h> /* KFileRelease */
#include <klib/debug.h> /* KDbgSetString */
#include <kns/kns-mgr-priv.h> /* KNSManagerMakeReliableHttpFile */
#include <kns/manager.h> /* KNSManagerMake */
#include <ktst/unit_test.hpp>
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/resolver.h> /* VResolver */
#include <vfs/path.h> /* VPath */
#include <climits> /* PATH_MAX */

#define ALL

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

using ncbi::NK::TestCase;
using std::cerr;
using std::string;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(flatSraKfgTestSuite, argsHandler);

static KNSManager * kns = NULL;

class Test : private TestCase {
    TestCase *_dad;

    void compare ( const VPath * remote, const string & expected ) {
        assert ( expected . size ( ) );
        char buffer [ PATH_MAX ] = "";
        size_t num_read = 0;
        REQUIRE_RC( VPathReadUri ( remote, buffer, sizeof buffer, & num_read) );
        REQUIRE ( num_read );
        REQUIRE_LT ( num_read, sizeof buffer );
        REQUIRE_EQ ( buffer [ num_read ], '\0' );
        REQUIRE_EQ ( expected, string ( buffer ) );
    }

public:
    Test(TestCase *dad, const VFSManager * manager, const KConfig * config,
            const VPath * queryShort, const VPath * queryLong,
            const string & name, const string & expectedShort = "",
                                 const string & expectedLong  = "")
        : TestCase(name), _dad(dad)
    {
        rc_t rc = 0;

        VResolver * resolver = NULL;
        REQUIRE_RC ( VFSManagerMakeResolver ( manager, & resolver, config ) );

        const VPath * remote = NULL;
        const KFile * f = NULL;
        if ( expectedShort . size () > 0 ) {
            REQUIRE_RC ( VResolverQuery ( resolver, eProtocolHttps, queryShort,
                                          NULL, & remote, NULL ) );
            compare ( remote, expectedShort );
            RELEASE ( VPath, remote );
            REQUIRE_RC ( KNSManagerMakeReliableHttpFile
                ( kns, & f, NULL, 0x01010000, expectedShort . c_str () ) );
            RELEASE ( KFile, f );
        } else {
            REQUIRE_RC_FAIL ( VResolverQuery
                ( resolver, 0, queryShort, NULL, & remote, NULL ) );
        }

        if ( expectedLong . size () > 0 ) {
            REQUIRE_RC ( VResolverQuery ( resolver, eProtocolHttps, queryLong,
                         NULL, & remote, NULL ) );
            compare ( remote, expectedLong );
            RELEASE ( VPath, remote );
            REQUIRE_RC ( KNSManagerMakeReliableHttpFile
                ( kns, & f, NULL, 0x01010000, expectedLong . c_str () ) );
            RELEASE ( KFile, f );
        } else {
            REQUIRE_RC_FAIL ( VResolverQuery
                ( resolver, 0, queryLong, NULL, & remote, NULL ) );
        }


        RELEASE ( VResolver, resolver );

        REQUIRE_EQ ( rc, 0u );

    }

    ~Test(void) {
        assert(_dad);
        _dad->ErrorCounterAdd(GetErrorCounter());
    }
};

static const char cgiPath[] = "/repository/remote/main/CGI/resolver-cgi";
static const char goodCgi[]
    = "https://www.ncbi.nlm.nih.gov/Traces/names/names.fcgi";
static const char badCgi[]
    = "https://XXX.ncbi.nlm.nih.gov/Traces/names/names.fcgi";
#ifdef ALL
TEST_CASE(test_sra) {
    const string newShort
     ("https://sra-download.ncbi.nlm.nih.gov/traces/sra27/SRR/000000/SRR000001");
    const string newLong
    ("https://sra-download.ncbi.nlm.nih.gov/traces/sra14/SRR/000976/SRR1000254");
    const string oldShort("https://ftp-trace.ncbi.nlm.nih.gov/sra/sra-instant/"
                "reads/ByRun/sra/SRR/SRR000/SRR000001/SRR000001.sra");

    rc_t rc = 0;

    KDirectory * native = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( & native ) );

    const KDirectory * dirc = NULL;
    KConfig * cfg = NULL;
    VFSManager * mgr = NULL;

    // LOAD OLD CONFIGURATION
    REQUIRE_RC(KDirectoryOpenDirRead(native, &dirc, false, "flat-sra-kfg/old"));
    REQUIRE_RC ( KConfigMake ( & cfg, dirc ) );
    REQUIRE_RC ( VFSManagerMakeFromKfg ( & mgr, cfg ) );

    VPath * queryShort = NULL;
    REQUIRE_RC ( VFSManagerMakeAccPath ( mgr, & queryShort, "SRR000001" ) );

    VPath * queryLong = NULL;
    REQUIRE_RC ( VFSManagerMakeAccPath ( mgr, & queryLong, "SRR1000254" ) );

//  const char rootPath [] = "/repository/remote/aux/NCBI/root";
//  const char newRoot[]  = "https://sra-download.ncbi.nlm.nih.gov";

    // fail using incomplete configuration
    Test(this, mgr, cfg, queryShort, queryLong, "incomplete-old");

    // resolve using correct cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, goodCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "cgi-old", newShort, newLong);

    // fail using incorrect cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, badCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "bad cgi-old");

/* aux repositories are ignored : VDB-3090 
    // old aux configuration cannot resolve long accession
    const char oldRoot  []  = "https://ftp-trace.ncbi.nlm.nih.gov/sra";
    REQUIRE_RC ( KConfigWriteString ( cfg, rootPath, oldRoot ) );
    Test(this, mgr, cfg, queryShort, queryLong, "aux-old", oldShort);
*/

    // LOAD NEW CONFIGURATION
    RELEASE ( VFSManager, mgr );
    RELEASE ( KConfig, cfg );
    RELEASE ( KDirectory, dirc );

    REQUIRE_RC(KDirectoryOpenDirRead(native, &dirc, false, "flat-sra-kfg/new"));
    REQUIRE_RC ( KConfigMake ( & cfg, dirc ) );
    REQUIRE_RC ( VFSManagerMakeFromKfg ( & mgr, cfg ) );

    // fail using incomplete configuration
    Test(this, mgr, cfg, queryShort, queryLong, "incomplete-new");

    // resolve using correct cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, goodCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "cgi-new", newShort, newLong);

    // fail using incorrect cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, badCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "bad cgi-new");

/* aux repositories are ignored : VDB-3090 
    // resolve using new aux configuration
    REQUIRE_RC ( KConfigWriteString ( cfg, rootPath, newRoot ) );
    Test(this, mgr, cfg, queryShort, queryLong, "aux-new", newShort, newLong);
*/

    RELEASE ( VFSManager, mgr );
    RELEASE ( KConfig, cfg );
    RELEASE ( KDirectory, dirc );

    RELEASE ( VPath, queryLong );

    RELEASE ( VPath, queryShort );

    RELEASE ( KDirectory, native );

    REQUIRE_EQ ( rc, 0u );
}
#endif

class Fixture : private TestCase {
    TestCase *_dad;
public:
    Fixture(TestCase *dad, const string & tname,
             const char * acc, const char * name, const char * value,
             const char * expected = "" )
        : TestCase(tname), _dad(dad)
    {
        rc_t rc = 0;

        KDirectory * native = NULL;
        REQUIRE_RC ( KDirectoryNativeDir ( & native ) );

        const KDirectory * dir = NULL;
        REQUIRE_RC(KDirectoryOpenDirRead
            (native, &dir, false, "flat-sra-kfg/aux_root"));

        KConfig * cfg;
        REQUIRE_RC ( KConfigMake ( & cfg, dir ) );

        VFSManager * mgr = NULL;
        REQUIRE_RC ( VFSManagerMakeFromKfg ( & mgr, cfg ) );

        VPath * query = NULL;
        if ( string ( value ) == "refseq" ) {
            REQUIRE_RC ( VFSManagerMakeAccPath
                ( mgr, & query, "ncbi-acc:%s?vdb-ctx=refseq", acc ) );
        } else {
            REQUIRE_RC ( VFSManagerMakeAccPath ( mgr, & query, acc ) );
        }

        const VPath * remoteCgi = NULL;
        const VPath * remote = NULL;
        VResolver * resolver = NULL;

        REQUIRE_RC ( VFSManagerMakeResolver ( mgr, & resolver, cfg ) );
// fail to resolve using empty config
        REQUIRE_RC_FAIL ( VResolverQuery
            ( resolver, 0, query, NULL, & remote, NULL ) );
        RELEASE ( VResolver, resolver );

// resolve using good cgi
        REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, goodCgi ) );
        REQUIRE_RC ( VFSManagerMakeResolver ( mgr, & resolver, cfg ) );

        REQUIRE_RC ( VResolverQuery
            ( resolver, 0, query, NULL, & remoteCgi, NULL ) );
        if ( ! expected ) {
cerr << "\nTO FIX !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :\n "
            << "\tVResolverQuery(CGI, " << acc << ") SHOULD FAIL !!!!!!!!!!!\n";
        }

        RELEASE ( VResolver, resolver );

// fail to resolve using bad cgi
        REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, badCgi ) );
        REQUIRE_RC ( VFSManagerMakeResolver ( mgr, & resolver, cfg ) );
        REQUIRE_RC_FAIL ( VResolverQuery
            ( resolver, 0, query, NULL, & remote, NULL ) );
        RELEASE ( VResolver, resolver );

// resolve using aux configuration
        REQUIRE_RC ( KConfigWriteString ( cfg, name, value ) );
        REQUIRE_RC ( VFSManagerMakeResolver ( mgr, & resolver, cfg ) );
        REQUIRE_RC ( VResolverQuery
            ( resolver, 0, query, NULL, & remote, NULL ) );
        if ( ! expected ) {
cerr << "TO FIX !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :\n "
            << "\tVResolverQuery(AUX, " << acc << ") SHOULD FAIL !!!!!!!!!!!\n";
        }
        RELEASE ( VResolver, resolver );

// compare cgi and aux results
        char buffer [ PATH_MAX ] = "";
        size_t num_read = 0;
        REQUIRE_RC (VPathReadUri(remote, buffer, sizeof buffer, & num_read));
        REQUIRE ( num_read );
        REQUIRE_LT ( num_read, sizeof buffer );
        REQUIRE_EQ ( buffer [ num_read ], '\0' );

        char buffrC [ PATH_MAX ] = "";
        size_t num_readC = 0;
        REQUIRE_RC(VPathReadUri(remoteCgi, buffrC, sizeof buffrC, &num_readC) );
        REQUIRE ( num_readC );
        REQUIRE_LT ( num_readC, sizeof buffrC );
        REQUIRE_EQ ( buffrC [ num_readC ], '\0' );

if ((string(acc) != "AAAB01" || string(value) != "refseq")
  && (num_read == num_readC))
{ // https://jira.ncbi.nlm.nih.gov/browse/VDB-3046?focusedCommentId=4421122&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-4421122
        REQUIRE_EQ ( string ( buffer ) , string ( buffrC ) );
        REQUIRE_EQ ( num_read, num_readC );
        const KFile * f = NULL;
        REQUIRE_RC
            (KNSManagerMakeReliableHttpFile(kns, &f, NULL, 0x01010000, buffer));
        RELEASE ( KFile, f );
        if ( string (acc) == "AAAB01.1"
          && expected && expected [ 0 ] && string ( expected ) != buffer)
        {
cerr << "\nTO FIX !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :\n "
            << acc << "(" << tname << "):\tREAL    : " << buffer << "\n"
             "                        \tEXPECTED: " << expected << "\n\n";
        } else {
            assert ( ! expected || ! expected [ 0 ] );
            cerr << acc << "(" << tname << "): " << buffer << "\n";
        }
} else
cerr << "\nTO FIX !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :\n "
            << acc << "(" << tname << "): AUX: " << buffer << "\n"
                 "                        CGI: " << buffrC << "\n\n";

        RELEASE ( VPath, remote );
        RELEASE ( VPath, remoteCgi );

        RELEASE ( VPath, query );

        RELEASE ( VFSManager, mgr );

        RELEASE ( KConfig, cfg );

        RELEASE ( KDirectory, dir );

        RELEASE ( KDirectory, native );

        REQUIRE_EQ ( rc, 0u );
    }
    ~Fixture(void) {
        assert ( _dad );
        rc_t rc = 0;
        REQUIRE_EQ ( rc, 0u );
        _dad->ErrorCounterAdd(GetErrorCounter());
    }
};
/* aux repositories are ignored : VDB-3090 
#ifdef ALL
TEST_CASE(test_nakmer) {
    Fixture fixture(this, "nakmer", "GCA_000391885.1_R",
        "/repository/remote/aux/NCBI/apps/nakmer/volumes/fuseNAKMER", "sadb");
}
TEST_CASE(test_nannot) {
    Fixture fixture(this, "nannot", "NA000000007.1",
        "/repository/remote/aux/NCBI/apps/nannot/volumes/fuseNANNOT", "sadb");
}
#endif
#ifdef ALL
TEST_CASE(test_AAAB01_1) {
    Fixture fixture(this, "refseq AAAB01.1", "AAAB01.1",
        "/repository/remote/aux/NCBI/apps/refseq/volumes/refseq", "refseq",
        "https://ftp-trace.ncbi.nlm.nih.gov/sra/refseq/AAAB01" );
}
#endif
#ifdef ALL
TEST_CASE(test_AAAB01) {
    Fixture fixture(this, "refseq AAAB01", "AAAB01",
        "/repository/remote/aux/NCBI/apps/refseq/volumes/refseq", "refseq" );
}
#endif
#ifdef ALL
TEST_CASE(test_AAAB01000001) {
    Fixture fixture(this, "refseq AAAB01000001", "AAAB01000001",
        "/repository/remote/aux/NCBI/apps/refseq/volumes/refseq", "refseq" );
}
#endif
/ * TEST_CASE(test_AAAB01_2) {
    Fixture fixture(this, "refseq AAAB01.2", "AAAB01.2",
        "/repository/remote/aux/NCBI/apps/refseq/volumes/refseq", "refseq" );
} * /
#ifdef ALL
TEST_CASE(test_KC702199) {
    Fixture fixture(this, "refseq KC702199.1", "KC702199.1",
        "/repository/remote/aux/NCBI/apps/refseq/volumes/refseq", "refseq" );
}
#endif
TEST_CASE(test_WGS) {
    Fixture fixture(this, "WGS", "AFVF01.1",
        "/repository/remote/aux/NCBI/apps/wgs/volumes/fuseWGS", "wgs" );
}
TEST_CASE(test_WGS_AAAB01) {
    Fixture fixture(this, "WGS AAAB01", "AAAB01",
        "/repository/remote/aux/NCBI/apps/wgs/volumes/fuseWGS", "wgs" );
}
TEST_CASE(test_WGS_AAAB01_1) {
    Fixture fixture(this, "WGS AAAB01_1", "AAAB01.1",
        "/repository/remote/aux/NCBI/apps/wgs/volumes/fuseWGS", "wgs",
        "https://ftp-trace.ncbi.nlm.nih.gov/sra/wgs/AA/AB/AAAB01.1" );
}
TEST_CASE(test_WGS_AAAB01_9) {
    Fixture fixture(this, "WGS AAAB01_2", "AAAB01.9",
        "/repository/remote/aux/NCBI/apps/wgs/volumes/fuseWGS", "wgs", NULL );
}*/

static rc_t argsHandler(int argc, char * argv[]) {
    Args * args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}
rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName [] = "flat-sra-kfg";
rc_t CC UsageSummary ( const char * prog_name ) { return 0; }
extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char *argv [] ) {
const char * p = getenv("http_proxy");
//cerr << "http_proxy = '" << ( p == NULL ? "NULL" : p ) << "'\n";
if ( 1 ) assert ( ! KDbgSetString ( "VFS" ) );
        KConfigDisableUserSettings();
        rc_t rc = KNSManagerMake(&kns);
        if (rc == 0) {
            rc = flatSraKfgTestSuite(argc, argv);
        }
        RELEASE(KNSManager, kns);
        return rc;
    }
}
