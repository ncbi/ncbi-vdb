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

#include <ktst/unit_test.hpp> // TEST_CASE

#include <klib/symbol.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/view.h>
#include <vdb/blob.h>
#include <vdb/vdb-priv.h>

#include "../libs/vdb/database-priv.h"
#include "../libs/vdb/schema-priv.h"
#include "../libs/vdb/table-priv.h"
#include "../libs/vdb/cursor-priv.h"
#include "../libs/vdb/prod-priv.h"
#include "../libs/vdb/column-priv.h"

#include "../libs/schema/SchemaParser.hpp"
#include "../libs/schema/ASTBuilder.hpp"

#include "WVDB_Fixture.hpp"

using namespace std;
using namespace ncbi :: SchemaParser;

TEST_SUITE( ViewCursorTestSuite )

#include <ktst/unit_test.hpp> // THROW_ON_RC

const string ScratchDir = "./db/";

static const char * SchemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"

    "view ViewOnTable#1 < T p_tbl > "
    "{ "
    "   column ascii c = p_tbl . c1; "
    "   column ascii cc = \"\"; "
    "};"

    "view ViewOnView#1 < ViewOnTable p_v > { column ascii c = p_v . c; };"
    ;

static const char* TableName = "t";
static const char* TableColumnName = "c1";

static const char* ViewOnTableName = "ViewOnTable";
static const char* ViewOnViewName = "ViewOnView";
static const char* TableParamName = "p_tbl";
static const char* ViewParamName = "p_v";
static const char* ViewColumnName = "c";

class ViewCursorFixture : public WVDB_v2_Fixture
{
public:
    ViewCursorFixture()
    :   m_schemaText ( SchemaText ),
        m_view ( 0 ),
        m_table ( 0 ),
        m_cur ( 0 ),
        m_blob ( 0 ),
        m_base ( 0 )
    {
    }
    ~ViewCursorFixture()
    {
        VBlobRelease ( (VBlob*) m_blob );
        VCursorRelease ( m_cur );
        VTableRelease ( m_table );
        VViewRelease ( m_view );
    }

    VCursor * CreateTableOpenWriteCursor( const string & p_tableName, const string & p_columnName )
    {
        VCursor * ret = CreateTable ( p_tableName . c_str () );
        THROW_ON_RC ( VCursorAddColumn ( ret, & m_columnIdx, p_columnName . c_str () ) );
        THROW_ON_RC ( VCursorOpen ( ret ) );
        return ret;
    }

    void CreateDb ( const string & p_testName )
    {
        m_databaseName = ScratchDir + p_testName;
        MakeDatabase ( m_schemaText, "DB" );
        VCursor * cursor = CreateTableOpenWriteCursor ( TableName, TableColumnName );
        // insert some rows
        WriteRow ( cursor, m_columnIdx, "blah" );
        WriteRow ( cursor, m_columnIdx, "eeee" );
        THROW_ON_RC ( VCursorCommit ( cursor ) );
        THROW_ON_RC ( VCursorRelease ( cursor ) );
    }

    uint32_t AddColumn ( const char * p_colName )
    {
        uint32_t ret;
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & ret, "%s", p_colName ) );
        return ret;
    }

    string ReadAscii ( int64_t p_row, uint32_t p_colIdx )
    {
        THROW_ON_RC ( VCursorReadDirect ( m_cur, p_row, p_colIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
        return string ( m_buf, m_rowLen );
    }

    string          m_schemaText;
    const VView *   m_view;
    const VTable *  m_table;
    const VCursor * m_cur;
    uint32_t        m_columnIdx;
    const VBlob *   m_blob;
    char            m_buf [1024];
    uint32_t        m_rowLen;
    const void *    m_base;
    uint32_t        m_elemBits;
    uint32_t        m_boff;
};

class ViewOnTableCursorFixture : public ViewCursorFixture
{
public:
    void CreateCursor ( const string & p_testName, const char * p_viewName, bool p_bind = true )
    {
        CreateDb ( p_testName );
        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, p_viewName ) );
        if ( p_bind )
        {
            THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName ) );
            THROW_ON_RC ( VViewBindParameterTable ( m_view, TableParamName, m_table ) );
        }
        THROW_ON_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    }

    void CreateCursorAddColumn ( const string & p_testName,
                                 const char *   p_viewName,
                                 const char *   p_colName )
    {
        CreateCursor ( p_testName, p_viewName );
        m_columnIdx = AddColumn ( p_colName );
    }

    void CreateCursorOpen ( const string & p_testName,
                            const char *   p_viewName,
                            const char *   p_colName )
    {
        CreateCursorAddColumn ( p_testName, p_viewName, p_colName );
        THROW_ON_RC ( VCursorOpen ( m_cur ) );
    }

    void CreateCursorOpenRow ( const string & p_testName,
                               const char *   p_viewName,
                               const char *   p_colName )
    {
        CreateCursorOpen ( p_testName, p_viewName, p_colName );
        THROW_ON_RC ( VCursorOpenRow ( m_cur ) );
    }

    void CreateTablePopulate ( const char *     p_tableName,
                               const char *     p_colName,
                               const string &   p_val1,
                               const string &   p_val2  )
    {
        VCursor * cur = CreateTableOpenWriteCursor( p_tableName, p_colName );
        WriteRow ( cur, m_columnIdx, p_val1 );
        WriteRow ( cur, m_columnIdx, p_val2 );
        THROW_ON_RC ( VCursorCommit ( cur ) );
        THROW_ON_RC ( VCursorRelease ( cur ) );
    }
    void CreateTablePopulate ( const char *     p_tableName,
                               const char *     p_colName,
                               int64_t   p_val1,
                               int64_t   p_val2  )
    {
        VCursor * cur = CreateTableOpenWriteCursor( p_tableName, p_colName );
        WriteRow ( cur, m_columnIdx, p_val1 );
        WriteRow ( cur, m_columnIdx, p_val2 );
        THROW_ON_RC ( VCursorCommit ( cur ) );
        THROW_ON_RC ( VCursorRelease ( cur ) );
    }

    void OpenTableBind ( const char * p_tableName, const char * p_paramName )
    {
        const VTable * t;
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & t, p_tableName ) );
        THROW_ON_RC ( VViewBindParameterTable ( m_view, p_paramName, t ) );
        THROW_ON_RC ( VTableRelease ( t ) );
    }

    void ValidateBlob ( int64_t p_first, uint64_t p_count, size_t p_bytes )
    {
        THROW_ON_FALSE ( m_blob != 0 );
        int64_t first;
        uint64_t count;
        THROW_ON_RC ( VBlobIdRange ( m_blob, & first, & count ) );
        THROW_ON_FALSE ( p_first == first );
        THROW_ON_FALSE ( p_count == count );
        // blob size depends on release/debug build amd 32/64 bits, so will only check it in DEBUG/64
#if _DEBUGGING && _ARCH_BITS == 64
        size_t bytes;
        THROW_ON_RC ( VBlobSize ( m_blob, & bytes ) );
        THROW_ON_FALSE ( p_bytes == bytes );
#endif
    }
};

///////////////////////////// View-attached VCursor

FIXTURE_TEST_CASE ( ViewCursor_AddRef, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC ( VCursorAddRef ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) ); // use valgrind to find any leaks
    m_cur = 0;
}

// ViewCursor_AddColumn
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NullIdx, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, 0, "%s", ViewColumnName ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NullName, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_EmptyName, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "" ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NameTooLong, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", string ( 2048, 'z' ) . c_str () ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NameNotInView, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "zz" ) );
}

FIXTURE_TEST_CASE ( ViewCursor_AddColumn, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", ViewColumnName ) );
    REQUIRE_EQ ( 1u, m_columnIdx );

    const VColumn * vcol = 0;
    {   // verify insertion into m_cur->row at idx 1
        Vector * row = VCursorGetRow ( ( VCursor * ) m_cur );
        REQUIRE_NOT_NULL ( row );
        REQUIRE_EQ ( 1u, VectorLength ( row ) );
        vcol = ( const VColumn * )VectorGet ( row, 1 );
        REQUIRE_NOT_NULL ( vcol );
        REQUIRE_EQ ( string ( ViewColumnName ), ToCppString ( vcol -> scol -> name -> name ) );
    }

    {   // verify insertion into m_cur->col at idx [ 1, 0 ]
        VCursorCache * cols = VCursorColumns ( ( VCursor * ) m_cur );
        REQUIRE_NOT_NULL ( cols );
        VCtxId id = { 1, 0 };
        REQUIRE_EQ ( (void*)vcol, VCursorCacheGet ( cols, & id ) );
    }
}

//TODO: ViewCursor_AddColumn_IncompleteType (is that possible?)
//TODO: ViewCursor_AddColumn_IncompatibleType (is that possible?)

FIXTURE_TEST_CASE ( ViewCursor_AddColumn_PostOpen, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "cc" ) );
}

FIXTURE_TEST_CASE ( ViewCursor_AddColumn_AlreadyAdded, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", ViewColumnName ) );
}

// VCursorGetColumnIdx

FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NullIdx, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, 0, "%s", ViewColumnName ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NullName, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_EmptyName, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, "" ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NameTooLong, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, "%s", string ( 2048, 'z' ) . c_str () ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NameNotInView, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, "%s", "zz" ) );
}
//TODO: ViewCursor_GetColumnIdx_StateFailed

FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );

    uint32_t idx = 0;
    REQUIRE_RC ( VCursorGetColumnIdx ( m_cur, & idx, "%s", ViewColumnName ) );
    REQUIRE_EQ ( 1u, idx );
}

//TODO: ViewCursor_GetColumnIdx_Ambiguous
//TODO: ViewCursor_GetColumnIdx_Overloaded

// VCursorDataDype
FIXTURE_TEST_CASE ( ViewCursor_DataType_NullParams, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC_FAIL ( VCursorDatatype ( m_cur, m_columnIdx, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_DataType_BadIndex, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    struct VTypedecl type = { 1, 1};
    struct VTypedesc desc = { 1, 1, 1};
    REQUIRE_RC_FAIL ( VCursorDatatype ( m_cur, 0, & type, & desc ) );
    // on error, outputs are 0-filled
    REQUIRE_EQ ( 0u, type . type_id );
    REQUIRE_EQ ( 0u, type . dim );
    REQUIRE_EQ ( 0u, desc . intrinsic_bits );
    REQUIRE_EQ ( 0u, desc . intrinsic_dim );
    REQUIRE_EQ ( 0u, desc . domain );
}
FIXTURE_TEST_CASE ( ViewCursor_DataType, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    struct VTypedecl type = { 0, 0};
    struct VTypedesc desc = { 0, 0, 0};
    REQUIRE_RC ( VCursorDatatype ( m_cur, m_columnIdx, & type, & desc ) );
}

// VCursorOpen
FIXTURE_TEST_CASE ( ViewCursor_Open, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
}

// VCursorIdRange
FIXTURE_TEST_CASE ( ViewCursor_IdRange_NullParams, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorIdRange ( m_cur, m_columnIdx, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_IdRange_NotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t first;
    uint64_t count;
    REQUIRE_RC_FAIL ( VCursorIdRange ( m_cur, m_columnIdx, & first, & count ) );
}
FIXTURE_TEST_CASE ( ViewCursor_IdRange, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t first;
    uint64_t count;
    REQUIRE_RC ( VCursorIdRange ( m_cur, m_columnIdx, & first, & count ) );
    REQUIRE_EQ((int64_t)1, first);
    REQUIRE_EQ((uint64_t)2, count);
}

// VViewCursorRowId
FIXTURE_TEST_CASE ( ViewCursor_RowId_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRowId ( m_cur, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_RowId_NotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ((int64_t)0, id);
}
FIXTURE_TEST_CASE ( ViewCursor_RowId_Open, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ((int64_t)1, id);
}

// VCursorSetRowId
FIXTURE_TEST_CASE ( ViewCursor_SetRowId, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC ( VCursorSetRowId ( m_cur, 100 ) );

    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ((int64_t)100, id);
}

// VCursorFindNextRowId
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowId_ParamNull, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorFindNextRowId ( m_cur, m_columnIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowId, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorFindNextRowId ( m_cur, m_columnIdx, & id ) );
    REQUIRE_EQ((int64_t)2, id);
}

// VCursorFindNextRowIdDirect
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowIdDirect_ParamNull, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 1, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowIdDirect, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 2, & id ) );
    REQUIRE_EQ((int64_t)2, id);
}
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowIdDirect_NoMore, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC_FAIL ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 3, & id ) );
}

// VCursorOpenRow
FIXTURE_TEST_CASE ( ViewCursor_OpenRow_CursorClosed, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorOpenRow ( m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenRow, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenRow_Twice, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) ); // ignored
}

// VCursorCloseRow
FIXTURE_TEST_CASE ( ViewCursor_CloseRow_RowNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) ); // ignored
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ((int64_t)1, id); // row id not incremented
}
FIXTURE_TEST_CASE ( ViewCursor_CloseRow, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ((int64_t)2, id); // row id incremented
}
FIXTURE_TEST_CASE ( ViewCursor_CloseRow_Twice, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) ); // ignored
}

// VCursorGetBlob
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, 0, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, & m_blob, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_RowNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, & m_blob, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_BadColumnIDx, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, & m_blob, 200 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC ( VCursorGetBlob ( m_cur, & m_blob, m_columnIdx ) );
    ValidateBlob ( 1, 2, 3200 );
}

// VCursorGetBlobDirect
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlobDirect ( m_cur, 0, 2, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlobDirect ( m_cur, & m_blob, 2, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect_RowNotOpen, ViewOnTableCursorFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorGetBlobDirect ( m_cur, & m_blob, 2, m_columnIdx ) );
    ValidateBlob ( 1, 2, 3200 );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorGetBlobDirect ( m_cur, & m_blob, 2, m_columnIdx ) );
    ValidateBlob ( 1, 2, 3200 );
}

// VCursorRead
FIXTURE_TEST_CASE ( ViewCursor_Read_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_Misaligned, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 1, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 0, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_CursorRowNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_BadColumn, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, 100, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("blah"), string ( m_buf, m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_BufLen0, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, 0, & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_BufNULL, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, 0, sizeof ( m_buf ), & m_rowLen ) );
}

// VCursorReadDirect
FIXTURE_TEST_CASE ( ViewCursor_ReadDirect_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen )  );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadDirect, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadDirect_CursorRowNotOpen, ViewOnTableCursorFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
}

// VCursorReadBits
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_ElemBits0, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t num_read;
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 0, 0, m_buf, 0, sizeof ( m_buf ), & num_read, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_CursorRowNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining = 100;
    REQUIRE_RC ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
    REQUIRE_EQ ( string ("blah"), string ( m_buf, m_rowLen ) );
    REQUIRE_EQ ( 0u, remaining );
}

//TODO: ViewCursor_ReadBits_SubByte

// VCursorReadBitsDirect
FIXTURE_TEST_CASE ( ViewCursor_ReadBitsDirect_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining;
    REQUIRE_RC_FAIL ( VCursorReadBitsDirect ( m_cur, 2, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBitsDirect, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining = 100;
    REQUIRE_RC ( VCursorReadBitsDirect ( m_cur, 2, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
    REQUIRE_EQ ( 0u, remaining );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBitsDirect_CursorRowNotOpen, ViewOnTableCursorFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining = 100;
    REQUIRE_RC ( VCursorReadBitsDirect ( m_cur, 2, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
    REQUIRE_EQ ( 0u, remaining );
}

// ViewCursor_CellData

FIXTURE_TEST_CASE ( ViewCursor_CelLData_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, m_columnIdx, 0, 0, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CellData_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, m_columnIdx, 0, & m_base, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CellData_CursorRowNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, m_columnIdx, 0, & m_base, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLData_BadColumn, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, 100, 0, & m_base, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLData, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCellData ( m_cur, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
    REQUIRE_EQ ( 8u, m_elemBits );
    REQUIRE_EQ ( 0u, m_boff );
    REQUIRE_EQ ( 4u, m_rowLen );
}

// ViewCursor_CellDataDirect
FIXTURE_TEST_CASE ( ViewCursor_CellDataDirect_CursorNotOpen, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t m_elemBits = 0;
    uint32_t m_boff;
    uint32_t m_rowLen;
    REQUIRE_RC_FAIL ( VCursorCellDataDirect ( m_cur, 2, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLDataDirect, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCellDataDirect ( m_cur, 2, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
    REQUIRE_EQ ( 8u, m_elemBits );
    REQUIRE_EQ ( 0u, m_boff );
    REQUIRE_EQ ( 4u, m_rowLen );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLDataDirect_CursorRowNotOpen, ViewOnTableCursorFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCellDataDirect ( m_cur, 2, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
    REQUIRE_EQ ( 8u, m_elemBits );
    REQUIRE_EQ ( 0u, m_boff );
    REQUIRE_EQ ( 4u, m_rowLen );
}

FIXTURE_TEST_CASE ( ViewCursor_Prefetch, ViewOnTableCursorFixture )
{   // noop
    const int64_t row_ids[] = {1, 2, 3};
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorDataPrefetch ( m_cur, row_ids, m_columnIdx, 3, 1, 3, true ) );
}

// ViewCursor_OpenParentRead
FIXTURE_TEST_CASE ( ViewCursor_OpenParentRead_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorOpenParentRead ( m_cur, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenParentRead, ViewOnTableCursorFixture )
{   // open the first table in the parameter list
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    const VTable * table;
    REQUIRE_RC ( VCursorOpenParentRead ( m_cur, & table ) );
    REQUIRE_EQ ( m_table, table );
    VTableRelease ( table );
}

// ViewCursor_GetUserData
FIXTURE_TEST_CASE ( ViewCursor_GetUserData_NullParam, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetUserData ( m_cur, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetUserData_Default, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    void * data = (void*)1;
    REQUIRE_RC ( VCursorGetUserData ( m_cur, & data ) );
    REQUIRE_EQ ( (void*)0, data );
}

// ViewCursor_SetUserData
FIXTURE_TEST_CASE ( ViewCursor_SetUserData_NoDestructor, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    void * userData = (void*)1;
    REQUIRE_RC ( VCursorSetUserData ( m_cur, userData, 0 ) );
    void * data = 0;
    REQUIRE_RC ( VCursorGetUserData ( m_cur, & data ) );
    REQUIRE_EQ ( userData, data );
}

static void * UserDestroyCalledWith = 0;
static void CC UserDestroy(void* p_param)
{
    UserDestroyCalledWith = p_param;
}

FIXTURE_TEST_CASE ( ViewCursor_SetUserData_Destructor, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    void * userData = (void*)1;
    REQUIRE_RC ( VCursorSetUserData ( m_cur, userData, UserDestroy ) );
    VCursorRelease ( m_cur);
    m_cur = 0;
    REQUIRE_EQ ( userData, UserDestroyCalledWith );
}

FIXTURE_TEST_CASE( ViewCursor_PermitPostOpenAdd, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorPermitPostOpenAdd ( m_cur ) ); // RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported )
}

FIXTURE_TEST_CASE( ViewCursor_GetSchema, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_NOT_NULL ( VCursorGetSchema ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_PageIdRange, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    int64_t first=-1;
    int64_t last=-1;
    REQUIRE_RC ( VCursorPageIdRange ( m_cur, m_columnIdx, 1, & first, & last ) );
    REQUIRE_EQ((int64_t)1, first);
    REQUIRE_EQ((int64_t)2, last);
}

FIXTURE_TEST_CASE( ViewCursor_IsStaticColumn, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    bool is_static = true;
    REQUIRE_RC ( VCursorIsStaticColumn ( m_cur, m_columnIdx, & is_static ) );
    REQUIRE ( ! is_static );
}

FIXTURE_TEST_CASE( ViewCursor_LinkedCursorGet, ViewOnTableCursorFixture )
{   // not implemented for view cursors
    CreateCursor ( GetName(), ViewOnTableName );
    const VCursor * curs = 0;
    REQUIRE_RC_FAIL ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
}
FIXTURE_TEST_CASE( ViewCursor_LinkedCursorSet, ViewOnTableCursorFixture )
{   // not implemented for view cursors
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorLinkedCursorSet ( m_cur, "tbl", m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_GetCacheCapacity, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_EQUAL((uint64_t)0, VCursorGetCacheCapacity(m_cur));
}

FIXTURE_TEST_CASE( ViewCursor_SetCacheCapacity, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );

    // this cursor is not cached, so SetCapacity has no effect
    REQUIRE_EQUAL((uint64_t)0, VCursorSetCacheCapacity((VCursor*)m_cur, 100));
    REQUIRE_EQUAL((uint64_t)0, VCursorGetCacheCapacity(m_cur));
}

FIXTURE_TEST_CASE( ViewCursor_Columns, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    VCursorCache * cols = VCursorColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 2u, VectorLength ( & cols -> cache ) );
}

FIXTURE_TEST_CASE( ViewCursor_PhysicalColumns, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    VCursorCache * cols = VCursorPhysicalColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 1u, VectorLength ( & cols -> cache ) );
}

// VCursorMakeColumn is covered indirectly (any call to ViewOnTableCursorFixture::CreateCursor)

FIXTURE_TEST_CASE( ViewCursor_GetRow, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    Vector * row = VCursorGetRow ( (VCursor*)m_cur );
    REQUIRE_EQ ( 2u, VectorLength ( row ) );
}

FIXTURE_TEST_CASE( ViewCursor_IsReadOnly, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE ( VCursorIsReadOnly ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_GetBlobMruCache, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    // view cursors are not cached
    REQUIRE_NULL ( VCursorGetBlobMruCache ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_IncrementPhysicalProductionCount, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    // no-op for view cursors
    REQUIRE_EQ ( 0u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
    REQUIRE_EQ ( 0u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_FindOverride, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"

    "view V1#1 < T p_tbl > { column ascii c1 = virt; };"
    "view V2#1 < T p_tbl > { column ascii c2 = virt; };"
    "view V#1 < T p_tbl > = V1<p_tbl>, V2<p_tbl> { ascii virt = \"a\"; };"
    ;

    CreateCursor ( GetName(), "V" );
    VCtxId id = {0, 1};
    const KSymbol * sym = VCursorFindOverride ( m_cur, & id );
    REQUIRE_NOT_NULL ( sym );
    REQUIRE_EQ ( string ("virt"), ToCppString ( sym -> name ) );
}
//TODO: param is a view

FIXTURE_TEST_CASE( ViewCursor_LaunchPagemapThread, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) ); // no-op for views
}

FIXTURE_TEST_CASE( ViewCursor_PageMapProcessRequest, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) ); // no-op for views
    const PageMapProcessRequest * req = VCursorPageMapProcessRequest ( m_cur );
    REQUIRE_NULL ( req );
}

FIXTURE_TEST_CASE( ViewCursor_CacheActive, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t end;
    REQUIRE ( ! VCursorCacheActive ( m_cur, & end ) );
    REQUIRE_EQ((int64_t)0, end);
}

// View inheritance

FIXTURE_TEST_CASE( ViewCursor_SingleInheritance, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T1#1 { column ascii c1; };"
    "database DB#1 { table T1 t; };"

    "view V1#1 < T1 p_t > { column ascii c1 = p_t . c1; };"
    "view V#1 < T1 p_tbl > = V1 < p_tbl > {};" // inherits c1, p_tbl is bound to T1 via p_tbl
    ;

    CreateCursorOpen ( GetName(), "V", "c1" );

    // read c1
    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE( ViewCursor_MultipleInheritance, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T1#1 { column ascii c01; };"
    "table T2#1 { column ascii c02; };"
    "database DB#1 { table T1 t1; table T2 t2; };"

    "view V1#1 < T1 p_tbl > { column ascii c1 = p_tbl . c01; };"
    "view V2#1 < T2 p_tbl > { column ascii c2 = p_tbl . c02; };"
    "view V#1 < T1 p_t1, T2 p_t2 > = V1<p_t1>, V2<p_t2>"
    "{"   // inherits c1, c2
    "};"
    ;

    {
        // create DB
        m_databaseName = ScratchDir + GetName();
        MakeDatabase ( m_schemaText, "DB" );
        CreateTablePopulate( "t1", "c01", "t1_c_r1", "t1_c_r2" );
        CreateTablePopulate( "t2", "c02", "t2_c_r1", "t2_c_r2" );
    }

    // create V
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    OpenTableBind ( "t1", "p_t1" );
    OpenTableBind ( "t2", "p_t2" );

    // read c1 and c2 joined on rowId
    uint32_t colIdx1 = AddColumn ( "c1" );
    uint32_t colIdx2 = AddColumn ( "c2" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_EQ ( string ("t1_c_r1"), ReadAscii ( 1, colIdx1 ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( 1, colIdx2 ) );

    REQUIRE_EQ ( string ("t1_c_r2"), ReadAscii ( 2, colIdx1 ) );
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( 2, colIdx2 ) );
}

//TODO: deep inheritance

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"
    "function ascii f(ascii v) { return v; }"
    "view V1#1 < T p_tbl > { column ascii c1 = f(p_tbl.c1); };"
    ;

    CreateCursorOpen ( GetName(), "V1", "c1" );
    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) ); //TODO: modify the value inside f()
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall_Builtin, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"
    "function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo;"
    "view V1#1 < T p_tbl > { column ascii c1 = <ascii>echo<\"qq\">(); };"
    ;

    CreateCursorOpen ( GetName(), "V1", "c1" );
    REQUIRE_EQ ( string ("qq"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("qq"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall_External, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T#1 { column U8[4] c1; column U8 one; };"
    "database DB#1 { table T t; };"
    "extern function < type T > T NCBI:SRA:swap #1 ( T in, U8 called );"
    "view V1#1 < T p_tbl > {"
    "   column U8[4] c1 = < U8 [ 4 ] > NCBI:SRA:swap ( p_tbl . c1, p_tbl . one );"
    "};"
    ;
    m_databaseName = ScratchDir + GetName();

    {
        MakeDatabase ( m_schemaText, "DB" );
        m_cur = CreateTable ( TableName );
        uint32_t colIdx1 = AddColumn ( "c1" );
        uint32_t colIdx2 = AddColumn ( "one" );
        VCursor * cursor = (VCursor*) m_cur;
        REQUIRE_RC ( VCursorOpen ( cursor ) );

       // insert some rows
        const uint8_t c1_1[4] = {1, 2, 3, 4};
        const uint8_t c1_2[4] = {11, 22, 33, 44};
        const uint8_t One = 1;

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx1, 8, &c1_1, 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx2, 8, &One, 0, 1 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx1, 8, &c1_2, 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, colIdx2, 8, &One, 0, 1 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
        m_cur = 0;
    }

    {   // read and verify: swap(U8[2], 1) swaps the first 2 elements
        REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V1" ) );
        OpenTableBind ( TableName, TableParamName );
        REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
        m_columnIdx = AddColumn ( "c1" );
        REQUIRE_RC ( VCursorOpen ( m_cur ) );

        uint8_t val[4] = {0, 0, 0, 0};
        REQUIRE_RC ( VCursorReadDirect ( m_cur, 1, m_columnIdx, 8, &val, 4, & m_rowLen ) );
        REQUIRE_EQ ( 2u, (uint32_t)val[0] ); // array elements 0 and 1 swapped
        REQUIRE_EQ ( 1u, (uint32_t)val[1] );
        REQUIRE_EQ ( 3u, (uint32_t)val[2] );
        REQUIRE_EQ ( 4u, (uint32_t)val[3] );

        REQUIRE_RC ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, &val, 4, & m_rowLen ) );
        REQUIRE_EQ ( 22u, (uint32_t)val[0] ); // array elements 0 and swapped
        REQUIRE_EQ ( 11u, (uint32_t)val[1] );
        REQUIRE_EQ ( 33u, (uint32_t)val[2] );
        REQUIRE_EQ ( 44u, (uint32_t)val[3] );
    }
}

FIXTURE_TEST_CASE( ViewCursor_Join_Shorthand, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T1#1 { column I64 c01; };"
    "table T2#1 { column ascii c02; };"
    "database DB#1 { table T1 t1; table T2 t2; };"
    "view V#1 < T1 p_t1, T2 p_t2 > { column ascii c1 = p_t2 [ p_t1 . c01 ] . c02; };"
    ;

    {
        // create DB
        m_databaseName = ScratchDir + GetName();
        MakeDatabase ( m_schemaText, "DB" );
        CreateTablePopulate( "t1", "c01", 2, 1 );
        CreateTablePopulate( "t2", "c02", "t2_c_r1", "t2_c_r2" );
    }
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    OpenTableBind ( "t1", "p_t1" );
    OpenTableBind ( "t2", "p_t2" );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    m_columnIdx = AddColumn ( "c1" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    // T2 rows seen through V in reverse order
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( 2, m_columnIdx ) );
}

FIXTURE_TEST_CASE( ViewCursor_Join_Shorthand_Production, ViewOnTableCursorFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T1#1 { column I64 c01; };"
    "table T2#1 { column ascii c02; ascii p = c02; };"
    "database DB#1 { table T1 t1; table T2 t2; };"
    "view V#1 < T1 p_t1, T2 p_t2 > { column ascii c1 = p_t2 [ p_t1 . c01 ] . p; };"
    ;

    {
        // create DB
        m_databaseName = ScratchDir + GetName();
        MakeDatabase ( m_schemaText, "DB" );
        CreateTablePopulate( "t1", "c01", 2, 1 );
        CreateTablePopulate( "t2", "c02", "t2_c_r1", "t2_c_r2" );
    }
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    OpenTableBind ( "t1", "p_t1" );
    OpenTableBind ( "t2", "p_t2" );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    m_columnIdx = AddColumn ( "c1" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    // T2 rows seen through V in reverse order
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( 2, m_columnIdx ) );
}

//TODO: join with a non-int key (error)
//TODO: join with a non-I64 integer key (cast required?)

//TODO: external function call in a view parameter's productions

// View on a View
class ViewOnViewCursorFixture : public ViewCursorFixture
{
public:
    void CreateCursor ( const string & p_testName )
    {
        CreateDb ( p_testName );
        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, ViewOnViewName ) );
        // instantiate a view with another view as a parameter
        const VView * paramView; // the parameter view, bound to a table
        THROW_ON_RC ( VDBManagerOpenView ( m_mgr, & paramView, m_schema, ViewOnTableName ) );
        THROW_ON_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName ) );
        THROW_ON_RC ( VViewBindParameterTable ( paramView, TableParamName, m_table ) );

        THROW_ON_RC ( VViewBindParameterView ( m_view, ViewParamName, paramView ) );
        THROW_ON_RC ( VViewRelease ( (VView*)paramView ) );

        THROW_ON_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    }
};

FIXTURE_TEST_CASE ( ViewCursor_GetTable_OnView, ViewOnViewCursorFixture )
{
    CreateCursor ( GetName () );
    // VCursorGetTable on the view's cursor should return the parameter view's underlying table
    const VTable * tbl = VCursorGetTable ( m_cur );
    REQUIRE_NOT_NULL ( tbl );
    REQUIRE_EQ ( string ( "T" ), ToCppString ( tbl -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE ( ViewCursor_OnView_Read, ViewOnViewCursorFixture )
{
    CreateCursor ( GetName () );
    m_columnIdx = AddColumn ( ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

//TODO: VViewCreateCursor with multiple table/view parameters

//////////////////////////////////////////////////////////////////
// VCursor write-side methods, not implemented for view cursors

FIXTURE_TEST_CASE ( ViewCursor_Write, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorWrite ( (VCursor*)m_cur, m_columnIdx, 8, m_buf, 0, 1 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CommitRow, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCommitRow ( (VCursor*)m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_RepeatRow, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRepeatRow ( (VCursor*)m_cur, 2 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_FlushPage, ViewOnTableCursorFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorFlushPage ( (VCursor*)m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Default, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorDefault ( (VCursor*)m_cur, m_columnIdx, 8, m_buf, 0, 1 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Commit, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCommit ( (VCursor*)m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenParentUpdate, ViewOnTableCursorFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorOpenParentUpdate ( (VCursor*)m_cur, (VTable**) & m_table ) );
}
FIXTURE_TEST_CASE( ViewCursor_SuspendTriggers, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorSuspendTriggers ( m_cur ) );
}
FIXTURE_TEST_CASE( ViewCursor_InstallTrigger, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    VProduction prod;
    REQUIRE_RC_FAIL ( VCursorInstallTrigger ( (VCursor*)m_cur, & prod ) );
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

const char UsageDefaultName[] = "test-view-cursor";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=ViewCursorTestSuite(argc, argv);
    return rc;
}

}
