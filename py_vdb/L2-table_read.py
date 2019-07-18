#!/usr/bin/env python

from vdb import *

def get_one_value_v1( mgr, acc, column_name, row_id ) :
    try :
        tbl = mgr.OpenTable( acc )
        cur = tbl.CreateCursor()
        col = cur.OpenColumns( column_name )
        return col.Read( row_id )
    except vdb_error as e :
        print( e )

def get_one_value_v2( mgr, acc, column_name, row_id ) :
    try :
        return mgr.OpenTable( acc ).CreateCursor().OpenColumns( column_name ).Read( row_id )
    except vdb_error as e :
        print( e )

def get_readable_columns( mgr, acc ) :
    try :
        tbl = mgr.OpenTable( acc )
        return tbl.ListCol()
    except vdb_error as e :
        print( e )

def print_column_infos( mgr, acc, column_names ) :
    try :
        tbl = mgr.OpenTable( acc )
        cur = tbl.CreateCursor()
        cols = cur.OpenColumns( column_names )
        try :
            v = cols.itervalues()
        except AttributeError :
            v = cols.values()
        for c in v :
            print( "%s\t %s"%( c.name, c.row_range() ) )
            print( "domain: %s, bits: %d, dim : %d"%( c.domain(), c.bits(), c.dim() ) )
    except vdb_error as e :
        print( e )

def inspect_column_values( mgr, acc, column_names, row_id ) :
    try :
        tbl = mgr.OpenTable( acc )
        cur = tbl.CreateCursor()
        cols = cur.OpenColumns( column_names )
        
        #retrieve a cell-value as an array of Uint32-values ( in this case a single value )
        spot_id = cols[ "SPOT_ID" ].Read( row_id )
        print( "SPOT_ID[1] = %s"%( spot_id ) )
        
        #retrieve a cell-value as a string
        read = cols[ "READ" ].Read( row_id )
        print( "READ[1] = %s"%( read ) )
        
        #retrieve a cell-value as an array of Uint8-values
        q = cols[ "QUALITY" ].Read( row_id )
        print( "QUALITY[1] = %s"%( q ) )
        print( "avg of quality values = %f"%( sum( q ) / float( len( q ) ) ) )

    except vdb_error as e :
        print( e )

if __name__ == '__main__' :
    
    #open a manager in read-mode
    mgr = manager()
    
    #a long and a short way of reading a value from a table
    print( get_one_value_v1( mgr, "SRR000002", "READ", 2000 ) )
    print( get_one_value_v2( mgr, "SRR000002", "READ", 2000 ) )
    
    #print readable columns
    print( get_readable_columns( mgr, "SRR000001" ) )

    #print details about some columns
    print_column_infos( mgr, "SRR000001", [ "SPOT_ID", "READ", "QUALITY" ] )

    #open multiple columns at the same time
    inspect_column_values( mgr, "SRR000001", [ "SPOT_ID", "READ", "QUALITY" ], 1 )
