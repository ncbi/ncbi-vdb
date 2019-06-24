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
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/vdb-priv.h>

#include <../libs/vdb/schema-priv.h>
#include <../libs/vdb/table-priv.h>
#include <../libs/vdb/cursor-priv.h>
#include <../libs/vdb/prod-priv.h>

#include <ktst/unit_test.hpp>

using namespace std;

TEST_SUITE( VdbTableCursorTestSuite_Read );
const string ScratchDir = "./db/";

class TableCursorFixture
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

    void MakeReadCursor( const char * p_dbName )
    {
        const VDBManager * mgr;
        THROW_ON_RC ( VDBManagerMakeRead ( & mgr, NULL ) );
        const VDatabase * db;
        THROW_ON_RC ( VDBManagerOpenDBRead ( mgr, & db, NULL, "%s", p_dbName ) );
        const VTable * tbl;
        THROW_ON_RC ( VDatabaseOpenTableRead ( db, & tbl, "SEQUENCE" ) );
        THROW_ON_RC ( VTableCreateCursorRead ( tbl, & m_cur ) );
        THROW_ON_RC ( VTableRelease ( tbl ) );
        THROW_ON_RC ( VDatabaseRelease ( db ) );
        THROW_ON_RC ( VDBManagerRelease ( mgr ) );
    }

    void MakeReadCursorAddColumn( const char * p_dbName, const char * p_colName )
    {
        MakeReadCursor ( p_dbName );
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", p_colName ) );
    }

    void MakeReadCursorAddColumnOpen( const char * p_dbName, const char * p_colName )
    {
        MakeReadCursor ( p_dbName );
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", p_colName ) );
        THROW_ON_RC ( VCursorOpen ( m_cur ) );
    }

    static std :: string ToCppString ( const String & p_str)
    {
        return std :: string ( p_str . addr, p_str . len );
    }

    const VCursor * m_cur;
    uint32_t  m_columnIdx;
};

const char * Accession = "SRR600096";
const char * Column = "SPOT_GROUP";

// The original purpose of these tests was to cover all VTableCursor methods in the process of
// introducing a vtable, so the tests cases themselves are rather simplistic, just making sure
// methods are plugged in correctly (i.e. do not crash).
// Evolve as required.

FIXTURE_TEST_CASE( VTableCursor_MakeRead, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_NOT_NULL ( m_cur );
}

FIXTURE_TEST_CASE( VTableCursor_AddRef, TableCursorFixture )
{
    MakeReadCursor ( Accession );

    REQUIRE_RC ( VCursorAddRef ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) ); // use valgrind to find any leaks
}

FIXTURE_TEST_CASE( VTableCursor_AddColumn, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", Column ) );
    // also covers VCursorMakeColumn
}

FIXTURE_TEST_CASE( VTableCursor_GetColumnIdx, TableCursorFixture )
{
    MakeReadCursorAddColumn ( Accession, Column );

    uint32_t idx;
    REQUIRE_RC ( VCursorGetColumnIdx ( m_cur, & idx, "%s", Column ) );
    REQUIRE_EQ ( m_columnIdx, idx );
}

FIXTURE_TEST_CASE( VTableCursor_Datatype, TableCursorFixture )
{
    MakeReadCursorAddColumn ( Accession, Column );

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
    MakeReadCursorAddColumn ( Accession, Column );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_IdRange, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    int64_t first = 0;
    uint64_t count = 0;
    REQUIRE_RC ( VCursorIdRange ( m_cur, m_columnIdx, & first, & count ) );
    REQUIRE_EQ( (int64_t)1, first );
    REQUIRE_EQ((uint64_t)16, count);
}

FIXTURE_TEST_CASE( VTableCursor_RowId, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    int64_t rowId = 0;
    REQUIRE_RC ( VCursorRowId ( m_cur, & rowId ) );
    REQUIRE_EQ( (int64_t)1, rowId );
}

FIXTURE_TEST_CASE( VTableCursor_SetRowId, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    const int64_t RowId = 22;
    REQUIRE_RC ( VCursorSetRowId ( m_cur, RowId ) );
    int64_t rowId = 0;
    REQUIRE_RC ( VCursorRowId ( m_cur, & rowId ) );
    REQUIRE_EQ( RowId, rowId );
}

FIXTURE_TEST_CASE( VTableCursor_FindNextRowId, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    int64_t rowId;
    REQUIRE_RC ( VCursorFindNextRowId ( m_cur, m_columnIdx, & rowId ) );
    REQUIRE_EQ((int64_t)2, rowId);
}

FIXTURE_TEST_CASE( VTableCursor_FindNextRowIdDirect_Empty, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    int64_t rowId;
    REQUIRE_RC ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 10, & rowId ) );
    REQUIRE_EQ((int64_t)10, rowId);
}

FIXTURE_TEST_CASE( VTableCursor_OpenRow, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_Write, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    const char * buf = "abc";
    rc_t rc = VCursorWrite ( (VCursor*)m_cur, m_columnIdx, 8, (const void*)buf, 0, sizeof( buf ) );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
}

FIXTURE_TEST_CASE( VTableCursor_CommitRow, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    rc_t rc = VCursorCommitRow ( (VCursor*)m_cur );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
}

FIXTURE_TEST_CASE( VTableCursor_CloseRow, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_RepeatRow, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    rc_t rc = VCursorRepeatRow ( (VCursor*)m_cur, 3 );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
}

FIXTURE_TEST_CASE( VTableCursor_FlushPage, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    rc_t rc = VCursorFlushPage ( (VCursor*)m_cur );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
}

FIXTURE_TEST_CASE( VTableCursor_GetBlob, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    const VBlob * blob;
    REQUIRE_RC ( VCursorGetBlob ( m_cur, & blob, m_columnIdx ) );
    REQUIRE_RC ( VBlobRelease ( (VBlob *) blob ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetBlobDirect, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    const VBlob * blob;
    REQUIRE_RC ( VCursorGetBlobDirect ( m_cur, & blob, 1, m_columnIdx ) );
    REQUIRE_RC ( VBlobRelease ( (VBlob *) blob ) );
}

FIXTURE_TEST_CASE( VTableCursor_Read, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    char buf[1024];
    uint32_t row_len;
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, buf, sizeof ( buf ), & row_len ) );
}

FIXTURE_TEST_CASE( VTableCursor_ReadDirect, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    char buf[1024];
    uint32_t row_len;
    REQUIRE_RC ( VCursorReadDirect ( m_cur, 1, m_columnIdx, 8, buf, sizeof ( buf ), & row_len ) );
}

FIXTURE_TEST_CASE( VTableCursor_ReadBits, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    char buf[1024];
    uint32_t num_read;
    uint32_t remaining;
    REQUIRE_RC ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, buf, 0, sizeof ( buf ), & num_read, & remaining ) );
}

FIXTURE_TEST_CASE( VTableCursor_ReadBitsDirect, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    char buf[1024];
    uint32_t num_read;
    uint32_t remaining;
    REQUIRE_RC ( VCursorReadBitsDirect ( m_cur, 1, m_columnIdx, 8, 0, buf, 0, sizeof ( buf ), & num_read, & remaining ) );
}

FIXTURE_TEST_CASE( VTableCursor_CellData, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );

    uint32_t elem_bits;
    const void * base;
    uint32_t boff;
    uint32_t row_len;
    REQUIRE_RC ( VCursorCellData ( m_cur, m_columnIdx, & elem_bits, & base, & boff, & row_len ) );
}

FIXTURE_TEST_CASE( VTableCursor_CellDataDirect, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    uint32_t elem_bits;
    const void * base;
    uint32_t boff;
    uint32_t row_len;
    REQUIRE_RC ( VCursorCellDataDirect ( m_cur, 1, m_columnIdx, & elem_bits, & base, & boff, & row_len ) );
}

FIXTURE_TEST_CASE( VTableCursor_DataPrefetch, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    const int64_t row_ids[3] = {1, 2, 3};
    REQUIRE_RC ( VCursorDataPrefetch ( m_cur, row_ids, m_columnIdx, 3, 1, 3, true ) );
}

FIXTURE_TEST_CASE( VTableCursor_Default, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    const char buf[4] = "abc";
    rc_t rc = VCursorDefault ( (VCursor*)m_cur, m_columnIdx, 8, buf, 0, 3 );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
}

FIXTURE_TEST_CASE( VTableCursor_Commit, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    rc_t rc = VCursorCommit ( (VCursor*)m_cur );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
}

FIXTURE_TEST_CASE( VTableCursor_OpenParentRead, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    const VTable * tbl = 0;
    REQUIRE_RC ( VCursorOpenParentRead ( m_cur, & tbl ) );
    REQUIRE_NOT_NULL ( tbl );
    REQUIRE_RC ( VTableRelease ( (VTable*) tbl ) );
}

FIXTURE_TEST_CASE( VTableCursor_OpenParentUpdate, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    VTable * tbl = 0;
    rc_t rc = VCursorOpenParentUpdate ( (VCursor*)m_cur, & tbl );
    REQUIRE_EQ ( GetRCState ( rc ), rcReadonly );
    REQUIRE_RC ( VTableRelease ( tbl ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetUserData, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    void * data;
    REQUIRE_RC ( VCursorGetUserData ( m_cur, & data ) );
    REQUIRE_NULL ( data );
}

FIXTURE_TEST_CASE( VTableCursor_SetUserData, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorSetUserData ( m_cur, 0, 0 ) );
}

FIXTURE_TEST_CASE( VTableCursor_PermitPostOpenAdd, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_RC ( VCursorPermitPostOpenAdd ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_SuspendTriggers, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_RC ( VCursorSuspendTriggers ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetSchema, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_NOT_NULL ( VCursorGetSchema ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_PageIdRange, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    int64_t first=-1;
    int64_t last=-1;
    REQUIRE_RC ( VCursorPageIdRange ( m_cur, m_columnIdx, 1, & first, & last ) );
    REQUIRE_EQ ( (int64_t)1, first );
    REQUIRE_EQ( (int64_t)16, last);
}

FIXTURE_TEST_CASE( VTableCursor_IsStaticColumn, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    bool is_static = false;
    REQUIRE_RC ( VCursorIsStaticColumn ( m_cur, m_columnIdx, & is_static ) );
    REQUIRE ( is_static );
}

FIXTURE_TEST_CASE( VTableCursor_LinkedCursorGet, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    const VCursor * curs = 0;
    REQUIRE_RC_FAIL ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
}

FIXTURE_TEST_CASE( VTableCursor_LinkedCursorSet, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    // reuse m_cur
    REQUIRE_RC ( VCursorLinkedCursorSet ( m_cur, "tbl", m_cur ) ); // calls AddRef(m_cur)
    REQUIRE_RC ( VCursorRelease ( m_cur ) );

    const VCursor * curs = 0;
    REQUIRE_RC ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
    REQUIRE_EQ ( m_cur, curs );
    REQUIRE_RC ( VCursorRelease ( curs ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetCacheCapacity, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_EQUAL((uint64_t)0, VCursorGetCacheCapacity(m_cur));
}

FIXTURE_TEST_CASE( VTableCursor_SetCacheCapacity, TableCursorFixture )
{
    MakeReadCursor ( Accession );

    // this cursor is not cached, so SetCapacity has no effect
    REQUIRE_EQUAL((uint64_t)0, VCursorSetCacheCapacity((VCursor*)m_cur, 100));
    REQUIRE_EQUAL((uint64_t)0, VCursorGetCacheCapacity(m_cur));
}

FIXTURE_TEST_CASE( VTableCursor_Columns, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    VCursorCache * cols = VCursorColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 6u, VectorLength ( & cols -> cache ) );
}

FIXTURE_TEST_CASE( VTableCursor_PhysicalColumns, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    VCursorCache * cols = VCursorPhysicalColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 6u, VectorLength ( & cols -> cache ) );
}

// VCursorMakeColumn is covered indirectly (see VTableCursor_AddColumn)

FIXTURE_TEST_CASE( VTableCursor_GetRow, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    Vector * row = VCursorGetRow ( (VCursor*)m_cur );
    REQUIRE_EQ ( 1u, VectorLength ( row ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetTable, TableCursorFixture )
{
    MakeReadCursor ( Accession );

    const VTable * tbl = VCursorGetTable ( m_cur );
    REQUIRE_NOT_NULL ( tbl );
    // Oddly, there is no easy way to get the database members's name (SEQUENCE)
    // through the VTable's API. Easier to get to it's schema type name (seq),
    // which suffices here:
    REQUIRE_EQ ( string ( "seq" ), ToCppString ( tbl -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE( VTableCursor_IsReadOnly, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE ( VCursorIsReadOnly ( m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_GetBlobMruCache, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    // this cursor is not cached
    REQUIRE_NULL ( VCursorGetBlobMruCache ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_IncrementPhysicalProductionCount, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    REQUIRE_EQ ( 1u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
    REQUIRE_EQ ( 2u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_FindOverride, TableCursorFixture )
{
    MakeReadCursor ( Accession );
    VCtxId id = {5, 3};
    const KSymbol * sym = VCursorFindOverride ( m_cur, & id );
    REQUIRE_NOT_NULL ( sym );
    REQUIRE_EQ ( string ("out_spot_group"), ToCppString ( sym -> name ) );
}

FIXTURE_TEST_CASE( VTableCursor_LaunchPagemapThread, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( VTableCursor_PageMapProcessRequest, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );

    // On the read side, VCursorLaunchPagemapThread has no effect in the first 200 calls
    // per non-cached cursor (see VTableCreateCachedCursorReadImpl() in cursor-table.c)
    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) );
    const PageMapProcessRequest * req = VCursorPageMapProcessRequest ( m_cur );
    // VDB-3652: VCursorPageMapProcessRequest returns a non-NULL value even if the thread is not running
    REQUIRE_NOT_NULL ( req );
    REQUIRE_NULL ( req -> pm );
}

FIXTURE_TEST_CASE( VTableCursor_CacheActive, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    int64_t end;
    REQUIRE ( ! VCursorCacheActive ( m_cur, & end ) );
    REQUIRE_EQ((int64_t)0, end);
}

FIXTURE_TEST_CASE( VTableCursor_InstallTrigger, TableCursorFixture )
{
    MakeReadCursorAddColumnOpen ( Accession, Column );
    VProduction prod;
    REQUIRE_RC_FAIL ( VCursorInstallTrigger ( (VCursor*)m_cur, & prod ) ); // write side only
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

const char UsageDefaultName[] = "test-tablecursor";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbTableCursorTestSuite_Read(argc, argv);
    return rc;
}

}
