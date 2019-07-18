// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#include <vdb/manager.h> // VDBManager
#include <kdb/manager.h> // KDBManager
#include <kdb/kdb-priv.h>
#include <vdb/vdb-priv.h>

#include <ktst/unit_test.hpp> // TEST_CASE
#include <vfs/path.h>
#include <vfs/manager.h>
#include <klib/text.h> 
#include <klib/out.h> 
#include <klib/printf.h> 
#include <kfs/directory.h> 
#include <kfg/config.h> 

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>

using namespace std;

TEST_SUITE( VDB_3305 )

const VDBManager * vdb_mgr = NULL;
VFSManager * vfs_mgr = NULL;

static rc_t make_global_managers( void )
{
    rc_t rc = VDBManagerMakeRead( &vdb_mgr, NULL );
    if ( rc != 0 )
        std::cout << "VDB-3060.VdbFixture: VDBManagerMakeRead() failed" << std::endl;
    else
    {
        rc = VFSManagerMake ( &vfs_mgr );
        if ( rc != 0 )
            std::cout << "VdbFixture: VFSManagerMake() failed" << std::endl;
    }
    return rc;
}

static void release_global_managers( void )
{
    VFSManagerRelease ( vfs_mgr );
    VDBManagerRelease ( vdb_mgr );
}

const char other_path[] = "/some/other/path";

/*
    test VDBManagerSetCacheRoot() with invalid and valid parameters
    set the value to "/home/user/somepath"
*/
TEST_CASE( SetCacheRoot_1 )
{
    VPath * vpath;
    rc_t rc = VFSManagerMakePath( vfs_mgr, &vpath, other_path );
    if ( rc != 0 )
        FAIL( "FAIL: VFSManagerMakePath() failed" );
        
    rc = VDBManagerSetCacheRoot( vdb_mgr, vpath );
    if ( rc != 0 )
        FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, vpath ) failed" );

    if ( vpath != NULL )
        VPathRelease( vpath );
}


char * org_home;
const char HomeSub[] = "test_root_history";
char new_home[ 1024 ];
char new_home_buffer[ 1024 ]; /* buffer for putenv has to stay alive! */

rc_t prepare_test( const char * sub )
{
    org_home = getenv( "HOME" );
    size_t num_writ;
    rc_t rc = string_printf ( new_home, sizeof new_home, &num_writ, "%s/%s", org_home, sub );
    if ( rc == 0 )
        rc = string_printf ( new_home_buffer, sizeof new_home_buffer, &num_writ, "HOME=%s", new_home );
    if ( rc == 0 )
        rc = putenv( new_home_buffer );
    if ( rc == 0 )
    {
        /* create a '.ncbi' directory below */
        KDirectory * dir;
        rc = KDirectoryNativeDir( &dir );
        if ( rc == 0 )
        {
            KOutMsg( "%s\n", new_home );
            
            rc = KDirectoryCreateDir( dir, 0775, ( kcmInit | kcmParents ) , "%s/.ncbi", new_home );
            KDirectoryRelease( dir );
        }
    }
    return rc;
}

void finish_test( void )
{
    /* clear the temp. home-directory */
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir( &dir );
    if ( rc == 0 )
    {
        rc = KDirectoryRemove( dir, true, "%s/%s", org_home, HomeSub );
        KDirectoryRelease( dir );
    }
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char * progname ) { return 0; }
rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName[] = "test-VDB-3305";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc = prepare_test( HomeSub );
    if ( rc == 0 )
    {
        rc = make_global_managers();
        if ( rc == 0 )
        {
            rc = VDB_3305( argc, argv );
            release_global_managers();
        }
    }
    //finish_test();
    return rc;
}

}
