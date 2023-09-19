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
* Unit tests for read-side KMetadata
*/

#include <ktst/unit_test.hpp>

#include <klib/rc.h>
#include <kdb/manager.h>
#include <kdb/table.h>
#include <arch-impl.h>

#include <../libs/kdb/rmetadatanode.h>
#include <../libs/kdb/rdbmgr.h>

using namespace std;

TEST_SUITE(KDBRMDataNodeTestSuite);

static const string ScratchDir = "./data/";

class KRMDataNode_Fixture
{
public:
    KRMDataNode_Fixture()
    {
        THROW_ON_RC( KDirectoryNativeDir( & m_dir ) );
        THROW_ON_RC( KDBManagerMakeRead ( & m_mgr, m_dir ) );
    }
    ~KRMDataNode_Fixture()
    {
        KMDataNodeRelease( m_node );
        KMetadataRelease( m_meta );
        KDBManagerRelease( m_mgr );
        KDirectoryRelease( m_dir );
    }

    void Open( const char * path, const char * node )
    {
        const KDirectory *subdir;
        THROW_ON_RC(  KDirectoryOpenDirRead( m_dir, &subdir, false, "%s", path ) );
        THROW_ON_RC( KDBManagerOpenMetadataReadInt ( m_mgr, (KMetadata **)& m_meta, subdir, 0, false ) );
        THROW_ON_RC( KMetadataOpenNodeRead ( m_meta, & m_node, "%s", node ) );

        THROW_ON_RC( KDirectoryRelease( subdir ) );
    }

    KDirectory * m_dir = nullptr;
    const KDBManager * m_mgr = nullptr;
    const KMetadata * m_meta = nullptr;
    const KMDataNode * m_node = nullptr;
};

//NB for now make the simplest calls possible, to test the vtable plumbing

FIXTURE_TEST_CASE(KRMDataNode_AddRelease, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    REQUIRE_NOT_NULL( m_node );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_node -> dad . refcount ) );
    REQUIRE_RC( KMDataNodeAddRef( m_node ) );
    REQUIRE_EQ( 3, (int)atomic32_read( & m_node -> dad . refcount ) );
    REQUIRE_RC( KMDataNodeRelease( m_node ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_node -> dad . refcount ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KRMDataNode_ByteOrder, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    bool reversed = true;
    REQUIRE_RC( KMDataNodeByteOrder ( m_node, &reversed ) );
    REQUIRE( ! reversed );
}

FIXTURE_TEST_CASE(KRMDataNode_Read, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    char buffer[1024];
    size_t num_read;
    size_t remaining;
    REQUIRE_RC( KMDataNodeRead ( m_node, 0, buffer, sizeof buffer, & num_read, & remaining ) );
    REQUIRE_EQ( (size_t)0, num_read );
    REQUIRE_EQ( (size_t)0, remaining );
}

FIXTURE_TEST_CASE(KRMDataNode_OpenRead, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    const KMDataNode * node = nullptr;
    REQUIRE_RC( KMDataNodeOpenNodeRead ( m_node, &node, "%s", "COL1" ) );
    REQUIRE_NOT_NULL( node );
    KMDataNodeRelease( node );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int8_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB8 ( m_node,  &b ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB16, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int16_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB16 ( m_node,  &b ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB32, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int32_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB32 ( m_node,  &b ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int64_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB64 ( m_node,  &b ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB128, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int128_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB128 ( m_node,  &b ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsI16, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int16_t i;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsI16 ( m_node,  &i ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsU16, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint16_t u;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsU16 ( m_node,  &u ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsI32, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int32_t i;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsI32 ( m_node,  &i ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsU32, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint32_t u;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsU32 ( m_node,  &u ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsI64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int64_t i;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsI64 ( m_node,  &i ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsU64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint64_t u;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsU64 ( m_node,  &u ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsF64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    double f;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsF64 ( m_node,  &f ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadCString, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    char buffer[1024];
    size_t size = 1;
    REQUIRE_RC( KMDataNodeReadCString ( m_node, buffer, sizeof buffer, & size ) );
    REQUIRE_EQ( (size_t)0, size );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttr, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    char buffer[1024];
    size_t size;
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "qq", buffer, sizeof buffer, & size ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI16, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int16_t i;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsI16 ( m_node, "qq", & i ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU16, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint16_t u;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsU16 ( m_node, "qq", & u ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI32, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int32_t i;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsI32 ( m_node, "qq", & i ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU32, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint32_t u;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsU32 ( m_node, "qq", & u ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int64_t i;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsI64 ( m_node, "qq", & i ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint64_t u;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsU64 ( m_node, "qq", & u ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsF64, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    double f;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsF64 ( m_node, "qq", & f ) );
}

FIXTURE_TEST_CASE(KRMDataNode_Compare, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    // compare to itself
    const KMDataNode * node = nullptr;
    REQUIRE_RC( KMetadataOpenNodeRead ( m_meta, & node, "%s", "col" ) );

    bool equal = false;
    REQUIRE_RC( KMDataNodeCompare( m_node, node, & equal ) );
    REQUIRE( equal );

    KMDataNodeRelease( node );
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

const char UsageDefaultName[] = "Test_KDB_RMDataNode";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBRMDataNodeTestSuite(argc, argv);
    return rc;
}

}
