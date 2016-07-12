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
* Unit tests for Kdb interface
*/

#include <ktst/unit_test.hpp>

#include <sysalloc.h>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>
#include <kdb/table.h>
#include <kdb/column.h>
#include <kdb/meta.h>

using namespace std;

TEST_SUITE(KdbTestSuite);

#define KDB_MANAGER_MAKE(m_mgr, m_wd) KDBManagerMakeUpdate((KDBManager **)m_mgr, (struct KDirectory *)m_wd)
#include "remote_open_test.cpp"

class WKDB_Fixture
{
public:
    WKDB_Fixture()
    {
        THROW_ON_RC ( KDirectoryNativeDir ( & m_wd ) );
        THROW_ON_RC ( KDBManagerMakeUpdate ( & m_mgr, m_wd ) );
    }
    ~WKDB_Fixture()
    {
        KDBManagerRelease ( m_mgr );
        KDirectoryRelease ( m_wd );
    }
    
    std::string GetColumnMetadata ( const KColumn* p_col, const char* p_metadataName )
    {
        const KMetadata *meta;
        THROW_ON_RC ( KColumnOpenMetadataRead( p_col, &meta ) );
        const KMDataNode* node;
        THROW_ON_RC ( KMetadataOpenNodeRead ( meta, & node, p_metadataName ) );
        size_t num_read;
        char buf[1024];
        THROW_ON_RC ( KMDataNodeRead ( node, 0, buf, sizeof buf, & num_read, 0 ) );
        THROW_ON_RC ( KMDataNodeRelease ( node ) );
        THROW_ON_RC ( KMetadataRelease ( meta ) );
        return string ( buf, num_read );
    }
    
    KDirectory* m_wd;
    KDBManager* m_mgr;
};

FIXTURE_TEST_CASE ( MissingRows, WKDB_Fixture )
{   // VDB-177
    KDirectoryRemove(m_wd, true, GetName());
    KDatabase* db;
    REQUIRE_RC(KDBManagerCreateDB(m_mgr, &db, kcmCreate, GetName()));
 
    KIndex *idx;
    REQUIRE_RC(KDatabaseCreateIndex(db, &idx, kitText, kcmCreate, "index"));
    
    REQUIRE_RC(KIndexInsertText(idx, true, "aaaa1", 1));
    REQUIRE_RC(KIndexInsertText(idx, true, "aaaa3", 3));
    
    int64_t start_id;
    uint64_t id_count;
    REQUIRE_RC(KIndexFindText (idx, "aaaa1", &start_id, &id_count, NULL, NULL));
    REQUIRE_EQ(start_id, (int64_t)1);
    REQUIRE_EQ(id_count, (uint64_t)1);
    
    REQUIRE_RC_FAIL(KIndexFindText (idx, "aaaa2", &start_id, &id_count, NULL, NULL));
    REQUIRE_RC_FAIL(KIndexFindText (idx, "", &start_id, &id_count, NULL, NULL));
    
    REQUIRE_RC(KIndexFindText (idx, "aaaa3", &start_id, &id_count, NULL, NULL));
    REQUIRE_EQ(start_id, (int64_t)3);
    REQUIRE_EQ(id_count, (uint64_t)1);
    
    REQUIRE_RC(KIndexRelease(idx));
   
    REQUIRE_RC(KDatabaseRelease(db));
    KDirectoryRemove(m_wd, true, GetName());
}

FIXTURE_TEST_CASE ( ColumnMetadataWKDB_Fixture, WKDB_Fixture )
{
    KDirectoryRemove(m_wd, true, GetName());
    
    const char* TableName = "tbl";
    const char* ColumnName = "col";
    const char* MetadataName = "metaname";
    const char* MetadataValue = "metavalue";
    
    {
        KDatabase* db;
        REQUIRE_RC(KDBManagerCreateDB(m_mgr, &db, kcmCreate, GetName()));
     
        KTable* tbl;
        REQUIRE_RC ( KDBManagerCreateTable ( m_mgr, & tbl, kcmInit + kcmMD5, TableName ) );
        
        KColumn* col;
        REQUIRE_RC ( KTableCreateColumn ( tbl, & col, kcmInit, kcmMD5, 0, ColumnName ) );
        
        KMetadata *meta;
        REQUIRE_RC ( KColumnOpenMetadataUpdate ( col, &meta ) );
        
        KMDataNode* node;
        REQUIRE_RC ( KMetadataOpenNodeUpdate ( meta, & node, MetadataName ) );
        
        REQUIRE_RC ( KMDataNodeWrite ( node, MetadataValue, string_size ( MetadataValue ) ) );
        
        REQUIRE_RC ( KMDataNodeRelease ( node ) );
        
        REQUIRE_RC ( KMetadataRelease ( meta ) );
       
        // can only read after metadata has been released
        REQUIRE_EQ ( string ( MetadataValue ), GetColumnMetadata ( col, MetadataName ) );
        
        REQUIRE_RC ( KColumnRelease ( col ) );
        REQUIRE_RC ( KTableRelease ( tbl ) );
        REQUIRE_RC ( KDatabaseRelease ( db ) );
    }
    {   // reopen, verify
        KDatabase* db;
        REQUIRE_RC ( KDBManagerOpenDBUpdate ( m_mgr, &db, GetName() ) );
        
        const KTable* tbl;
        REQUIRE_RC ( KDBManagerOpenTableRead ( m_mgr, & tbl, TableName ) );
        
        const KColumn* col;
        REQUIRE_RC ( KTableOpenColumnRead ( tbl, & col, ColumnName ) );
        
        REQUIRE_EQ ( string ( MetadataValue ), GetColumnMetadata ( col, MetadataName ) );
        
        REQUIRE_RC ( KColumnRelease ( col ) );
        REQUIRE_RC ( KTableRelease ( tbl ) );
        REQUIRE_RC ( KDatabaseRelease ( db ) );
    }
    
    KDirectoryRemove(m_wd, true, TableName);
    KDirectoryRemove(m_wd, true, GetName());
}

// KColumnBlob
// see same tests on the read side, kdbtest.cpp

class ColumnBlobReadFixture
{
public:
    ColumnBlobReadFixture()
    :   m_num_read ( 0 ),
        m_remaining ( 0 )
    {
        KDBManager* mgr;
        THROW_ON_RC ( KDBManagerMakeUpdate( & mgr, NULL ) );
        
        const KTable* tbl;
        THROW_ON_RC ( KDBManagerOpenTableRead ( mgr, & tbl, "SRR000123" ) );
        
        const KColumn* col;
        THROW_ON_RC ( KTableOpenColumnRead ( tbl, & col, "X" ) );
        
        THROW_ON_RC ( KColumnOpenBlobRead ( col, & m_blob, 1 ) );
        
        THROW_ON_RC ( KColumnRelease ( col ) );
        THROW_ON_RC ( KTableRelease ( tbl ) );
        THROW_ON_RC ( KDBManagerRelease ( mgr ) );
    }
    ~ColumnBlobReadFixture()
    {
        KColumnBlobRelease ( m_blob );
    }

    const KColumnBlob*  m_blob;
    size_t m_num_read;
    size_t m_remaining;
};

FIXTURE_TEST_CASE ( ColumnBlobRead_basic, ColumnBlobReadFixture )
{   
    const size_t BlobSize = 1882; 
    const size_t BufSize = 2024; 
    char buffer [ BufSize ];
    REQUIRE_RC ( KColumnBlobRead ( m_blob, 0, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BlobSize, m_num_read );
    REQUIRE_EQ ( (size_t)0, m_remaining );
}

FIXTURE_TEST_CASE ( ColumnBlobRead_insufficient_buffer, ColumnBlobReadFixture )
{   
    const size_t BlobSize = 1882; 
    const size_t BufSize = 1024; 
    char buffer [ BufSize ];
    // first read incomplete
    REQUIRE_RC ( KColumnBlobRead ( m_blob, 0, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BufSize, m_num_read );
    REQUIRE_EQ ( BlobSize - BufSize, m_remaining );
    // the rest comes in on the second read
    REQUIRE_RC ( KColumnBlobRead ( m_blob, BufSize, buffer, BufSize, & m_num_read, & m_remaining ) );
    REQUIRE_EQ ( BlobSize - BufSize, m_num_read );
    REQUIRE_EQ ( (size_t)0, m_remaining );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/debug.h> // KDbgSetString

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

const char UsageDefaultName[] = "test-wkdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    //assert(!KDbgSetString("KFG"));
    //assert(!KDbgSetString("VFS"));
    KConfigDisableUserSettings();
    rc_t rc=KdbTestSuite(argc, argv);
    return rc;
}

}
