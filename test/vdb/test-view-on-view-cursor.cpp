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
        String t;
        StringInitCString ( & t, TableParamName );
        THROW_ON_RC ( VViewBindParameterTable ( paramView, & t, m_table ) );

        StringInitCString ( & t, ViewParamName );
        THROW_ON_RC ( VViewBindParameterView ( m_view, & t, paramView ) );
        THROW_ON_RC ( VViewRelease ( (VView*)paramView ) );

        THROW_ON_RC ( VViewCreateCursor ( m_view, & m_cur ) );
    }

    void CreateCursorAddColumn ( const string & p_testName,
                                 const char *   p_colName )
    {
        CreateCursor ( p_testName );
        m_columnIdx = AddColumn ( p_colName );
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
    CreateCursorAddColumn ( GetName (), ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    REQUIRE_EQ ( string ("blah"), ReadAscii ( 1, m_columnIdx ) );
    REQUIRE_EQ ( string ("eeee"), ReadAscii ( 2, m_columnIdx ) );
}

//TODO: VViewCreateCursor with multiple table/view parameters

FIXTURE_TEST_CASE( ViewCursor_ListReadableColumns, ViewOnViewCursorFixture )
{
    CreateCursorAddColumn ( GetName (), ViewColumnName );
    REQUIRE_RC ( VCursorOpen ( m_cur ) );

    BSTree columns; // VColumnRef
    BSTreeInit ( & columns );
    REQUIRE_RC ( VCursorListReadableColumns ( (VCursor*)m_cur, & columns ) );
    REQUIRE_NOT_NULL ( columns . root );
    VColumnRef * root = ( VColumnRef * ) columns . root;
    REQUIRE_EQ ( string ( ViewColumnName ), ToCppString ( root -> name ) );
    BSTreeWhack ( & columns, VColumnRefWhack, NULL );
}
