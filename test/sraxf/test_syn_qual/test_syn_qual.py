#!/usr/bin/env python

from vdb import *

lib_wr = "./2.9.2/libncbi-wvdb.so.2.9.2"

schematxt = '''
version 1;

include 'insdc/insdc.vschema';

extern function
INSDC:quality:phred NCBI:SRA:syn_quality #1
    < INSDC:quality:phred good_quality, INSDC:quality:phred bad_quality >
    ( INSDC:coord:len read_len, INSDC:SRA:read_filter read_filter );

table TEST_TBL #1.0
{
    column INSDC:coord:len READ_LEN;
    column INSDC:SRA:read_filter READ_FILTER;
    
    INSDC:coord:len out_read_len = .READ_LEN;
    INSDC:SRA:read_filter out_read_filter = .READ_FILTER;
    
    readonly column INSDC:quality:phred QUALITY =
        NCBI:SRA:syn_quality #1 < 30, 3 > ( out_read_len, out_read_filter );
};

''' 

def make_table( mgr, schema_txt, schema_table_name, table_name, row_count ) :
    try :
        schema = mgr.MakeSchema( schema_txt )
        tbl = mgr.CreateTable( schema, schema_table_name, table_name )
        cur = tbl.CreateCursor( OpenMode.Write )
        cols = cur.OpenColumns( [ "READ_LEN", "READ_FILTER" ] )
        for idx in xrange( 0, row_count ) :
            cur.OpenRow()
            cols[ "READ_LEN" ].write( random_data( 3, 5, 10 ) )
            cols[ "READ_FILTER" ].write( [ 0, 1 ] )
            cur.CommitRow()
            cur.CloseRow()
        cur.Commit()
    except vdb_error as e :
        print( e )

        
if __name__ == '__main__' :
    
    schema_table_name = "TEST_TBL"
    table_name = "T_TEST_SYN_QUAL"
    row_count = 10
    
    try :
        #open a manager
        mgr = manager( OpenMode.Write, lib_wr )
        make_table( mgr, schematxt, schema_table_name, table_name, row_count )
        mgr.OpenTable( table_name ).print_rows()

    except vdb_error as e :
        print( e )
