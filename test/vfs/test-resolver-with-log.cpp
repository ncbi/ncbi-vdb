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

/* tests of names service */

#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <klib/debug.h> /* KDbgSetString */
#include <ktst/unit_test.hpp> /* KMain */
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path.h> /* VPathRelease */
#include <vfs/resolver.h> /* VResolverRelease */

#include "resolver-cgi.h" /* RESOLVER_CGI */

#include "../../../ncbi-vdb/libs/vfs/resolver-priv.h" /* VResolverSetVersion */

TEST_SUITE ( VResolverWithLogTestSuite );

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

using std::string;

static KConfig * KFG = NULL;

class Fixture {
protected:
    VFSManager * _mgr;
    VResolver * _resolver;

    VPath * _query;
    const VPath * _remote;

public:

    Fixture ()
        : _mgr ( NULL ), _resolver ( NULL ), _query ( NULL ), _remote ( NULL )
    {
        rc_t rc = VFSManagerMakeFromKfg ( & _mgr, KFG );
        if ( rc != 0 )
            throw "VFSManagerMake";

        rc = VFSManagerGetResolver ( _mgr, & _resolver );
        if ( rc != 0 )
            throw "VFSManagerGetResolver";
    }

    ~Fixture () {
        rc_t rc = 0;

        RELEASE ( VResolver, _resolver );
        RELEASE ( VFSManager, _mgr );

        RELEASE ( VPath, _query );
        RELEASE ( VPath, _remote );

        std::cerr << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
    }
};

FIXTURE_TEST_CASE ( AFVF01, Fixture ) {
    REQUIRE_RC ( VFSManagerMakePath ( _mgr, & _query, "AFVF01.1" ) );
    REQUIRE_RC ( VResolverQuery
        ( _resolver, eProtocolHttpHttps, _query, NULL, & _remote, NULL ) );
}

FIXTURE_TEST_CASE ( AAAB01, Fixture ) {
    REQUIRE_RC ( VFSManagerMakePath ( _mgr, & _query, "AAAB01" ) );
    REQUIRE_RC ( VResolverQuery
        ( _resolver, eProtocolHttps, _query, NULL, & _remote, NULL ) );
}

FIXTURE_TEST_CASE ( AAAB01008846, Fixture ) {
    REQUIRE_RC ( VFSManagerMakePath ( _mgr, & _query, "AAAB01008846.1" ) );
    REQUIRE_RC ( VResolverQuery
        ( _resolver, eProtocolHttpHttps, _query, NULL, & _remote, NULL ) );
}

FIXTURE_TEST_CASE ( SRR1008846, Fixture ) {
    REQUIRE_RC ( VResolverSetVersion ( _resolver, "1.2" ));

    REQUIRE_RC ( VFSManagerMakePath ( _mgr, & _query, "SRR1008846" ) );
    REQUIRE_RC ( VResolverQuery
        ( _resolver, eProtocolFaspHttps, _query, NULL, & _remote, NULL ) );

    char buffer [ 9 ];
    REQUIRE_RC ( VPathReadScheme ( _remote, buffer, sizeof buffer, NULL ) );
    REQUIRE_EQ ( string ( buffer ), string ( "fasp" ) );
}

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) {
        KDbgSetString ( "VFS" );

        KConfigDisableUserSettings ();

        rc_t rc = KConfigMake ( & KFG, NULL );
        if ( rc == 0 )
            rc = KConfigWriteString ( KFG,
                "repository/remote/main/CGI/resolver-cgi", RESOLVER_CGI );

        std::cerr << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n"; 

        if ( rc == 0 )
            rc = VResolverWithLogTestSuite ( argc, argv );

        RELEASE ( KConfig, KFG );

        return rc;
    }
}
