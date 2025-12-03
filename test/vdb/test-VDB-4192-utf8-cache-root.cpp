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

#include "klib/defs.h"
#include <ktst/unit_test.hpp> // TEST_CASE

#include <vdb/manager.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#include <kfs/directory.h>

using namespace std;


TEST_SUITE( VDB_4192 )

std::string get_home_dir( void ) {
    std::string res;
    VFSManager * vfs_manager;
    rc_t rc = VFSManagerMake( &vfs_manager );
    if ( rc == 0 ) {
        KDirectory * dir;
        rc = VFSManagerGetCWD( vfs_manager, &dir );
        if ( rc == 0 ) {
            char buffer[ 4096 ];
            rc = KDirectoryResolvePath( dir, true, buffer, sizeof buffer, "." );
            if ( rc == 0 ) {
                res . assign( buffer );
            }
            KDirectoryRelease( dir );
        }
        VFSManagerRelease( vfs_manager );
    }
    return res;
}


//////////////////////////////////////////// Main
int main ( int argc, char *argv [] )
{
    std::string cwd = get_home_dir();
    if ( cwd . empty() ) {
        FAIL( "cannot determine current working directory" );
    } else {
        std::cout << "cwd = " << cwd << std::endl;
        const VDBManager * vdb_mgr;
        rc_t rc = VDBManagerMakeRead( &vdb_mgr, NULL );
        if ( rc != 0 ) {
            FAIL( "FAIL: VDBManagerMakeRead( &vdb_mgr ) failed" );
        } else {
            VFSManager * vfs_mgr = NULL;
            rc = VFSManagerMake( &vfs_mgr );
            if ( rc != 0 ) {
                FAIL( "FAIL: VFSManagerMake( &vfs_mgr ) failed" );
            } else {
                VPath * vpath;
                const std::string s{ "/some/other/p哈path" };

                rc = VFSManagerMakePath( vfs_mgr, &vpath, s . c_str() );
                if ( rc != 0 ) {
                    FAIL( "FAIL: VFSManagerMakePath() failed" );
                } else {
                    rc = VDBManagerSetCacheRoot( vdb_mgr, vpath );
                    if ( rc != 0 ) {
                        FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, vpath ) failed" );
                    }
                    VPathRelease( vpath );
                }
                VFSManagerRelease( vfs_mgr );
            }
            VDBManagerRelease( vdb_mgr );
        }
    }
    return 0;
}
