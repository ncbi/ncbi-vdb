#!/usr/bin/env python3

from vdb import *

#for included schema to be found:
#assuming we have vdb.vschema at $HOME/devel/ncbi-vdb/interfaces/vdb_error
#we need this line in $HOME/.ncbi/user-settings.mkfg:
#/vdb/schema/paths = "$HOME/devel/ncbi-vdb/interfaces"

schematxt = '''
version 1;

include 'vdb/vdb.vschema';

table A_TABLE #1.0
{
    column < U8 > izip_encoding C1;
};

''' 

def fill_table_random( cur, cols, row_count, value_count ) :
    for idx in xrange( 0, row_count ) :
        cur.OpenRow()
        cols[ "C1" ].write_rand( value_count, 255 )
        cur.CommitRow()
        cur.CloseRow()

def make_table( mgr, schema_txt, table_name ) :
    try :
        schema = mgr.MakeSchema( schema_txt )
        tbl = mgr.CreateTable( schema, "A_TABLE", table_name )
        cur = tbl.CreateCursor( OpenMode.Write )
        cols = cur.OpenColumns( [ "C1" ] )
        fill_table_random( cur, cols, 320, 1024 )
        cur.Commit()
    except vdb_error as e :
        print( e )

        
if __name__ == '__main__' :
    
    table_name = "L7"
        
    try :
        mgr = manager( OpenMode.Write )
    
        make_table( mgr, schematxt, table_name )
        mgr.OpenTable( table_name ).print_rows()

    except vdb_error as e :
        print( e )
