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

/**
* Unit tests for VResolver interface
*/

#include <kapp/args.h> /* ArgsMakeAndHandle */

#include <klib/debug.h> /* KDbgSetString */
#include <klib/text.h>

#include <ktst/unit_test.hpp>

#include <kfg/config.h>
#include <kfg/kfg-priv.h>
#include <kfs/file.h>
#include <kfs/directory.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/resolver.h>
#include <vfs/path.h>

#include "../../libs/vfs/resolver-cgi.h" /* RESOLVER_CGI */

#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <climits> /* PATH_MAX */

#include <sysalloc.h>

#define RELEASE(type, obj) do { rc_t rc2 = type##Release(obj); \
    if (rc2 != 0 && rc == 0) { rc = rc2; } obj = NULL; } while (false)

static rc_t argsHandler(int argc, char* argv[]) {
    return ArgsMakeAndHandle ( NULL, argc, argv, 0, NULL, 0 );
}

TEST_SUITE_WITH_ARGS_HANDLER(VResolverTestSuite, argsHandler);

using namespace std;

// this is NCBI-specific, move to a private repo
//#ifdef MAC
//#define NETMNT "/net"
//#else
//#define NETMNT "/netmnt"
//#endif
//const string Netmnt(NETMNT);
//
//static bool hasLocal = true;

static string ToString(const VPath* path)
{
    const String * s;
    if ( VPathMakeString (path, &s) != 0 )
        throw logic_error ( "ToString(VPath) failed" );
    string ret = string(s->addr, s->size);
    free((void*)s);
    return ret;
}

class ResolverFixture
{
public:
    ResolverFixture()
        : vfs ( 0 )
        , resolver ( 0 )
        , query ( 0 )
        , local ( 0 )
        , remote ( 0 )
        , cache ( 0 )
    {
        rc_t rc = VFSManagerMake ( & vfs );
        if ( rc != 0 )
            throw logic_error ( "ResolverFixture: VFSManagerMake failed" );
        rc = VFSManagerGetResolver ( vfs, & resolver );
        if ( rc != 0 )
            throw logic_error ( "ResolverFixture: VFSManagerGetResolver failed" );
    }

    ~ResolverFixture()
    {
        VResolverRelease ( resolver );
        VFSManagerRelease ( vfs );
    }
    
    VFSManager * vfs;
    VResolver * resolver;
    VPath * query;
    const VPath * local;
    const VPath * remote;
    const VPath * cache;
};

FIXTURE_TEST_CASE ( VDB_2936_resolve_local_WGS_without_version, ResolverFixture )
{
    String acc;
    CONST_STRING ( & acc, "JBBO01" );

    REQUIRE_RC ( VFSManagerMakePath ( vfs, & query, "%S", & acc ) );
    try
    {
        rc_t rc2 = VResolverQuery ( resolver, eProtocolHttp, query, & local, 0, 0 );
        if ( rc2 == 0 )
        {
            try
            {
                String path;
                REQUIRE_RC ( VPathGetPath ( local, & path ) );
                if ( path . size > acc . size )
                    REQUIRE_EQ ( path . addr [ path . size - acc . size - 1 ], '/' );
                else
                    REQUIRE_EQ ( path . size, acc . size );

                String sub;
                StringSubstr ( & path, & sub, path . len - acc . len, 0 );
                REQUIRE_EQ ( StringCompare ( & acc, & sub ), ( int ) 0 );
            }
            catch ( ... )
            {
                VPathRelease ( local ); local = 0;
                throw;
            }

            VPathRelease ( local ); local = 0;
        }
    }
    catch ( ... )
    {
        VPathRelease ( query ); query = 0;
        throw;
    }

    VPathRelease ( query ); query = 0;
}

FIXTURE_TEST_CASE ( VDB_2936_resolve_local_WGS_with_version, ResolverFixture )
{
    String acc;
    CONST_STRING ( & acc, "JBBO01.1" );

    REQUIRE_RC ( VFSManagerMakePath ( vfs, & query, "%S", & acc ) );
    try
    {
        rc_t rc2 = VResolverQuery ( resolver, eProtocolHttp, query, & local, 0, 0 );
        if ( rc2 == 0 )
        {
            try
            {
                String path;
                REQUIRE_RC ( VPathGetPath ( local, & path ) );
                if ( path . size > acc . size )
                    REQUIRE_EQ ( path . addr [ path . size - acc . size - 1 ], '/' );
                else
                    REQUIRE_EQ ( path . size, acc . size );

                String sub;
                StringSubstr ( & path, & sub, path . len - acc . len, 0 );
                REQUIRE_EQ ( StringCompare ( & acc, & sub ), ( int ) 0 );
            }
            catch ( ... )
            {
                VPathRelease ( local ); local = 0;
                throw;
            }

            VPathRelease ( local ); local = 0;
        }
    }
    catch ( ... )
    {
        VPathRelease ( query ); query = 0;
        throw;
    }

    VPathRelease ( query ); query = 0;
}

FIXTURE_TEST_CASE(WGS_with_6letter_prefix, ResolverFixture)
{
    REQUIRE_RC(VFSManagerMakePath(vfs, &query, "AAAAAA01"));
    REQUIRE_RC(VResolverQuery(resolver, 0, query, &local, &remote, 0));
    REQUIRE(local || remote);
    VPathRelease(query); query = 0;
    VPathRelease(local); local = 0;
    VPathRelease(remote); remote = 0;
}

FIXTURE_TEST_CASE(WGS_with_6letter_prefix_and_version, ResolverFixture)
{
    REQUIRE_RC(VFSManagerMakePath(vfs, &query, "AAAAAA01.1"));
    REQUIRE_RC(VResolverQuery(resolver, 0, query, &local, &remote, 0));
    REQUIRE(local || remote);
    VPathRelease(query); query = 0;
    VPathRelease(local); local = 0;
    VPathRelease(remote); remote = 0;
}

// this is NCBI-specific, move to a private repo
//FIXTURE_TEST_CASE(HS37D5, ResolverFixture) {
//    REQUIRE_RC(VFSManagerMakePath(vfs, &query, "hs37d5"));
//
//    REQUIRE_RC(VResolverQuery(resolver, 0, query, NULL, &remote, NULL));
//    REQUIRE_RC(VPathRelease(remote)); remote = NULL;
//
//    if (hasLocal) {
//        REQUIRE_RC(VResolverQuery(resolver, 0, query, &local, NULL, NULL));
//        REQUIRE_RC(VPathRelease(local)); local = NULL;
//    }
//    else
//        REQUIRE_RC_FAIL(VResolverQuery(resolver, 0, query, &local, NULL, NULL));
//
//    REQUIRE_RC(VPathRelease(query)); query = NULL;
//}

class ResolverFixtureCustomConfig
{
public:
    ResolverFixtureCustomConfig()
        : vfs ( 0 )
        , resolver ( 0 )
    {
    }

    ~ResolverFixtureCustomConfig()
    {
        rc_t rc = VFSManagerRelease(vfs);
        if ( rc != 0 )
            fprintf ( stderr, "~ResolverFixtureCustomConfig: VFSManagerRelease()=%u\n", rc );
        vfs = NULL;

        rc = VResolverRelease(resolver);
        if ( rc != 0 )
            fprintf ( stderr, "~ResolverFixtureCustomConfig: VResolverRelease()=%u\n", rc );
        resolver = NULL;

        remove(m_configName.c_str());
    }

    void Configure(const string& name, const string& config_content)
    {
        m_configName = name+".kfg";
        std::ofstream f(m_configName.c_str());
        f   << config_content;
        f.close();

        KDirectory *wd;
        if (KDirectoryNativeDir(&wd))
            throw logic_error ( "ResolverFixtureCustomConfig::Configure: KDirectoryNativeDir failed" );

        KConfig *cfg;
        if (KConfigMakeLocal(&cfg, wd))
            throw logic_error ( "ResolverFixtureCustomConfig::Configure: KConfigMakeLocal failed" );

        if (KConfigWriteString ( cfg,
                "repository/remote/main/CGI/resolver-cgi", RESOLVER_CGI))
        {
            throw logic_error ( "ResolverFixtureCustomConfig::Configure:"
                " KConfigWriteString failed" );
        }

        if (VFSManagerMakeFromKfg(&vfs, cfg))
            throw logic_error ( "ResolverFixtureCustomConfig::Configure: VFSManagerMakeFromKfg failed" );

        if (VFSManagerGetResolver ( vfs, & resolver ))
            throw logic_error ( "ResolverFixtureCustomConfig::Configure: VFSManagerGetResolver failed" );

        if (KConfigRelease(cfg))
            throw logic_error ( "ResolverFixtureCustomConfig::Configure: KConfigRelease failed" );

        if (KDirectoryRelease(wd))
            throw logic_error ( "ResolverFixtureCustomConfig::Configure: KDirectoryRelease failed" );
    }

    string Resolve(const string& p_accession)
    {
        VPath * query;
        const VPath * local;
        string result;
        if ( VFSManagerMakePath ( vfs, & query, "%s", p_accession.c_str() ) )
            throw logic_error ( "ResolverFixtureCustomConfig::Resolve: VFSManagerMakePath failed" );
        try
        {
            if ( VResolverQuery ( resolver, eProtocolHttp, query, & local, 0, 0 ) )
                throw logic_error ( "ResolverFixtureCustomConfig::Resolve: VResolverQuery failed" );
            try
            {
                result = ToString(local);
            }
            catch ( ... )
            {
                VPathRelease ( local ); local = 0;
                throw;
            }

            VPathRelease ( local ); local = 0;
        }
        catch ( ... )
        {
            VPathRelease ( query ); query = 0;
            throw;
        }

        VPathRelease ( query ); query = 0;
        return result;
    }

    VFSManager * vfs;
    VResolver * resolver;

    string m_configName;
};

FIXTURE_TEST_CASE(VDB_2963_resolve_local_new_wgs, ResolverFixtureCustomConfig)
{
    string config =
            "repository/user/main/public/root=\"./repo\"\n"
            "repository/user/main/public/apps/wgs/volumes/wgs=\"wgs\"\n"
            "repository/user/main/public/apps/wgs/volumes/wgs2=\"wgs2\"\n"
            ;
    Configure(GetName(), config);

    REQUIRE_EQ ( Resolve ( "YYZZ99" ), string("./repo/wgs2/WGS/YY/ZZ/YYZZ99") );
    REQUIRE_EQ ( Resolve ( "YYZZ20" ), string("./repo/wgs2/WGS/YY/ZZ/YYZZ20") );
    REQUIRE_EQ ( Resolve ( "YYZZ10" ), string("./repo/wgs/WGS/YY/ZZ/YYZZ10") );
}

FIXTURE_TEST_CASE(VDB_2963_resolve_local_no_new_wgs, ResolverFixtureCustomConfig)
{
    string config =
            "repository/user/main/public/root=\"./repo\"\n"
            "repository/user/main/public/apps/wgs/volumes/wgs=\"wgs\"\n"
            ;
    Configure(GetName(), config);

    REQUIRE_EQ ( Resolve ( "YYZZ99" ), string("./repo/wgs/WGS/YY/ZZ/YYZZ99") );
    REQUIRE_EQ ( Resolve ( "YYZZ10" ), string("./repo/wgs/WGS/YY/ZZ/YYZZ10") );

    try {
        Resolve ( "YYZZ20" );
        REQUIRE ( false ); // should throw earlier
    } catch (...) { }
}

//////////////////////////////////////////// Main

extern "C"
{

#include <kapp/args.h>
#include <klib/rc.h>

    ver_t CC KAppVersion ( void )
    {
        return 0x1000000;
    }

    rc_t CC UsageSummary (const char * progname)
    {
        return 0;
    }

    rc_t CC Usage ( const Args * args )
    {
        return 0;
    }

    const char UsageDefaultName[] = "test-resolver";

    static void clear_recorded_errors ( void )
    {
        rc_t rc;
        const char * filename;
        const char * funcname;
        uint32_t line_nr;
        while ( GetUnreadRCInfo ( &rc, &filename, &funcname, &line_nr ) )
        {
        }
    }

    rc_t CC KMain ( int argc, char *argv [] )
    {
        if (
1) assert(!KDbgSetString("VFS"));

        KConfigDisableUserSettings ();
		rc_t rc;

		// this is NCBI-specific, move to a private repo
        //KDirectory * dir = NULL;
        //rc_t rc = KDirectoryNativeDir(&dir);
        //if (rc != 0)
        //    return rc;
        //uint32_t t = KDirectoryPathType(dir, NETMNT "/traces04") & ~ kptAlias;
        //if (t == kptNotFound || t == kptBadPath)
        //{
        //    hasLocal = false;
        //}
        //rc = KDirectoryRelease(dir);
        //if (rc != 0)
        //    return rc;

        rc = VResolverTestSuite ( argc, argv );

        clear_recorded_errors();

        return rc;
    }

}
