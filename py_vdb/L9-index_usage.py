#!/usr/bin/env python

from vdb import *

lib_wr = "./2.8.0/libncbi-wvdb.so.2.8.0"

schematxt = '''
version 1;
table A_TABLE #1.0
{
    column U32 C1;
    column ascii C2;
};

''' 

def fill_table_random( cur, cols, kidx, row_count, value_count ) :
    for idx in xrange( 0, row_count ) :
        cur.OpenRow()
        row_id = cur.RowId()
        cols[ "C1" ].write_rand( value_count, 0xffffffff )
        rand_str = random_string( value_count )
        cols[ "C2" ].write( rand_str )
        cur.CommitRow()
        cur.CloseRow()
        #print( "Row = %d"%row_id )
        if ( idx % 1000 ) == 0 :
            kidx.InsertText( True, rand_str, row_id )

def make_table( mgr, schema_txt, table_spec, table_name ) :
    try :
        schema = mgr.MakeSchema( schema_txt )
        tbl = mgr.CreateTable( schema, table_spec, table_name )
        kidx = tbl.CreateIndex( "C2_IDX", IndexType.Text )
        cur = tbl.CreateCursor( OpenMode.Write )
        cols = cur.OpenColumns( [ "C1", "C2" ] )
        fill_table_random( cur, cols, kidx, 200000, 25 )
        kidx.Commit()
        cur.Commit()
    except vdb_error as e :
        print( e )


if __name__ == '__main__' :
    
    table_name = 'L9'
    table_spec = 'A_TABLE'
    
    try :
        #open a manager
        mgr = manager( OpenMode.Write, lib_wr )
    
        make_table( mgr, schematxt, table_spec, table_name )
        #mgr.OpenTable( table_name ).print_rows()

    except vdb_error as e :
        print( e )
