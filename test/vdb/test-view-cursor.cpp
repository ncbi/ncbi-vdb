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

#include <sysalloc.h>

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

#include <ktst/unit_test.hpp> // TEST_CASE

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

class ViewFixture : public WVDB_v2_Fixture
{
public:
    ViewFixture()
    :   m_schemaText ( SchemaText ),
        m_view ( 0 ),
        m_table ( 0 ),
        m_cur ( 0 ),
        m_blob ( 0 ),
        m_base ( 0 )
    {
    }
    ~ViewFixture()
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

class ViewOnTableFixture : public ViewFixture
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
        THROW_ON_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", p_colName ) );
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

    void ValidateBlob ( int64_t p_first, uint64_t p_count, size_t p_bytes )
    {
        THROW_ON_FALSE ( m_blob != 0 );
        int64_t first;
        uint64_t count;
        THROW_ON_RC ( VBlobIdRange ( m_blob, & first, & count ) );
        THROW_ON_FALSE ( p_first == first );
        THROW_ON_FALSE ( p_count == count );
        size_t bytes;
        THROW_ON_RC ( VBlobSize ( m_blob, & bytes ) );
        THROW_ON_FALSE ( p_bytes == bytes );
    }

    string ReadAscii ( uint32_t p_colIdx )
    {
        THROW_ON_RC ( VCursorRead ( m_cur, p_colIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
        return string ( m_buf, m_rowLen );
    }
};

///////////////////////////// View-attached VCursor

FIXTURE_TEST_CASE ( ViewCursor_AddRef, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC ( VCursorAddRef ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) );
    REQUIRE_RC ( VCursorRelease ( m_cur ) ); // use valgrind to find any leaks
    m_cur = 0;
}

// ViewCursor_AddColumn
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NullIdx, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, 0, "%s", ViewColumnName ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NullName, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_EmptyName, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "" ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NameTooLong, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", string ( 2048, 'z' ) . c_str () ) );
}
FIXTURE_TEST_CASE ( ViewCursor_AddColumn_NameNotInView, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "zz" ) );
}

FIXTURE_TEST_CASE ( ViewCursor_AddColumn, ViewOnTableFixture )
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

FIXTURE_TEST_CASE ( ViewCursor_AddColumn_PostOpen, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "cc" ) );
}

FIXTURE_TEST_CASE ( ViewCursor_AddColumn_AlreadyAdded, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", ViewColumnName ) );
}

// VCursorGetColumnIdx

FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NullIdx, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, 0, "%s", ViewColumnName ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NullName, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_EmptyName, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, "" ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NameTooLong, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, "%s", string ( 2048, 'z' ) . c_str () ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx_NameNotInView, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetColumnIdx ( m_cur, & m_columnIdx, "%s", "zz" ) );
}
//TODO: ViewCursor_GetColumnIdx_StateFailed

FIXTURE_TEST_CASE ( ViewCursor_GetColumnIdx, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );

    uint32_t idx = 0;
    REQUIRE_RC ( VCursorGetColumnIdx ( m_cur, & idx, "%s", ViewColumnName ) );
    REQUIRE_EQ ( 1u, idx );
}

//TODO: ViewCursor_GetColumnIdx_Ambiguous
//TODO: ViewCursor_GetColumnIdx_Overloaded

// VCursorDataDype
FIXTURE_TEST_CASE ( ViewCursor_DataType_NullParams, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC_FAIL ( VCursorDatatype ( m_cur, m_columnIdx, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_DataType_BadIndex, ViewOnTableFixture )
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
FIXTURE_TEST_CASE ( ViewCursor_DataType, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    struct VTypedecl type = { 0, 0};
    struct VTypedesc desc = { 0, 0, 0};
    REQUIRE_RC ( VCursorDatatype ( m_cur, m_columnIdx, & type, & desc ) );
}

// VCursorOpen
FIXTURE_TEST_CASE ( ViewCursor_Open, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );
}

// VCursorIdRange
FIXTURE_TEST_CASE ( ViewCursor_IdRange_NullParams, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorIdRange ( m_cur, m_columnIdx, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_IdRange_NotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t first;
    uint64_t count;
    REQUIRE_RC_FAIL ( VCursorIdRange ( m_cur, m_columnIdx, & first, & count ) );
}
FIXTURE_TEST_CASE ( ViewCursor_IdRange, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t first;
    uint64_t count;
    REQUIRE_RC ( VCursorIdRange ( m_cur, m_columnIdx, & first, & count ) );
    REQUIRE_EQ ( 1l, first );
    REQUIRE_EQ ( 2lu, count );
}

// VViewCursorRowId
FIXTURE_TEST_CASE ( ViewCursor_RowId_NullParam, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRowId ( m_cur, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_RowId_NotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ ( 0l, id );
}
FIXTURE_TEST_CASE ( ViewCursor_RowId_Open, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ ( 1l, id );
}

// VCursorSetRowId
FIXTURE_TEST_CASE ( ViewCursor_SetRowId, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC ( VCursorSetRowId ( m_cur, 100 ) );

    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ ( 100l, id );
}

// VCursorFindNextRowId
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowId_ParamNull, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorFindNextRowId ( m_cur, m_columnIdx, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowId, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorFindNextRowId ( m_cur, m_columnIdx, & id ) );
    REQUIRE_EQ ( 2l, id );
}

// VCursorFindNextRowIdDirect
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowIdDirect_ParamNull, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 1, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowIdDirect, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 2, & id ) );
    REQUIRE_EQ ( 2l, id );
}
FIXTURE_TEST_CASE ( ViewCursor_FindNextRowIdDirect_NoMore, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t id = -1;
    REQUIRE_RC_FAIL ( VCursorFindNextRowIdDirect ( m_cur, m_columnIdx, 3, & id ) );
}

// VCursorOpenRow
FIXTURE_TEST_CASE ( ViewCursor_OpenRow_CursorClosed, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorOpenRow ( m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenRow, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenRow_Twice, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) ); // ignored
}

// VCursorCloseRow
FIXTURE_TEST_CASE ( ViewCursor_CloseRow_RowNotOpen, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) ); // ignored
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ ( 1l, id ); // row id not incremented
}
FIXTURE_TEST_CASE ( ViewCursor_CloseRow, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    int64_t id = -1;
    REQUIRE_RC ( VCursorRowId ( m_cur, & id ) );
    REQUIRE_EQ ( 2l, id ); // row id incremented
}
FIXTURE_TEST_CASE ( ViewCursor_CloseRow_Twice, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) ); // ignored
}

// VCursorGetBlob
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_NullParam, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, 0, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, & m_blob, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_RowNotOpen, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, & m_blob, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob_BadColumnIDx, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlob ( m_cur, & m_blob, 200 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlob, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC ( VCursorGetBlob ( m_cur, & m_blob, m_columnIdx ) );
    ValidateBlob ( 1, 2, 3200 );
}

// VCursorGetBlobDirect
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect_NullParam, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlobDirect ( m_cur, 0, 2, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetBlobDirect ( m_cur, & m_blob, 2, m_columnIdx ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect_RowNotOpen, ViewOnTableFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorGetBlobDirect ( m_cur, & m_blob, 2, m_columnIdx ) );
    ValidateBlob ( 1, 2, 3200 );
}
FIXTURE_TEST_CASE ( ViewCursor_GetBlobDirect, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorGetBlobDirect ( m_cur, & m_blob, 2, m_columnIdx ) );
    ValidateBlob ( 1, 2, 3200 );
}

// VCursorRead
FIXTURE_TEST_CASE ( ViewCursor_Read_NullParam, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_Misaligned, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 1, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 0, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_CursorRowNotOpen, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_BadColumn, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, 100, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("blah"), string ( m_buf, m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_BufLen0, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, 0, & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Read_BufNULL, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRead ( m_cur, m_columnIdx, 8, 0, sizeof ( m_buf ), & m_rowLen ) );
}

// VCursorReadDirect
FIXTURE_TEST_CASE ( ViewCursor_ReadDirect_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen )  );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadDirect, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadDirect_CursorRowNotOpen, ViewOnTableFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorReadDirect ( m_cur, 2, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
}

// VCursorReadBits
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_NullParam, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_ElemBits0, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t num_read;
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 0, 0, m_buf, 0, sizeof ( m_buf ), & num_read, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits_CursorRowNotOpen, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBits, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining = 100;
    REQUIRE_RC ( VCursorReadBits ( m_cur, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
    REQUIRE_EQ ( string ("blah"), string ( m_buf, m_rowLen ) );
    REQUIRE_EQ ( 0u, remaining );
}

//TODO: ViewCursor_ReadBits_SubByte

// VCursorReadBitsDirect
FIXTURE_TEST_CASE ( ViewCursor_ReadBitsDirect_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining;
    REQUIRE_RC_FAIL ( VCursorReadBitsDirect ( m_cur, 2, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBitsDirect, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining = 100;
    REQUIRE_RC ( VCursorReadBitsDirect ( m_cur, 2, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
    REQUIRE_EQ ( 0u, remaining );
}
FIXTURE_TEST_CASE ( ViewCursor_ReadBitsDirect_CursorRowNotOpen, ViewOnTableFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t remaining = 100;
    REQUIRE_RC ( VCursorReadBitsDirect ( m_cur, 2, m_columnIdx, 8, 0, m_buf, 0, sizeof ( m_buf ), & m_rowLen, & remaining ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
    REQUIRE_EQ ( 0u, remaining );
}

// ViewCursor_CellData

FIXTURE_TEST_CASE ( ViewCursor_CelLData_NullParam, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, m_columnIdx, 0, 0, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CellData_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, m_columnIdx, 0, & m_base, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CellData_CursorRowNotOpen, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, m_columnIdx, 0, & m_base, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLData_BadColumn, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCellData ( m_cur, 100, 0, & m_base, 0, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLData, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCellData ( m_cur, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
    REQUIRE_EQ ( 8u, m_elemBits );
    REQUIRE_EQ ( 0u, m_boff );
    REQUIRE_EQ ( 4u, m_rowLen );
}

// ViewCursor_CellDataDirect
FIXTURE_TEST_CASE ( ViewCursor_CellDataDirect_CursorNotOpen, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    uint32_t m_elemBits = 0;
    uint32_t m_boff;
    uint32_t m_rowLen;
    REQUIRE_RC_FAIL ( VCursorCellDataDirect ( m_cur, 2, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLDataDirect, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCellDataDirect ( m_cur, 2, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
    REQUIRE_EQ ( 8u, m_elemBits );
    REQUIRE_EQ ( 0u, m_boff );
    REQUIRE_EQ ( 4u, m_rowLen );
}
FIXTURE_TEST_CASE ( ViewCursor_CelLDataDirect_CursorRowNotOpen, ViewOnTableFixture )
{   // direct read does not need an open row
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorCellDataDirect ( m_cur, 2, m_columnIdx, & m_elemBits, & m_base, & m_boff, & m_rowLen ) );
    REQUIRE_EQ ( 8u, m_elemBits );
    REQUIRE_EQ ( 0u, m_boff );
    REQUIRE_EQ ( 4u, m_rowLen );
}

FIXTURE_TEST_CASE ( ViewCursor_Prefetch, ViewOnTableFixture )
{   // noop
    const int64_t row_ids[] = {1, 2, 3};
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorDataPrefetch ( m_cur, row_ids, m_columnIdx, 3, 1, 3, true ) );
}

// ViewCursor_OpenParentRead
FIXTURE_TEST_CASE ( ViewCursor_OpenParentRead_NullParam, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorOpenParentRead ( m_cur, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenParentRead, ViewOnTableFixture )
{   // open the first table in the parameter list
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    const VTable * table;
    REQUIRE_RC ( VCursorOpenParentRead ( m_cur, & table ) );
    REQUIRE_EQ ( m_table, table );
    VTableRelease ( table );
}

// ViewCursor_GetUserData
FIXTURE_TEST_CASE ( ViewCursor_GetUserData_NullParam, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorGetUserData ( m_cur, 0 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_GetUserData_Default, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    void * data = (void*)1;
    REQUIRE_RC ( VCursorGetUserData ( m_cur, & data ) );
    REQUIRE_EQ ( (void*)0, data );
}

// ViewCursor_SetUserData
FIXTURE_TEST_CASE ( ViewCursor_SetUserData_NoDestructor, ViewOnTableFixture )
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

FIXTURE_TEST_CASE ( ViewCursor_SetUserData_Destructor, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    void * userData = (void*)1;
    REQUIRE_RC ( VCursorSetUserData ( m_cur, userData, UserDestroy ) );
    VCursorRelease ( m_cur);
    m_cur = 0;
    REQUIRE_EQ ( userData, UserDestroyCalledWith );
}

FIXTURE_TEST_CASE( ViewCursor_PermitPostOpenAdd, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorPermitPostOpenAdd ( m_cur ) ); // RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported )
}

FIXTURE_TEST_CASE( ViewCursor_GetSchema, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_NOT_NULL ( VCursorGetSchema ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_PageIdRange, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    int64_t first=-1;
    int64_t last=-1;
    REQUIRE_RC ( VCursorPageIdRange ( m_cur, m_columnIdx, 1, & first, & last ) );
    REQUIRE_EQ ( 1l, first );
    REQUIRE_EQ ( 2l, last );
}

FIXTURE_TEST_CASE( ViewCursor_IsStaticColumn, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    bool is_static = true;
    REQUIRE_RC ( VCursorIsStaticColumn ( m_cur, m_columnIdx, & is_static ) );
    REQUIRE ( ! is_static );
}

FIXTURE_TEST_CASE( ViewCursor_LinkedCursorGet, ViewOnTableFixture )
{   // not implemented for view cursors
    CreateCursor ( GetName(), ViewOnTableName );
    const VCursor * curs = 0;
    REQUIRE_RC_FAIL ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
}
FIXTURE_TEST_CASE( ViewCursor_LinkedCursorSet, ViewOnTableFixture )
{   // not implemented for view cursors
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorLinkedCursorSet ( m_cur, "tbl", m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_GetCacheCapacity, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_EQUAL ( 0ul, VCursorGetCacheCapacity ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_SetCacheCapacity, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );

    // this cursor is not cached, so SetCapacity has no effect
    REQUIRE_EQUAL ( 0ul, VCursorSetCacheCapacity ( (VCursor*)m_cur, 100 ) );
    REQUIRE_EQUAL ( 0ul, VCursorGetCacheCapacity ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_Columns, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    VCursorCache * cols = VCursorColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 2u, VectorLength ( & cols -> cache ) );
}

FIXTURE_TEST_CASE( ViewCursor_PhysicalColumns, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    VCursorCache * cols = VCursorPhysicalColumns ( (VCursor*)m_cur );
    REQUIRE_EQ ( 1u, VectorLength ( & cols -> cache ) );
}

// VCursorMakeColumn is covered indirectly (any call to ViewOnTableFixture::CreateCursor)

FIXTURE_TEST_CASE( ViewCursor_GetRow, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    Vector * row = VCursorGetRow ( (VCursor*)m_cur );
    REQUIRE_EQ ( 2u, VectorLength ( row ) );
}

FIXTURE_TEST_CASE( ViewCursor_IsReadOnly, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE ( VCursorIsReadOnly ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_GetBlobMruCache, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    // view cursors are not cached
    REQUIRE_NULL ( VCursorGetBlobMruCache ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_IncrementPhysicalProductionCount, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    // no-op for view cursors
    REQUIRE_EQ ( 0u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
    REQUIRE_EQ ( 0u, VCursorIncrementPhysicalProductionCount ( (VCursor*)m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_FindOverride, ViewOnTableFixture )
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

FIXTURE_TEST_CASE( ViewCursor_LaunchPagemapThread, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) ); // no-op for views
}

FIXTURE_TEST_CASE( ViewCursor_PageMapProcessRequest, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );

    REQUIRE_RC ( VCursorLaunchPagemapThread ( (VCursor*)m_cur ) ); // no-op for views
    const PageMapProcessRequest * req = VCursorPageMapProcessRequest ( m_cur );
    REQUIRE_NULL ( req );
}

FIXTURE_TEST_CASE( ViewCursor_CacheActive, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    int64_t end;
    REQUIRE ( ! VCursorCacheActive ( m_cur, & end ) );
    REQUIRE_EQ ( 0l, end );
}

// View inheritance

FIXTURE_TEST_CASE( ViewCursor_SingleInheritance, ViewOnTableFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T1#1 { column ascii c01; };"
    "database DB#1 { table T1 t1; };"

    "view V1#1 < T1 p_tbl > { column ascii c1 = p_tbl . c01; };"
    "view V#1 < T1 p_t1 > = V1 < p_t1 > {};" // inherits c1, p_tbl is bound to T1 via p_t1
    ;

    {
        // create DB
        m_databaseName = ScratchDir + GetName();
        MakeDatabase ( m_schemaText, "DB" );

        // create T1, populate
        VCursor * cur = CreateTableOpenWriteCursor( "t1", "c01" );
        WriteRow ( cur, m_columnIdx, "t1_c_r1" );
        WriteRow ( cur, m_columnIdx, "t1_c_r2" );
        REQUIRE_RC ( VCursorCommit ( cur ) );
        REQUIRE_RC ( VCursorRelease ( cur ) );
    }

    // create V
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );

    // open, bind T1
    const VTable * t1;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & t1, "t1" ) );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, "p_t1", t1 ) );
    REQUIRE_RC ( VTableRelease ( t1 ) );

    // read c1
    uint32_t colIdx1;
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & colIdx1, "%s", "c1" ) );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_EQ ( string ("t1_c_r1"), ReadAscii ( colIdx1 ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
}

FIXTURE_TEST_CASE( ViewCursor_MultipleInheritance, ViewOnTableFixture )
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

    m_keepDb = true;

    {
        // create DB
        m_databaseName = ScratchDir + GetName();
        MakeDatabase ( m_schemaText, "DB" );

        // create T1, populate
        VCursor * cur = CreateTableOpenWriteCursor( "t1", "c01" );
        WriteRow ( cur, m_columnIdx, "t1_c_r1" );
        WriteRow ( cur, m_columnIdx, "t1_c_r2" );
        REQUIRE_RC ( VCursorCommit ( cur ) );
        REQUIRE_RC ( VCursorRelease ( cur ) );

        // create T2, populate
        cur = CreateTableOpenWriteCursor( "t2", "c02" );
        WriteRow ( cur, m_columnIdx, "t2_c_r1" );
        WriteRow ( cur, m_columnIdx, "t2_c_r2" );
        REQUIRE_RC ( VCursorCommit ( cur ) );
        REQUIRE_RC ( VCursorRelease ( cur ) );
    }

    // create V
    REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V" ) );
    REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );

    // open, bind T1, T2
    const VTable * t1;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & t1, "t1" ) );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, "p_t1", t1 ) );
    REQUIRE_RC ( VTableRelease ( t1 ) );
    const VTable * t2;
    REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & t2, "t2" ) );
    REQUIRE_RC ( VViewBindParameterTable ( m_view, "p_t2", t2 ) );
    REQUIRE_RC ( VTableRelease ( t2 ) );

    // read c1 and c2 joined on rowId
    uint32_t colIdx1;
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & colIdx1, "%s", "c1" ) );
    uint32_t colIdx2;
    REQUIRE_RC ( VCursorAddColumn ( m_cur, & colIdx2, "%s", "c2" ) );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_EQ ( string ("t1_c_r1"), ReadAscii ( colIdx1 ) );
    REQUIRE_EQ ( string ("t2_c_r1"), ReadAscii ( colIdx2 ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );

    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_EQ ( string ("t1_c_r2"), ReadAscii ( colIdx1 ) );
    REQUIRE_EQ ( string ("t2_c_r2"), ReadAscii ( colIdx2 ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
}

//TODO: deep inheritance

//////////////////////////////////////////////////////////////////
// VCursor write-side methods, not implemented for view cursors

FIXTURE_TEST_CASE ( ViewCursor_Write, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorWrite ( (VCursor*)m_cur, m_columnIdx, 8, m_buf, 0, 1 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_CommitRow, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCommitRow ( (VCursor*)m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_RepeatRow, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorRepeatRow ( (VCursor*)m_cur, 2 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_FlushPage, ViewOnTableFixture )
{
    CreateCursorOpenRow ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorFlushPage ( (VCursor*)m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Default, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorDefault ( (VCursor*)m_cur, m_columnIdx, 8, m_buf, 0, 1 ) );
}
FIXTURE_TEST_CASE ( ViewCursor_Commit, ViewOnTableFixture )
{
    CreateCursorOpen ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorCommit ( (VCursor*)m_cur ) );
}
FIXTURE_TEST_CASE ( ViewCursor_OpenParentUpdate, ViewOnTableFixture )
{
    CreateCursorAddColumn ( GetName(), ViewOnTableName, ViewColumnName );
    REQUIRE_RC_FAIL ( VCursorOpenParentUpdate ( (VCursor*)m_cur, (VTable**) & m_table ) );
}
FIXTURE_TEST_CASE( ViewCursor_SuspendTriggers, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    REQUIRE_RC_FAIL ( VCursorSuspendTriggers ( m_cur ) );
}
FIXTURE_TEST_CASE( ViewCursor_InstallTrigger, ViewOnTableFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    VProduction prod;
    REQUIRE_RC_FAIL ( VCursorInstallTrigger ( (VCursor*)m_cur, & prod ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall, ViewOnTableFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"
    "function ascii f(ascii v) { return v; }"
    "view V1#1 < T p_tbl > { column ascii c1 = f(p_tbl.c1); };"
    ;

    CreateCursorOpenRow ( GetName(), "V1", "c1" );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("blah"), string ( m_buf, m_rowLen ) ); //TODO: modify the value inside f()
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall_Builtin, ViewOnTableFixture )
{
    m_schemaText =
    "version 2.0;"
    "table T#1 { column ascii c1; };"
    "database DB#1 { table T t; };"
    "function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo;"
    "view V1#1 < T p_tbl > { column ascii c1 = <ascii>echo<\"qq\">(); };"
    ;

    CreateCursorOpenRow ( GetName(), "V1", "c1" );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("qq"), string ( m_buf, m_rowLen ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );
    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("qq"), string ( m_buf, m_rowLen ) );
}

FIXTURE_TEST_CASE ( ViewCursor_FunctionCall_External, ViewOnTableFixture )
{
    m_keepDb = true;
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
        VCursor * cursor = CreateTable ( TableName );
        REQUIRE_RC ( VCursorAddColumn ( cursor, & m_columnIdx, "c1" ) );
        uint32_t oneIdx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & oneIdx, "one" ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

       // insert some rows
        const uint8_t c1_1[4] = {1, 2, 3, 4};
        const uint8_t c1_2[4] = {11, 22, 33, 44};
        const uint8_t One = 1;

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, m_columnIdx, 8, &c1_1, 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, oneIdx, 8, &One, 0, 1 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorOpenRow ( cursor ) );
        REQUIRE_RC ( VCursorWrite ( cursor, m_columnIdx, 8, &c1_2, 0, 4 ) );
        REQUIRE_RC ( VCursorWrite ( cursor, oneIdx, 8, &One, 0, 1 ) );
        REQUIRE_RC ( VCursorCommitRow ( cursor ) );
        REQUIRE_RC ( VCursorCloseRow ( cursor ) );

        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }

    {   // read and verify: swap(U8[2], 1) swaps the first 2 elements
        REQUIRE_RC ( VDBManagerOpenView ( m_mgr, & m_view, m_schema, "V1" ) );
        REQUIRE_RC ( VDatabaseOpenTableRead ( m_db, & m_table, TableName ) );
        REQUIRE_RC ( VViewBindParameterTable ( m_view, TableParamName, m_table ) );
        REQUIRE_RC ( VViewCreateCursor ( m_view, & m_cur ) );
        REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", "c1" ) );
        REQUIRE_RC ( VCursorOpen ( m_cur ) );

        uint8_t val[4] = {0, 0, 0, 0};
        REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
        REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, &val, 4, & m_rowLen ) );
        REQUIRE_EQ ( 2u, (uint32_t)val[0] ); // array elements 0 and 1 swapped
        REQUIRE_EQ ( 1u, (uint32_t)val[1] );
        REQUIRE_EQ ( 3u, (uint32_t)val[2] );
        REQUIRE_EQ ( 4u, (uint32_t)val[3] );
        REQUIRE_RC ( VCursorCloseRow ( m_cur ) );

        REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
        REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, &val, 4, & m_rowLen ) );
        REQUIRE_EQ ( 22u, (uint32_t)val[0] ); // array elements 0 and swapped
        REQUIRE_EQ ( 11u, (uint32_t)val[1] );
        REQUIRE_EQ ( 33u, (uint32_t)val[2] );
        REQUIRE_EQ ( 44u, (uint32_t)val[3] );
    }
}

//TODO: external function call in a view parameter's productions

// View on a View
class ViewOnViewFixture : public ViewFixture
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

FIXTURE_TEST_CASE ( ViewCursor_GetTable_OnView, ViewOnViewFixture )
{
    CreateCursor ( GetName () );
    // VCursorGetTable on the view's cursor should return the parameter view's underlying table
    const VTable * tbl = VCursorGetTable ( m_cur );
    REQUIRE_NOT_NULL ( tbl );
    REQUIRE_EQ ( string ( "T" ), ToCppString ( tbl -> stbl -> name -> name ) );
}

FIXTURE_TEST_CASE ( ViewCursor_OnView_Read, ViewOnViewFixture )
{
    CreateCursor ( GetName () );

    REQUIRE_RC ( VCursorAddColumn ( m_cur, & m_columnIdx, "%s", ViewColumnName ) );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("blah"), string ( m_buf, m_rowLen ) );
    REQUIRE_RC ( VCursorCloseRow ( m_cur ) );

    REQUIRE_RC ( VCursorOpenRow ( m_cur ) );
    REQUIRE_RC ( VCursorRead ( m_cur, m_columnIdx, 8, m_buf, sizeof ( m_buf ), & m_rowLen ) );
    REQUIRE_EQ ( string ("eeee"), string ( m_buf, m_rowLen ) );
}

//TODO: VViewCreateCursor with multiple table/view parameters

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
