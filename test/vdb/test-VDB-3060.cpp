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

TEST_SUITE( VDB_3060 )

std::string original_value = std::string( "" );

class VdbFixture
{
    public:
        VdbFixture() : vdb_mgr( NULL ), vfs_mgr( NULL )
        {
            if ( VDBManagerMakeRead( &vdb_mgr, NULL ) != 0 )
                throw logic_error ( "VDB-3060.VdbFixture: VDBManagerMakeRead() failed" );
            if ( VFSManagerMake ( &vfs_mgr ) != 0 )
                throw logic_error ( "VdbFixture: VFSManagerMake() failed" );
        }
    
        ~VdbFixture()
        {
            if ( vdb_mgr && VDBManagerRelease ( vdb_mgr ) != 0 )
                throw logic_error ( "~VDB-3060.VdbFixture: VDBManagerRelease() failed" );
            if ( vfs_mgr && VFSManagerRelease ( vfs_mgr ) != 0 )
                throw logic_error ( "~VDB-3060.VdbFixture: VFSManagerRelease() failed" );
        }
    
    const VDBManager * vdb_mgr;
    VFSManager * vfs_mgr;
    rc_t rc;
};


/*
    test VDBManagerGetCacheRoot() with invalid and valid parameters
    print the currently stored value
    store this value in the global: original_value
*/
FIXTURE_TEST_CASE( GetCacheRoot_1, VdbFixture )
{
    VPath const * vpath = NULL;
    rc = VDBManagerGetCacheRoot( NULL, &vpath );
    if ( rc == 0 )
        FAIL( "FAIL: VDBManagerGetCacheRoot( NULL, &vpath ) succeed" );
    rc = VDBManagerGetCacheRoot( vdb_mgr, NULL );
    if ( rc == 0 )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, NULL ) succeed" );
    rc = VDBManagerGetCacheRoot( vdb_mgr, &vpath );
    if ( rc != 0 )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, &vpath ) failed" );
    if ( vpath == NULL )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, &vpath ) returned vpath == NULL " );

    String const * spath = NULL;
    rc = VPathMakeString ( vpath, &spath );
    if ( rc != 0 )
        FAIL( "FAIL: VPathMakeString( vpath, &spatch ) failed" );
    
    original_value = std::string( spath->addr, spath->size );
    std::cout << "original value: " << original_value << std::endl;
    
    if ( spath != NULL )
        StringWhack( spath );

    if ( vpath != NULL )
        VPathRelease( vpath );
}

const char other_path[] = "/some/other/path";

/*
    test VDBManagerSetCacheRoot() with invalid and valid parameters
    set the value to "/home/raetzw/somepath"
*/
FIXTURE_TEST_CASE( SetCacheRoot_1, VdbFixture )
{
    rc = VDBManagerSetCacheRoot( vdb_mgr, NULL );
    if ( rc == 0 )
        FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, NULL ) succeed" );

    VPath * vpath;
    rc = VFSManagerMakePath ( vfs_mgr, &vpath, other_path );
    if ( rc != 0 )
        FAIL( "FAIL: VFSManagerMakePath() failed" );
        
    rc = VDBManagerSetCacheRoot( NULL, vpath );
    if ( rc == 0 )
        FAIL( "FAIL: VDBManagerSetCacheRoot( NULL, vpath ) succeed" );
        
    rc = VDBManagerSetCacheRoot( vdb_mgr, vpath );
    if ( rc != 0 )
        FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, vpath ) failed" );

    if ( vpath != NULL )
        VPathRelease( vpath );
}


/*
    call VDBManagerGetCacheRoot() to verify that the new value
    is indeed the value we did set in the test-case above
*/
FIXTURE_TEST_CASE( GetCacheRoot_2, VdbFixture )
{
    VPath const * vpath = NULL;
    rc = VDBManagerGetCacheRoot( vdb_mgr, &vpath );
    if ( rc != 0 )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, &vpath ) failed" );
    if ( vpath == NULL )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, &vpath ) returned vpath == NULL " );

    String const * spath = NULL;
    rc = VPathMakeString ( vpath, &spath );
    if ( rc != 0 )
        FAIL( "FAIL: VPathMakeString( vpath, &spatch ) failed" );
    
    std::string s1 = std::string( spath->addr, spath->size );
    std::string s2 = std::string( other_path );
    std::cout << "after setting different value: " << s1;
    if ( s1 == s2 )
        std::cout << " - as expected" << std::endl;
    else
    {
        std::cout << " - we did not expected this!" << std::endl;
        FAIL( "FAIL: unexpected value after setting a new cache-root" );
    }
    
    if ( spath != NULL )
        StringWhack( spath );

    if ( vpath != NULL )
        VPathRelease( vpath );
}


/*
    put the value stored in the global 'original_value' back in place
*/
FIXTURE_TEST_CASE( SetCacheRoot_2, VdbFixture )
{
    VPath * vpath;
    rc = VFSManagerMakePath ( vfs_mgr, &vpath, original_value.c_str() );
    if ( rc != 0 )
        FAIL( "FAIL: VFSManagerMakePath() failed" );
        
    rc = VDBManagerSetCacheRoot( vdb_mgr, vpath );
    if ( rc != 0 )
        FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, vpath ) failed" );

    if ( vpath != NULL )
        VPathRelease( vpath );
}


/*
    check if the original value is back in place
*/
FIXTURE_TEST_CASE( GetCacheRoot_3, VdbFixture )
{
    VPath const * vpath = NULL;
    rc = VDBManagerGetCacheRoot( vdb_mgr, &vpath );
    if ( rc != 0 )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, &vpath ) failed" );
    if ( vpath == NULL )
        FAIL( "FAIL: VDBManagerGetCacheRoot( mgr, &vpath ) returned vpath == NULL " );

    String const * spath = NULL;
    rc = VPathMakeString ( vpath, &spath );
    if ( rc != 0 )
        FAIL( "FAIL: VPathMakeString( vpath, &spatch ) failed" );
    
    std::string s = std::string( spath->addr, spath->size );
    std::cout << "reverted to original value of: " << s;

    if ( s != original_value )
        FAIL( "FAIL: did not restore original value" );
        
    if ( spath != NULL )
        StringWhack( spath );

    if ( vpath != NULL )
        VPathRelease( vpath );
}

char * org_home;
const char HomeSub[] = "test_root_history";
char new_home[ 1024 ];
char new_home_buffer[ 1024 ]; /* buffer for putenv has to stay alive! */


rc_t write_root( KConfig *cfg, const char * base, const char * cat, const char * sub_cat )
{
    char key[ 256 ];
    size_t num_writ;
    rc_t rc = string_printf ( key, sizeof key, &num_writ, "/repository/user/%s/%s/root", cat, sub_cat );
    if ( rc == 0 )
    {
        char value[ 256 ];
        rc = string_printf ( value, sizeof value, &num_writ, "%s/ncbi/%s", base, sub_cat );
        if ( rc == 0 )
            rc = KConfigWriteString( cfg, key, value );
    }
    return rc;
}


rc_t write_dflt_path( KConfig *cfg, const char * base )
{
    char value[ 256 ];
    size_t num_writ;
    rc_t rc = string_printf ( value, sizeof value, &num_writ, "%s/ncbi", base );
    if ( rc == 0 )
        rc = KConfigWriteString( cfg, "/repository/user/default-path", value );
    return rc;
}

rc_t create_test_config( const char * base )
{
    KConfig *cfg;
    rc_t rc = KConfigMake ( &cfg, NULL );
    if ( rc == 0 )
    {
        rc = write_root( cfg, base, "main", "public" );
        if ( rc == 0 )
            rc = write_root( cfg, base, "protected", "dbGaP-2956" );
        if ( rc == 0 )
            rc = write_root( cfg, base, "protected", "dbGaP-4831" );
        if ( rc == 0 )
            rc = write_dflt_path( cfg, base );
        if ( rc == 0 )
            rc = KConfigCommit ( cfg );
        KConfigRelease ( cfg );
    }
    return rc;
}

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
        rc = create_test_config( org_home );
    return rc;
}

void finish_test()
{
    /* clear the temp. home-directory */
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir( &dir );
    if ( rc == 0 )
    {
        //rc = KDirectoryRemove( dir, true, "%s/%s", org_home, HomeSub );
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
const char UsageDefaultName[] = "test-VDB-3060";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc = prepare_test( HomeSub );
    if ( rc == 0 )
        rc = VDB_3060( argc, argv );
    finish_test();
    return rc;
}

}
