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
* Unit tests for KDBColumn for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/column.hpp"

#include <kdb/column.h>
#include <kdb/manager.h>
#include <kdb/table.h>

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextColumnTestSuite);

const char * TestColumn = R"({"name":"col",
            "data":
                [
                    {"row":1,"value":"AGCT"},
                    {"row":2,"value":"AGCT"}
                ]})";

class KTextColumn_Fixture
{
public:
    KTextColumn_Fixture()
    {
    }
    ~KTextColumn_Fixture()
    {
        delete m_col;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_col = new Column( json, nullptr );
    }

    KJsonValue * m_json = nullptr;
    Column * m_col = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KTextColumn_Make_Empty, KTextColumn_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumn_Make, KTextColumn_Fixture)
{
    Setup(TestColumn);
    REQUIRE_RC( m_col -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("col"), m_col->getName() );
}

FIXTURE_TEST_CASE(KTextColumn_Make_DataNotArray, KTextColumn_Fixture)
{
    Setup(R"({"name":"col","data":{}})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumn_Make_CellNotObject, KTextColumn_Fixture)
{
    Setup(R"({"name":"col","data":[1]})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumn_CellMake_RowMissing, KTextColumn_Fixture)
{
    Setup(R"({"name":"col","data":[{}]})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumn_CellMake_ValueMissing, KTextColumn_Fixture)
{
    Setup(R"({"name":"col","data": [ {"row":"1"} ] })");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumn_IdRange, KTextColumn_Fixture)
{
    Setup(TestColumn);
    auto r = m_col->idRange();
    REQUIRE_EQ( (int64_t)1, r . first );
    REQUIRE_EQ( (uint64_t)2, r . second );
}

// API

const char * FullTable = R"({"type": "table", "name": "testtbl",
    "columns":[
        {
            "name":"col",
            "data":
                [
                    {"row":1,"value":"AGCT"},
                    {"row":2,"value":"AGCT"}
                ]
        }
    ]
})";

class KTextColumn_ApiFixture
{
public:
    KTextColumn_ApiFixture()
    {
    }
    ~KTextColumn_ApiFixture()
    {
        KColumnRelease( m_col );
    }
    void Setup( const char * input, const char * col )
    {
        try
        {
            const KDBManager * mgr = nullptr;
            THROW_ON_RC( KDBManagerMakeText ( & mgr, input, m_error, sizeof m_error ) );
            const KTable * tbl;
            THROW_ON_RC( KDBManagerOpenTableRead( mgr, & tbl, "%s", "testtbl" ) );
            THROW_ON_RC( KTableOpenColumnRead( tbl, & m_col, "%s", col ) );
            KTableRelease( tbl );
            KDBManagerRelease( mgr );
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
            throw;
        }

    }

    const KColumn * m_col = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KColumn_AddRelease, KTextColumn_ApiFixture)
{
    Setup(FullTable, "col");

    REQUIRE_RC( KColumnAddRef( m_col ) );
    REQUIRE_RC( KColumnRelease( m_col ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KColumn_Locked, KTextColumn_ApiFixture)
{   // always false for this library
    Setup(FullTable, "col");
    REQUIRE( ! KColumnLocked( m_col ) );
}
FIXTURE_TEST_CASE(KColumn_Version, KTextColumn_ApiFixture)
{   // always 0 for this library
    Setup(FullTable, "col");
    uint32_t version = 99;
    REQUIRE_RC( KColumnVersion( m_col, &version ) );
    REQUIRE_EQ( (uint32_t)0, version );
}
FIXTURE_TEST_CASE(KColumn_ByteOrder, KTextColumn_ApiFixture)
{   // always false for this library
    Setup(FullTable, "col");
    bool reversed = true;
    REQUIRE_RC( KColumnByteOrder( m_col, &reversed ) );
    REQUIRE( ! reversed );
}

FIXTURE_TEST_CASE(KColumn_IdRange, KTextColumn_ApiFixture)
{
    Setup(FullTable, "col");
    int64_t first = 0;
    uint64_t count = 0;
    REQUIRE_RC( KColumnIdRange ( m_col, & first, & count ) );
    REQUIRE_EQ( (int64_t)1, first );
    REQUIRE_EQ( (uint64_t)2, count );
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

const char UsageDefaultName[] = "Test_KDBText_Column";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KTextColumnTestSuite(argc, argv);
    return rc;
}

}
