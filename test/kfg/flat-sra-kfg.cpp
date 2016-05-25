////////////////////////////////////////////////////////////////////////////////

#include <kfg/config.h> /* KConfig */
#include <kfs/directory.h> /* KDirectory */
#include <ktst/unit_test.hpp>
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/resolver.h> /* VResolver */
#include <vfs/path.h> /* VPath */
#include <climits> /* PATH_MAX */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

using std::string;

TEST_SUITE(flatSraKfgTestSuite);

class Test : private ncbi::NK::TestCase {
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
            const string &name, const string & expectedShort = "",
                                const string & expectedLong  = "")
        : TestCase(name), _dad(dad)
    {
        rc_t rc = 0;

        VResolver * resolver = NULL;
        REQUIRE_RC ( VFSManagerMakeResolver ( manager, & resolver, config ) );

        const VPath * remote = NULL;
        if ( expectedShort . size () ) {
/* TO FIX : THE FOLLOWING CALL FAILS
SRR000001 RESOLVES TO
http://sra-download.ncbi.nlm.nih.gov/srapub/SRR000001.sra WHILE IT SHOULD BE
http://sra-download.ncbi.nlm.nih.gov/srapub/SRR000001
*/ // KConfigPrint(config,0);
            REQUIRE_RC ( VResolverQuery
                ( resolver, eProtocolHttp, queryShort, NULL, & remote, NULL ) );
            compare ( remote, expectedShort );
            RELEASE ( VPath, remote );
        } else {
            REQUIRE_RC_FAIL ( VResolverQuery
                ( resolver, eProtocolHttp, queryShort, NULL, & remote, NULL ) );
        }

        if ( expectedLong . size () ) {
            REQUIRE_RC ( VResolverQuery
                ( resolver, eProtocolHttp, queryLong, NULL, & remote, NULL ) );
            compare ( remote, expectedLong );
            RELEASE ( VPath, remote );
        } else {
            REQUIRE_RC_FAIL ( VResolverQuery
                ( resolver, eProtocolHttp, queryLong, NULL, & remote, NULL ) );
        }


        RELEASE ( VResolver, resolver );

        REQUIRE_EQ ( rc, 0u );

    }

    ~Test(void) {
        assert(_dad);
        _dad->ErrorCounterAdd(GetErrorCounter());
    }
};

TEST_CASE(T) {
    const string newShort
        ("http://sra-download.ncbi.nlm.nih.gov/srapub/SRR000001");
    const string newLong
        ("http://sra-download.ncbi.nlm.nih.gov/srapub/SRR1000254");
    const string oldShort("http://ftp-trace.ncbi.nlm.nih.gov/sra/sra-instant/"
                "reads/ByRun/sra/SRR/SRR000/SRR000001/SRR000001.sra");

    rc_t rc = 0;

    KDirectory * native = NULL;
    REQUIRE_RC ( KDirectoryNativeDir ( & native ) );

    const KDirectory * dirc = NULL;
    KConfig * cfg = NULL;
    VFSManager * mgr = NULL;

    REQUIRE_RC(KDirectoryOpenDirRead(native, &dirc, false, "flat-sra-kfg/new"));
    REQUIRE_RC ( KConfigMake ( & cfg, dirc ) ); // KConfigPrint(cfg, 0);
    REQUIRE_RC ( VFSManagerMakeFromKfg ( & mgr, cfg ) );

    VPath * queryShort = NULL;
    REQUIRE_RC ( VFSManagerMakeAccPath ( mgr, & queryShort, "SRR000001" ) );

    VPath * queryLong = NULL;
    REQUIRE_RC ( VFSManagerMakeAccPath ( mgr, & queryLong, "SRR1000254" ) );

    const char rootPath [] = "/repository/remote/aux/NCBI/root";
    const char newRoot[]  = "http://sra-download.ncbi.nlm.nih.gov";

    // resolve using new aux configuration
    REQUIRE_RC ( KConfigWriteString ( cfg, rootPath, newRoot ) );

// TO FIX : THE FOLLOWING TEST FAILS
    Test(this, mgr, cfg, queryShort, queryLong, "to fix aux-new", newShort);

    // LOAD OLD CONFIGURATION
    RELEASE ( VFSManager, mgr );
    RELEASE ( KConfig, cfg );
    RELEASE ( KDirectory, dirc );

    REQUIRE_RC(KDirectoryOpenDirRead(native, &dirc, false, "flat-sra-kfg/old"));
    REQUIRE_RC ( KConfigMake ( & cfg, dirc ) ); // KConfigPrint(cfg, 0);
    REQUIRE_RC ( VFSManagerMakeFromKfg ( & mgr, cfg ) );

    // fail using incomplete configuration
    Test(this, mgr, cfg, queryShort, queryLong, "incomplete-old");

    // resolve using correct cgi
    const char cgiPath[] = "/repository/remote/main/CGI/resolver-cgi";
    const char goodCgi[] = "http://www.ncbi.nlm.nih.gov/Traces/names/names.cgi";
    const char badCgi [] = "http://XXX.ncbi.nlm.nih.gov/Traces/names/names.cgi";
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, goodCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "cgi-old", newShort, newLong);

    // fail using incorrect cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, badCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "bad cgi-old");

    // old aux configuration cannot resolve long accession
    const char oldRoot  []  = "http://ftp-trace.ncbi.nlm.nih.gov/sra";
    REQUIRE_RC ( KConfigWriteString ( cfg, rootPath, oldRoot ) );
    Test(this, mgr, cfg, queryShort, queryLong, "aux-old", oldShort);

    // LOAD NEW CONFIGURATION
    RELEASE ( VFSManager, mgr );
    RELEASE ( KConfig, cfg );
    RELEASE ( KDirectory, dirc );

    REQUIRE_RC(KDirectoryOpenDirRead(native, &dirc, false, "flat-sra-kfg/new"));
    REQUIRE_RC ( KConfigMake ( & cfg, dirc ) ); // KConfigPrint(cfg, 0);
    REQUIRE_RC ( VFSManagerMakeFromKfg ( & mgr, cfg ) );

    // fail using incomplete configuration
    Test(this, mgr, cfg, queryShort, queryLong, "incomplete-new");

    // resolve using correct cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, goodCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "cgi-new", newShort, newLong);

    // fail using incorrect cgi
    REQUIRE_RC ( KConfigWriteString ( cfg, cgiPath, badCgi ) );
    Test(this, mgr, cfg, queryShort, queryLong, "bad cgi-new");

    // resolve using new aux configuration
    REQUIRE_RC ( KConfigWriteString ( cfg, rootPath, newRoot ) );
    Test(this, mgr, cfg, queryShort, queryLong, "aux-new", newShort, newLong);

    RELEASE ( VFSManager, mgr );
    RELEASE ( KConfig, cfg );
    RELEASE ( KDirectory, dirc );

    RELEASE ( VPath, queryLong );

    RELEASE ( VPath, queryShort );

    RELEASE ( KDirectory, native );

    REQUIRE_EQ ( rc, 0u );
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }
    rc_t CC KMain ( int argc, char *argv [] ) {
        KConfigDisableUserSettings();
        return flatSraKfgTestSuite(argc, argv);
    }
}