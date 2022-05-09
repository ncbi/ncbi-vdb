#!/usr/bin/env python

import os, sys, shutil
from vdb import *

test_schema = '''
version 1;

table A_TABLE #1.0
{
    column U8 C1;
    column U32 C2;
    column ascii C3;
    column I16 C4;
    column F32 C5;
    column bool C6;
};

database SUB_DB #1.0
{
    table A_TABLE #1 TAB20;
};

database MAIN_DB #1.0
{
    table A_TABLE #1 TAB10;
    database SUB_DB #1 SUBDB;
};
''' 

tbl_columns = [ "C1", "C2", "C3", "C4", "C5", "C6" ]
C1_values = [ 1, 2, 3, 4 ]
C2_values = [ 1000, 1001, 1002 ]
C3_values = "hallo"
C4_values = [ -10101, 2020, 3030 ]
C5_values = [ 10.101, 20.202, 30.303 ]
C6_values = [ True, True, False, False, True ]

def mgr_test( mgr ) :
    print( "\ntesting the manager: " )
    print( "lib:      '%s'"%( mgr.libname ) )
    print( "version:  '%s'"%( mgr.Version() ) )
    print( "writable: '%r'"%( mgr.writable() ) )

def fill_table_with_values( cols ) :
    cur = cols[ "C1" ].cursor()
    cur.OpenRow()
    cols[ "C1" ].write( C1_values )
    cols[ "C2" ].write( C2_values )
    cols[ "C3" ].write( C3_values )
    cols[ "C4" ].write( C4_values )
    cols[ "C5" ].write( C5_values ) #!!! this writes rounded values !!!
    cols[ "C6" ].write( C6_values )
    cur.CommitRow()
    cur.CloseRow()
    cur.Commit()

def tbl_write_test( mgr, tbl_path ) :
    print( "\ntesting table-writing: " )
    schema = mgr.MakeSchema( test_schema )
    cur = mgr.CreateTable( schema, "A_TABLE", tbl_path ).CreateCursor( OpenMode.Write )
    fill_table_with_values( cur.OpenColumns( tbl_columns ) )

def compare_values( cols ) :
    res = cols[ "C1" ].read( 1 ) == C1_values
    res = res and cols[ "C2" ].read( 1 ) == C2_values
    res = res and cols[ "C3" ].read( 1 ) == C3_values
    res = res and cols[ "C4" ].read( 1 ) == C4_values
    #!!! we have to round the values, because F32 did the rounding at writing !!!
    res = res and list( map( lambda x: round( x, 3 ), cols[ "C5" ].read( 1 ) ) ) == C5_values
    return res and cols[ "C6" ].read( 1 ) == C6_values

def tbl_read_test( mgr, tbl_path ) :
    print( "\ntesting table-reading: " )
    res = compare_values( mgr.OpenTable( tbl_path ).CreateCursor().OpenColumns( tbl_columns ) )
    print( f"tbl_read_test: {res}" )
    return res

def db_write_test( mgr, db_path ) :
    print( "\ntesting database-writing: " )
    schema = mgr.MakeSchema( test_schema )
    db = mgr.CreateDB( schema, "MAIN_DB", db_path )
    cur1 = db.CreateTable( "TAB10" ).CreateCursor( OpenMode.Write )
    fill_table_with_values( cur1.OpenColumns( tbl_columns ) )
    subdb = db.CreateDB( "SUBDB" )
    cur2 = subdb.CreateTable( "TAB20" ).CreateCursor( OpenMode.Write )
    fill_table_with_values( cur2.OpenColumns( tbl_columns ) )

def db_read_test( mgr, db_path ) :
    print( "\ntesting database-reading: " )
    db = mgr.OpenDB( db_path )
    res = compare_values( db.OpenTable( "TAB10" ).CreateCursor().OpenColumns( tbl_columns ) )
    print( f"tbl_read_test (TAB10) : {res}" )
    res = res and compare_values( db.OpenDB( "SUBDB" ).OpenTable( "TAB20" ).CreateCursor().OpenColumns( tbl_columns ) )
    print( f"tbl_read_test (TAB20) : {res}" )
    return res

if __name__ == '__main__' :
    location = "py_vdb_test_folder"
    if not os.path.isdir( location ) : os.mkdir( location )
    if not os.path.isdir( location ) : sys.exit( 3 )
    try :
        mgr_test( manager( OpenMode.Read ) )
        mgr_test( manager( OpenMode.Write ) )

        tbl_path = os.path.join( os.path.sep, os.getcwd(), location, "tbl_a" )
        tbl_write_test( manager( OpenMode.Write ), tbl_path )
        if not tbl_read_test( manager( OpenMode.Read ), tbl_path ) : sys.exit( 3 )

        db_path = os.path.join( os.path.sep, os.getcwd(), location, "db_a" )
        db_write_test( manager( OpenMode.Write ), db_path )
        if not db_read_test( manager( OpenMode.Read ), db_path ) : sys.exit( 3 )

    except vdb_error as e :
        print( e )
        sys.exit( 3 )

    if os.path.isdir( location ) : shutil.rmtree( location )
