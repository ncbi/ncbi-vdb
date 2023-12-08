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
* Unit tests for KDBTable for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/table.hpp"

#include <kdb/manager.h>

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextTableTestSuite);

class KDBTextTable_Fixture
{
public:
    KDBTextTable_Fixture()
    {
    }
    ~KDBTextTable_Fixture()
    {
        delete m_tbl;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_tbl = new Table( json );
    }

    KJsonValue * m_json = nullptr;
    Table * m_tbl = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KDBTextTable_Make_Empty, KDBTextTable_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextTable_Make_NoName, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextTable_Make_NoType, KDBTextTable_Fixture)
{
    Setup(R"({"name": "testtbl"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextTable_Make_InvalidType, KDBTextTable_Fixture)
{
    Setup(R"({"type": [], "name": "testtbl"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextTable_Make_WrongType, KDBTextTable_Fixture)
{
    Setup(R"({"type": "database", "name": "testtbl"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextTable_Make_Flat, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl"})");

    REQUIRE_RC( m_tbl -> inflate( m_error, sizeof m_error ) );

    REQUIRE_EQ( string("testtbl"), m_tbl -> getName() );
}

FIXTURE_TEST_CASE(KDBTextTable_Make_ColumnsNoArray, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":{"name":"col1"}})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextTable_Make_ColumnDuplicate, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":[{"name":"col1"},{"name":"col1"}]})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextTable_Make_ColumnNotObject, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":["1"]})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextTable_Make_WithColumns, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":[{"name":"col1"},{"name":"col2"}]})");

    REQUIRE_RC( m_tbl -> inflate( m_error, sizeof m_error ) );

    REQUIRE_NULL( m_tbl -> getColumn( "nocol" ) );
    REQUIRE_NOT_NULL( m_tbl -> getColumn( "col1" ) );
    REQUIRE_NOT_NULL( m_tbl -> getColumn( "col2" ) );
}

//TODO: columns, metadata, indexes

FIXTURE_TEST_CASE(KDBTextTable_exists_empty, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl"})");
    REQUIRE_RC( m_tbl-> inflate( m_error, sizeof m_error ) );

    Path p( "" );
    REQUIRE( ! m_tbl -> exists( kptTable, p ) );
}
FIXTURE_TEST_CASE(KDBTextTable_exists_WrongType, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl"})");
    REQUIRE_RC( m_tbl-> inflate( m_error, sizeof m_error ) );

    Path p( "testtbl" );
    REQUIRE( ! m_tbl -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KDBTextTable_exists, KDBTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl"})");
    REQUIRE_RC( m_tbl-> inflate( m_error, sizeof m_error ) );

    Path p( "testtbl" );
    REQUIRE( m_tbl -> exists( kptTable, p ) );
}
//TODO: columns, metadata, indexes

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

const char UsageDefaultName[] = "Test_KDBText_Table";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextTableTestSuite(argc, argv);
    return rc;
}

}
