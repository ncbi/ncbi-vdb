#!/usr/bin/env python

from vdb import *

#these libs are not in the repository, they have to be downloaded an put in place
#by the user...

lib_262_rd = "./2.6.2/libncbi-vdb.so.2.6.2"
lib_262_wr = "./2.6.2/libncbi-wvdb.so.2.6.2"

lib_263_rd = "./2.6.3/libncbi-vdb.so.2.6.3"
lib_263_wr = "./2.6.3/libncbi-wvdb.so.2.6.3"

lib_270_rd = "./2.7.0/libncbi-vdb.so.2.7.0"
lib_270_wr = "./2.7.0/libncbi-wvdb.so.2.7.0"

lib_280_rd = "./2.8.0/libncbi-vdb.so.2.8.0"
lib_280_wr = "./2.8.0/libncbi-wvdb.so.2.8.0"

lib_301_rd = "./3.0.1/libncbi-vdb.so.3.0.1"
lib_301_wr = "./3.0.1/libncbi-wvdb.so.3.0.1"

acc = "SRR000001"

if __name__ == '__main__' :

    rd_libs = [ lib_262_rd, lib_263_rd, lib_270_rd, lib_280_rd, lib_301_rd ]
    wr_libs = [ lib_262_wr, lib_263_wr, lib_270_wr, lib_280_wr, lib_301_wr ]

    for lib in rd_libs :
        try :
            mgr = manager( OpenMode.Read, lib )
            print( "%s\tmgr.Version() = %s\tmgr.writable() = %r"%( lib, mgr.Version(), mgr.writable() ) )
        except vdb_error as e :
            print( e )

    for lib in wr_libs :
        try :
            mgr = manager( OpenMode.Write, lib )
            print( "%s\tmgr.Version() = %s\tmgr.writable() = %r"%( lib, mgr.Version(), mgr.writable() ) )
        except vdb_error as e :
            print( e )
