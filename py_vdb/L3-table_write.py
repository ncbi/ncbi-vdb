#!/usr/bin/env python

from vdb import *

lib_wr = "./2.8.0/libncbi-wvdb.so.2.8.0"

schematxt = '''
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

''' 

def fill_table_random( cur, cols, row_count, value_count ) :
    for idx in xrange( 0, row_count ) :
        cur.OpenRow()
        cols[ "C1" ].write_rand( value_count, 100 )
        cols[ "C2" ].write_rand( value_count, 100000 )
        cols[ "C3" ].write_rand( value_count )
        cols[ "C4" ].write_rand( value_count, 100000 )
        cols[ "C5" ].write_rand( value_count, 100000 )
        cols[ "C6" ].write_rand( value_count )
        cur.CommitRow()
        cur.CloseRow()

def fill_table_with_values( cur, cols ) :
    cur.OpenRow()
    cols[ "C1" ].write( [ 1, 2, 3, 4 ] )
    cols[ "C2" ].write( [ 1000, 1001, 1002 ] )
    cols[ "C3" ].write( "hallo" )
    cols[ "C4" ].write( [ 101010, 2020, 3030 ] )
    cols[ "C5" ].write( [ 10.101, 20.202, 30.303 ] )
    cols[ "C6" ].write( [ True, True, False, False, True ] )
    cur.CommitRow()
    cur.CloseRow()

def fill_table_with_single_values( cur, cols ) :
    cur.OpenRow()
    cols[ "C1" ].write( 1 )
    cols[ "C2" ].write( 1000 )
    cols[ "C3" ].write( "hallo" )
    cols[ "C4" ].write( 5544 )
    cols[ "C5" ].write( 55.5 )
    cols[ "C6" ].write( True )
    cur.CommitRow()
    cur.CloseRow()

def fill_table_with_default_values( cur, cols, row_count ) :
    cols[ "C1" ].set_default( [ 100, 101 ] )
    for idx in xrange( 0, row_count ) :
        cur.OpenRow()
        cols[ "C2" ].write( [ 10 + idx, 11 + idx, 12 + idx ] )
        cols[ "C3" ].write( "line #%d"%idx )
        cols[ "C4" ].write( [ 101 + idx, 102 + idx, 103 + idx ] )
        cols[ "C5" ].write( [ 10.1 + idx, 20.2 + idx, 30.3 + idx ] )
        cols[ "C6" ].write( [ True ] )
        cur.CommitRow()
        cur.CloseRow()

def make_table( mgr, schema_txt, table_name ) :
    try :
        schema = mgr.MakeSchema( schema_txt )
        tbl = mgr.CreateTable( schema, "A_TABLE", table_name )
        cur = tbl.CreateCursor( OpenMode.Write )
        cols = cur.OpenColumns( [ "C1", "C2", "C3", "C4", "C5", "C6" ] )
        fill_table_random( cur, cols, 2, 5 )
        fill_table_with_values( cur, cols )
        fill_table_with_single_values( cur, cols )
        fill_table_with_default_values( cur, cols, 2 )
        cur.Commit()
    except vdb_error as e :
        print( e )

        
if __name__ == '__main__' :
    
    table_name = "L3"
        
    try :
        #open a manager
        mgr = manager( OpenMode.Write, lib_wr )
    
        make_table( mgr, schematxt, table_name )
        mgr.OpenTable( table_name ).print_rows()

    except vdb_error as e :
        print( e )
