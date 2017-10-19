#!/usr/bin/env python

from vdb import *

lib_rd = './2.8.0/libncbi-vdb.so.2.8.0'

# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
        
if __name__ == '__main__' :
    
    try :
        #open a manager in read-mode
        mgr = manager( OpenMode.Read, lib_rd )
        
        #open accession ( a table-accession )
        tbl = mgr.OpenTable( 'SRR942391' )
       
        #open meta-data on table
        meta = tbl.OpenMetadata()
        
        print( "version = %s"%meta.Version() )
        print( "byte-order reversed = %r"%meta.ByteOrder() )
        print( "revision = %d"%meta.Revision() )
        print( "max-revision = %d"%meta.MaxRevision() )
        
        #open the root
        root_node = meta.OpenNode( '/' )
        child_list = root_node.ListChildren()
        print( "sub-nodes: %s"%child_list )
        for node_name in child_list :
            node = root_node.OpenNode( node_name )
            print ( "%s.byte-order reversed = %s"%( node_name, node.ByteOrder() ) )
            print ( "%s.size = %d"%( node_name, node.size() ) )
            print ( "%s.data = %s"%( node_name, node.as_string( 41 ) ) )
            print ( "%s.data (uint8)  = %s"%( node_name, node.as_uint8( 41 ) ) )
            print ( "%s.data (int8)   = %s"%( node_name, node.as_int8( 41 ) ) )
            print ( "%s.data (uint16) = %s"%( node_name, node.as_uint16( 41 ) ) )
            print ( "%s.data (int16)  = %s"%( node_name, node.as_int16( 41 ) ) )
            print ( "%s.data (uint32) = %s"%( node_name, node.as_uint32( 41 ) ) )
            print ( "%s.data (int32)  = %s"%( node_name, node.as_int32( 41 ) ) )
            print ( "%s.data (uint64) = %s"%( node_name, node.as_uint64( 41 ) ) )
            print ( "%s.data (int64)  = %s"%( node_name, node.as_int64( 41 ) ) )

    except vdb_error as e :
        print( e )
