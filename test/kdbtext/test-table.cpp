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
#include "../../libs/kdbtext/column.hpp"

#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/database.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <kdb/kdb-priv.h>

#include <klib/rc.h>
#include <klib/json.h>
#include <klib/namelist.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextTableTestSuite);

class KTextTable_Fixture
{
public:
    KTextTable_Fixture()
    {
    }
    ~KTextTable_Fixture()
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
    void SetupAndInflate( const char * input )
    {
        Setup( input );
        THROW_ON_RC( m_tbl -> inflate( m_error, sizeof m_error ) );
    }

    KJsonValue * m_json = nullptr;
    Table * m_tbl = nullptr;
    char m_error[1024] = {0};
};

const char * FullTable = R"({"type": "table", "name": "testtbl",
    "columns":[ {"name":"col1","type":"text"},{"name":"col2","type":"text"} ],
    "indexes":[ {"name":"i1","text":[]}, {"name":"i2","text":[]} ],
    "metadata":{"name":"", "value":"blah"}
})";

FIXTURE_TEST_CASE(KTextTable_Make_Empty, KTextTable_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextTable_Make_NoName, KTextTable_Fixture)
{
    Setup(R"({"type": "table"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextTable_Make_NoType, KTextTable_Fixture)
{
    Setup(R"({"name": "testtbl"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextTable_Make_InvalidType, KTextTable_Fixture)
{
    Setup(R"({"type": [], "name": "testtbl"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextTable_Make_WrongType, KTextTable_Fixture)
{
    Setup(R"({"type": "database", "name": "testtbl"})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextTable_Make_Flat, KTextTable_Fixture)
{
    SetupAndInflate(R"({"type": "table", "name": "testtbl"})");
    REQUIRE_EQ( string("testtbl"), m_tbl -> getName() );
}

FIXTURE_TEST_CASE(KTextTable_Make_ColumnsNoArray, KTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":{"name":"col1","type":"text"}})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextTable_Make_ColumnDuplicate, KTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":[{"name":"col1","type":"text"},{"name":"col1","type":"text"}]})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KTextTable_Make_ColumnNotObject, KTextTable_Fixture)
{
    Setup(R"({"type": "table", "name": "testtbl","columns":["1"]})");
    REQUIRE_RC_FAIL( m_tbl -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KTextTable_Make_WithColumns, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);

    REQUIRE( ! m_tbl -> hasColumn( "nocol" ) );
    REQUIRE( m_tbl -> hasColumn( "col1" ) );
    REQUIRE( m_tbl -> hasColumn( "col2" ) );
}

FIXTURE_TEST_CASE(KTextTable_Make_WithIndex, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);

    REQUIRE( ! m_tbl -> hasIndex( "noidx" ) );
    REQUIRE( m_tbl -> hasIndex( "i1" ) );
    REQUIRE( m_tbl -> hasIndex( "i2" ) );
}

FIXTURE_TEST_CASE(KTextTable_Make_WithMetadata, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    REQUIRE_NOT_NULL( m_tbl -> openMetadata() );
}

FIXTURE_TEST_CASE(KTextTable_exists_empty, KTextTable_Fixture)
{
    SetupAndInflate(R"({"type": "table", "name": "testtbl"})");
    Path p( "" );
    REQUIRE( ! m_tbl -> exists( kptTable, p ) );
}
FIXTURE_TEST_CASE(KTextTable_exists_WrongType, KTextTable_Fixture)
{
    SetupAndInflate(R"({"type": "table", "name": "testtbl"})");
    Path p( "testtbl" );
    REQUIRE( ! m_tbl -> exists( kptDatabase, p ) );
}
FIXTURE_TEST_CASE(KTextTable_exists, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl" );
    REQUIRE( m_tbl -> exists( kptTable, p ) );
}
FIXTURE_TEST_CASE(KTextTable_exists_Index, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl/idx/i1" );
    REQUIRE( m_tbl -> exists( kptIndex, p ) );
}
FIXTURE_TEST_CASE(KTextTable_exists_Column, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl/col/col2" );
    REQUIRE( m_tbl -> exists( kptColumn, p ) );
}
FIXTURE_TEST_CASE(KTextTable_exists_Metadata, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl/md" );
    REQUIRE( m_tbl -> exists( kptMetadata, p ) );
}
FIXTURE_TEST_CASE(KTextTable_exists_Metadata_Not, KTextTable_Fixture)
{
    SetupAndInflate(R"({"type": "table", "name": "testtbl"})");
    Path p( "testtbl/md" );
    REQUIRE( ! m_tbl -> exists( kptMetadata, p ) );
}

//pathType
FIXTURE_TEST_CASE(KTextTable_pathType_Empty, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "" );
    REQUIRE_EQ( (int)kptNotFound, m_tbl -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextTable_pathType_Self, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl" );
    REQUIRE_EQ( (int)kptTable, m_tbl -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextTable_pathType_Column, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl/col/col2" );
    REQUIRE_EQ( (int)kptColumn, m_tbl -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextTable_pathType_Index, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl/idx/i1" );
    REQUIRE_EQ( (int)kptIndex, m_tbl -> pathType( p ) );
}
FIXTURE_TEST_CASE(KTextTable_pathType_Metadata, KTextTable_Fixture)
{
    SetupAndInflate(FullTable);
    Path p( "testtbl/md" );
    REQUIRE_EQ( (int)kptMetadata, m_tbl -> pathType( p ) );
}

//  API

class KTextTable_ApiFixture
{
public:
    KTextTable_ApiFixture()
    {
    }
    ~KTextTable_ApiFixture()
    {
        KTableRelease( m_tbl );
    }
    void Setup( const char * input )
    {
        try
        {
            const KDBManager * mgr = nullptr;
            THROW_ON_RC( KDBManagerMakeText ( & mgr, input, m_error, sizeof m_error ) );
            THROW_ON_RC( KDBManagerOpenTableRead( mgr, & m_tbl, "%s", "testtbl" ) );
            KDBManagerRelease( mgr );
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
        }

    }

    const KTable * m_tbl = nullptr;
    char m_error[1024];
};

FIXTURE_TEST_CASE(KTextTable_AddRelease, KTextTable_ApiFixture)
{
    Setup(FullTable);

    REQUIRE_NOT_NULL( m_tbl ) ;
    REQUIRE_RC( KTableAddRef( m_tbl ) );
    REQUIRE_RC( KTableRelease( m_tbl ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KTextTable_Locked, KTextTable_ApiFixture)
{
    Setup(FullTable);
    REQUIRE( ! KTableLocked( m_tbl ) );
}

FIXTURE_TEST_CASE(KTextTable_Exists, KTextTable_ApiFixture)
{
    Setup(FullTable);
    REQUIRE( KTableExists( m_tbl, kptColumn, "col/%s", "col2" ) );
}

FIXTURE_TEST_CASE(KTextTable_IsAlias, KTextTable_ApiFixture)
{
    Setup(FullTable);
    REQUIRE( ! KTableIsAlias( m_tbl, kptColumn, nullptr, 0, "col2" ) );
}

FIXTURE_TEST_CASE(KTextTable_IsWritable, KTextTable_ApiFixture)
{
    Setup(FullTable);
    REQUIRE( ! KTableWritable( m_tbl, kptColumn, "%s", "col2" ) );
}

FIXTURE_TEST_CASE(KTextTable_OpenManagerRead, KTextTable_ApiFixture)
{
    Setup(FullTable);
    const KDBManager * mgr = nullptr;
    REQUIRE_RC( KTableOpenManagerRead( m_tbl, & mgr ) );
    REQUIRE_NOT_NULL( mgr );
    REQUIRE_RC( KDBManagerRelease( mgr ) );
}

FIXTURE_TEST_CASE(KTextTable_OpenParentRead_Null, KTextTable_ApiFixture)
{
    Setup(FullTable);
    const KDatabase * par = nullptr;
    REQUIRE_RC( KTableOpenParentRead( m_tbl, & par ) );
    REQUIRE_NULL( par );
}
FIXTURE_TEST_CASE(KTextTable_OpenParentRead, KTextTable_ApiFixture)
{
    const char * Db = R"({
        "type": "database",
        "name": "testdb",
        "tables":[ {"type": "table", "name": "tbl0-1"} ]
    })";
    const KDatabase * db;
    const KDBManager * mgr = nullptr;
    THROW_ON_RC( KDBManagerMakeText ( & mgr, Db, m_error, sizeof m_error ) );
    THROW_ON_RC( KDBManagerOpenDBRead( mgr, & db, "%s", "testdb" ) );
    KDBManagerRelease( mgr );

    REQUIRE_RC( KDatabaseOpenTableRead ( db, & m_tbl, "%s", "tbl0-1" ) );
    REQUIRE_NOT_NULL( m_tbl );

    const KDatabase * par = nullptr;
    REQUIRE_RC( KTableOpenParentRead( m_tbl, & par ) );
    REQUIRE_NOT_NULL( par );
    REQUIRE_EQ( db, par );
    KDatabaseRelease( par );

    KDatabaseRelease( db );
}

FIXTURE_TEST_CASE(KTextTable_HasRemoteData, KTextTable_ApiFixture)
{
    Setup(FullTable);
    REQUIRE( ! KTableHasRemoteData( m_tbl ) );
}

FIXTURE_TEST_CASE(KTextTable_OpenDirectoryRead, KTextTable_ApiFixture)
{
    Setup(FullTable);
    const KDirectory *dir = nullptr;
    rc_t rc = KTableOpenDirectoryRead( m_tbl, & dir );
    REQUIRE_EQ( SILENT_RC ( rcDB, rcTable, rcAccessing, rcDirectory, rcUnsupported ), rc);
}

FIXTURE_TEST_CASE(KTextTable_OpenColumnRead, KTextTable_ApiFixture)
{
    Setup(FullTable);
    const KColumn *col = nullptr;
    REQUIRE_RC( KTableOpenColumnRead( m_tbl, & col, "%s", "col2" ) );
    REQUIRE_NOT_NULL( col );
    REQUIRE_RC( KColumnRelease( col ) );
}

FIXTURE_TEST_CASE(KTextTable_OpenMetadataRead, KTextTable_ApiFixture)
{
    Setup( FullTable );
    const KMetadata * m = nullptr;
    REQUIRE_RC( KTableOpenMetadataRead ( m_tbl, &m ) );
    REQUIRE_NOT_NULL( m );
    KMetadataRelease( m );
}

FIXTURE_TEST_CASE(KTextTable_OpenIndexRead, KTextTable_ApiFixture)
{
    Setup( FullTable );
    const KIndex * idx = nullptr;
    REQUIRE_RC( KTableOpenIndexRead ( m_tbl, &idx, "%s", "i2" ) );
    REQUIRE_NOT_NULL( idx );
    KIndexRelease( idx );
}

FIXTURE_TEST_CASE(KTextTable_GetPath, KTextTable_ApiFixture)
{
    Setup(FullTable);
    const char * p = nullptr;
    rc_t rc = KTableGetPath( m_tbl, & p );
    REQUIRE_EQ( SILENT_RC ( rcDB, rcTable, rcAccessing, rcPath, rcUnsupported ), rc);
}

FIXTURE_TEST_CASE(KTextTable_GetName, KTextTable_ApiFixture)
{
    Setup(FullTable);
    const char * n = nullptr;
    REQUIRE_RC( KTableGetName( m_tbl, & n ) );
    REQUIRE_EQ( string("testtbl"), string(n) );
}

FIXTURE_TEST_CASE(KTextTable_ListCol, KTextTable_ApiFixture)
{
    Setup( FullTable );
    KNamelist * names;
    REQUIRE_RC( KTableListCol( m_tbl, & names ) );
    REQUIRE_NOT_NULL( names );
    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)2, count );
    REQUIRE( KNamelistContains( names, "col1" ) );
    REQUIRE( KNamelistContains( names, "col2" ) );
    KNamelistRelease( names );
}

FIXTURE_TEST_CASE(KTextTable_ListIdx, KTextTable_ApiFixture)
{
    Setup( FullTable );
    KNamelist * names;
    REQUIRE_RC( KTableListIdx( m_tbl, & names ) );
    REQUIRE_NOT_NULL( names );
    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)2, count );
    REQUIRE( KNamelistContains( names, "i1" ) );
    REQUIRE( KNamelistContains( names, "i2" ) );
    KNamelistRelease( names );
}

FIXTURE_TEST_CASE(KTextTable_MetaCompare, KTextTable_ApiFixture)
{
    Setup( FullTable );
    bool equal = false;
    rc_t rc = KTableMetaCompare( m_tbl, m_tbl, "", &equal );
    REQUIRE_EQ( SILENT_RC ( rcDB, rcTable, rcComparing, rcMetadata, rcUnsupported ), rc);
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

const char UsageDefaultName[] = "Test_KDBText_Table";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KTextTableTestSuite(argc, argv);
    return rc;
}

}
