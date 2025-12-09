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
#include <cstdint>
#include <ktst/unit_test.hpp> // TEST_CASE

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
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

rc_t cstr_2_vpath( const char * s, VPath** vpath ) {
    VFSManager * vfs_mgr = NULL;
    rc_t rc = VFSManagerMake( &vfs_mgr );
    if ( 0 == rc ) {
        rc = VFSManagerMakePath( vfs_mgr, vpath, s );
        if ( 0 != rc ) {
            FAIL( "FAIL: VFSManagerMakePath() failed" );
        }
        VFSManagerRelease( vfs_mgr );
    } else {
        FAIL( "FAIL: VFSManagerMake( &vfs_mgr ) failed" );
    }
    return rc;
}

rc_t set_cache_root( const std::string& dir ) {
    const VDBManager * vdb_mgr;
    rc_t rc = VDBManagerMakeRead( &vdb_mgr, NULL );
    if ( rc != 0 ) {
        FAIL( "FAIL: VDBManagerMakeRead( &vdb_mgr ) failed [1]" );
    } else {
        VPath * vpath;
        rc = cstr_2_vpath( dir . c_str(), &vpath );
        rc = VDBManagerSetCacheRoot( vdb_mgr, vpath );
        if ( rc != 0 ) {
            FAIL( "FAIL: VDBManagerSetCacheRoot( mgr, vpath ) failed" );
        } else {
            std::cout << "VDBManagerSetCacheRoot : OK"<< std::endl;
        }
        VPathRelease( vpath );
        VDBManagerRelease( vdb_mgr );
    }
    return rc;
}

rc_t open_db( const std::string& acc, const VDatabase ** db ) {
    VPath * vpath;
    rc_t rc = cstr_2_vpath( acc . c_str(), &vpath );
    if ( 0 == rc ) {
        const VDBManager * mgr;
        rc = VDBManagerMakeRead( &mgr, NULL );
        if ( rc != 0 ) {
            FAIL( "FAIL: VDBManagerMakeRead( &vdb_mgr ) failed [1]" );
        } else {
            rc = VDBManagerOpenDBReadVPath( mgr, db, nullptr, vpath );
            if ( 0 != rc  ) {
                FAIL( "FAIL: VDBManagerOpenDBReadVPath() failed" );
            }
            VDBManagerRelease( mgr );
        }
        VPathRelease( vpath );
    }
    return rc;
}

rc_t access_sra( const std::string& acc ) {
    const VDatabase * db;
    rc_t rc = open_db( acc, &db );
    if ( 0 == rc ) {
        const VTable * tbl;
        rc = VDatabaseOpenTableRead( db, &tbl, "SEQUENCE" );
        if ( rc != 0 ) {
            FAIL( "FAIL: VDatabaseOpenTableRead() failed" );
        } else {
            const VCursor * cur;
            rc = VTableCreateCursorRead( tbl, &cur );
            if ( rc != 0 ) {
                FAIL( "FAIL: VTableCreateCursorRead() failed" );
            } else {
                uint32_t idx;
                rc = VCursorAddColumn( cur, &idx, "READ" );
                if ( rc != 0 ) {
                    FAIL( "FAIL: VCursorAddColumn( READ ) failed" );
                } else {
                    rc = VCursorOpen( cur );
                    if ( rc != 0 ) {
                        FAIL( "FAIL: VCursorOpen() failed" );
                    } else {
                        uint32_t elem_bits, boff, row_len;
                        const void * base;
                        rc = VCursorCellDataDirect( cur, 1, idx, &elem_bits, &base, &boff, &row_len );
                        if ( rc != 0 ) {
                            FAIL( "FAIL: VCursorCellDataDirect() failed" );
                        } else {
                            std::cout << "elem_bits : " << elem_bits << std::endl;
                            std::cout << "boff      : " << boff << std::endl;
                            std::cout << "row_len   : " << row_len << std::endl;
                        }
                    }
                }
                VCursorRelease( cur );
            }
            VTableRelease( tbl );
        }
        VDatabaseRelease( db );
    }
    return rc;
}

//////////////////////////////////////////// Main
int main ( int argc, char *argv [] )
{
    rc_t rc = 0;
    std::string dir;
    if ( argc > 1 ) {
        dir = argv[ 1 ];
    } else {
        dir = get_home_dir();
    }
    if ( dir . empty() ) {
        FAIL( "cannot determine current working directory" );
    } else {
        rc = set_cache_root( dir );
        if ( rc == 0 ) {
            rc = access_sra( "SRR341578" );
        }
    }
    return rc;
}
