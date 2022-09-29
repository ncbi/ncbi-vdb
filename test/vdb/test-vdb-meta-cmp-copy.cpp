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

#include <map>
#include <string>
#include <stdexcept>

using namespace std;

TEST_SUITE( VDB_META_CMP_COPY_Suite )

const char * tbl_test_schema = "version 1; table A_TABLE #1.0 { column U8 C1; column U32 C2; };";

const char * db_test_schema = "version 1;\
    table SUB_TAB #1.0 { column U8 C1; column U16 C2; };\
    database MAIN_DB #1.0 { table SUB_TAB #1 T1; }; ";

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

class Test_Meta_Fixture {
public:
    typedef std::map< std::string, const VTable * > t_tables;
    typedef std::map< std::string, const VDatabase * > t_dbs;
    
    KDirectory * dir;
    VDBManager * mgr;
    VSchema * schema;
    bool remove_in_destructor;
    t_tables tables;
    t_dbs dbs;
    
    Test_Meta_Fixture() : dir( nullptr ), mgr( nullptr ), schema( nullptr ),
                         remove_in_destructor( true ) {
        if ( 0 != KDirectoryNativeDir( &dir ) ) {
            throw std :: logic_error ( "Test_Meta_Fixture: KDirectoryNativeDir failed" );
        }
        if ( 0 != VDBManagerMakeUpdate( &mgr, dir ) ) {
            throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerMakeUpdate failed" );
        }
        if ( 0 != VDBManagerMakeSchema( mgr, &schema ) ) {
            throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerMakeSchema failed" );
        }
    }

    VTable * create_table( const char * spec, std::string name, VDatabase * db = nullptr ) {
        VTable * tbl = nullptr;
        if ( nullptr == db ) {
            if ( 0 != VDBManagerCreateTable( mgr, &tbl, schema, spec, kcmInit | kcmMD5,
                                        name.c_str() ) ) {
                throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerCreateTable failed" );
            }
        } else {
            if ( 0 != VDatabaseCreateTable( db, &tbl, spec, kcmInit | kcmMD5,
                                        name.c_str() ) ) {
                throw std :: logic_error ( "Test_Meta_Fixture: VDatabaseCreateTable failed" );
            }
        }
        tables . insert( { name, tbl } ); 
        return tbl;
    }

    VDatabase * create_db( const char * spec, std::string name ) {
        VDatabase * db = nullptr;
        if ( 0 != VDBManagerCreateDB( mgr, &db, schema, spec, kcmInit | kcmMD5,
                                       name.c_str() ) ) {
            throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerCreateDB failed" );
        }
        dbs . insert( { name, db } ); 
        return db;
    }

    const VTable * open_table_read( std::string name, const VDatabase * db = nullptr ) {
        const VTable * tbl = nullptr;
        if ( nullptr == db ) {
            if ( 0 != VDBManagerOpenTableRead( mgr, &tbl, nullptr, name .  c_str() ) ) {
                throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerOpenTableRead failed" );
            }
        } else {
            if ( 0 != VDatabaseOpenTableRead( db, &tbl, nullptr, name .  c_str() ) ) {
                throw std :: logic_error ( "Test_Meta_Fixture: VDatabaseOpenTableRead failed" );
            }
        }
        tables . insert( { name, tbl } ); 
        return tbl;
    }

    const VDatabase * open_db_read( std::string name ) {
        const VDatabase * db = nullptr;
        if ( 0 != VDBManagerOpenDBRead( mgr, &db, nullptr, name . c_str() ) ) {
            throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerOpenDBRead failed" );
        }
        dbs . insert( { name, db } ); 
        return db;
    }

    VTable * open_table_update( std::string name, VDatabase * db = nullptr ) {
        VTable * tbl = nullptr;
        if ( nullptr == db ) {
            if ( 0 != VDBManagerOpenTableUpdate( mgr, &tbl, nullptr, name .  c_str() ) ) {
                throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerOpenTableUpdate failed" );
            }
        } else {
            if ( 0 != VDatabaseOpenTableUpdate( db, &tbl, nullptr, name .  c_str() ) ) {
                throw std :: logic_error ( "Test_Meta_Fixture: VDatabaseOpenTableRead failed" );
            }
        }
        tables . insert( { name, tbl } ); 
        return tbl;
    }

    VDatabase * open_db_update( std::string name ) {
        VDatabase * db = nullptr;
        if ( 0 != VDBManagerOpenDBUpdate( mgr, &db, nullptr, name .  c_str() ) ) {
            throw std :: logic_error ( "Test_Meta_Fixture: VDBManagerOpenDBUpdate failed" );
        }
        dbs . insert( { name, db } ); 
        return db;
    }

    void close_tables( bool remove_dir ) {
        for ( auto i = tables . begin(); i != tables . end(); ++i ) {
            VTableRelease( i -> second );
            if ( remove_dir ) { KDirectoryRemove( dir, true, "%s", i -> first . c_str() ); }
        }
        tables . clear();
    }

    void close_dbs( bool remove_dir ) {
        for ( auto i = dbs . begin(); i != dbs . end(); ++i ) {
            VDatabaseRelease( i -> second );
            if ( remove_dir ) { KDirectoryRemove( dir, true, "%s", i -> first . c_str() ); }
        }
        dbs . clear();
    }

    ~Test_Meta_Fixture() {
        close_tables( remove_in_destructor );
        close_dbs( remove_in_destructor );
        VSchemaRelease( schema );
        VDBManagerRelease( mgr );
        KDirectoryRelease( dir );
    }
};

/*
 * create 2 vdb-tables, insert some rows, create some meta-data, compare them
 */
FIXTURE_TEST_CASE( Compare_Table_Meta_Equal, Test_Meta_Fixture ) {
    REQUIRE_RC( VSchemaParseText( schema, "TableTestSchema", tbl_test_schema, strlen( tbl_test_schema ) ) );

    VTable * Tbl1 = create_table( "A_TABLE", "TBL1" );
    VTable * Tbl2 = create_table( "A_TABLE", "TBL2" );

    REQUIRE_RC( write_some_data( Tbl1, true ) );    // creates identical data in both tables
    REQUIRE_RC( write_some_data( Tbl2, true ) );
    close_tables( false );
    
    /* the meta-node 'col' should now be equal in both tables... */
    /*
     * very important: we cannot compare tables that are still open after
     * beeing written into them.
     * 
     * these tables have to be closed, then reopened in read-only mode!
     * --- the write_some_data() - function does that...
     */
    const VTable * Tbl1_c = open_table_read( "TBL1" );
    const VTable * Tbl2_c = open_table_read( "TBL2" );

    bool equal;
    REQUIRE_RC( VTableMetaCompare( Tbl1_c, Tbl2_c, "col", &equal ) );
    REQUIRE_EQ( equal, true );
}

FIXTURE_TEST_CASE( Compare_Table_Meta_Not_Equal, Test_Meta_Fixture ) {
    REQUIRE_RC( VSchemaParseText( schema, "TableTestSchema", tbl_test_schema, strlen( tbl_test_schema ) ) );

    VTable * Tbl1 = create_table( "A_TABLE", "TBL1" );
    VTable * Tbl2 = create_table( "A_TABLE", "TBL2" );

    REQUIRE_RC( append_some_metadata( Tbl1 ) );
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
    const VTable * Tbl1_c = open_table_read( "TBL1" );
    const VTable * Tbl2_c = open_table_read( "TBL2" );

    bool equal;
    REQUIRE_RC( VTableMetaCompare( Tbl1_c, Tbl2_c, "col", &equal ) );
    REQUIRE_EQ( equal, false );
}

FIXTURE_TEST_CASE( Copy_Table_Meta, Test_Meta_Fixture ) {
    REQUIRE_RC( VSchemaParseText( schema, "TableTestSchema", tbl_test_schema, strlen( tbl_test_schema ) ) );

    VTable * Tbl1 = create_table( "A_TABLE", "TBL1" );
    VTable * Tbl2 = create_table( "A_TABLE", "TBL2" );

    REQUIRE_RC( append_some_metadata( Tbl1 ) );
    REQUIRE_RC( write_some_data( Tbl1, true ) );        // creates different data in the tables
    REQUIRE_RC( write_some_data( Tbl2, false ) );

    const VTable * src = open_table_read( "TBL1" );
    VTable * dst = open_table_update( "TBL2" );

    // check that there is not special-node in dst
    bool equal;
    REQUIRE_RC( VTableMetaCompare( src, dst, "special", &equal ) );
    REQUIRE_EQ( equal, false );

    // copy the special node from src to dst
    REQUIRE_RC( VTableMetaCopy( dst, src, "special" ) );
    // check that this special node is now equal in both tables
    REQUIRE_RC( VTableMetaCompare( src, dst, "special", &equal ) );
    REQUIRE_EQ( equal, true );
}

FIXTURE_TEST_CASE( Compare_DB_Meta_Equal, Test_Meta_Fixture ) {
    REQUIRE_RC( VSchemaParseText( schema, "DBTestSchema", db_test_schema, strlen( db_test_schema ) ) );

    VDatabase * db1 = create_db( "MAIN_DB", "DB1" );
    VDatabase * db2 = create_db( "MAIN_DB", "DB2" );

    VTable * db1_t1 = create_table( "T1", "TBL1", db1 );
    VTable * db1_t2 = create_table( "T1", "TBL2", db1 );
    REQUIRE_RC( write_some_data( db1_t1, true ) );    // creates identical data in both tables
    REQUIRE_RC( write_some_data( db1_t2, true ) );

    VTable * db2_t1 = create_table( "T1", "TBL1", db2 );
    VTable * db2_t2 = create_table( "T1", "TBL2", db2 );
    REQUIRE_RC( write_some_data( db2_t1, true ) );    // creates identical data in both tables
    REQUIRE_RC( write_some_data( db2_t2, true ) );

    close_tables( false );
    close_dbs( false );

    const VDatabase * db1_c = open_db_read( "DB1" );
    const VDatabase * db2_c = open_db_read( "DB2" );
    
    bool equal;
    REQUIRE_RC( VDatabaseMetaCompare( db1_c, db2_c, "col", "TBL1", &equal ) );
    REQUIRE_EQ( equal, true );

    REQUIRE_RC( VDatabaseMetaCompare( db1_c, db2_c, "col", "TBL2", &equal ) );
    REQUIRE_EQ( equal, true );

    REQUIRE_RC( VDatabaseMetaCompare( db1_c, db2_c, "col", nullptr, &equal ) );
    REQUIRE_EQ( equal, true );
}

FIXTURE_TEST_CASE( Compare_DB_Meta_Not_Equal, Test_Meta_Fixture ) {
    REQUIRE_RC( VSchemaParseText( schema, "DBTestSchema", db_test_schema, strlen( db_test_schema ) ) );

    VDatabase * db1 = create_db( "MAIN_DB", "DB1" );
    VDatabase * db2 = create_db( "MAIN_DB", "DB2" );

    VTable * db1_t1 = create_table( "T1", "TBL1", db1 );
    VTable * db1_t2 = create_table( "T1", "TBL2", db1 );
    REQUIRE_RC( write_some_data( db1_t1, true ) );    // creates identical data in both tables
    REQUIRE_RC( write_some_data( db1_t2, true ) );

    VTable * db2_t1 = create_table( "T1", "TBL1", db2 );
    VTable * db2_t2 = create_table( "T1", "TBL2", db2 );
    REQUIRE_RC( write_some_data( db2_t1, false ) );    // creates different data in both tables
    REQUIRE_RC( write_some_data( db2_t2, false ) );    // different from db1_*

    close_tables( false );
    close_dbs( false );

    const VDatabase * db1_c = open_db_read( "DB1" );
    const VDatabase * db2_c = open_db_read( "DB2" );
    
    bool equal;
    REQUIRE_RC( VDatabaseMetaCompare( db1_c, db2_c, "col", "TBL1", &equal ) );
    REQUIRE_EQ( equal, false );

    REQUIRE_RC( VDatabaseMetaCompare( db1_c, db2_c, "col", "TBL2", &equal ) );
    REQUIRE_EQ( equal, false );

    REQUIRE_RC( VDatabaseMetaCompare( db1_c, db2_c, "col", nullptr, &equal ) );
    REQUIRE_EQ( equal, false );
}

FIXTURE_TEST_CASE( Copy_DB_Meta, Test_Meta_Fixture ) {
    REQUIRE_RC( VSchemaParseText( schema, "DBTestSchema", db_test_schema, strlen( db_test_schema ) ) );

    VDatabase * db1 = create_db( "MAIN_DB", "DB1" );
    VDatabase * db2 = create_db( "MAIN_DB", "DB2" );

    VTable * db1_t1 = create_table( "T1", "TBL1", db1 );
    VTable * db1_t2 = create_table( "T1", "TBL2", db1 );
    REQUIRE_RC( append_some_metadata( db1_t1 ) );
    REQUIRE_RC( write_some_data( db1_t1, true ) );    // creates identical data in both tables
    REQUIRE_RC( write_some_data( db1_t2, true ) );

    VTable * db2_t1 = create_table( "T1", "TBL1", db2 );
    VTable * db2_t2 = create_table( "T1", "TBL2", db2 );
    REQUIRE_RC( write_some_data( db2_t1, true ) );    // creates different data in both tables
    REQUIRE_RC( write_some_data( db2_t2, true ) );    // different from db1_*

    close_tables( false );
    close_dbs( false );

    const VDatabase * src = open_db_read( "DB1" );
    VDatabase * dst = open_db_update( "DB2" );

    bool equal;
    REQUIRE_RC( VDatabaseMetaCompare( src, dst, "special", nullptr, &equal ) );
    REQUIRE_EQ( equal, false );

    // copy the special node from src to dst
    REQUIRE_RC( VDatabaseMetaCopy( dst, src, "special", nullptr ) );

    // it is not equal for all tables...
    REQUIRE_RC( VDatabaseMetaCompare( src, dst, "special", nullptr, &equal ) );
    REQUIRE_EQ( equal, false );

    // but it is equal for TBL1...
    REQUIRE_RC( VDatabaseMetaCompare( src, dst, "special", "TBL1", &equal ) );
    REQUIRE_EQ( equal, true );

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
