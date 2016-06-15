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


FIXTURE_TEST_CASE( SetCacheRoot_1, VdbFixture )
{
    rc = VDBManagerSetCacheRoot( vdb_mgr, NULL );
    if ( rc == 0 )
        FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, NULL ) succeed" );

    VPath * vpath;
    rc = VFSManagerMakePath ( vfs_mgr, &vpath, "/home/raetzw/somepath" );
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
    
    std::string s = std::string( spath->addr, spath->size );
    std::cout << "after setting different value: " << s << std::endl;
    
    if ( spath != NULL )
        StringWhack( spath );

    if ( vpath != NULL )
        VPathRelease( vpath );
}


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
    std::cout << "reverted to original value of: " << s << std::endl;
    
    if ( s != original_value )
        FAIL( "FAIL: did not restore original value" );
        
    if ( spath != NULL )
        StringWhack( spath );

    if ( vpath != NULL )
        VPathRelease( vpath );
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
    rc_t rc = VDB_3060( argc, argv );
    return rc;
}

}
