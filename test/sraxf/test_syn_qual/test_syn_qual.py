#!/usr/bin/env python

import sys
import os.path
from vdb import *

schematxt = '''
version 1;

include 'insdc/insdc.vschema';

extern function
INSDC:quality:phred NCBI:SRA:syn_quality #1
    < INSDC:quality:phred good_quality, INSDC:quality:phred bad_quality >
    ( INSDC:coord:len read_len, INSDC:SRA:spot_filter spot_filter );

table TEST_TBL #1.0
{
    column INSDC:coord:len READ_LEN;
    column INSDC:SRA:spot_filter SPOT_FILTER;
    
    INSDC:coord:len out_read_len = .READ_LEN;
    INSDC:SRA:spot_filter out_spot_filter = .SPOT_FILTER;
    
    readonly column INSDC:quality:phred QUALITY =
        NCBI:SRA:syn_quality #1 < 30, 3 > ( out_read_len, out_spot_filter );
};

''' 

def make_rows( cur, cols, row_count, read_len_data, filter_data ) :
    for idx in xrange( 0, row_count ) :
        cur.OpenRow()
        cols[ "READ_LEN" ].write( read_len_data )
        cols[ "SPOT_FILTER" ].write( filter_data )
        cur.CommitRow()
        cur.CloseRow()
    cur.Commit()

def make_table( mgr, schema_txt, schema_table_name, table_name ) :
    try :
        schema = mgr.MakeSchema( schema_txt )
        tbl = mgr.CreateTable( schema, schema_table_name, table_name )
        cur = tbl.CreateCursor( OpenMode.Write )
        cols = cur.OpenColumns( [ "READ_LEN", "SPOT_FILTER" ] )
        row_count = 1
        make_rows( cur, cols, row_count, [ 9, 8, 7 ], [ 0 ] )
        make_rows( cur, cols, row_count, [ 8, 7, 6 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 7, 6, 5 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 6, 5, 4 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 5, 4, 3 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 4, 3, 2 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 3, 2, 1 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 1, 2, 3 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 2, 3, 4 ], [ 0 ] )
        make_rows( cur, cols, row_count, [ 3, 4, 5 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 4, 5, 6 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 6, 7, 8 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 7, 8, 9 ], [ 0 ] )
        make_rows( cur, cols, row_count, [ 8, 9, 9 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 9, 9, 9 ], [] )

        make_rows( cur, cols, row_count, [ 9, 8 ], [ 0 ] )
        make_rows( cur, cols, row_count, [ 8, 7 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 7, 6 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 6, 5 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 5, 4 ], [ 0 ] )
        make_rows( cur, cols, row_count, [ 4, 3 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 3, 2 ], [] )

        make_rows( cur, cols, row_count, [ 9 ], [ 0 ] )
        make_rows( cur, cols, row_count, [ 8 ], [ 1 ] )
        make_rows( cur, cols, row_count, [ 7 ], [] )

        make_rows( cur, cols, row_count, [], [ 0 ] )
        make_rows( cur, cols, row_count, [], [ 1 ] )
        make_rows( cur, cols, row_count, [], [] )
        
    except vdb_error as e :
        print( e )

        
if __name__ == '__main__' :

    argc = len( sys.argv )
    
    schema_table_name = "TEST_TBL"
    table_name = "T_TEST_SYN_QUAL"
    lib_name = None
    
    if argc > 1 : lib_wr = sys.argv[ 1 ]
    if argc > 2 : table_name = sys.argv[ 2 ]

    if ( lib_wr != None ) and ( os.path.isfile( lib_wr ) ) :
        try :
            mgr = manager( OpenMode.Write, lib_wr )
            make_table( mgr, schematxt, schema_table_name, table_name )
            #mgr.OpenTable( table_name ).print_rows()

        except vdb_error as e :
            print( e )
