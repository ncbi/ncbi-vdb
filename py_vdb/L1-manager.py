#!/usr/bin/env python3

from vdb import *

acc = "SRR000001"

if __name__ == '__main__' :

    try :
        # if the lib is omitted, the manager is looking for a lib itself
        # based on platform/OpenMode in the current directory first, then in $(HOME)/.ncbi/lib64
        # if no lib can be found or the lib cannot be loaded, then the constructor throws an exception
        mgr = manager()

        print( f"mgr.vers = {mgr.Version()}" )
        print( f"mgr.PathType( {acc} ) = {mgr.PathType( acc )}" )
#        print( f"mgr.GetObjVersion( {acc} ) = {mgr.GetObjVersion( acc )}" )
#        print( f"mgr.GetObjModDate( {acc} ) = {mgr.GetObjModDate( acc )}" )

        repo_mgr = mgr.MakeKConfig().MakeRepositoryMgr()
        print( "repo_mgr.HasRemoteAccess ... %r"%( repo_mgr.HasRemoteAccess() ) )
        try :
            for repo_list in repo_mgr.AllRepos() :
                for repo in repo_list :
                    print( repo )
        except :
            pass

    except vdb_error as e :
        print( e )
