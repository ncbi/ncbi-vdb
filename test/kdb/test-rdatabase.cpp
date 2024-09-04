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
* Unit tests for read-side KDatabase
*/

#include <ktst/unit_test.hpp>

#include <../libs/kdb/rdatabase.h>
#include <../libs/kdb/dbmgr.h>

#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <kdb/table.h>
#include <kdb/meta.h>
#include <kdb/index.h>
#include <kdb/namelist.h>

#include <klib/rc.h>
#include <klib/namelist.h>

using namespace std;

TEST_SUITE(KRDatabaseTestSuite);

static const string ScratchDir = "./data/";

class KDatabase_Fixture
{
public:
    KDatabase_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeRead ( & m_mgr, m_dir ) );
    }
    ~KDatabase_Fixture()
    {
        KDatabaseRelease( m_db );
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }
    void Setup( const string testName )
    {
        const string DbName = ScratchDir + testName;
        THROW_ON_RC( KRDatabaseMake( & m_db, m_dir, DbName.c_str(), m_mgr ) );
        KDirectoryAddRef( m_dir); // KRDatabaseMake does not call AddRef
    }

    KDirectory * m_dir = nullptr;
    const KDBManager * m_mgr = nullptr;
    const KDatabase * m_db = nullptr;
};


//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KRDatabase_AddRelease, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE_EQ( 1, (int)atomic32_read( & m_db -> dad . refcount ) );
    REQUIRE_RC( KDatabaseAddRef( m_db ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_db -> dad . refcount ) );
    REQUIRE_RC( KDatabaseRelease( m_db ) );
    REQUIRE_EQ( 1, (int)atomic32_read( & m_db -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRDatabase_Locked, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseLocked( m_db ) );
}

FIXTURE_TEST_CASE(KRDatabase_Exists, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseExists( m_db, kptIndex, "%s", GetName() ) );
}

FIXTURE_TEST_CASE(KRDatabase_IsAlias, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE( ! KDatabaseIsAlias( m_db, kptIndex, nullptr, 0, GetName() ) );
}

FIXTURE_TEST_CASE(KRDatabase_Writable, KDatabase_Fixture)
{
    Setup( GetName() );

    REQUIRE_EQ( -1, (int)KDatabaseWritable( m_db, kptIndex, "%s", GetName() ) ); // "TBD" for read side
}

FIXTURE_TEST_CASE(KRDatabase_OpenManagerRead, KDatabase_Fixture)
{
    Setup( GetName() );

    struct KDBManager const *mgr;
    REQUIRE_RC( KDatabaseOpenManagerRead( m_db, &mgr ) );
    REQUIRE_EQ( m_mgr, mgr );
    KDBManagerRelease( mgr );
}

FIXTURE_TEST_CASE(KRDatabase_OpenParentRead, KDatabase_Fixture)
{
    Setup( GetName() );

    struct KDatabase const *db;
    REQUIRE_RC( KDatabaseOpenParentRead( m_db, &db ) );
    REQUIRE_NULL( db );
}

FIXTURE_TEST_CASE(KRDatabase_OpenDirectoryRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KDirectory * dir = nullptr;
    REQUIRE_RC( KDatabaseOpenDirectoryRead( m_db, &dir ) );
    REQUIRE_EQ( (const KDirectory *)m_dir, dir );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenDBRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KDatabase * subdb = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenDBRead( m_db, & subdb, "%s", "subdb" ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenTableRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KTable * t = nullptr;
    rc_t rc = SILENT_RC( rcFS,rcDirectory,rcOpening,rcPath,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenTableRead( m_db, & t, "%s", "t" ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenMetadataRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KMetadata * meta = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcMetadata,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenMetadataRead( m_db, & meta ) );
}

FIXTURE_TEST_CASE(KRDatabase_OpenIndexRead, KDatabase_Fixture)
{
    Setup( GetName() );
    const KIndex * idx = nullptr;
    rc_t rc = SILENT_RC( rcDB,rcMgr,rcOpening,rcIndex,rcNotFound );
    REQUIRE_EQ( rc, KDatabaseOpenIndexRead( m_db, & idx, "%s", "qq" ) );
}

FIXTURE_TEST_CASE(KRDatabase_ListDB, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & m_db, "testdb" ) );

    struct KNamelist * names;
    REQUIRE_RC( KDatabaseListDB( m_db, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KRDatabase_ListTbl, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & m_db, "testdb" ) );

    struct KNamelist * names;
    REQUIRE_RC( KDatabaseListTbl( m_db, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)1, count );
    REQUIRE( KNamelistContains( names, "SEQUENCE" ) );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KRDatabase_ListIdx, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & m_db, "testdb" ) );

    struct KNamelist * names;
    REQUIRE_RC( KDatabaseListIdx( m_db, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KRDatabase_GetPath, KDatabase_Fixture)
{
    REQUIRE_RC( KDBManagerOpenDBRead( m_mgr, & m_db, "testdb" ) );

    const char * path;
    REQUIRE_RC( KDatabaseGetPath( m_db, & path ) );
}

extern "C" rc_t KDBRManagerPathContents(const KDBManager *self, KDBContents const **result, const char *path, ...);

static KDBContents const *findChildNamed(KDBContents const *node, std::string const &name)
{
    auto n = node->firstChild;
    while (n != nullptr) {
        if (name == n->name)
            return n;
        n = n->nextSibling;
    }
    return nullptr;
}

FIXTURE_TEST_CASE(KDBRManager_PathContents_dir, KDatabase_Fixture)
{
    auto const name = std::string("testdb");
    KDBContents const *contents = NULL;
    REQUIRE_RC(KDBRManagerPathContents(m_mgr, &contents, "%s", name.c_str()));
    REQUIRE_NOT_NULL(contents);
    REQUIRE_EQ(name, string(contents->name));
    
    // REQUIRE_EQ((int)(contents->fstype | kptAlias), (int)(kptDir | kptAlias));
    
    REQUIRE_EQ((int)contents->dbtype, (int)kptDatabase);
    REQUIRE_EQ((int)(contents->attributes & cca_HasErrors), (int)cca_HasErrors);

    auto const db_dummy = findChildNamed(contents, "dummy");
    REQUIRE_NOT_NULL(db_dummy);

    auto const tbl_SEQUENCE = findChildNamed(contents, "SEQUENCE");
    REQUIRE_NOT_NULL(tbl_SEQUENCE);

    auto const col_dummy = findChildNamed(tbl_SEQUENCE, "dummy");
    REQUIRE_NOT_NULL(col_dummy);

    auto const col_READ = findChildNamed(tbl_SEQUENCE, "READ");
    REQUIRE_NOT_NULL(col_READ);

    // check parent/child and sibling pointers
    REQUIRE_EQ(db_dummy->parent, contents);
    REQUIRE_EQ(tbl_SEQUENCE->parent, contents);

    REQUIRE(tbl_SEQUENCE->nextSibling == db_dummy || db_dummy->nextSibling == tbl_SEQUENCE);
    REQUIRE(tbl_SEQUENCE->prevSibling == db_dummy || db_dummy->prevSibling == tbl_SEQUENCE);

    REQUIRE_EQ(col_dummy->parent, tbl_SEQUENCE);
    REQUIRE_EQ(col_READ->parent, tbl_SEQUENCE);
    REQUIRE(col_dummy->nextSibling == col_READ || col_READ->nextSibling == col_dummy);
    REQUIRE(col_dummy->prevSibling == col_READ || col_READ->prevSibling == col_dummy);

    // check file system types
    REQUIRE_EQ((int)(db_dummy->fstype | kptAlias), (int)(kptFile | kptAlias));
    REQUIRE_EQ((int)(col_dummy->fstype | kptAlias), (int)(kptFile | kptAlias));
    REQUIRE_EQ((int)(tbl_SEQUENCE->fstype | kptAlias), (int)(kptDir | kptAlias));
    REQUIRE_EQ((int)(col_READ->fstype | kptAlias), (int)(kptDir | kptAlias));

    // check KDB types
    REQUIRE_EQ((int)tbl_SEQUENCE->dbtype, (int)kptTable);
    REQUIRE_EQ((int)col_READ->dbtype, (int)kptColumn);
    // REQUIRE_EQ((int)col_dummy->dbtype, (int)kptFile);
    // REQUIRE_EQ((int)db_dummy->dbtype, (int)kptFile);

    // check error bit
    REQUIRE_EQ((int)(col_READ->attributes & cca_HasErrors), (int)cca_HasErrors);
    REQUIRE_EQ((int)(tbl_SEQUENCE->attributes & cca_HasErrors), (int)cca_HasErrors);

    KDBContentsWhack(contents);
}

FIXTURE_TEST_CASE(KDBRManager_PathContents_kar, KDatabase_Fixture)
{
    auto const name = std::string("testdb.kar");
    KDBContents const *contents = NULL;
    REQUIRE_RC(KDBRManagerPathContents(m_mgr, &contents, "%s", name.c_str()));
    REQUIRE_NOT_NULL(contents);
    REQUIRE_EQ(name, string(contents->name));
    
    REQUIRE_EQ((int)(contents->fstype | kptAlias), (int)(kptFile | kptAlias));
    
    REQUIRE_EQ((int)contents->dbtype, (int)kptDatabase);
    REQUIRE_EQ((int)(contents->attributes & cca_HasErrors), (int)cca_HasErrors);

    auto const db_dummy = findChildNamed(contents, "dummy");
    REQUIRE_NOT_NULL(db_dummy);

    auto const tbl_SEQUENCE = findChildNamed(contents, "SEQUENCE");
    REQUIRE_NOT_NULL(tbl_SEQUENCE);

    auto const col_dummy = findChildNamed(tbl_SEQUENCE, "dummy");
    REQUIRE_NOT_NULL(col_dummy);

    auto const col_READ = findChildNamed(tbl_SEQUENCE, "READ");
    REQUIRE_NOT_NULL(col_READ);

    // check parent/child and sibling pointers
    REQUIRE_EQ(db_dummy->parent, contents);
    REQUIRE_EQ(tbl_SEQUENCE->parent, contents);

    REQUIRE(tbl_SEQUENCE->nextSibling == db_dummy || db_dummy->nextSibling == tbl_SEQUENCE);
    REQUIRE(tbl_SEQUENCE->prevSibling == db_dummy || db_dummy->prevSibling == tbl_SEQUENCE);

    REQUIRE_EQ(col_dummy->parent, tbl_SEQUENCE);
    REQUIRE_EQ(col_READ->parent, tbl_SEQUENCE);
    REQUIRE(col_dummy->nextSibling == col_READ || col_READ->nextSibling == col_dummy);
    REQUIRE(col_dummy->prevSibling == col_READ || col_READ->prevSibling == col_dummy);

    // check file system types
    REQUIRE_EQ((int)(db_dummy->fstype | kptAlias), (int)(kptFile | kptAlias));
    REQUIRE_EQ((int)(col_dummy->fstype | kptAlias), (int)(kptFile | kptAlias));
    REQUIRE_EQ((int)(tbl_SEQUENCE->fstype | kptAlias), (int)(kptDir | kptAlias));
    REQUIRE_EQ((int)(col_READ->fstype | kptAlias), (int)(kptDir | kptAlias));

    // check KDB types
    REQUIRE_EQ((int)tbl_SEQUENCE->dbtype, (int)kptTable);
    REQUIRE_EQ((int)col_READ->dbtype, (int)kptColumn);
    // REQUIRE_EQ((int)col_dummy->dbtype, (int)kptFile);
    // REQUIRE_EQ((int)db_dummy->dbtype, (int)kptFile);

    // check error bit
    REQUIRE_EQ((int)(col_READ->attributes & cca_HasErrors), (int)cca_HasErrors);
    REQUIRE_EQ((int)(tbl_SEQUENCE->attributes & cca_HasErrors), (int)cca_HasErrors);

    KDBContentsWhack(contents);
}
/*
FIXTURE_TEST_CASE(KDBRManager_PathContents_SRR, KDatabase_Fixture)
{
    KDBContents const *contents = NULL;
    REQUIRE_RC(KDBRManagerPathContents(m_mgr, &contents, "SRR000001"));
    REQUIRE_NOT_NULL(contents);
    
    REQUIRE_EQ((int)(contents->fstype), 0); ///< fstype is unknown
    
    REQUIRE_EQ((int)contents->dbtype, (int)kptTable); ///< SRR000001 is a table
    REQUIRE_EQ((int)(contents->attributes & cca_HasMetadata), (int)cca_HasMetadata);
    REQUIRE_EQ((int)(contents->attributes & cca_HasMD5_File), (int)cca_HasMD5_File);
    REQUIRE_EQ((int)(contents->attributes & cca_HasLock), (int)cca_HasLock);
    REQUIRE_EQ((int)(contents->attributes & cca_HasSealed), 0);
    REQUIRE_EQ((int)(contents->attributes & cca_HasErrors), 0);
    REQUIRE_EQ((int)(contents->attributes & cta_HasColumns), (int)cta_HasColumns);

    auto const col_READ = findChildNamed(contents, "READ");
    REQUIRE_NOT_NULL(col_READ);
    REQUIRE_EQ((int)col_READ->dbtype, (int)kptColumn);
    REQUIRE_EQ((int)(col_READ->attributes & cca_HasMetadata), (int)cca_HasMetadata);
    REQUIRE_EQ((int)(col_READ->attributes & cca_HasChecksum_CRC), (int)cca_HasChecksum_CRC);

    KDBContentsWhack(contents);
}
*/

//KDB_EXTERN rc_t CC KDatabaseGetPath ( struct KDatabase const *self, const char **path );

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "Test_KDB_KRDatabase";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KRDatabaseTestSuite(argc, argv);
    return rc;
}

}
