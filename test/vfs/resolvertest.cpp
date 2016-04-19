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


#include <klib/text.h>

#include <ktst/unit_test.hpp>

#include <vfs/manager.h>
#include <vfs/resolver.h>
#include <vfs/path.h>

#include <stdexcept>

#include <cstdlib>

#include <climits> /* PATH_MAX */

#include <sysalloc.h>


TEST_SUITE(VResolverTestSuite);

using namespace std;

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
        rc_t rc = VResolverTestSuite ( argc, argv );
        clear_recorded_errors();
        return rc;
    }

}
