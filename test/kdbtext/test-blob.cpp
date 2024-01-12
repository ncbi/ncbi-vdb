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

#include "../../libs/kdbtext/columnblob.hpp"

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextColumnBlobTestSuite);

const char * TestColumn = R"({"name":"col",
            "type":"ascii",
            "data":
                [
                    {"row":1,"value":"AGCT"},
                    {"row":2,"value":"AGCT"}
                ]})";

class KTextColumnBlob_Fixture
{
public:
    KTextColumnBlob_Fixture()
    {
    }
    ~KTextColumnBlob_Fixture()
    {
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

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Empty, KTextColumnBlob_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_Make_TypeMissing, KTextColumnBlob_Fixture)
{
    Setup(R"({"name":"col"})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumnBlob_Make, KTextColumnBlob_Fixture)
{
    Setup(TestColumn);
    REQUIRE_RC( m_col -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("col"), m_col->getName() );
}

FIXTURE_TEST_CASE(KTextColumnBlob_Make_DataNotArray, KTextColumnBlob_Fixture)
{
    Setup(R"({"name":"col","data":{}})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumnBlob_Make_CellNotObject, KTextColumnBlob_Fixture)
{
    Setup(R"({"name":"col","data":[1]})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_CellMake_RowMissing, KTextColumnBlob_Fixture)
{
    Setup(R"({"name":"col","data":[{}]})");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextColumnBlob_CellMake_RowBad, KTextColumnBlob_Fixture)
{
    Setup(R"({"name":"col","data": [ {"row":"a","value":"q"} ] })");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumnBlob_CellMake_ValueMissing, KTextColumnBlob_Fixture)
{
    Setup(R"({"name":"col","data": [ {"row":"1"} ] })");
    REQUIRE_RC_FAIL( m_col -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextColumnBlob_IdRange, KTextColumnBlob_Fixture)
{
    Setup(TestColumn);
    REQUIRE_RC( m_col -> inflate( m_error, sizeof m_error ) );
    auto r = m_col->idRange();
    REQUIRE_EQ( (int64_t)1, r . first );
    REQUIRE_EQ( (uint64_t)2, r . second );
}

// API

const char * FullTable = R"({"type": "table", "name": "testtbl",
    "columns":[
        {
            "name":"col",
            "type":"ascii",
            "data":
                [
                    {"row":1,"value":"AGCT"},
                    {"row":2,"value":"AGCT"},
                    {"row":11,"value":"AGCT"},
                    {"row":22,"value":"AGCT"}
                ],
            "metadata":{"name":"", "value":"blah"}
        }
    ]
})";

class KTextColumnBlob_ApiFixture
{
public:
    KTextColumnBlob_ApiFixture()
    {
    }
    ~KTextColumnBlob_ApiFixture()
    {
        KColumnRelease( m_col );
        KTableRelease( m_tbl );
        KDBManagerRelease( m_mgr );
    }
    void Setup( const char * input, const char * col )
    {
        try
        {
            THROW_ON_RC( KDBManagerMakeText ( & m_mgr, input, m_error, sizeof m_error ) );
            THROW_ON_RC( KDBManagerOpenTableRead( m_mgr, & m_tbl, "%s", "testtbl" ) );
            THROW_ON_RC( KTableOpenColumnRead( m_tbl, & m_col, "%s", col ) );
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
            throw;
        }

    }

    const KDBManager * m_mgr = nullptr;
    const KTable * m_tbl = nullptr;
    const KColumn * m_col = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KColumn_AddRelease, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");

    REQUIRE_RC( KColumnAddRef( m_col ) );
    REQUIRE_RC( KColumnRelease( m_col ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KColumn_Locked, KTextColumnBlob_ApiFixture)
{   // always false for this library
    Setup(FullTable, "col");
    REQUIRE( ! KColumnLocked( m_col ) );
}
FIXTURE_TEST_CASE(KColumn_Version, KTextColumnBlob_ApiFixture)
{   // always 0 for this library
    Setup(FullTable, "col");
    uint32_t version = 99;
    REQUIRE_RC( KColumnVersion( m_col, &version ) );
    REQUIRE_EQ( (uint32_t)0, version );
}
FIXTURE_TEST_CASE(KColumn_ByteOrder, KTextColumnBlob_ApiFixture)
{   // always false for this library
    Setup(FullTable, "col");
    bool reversed = true;
    REQUIRE_RC( KColumnByteOrder( m_col, &reversed ) );
    REQUIRE( ! reversed );
}

FIXTURE_TEST_CASE(KColumn_IdRange, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");
    int64_t first = 0;
    uint64_t count = 0;
    REQUIRE_RC( KColumnIdRange ( m_col, & first, & count ) );
    REQUIRE_EQ( (int64_t)1, first );
    REQUIRE_EQ( (uint64_t)22, count );
}

FIXTURE_TEST_CASE(KColumn_FindFirstRowId, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");
    int64_t found = 0;
    REQUIRE_RC( KColumnFindFirstRowId ( m_col, & found, 4 ) );
    REQUIRE_EQ( (int64_t)11, found );
}

FIXTURE_TEST_CASE(KColumn_OpenManagerRead, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");
    const KDBManager * mgr = nullptr;
    REQUIRE_RC( KColumnOpenManagerRead ( m_col, & mgr ) );
    REQUIRE_EQ( m_mgr, mgr );
    REQUIRE_RC( KDBManagerRelease( mgr ) );
}

FIXTURE_TEST_CASE(KColumn_OpenParentRead, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");

    const KTable * par = nullptr;
    REQUIRE_RC( KColumnOpenParentRead( m_col, & par ) );
    REQUIRE_NOT_NULL( par );
    REQUIRE_EQ( m_tbl, par );
    KTableRelease( par );
}

FIXTURE_TEST_CASE(KColumn_OpenMetadataRead, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");
    const KMetadata * m = nullptr;
    REQUIRE_RC( KColumnOpenMetadataRead ( m_col, &m ) );
    REQUIRE_NOT_NULL( m );
    REQUIRE_RC( KMetadataRelease( m ) );
}

FIXTURE_TEST_CASE(KColumn_OpenBlobRead, KTextColumnBlob_ApiFixture)
{
    Setup(FullTable, "col");
    const KColumnBlob * blob = nullptr;
    REQUIRE_RC( KColumnOpenBlobRead ( m_col, & blob, 11 ) );
    REQUIRE_NOT_NULL( blob );
    REQUIRE_RC( KColumnBlobRelease( blob ) );
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
    rc_t rc=KTextColumnBlobTestSuite(argc, argv);
    return rc;
}

}
