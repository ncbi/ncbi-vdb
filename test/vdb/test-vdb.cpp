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

#include "VDB_Fixture.hpp"

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/view.h>
#include <vdb/cursor.h>
#include <vdb/blob.h>
#include <vdb/vdb-priv.h>
#include <vdb/blob.h>

extern "C" {
    #include <../libs/vdb/blob-priv.h>
    #include <../libs/vdb/page-map.h>
}

#include <ktst/unit_test.hpp> // TEST_CASE
#include <klib/container.h>
#include <kfg/config.h>

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

TEST_SUITE( VdbTestSuite )

extern "C" rc_t VDBManagerRunPeriodicTasks(const VDBManager*);

TEST_CASE( VdbMgr ) {
    const VDBManager *mgr = NULL;
    rc_t rc = VDBManagerMakeRead(NULL, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerMakeRead(NULL) succeed" );

    rc = VDBManagerMakeRead(&mgr, NULL);
    if (rc != 0)
        FAIL( "failed to make VDB manager" );

    rc = VDBManagerAddRef(mgr);
    if (rc != 0)
        FAIL( "failed to AddRef VDB manager" );

    rc = VDBManagerRelease(mgr);
    if (rc != 0)
        FAIL( "failed to release VDB manager" );

    rc = VDBManagerVersion(mgr, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerVersion(mgr, NULL) succeed" );

    uint32_t version = 0;
    rc = VDBManagerVersion(NULL, &version);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerVersion(NULL) succeed" );

    rc = VDBManagerVersion(mgr, &version);
    if (rc == 0)
        CHECK_GE( version, (uint32_t)0x2000000 );
    else
        FAIL( "failed to get VDB manager Version" );

    rc = VDBManagerRunPeriodicTasks(NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerRunPeriodicTasks(NULL) succeed" );

    rc = VDBManagerRunPeriodicTasks(mgr);
    if (rc != 0)
        FAIL( "failed VDBManagerRunPeriodicTasks" );

    // In the following, NULL and "" are interpreted as "."

    // VDBManagerAddSchemaIncludePath
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, NULL));
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, ""));
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, "/"));
    REQUIRE_RC(VDBManagerAddSchemaIncludePath(mgr, "/usr"));

    // VDBManagerAddLoadLibraryPath
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, NULL));
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, ""));
#ifndef WINDOWS
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, "/"));
    REQUIRE_RC(VDBManagerAddLoadLibraryPath(mgr, "/usr"));
#else
    REQUIRE_RC_FAIL(VDBManagerAddLoadLibraryPath(mgr, "/"));
    REQUIRE_RC_FAIL(VDBManagerAddLoadLibraryPath(mgr, "/usr"));
#endif
    //

    struct Test { int i; };
    Test t;
    t.i = 12345;
    rc = VDBManagerGetUserData(mgr, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerGetUserData(mgr, NULL) succeed" );
    void *data = NULL;
    rc = VDBManagerGetUserData(NULL, &data);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerGetUserData(NULL) succeed" );
    rc = VDBManagerGetUserData(mgr, &data);
    if (rc != 0)
        FAIL( "failed VDBManagerGetUserData" );
    if (data != NULL)
        FAIL( "VDBManagerGetUserData != NULL" );

    rc = VDBManagerSetUserData(NULL, &t, free);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerSetUserData(NULL) succeed" );
    rc = VDBManagerSetUserData(mgr, &t, NULL);
    if (rc != 0)
        FAIL( "failed VDBManagerSetUserData" );
    rc = VDBManagerGetUserData(mgr, &data);
    if (rc != 0)
        FAIL( "failed VDBManagerGetUserData after Set" );
    if (data == NULL)
        FAIL( "VDBManagerGetUserData == NULL after Get" );
    else {
        Test *r = static_cast<Test*>(data);
        CHECK_EQUAL( r->i, 12345 );
    }

    Test *pt = static_cast<Test*>(malloc(sizeof *pt));
    rc = VDBManagerSetUserData(mgr, pt, free);
    if (rc != 0)
        FAIL( "failed VDBManagerSetUserData(free)" );

    //
    rc = VDBManagerRelease(mgr);
    if (rc != 0)
        FAIL( "failed to release VDB manager" );
    mgr = NULL;

}

#if CANT_WAIT_FOR_THIS_ONE_TO_FINISH_INVESTIGATE_AND_SPEEDUP
TEST_CASE(SimultaneousCursors)
{   // SRA-1669 WGS ALWZ01 cannot open multiple cursors with SEQUENCE.CONTIG_NAME column simultaneously (Win32)

    for ( int i = 0; i < 1; ++i ) {
        const VDBManager* mgr = 0;
        REQUIRE_RC(VDBManagerMakeRead(&mgr, 0));
        const VDatabase* db = 0;
        REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, 0, "ALWZ01"));
        const VTable* table = 0;
        REQUIRE_RC(VDatabaseOpenTableRead(db, &table, "SEQUENCE"));
        const size_t CURSOR_CNT = 10;
        const VCursor* cursors[CURSOR_CNT] = {};
        for ( size_t i = 0; i < CURSOR_CNT; ++i ) {
            const VCursor* cursor = 0;
            REQUIRE_RC(VTableCreateCursorRead(table, &cursor));
            cursors[i] = cursor;
            REQUIRE_RC(VCursorPermitPostOpenAdd(cursor));
            REQUIRE_RC(VCursorOpen(cursor));
            uint32_t col_index;

The following call takes an insane amount of time, abnout 3 minutes!
            REQUIRE_RC(VCursorAddColumn(cursor, &col_index, "CONTIG_NAME"));

        }
        for ( size_t i = 0; i < CURSOR_CNT; ++i ) {
            REQUIRE_RC(VCursorRelease(cursors[i]));
        }
        REQUIRE_RC(VTableRelease(table));
        REQUIRE_RC(VDatabaseRelease(db));
        REQUIRE_RC(VDBManagerRelease(mgr));
    }
}
#endif

FIXTURE_TEST_CASE(TestCursorIsStatic_SingleRowRun1, VDB_Fixture)
{
    static char const *columns[] = { "READ_LEN", 0 };
    REQUIRE_RC ( Setup ( "SRR002749", columns ) );
    bool is_static = false;
    REQUIRE_RC ( VCursorIsStaticColumn ( curs, col_idx[0], &is_static) );
    REQUIRE ( is_static );
}
FIXTURE_TEST_CASE(TestCursorIsStatic_VariableREAD_LEN, VDB_Fixture)
{
    static char const *columns[] = { "READ_LEN", 0 };
    REQUIRE_RC ( Setup ( "SRR050566", columns ) );
    bool is_static = true;
    REQUIRE_RC ( VCursorIsStaticColumn ( curs, col_idx[0], &is_static) );
    REQUIRE ( ! is_static );
}
#if 0
FIXTURE_TEST_CASE(TestCursorIsStatic_SingleRowRun2, VDB_Fixture)
{
    static char const *columns[] = { "READ_LEN", 0 };
    REQUIRE_RC ( Setup ( "SRR053325", columns ) );
    bool is_static = false;
    REQUIRE_RC ( VCursorIsStaticColumn ( curs, col_idx[0], &is_static) );
    REQUIRE ( is_static );
}
#endif
FIXTURE_TEST_CASE(TestCursorIsStatic_FixedREAD_LEN_MultipleRows, VDB_Fixture)
{
    static char const *columns[] = { "READ_LEN", 0 };
    REQUIRE_RC ( Setup ( "SRR125365", columns ) );
    bool is_static = false;
    REQUIRE_RC ( VCursorIsStaticColumn ( curs, col_idx[0], &is_static) );
    REQUIRE ( is_static );
}
FIXTURE_TEST_CASE(TestCursorIsStatic_DB_FixedREAD_LEN_MultipleRows, VDB_Fixture)
{
    static char const *columns[] = { "READ_LEN", 0 };
    REQUIRE_RC ( Setup ( "SRR600096", columns ) );
    bool is_static = false;
    REQUIRE_RC ( VCursorIsStaticColumn ( curs, col_idx[0], &is_static) );
    REQUIRE ( is_static );
}
FIXTURE_TEST_CASE(TestCursorIsStatic_DB_VariableREAD_LEN_MultipleRows, VDB_Fixture)
{
    static char const *columns[] = { "READ_LEN", 0 };
    REQUIRE_RC ( Setup ( "SRR619505", columns ) );
    bool is_static = true;
    REQUIRE_RC ( VCursorIsStaticColumn ( curs, col_idx[0], &is_static) );
    REQUIRE ( ! is_static );
}

/////////////////// VBlob

FIXTURE_TEST_CASE(VCursor_GetBlob_SRA, VDB_Fixture)
{   // multiple fragments per row (some are technical), multiple rows per blob
    static char const *columns[] = { "READ", 0 };
    REQUIRE_RC ( Setup ( "SRR000123", columns ) );

    {
        REQUIRE_RC ( VCursorSetRowId (curs, 1 ) );
        REQUIRE_RC ( VCursorOpenRow (curs ) );

        struct VBlob const *blob;
        REQUIRE_RC ( VCursorGetBlob ( curs, &blob, col_idx[0] ) );

        int64_t first;
        uint64_t count;
        REQUIRE_RC ( VBlobIdRange ( blob, &first, &count ) );
        REQUIRE_EQ ( (int64_t)1, first );
        REQUIRE_EQ ( (uint64_t)4, count );

        REQUIRE_EQ ( (size_t)858, BlobBufferBytes ( blob ) );

        //TODO: use row map to convert an offset into a rowId
        REQUIRE_NOT_NULL ( blob->pm );
        REQUIRE ( ! blob->pm->random_access );
        REQUIRE_EQ ( (int)PageMap::eBlobPageMapOptimizedNone, (int)blob->pm->optimized );

        REQUIRE_EQ ( (pm_size_t)4, blob->pm->leng_recs ); // not expanded ?
        REQUIRE_NOT_NULL ( blob->pm->length );   // array of lengths, size blob->pm->leng_recs
        REQUIRE_EQ ( (elem_count_t)157, blob->pm->length[0] );
        REQUIRE_EQ ( (elem_count_t)280, blob->pm->length[1] );
        REQUIRE_EQ ( (elem_count_t)168, blob->pm->length[2] );
        REQUIRE_EQ ( (elem_count_t)253, blob->pm->length[3] );

        REQUIRE_NOT_NULL ( blob->pm->leng_run ); // array of run lengths, size blob->pm->leng_recs
        REQUIRE_EQ ( (row_count_t)1, blob->pm->leng_run[0] );
        REQUIRE_EQ ( (row_count_t)1, blob->pm->leng_run[1] );
        REQUIRE_EQ ( (row_count_t)1, blob->pm->leng_run[2] );
        REQUIRE_EQ ( (row_count_t)1, blob->pm->leng_run[3] );
        //NOTE: very unlikely to happen for READ column, but if leng_run > 1, the corresponding fragments should all appear in the search results

        REQUIRE_EQ ( (pm_size_t)4, blob->pm->data_recs );

        REQUIRE_EQ ( (row_count_t)4,  blob->pm->row_count );
        REQUIRE_EQ ( (row_count_t)0,  blob->pm->pre_exp_row_count );

        REQUIRE_RC ( VCursorCloseRow (curs ) );
        REQUIRE_RC ( VBlobRelease ( (struct VBlob *) blob ) );
    }

    {
        REQUIRE_RC ( VCursorSetRowId (curs, 5 ) );
        REQUIRE_RC ( VCursorOpenRow (curs ) );

        struct VBlob const *blob;

        int64_t first;
        uint64_t count;
        REQUIRE_RC ( VCursorGetBlob ( curs, &blob, col_idx[0] ) );
        REQUIRE_RC ( VBlobIdRange ( blob, &first, &count ) );
        REQUIRE_EQ ( (int64_t)5, first );
        REQUIRE_EQ ( (uint64_t)4, count );

        REQUIRE_EQ ( (size_t)1221, BlobBufferBytes ( blob ) );

        //TODO: use row map to convert an offset into a rowId

        REQUIRE_RC ( VCursorCloseRow (curs ) );
        REQUIRE_RC ( VBlobRelease ( (struct VBlob *) blob ) );
    }
}

FIXTURE_TEST_CASE(VCursor_GetBlob_SequentialAccess, VDB_Fixture)
{   // VDB-2858: sequential access to blobs broken
    static char const *columns[] = { "READ", 0 };
    REQUIRE_RC ( Setup ( "ALAI01", columns ) );

    int64_t first;
    uint64_t count;

    REQUIRE_RC ( VCursorIdRange (curs, 0, &first, &count ) );

    int64_t rowId = 1;
    while (true)
    {
        REQUIRE_RC ( VCursorSetRowId ( curs, rowId ) );
        REQUIRE_RC ( VCursorOpenRow ( curs ) );

        struct VBlob const *blob;
        if ( VCursorGetBlob ( curs, &blob, col_idx[0] ) != 0 )
        {
            break;
        }

        REQUIRE_RC ( VBlobIdRange ( blob, &first, &count ) );
        REQUIRE_EQ ( rowId, first );

        REQUIRE_RC ( VCursorCloseRow (curs ) );
        REQUIRE_RC ( VBlobRelease ( (struct VBlob *) blob ) );

        rowId += count;
    }
}

class VdbBlobFixture : public VDB_Fixture
{
public:
    bool CheckBlobRange ( int64_t p_rowId, int64_t p_first, uint64_t p_count )
    {
        THROW_ON_RC ( VCursorSetRowId ( curs, p_rowId ) );
        THROW_ON_RC ( VCursorOpenRow ( curs ) );

        struct VBlob const *blob;
        THROW_ON_RC ( VCursorGetBlob ( curs, &blob, col_idx[0] ) );

        bool ret = true;

        int64_t first;
        uint64_t count;
        THROW_ON_RC ( VBlobIdRange ( blob, &first, &count ) );
        if ( p_first != first )
        {
            cout << "CheckBlobRange(" << p_rowId << " ) : expected first = " << p_first << ", actual = " << first << endl;
            ret = false;
        }
        if ( p_count != count )
        {
            cout << "CheckBlobRange(" << p_rowId << " ) : expected count = " << p_count << ", actual = " << count << endl;
            ret = false;
        }

        THROW_ON_RC ( VCursorCloseRow (curs ) );
        THROW_ON_RC ( VBlobRelease ( (struct VBlob *) blob ) );
        return ret;
    }
};

FIXTURE_TEST_CASE(VCursor_GetBlob_RandomAccess, VdbBlobFixture)
{
    static char const *columns[] = { "READ", 0 };
    REQUIRE_RC ( Setup ( "SRR000001", columns ) );

    // when accessing randomly, blob sizes stay very small
    REQUIRE ( CheckBlobRange ( 1, 1, 4 ) );
    REQUIRE ( CheckBlobRange ( 1000, 1000, 1 ) );
    REQUIRE ( CheckBlobRange ( 14, 14, 1 ) );
    REQUIRE ( CheckBlobRange ( 1200, 1200, 1 ) );
    REQUIRE ( CheckBlobRange ( 14000, 14000, 1 ) );
    REQUIRE ( CheckBlobRange ( 14001, 14001, 4 ) ); // sequential access starts growing the blob
    REQUIRE ( CheckBlobRange ( 1400, 1400, 1 ) ); // back to random
    REQUIRE ( CheckBlobRange ( 140000, 140000, 1 ) );
    REQUIRE ( CheckBlobRange ( 14001, 14001, 1 ) );
}

FIXTURE_TEST_CASE ( VCursor_FindNextRowIdDirect, VDB_Fixture )
{
    static char const *columns[] = { "SPOT_ID", "READ", 0 };
    REQUIRE_RC ( Setup ( "SRR000001", columns ) );
    int64_t next;
    REQUIRE_RC ( VCursorFindNextRowIdDirect ( curs, 0, 1, & next ) );
    REQUIRE_EQ ( (int64_t)1, next ) ;
    REQUIRE_RC ( VCursorFindNextRowIdDirect ( curs, 0, 2, & next ) );
    REQUIRE_EQ ( (int64_t)2, next ) ; // VDB-3075: next == 1
}

FIXTURE_TEST_CASE ( DoubleOpen, VDB_Fixture )
{
    const char SRR619505[] = "SRR619505";

    const VDatabase *db = NULL;
    REQUIRE_RC ( VDBManagerOpenDBRead ( mgr, &db, NULL, SRR619505 ) );
    const VTable * tbl;
    REQUIRE_RC ( VDatabaseOpenTableRead ( db, &tbl, "REFERENCE") );
    //DumpScope( tbl -> stbl -> scope, "Table" );
    REQUIRE_RC ( VTableRelease ( tbl ) );
    //DumpScope( db -> schema -> dad -> scope, "Db->dad" );
    //DumpScope( db -> schema -> scope, "Db" );

    // this used to corrupt the heap
    REQUIRE_RC ( VDatabaseOpenTableRead ( db, &tbl, "SEQUENCE") );
    // DumpScope( tbl -> stbl -> scope, "Table" );
    REQUIRE_RC ( VTableRelease ( tbl ) );

    REQUIRE_RC ( VDatabaseRelease ( db ) );
}

FIXTURE_TEST_CASE(VCursor_PermitPostOpenAdd, VDB_Fixture)
{
    static char const *columns[] = { "SPOT_ID", 0 };
    REQUIRE_RC(Setup("SRR000001", columns, false));
    REQUIRE_RC(VCursorPermitPostOpenAdd(curs));
}

#if TO_FIX_VDB_4470
static rc_t CC FlushSchema ( void *fd, const void * buffer, size_t size )
{
    string * strPtr = static_cast<string *>( fd );
    strPtr -> append ( static_cast<const char*>( buffer ), size );
    return 0;
}

FIXTURE_TEST_CASE(VersionProblem, VDB_Fixture)
{
    const string input =
    "version 1; table NCBI:tbl:base_space #3 {}; "
    " table NCBI:align:tbl:reference #3 = NCBI:tbl:base_space #3 {}; "
    " table NCBI:tbl:base_space #3.1 {};";

    string out;
    {
        VSchema * schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        REQUIRE_RC ( VDBManagerAddSchemaIncludePath ( mgr, "%s", "../../interfaces" ) );
        REQUIRE_RC ( VSchemaParseText ( schema, NULL, input . c_str(), input . size () ) );
        REQUIRE_RC ( VSchemaDump ( schema, sdmPrint, "NCBI:align:tbl:reference #3", FlushSchema, & out ) );
        VSchemaRelease ( schema );
    }
cout << out;
    {
        VSchema * schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseText ( schema, NULL, out . c_str(), out . size () ) );
        VSchemaRelease ( schema );
    }
}
#endif
FIXTURE_TEST_CASE ( V2ParserError, VDB_Fixture )
{   // This exercises an "extended schema" scenario, when a schema embedded
    // in a table gets tacked onto the exsiting schema, possibly creating namespaces
    // that obscure the parent schema (see callers of VSchemaParseTextCallback)
    const VTable *tbl = NULL;
    VSchema *schema = NULL;
    REQUIRE_RC ( VDBManagerMakeSRASchema(mgr, &schema) );
    REQUIRE_RC ( VDBManagerOpenTableRead ( mgr, &tbl, schema, "SRR053325" ) );
    REQUIRE_RC ( VTableCreateCursorRead(tbl, &curs) );
    uint32_t colIdx;
    REQUIRE_RC ( VCursorAddColumn ( curs, & colIdx, "READ" ) );
    REQUIRE_RC ( VCursorOpen (curs ) );
    REQUIRE_RC ( VSchemaRelease ( schema ) );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

rc_t CC FlushSchema(void *fd, const void * buffer, size_t size)
{
    ostream & out = *static_cast < ostream * > (fd);
    out.write(static_cast < const char * > (buffer), size);
    out.flush();
    return 0;
}

#include <sstream>
string
DumpSchema(const VSchema * p_schema)
{
    ostringstream out;
    if (VSchemaDump(p_schema, sdmPrint, 0, FlushSchema, &out) != 0)
    {
        throw runtime_error("DumpSchema failed");
    }
    return out.str();
}

TEST_CASE(View_On_An_Existing_Schema)
{
    const VDBManager * mgr;
    REQUIRE_RC(VDBManagerMakeRead(&mgr, NULL));

    const VDatabase *db = NULL;
    const char * acc = "SRR1063272";

    const string schemaText =
        "version 2; "
        "view V1#1 < NCBI:align:tbl:align_cmn align >"
        "{"
        "    column I64 SEQ_SPOT_ID = align.SEQ_SPOT_ID;"
        "}";

    VSchema * schema;
    REQUIRE_RC(VDBManagerOpenDBRead(mgr, &db, NULL, acc));
    REQUIRE_RC(VDatabaseOpenSchema(db, (const VSchema **)& schema));
    REQUIRE_RC(VSchemaParseText(schema, "", schemaText.c_str(), schemaText.size()));

    const VView * view;
    REQUIRE_RC(VDBManagerOpenView(mgr, &view, schema, "V1"));

    const VTable * tbl_pa;
    REQUIRE_RC(VDatabaseOpenTableRead(db, &tbl_pa, "PRIMARY_ALIGNMENT")); // this used to mess up VSchema

    string d = DumpSchema(schema);

    REQUIRE_RC(VTableRelease(tbl_pa));
    REQUIRE_RC(VViewRelease(view));
    REQUIRE_RC(VSchemaRelease(schema));

    REQUIRE_RC(VDatabaseRelease(db));
    REQUIRE_RC(VDBManagerRelease(mgr));
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-vdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbTestSuite(argc, argv);
    return rc;
}

}
