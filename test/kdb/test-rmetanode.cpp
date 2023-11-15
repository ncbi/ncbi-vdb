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
#include <klib/namelist.h>

#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/kdb-priv.h>
#include <kdb/namelist.h>

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

    void Open( const string & path, const char * node )
    {
        const KDirectory *subdir;
        THROW_ON_RC( KDirectoryOpenDirRead( m_dir, &subdir, false, "%s", path.c_str() ) );
        THROW_ON_RC( KDBRManagerOpenMetadataReadInt ( m_mgr, (KMetadata **)& m_meta, subdir, 0, false ) );
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
    REQUIRE_EQ( 2, (int)atomic32_read( & m_node -> refcount ) );
    REQUIRE_RC( KMDataNodeAddRef( m_node ) );
    REQUIRE_EQ( 3, (int)atomic32_read( & m_node -> refcount ) );
    REQUIRE_RC( KMDataNodeRelease( m_node ) );
    REQUIRE_EQ( 2, (int)atomic32_read( & m_node -> refcount ) );
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

FIXTURE_TEST_CASE(KRMDataNode_ReadB8_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int8_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB8 ( m_node,  &b ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadB8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    int8_t b;
    REQUIRE_RC( KMDataNodeReadB8 ( m_node,  &b ) );
    REQUIRE_EQ( (int8_t)4, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB16_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int16_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB16 ( m_node,  &b ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadB16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int16_t b;
    REQUIRE_RC( KMDataNodeReadB16 ( m_node,  &b ) );
    REQUIRE_EQ( (int16_t)0x0102, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB32_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int32_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB32 ( m_node,  &b ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadB32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b32" );
    int32_t b;
    REQUIRE_RC( KMDataNodeReadB32 ( m_node,  &b ) );
    REQUIRE_EQ( (int32_t)0x01020304, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int64_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB64 ( m_node,  &b ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadB64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b64" );
    int64_t b;
    REQUIRE_RC( KMDataNodeReadB64 ( m_node,  &b ) );
    REQUIRE_EQ( (int64_t)0x0102030405060708, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadB128_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int128_t b;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadB128 ( m_node,  &b ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadB128, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b128" );
    uint128_t b;
    REQUIRE_RC( KMDataNodeReadB128 ( m_node,  & b ) );
    REQUIRE_EQ( (uint64_t)0x0102030405060708, uint128_hi( & b ) );
    REQUIRE_EQ( (uint64_t)0x1112131415161718, uint128_lo( & b ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsI16_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int16_t i;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsI16 ( m_node,  &i ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI16_From8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    int16_t b;
    REQUIRE_RC( KMDataNodeReadAsI16 ( m_node,  &b ) );
    REQUIRE_EQ( (int16_t)4, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI16_From16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int16_t b;
    REQUIRE_RC( KMDataNodeReadAsI16 ( m_node,  &b ) );
    REQUIRE_EQ( (int16_t)0x0102, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsU16_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint16_t u;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsU16 ( m_node,  &u ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU16_From8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    uint16_t b;
    REQUIRE_RC( KMDataNodeReadAsU16 ( m_node,  &b ) );
    REQUIRE_EQ( (uint16_t)4, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU16_From16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    uint16_t b;
    REQUIRE_RC( KMDataNodeReadAsU16 ( m_node,  &b ) );
    REQUIRE_EQ( (uint16_t)0x0102, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsI32_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int32_t i;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsI32 ( m_node,  &i ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI32_From8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    int32_t b;
    REQUIRE_RC( KMDataNodeReadAsI32 ( m_node,  &b ) );
    REQUIRE_EQ( (int32_t)4, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI32_From16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int32_t b;
    REQUIRE_RC( KMDataNodeReadAsI32 ( m_node,  &b ) );
    REQUIRE_EQ( (int32_t)0x0102, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI32_From32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b32" );
    int32_t b;
    REQUIRE_RC( KMDataNodeReadAsI32 ( m_node,  &b ) );
    REQUIRE_EQ( (int32_t)0x01020304, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsU32_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint32_t u;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsU32 ( m_node,  &u ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU32_From8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    uint32_t b;
    REQUIRE_RC( KMDataNodeReadAsU32 ( m_node,  &b ) );
    REQUIRE_EQ( (uint32_t)4, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU32_From16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    uint32_t b;
    REQUIRE_RC( KMDataNodeReadAsU32 ( m_node,  &b ) );
    REQUIRE_EQ( (uint32_t)0x0102, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU32_From32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b32" );
    uint32_t b;
    REQUIRE_RC( KMDataNodeReadAsU32 ( m_node,  &b ) );
    REQUIRE_EQ( (uint32_t)0x01020304, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsI64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int64_t i;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsI64 ( m_node,  &i ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI64_From8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    int64_t b;
    REQUIRE_RC( KMDataNodeReadAsI64 ( m_node,  &b ) );
    REQUIRE_EQ( (int64_t)4, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI64_From16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int64_t b;
    REQUIRE_RC( KMDataNodeReadAsI64 ( m_node,  &b ) );
    REQUIRE_EQ( (int64_t)0x0102, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI64_From32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b32" );
    int64_t b;
    REQUIRE_RC( KMDataNodeReadAsI64 ( m_node,  &b ) );
    REQUIRE_EQ( (int64_t)0x01020304, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsI64_From64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b64" );
    int64_t b;
    REQUIRE_RC( KMDataNodeReadAsI64 ( m_node,  &b ) );
    REQUIRE_EQ( (int64_t)0x0102030405060708, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsU64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint64_t u;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsU64 ( m_node,  &u ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU64_From8, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col/COL1/row_count" );
    uint64_t b;
    REQUIRE_RC( KMDataNodeReadAsU64 ( m_node,  &b ) );
    REQUIRE_EQ( (uint64_t)4, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU64_From16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    uint64_t b;
    REQUIRE_RC( KMDataNodeReadAsU64 ( m_node,  &b ) );
    REQUIRE_EQ( (uint64_t)0x0102, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU64_From32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b32" );
    uint64_t b;
    REQUIRE_RC( KMDataNodeReadAsU64 ( m_node,  &b ) );
    REQUIRE_EQ( (uint64_t)0x01020304, b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsU64_From64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b64" );
    uint64_t b;
    REQUIRE_RC( KMDataNodeReadAsU64 ( m_node,  &b ) );
    REQUIRE_EQ( (uint64_t)0x0102030405060708, b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAsF64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    double f;
    rc_t rc = SILENT_RC( rcDB,rcMetadata,rcReading,rcTransfer,rcIncomplete );
    REQUIRE_EQ( rc, KMDataNodeReadAsF64 ( m_node,  &f ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsF64_From32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b32" );
    const uint32_t u32 = 0x01020304;
    double b;
    REQUIRE_RC( KMDataNodeReadAsF64 ( m_node,  &b ) );
    REQUIRE_EQ( ( ( const float* ) &u32 ) [ 0 ], (float)b );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAsF64_From64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b64" );
    const uint64_t u64 = 0x0102030405060708;
    double b;
    REQUIRE_RC( KMDataNodeReadAsF64 ( m_node,  &b ) );
    REQUIRE_EQ( ( ( const double* ) &u64 ) [ 0 ], b );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadCString_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    char buffer[1024];
    size_t size = 1;
    REQUIRE_RC( KMDataNodeReadCString ( m_node, buffer, sizeof buffer, & size ) );
    REQUIRE_EQ( (size_t)0, size );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadCString, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "schema" );
    char buffer[1024];
    size_t size = 1;
    REQUIRE_RC( KMDataNodeReadCString ( m_node, buffer, sizeof buffer, & size ) );
    const string SchemaText  =
"version 2;table table1#1{column ascii col;}database root_database#1{table table1#1 SEQUENCE;}";
    REQUIRE_EQ( SchemaText.size(), size );
    REQUIRE_EQ( SchemaText, string( buffer ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttr_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    char buffer[1024];
    size_t size;
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "qq", buffer, sizeof buffer, & size ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttr, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "schema" );
    char buffer[1024];
    size_t size;
    REQUIRE_RC( KMDataNodeReadAttr ( m_node, "name", buffer, sizeof buffer, & size ) );
    const string Expected = "root_database#1";
    REQUIRE_EQ( Expected.size(), size );
    REQUIRE_EQ( Expected, string( buffer ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI16_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int16_t i;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsI16 ( m_node, "qq", & i ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int16_t i;
    REQUIRE_RC( KMDataNodeReadAttrAsI16 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (int16_t)16, i );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU16_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint16_t u;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsU16 ( m_node, "qq", & u ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU16, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    uint16_t i;
    REQUIRE_RC( KMDataNodeReadAttrAsU16 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (uint16_t)16, i );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI32_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int32_t i;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsI32 ( m_node, "qq", & i ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int32_t i;
    REQUIRE_RC( KMDataNodeReadAttrAsI32 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (int32_t)16, i );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU32_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint32_t u;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsU32 ( m_node, "qq", & u ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU32, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    uint32_t i;
    REQUIRE_RC( KMDataNodeReadAttrAsU32 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (uint32_t)16, i );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    int64_t i;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsI64 ( m_node, "qq", & i ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsI64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    int64_t i;
    REQUIRE_RC( KMDataNodeReadAttrAsI64 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (int64_t)16, i );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    uint64_t u;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsU64 ( m_node, "qq", & u ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsU64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    uint64_t i;
    REQUIRE_RC( KMDataNodeReadAttrAsU64 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (uint64_t)16, i );
}

FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsF64_Fail, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    double f;
    REQUIRE_RC_FAIL( KMDataNodeReadAttrAsF64 ( m_node, "qq", & f ) );
}
FIXTURE_TEST_CASE(KRMDataNode_ReadAttrAsF64, KRMDataNode_Fixture)
{
    Open( ScratchDir + "TestDB", "b16" );
    double i;
    REQUIRE_RC( KMDataNodeReadAttrAsF64 ( m_node, "bits", & i ) );
    REQUIRE_EQ( (double)16, i );
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

FIXTURE_TEST_CASE(KRMDataNode_Addr, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    const void * addr = nullptr;
    size_t size = 0;
    REQUIRE_RC( KMDataNodeAddr ( m_node, & addr, & size ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ListAttr, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    struct KNamelist * names = nullptr;
    REQUIRE_RC( KMDataNodeListAttr ( m_node, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)0, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
}

FIXTURE_TEST_CASE(KRMDataNode_ListChildren, KRMDataNode_Fixture)
{
    Open( "testdb/tbl/SEQUENCE", "col" );
    struct KNamelist * names = nullptr;
    REQUIRE_RC( KMDataNodeListChildren ( m_node, & names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, &count ) );
    REQUIRE_EQ( (uint32_t)1, count );

    REQUIRE_RC( KNamelistRelease ( names ) );
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
