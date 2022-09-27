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

#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <vdb/table.h>
#include <vdb/database.h>
#include <kdb/meta.h>

#include <ktst/unit_test.hpp> // THROW_ON_RC

#include <sstream>

using namespace std;

TEST_SUITE( VDB_META_CMP_COPY_Suite )

const char * tbl_test_schema = "version 1; table A_TABLE #1.0 { column U8 C1; column U32 C2; };";

rc_t write_some_data( VTable * t, bool full ) {
    VCursor * c;
    rc_t rc = VTableCreateCursorWrite( t, &c, kcmInsert );
    if ( 0 == rc ) {
        uint32_t col_ids_1[ 2 ];
        rc = VCursorAddColumn( c, &( col_ids_1[ 0 ] ), "C1" );
        if ( 0 == rc ) { rc = VCursorAddColumn( c, &( col_ids_1[ 1 ] ), "C2" ); }
        if ( 0 == rc ) { rc = VCursorOpen( c ); }
        const uint8_t data8[ 5 ] = { 1, 2, 3, 4, 5 };
        const uint32_t data32[ 5 ] = { 10, 20, 30, 40, 50 };
        for ( uint32_t i = 0; 0 == rc && i < 10; ++i ) {
            rc = VCursorOpenRow( c );
            uint32_t n = full ? 5 : 4;
            if ( 0 == rc ) { rc = VCursorWrite( c, col_ids_1[ 0 ], 8, data8, 0, n ); }
            if ( 0 == rc ) { rc = VCursorWrite( c, col_ids_1[ 1 ], 32, data32, 0, n ); }
            if ( 0 == rc ) { rc = VCursorCommitRow( c ); }
            if ( 0 == rc ) { rc = VCursorCloseRow( c ); }
        }
        if ( 0 == rc ) { rc = VCursorCommit( c ); }
        VCursorRelease( c );
    }
    if ( 0 == rc ) { rc = VTableRelease( t ); }
    return rc;
}

rc_t append_some_metadata( VTable * t ) {
    KMetadata * meta;
    rc_t rc = VTableOpenMetadataUpdate( t, &meta );
    if ( 0 == rc ) {
        KMDataNode * special;
        rc = KMetadataOpenNodeUpdate( meta, &special, "/special" );
        if ( 0 == rc ) {
            rc = KMDataNodeWriteCString( special, "hallo" );
            if ( 0 == rc ) {
                rc = KMetadataCommit( meta );
            }
            KMDataNodeRelease( special );
        }
        KMetadataRelease( meta );
    }
    return rc;
}

void remove_dir( KDirectory *d, const char * name ) {
    if ( nullptr != d ) { KDirectoryRemove( d, true, "%s", name ); }
}

void remove_dirs( KDirectory *d, const char * name1, const char * name2 ) {
    remove_dir( d, name1 );
    remove_dir( d, name2 );
}

/*
 * create 2 vdb-tables, insert some rows, create some meta-data, compare them
 */
TEST_CASE( Compare_Table_Meta_Equal ) {
    KDirectory * Dir;
    REQUIRE_RC( KDirectoryNativeDir( &Dir ) );

    VDBManager * Mgr;
    REQUIRE_RC( VDBManagerMakeUpdate( &Mgr, Dir ) );

    /* make 2 simple tables */
    VSchema * Schema;
    REQUIRE_RC( VDBManagerMakeSchema( Mgr, &Schema ) );
    REQUIRE_RC( VSchemaParseText( Schema, "TableTestSchema", tbl_test_schema, strlen( tbl_test_schema ) ) );

    VTable * Tbl1;
    VTable * Tbl2;    
    REQUIRE_RC( VDBManagerCreateTable( Mgr, &Tbl1, Schema, "A_TABLE", kcmInit | kcmMD5, "%s", "TBL1" ) );
    REQUIRE_RC( VDBManagerCreateTable( Mgr, &Tbl2, Schema, "A_TABLE", kcmInit | kcmMD5, "%s", "TBL2" ) );

    /* write some data into them */
    REQUIRE_RC( write_some_data( Tbl1, true ) );    // creates identical data in both tables
    REQUIRE_RC( write_some_data( Tbl2, true ) );

    /* the meta-node 'col' should now be equal in both tables... */
    /*
     * very important: we cannot compare tables that are still open after
     * beeing written into them.
     * 
     * these tables have to be closed, then reopened in read-only mode!
     * --- the write_some_data() - function does that...
     */
    const VTable * Tbl1_c;
    const VTable * Tbl2_c;
    REQUIRE_RC( VDBManagerOpenTableRead( Mgr, &Tbl1_c, nullptr, "TBL1" ) );
    REQUIRE_RC( VDBManagerOpenTableRead( Mgr, &Tbl2_c, nullptr, "TBL2" ) );

    bool equal;
    REQUIRE_RC( VTableMetaCompare( Tbl1_c, Tbl2_c, "col", &equal ) );
    REQUIRE_EQ( equal, true );

    REQUIRE_RC( VTableRelease( Tbl2_c ) );
    REQUIRE_RC( VTableRelease( Tbl1_c ) );

    REQUIRE_RC( VSchemaRelease( Schema ) );
    REQUIRE_RC( VDBManagerRelease( Mgr ) );
    remove_dirs( Dir, "TBL1", "TBL2" );
    REQUIRE_RC( KDirectoryRelease( Dir ) );
}

TEST_CASE( Compare_Table_Meta_Not_Equal ) {
    KDirectory * Dir;
    REQUIRE_RC( KDirectoryNativeDir( &Dir ) );

    VDBManager * Mgr;
    REQUIRE_RC( VDBManagerMakeUpdate( &Mgr, Dir ) );

    /* make 2 simple tables */
    VSchema * Schema;
    REQUIRE_RC( VDBManagerMakeSchema( Mgr, &Schema ) );
    REQUIRE_RC( VSchemaParseText( Schema, "TableTestSchema", tbl_test_schema, strlen( tbl_test_schema ) ) );

    VTable * Tbl1;
    REQUIRE_RC( VDBManagerCreateTable( Mgr, &Tbl1, Schema, "A_TABLE", kcmInit | kcmMD5, "%s", "TBL1" ) );

    VTable * Tbl2;
    REQUIRE_RC( VDBManagerCreateTable( Mgr, &Tbl2, Schema, "A_TABLE", kcmInit | kcmMD5, "%s", "TBL2" ) );

    REQUIRE_RC( append_some_metadata( Tbl1 ) );
    /* write some data into them */
    REQUIRE_RC( write_some_data( Tbl1, true ) );        // creates different data in the tables
    REQUIRE_RC( write_some_data( Tbl2, false ) );

    /* the meta-node 'col' should not be equal in both tables... ( different data )*/
    /*
     * very important: we cannot compare tables that are still open after
     * beeing written into them.
     * 
     * these tables have to be closed, then reopened in read-only mode!
     * --- the write_some_data() - function does that...     * 
     */
    const VTable * Tbl1_c;
    const VTable * Tbl2_c;
    REQUIRE_RC( VDBManagerOpenTableRead( Mgr, &Tbl1_c, nullptr, "TBL1" ) );
    REQUIRE_RC( VDBManagerOpenTableRead( Mgr, &Tbl2_c, nullptr, "TBL2" ) );

    bool equal;
    REQUIRE_RC( VTableMetaCompare( Tbl1_c, Tbl2_c, "col", &equal ) );
    REQUIRE_EQ( equal, false );

    REQUIRE_RC( VTableRelease( Tbl2_c ) );
    REQUIRE_RC( VTableRelease( Tbl1_c ) );

    REQUIRE_RC( VSchemaRelease( Schema ) );
    REQUIRE_RC( VDBManagerRelease( Mgr ) );
    REQUIRE_RC( KDirectoryRelease( Dir ) );
}

TEST_CASE( Copy_Table_Meta ) {
    KDirectory * Dir;
    REQUIRE_RC( KDirectoryNativeDir( &Dir ) );

    VDBManager * Mgr;
    REQUIRE_RC( VDBManagerMakeUpdate( &Mgr, Dir ) );

    /* make 2 simple tables */
    VSchema * Schema;
    REQUIRE_RC( VDBManagerMakeSchema( Mgr, &Schema ) );
    REQUIRE_RC( VSchemaParseText( Schema, "TableTestSchema", tbl_test_schema, strlen( tbl_test_schema ) ) );

    VTable * Tbl1;
    REQUIRE_RC( VDBManagerCreateTable( Mgr, &Tbl1, Schema, "A_TABLE", kcmInit | kcmMD5, "%s", "TBL1" ) );

    VTable * Tbl2;
    REQUIRE_RC( VDBManagerCreateTable( Mgr, &Tbl2, Schema, "A_TABLE", kcmInit | kcmMD5, "%s", "TBL2" ) );

    REQUIRE_RC( append_some_metadata( Tbl1 ) );
    /* write some data into them */
    REQUIRE_RC( write_some_data( Tbl1, true ) );        // creates different data in the tables
    REQUIRE_RC( write_some_data( Tbl2, false ) );

    const VTable * src;
    VTable * dst;
    REQUIRE_RC( VDBManagerOpenTableRead( Mgr, &src, nullptr, "TBL1" ) );
    REQUIRE_RC( VDBManagerOpenTableUpdate( Mgr, &dst, nullptr, "TBL2" ) );

    // check that there is not special-node in dst
    bool equal;
    rc_t rc = VTableMetaCompare( src, dst, "special", &equal );
    REQUIRE_NE( rc, ( rc_t )0 );
    REQUIRE_EQ( equal, false );

    // copy the special node from src to dst
    REQUIRE_RC( VTableMetaCopy( dst, src, "special" ) );
    // check that this special node is now equal in both tables
    REQUIRE_RC( VTableMetaCompare( src, dst, "special", &equal ) );
    REQUIRE_EQ( equal, true );
    
    REQUIRE_RC( VTableRelease( src ) );
    REQUIRE_RC( VTableRelease( dst ) );

    REQUIRE_RC( VSchemaRelease( Schema ) );
    REQUIRE_RC( VDBManagerRelease( Mgr ) );
    remove_dirs( Dir, "TBL1", "TBL2" );
    REQUIRE_RC( KDirectoryRelease( Dir ) );

}

//////////////////////////////////////////// Main
extern "C" {
    #include <kapp/args.h>
    ver_t CC KAppVersion( void ) { return 0x1000000; }
    rc_t CC UsageSummary(const char * progname) { return 0; }
    rc_t CC Usage( const Args * args ) { return 0; }
    const char UsageDefaultName[] = "test-vdb-meta-cmp-copy";
    rc_t CC KMain( int argc, char *argv [] ) { return VDB_META_CMP_COPY_Suite( argc, argv ); }
}
