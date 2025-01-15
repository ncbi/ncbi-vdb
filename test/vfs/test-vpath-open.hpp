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
* ==============================================================================
*/

#include <kapp/args.h> /* ArgsMakeAndHandle */
#include <kdb/kdb-priv.h> /* KDBManagerMakeReadWithVFSManager */
#include <kdb/database.h> /* KDBManagerOpenDBRead */
#include <kdb/manager.h> /* KDBManager */
#include <kdb/table.h> /* KDBManagerOpenTableReadVPath */
#include <kfg/config.h> /* KConfigRelease */
#include <kfs/directory.h> /* KDirectoryRelease */
#include <klib/debug.h> /* KDbgSetString */
#include <klib/text.h> /* String */
#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h> /* VDBManager */
#include <vdb/table.h> /* VTable */
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path-priv.h> /* VPathGetDirectory */
#include <vfs/resolver.h> /* VRevolverRelease */
#include <ktst/unit_test.hpp> /* KMain */
#include "../../libs/vfs/path-priv.h" /* VPathSetDirectory */
#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

using std::cerr;
using std::string;

#define ALL

#define KDBManagerOpenDB_FUNC              KDBManagerOpenDBRead
#define KDBManagerOpenDB_FUNC_VPath        KDBManagerOpenDBReadVPath
#define KDBManagerOpenTable_FUNC_VPath     KDBManagerOpenTableReadVPath
#define VDBManagerOpenDB_FUNC_VPath    VDBManagerOpenDBReadVPath
#define VDBManagerOpenTable_FUNC_VPath VDBManagerOpenTableReadVPath

extern "C" { rc_t LegacyVPathMake(VPath ** new_path, const char * posix_path); }

static rc_t argsHandler(int argc, char* argv[])
{	return ArgsMakeAndHandle(NULL, argc, argv, 0, NULL, 0); }

TEST_SUITE_WITH_ARGS_HANDLER(TestVpathOpenSuite, argsHandler)

static const char AccTPath[]("data/SRR053325.sra");     // path to Tbl
static const char AccTR   []("SRR053325");              // remote Tbl
static const char AccDR   []("SRR600096");              // remote DB
static const char AccDL   []("SRR599578");              // local DB accession
static const char AccDPath[]("data/SRR600096.sralite"); // path to DB
static const char AccTL   []("SRR045450");              // local table accession

struct Common {
    VKONST VDBManager * vdb;
    VFSManager * vfs;
    KKONST KDBManager * kdb;
    string dbLPath;    // path to local DB
    string dbLAccPath; // local DB accession
    string dbRPath;    // remote DB accession
    string tbLPath;    // path to local Tbl
    string tbLAccPath; // local Tbl accession
    string tbRPath;    // remote Tbl accession

    Common()
        : vdb(0), vfs(0), kdb(0)
    {
        /* Keep SDL caching - some tests rely on that.
        putenv((char*)"NCBI_VDB_NO_CACHE_SDL_RESPONSE=1"); */

        KConfigDisableUserSettings();

        rc_t rc(VDBManagerMake_FUNC(&vdb, 0));
        if (rc != 0)
            throw rc;

        rc = VFSManagerMake(&vfs);
        if (rc != 0)
            throw rc;

        rc = KDBManagerMake_FUNC_WithVFSManager(&kdb, 0, vfs);
        if (rc != 0)
            throw rc;

        KDirectory * d(0);
        rc = KDirectoryNativeDir(&d);
        if (rc != 0)
            throw rc;

        char c[PATH_MAX] = "";

        rc = KDirectoryResolvePath(d, true, c, sizeof c, AccTPath);
        if (rc != 0)
            throw rc;
        tbLPath = c;

        rc = KDirectoryResolvePath(d, true, c, sizeof c, AccDPath);
        if (rc != 0)
            throw rc;
        dbLPath = c;

        rc = KDirectoryRelease(d);
        if (rc != 0)
            throw rc;

        KConfig * k(0);
        rc = KConfigMake(&k, 0);
        if (rc != 0)
            throw rc;

        VResolver * r(0);
        rc = VFSManagerMakeResolver(vfs, &r, k);
        if (rc != 0)
            throw rc;

        rc = KConfigRelease(k);
        if (rc != 0)
            throw rc;

        VPath * pathR(0);
        const VPath * pc(0);
        const VPath * pc2(0);
        String s;

        rc = LegacyVPathMake(&pathR, AccTR);
        if (rc != 0)
            throw rc;
        rc = VResolverQuery(r, 0, pathR, NULL, &pc, NULL);
        if (rc != 0)
            throw rc;
        int notequal(-1);
        rc = VFSManagerResolveAll(vfs, AccTR, NULL, &pc2, NULL);
        if (rc != 0)
            throw rc;
        rc = VPathEqual(pc, pc2, &notequal);
        if (rc != 0)
            throw rc;
        if (notequal)
            throw notequal;
        rc = VPathRelease(pc2);
        if (rc != 0)
            throw rc;

        rc = VPathRelease(pathR);
        if (rc != 0)
            throw rc;
        rc = VPathGetPath(pc, &s);
        if (rc != 0)
            throw rc;
        tbRPath = s.addr;
        rc = VPathRelease(pc);
        if (rc != 0)
            throw rc;

        rc = LegacyVPathMake(&pathR, AccDR);
        if (rc != 0)
            throw rc;
        rc = VResolverQuery(r, 0, pathR, NULL, &pc, NULL);
        if (rc != 0)
            throw rc;
        rc = VFSManagerResolveAll(vfs, AccDR, NULL, &pc2, NULL);
        if (rc != 0)
            throw rc;
        rc = VPathEqual(pc, pc2, &notequal);
        if (rc != 0)
            throw rc;
        if (notequal)
            throw notequal;
        rc = VPathRelease(pc2);
        if (rc != 0)
            throw rc;
        rc = VPathRelease(pathR);
        if (rc != 0)
            throw rc;
        rc = VPathGetPath(pc, &s);
        if (rc != 0)
            throw rc;
        dbRPath = s.addr;
        rc = VPathRelease(pc);
        if (rc != 0)
            throw rc;

        rc = LegacyVPathMake(&pathR, AccDL);
        if (rc != 0)
            throw rc;
        rc = VResolverQuery(r, 0, pathR, &pc, NULL, NULL);
        if (rc != 0)
            throw rc;
        rc = VFSManagerResolveLocal(vfs, AccDL, &pc2);
        if (rc != 0)
            throw rc;
        rc = VPathEqual(pc, pc2, &notequal);
        if (rc != 0)
            throw rc;
        if (notequal)
            throw notequal;
        rc = VPathRelease(pc2);
        if (rc != 0)
            throw rc;
        rc = VPathRelease(pathR);
        if (rc != 0)
            throw rc;
        rc = VPathGetPath(pc, &s);
        if (rc != 0)
            throw rc;
        dbLAccPath = s.addr;
        rc = VPathRelease(pc);
        if (rc != 0)
            throw rc;

        rc = LegacyVPathMake(&pathR, AccTL);
        if (rc != 0)
            throw rc;
        rc = VResolverQuery(r, 0, pathR, &pc, NULL, NULL);
        if (rc != 0)
            throw rc;
        rc = VFSManagerResolveLocal(vfs, AccTL, &pc2);
        if (rc != 0)
            throw rc;
        rc = VPathEqual(pc, pc2, &notequal);
        if (rc != 0)
            throw rc;
        if (notequal)
            throw notequal;
        rc = VPathRelease(pc2);
        if (rc != 0)
            throw rc;
        rc = VPathRelease(pathR);
        if (rc != 0)
            throw rc;
        rc = VPathGetPath(pc, &s);
        if (rc != 0)
            throw rc;
        tbLAccPath = s.addr;
        rc = VPathRelease(pc);
        if (rc != 0)
            throw rc;

        rc = VResolverRelease(r);
        if (rc != 0)
            throw rc;
    }

    ~Common() {
        rc_t rc(KDBManagerRelease(kdb));
        if (rc != 0)
            cerr << "failed to KDBManagerRelease\n";

        rc = VFSManagerRelease(vfs);
        if (rc != 0)
            cerr << "failed to VFSManagerRelease\n";

        rc = VDBManagerRelease(vdb);
        if (rc != 0)
            cerr << "failed to VDBManagerRelease\n";
    }
};

static const Common C;

class Fixture {
protected:
    const string & tbLPath;    // AccTPath - path to Tbl
    const string & tbRPath;    // AccTR    - remote Tbl
    const string & dbRPath;    // AccDR    - remote DB
    const string & dbLAccPath; // AccDL    - local DB accession
    const string & dbLPath;    // AccDPath - path to DB
    const string & tbLAccPath; // AccTL    - local Tbl accession

    const VDBManager * vdb;
    VFSManager * vfs;
    const KDBManager * kdb;

    char buf[PATH_MAX];

    Fixture() : tbLPath(C.tbLPath), tbRPath(C.tbRPath), dbRPath(C.dbRPath)
        , dbLAccPath(C.dbLAccPath), dbLPath(C.dbLPath), tbLAccPath(C.tbLAccPath)
        , vdb(C.vdb), vfs(C.vfs), kdb(C.kdb) {}

    rc_t Compare(const VPath * path, const string & dirPath,
        const KDirectory ** a = NULL)
    {
        const KDirectory * d(0);
        rc_t rc(VPathGetDirectory(path, &d));
        if (rc != 0)
            return rc;
        rc = KDirectoryResolvePath(d, true, buf, sizeof buf, ".");
        if (rc != 0)
            return rc;
        if (string(buf) != dirPath)
            return 1;

        if (a == NULL)
            return KDirectoryRelease(d);

        *a = d;
        return rc;
    }
};

////////////////////////////////////////////////////////////////////////////////
#ifdef ALL
FIXTURE_TEST_CASE(TestKDBManagerOpenDB_FUNC, Fixture) {
    const KDatabase * kd(0);
    REQUIRE_RC(KDBManagerOpenDB_FUNC(kdb, &kd, AccDR));
    REQUIRE_RC(KDatabaseRelease(kd));

    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, 0));

    // try to open sra file table as DB
    VPath * pathL(0);
    REQUIRE_RC(LegacyVPathMake(&pathL, AccTPath));
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathL));
    const KDirectory * dirLoc(0);
    REQUIRE_RC(Compare(pathL, tbLPath, &dirLoc));

    // try to open sra file table as DB: cached dir
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath));

    // try to open sra db as DB
    // 1. KDB/VDB: Remote DB -> Local Table
    VPath * pathDb(0);
    // 1.1. Create Remote DB Path
    REQUIRE_RC(LegacyVPathMake(&pathDb, AccDR));

    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(0, &kd, pathDb));
    const KDirectory * dirDb(0);
    REQUIRE_RC(VPathGetDirectory(pathDb, &dirDb));
    REQUIRE_NULL(dirDb);

    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, 0, pathDb));
    REQUIRE_RC(VPathGetDirectory(pathDb, &dirDb));
    REQUIRE_NULL(dirDb);

    // 1.2. KDBManagerOpenDB_FUNC_VPath(Remote DB Path) Succeeds
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathDb));
    REQUIRE_RC(Compare(pathDb, dbRPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    // try to open Remote DB Path as DB: cached dir
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathDb));
    REQUIRE_RC(Compare(pathDb, dbRPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    VPath * path(0);

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDL));

    // try to open local DB accession as DB
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, dbLAccPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    // try to open local DB accession as DB: cached dir
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, dbLAccPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDPath));

    // try to open path to local DB as DB
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, dbLPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    // try to open path to local DB as DB: cached dir
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, dbLPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTR));

    // try to open remote table accession as DB
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, tbRPath));

    // try to open remote table accession as DB: cached dir
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, tbRPath));
    REQUIRE_RC(KDatabaseRelease(kd));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTL));

    // try to open local table accession as DB
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, tbLAccPath));

    // try to open local table accession as DB: cached dir
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, path));
    REQUIRE_RC(Compare(path, tbLAccPath));

    REQUIRE_RC(VPathRelease(path));

    // 1.3. KDBManagerPathTypeVP(Remote DB Path) == kptDatabase
    REQUIRE(KDBManagerPathTypeVP(kdb, pathDb) == kptDatabase);
    REQUIRE_RC(Compare(pathDb, dbRPath));

    const VTable * t(0);
    // 1.4. VDBManagerOpenTable_FUNC_VPath(Remote DB Path) Fails
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathDb));
    REQUIRE_RC(Compare(pathDb, dbRPath));

    // 1.5. Set Local Table Directory for Remote DB Path
    // Set sra file's directory for remote DB run
    REQUIRE_RC(VPathSetDirectory(pathDb, dirLoc));
    // 1.6. KDBManagerOpenDB_FUNC_VPath(Updated Remote DB Path) Fails
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathDb));
    REQUIRE_RC(Compare(pathDb, tbLPath));

    // 1.7. VDBManagerOpenTable_FUNC_VPath(Updated Remote DB Path) Succees
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathDb));
    REQUIRE_RC(Compare(pathDb, tbLPath));
    REQUIRE_RC(VTableRelease(t));

    // 1.8. KDBManagerPathTypeVP(Updated Remote DB Path) == kptTable
    REQUIRE(VDBManagerPathTypeVP(vdb, pathDb) == kptTable);
    REQUIRE(VDBManagerPathTypeVPath(vdb, pathDb) == kptTable);
    REQUIRE_RC(Compare(pathDb, tbLPath));

    REQUIRE_RC(KDirectoryRelease(dirLoc));
    REQUIRE_RC(VPathRelease(pathDb));
    REQUIRE_RC(VPathRelease(pathL));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestKDBManagerOpenTable_FUNC_VPath, Fixture) {
    // open sra file table as table
    VPath * pathL(0);
    REQUIRE_RC(LegacyVPathMake(&pathL, AccTPath));

    const KDirectory * dirLoc(0);
    const KTable * t(0);
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath, &dirLoc));
    REQUIRE_RC(KTableRelease(t));

    REQUIRE(VDBManagerPathTypeVP(vdb, pathL) == kptTable);
    REQUIRE_RC(Compare(pathL, tbLPath));

    // open sra file table as table: cached dir
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath));
    REQUIRE_RC(KTableRelease(t));

    VPath * pathR(0);
    REQUIRE_RC(LegacyVPathMake(&pathR, AccTR));

    // open remote table as table
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, pathR));
    REQUIRE_RC(Compare(pathR, tbRPath));
    REQUIRE_RC(KTableRelease(t));
 
    // open remote table as table: cached dir
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, pathR));
    REQUIRE_RC(Compare(pathR, tbRPath));
    REQUIRE_RC(KTableRelease(t));

    // Set sra file's directory for remote run
    REQUIRE_RC(VPathSetDirectory(pathR, dirLoc));

    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));
    REQUIRE_RC(KTableRelease(t));

    // try to open sra file table as DB
    const VDatabase * vd(0);
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &vd, 0, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath));

    const KDatabase * kd(0);
    REQUIRE_RC_FAIL(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath));

    REQUIRE_RC(KDirectoryRelease(dirLoc));
    REQUIRE_RC(VPathRelease(pathR));
    REQUIRE_RC(VPathRelease(pathL));

    VPath * path(0);

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTR));

    // try to open remote table accession as table
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, tbRPath));
    REQUIRE_RC(KTableRelease(t));

    // try to open remote table accession as table: cached dir
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, tbRPath));
    REQUIRE_RC(KTableRelease(t));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDR));

    // try to open remote DB accession as table
    REQUIRE_RC_FAIL(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, dbRPath));

    // try to open remote DB accession as table: cached dir
    REQUIRE_RC_FAIL(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, dbRPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDL));

    // try to open local DB accession as table
    REQUIRE_RC_FAIL(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, dbLAccPath));

    // try to open local DB accession as table: cached dir
    REQUIRE_RC_FAIL(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, dbLAccPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDPath));

    // try to open path to a local DB as table
    REQUIRE_RC_FAIL(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, dbLPath));

    // try to open path to a local DB as table: cached dir
    REQUIRE_RC_FAIL(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, dbLPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTL));

    // try to open local table accession as table
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, tbLAccPath));
    REQUIRE_RC(KTableRelease(t));

    // try to open local table accession as table: cached dir
    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &t, path));
    REQUIRE_RC(Compare(path, tbLAccPath));
    REQUIRE_RC(KTableRelease(t));

    REQUIRE_RC(VPathRelease(path));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestKDBManagerPathTypeVP, Fixture) {
    VPath * pathL(0);

    // PathType of an sra file
    const KDirectory * dirLoc(0);
    REQUIRE_RC(LegacyVPathMake(&pathL, AccTPath));
    REQUIRE_RC(VPathGetDirectory(pathL, &dirLoc));
    REQUIRE_NULL(dirLoc);
    
    REQUIRE(KDBManagerPathTypeVP(kdb, pathL) == kptTable);
    REQUIRE_RC(Compare(pathL, tbLPath));

    // PathType of an sra file: cached dir
    REQUIRE(KDBManagerPathTypeVPath(kdb, pathL) == kptTable);
    REQUIRE_RC(Compare(pathL, tbLPath));

    REQUIRE_RC_FAIL(VPathGetDirectory(pathL, 0));
    REQUIRE_RC_FAIL(VPathGetDirectory(0, &dirLoc));
    
    // PathType of an sra file: cached dir
    REQUIRE(KDBManagerPathTypeVP(kdb, pathL) == kptTable);
    REQUIRE_RC(Compare(pathL, tbLPath, &dirLoc));

    VPath * pathR(0);
    REQUIRE_RC(LegacyVPathMake(&pathR, AccTR));

    // PathType of a remote table
    REQUIRE(KDBManagerPathTypeVP(kdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbRPath));

    // PathType of a remote table: cached dir
    REQUIRE(KDBManagerPathTypeVP(kdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbRPath));

    // Set sra file's directory for remote run
    REQUIRE_RC(VPathSetDirectory(pathR, dirLoc));
    REQUIRE(KDBManagerPathTypeVP(kdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbLPath));

    REQUIRE_RC(KDirectoryRelease(dirLoc));

    // reuse path for VDBManagerOpenTable_FUNC_VPath
    const VTable * t(0);
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));
    REQUIRE_RC(VTableRelease(t));

    // reuse path for VDBManagerOpenDB_FUNC_VPath
    const VDatabase * d(0);
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));

    // remote DB
    VPath * pathDb(0);
    REQUIRE_RC(LegacyVPathMake(&pathDb, AccDR));
   
    // PathType of a remote DB
    REQUIRE(KDBManagerPathTypeVP(kdb, pathDb) == kptDatabase);
    REQUIRE_RC(Compare(pathDb, dbRPath));

    // PathType of a remote DB: cached dir
    REQUIRE(KDBManagerPathTypeVP(kdb, pathDb) == kptDatabase);
    REQUIRE_RC(Compare(pathDb, dbRPath));

    const KDatabase * kd(0);
    REQUIRE_RC(KDBManagerOpenDB_FUNC_VPath(kdb, &kd, pathDb));
    REQUIRE_RC(Compare(pathDb, dbRPath));
    REQUIRE_RC(KDatabaseRelease(kd));
    REQUIRE(VDBManagerPathTypeVP(vdb, pathDb) == kptDatabase);
    const KDirectory * dirDb(0);
    REQUIRE_RC(Compare(pathDb, dbRPath, &dirDb));

    // Set remote DB's directory for remote table
    REQUIRE_RC(VPathSetDirectory(pathR, dirDb));
    REQUIRE_RC(Compare(pathR, dbRPath));
    REQUIRE(VDBManagerPathTypeVP(vdb, pathR) == kptDatabase);
    REQUIRE_RC(Compare(pathR, dbRPath));

    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathR));
    REQUIRE_RC(Compare(pathR, dbRPath));

/* N.B. Directory is not saved for change of path from table to DB
and use of VDBManagerOpenDBReadVPath.
   Here - first, pathR (remote table accession) is resolved.
       It (remote VPath) is found in SDL cache.
       It already has corresponding directory set (of table remote location). */
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathR));

    REQUIRE_RC(Compare(pathR, dbRPath));
    REQUIRE_RC(VDatabaseRelease(d));

    REQUIRE_RC(KDirectoryRelease(dirDb));

    REQUIRE_RC(VPathRelease(pathDb));
    REQUIRE_RC(VPathRelease(pathR));
    REQUIRE_RC(VPathRelease(pathL));

    VPath * path(0);

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDL));

    // PathType of a local DB accession
    REQUIRE(KDBManagerPathTypeVP(kdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLAccPath));

    // PathType of a local DB accession: cached dir
    REQUIRE(KDBManagerPathTypeVP(kdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLAccPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDPath));

    // PathType of a path to DB
    REQUIRE(KDBManagerPathTypeVP(kdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLPath));

    // PathType of a path to DB: cached dir
    REQUIRE(KDBManagerPathTypeVP(kdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTL));

    // PathType of a local table accession
    REQUIRE(KDBManagerPathTypeVP(kdb, path) == kptTable);
    REQUIRE_RC(Compare(path, tbLAccPath));

    // PathType of a local table accession: cached dir
    REQUIRE(KDBManagerPathTypeVP(kdb, path) == kptTable);
    REQUIRE_RC(Compare(path, tbLAccPath));

    REQUIRE_RC(VPathRelease(path));
} // TestKDBManagerPathTypeVP
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestVDBManagerOpenDB_FUNC_VPath, Fixture) {
    VPath * pathL(0);
    REQUIRE_RC(LegacyVPathMake(&pathL, AccTPath));
 
    const KDirectory * dirLoc(0);
    REQUIRE_RC(VPathGetDirectory(pathL, &dirLoc));
    REQUIRE_NULL(dirLoc);
  
    const VDatabase * d(0);
    // try to open sra file table as DB
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath, &dirLoc));
    
    // try to open sra file table as DB: cached dir
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath));
    /**/
    REQUIRE_RC(VPathRelease(pathL));

    VPath * pathR(0);
    REQUIRE_RC(LegacyVPathMake(&pathR, AccTR));

    // open remote run table as DB
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbRPath));

    // open remote run table as DB: cached dir
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbRPath));
    
    // Set sra file's directory for remote run
    REQUIRE_RC(VPathSetDirectory(pathR, dirLoc));
    const VTable * t(0);
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));
    REQUIRE_RC(VTableRelease(t));
    
    REQUIRE(KDBManagerPathTypeVP(kdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbLPath));
/**/
    REQUIRE_RC(KDirectoryRelease(dirLoc));
    REQUIRE_RC(VPathRelease(pathR));

    VPath * path(0);

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDR));

    // try to open remote DB accession as DB
    REQUIRE_RC(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, dbRPath));
    REQUIRE_RC(VDatabaseRelease(d));

    // try to open remote DB accession as DB: cached dir
    REQUIRE_RC(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, dbRPath));
    REQUIRE_RC(VDatabaseRelease(d));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDL));

    // try to open local DB accession as DB
    REQUIRE_RC(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, dbLAccPath));
    REQUIRE_RC(VDatabaseRelease(d));

    // try to open local DB accession as DB: cached dir
    REQUIRE_RC(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, dbLAccPath));
    REQUIRE_RC(VDatabaseRelease(d));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDPath));

    // try to open a path to DB as DB
    REQUIRE_RC(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, dbLPath));
    REQUIRE_RC(VDatabaseRelease(d));

    // try to open a path to DB as DB: cached dir
    REQUIRE_RC(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, dbLPath));
    REQUIRE_RC(VDatabaseRelease(d));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTL));

    // try to open local table accession as DB
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, tbLAccPath));

    // try to open local table accession as DB: cached dir
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, path));
    REQUIRE_RC(Compare(path, tbLAccPath));

    REQUIRE_RC(VPathRelease(path));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestVDBManagerOpenTable_FUNC_VPath, Fixture) {
    // open sra file table as table
    VPath * pathL(0);
    const KDirectory * dirLoc(0);
    const VTable * t(0);

    REQUIRE_RC(LegacyVPathMake(&pathL, AccTPath));

    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath));
    REQUIRE_RC(VTableRelease(t)); 
    
    // open sra file table as table: cached dir
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathL));
    REQUIRE_RC(Compare(pathL, tbLPath, &dirLoc));
    REQUIRE_RC(VTableRelease(t));
    
    REQUIRE_RC(VPathRelease(pathL)); 

    VPath * pathR(0);
    REQUIRE_RC(LegacyVPathMake(&pathR, AccTR));
    
    // open remote run table as table
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbRPath));
    REQUIRE_RC(VTableRelease(t));

    // open remote run table as table: cached dir
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbRPath));
    REQUIRE_RC(VTableRelease(t));

    // Set sra file's directory for remote run
    REQUIRE_RC(VPathSetDirectory(pathR, dirLoc));
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));
    REQUIRE_RC(VTableRelease(t));

    // reuse path for KDBManagerPathTypeVP
    REQUIRE(KDBManagerPathTypeVP(kdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbLPath));

    // open remote run table as DB
    const VDatabase * d(0);
    REQUIRE_RC_FAIL(VDBManagerOpenDB_FUNC_VPath(vdb, &d, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));

    REQUIRE_RC(VPathRelease(pathR));
    REQUIRE_RC(KDirectoryRelease(dirLoc));

    VPath * path(0);
    
    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTL));

    // try to open local table accession as table
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, tbLAccPath));
    REQUIRE_RC(VTableRelease(t));

    // try to open local table accession as table: cached dir
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, tbLAccPath));
    REQUIRE_RC(VTableRelease(t));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDL));

    // try to open local DB accession as table
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, dbLAccPath));

    // try to open local DB accession as table: cached dir
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, dbLAccPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDR));

    // try to open a remote DB accession as table
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, dbRPath));

    // try to open a remote DB accession as table: cached dir
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, dbRPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDPath));

    // try to open a path to DB as table
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, dbLPath));

    // try to open a path to DB as table: cached dir
    REQUIRE_RC_FAIL(VDBManagerOpenTable_FUNC_VPath(vdb, &t, 0, path));
    REQUIRE_RC(Compare(path, dbLPath));

    REQUIRE_RC(VPathRelease(path));
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TestVDBManagerPathType, Fixture) {
    REQUIRE(VDBManagerPathType(vdb, AccDR) == kptDatabase);

    REQUIRE(VDBManagerPathTypeVP(0, 0) == kptBadPath);
    REQUIRE(VDBManagerPathTypeVP(vdb, 0) == kptBadPath);

    VPath * pathL(0);
    REQUIRE_RC(LegacyVPathMake(&pathL, AccTPath));
  
    REQUIRE(VDBManagerPathTypeVP(0, pathL) == kptBadPath);
    const KDirectory * dirLoc(0);
    REQUIRE_RC(VPathGetDirectory(pathL, &dirLoc));
    REQUIRE_NULL(dirLoc);

    // PathType of an sra file
    REQUIRE(VDBManagerPathTypeVP(vdb, pathL) == kptTable);
    REQUIRE_RC(Compare(pathL, tbLPath, &dirLoc));

    // PathType of an sra file: cached dir
    REQUIRE(VDBManagerPathTypeVP(vdb, pathL) == kptTable);
    REQUIRE_RC(Compare(pathL, tbLPath));

    VPath * pathR(0);
    REQUIRE_RC(LegacyVPathMake(&pathR, AccTR));

    // PathType of a remote table
    REQUIRE(VDBManagerPathTypeVP(vdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbRPath));

    // PathType of a remote table: cached dir
    REQUIRE(VDBManagerPathTypeVP(vdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbRPath));

    // Set sra file's directory for remote run
    REQUIRE_RC(VPathSetDirectory(pathR, dirLoc));
    REQUIRE(VDBManagerPathTypeVP(vdb, pathR) == kptTable);
    REQUIRE_RC(Compare(pathR, tbLPath));

    REQUIRE_RC(KDirectoryRelease(dirLoc));
    REQUIRE_RC(VPathRelease(pathL));
    
    // reuse path for VDBManagerOpenTable_FUNC_VPath
    const VTable * vt(0);
    REQUIRE_RC(VDBManagerOpenTable_FUNC_VPath(vdb, &vt, 0, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));
    REQUIRE_RC(VTableRelease(vt));

    // reuse path for KDBManagerOpenTable_FUNC_VPath
    const KTable * kt(0);
    REQUIRE_RC(KTableRelease(kt));

    REQUIRE_RC(KDBManagerOpenTable_FUNC_VPath(kdb, &kt, pathR));
    REQUIRE_RC(Compare(pathR, tbLPath));
    REQUIRE_RC(KTableRelease(kt));

    REQUIRE_RC(VPathRelease(pathR));
    
    VPath * path(0);

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDR));

    // PathType of a remote DB accession
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbRPath));

    // PathType of a remote DB accession: cached dir
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbRPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDPath));

    // PathType of a path to a local DB
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLPath));

    // PathType of a path to a local DB: cached dir
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccDL));

    // PathType of a local DB accession
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLAccPath));

    // PathType of a local DB accession: cached dir
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptDatabase);
    REQUIRE_RC(Compare(path, dbLAccPath));

    REQUIRE_RC(VPathRelease(path));

    ////////////////////////////////////////////////////////////////////////////
    REQUIRE_RC(LegacyVPathMake(&path, AccTL));

    // PathType of a local table accession
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptTable);
    REQUIRE_RC(Compare(path, tbLAccPath));

    // PathType of a local table accession: cached dir
    REQUIRE(VDBManagerPathTypeVP(vdb, path) == kptTable);
    REQUIRE_RC(Compare(path, tbLAccPath));

    REQUIRE_RC(VPathRelease(path));
}
#endif

extern "C" {
    ver_t CC KAppVersion(void) { return 0; }
    const char UsageDefaultName[] = "test-vpath-open";
    rc_t CC UsageSummary(const char * progname) { return 0; }
    rc_t CC Usage(const struct Args * args) { return 0; }
    rc_t CC KMain(int argc, char * argv[]) {
#if 0
        KDbgSetString("VFS");
#endif
        return TestVpathOpenSuite(argc, argv);
    }
}

/*
KDBManager_FUNC_DBRead	        d
KDBManagerOpenTable_FUNC_VPath	t
KDBManagerPathTypeVP	        p
VDBManagerOpenDB_FUNC_VPath	    D
VDBManagerOpenTable_FUNC_VPath	T
VDBManagerPathType	            P

TestKDBManagerOpenDBRead	        dpdTP
TestKDBManagerOpenTable_FUNC_VPath	tPtDd
TestKDBManagerPathTypeVP	        pTDdPTD
TestVDBManagerOpenDB_FUNC_VPath	    DTp
TestVDBManagerOpenTableReadVPath	TpD
TestVDBManagerPathType	            PTt
*/
