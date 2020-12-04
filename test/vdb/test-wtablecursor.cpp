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

#include <vdb/cursor.h>

#include <sstream>

#include <klib/rc.h>
#include <klib/symbol.h>

#include <vdb/table.h>
#include <vdb/vdb-priv.h>

#include <../libs/vdb/schema-priv.h>
#include <../libs/vdb/table-priv.h>
#include <../libs/vdb/cursor-priv.h>
#include <../libs/vdb/prod-priv.h>

#include "WVDB_Fixture.hpp"

using namespace std;

TEST_SUITE( VdbTableCursorTestSuite_Write )
const string ScratchDir = "./db/";

class TableCursorFixture : public WVDB_Fixture
{
public:
    TableCursorFixture()
    : m_cur ( 0 )
    {
    }

    ~TableCursorFixture()
    {
        if ( m_cur != 0 )
        {
            VCursorRelease ( m_cur );
        }
    }

    void MakeWriteCursor( const char * p_dbName, const char * p_schema )
    {
        m_databaseName = ScratchDir + p_dbName;
        const char * schemaSpec = "db";
        MakeDatabase ( p_schema, schemaSpec );

        VTable* table;
        THROW_ON_RC ( VDatabaseCreateTable ( m_db, & table, "t", kcmCreate | kcmMD5, "%s", "t" ) );
        THROW_ON_RC ( VTableCreateCursorWrite ( table, & m_cur, kcmInsert ) );
        THROW_ON_RC ( VTableRelease ( table ) );
    }

    void MakeWriteCursorAddColumn ( const char * p_dbName, const char * p_schema )
    {
        MakeWriteCursor( p_dbName, p_schema );
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "c" ) );
    }

    void MakeWriteCursorAddColumnOpen ( const char * p_dbName, const char * p_schema )
    {
        MakeWriteCursor( p_dbName, p_schema );
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "c" ) );
        THROW_ON_RC ( VCursorOpen ( m_cur ) );
    }

    void OpenRowWriteCommit ( const string & p_value )
    {
        THROW_ON_RC ( VCursorOpenRow ( m_cur ) );
        THROW_ON_RC ( VCursorWrite ( m_cur, m_columnIdx, 8,
                                    (const void*)p_value.data(), 0, p_value . length () ) );
        THROW_ON_RC ( VCursorCommitRow ( m_cur ) );
    }

    VCursor * m_cur;
    uint32_t  m_columnIdx;
};

// The original purpose of these tests was to cover all VTableCursor methods in the process of
// introducing a vtable, so the tests cases themselves are rather simplistic, just making sure
// methods are plugged in correctly (i.e. do not crash).
// Evolve as required.

static const char * SimpleSchema =
    "table T#1 { column ascii c; };"
    "database db #1 { table T#1 t; };"
;

FIXTURE_TEST_CASE( VTableCursor_MakeWrite, TableCursorFixture )
{
    m_databaseName = ScratchDir + GetName();
    MakeDatabase ( SimpleSchema, "db" );

    VTable* table;
    REQUIRE_RC ( VDatabaseCreateTable ( m_db, & table, "t", kcmCreate | kcmMD5, "%s", "t" ) );

    REQUIRE_RC ( VTableCreateCursorWrite ( table, & m_cur, kcmInsert ) );

    REQUIRE_RC ( VTableRelease ( table ) );
}

FIXTURE_TEST_CASE( VTableCursor_AddRef_Release, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );

    REQUIRE_RC ( VCursorAddRef ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) ); // use valgrind to find any leaks
}

FIXTURE_TEST_CASE( VTableCursor_AddColumn, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "c" ) );
    // also covers VCursorMakeColumn
}

FIXTURE_TEST_CASE( VTableCursor_GetColumnIdx, TableCursorFixture )
{
    MakeWriteCursorAddColumn ( GetName(), SimpleSchema );

    uint32_t idx;
    REQUIRE_RC ( VCursorGetColumnIdx ( m_cur, & idx, "%s", "c" ) );
    REQUIRE_EQ ( m_columnIdx, idx );
}

FIXTURE_TEST_CASE( VTableCursor_Datatype, TableCursorFixture )
{
    MakeWriteCursorAddColumn ( GetName(), SimpleSchema );

    struct VTypedecl type;
    struct VTypedesc desc;
    REQUIRE_RC ( VCursorDatatype ( m_cur, m_columnIdx, & type, & desc ) );
    REQUIRE_EQ ( 23u, type . type_id );// "ascii" index, see VSchemaInitTypes()
    REQUIRE_EQ ( 1u, type . dim);
    REQUIRE_EQ ( 8u, desc . intrinsic_bits);
    REQUIRE_EQ ( 1u, desc . intrinsic_dim);
    REQUIRE_EQ ( (uint32_t)ddAscii, desc . domain);
}

FIXTURE_TEST_CASE( VTableCursor_Open, TableCursorFixture )
{
    MakeWriteCursorAddColumn ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_IdRange, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    int64_t first = 0;
    uint64_t count = 0;
    REQUIRE_RC ( VCursorIdRange ( m_cur, m_columnIdx, & first, & count ) );
    REQUIRE_EQ((int64_t)1, first);
    REQUIRE_EQ((uint64_t)0, count);
}

FIXTURE_TEST_CASE( VTableCursor_RowId, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    int64_t rowId = 0;
    REQUIRE_RC ( VCursorRowId ( m_cur, & rowId ) );
    REQUIRE_EQ((int64_t)1, rowId);
}

FIXTURE_TEST_CASE( VTableCursor_SetRowId, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    const int64_t RowId = 22;
    REQUIRE_RC ( VCursorSetRowId ( m_cur, RowId ) );
    int64_t rowId = 0;
    REQUIRE_RC ( VCursorRowId ( m_cur, & rowId ) );
    REQUIRE_EQ( RowId, rowId );
}

FIXTURE_TEST_CASE( VTableCursor_FindNextRowId_Empty, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    int64_t rowId;
    rc_t rc = VCursorFindNextRowId ( m_cur, m_columnIdx, & rowId );
    REQUIRE_EQ ( GetRCState ( rc ), rcNotOpen );
}

FIXTURE_TEST_CASE( VTableCursor_FindNextRowIdDirect_Empty, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    int64_t rowId;
    rc_t rc = VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 100, & rowId );
    REQUIRE_EQ ( GetRCState ( rc ), rcNotOpen );
}

FIXTURE_TEST_CASE( VTableCursor_OpenRow, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_Write, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    const char * buf = "abc";
    REQUIRE_RC ( VCursorWrite ( m_cur, m_columnIdx, 8, (const void*)buf, 0, sizeof( buf ) ) );
}

FIXTURE_TEST_CASE( VTableCursor_CommitRow, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    const char * buf = "abc";
    REQUIRE_RC ( VCursorWrite ( m_cur, m_columnIdx, 8, (const void*)buf, 0, sizeof( buf ) ) );

    REQUIRE_RC ( VCursorCommitRow ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_CloseRow, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    OpenRowWriteCommit("abc");

    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_RepeatRow, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    OpenRowWriteCommit("abc");

    REQUIRE_RC ( VCursorRepeatRow ( m_cur, 3 ) );
}

FIXTURE_TEST_CASE( VTableCursor_FlushPage, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    OpenRowWriteCommit("abc");
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );

    REQUIRE_RC ( VCursorFlushPage ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetBlob, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    struct VBlob const * blob;
    rc_t rc = VCursorGetBlob ( m_cur, & blob, m_columnIdx );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_GetBlobDirect, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    struct VBlob const * blob;
    rc_t rc = VCursorGetBlobDirect ( m_cur, & blob, 1,m_columnIdx );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_Read, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    char buf[10];
    uint32_t row_len;
    rc_t rc = VCursorRead ( m_cur, m_columnIdx, 8, buf, sizeof ( buf ), & row_len );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_ReadDirect, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    char buf[10];
    uint32_t row_len;
    rc_t rc = VCursorReadDirect ( m_cur, 1, m_columnIdx, 8, buf, sizeof ( buf ), & row_len );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_ReadBits, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    char buf[10];
    uint32_t num_read;
    uint32_t remaining;
    rc_t rc = VCursorReadBits ( m_cur, m_columnIdx, 8, 0, buf, 0, sizeof ( buf ), & num_read, & remaining );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_ReadBitsDirect, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    char buf[10];
    uint32_t num_read;
    uint32_t remaining;
    rc_t rc = VCursorReadBitsDirect ( m_cur, 1, m_columnIdx, 8, 0, buf, 0, sizeof ( buf ), & num_read, & remaining );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_CellData, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    uint32_t elem_bits;
    const void * base;
    uint32_t boff;
    uint32_t row_len;
    rc_t rc = VCursorCellData ( m_cur, m_columnIdx, & elem_bits, & base, & boff, & row_len );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_CellDataDirect, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    uint32_t elem_bits;
    const void * base;
    uint32_t boff;
    uint32_t row_len;
    rc_t rc = VCursorCellDataDirect ( m_cur, 1, m_columnIdx, & elem_bits, & base, & boff, & row_len );
    REQUIRE_EQ ( GetRCState ( rc ), rcWriteonly );
}

FIXTURE_TEST_CASE( VTableCursor_DataPrefetch, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    const int64_t row_ids[3] = {1, 2, 3};
    REQUIRE_RC ( VCursorDataPrefetch ( m_cur, row_ids, m_columnIdx, 3, 1, 3, true ) );
}

FIXTURE_TEST_CASE( VTableCursor_Default, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    const char buf[4] = "abc";
    REQUIRE_RC ( VCursorDefault ( m_cur, m_columnIdx, 8, buf, 0, 3 ) );
}

FIXTURE_TEST_CASE( VTableCursor_Commit, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    OpenRowWriteCommit("abc");
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );

    REQUIRE_RC ( VCursorCommit ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_OpenParentRead, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    const VTable * tbl = 0;
    REQUIRE_RC ( VCursorOpenParentRead ( m_cur, & tbl ) );
    REQUIRE_NOT_NULL ( tbl );
    REQUIRE_RC ( VTableRelease ( (VTable*)tbl ) );
}

FIXTURE_TEST_CASE( VTableCursor_OpenParentUpdate, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    VTable * tbl = 0;
    REQUIRE_RC ( VCursorOpenParentUpdate ( m_cur, & tbl ) );
    REQUIRE_NOT_NULL ( tbl );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetUserData, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    void * data;
    REQUIRE_RC ( VCursorGetUserData ( m_cur, & data ) );
    REQUIRE_NULL ( data );
}

FIXTURE_TEST_CASE( VTableCursor_SetUserData, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorSetUserData ( m_cur, 0, 0 ) );
}

FIXTURE_TEST_CASE( VTableCursor_PermitPostOpenAdd, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorPermitPostOpenAdd ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_SuspendTriggers, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorSuspendTriggers ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetSchema, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE_NOT_NULL ( VCursorGetSchema ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_PageIdRange, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    OpenRowWriteCommit("abc");
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    OpenRowWriteCommit("def");
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    REQUIRE_RC ( VCursorCommit ( m_cur ) );

    int64_t first=-1;
    int64_t last=-1;
    REQUIRE_RC ( VCursorPageIdRange ( m_cur, m_columnIdx, 1, & first, & last ) );
    REQUIRE_EQ((int64_t)1, first);
    REQUIRE_EQ((int64_t)2, last);
}

FIXTURE_TEST_CASE( VTableCursor_IsStaticColumn, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    OpenRowWriteCommit("abc");
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    OpenRowWriteCommit("abc");
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    REQUIRE_RC ( VCursorCommit ( m_cur ) );

    bool is_static = false;
    REQUIRE_RC ( VCursorIsStaticColumn ( m_cur, m_columnIdx, & is_static ) );
    REQUIRE ( is_static );
}

FIXTURE_TEST_CASE( VTableCursor_LinkedCursorGet, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    const VCursor * curs = 0;
    REQUIRE_RC_FAIL ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
}

FIXTURE_TEST_CASE( VTableCursor_LinkedCursorSet, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    // reuse m_cur
    REQUIRE_RC ( VCursorLinkedCursorSet ( m_cur, "tbl", m_cur ) ); // calls AddRef(m_cur)
    REQUIRE_RC ( VCursorRelease ( m_cur ) );

    const VCursor * curs = 0;
    REQUIRE_RC ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
    REQUIRE_EQ ( (const VCursor*)m_cur, curs );
    REQUIRE_RC ( VCursorRelease ( curs ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetCacheCapacity, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE_EQUAL((uint64_t)0, VCursorGetCacheCapacity(m_cur));
}

FIXTURE_TEST_CASE( VTableCursor_SetCacheCapacity, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );

    // this cursor is not cached, so SetCapacity has no effect
    REQUIRE_EQUAL((uint64_t)0, VCursorSetCacheCapacity((VCursor*)m_cur, 100));
    REQUIRE_EQUAL((uint64_t)0, VCursorGetCacheCapacity(m_cur));
}

FIXTURE_TEST_CASE( VTableCursor_Columns, TableCursorFixture )
{
    MakeWriteCursorAddColumn ( GetName(), SimpleSchema );
    VCursorCache * cols = VCursorColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 1u, VectorLength ( & cols -> cache ) );
}

FIXTURE_TEST_CASE( VTableCursor_PhysicalColumns, TableCursorFixture )
{
    MakeWriteCursorAddColumn ( GetName(), SimpleSchema );
    VCursorCache * cols = VCursorPhysicalColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 0u, VectorLength ( & cols -> cache ) );
}

// VCursorMakeColumn is covered indirectly (see VTableCursor_AddColumn)

FIXTURE_TEST_CASE( VTableCursor_GetRow, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    Vector * row = VCursorGetRow ( (VCursor*)m_cur );
    REQUIRE_EQ ( 1u, VectorLength ( row ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetTable, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );

    const VTable * tbl = VCursorGetTable ( m_cur );
    REQUIRE_NOT_NULL ( tbl );
    // Oddly, there is no easy way to get the database members's name (t)
    // through the VTable's API. Easier to get to it's schema type name (T),
    // which suffices here:
    REQUIRE_EQ ( string ( "T" ), ToCppString ( tbl -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE( VTableCursor_IsReadOnly, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE ( ! VCursorIsReadOnly ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetBlobMruCache, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    // this cursor is not cached
    REQUIRE_NULL ( VCursorGetBlobMruCache ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_IncrementPhysicalProductionCount, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );
    REQUIRE_EQ ( 1u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
    REQUIRE_EQ ( 2u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_FindOverride, TableCursorFixture )
{
    const char * SchemaWithOverrides =
    "table T0#1 { column ascii c=x; };"
    "table T#1 = T0 { column U8 c=2; ascii x=\"a\"; };"
    "database db #1 { table T#1 t; };"
    ;

    MakeWriteCursor ( GetName(), SchemaWithOverrides );
    VCtxId id = {0, 1}; // 0 is the id of T0 which first introduced x, 1 is its column id in T
    const KSymbol * sym = VCursorFindOverride ( m_cur, & id );
    REQUIRE_NOT_NULL ( sym );
    REQUIRE_EQ ( string ("x"), ToCppString ( sym -> name ) );
}

FIXTURE_TEST_CASE( VTableCursor_LaunchPagemapThread, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_PageMapProcessRequest, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) );
    const PageMapProcessRequest * req = VCursorPageMapProcessRequest ( m_cur );
    REQUIRE_NOT_NULL ( req );
}

FIXTURE_TEST_CASE( VTableCursor_CacheActive, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    int64_t end;
    REQUIRE ( ! VCursorCacheActive ( m_cur, & end ) );
    REQUIRE_EQ((int64_t)0, end);
}

FIXTURE_TEST_CASE( VTableCursor_InstallTrigger, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );
    VProduction prod;
    REQUIRE_RC ( VCursorInstallTrigger ( (VCursor*)m_cur, & prod ) );
}

//////////////////////////////////////////// Main
#include <kfg/config.h>

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

const char UsageDefaultName[] = "test-wtablecursor";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbTableCursorTestSuite_Write(argc, argv);
    return rc;
}

}
