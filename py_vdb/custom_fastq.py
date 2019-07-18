#!/usr/bin/env python

from vdb import *


if __name__ == '__main__':
    lib_262_r = "./2.8.0/libncbi-vdb.so.2.8.0"
    tablename = "SRR000001"
    
    try :
        mgr = manager( OpenMode.Read, lib_262_r )
        tab = mgr.open_tab( tablename )
        cur = tab.make_cursor( OpenMode.Read )
        cols = cur.open( [ "NAME", "READ", "(INSDC:quality:text:phred_33)QUALITY" ] )
        c_name = cols[ "NAME" ]
        c_read = cols[ "READ" ]
        c_qual = cols[ "(INSDC:quality:text:phred_33)QUALITY" ]
        for row in xrange( 1, 11 ) :
            name  = c_name.read( row )
            read  = c_read.read( row )
            print "@%s.%d %s lenght=%d"%( tablename, row, name, len( read ) )
            print read
            print "+%s.%d %s lenght=%d"%( tablename, row, name, len( read ) )
            print c_qual.read( row )
            
    except vdb_error, e :
        print e
