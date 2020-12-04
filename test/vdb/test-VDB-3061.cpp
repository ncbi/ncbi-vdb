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
#include <klib/time.h> 
#include <kfs/directory.h>
#include <kfs/file.h> 
#include <kfg/config.h> 

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>

using namespace std;

TEST_SUITE( VDB_3061 )

const char HomeSub[] = "test_root_history";
char new_home[ 4096 ];

static rc_t create_cache_file( KDirectory * dir, const char * path, const char * sub, const char * name,
                               int64_t age_in_seconds )
{
    KFile * f;
    rc_t rc = KDirectoryCreateFile( dir, &f, false, 0775, kcmInit, "%s/ncbi/%s/%s", path, sub, name );
    if ( rc == 0 )
    {
        KFileRelease( f );
        
        KTime_t date = KTimeStamp() - age_in_seconds;
        rc = KDirectorySetDate ( dir, false, date, "%s/ncbi/%s/%s", path, sub, name );
    }
    return rc;
}

static rc_t create_repo_dirs( KDirectory * dir, const char * path, const char * sub,
                              uint32_t age_in_days, int32_t offset_in_seconds )
{
    rc_t rc = KDirectoryCreateDir( dir, 0775, kcmInit | kcmParents, "%s/ncbi/%s/files", path, sub );
    if ( rc == 0 )
        rc = KDirectoryCreateDir( dir, 0775, kcmInit | kcmParents, "%s/ncbi/%s/nannot", path, sub );
    if ( rc == 0 )
        rc = KDirectoryCreateDir( dir, 0775, kcmInit | kcmParents, "%s/ncbi/%s/refseq", path, sub );
    if ( rc == 0 )
        rc = KDirectoryCreateDir( dir, 0775, kcmInit | kcmParents, "%s/ncbi/%s/sra", path, sub );
    if ( rc == 0 )
        rc = KDirectoryCreateDir( dir, 0775, kcmInit | kcmParents, "%s/ncbi/%s/wgs", path, sub );
    if ( rc == 0 )
    {
        int64_t age_in_seconds = age_in_days;
        age_in_seconds *= ( 60 * 60 * 24 );
        age_in_seconds += offset_in_seconds;
        rc = create_cache_file( dir, path, sub, "/sra/file1.txt", age_in_seconds );
    }
    return rc;
}


static void clear_out( const char * path )
{
    /* clear the temp. home-directory */
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir( &dir );
    if ( rc == 0 )
    {
#ifdef WINDOWS
        rc = KDirectoryRemove( dir, true, "%s", path );
#else
        rc = KDirectoryRemove( dir, true, "%s", path );
#endif
        KDirectoryRelease( dir );
    }
}

TEST_CASE( CLEAR_CACHE_1 )
{
	REQUIRE_RC( KOutMsg( "running: CLEAR_CACHE_1\n" ) );
    
    /* create a repository-structure equivalent to the config-values, with 3 files in it */
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );
    REQUIRE_RC( create_repo_dirs( dir, new_home, "public", 10, 0 ) ); /* 10 days old */
    REQUIRE_RC( create_repo_dirs( dir, new_home, "dbGaP-2956", 4, 0 ) ); /* 4 days old */
    REQUIRE_RC( create_repo_dirs( dir, new_home, "dbGaP-4831", 5, -5 ) ); /* 5 days - 5 seconds old */
    REQUIRE_RC( KDirectoryRelease ( dir ) );
    
    /* we run the new function VDBManagerDeleteCacheOlderThan() with a 5-day threshold */
    const VDBManager * vdb_mgr;
    REQUIRE_RC( VDBManagerMakeRead( &vdb_mgr, NULL ) );
    REQUIRE_RC( VDBManagerDeleteCacheOlderThan ( vdb_mgr, 5 ) );
    REQUIRE_RC( VDBManagerRelease ( vdb_mgr ) );

    /* now the 10 day old one should have disappeared, the other 2 are still there */
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );
    uint32_t pt1 = KDirectoryPathType( dir, "%s/ncbi/public/sra/file1.txt", new_home );
    REQUIRE_EQ( pt1, (uint32_t)kptNotFound );
    uint32_t pt2 = KDirectoryPathType( dir, "%s/ncbi/dbGaP-2956/sra/file1.txt", new_home );
    REQUIRE_EQ( pt2, (uint32_t)kptFile );
    uint32_t pt3 = KDirectoryPathType( dir, "%s/ncbi/dbGaP-4831/sra/file1.txt", new_home );
    REQUIRE_EQ( pt3, (uint32_t)kptFile );
    REQUIRE_RC( KDirectoryRelease ( dir ) );
    
	REQUIRE_RC( KOutMsg( "done: CLEAR_CACHE_1\n" ) );
}


TEST_CASE( CLEAR_CACHE_2 )
{
	REQUIRE_RC( KOutMsg( "running: CLEAR_CACHE_2\n" ) );

	REQUIRE_RC( KOutMsg( "clearing: %s\n", new_home ) );
	clear_out( new_home );

    const VDBManager * vdb_mgr;
    REQUIRE_RC( VDBManagerMakeRead( &vdb_mgr, NULL ) );
    REQUIRE_RC( VDBManagerDeleteCacheOlderThan ( vdb_mgr, 0 ) );
    REQUIRE_RC( VDBManagerRelease ( vdb_mgr ) );

	REQUIRE_RC( KOutMsg( "done: CLEAR_CACHE_2\n" ) );
}


static rc_t write_root( KConfig *cfg, const char * base, const char * cat, const char * sub_cat )
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


static rc_t write_dflt_path( KConfig *cfg, const char * base )
{
    char value[ 256 ];
    size_t num_writ;
    rc_t rc = string_printf ( value, sizeof value, &num_writ, "%s/ncbi", base );
    if ( rc == 0 )
        rc = KConfigWriteString( cfg, "/repository/user/default-path", value );
    return rc;
}

#ifdef WINDOWS
static char * convert_sys_path( const char * sys_path )
{
    char * res = NULL;
    VFSManager * vfs_mgr;
    rc_t rc = VFSManagerMake ( &vfs_mgr );
    if ( rc == 0 )
    {
        VPath * p;
        rc = VFSManagerMakeSysPath ( vfs_mgr, &p, sys_path );
        if ( rc == 0 )
        {
            const String * S;
            rc = VPathMakeString( p, &S );
            if ( rc == 0 )
                res = string_dup ( S->addr, S->size );
            VPathRelease( p );
        }
        VFSManagerRelease( vfs_mgr );
    }
    return res;
}
#endif

static rc_t create_test_config( KConfig **cfg, const char * base )
{
    rc_t rc = KConfigMake ( cfg, NULL );
    if ( rc == 0 )
    {
#ifdef WINDOWS
        const char * cfg_base = convert_sys_path( base );
#else
        const char * cfg_base = base;
#endif
        if ( cfg_base != NULL )
        {
            rc = write_root( *cfg, cfg_base, "main", "public" );
            if ( rc == 0 )
                rc = write_root( *cfg, cfg_base, "protected", "dbGaP-2956" );
            if ( rc == 0 )
                rc = write_root( *cfg, cfg_base, "protected", "dbGaP-4831" );
            if ( rc == 0 )
                rc = write_dflt_path( *cfg, cfg_base );
#ifdef WINDOWS
            free( ( void * ) cfg_base );
#endif
        }
    }
    return rc;
}

char * org_home;
char new_home_buffer[ 4096 ]; /* buffer for putenv has to stay alive! */

static rc_t prepare_test( KConfig **cfg, const char * sub )
{
	size_t num_writ;
#ifdef WINDOWS
    org_home = getenv ( "USERPROFILE" );
    rc_t rc = string_printf ( new_home, sizeof new_home, &num_writ, "%s\\%s", org_home, sub );
#else
	org_home = getenv( "HOME" );
    rc_t rc = string_printf ( new_home, sizeof new_home, &num_writ, "%s/%s", org_home, sub );
#endif
    if ( rc == 0 )
#ifdef WINDOWS
        rc = string_printf ( new_home_buffer, sizeof new_home_buffer, &num_writ, "HOME=%s", new_home );
#else
        rc = string_printf ( new_home_buffer, sizeof new_home_buffer, &num_writ, "USERPROFILE=%s", new_home );
#endif
    if ( rc == 0 )
        rc = putenv( new_home_buffer );
    if ( rc == 0 )
        rc = create_test_config( cfg, new_home );
    return rc;
}

void finish_test( const char * sub )
{
    /* clear the temp. home-directory */
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir( &dir );
    if ( rc == 0 )
    {
#ifdef WINDOWS
        rc = KDirectoryRemove( dir, true, "%s/%s", org_home, sub );
#else
        rc = KDirectoryRemove( dir, true, "%s\\%s", org_home, sub );
#endif
        rc = KDirectoryRemove( dir, true, "%s", new_home );
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
    KConfigDisableUserSettings();
    const char HomeSub[] = "test_root_history";
    KConfig *cfg;
    rc_t rc = prepare_test( &cfg, HomeSub );
    if ( rc == 0 )
        rc = VDB_3061( argc, argv );
    finish_test( HomeSub );
    KConfigRelease ( cfg );
    return rc;
}

}
