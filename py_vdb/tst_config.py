#!/usr/bin/env python

from vdb import *

def print_repolist( repos ) :
    for r in repos :
        name = r.name()
        print "%s.displayname = %s"%( name, r.display_name() )
        print "%s.category = %s"%( name, RepoCat2String( r.category() ) )
        print "%s.subcategory = %s"%( name, RepoSubCat2String( r.sub_category() ) )
        print "%s.root = %s"%( name, r.get_root() )
        print "%s.resolver = %s"%( name, r.resolver() )
        print "%s.disabled = %s"%( name, r.is_disabled() )
        print "%s.cache enabled = %s"%( name, r.cache_enabled() )
        print "-"

def report_repos( rm ) :
    print "user repos:"
    print_repolist( rm.user_repos() )
    print "site repos:"
    print_repolist( rm.site_repos() )
    print "remote repos:"
    print_repolist( rm.remote_repos() )

if __name__ == '__main__':
    lib_263_rd = "./2.6.3/libncbi-vdb.so.2.6.3"
    lib_262_rd = "./2.6.2/libncbi-vdb.so.2.6.2"
    lib_rd = "/home/raetzw/.ncbi/lib64/libncbi-vdb.so"
    
    mgr = manager( OpenMode.Read, lib_262_rd )
    cfg = mgr.make_config()

    rm = cfg.make_repo_mgr()
    print "has remote access = %s"%( rm.has_remote_access() )

    #report_repos( rm )
    
    print "user.disabled = %s"%( rm.cat_disabled( RepoCat.User ) )
    print "site.disabled = %s"%( rm.cat_disabled( RepoCat.Site ) )
    print "remote.disabled = %s"%( rm.cat_disabled( RepoCat.Remote ) )
    
    vfs_mgr = mgr.make_vfs_mgr()
    resolv = vfs_mgr.make_resolver( cfg )
    #resolv. remote_enable( ResolvEnable.AlwaysEnable )

    print "local  = %s"%( resolv.query_local( vfs_mgr.make_path( "ncbi-acc:SRR001000" ) ) )
    print "remote = %s"%( resolv.query_remote( vfs_mgr.make_path( "ncbi-acc:SRR001000" ) ) )
    print "cache  = %s"%( resolv.query_cache( vfs_mgr.make_path( "ncbi-acc:SRR001000" ) ) )    
