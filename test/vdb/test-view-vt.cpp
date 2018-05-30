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

// this file is included into test-view-cursor.cpp

///////////////////////////// View-attached VCursor, tests covering virtual table functions

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

    {   // verify insertion into m_cur->view_col at idx [ 0, 0 ]
        VCursorCache * cols = VCursorColumns ( ( VCursor * ) m_cur, eView );
        REQUIRE_NOT_NULL ( cols );
        VCtxId id = { 0, 0, eView };
        REQUIRE_EQ ( (void*)vcol, VCursorCacheGet ( cols, & id ) );
    }
    {   // verify non-insertion into m_cur->col at idx [ 1, 0 ] (for table columns only)
        VCursorCache * cols = VCursorColumns ( ( VCursor * ) m_cur, eTable );
        REQUIRE_NOT_NULL ( cols );
        VCtxId id = { 0, 0, eView };
        REQUIRE_NE ( (void*)vcol, VCursorCacheGet ( cols, & id ) );
    }
}

FIXTURE_TEST_CASE ( ViewCursor_AddProduction, ViewOnTableCursorFixture )
{
    CreateCursorOpen ( GetName(), ViewWithProductionName, ViewColumnName );
    // adding column "c" involves adding production "prod" which it depends on
    // verify insertion of production into m_cur->view_prod at [ 2, 0 ]
    const VCursorCache * prods = VCursorProductions ( ( VCursor * ) m_cur, eView );
    VCtxId id = { 2, 0, eView };
    REQUIRE_NOT_NULL ( VCursorCacheGet ( prods, & id ) );
}

//TODO: add a production, verify access through VCursorProductions
//TODO: same for table-cursor (make sure VTableCursorColumns is covered, as well)

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
    //TODO: verify
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
{
    CreateCursor ( GetName(), ViewOnTableName );
    const VCursor * curs = 0;
    REQUIRE_RC_FAIL ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
}
FIXTURE_TEST_CASE( ViewCursor_LinkedCursorSet, ViewOnTableCursorFixture )
{
    CreateCursor ( GetName(), ViewOnTableName );
    // reuse m_cur
    REQUIRE_RC ( VCursorLinkedCursorSet ( m_cur, "tbl", m_cur ) ); // calls AddRef(m_cur)
    REQUIRE_RC ( VCursorRelease ( m_cur ) );

    const VCursor * curs = 0;
    REQUIRE_RC ( VCursorLinkedCursorGet ( m_cur, "tbl", & curs ) );
    REQUIRE_EQ ( (const VCursor*)m_cur, curs );
    REQUIRE_RC ( VCursorRelease ( curs ) );
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
    CreateCursor ( GetName(), ViewOnTableName );
    AddColumn ( ViewColumnName );
    AddColumn ( "cc" );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    VCursorCache * cols = VCursorColumns ( (VCursor*)m_cur, eView );
    REQUIRE_EQ ( 1u, VectorLength ( & cols -> cache ) ); // there is 1 view
    REQUIRE_EQ ( 2u, VectorLength ( ( Vector * ) VectorGet ( & cols -> cache, 0 ) ) ); // ... with 2 columns
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
    VCtxId id = {0, 1, eView };
    const KSymbol * sym = VCursorFindOverride ( m_cur, & id, NULL );
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
