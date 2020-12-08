// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#include <cmath>

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h> /* VSchemaRelease */
#include <vdb/vdb-priv.h>

#include <sra/sraschema.h> // VDBManagerMakeSRASchema

#include <kdb/meta.h>
#include <kdb/table.h>

#include <klib/rc.h>

#include <ktst/unit_test.hpp> // TEST_CASE

#include <sysalloc.h>

#include <sstream>
#include <cstdlib>

using namespace std;

/*
 * We will compile this test file first in read/write mode, create DB and
 * run tests using libncbi-wvdb library; then compile it again, but with -DREAD_ONLY
 * and link it against read-only lincbi-vdb library and run tests
 *
 * NB: rw mode test creates DB, rd mode deletes it.
 */

TEST_SUITE( SparseColTestSuite )

const string ScratchDir = "./db/";

class VDB_Fixture
{
public:
    VDB_Fixture()
#if !READ_ONLY
    : m_wdb(NULL), m_rdb(NULL)
#else
    : m_rdb(NULL)
#endif
    {
        THROW_ON_RC ( KDirectoryNativeDir ( & m_wd ) );
#if !READ_ONLY
        THROW_ON_RC ( VDBManagerMakeUpdate ( & m_mgr, m_wd ) );
#else
        THROW_ON_RC ( VDBManagerMakeRead ( & m_mgr, m_wd ) );
#endif
    }
    ~VDB_Fixture()
    {
#if READ_ONLY
        RemoveDatabase ();
#endif
        VDBManagerRelease ( m_mgr );
        KDirectoryRelease ( m_wd );
    }
    void RemoveDatabase()
    {
        if ( ! m_databaseName . empty () )
        {
            KDirectoryRemove ( m_wd, true, m_databaseName . c_str () );
        }
    }

#if !READ_ONLY
    VCursor* CreateDatabase()
    {
        const string schemaText =
        "table t1 #1\n"
        "{\n"
        "    column ascii COL1;\n"
        "    column ascii COL2;\n"
        "};\n"

        "database db #1\n"
        "{\n"
        "    table t1 #1 T1;\n"
        "};\n"
        ;
        const char * schemaSpec = "db";

        assert ( m_wdb == NULL );

        // MakeDatabase
        {
            VDBManager* mgr;
            THROW_ON_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
            VSchema* schema;
            THROW_ON_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
            THROW_ON_RC ( VSchemaParseText(schema, NULL, schemaText . c_str(), schemaText . size () ) );

            THROW_ON_RC ( VDBManagerCreateDB ( mgr,
                                              & m_wdb,
                                              schema,
                                              schemaSpec,
                                              kcmInit + kcmMD5,
                                              "%s",
                                              m_databaseName . c_str () ) );
            THROW_ON_RC ( VSchemaRelease ( schema ) );
            THROW_ON_RC ( VDBManagerRelease ( mgr ) );
        }

        // MakeCursor
        VCursor* cursor;
        {
            VTable* table;
            THROW_ON_RC ( VDatabaseCreateTable ( m_wdb, & table, "T1", kcmCreate | kcmMD5, "%s", "T1" ) );
            THROW_ON_RC ( VTableCreateCursorWrite ( table, & cursor, kcmInsert ) );
            THROW_ON_RC ( VTableRelease ( table ) );
        }

        return cursor;
    }

    void CommitDatabase ( VCursor * cursor )
    {
        assert ( m_wdb != NULL );

        THROW_ON_RC ( VCursorCommit ( cursor ) );    // this returns rcVDB,rcBlob,rcValidating,rcBlob,rcCorrupt if the schema does not support
                                                        // writing to the LABEL column from the code
        THROW_ON_RC ( VCursorRelease ( cursor ) );

        THROW_ON_RC ( VDatabaseRelease ( m_wdb ) );

        m_wdb = NULL;
    }

    void WriteRow ( VCursor * cursor, uint32_t column_idx, int64_t row_id, std::string data )
    {
        THROW_ON_RC ( VCursorSetRowId ( cursor, row_id ) );
        THROW_ON_RC ( VCursorOpenRow ( cursor ) );
        THROW_ON_RC ( VCursorWrite ( cursor, column_idx, 8, data.c_str(), 0, data.size() ) );
        THROW_ON_RC ( VCursorCommitRow ( cursor ) );
        THROW_ON_RC ( VCursorCloseRow ( cursor ) );
        THROW_ON_RC ( VCursorFlushPage ( cursor ) );
    }
#endif

    const VCursor* OpenDatabaseRead ()
    {
        assert ( m_rdb == NULL );

        // MakeDatabase
        {
#if !READ_ONLY
            VDBManager* mgr;
            THROW_ON_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
#else
            const VDBManager* mgr;
            THROW_ON_RC ( VDBManagerMakeRead ( & mgr, NULL ) );
#endif

            THROW_ON_RC ( VDBManagerOpenDBRead ( mgr,
                                              & m_rdb,
                                              NULL,
                                              "%s",
                                              m_databaseName . c_str () ) );
            THROW_ON_RC ( VDBManagerRelease ( mgr ) );
        }

        // MakeCursor
        const VCursor* cursor;
        {
            const VTable* table;
            THROW_ON_RC ( VDatabaseOpenTableRead( m_rdb, & table, "%s", "T1" ) );
            THROW_ON_RC ( VTableCreateCursorRead ( table, & cursor ) );
            THROW_ON_RC ( VTableRelease ( table ) );
        }

        return cursor;
    }

    void ReleaseDatabase ( const VCursor* cursor )
    {
        THROW_ON_RC ( VCursorRelease ( cursor ) );

        THROW_ON_RC ( VDatabaseRelease ( m_rdb ) );
    }

#if !READ_ONLY
    void WriteColumn ( int64_t rows[], int rows_size, const char * col, bool static_col )
    {
        uint32_t columnIdx;
        RemoveDatabase();

        VCursor * wcursor = CreateDatabase();

        // AddColumns
        THROW_ON_RC ( VCursorAddColumn ( wcursor, & columnIdx, "%s", col ) );
        THROW_ON_RC ( VCursorOpen ( wcursor ) );

        for ( int i = 0; i < rows_size; ++i)
        {
            int64_t row_id = rows[i];
            ostringstream out;
            out << row_id;

            WriteRow ( wcursor, columnIdx, row_id, static_col ? "abcd" : out.str() );
        }

        CommitDatabase(wcursor);
    }
#endif

    void RunChecks ( int64_t rows[], int rows_size, const char * col )
    {
        uint32_t columnIdx;
        const VCursor * rcursor = OpenDatabaseRead();
        THROW_ON_RC ( VCursorAddColumn ( rcursor, & columnIdx, "%s", col ) );
        THROW_ON_RC ( VCursorOpen ( rcursor ) );

        int64_t row_id;
        THROW_ON_RC ( VCursorRowId ( rcursor, &row_id ) );
        if ( rows[0] != row_id )
            FAIL("rows[i] != row_id");

        for ( int i = 1; i < rows_size; ++i) {
            THROW_ON_RC ( VCursorFindNextRowId ( rcursor, 0, &row_id ) );
            if ( rows[i] != row_id )
                FAIL("rows[i] != row_id");

            THROW_ON_RC ( VCursorRowId ( rcursor, &row_id ) );

            THROW_ON_RC ( VCursorFindNextRowIdDirect ( rcursor, 0, row_id + 1, &row_id ) );
            if ( rows[i] != row_id )
                FAIL("rows[i] != row_id");

            THROW_ON_RC ( VCursorSetRowId ( rcursor, row_id ) );
        }
        ReleaseDatabase(rcursor);
    }




    KDirectory* m_wd;

#if READ_ONLY
    const
#endif
    VDBManager* m_mgr;
    string m_databaseName;

#if !READ_ONLY
    VDatabase* m_wdb;
#endif
    const VDatabase* m_rdb;
};

FIXTURE_TEST_CASE ( SparseColNormal, VDB_Fixture)
{
    int64_t rows[] = { 3, 4, 5, 100, 1000000 };
    int rows_size = sizeof rows / sizeof rows[0];

    m_databaseName = ScratchDir + GetName();

#if !READ_ONLY
    WriteColumn ( rows, rows_size, "COL1", false );
#endif

    RunChecks ( rows, rows_size, "COL1" );
}

FIXTURE_TEST_CASE ( SparseColStatic, VDB_Fixture)
{
    int64_t rows[] = { 2, 3, 4, 5 };
    int rows_size = sizeof rows / sizeof rows[0];

    m_databaseName = ScratchDir + GetName();

#if !READ_ONLY
    WriteColumn ( rows, rows_size, "COL1", true );
#endif

    RunChecks ( rows, rows_size, "COL1" );
}

FIXTURE_TEST_CASE ( SparseColEmpty, VDB_Fixture)
{
    m_databaseName = ScratchDir + GetName();

#if !READ_ONLY
    int64_t rows[] = { 2, 3, 4, 5 };
    int rows_size = sizeof rows / sizeof rows[0];

    WriteColumn ( rows, rows_size, "COL1", true );
#endif

    const VCursor * rcursor = OpenDatabaseRead();
    REQUIRE_RC ( VCursorOpen ( rcursor ) );

    rc_t rc;
    int64_t row_id;
    rc = VCursorFindNextRowId ( rcursor, 0, &row_id );
    if ( rc == 0 || GetRCState ( rc ) != rcEmpty || GetRCObject ( rc ) != (int)rcCursor )
        FAIL("expected to fail with rcCursor rcNotFound");
    ReleaseDatabase(rcursor);
}


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

const char UsageDefaultName[] = "test-sparse-col";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=SparseColTestSuite(argc, argv);
    return rc;
}

}
