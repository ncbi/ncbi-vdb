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

#include "VDB_Fixture.hpp"

#include <vdb/vdb-priv.h>
#include <kdb/manager.h>

#include <klib/debug.h>
#include <ktst/unit_test.hpp>

using namespace std;

TEST_SUITE( VdbTextSuite );

class TextVdbFixture
{
public:
    TextVdbFixture() {}
    ~TextVdbFixture()
    {
        VDBManagerRelease( m_mgr );
    }

    void Setup( const string & input )
    {
//cout << input<<endl;
        const KDBManager * kdb = nullptr;
        THROW_ON_RC( KDBManagerMakeText( & kdb, input.c_str(), m_error, sizeof( m_error ) ) );
        THROW_ON_RC( VDBManagerMakeReadWithKDBManager( & m_mgr, kdb ));
        THROW_ON_RC( KDBManagerRelease( kdb ));
    }

    char m_error[1024];
    const VDBManager * m_mgr = nullptr;
};

FIXTURE_TEST_CASE( VdbMgr_NullSelf, TextVdbFixture )
{
    const char * input = "{}";
    const KDBManager * kdb = nullptr;
    REQUIRE_RC( KDBManagerMakeText( & kdb, input, m_error, sizeof( m_error ) ) );
    REQUIRE_RC_FAIL( VDBManagerMakeReadWithKDBManager( nullptr, kdb ));
    REQUIRE_RC( KDBManagerRelease( kdb ));
}
FIXTURE_TEST_CASE( VdbMgr_NullKdb, TextVdbFixture )
{
    REQUIRE_RC_FAIL( VDBManagerMakeReadWithKDBManager( & m_mgr, nullptr ));
}

FIXTURE_TEST_CASE( VdbMgr, TextVdbFixture )
{
    const KDBManager * kdb = nullptr;
    const char * input = "{}";
    REQUIRE_RC( KDBManagerMakeText( & kdb, input, m_error, sizeof( m_error ) ) );
    REQUIRE_RC( VDBManagerMakeReadWithKDBManager( & m_mgr, kdb ));
    REQUIRE_NOT_NULL( m_mgr );
    REQUIRE_RC( KDBManagerRelease( kdb ));
}

FIXTURE_TEST_CASE( VdbMgr_OpenDB_Empty, TextVdbFixture )
{
    Setup("{}");
    const VDatabase * db = nullptr;
    REQUIRE_RC_FAIL( VDBManagerOpenDBRead ( m_mgr, &db, nullptr, "db" ) );
}

class TextVdbReadFixture : public TextVdbFixture
{
public:
const char * TestDB =
    R"({
        "type": "database",
        "name": "testdb",
        "metadata": {
            "name":"",
            "root":{
                "name":"",
                "children":[{
                    "name":"schema",
                    "value":"version 2.0;
                             table table1 #1.0.0 { column %t column1; };
                             database root_database #1 { table table1 #1 TABLE1; } ;",
                    "attributes":{"name":"root_database#1"}
                }]
            }
        },
        "tables":[
            {
                "type":"table",
                "name":"TABLE1",
                "metadata": {
                    "name":"",
                    "root":{
                        "name":"",
                        "children":[{
                            "name":"schema",
                            "value":"version 2.0;
                                 table table1 #1.0.0 { column %t column1; };
                                database root_database #1 { table table1 #1 TABLE1; } ;",
                            "attributes":{"name":"table1#1"}
                        }]
                    }
                },
                "columns":[
                    {
                        "name":"column1",
                        "type":"%t",
                        "data":
                        [
                            {"row":1,"value":%v1},
                            {"row":2,"value":%v2}
                        ]
                    }
                ]
            }
        ]
    })";

    string
    FormatSchema( const char * type, const char * value1, const char * value2 )
    {
        string schema = TestDB;
        while ( true )
        {
            auto p = schema.find("%t");
            if ( p == schema.npos )
            {
                break;
            }
            schema.replace(p, 2, type);
        }
        schema.replace(schema.find("%v1"), 3, value1);
        schema.replace(schema.find("%v2"), 3, value2);
        return schema;
    }

    ~TextVdbReadFixture()
    {
        VCursorRelease( curs );
        VDatabaseRelease( db );
    }

    void Setup( const char * type, const char * value1, const char * value2 )
    {
        TextVdbFixture::Setup( FormatSchema( type, value1, value2 ) );
        THROW_ON_RC( VDBManagerOpenDBRead ( m_mgr, &db, nullptr, "testdb" ) );

        const VTable * tbl;
        THROW_ON_RC( VDatabaseOpenTableRead( db, &tbl, "TABLE1" ) );
        THROW_ON_FALSE( tbl != nullptr );

        THROW_ON_RC( VTableCreateCursorRead(tbl, &curs) );
        THROW_ON_RC( VCursorAddColumn(curs, &colId, "column1") );

        THROW_ON_RC( VCursorOpen(curs) );
        THROW_ON_RC( VCursorOpenRow ( curs ) );

        THROW_ON_RC( VTableRelease( tbl ) );
    }

    const VDatabase * db = nullptr;
    const VCursor * curs = nullptr;
    uint32_t colId = 0;
    uint32_t rowLen = 0;
};

FIXTURE_TEST_CASE( VdbMgr_OpenDB, TextVdbReadFixture )
{
    TextVdbFixture::Setup(FormatSchema( "ascii", "\"AGCT\"", "\"TCGAT\""));

    REQUIRE_RC( VDBManagerOpenDBRead ( m_mgr, &db, nullptr, "testdb" ) );
    REQUIRE_NOT_NULL( db );
}

FIXTURE_TEST_CASE( VdbMgr_Read_Ascii, TextVdbReadFixture )
{
    Setup( "ascii", "\"AGCT\"", "\"TCGAT\"" );

    char buf[1024];
    REQUIRE_RC( VCursorReadDirect ( curs, 1, colId, 8, buf, sizeof ( buf ), & rowLen ) );
    REQUIRE_EQ( 4, (int)rowLen );
    REQUIRE_EQ( string("AGCT"), string( buf, rowLen ) );
    REQUIRE_RC( VCursorReadDirect ( curs, 2, colId, 8, buf, sizeof ( buf ), & rowLen ) );
    REQUIRE_EQ( 5, (int)rowLen );
    REQUIRE_EQ( string("TCGAT"), string( buf, rowLen ) );
}

FIXTURE_TEST_CASE( VdbMgr_Read_U32, TextVdbReadFixture )
{
    Setup( "U32", "12345", "54321" );

    uint32_t v = 0;
    REQUIRE_RC( VCursorReadDirect ( curs, 1, colId, sizeof( v )*8, &v, 1, & rowLen ) );
    REQUIRE_EQ( 1, (int)rowLen );
    REQUIRE_EQ( (uint32_t)12345, v );
    REQUIRE_RC( VCursorReadDirect ( curs, 2, colId, sizeof( v )*8, &v, 1, & rowLen ) );
    REQUIRE_EQ( 1, (int)rowLen );
    REQUIRE_EQ( (uint32_t)54321, v );
}

FIXTURE_TEST_CASE( VdbMgr_Read_B16, TextVdbReadFixture )
{
    Setup( "B16", "1234", "4321" );

    uint16_t v = 0;
    REQUIRE_RC( VCursorReadDirect ( curs, 1, colId, sizeof( v )*8, &v, 1, & rowLen ) );
    REQUIRE_EQ( 1, (int)rowLen );
    REQUIRE_EQ( (uint16_t)1234, v );
    REQUIRE_RC( VCursorReadDirect ( curs, 2, colId, sizeof( v )*8, &v, 1, & rowLen ) );
    REQUIRE_EQ( 1, (int)rowLen );
    REQUIRE_EQ( (uint16_t)4321, v );
}

FIXTURE_TEST_CASE( VdbMgr_Read_I8, TextVdbReadFixture )
{
    Setup( "I8", "12", "21" );

    int8_t v = 0;
    REQUIRE_RC( VCursorReadDirect ( curs, 1, colId, sizeof( v )*8, &v, 1, & rowLen ) );
    REQUIRE_EQ( 1, (int)rowLen );
    REQUIRE_EQ( (int8_t)12, v );
    REQUIRE_RC( VCursorReadDirect ( curs, 2, colId, sizeof( v )*8, &v, 1, & rowLen ) );
    REQUIRE_EQ( 1, (int)rowLen );
    REQUIRE_EQ( (int8_t)21, v );
}

//////////////////////////////////////////// Main
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

const char UsageDefaultName[] = "test-vdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=VdbTextSuite(argc, argv);
    return rc;
}

}
