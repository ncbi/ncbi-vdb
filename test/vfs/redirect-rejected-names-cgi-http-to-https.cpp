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

#include <kfg/config.h> /* KConfig */
#include <kfs/directory.h> /* KDirectory */
#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp>
#include <vfs/manager.h> /* VFSManager */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path.h> /* VPath */
#include <vfs/resolver.h> /* VResolver */

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

extern "C" { void TESTING_VDB_3162 ( void ); }

TEST_SUITE(VResolverTestSuite);

typedef enum {
    e200,
    e403,
} EForbidden;

class Test : protected ncbi :: NK :: TestCase {
    TestCase * _dad;
public:
    Test ( const std :: string & description, TestCase * dad,
            EForbidden forbidden, const char * name, const char * url,
            bool fail = false )
        : TestCase ( name ), _dad ( dad )
    {
        rc_t rc = 0;

        DBGMSG ( DBG_VFS, DBG_FLAG ( DBG_VFS ), ( "TESTING %s\n", name ) );

        KDirectory * native  = NULL;
        REQUIRE_RC ( KDirectoryNativeDir ( & native ) );

        const KDirectory * dir = NULL;
        REQUIRE_RC ( KDirectoryOpenDirRead ( native,
            &dir, false, "redirect-rejected-names-cgi-http-to-https" ) );

        KConfig * cfg = NULL;
        REQUIRE_RC ( KConfigMake ( & cfg, dir ) );
        REQUIRE_RC ( KConfigWriteString
            ( cfg, "repository/remote/main/CGI/resolver-cgi", url ) );

        VFSManager * vfs = NULL;
        REQUIRE_RC ( VFSManagerMakeFromKfg ( & vfs, cfg ) );

        VResolver * resolver = NULL;
        REQUIRE_RC ( VFSManagerGetResolver ( vfs, & resolver ) );

        VPath * query = NULL;
        REQUIRE_RC ( VFSManagerMakePath ( vfs, & query, "SRR000001" ) );

        const VPath * remote = NULL;

        if ( forbidden == e403 ) {
            TESTING_VDB_3162 ();
        }

        if ( fail )
            REQUIRE_RC_FAIL
                ( VResolverQuery ( resolver, 0, query, 0, & remote, 0 ) );
        else
            REQUIRE_RC
                ( VResolverQuery ( resolver, 0, query, 0, & remote, 0 ) );
        RELEASE ( VPath, query );

        RELEASE ( VPath, remote );

        RELEASE ( VResolver, resolver );

        RELEASE ( VFSManager, vfs );

        RELEASE ( KConfig, cfg );

        RELEASE ( KDirectory, dir );

        RELEASE ( KDirectory, native );

        REQUIRE ( ! rc );
    }
    ~Test ( void ) {
        assert( _dad );
        _dad -> ErrorCounterAdd ( GetErrorCounter () );
    }
};

TEST_CASE(TEST) {
    Test ( "Called over HTTP: retry over HTTPS after 403", this, e403 , "HTTP",
        "http://www.ncbi.nlm.nih.gov/Traces/names/names.cgi" );
    Test ( "Called over HTTPS: got 200", this, e200, "HTTPS",
       "https://www.ncbi.nlm.nih.gov/Traces/names/names.cgi" );
    Test (
        "Called over HTTP: fail after 403 - not retrying non-government sites",
        this, e403, "HTTPS, not government",
        "http://www/Traces/names/names.cgi", true );
    Test ( "Called over HTTPS: fail after 403", this, e403, "403 by HTTPS",
       "https://www.ncbi.nlm.nih.gov/Traces/names/names.cgi", true );
}

extern "C" {
    ver_t CC KAppVersion ( void                     ) { return 0; }
    rc_t CC UsageSummary ( const char     * progname) { return 0; }
    rc_t CC Usage        ( const struct Args * args ) { return 0; }
    const char UsageDefaultName[] = "redirect-rejected-names-cgi-http-to-https";
    rc_t CC KMain ( int argc, char *argv [] ) {
        assert ( ! KDbgSetString ( "VFS" ) );
        return VResolverTestSuite ( argc, argv );
    }
}
