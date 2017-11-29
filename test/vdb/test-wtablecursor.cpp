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
#include <vdb/table.h>

#include <../libs/vdb/schema-priv.h>

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

FIXTURE_TEST_CASE( VTableCursor_AddRef, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );

    REQUIRE_RC ( VCursorAddRef ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) ); // use valgrind to find any leaks
}

FIXTURE_TEST_CASE( VTableCursor_AddColumn, TableCursorFixture )
{
    MakeWriteCursor ( GetName(), SimpleSchema );

    REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "c" ) );
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
    REQUIRE_EQ( 1l, first );
    REQUIRE_EQ( 0lu, count );
}

FIXTURE_TEST_CASE( VTableCursor_RowId, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    int64_t rowId = 0;
    REQUIRE_RC ( VCursorRowId ( m_cur, & rowId ) );
    REQUIRE_EQ( 1l, rowId );
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

    struct VTable const * tbl = 0;
    REQUIRE_RC ( VCursorOpenParentRead ( m_cur, & tbl ) );
    REQUIRE_NOT_NULL ( tbl );
}

FIXTURE_TEST_CASE( VTableCursor_OpenParentUpdate, TableCursorFixture )
{
    MakeWriteCursorAddColumnOpen ( GetName(), SimpleSchema );

    struct VTable * tbl = 0;
    REQUIRE_RC ( VCursorOpenParentUpdate ( m_cur, & tbl ) );
    REQUIRE_NOT_NULL ( tbl );
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
