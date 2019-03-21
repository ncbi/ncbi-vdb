/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

/**
* This file is to be #included to read and write side test suites
* KDB_MANAGER_MAKE has to be #defined depending on the side
*/

#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <kdb/kdb-priv.h>

#include <kfs/file.h>
#include <vfs/manager.h>
#include <vfs/manager-priv.h> /* VFSManagerMakeFromKfg */
#include <vfs/path.h>
#include <vfs/resolver.h>
#include <klib/text.h>
#include <kfg/config.h>

using namespace std;

static string ToString(const VPath* path)
{
    const String * s;
    if ( VPathMakeString (path, &s) != 0 )
        throw logic_error ( "ToString(VPath) failed" );
    string ret = string(s->addr, s->size);
    free((void*)s);
    return ret;
}

TEST_CASE(KDBManagerVPathOpenDB_Local)
{
    VPath * path;
    
    {
        VFSManager* vfsmgr;
        REQUIRE_RC(VFSManagerMake(&vfsmgr));
        REQUIRE_RC(VFSManagerMakePath(vfsmgr, &path, "./testdb"));
        REQUIRE_RC(VFSManagerRelease(vfsmgr));
    }

    {
        KDBManager* mgr;
        REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
        const KDatabase * db;
        REQUIRE_RC(KDBManagerVPathOpenLocalDBRead ( mgr, &db, path ));
        REQUIRE_NOT_NULL(db);
        const KTable *tbl;
        REQUIRE_RC(KDatabaseOpenTableRead(db, &tbl, "SEQUENCE"));
        REQUIRE_RC(KTableRelease(tbl));
        REQUIRE_RC(KDatabaseRelease(db));
        REQUIRE_RC(KDBManagerRelease(mgr));
    }
    
    REQUIRE_RC(VPathRelease(path));
}

TEST_CASE(KDBManagerVPathOpenDB_Local_Table)
{
    VPath * path;
    
    {
        VFSManager* vfsmgr;
        REQUIRE_RC(VFSManagerMake(&vfsmgr));
        REQUIRE_RC(VFSManagerMakePath(vfsmgr, &path, "SRR000123"));
        REQUIRE_RC(VFSManagerRelease(vfsmgr));
    }

    {
        KDBManager* mgr;
        REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
        const KDatabase * db;
        REQUIRE_RC_FAIL(KDBManagerVPathOpenLocalDBRead ( mgr, &db, path )); // not a database
        REQUIRE_RC(KDBManagerRelease(mgr));
    }
    
    REQUIRE_RC(VPathRelease(path));
}

TEST_CASE(KDBManagerVPathOpenDB_Local_BadPath)
{
    VPath * path;
    
    {
        VFSManager* vfsmgr;
        REQUIRE_RC(VFSManagerMake(&vfsmgr));
        REQUIRE_RC(VFSManagerMakePath(vfsmgr, &path, "xxxxx"));
        REQUIRE_RC(VFSManagerRelease(vfsmgr));
    }

    {
        KDBManager* mgr;
        REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
        const KDatabase * db;
        REQUIRE_RC_FAIL(KDBManagerVPathOpenLocalDBRead ( mgr, &db, path )); // not a database
        REQUIRE_RC(KDBManagerRelease(mgr));
    }
    
    REQUIRE_RC(VPathRelease(path));
}

class RemoteDBFixture
{
public:
    RemoteDBFixture()
    : m_vfsmgr(0), m_path(0), m_cache(0)
    {
    }
    
    ~RemoteDBFixture()
    {
        if (m_vfsmgr && VFSManagerRelease(m_vfsmgr) != 0)
            throw logic_error ( "~RemoteDBFixture: VFSManagerRelease failed" );
            
        if (m_path && VPathRelease(m_path) != 0)
            throw logic_error ( "~RemoteDBFixture: VPathRelease(m_path) failed" );
            
        if (m_cache && VPathRelease(m_cache) != 0)
            throw logic_error ( "~RemoteDBFixture: VPathRelease(m_path) failed" );
            
        remove("./root/sra");
        remove("./root");
        remove(m_configName.c_str());
        remove(m_cachedFile.c_str());
    }
    
    void Configure(const string& name)
    {
        m_configName = name+".kfg";
        std::ofstream f(m_configName.c_str()); 
        f   << "repository/remote/main/CGI/resolver-cgi "
                  "= \"https://trace.ncbi.nlm.nih.gov/Traces/names/names.fcgi\"\n"
            << "repository/user/main/public/root=\"./root\"\n" 
            << "repository/user/main/public/apps/sra/volumes/sraFlat=\"sra\"\n";
        f.close();
            
        KDirectory *wd;
        if (KDirectoryNativeDir(&wd))
            throw logic_error ( "RemoteDBFixture::Configure: KDirectoryNativeDir failed" );
        
        KConfig *cfg;
        if (KConfigMake(&cfg, wd))
            throw logic_error ( "RemoteDBFixture::Configure: KConfigMake failed" );

        if (VFSManagerMakeFromKfg(&m_vfsmgr, cfg))
            throw logic_error ( "RemoteDBFixture::Configure: VFSManagerMakeFromKfg failed" );
        
        if (KConfigRelease(cfg))
            throw logic_error ( "RemoteDBFixture::Configure: KConfigRelease failed" );
            
        if (KDirectoryRelease(wd))
            throw logic_error ( "RemoteDBFixture::Configure: KDirectoryRelease failed" );
    }
    
    void Resolve(const string& p_accession)
    {
        struct VResolver * resolver;
        if(VFSManagerGetResolver(m_vfsmgr, & resolver))
            throw logic_error ( "RemoteDBFixture::Resolve: VFSManagerGetResolver failed" );

        VPath * accession;
        if (VFSManagerMakePath ( m_vfsmgr, &accession, p_accession.c_str() ))
            throw logic_error ( "RemoteDBFixture::Resolve: VFSManagerMakePath failed" );
        
        if (VResolverQuery( resolver, 0, accession, NULL, &m_path, &m_cache))
            throw logic_error ( "RemoteDBFixture::Resolve: VResolverQuery failed" );
        
        //cout << ToString(m_path) << endl; 
        //cout << ToString(m_cache) << endl; 
        m_cachedFile = ToString(m_cache);
        /* make sure cached file does not exist */
        remove(m_cachedFile.c_str());
       
        if (VPathRelease(accession))
            throw logic_error ( "RemoteDBFixture::Resolve: VPathRelease failed" );
        if (VResolverRelease(resolver))
            throw logic_error ( "RemoteDBFixture::Resolve: VResolverRelease failed" );
    }

    
protected:
    string m_configName;
    
    VFSManager * m_vfsmgr;
    const VPath * m_path;
    const VPath * m_cache;
    string m_cachedFile;
};

FIXTURE_TEST_CASE(KDBManagerVPathOpenDB_Remote, RemoteDBFixture)
{
    Configure(GetName());
    Resolve("SRR600096");
    
    KDBManager* mgr;
    REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
    const KDatabase * db;
    REQUIRE_RC(KDBManagerVPathOpenRemoteDBRead ( mgr, &db, m_path, m_cache ));
    REQUIRE_NOT_NULL(db);
    
    /* read something */
    const KTable *tbl;
    REQUIRE_RC(KDatabaseOpenTableRead(db, &tbl, "SEQUENCE"));
    REQUIRE_RC(KTableRelease(tbl));

    REQUIRE_RC(KDatabaseRelease(db));
    REQUIRE_RC(KDBManagerRelease(mgr));
}

FIXTURE_TEST_CASE(KDBManagerVPathOpenDB_Remote_NoCache, RemoteDBFixture)
{
    Configure(GetName());
    Resolve("SRR600096");
    
    KDBManager* mgr;
    REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
    const KDatabase * db;
    REQUIRE_RC(KDBManagerVPathOpenRemoteDBRead ( mgr, &db, m_path, NULL ));
    REQUIRE_NOT_NULL(db);
    
    /* read something */
    const KTable *tbl;
    REQUIRE_RC(KDatabaseOpenTableRead(db, &tbl, "SEQUENCE"));
    REQUIRE_RC(KTableRelease(tbl));

    REQUIRE_RC(KDatabaseRelease(db));
    REQUIRE_RC(KDBManagerRelease(mgr));
}

FIXTURE_TEST_CASE(KDBManagerVPathOpenDB_Remote_Table, RemoteDBFixture)
{
    Configure(GetName());
    Resolve("SRR000123");
    
    KDBManager* mgr;
    REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
    const KDatabase * db;
    REQUIRE_RC_FAIL(KDBManagerVPathOpenRemoteDBRead ( mgr, &db, m_path, NULL )); // not a database
    
    REQUIRE_RC(KDBManagerRelease(mgr));
}

FIXTURE_TEST_CASE(KDBManagerVPathOpenDB_Remote_BadPath, RemoteDBFixture)
{
    Configure(GetName());
    
    KDBManager* mgr;
    REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
    const KDatabase * db;

    REQUIRE_RC(VFSManagerMakePath(m_vfsmgr, (VPath**)&m_path, "xxxx"));
    REQUIRE_RC_FAIL(KDBManagerVPathOpenRemoteDBRead ( mgr, &db, m_path, NULL )); 
    
    REQUIRE_RC(KDBManagerRelease(mgr));
}

#if 0
FIXTURE_TEST_CASE(KDBManagerVPathOpenDB_Remote_BadCache, RemoteDBFixture)
{
    Configure(GetName());
    Resolve("SRR600096");
    
    KDBManager* mgr;
    REQUIRE_RC(KDB_MANAGER_MAKE(&mgr, NULL));
    const KDatabase * db;
    
    VPath* cache;
    REQUIRE_RC(VFSManagerMakePath(m_vfsmgr, &cache, "/dev/null")); // unlikely to be usable
    REQUIRE_RC(KDBManagerVPathOpenRemoteDBRead ( mgr, &db, m_path, cache )); 
    REQUIRE_RC(VPathRelease(cache));

    /* read something */
    const KTable *tbl;
    REQUIRE_RC(KDatabaseOpenTableRead(db, &tbl, "SEQUENCE")); // works anyway, just not caching
    REQUIRE_RC(KTableRelease(tbl));
    
    REQUIRE_RC(KDatabaseRelease(db));
    REQUIRE_RC(KDBManagerRelease(mgr));
}
#endif
