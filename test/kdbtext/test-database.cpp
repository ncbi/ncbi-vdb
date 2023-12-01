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
* Unit tests for KDBDatabase for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/database.hpp"

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextDatabaseTestSuite);

class KDBTextDatabase_Fixture
{
public:
    KDBTextDatabase_Fixture()
    {
    }
    ~KDBTextDatabase_Fixture()
    {
        delete m_db;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_db = new Database( json );
    }

    KJsonValue * m_json = nullptr;
    Database * m_db = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Empty, KDBTextDatabase_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_NoName, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "database"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_NoType, KDBTextDatabase_Fixture)
{
    Setup(R"({"name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_InvalidType, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": [], "name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_WrongType, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "table", "name": "testdb"})");
    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Flat, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "database", "name": "testdb"})");

    REQUIRE_RC( m_db -> inflate( m_error, sizeof m_error ) );

    REQUIRE_EQ( string("testdb"), m_db -> getName() );
}

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_NotArray, KDBTextDatabase_Fixture)
{
    Setup(R"({"type": "database", "name": "testdb","databases":{"type": "database", "name":"subdb1"} })");

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_ElementNull, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb","databases":[ null ]})" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_ElementBad, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb","databases":[ {"type": "notadatabase", "name":"subdb1"} ]})" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested_Duplicate, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb","databases":[ {"type": "database", "name":"subdb1"} , {"type": "database", "name":"subdb1"} ]})" );

    REQUIRE_RC_FAIL( m_db -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextDatabase_Make_Nested, KDBTextDatabase_Fixture)
{
    Setup( R"({"type": "database", "name": "testdb","databases":[ {"type": "database", "name":"subdb1"} , {"type": "database", "name":"subdb2"} ]})" );

    REQUIRE_RC( m_db -> inflate( m_error, sizeof m_error ) );

    REQUIRE_NULL( m_db -> getDatabase( "notthere") );
    REQUIRE_NOT_NULL( m_db -> getDatabase( "subdb1") );
    REQUIRE_NOT_NULL( m_db -> getDatabase( "subdb2") );
}

//TODO: tables, metadata, indexes

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

const char UsageDefaultName[] = "Test_KDBText_Database";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextDatabaseTestSuite(argc, argv);
    return rc;
}

}
