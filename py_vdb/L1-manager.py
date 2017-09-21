#!/usr/bin/env python

from vdb import *

lib_262_rd = "./2.6.2/libncbi-vdb.so.2.6.2"
lib_262_wr = "./2.6.2/libncbi-wvdb.so.2.6.2"

lib_263_rd = "./2.6.3/libncbi-vdb.so.2.6.3"
lib_263_wr = "./2.6.3/libncbi-wvdb.so.2.6.3"

lib_270_rd = "./2.7.0/libncbi-vdb.so.2.7.0"
lib_270_wr = "./2.7.0/libncbi-wvdb.so.2.7.0"

lib_280_rd = "./2.8.0/libncbi-vdb.so.2.8.0"
lib_280_wr = "./2.8.0/libncbi-wvdb.so.2.8.0"

acc = "SRR000001"

if __name__ == '__main__' :

    rd_libs = [ lib_262_rd, lib_263_rd, lib_270_rd, lib_280_rd ]
    wr_libs = [ lib_262_wr, lib_263_wr, lib_270_wr, lib_280_wr ]
    
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

    try :
        # if the lib is omitted, the manager is looking for a lib itself
        # based on platform/OpenMode in the current directory first, then in $(HOME)/.ncbi/lib64
        # if no lib can be found or the lib cannot be loaded, then the constructor throws an exception
        mgr = manager()

        mv = mgr.Version()
        pt = mgr.PathType( acc )
        ov = mgr.GetObjVersion( acc )
        ot = mgr.GetObjModDate( acc )
        print( "mgr.vers\t%s\t%s\t%s\t Version %s\tObjModDate %s"%( mv, acc, pt, ov, ot ) )

        repo_mgr = mgr.MakeKConfig().MakeRepositoryMgr()
        print( "repo_mgr.HasRemoteAccess ... %r"%( repo_mgr.HasRemoteAccess() ) )
        for repo_list in repo_mgr.AllRepos() :
            for repo in repo_list :
                print( repo )
        
    except vdb_error as e :
        print( e )
