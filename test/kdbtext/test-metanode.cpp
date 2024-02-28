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
* Unit tests for KDBMetanode for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/metanode.hpp"
#include "../../libs/kdbtext/path.hpp"

#include <kdb/meta.h>
#include <kdb/kdb-priv.h>
#include <kdb/namelist.h>

#include <klib/rc.h>
#include <klib/json.h>
#include <klib/namelist.h>

#include <arch-impl.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextMetanodeTestSuite);

class KDBTextMetanode_Fixture
{
public:
    KDBTextMetanode_Fixture()
    {
    }
    ~KDBTextMetanode_Fixture()
    {
        delete m_node;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_node = new Metanode( json );
    }

    KJsonValue * m_json = nullptr;
    Metanode * m_node = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KDBTextMetanode_Make_Empty, KDBTextMetanode_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Short, KDBTextMetanode_Fixture)
{   // name reqired; value, attributes, children optional
    Setup(R"({"name":"mn"})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("mn"), m_node->getName() );
}

FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_Bad, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":[]})");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_Null, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":null})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string(), m_node->getValue() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_Numeric, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":1})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (int64_t)1, *(int64_t*)m_node->getValue().data() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_String, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":"qq"})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("qq"), m_node->getValue() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_Boolean, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":true})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("\x01"), m_node->getValue() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_Double, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":3.14E+2})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (double)3.14E+2, *(double*)m_node->getValue().data() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_SizedNumeric8, KDBTextMetanode_Fixture)
{
    Setup(R"({ "name":"mn","value":{"size":1,"int":255} })");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (size_t)1, m_node->getValue().size() );
    REQUIRE_EQ( (uint8_t)255, *(uint8_t*)m_node->getValue().data() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_SizedNumeric16, KDBTextMetanode_Fixture)
{
    Setup(R"({ "name":"mn","value":{"size":2,"int":321} })");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (size_t)2, m_node->getValue().size() );
    REQUIRE_EQ( (uint16_t)321, *(uint16_t*)m_node->getValue().data() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_SizedNumeric32, KDBTextMetanode_Fixture)
{
    Setup(R"({ "name":"mn","value":{"size":4,"int":321} })");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (size_t)4, m_node->getValue().size() );
    REQUIRE_EQ( (uint32_t)321, *(uint32_t*)m_node->getValue().data() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_SizedNumeric64, KDBTextMetanode_Fixture)
{
    Setup(R"({ "name":"mn","value":{"size":8,"int":321} })");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (size_t)8, m_node->getValue().size() );
    REQUIRE_EQ( (uint64_t)321, *(uint64_t*)m_node->getValue().data() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_SizedNumeric_BadSize, KDBTextMetanode_Fixture)
{
    Setup(R"({ "name":"mn","value":{"size":3,"int":321} })");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextMetanode_Make_Attributes_Bad, KDBTextMetanode_Fixture)
{   // has to be a json object {...}
    Setup(R"({"name":"mn","attributes":""})");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Attributes_AttrBad, KDBTextMetanode_Fixture)
{   // attributes have to be values
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":[]}})");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Attributes, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    auto a = m_node->getAttributes();
    REQUIRE_EQ( (size_t)2, a.size() );
    REQUIRE_EQ( string("v1"), a["a1"] );
    REQUIRE_EQ( string("v2"), a["a2"] );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Attributes_Numeric, KDBTextMetanode_Fixture)
{   // numeric attribute values are always stored as strings
    Setup(R"({"name":"mn","attributes":{"a1":1}})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    auto const & a = m_node->getAttributes().find( "a1" );
    REQUIRE_EQ( string("1"), a->second );
}

FIXTURE_TEST_CASE(KDBTextMetanode_Make_Children_Bad, KDBTextMetanode_Fixture)
{   // has to be an array
    Setup(R"({"name":"mn","children":""})");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Children_ChildBad, KDBTextMetanode_Fixture)
{   // children have to be objects
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, [] ]})");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Children_ChildIllFormed, KDBTextMetanode_Fixture)
{   // children have to be well-formed metanodes
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"shname":"ch"}] })");
    REQUIRE_RC_FAIL( m_node -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Children, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    auto c = m_node->getChildren();
    REQUIRE_EQ( (size_t)2, c.size() );
    REQUIRE_EQ( string("ch1"), c[0] -> getName() );
    REQUIRE_EQ( string("ch2"), c[1] -> getName() );
}

FIXTURE_TEST_CASE(KDBTextMetanode_GetNode_Self, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    { Path p (""); REQUIRE_EQ( (const Metanode*)m_node, m_node->getNode( p ) ); }
    { Path p ("."); REQUIRE_EQ( (const Metanode*)m_node, m_node->getNode( p ) ); }
}

FIXTURE_TEST_CASE(KDBTextMetanode_GetNode_Child, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    {
        Path p ("ch1");
        const Metanode * n = m_node->getNode( p );
        REQUIRE_NOT_NULL( n );
        REQUIRE_EQ( string("ch1"), n->getName() );
    }
    {
        Path p ("./ch2");
        const Metanode * n = m_node->getNode( p );
        REQUIRE_NOT_NULL( n );
        REQUIRE_EQ( string("ch2"), n->getName() );
    }
}

// API

class KTextMetanode_ApiFixture
{
public:
    KTextMetanode_ApiFixture()
    {
    }
    ~KTextMetanode_ApiFixture()
    {
        KMDataNodeRelease( m_node );
        KJsonValueWhack( m_json );
    }
    void Setup( const char * input )
    {
        try
        {
            THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
            THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

            const KJsonObject * json = KJsonValueToObject ( m_json );
            THROW_ON_FALSE( json != nullptr );

            Metanode * md = new Metanode( json );
            THROW_ON_RC( md -> inflate( m_error, sizeof m_error ) );
            m_node = (const KMDataNode*)md;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
            throw;
        }
    }

    KJsonValue * m_json = nullptr;
    const KMDataNode * m_node = nullptr;
    char m_error[1024] = {0};

    char m_buffer[1024];
    size_t m_num_read = 0;
    size_t m_remaining = 0;
};

FIXTURE_TEST_CASE(KMetanode_AddRelease, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");

    REQUIRE_RC( KMDataNodeAddRef( m_node ) );
    REQUIRE_RC( KMDataNodeRelease( m_node ) );
    // use valgrind to find any leaks
}


FIXTURE_TEST_CASE(KMetanode_ByteOrder_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");
    REQUIRE_RC_FAIL( KMDataNodeByteOrder( m_node, nullptr ) );
}
FIXTURE_TEST_CASE(KMetanode_ByteOrder, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");
    bool reversed = true;
    REQUIRE_RC( KMDataNodeByteOrder( m_node, &reversed ) );
    REQUIRE( ! reversed );
}


FIXTURE_TEST_CASE(KMetanode_Read_numRead_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");
    REQUIRE_RC_FAIL( KMDataNodeRead ( m_node, 0, m_buffer, sizeof m_buffer, nullptr, &m_remaining ) );
}
FIXTURE_TEST_CASE(KMetanode_Read_buffer_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");
    REQUIRE_RC_FAIL( KMDataNodeRead ( m_node, 0, nullptr, 1, &m_num_read, &m_remaining ) );
}

FIXTURE_TEST_CASE(KMetanode_Read_text, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":"qwerty"})");
    REQUIRE_RC( KMDataNodeRead ( m_node, 0, m_buffer, sizeof m_buffer, &m_num_read, &m_remaining ) );
    REQUIRE_EQ( string("qwerty"), string(m_buffer, m_num_read) );
    REQUIRE_EQ( (size_t)0, m_remaining );
}
FIXTURE_TEST_CASE(KMetanode_Read_text_offset, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":"qwerty"})");
    REQUIRE_RC( KMDataNodeRead ( m_node, 2, m_buffer, sizeof m_buffer, &m_num_read, &m_remaining ) );
    REQUIRE_EQ( string("erty"), string(m_buffer, m_num_read) );
    REQUIRE_EQ( (size_t)0, m_remaining );
}
FIXTURE_TEST_CASE(KMetanode_Read_text_offset_tooLarge, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":"qwerty"})");
    REQUIRE_RC( KMDataNodeRead ( m_node, 22, m_buffer, sizeof m_buffer, &m_num_read, &m_remaining ) );
    REQUIRE_EQ( string(), string(m_buffer, m_num_read) );
    REQUIRE_EQ( (size_t)0, m_remaining );
}
FIXTURE_TEST_CASE(KMetanode_Read_text_buffer_short, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":"qwerty"})");
    REQUIRE_RC( KMDataNodeRead ( m_node, 0, m_buffer, 4, &m_num_read, &m_remaining ) );
    REQUIRE_EQ( string("qwer"), string(m_buffer, m_num_read) );
    REQUIRE_EQ( (size_t)2, m_remaining );
}
FIXTURE_TEST_CASE(KMetanode_Read_text_buffer_short_with_offset, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":"qwerty"})");
    REQUIRE_RC( KMDataNodeRead ( m_node, 2, m_buffer, 3, &m_num_read, &m_remaining ) );
    REQUIRE_EQ( string("ert"), string(m_buffer, m_num_read) );
    REQUIRE_EQ( (size_t)1, m_remaining );
}

FIXTURE_TEST_CASE(KMetanode_Read_bool, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":true})");
    bool v = false;
    REQUIRE_RC( KMDataNodeReadB8 ( m_node, &v ) );
    REQUIRE( v );
    REQUIRE_RC_FAIL( KMDataNodeReadB16 ( m_node, &v ) ); // trying to read too much
}

FIXTURE_TEST_CASE(KMetanode_ReadAs_int, KTextMetanode_ApiFixture)
{   // is bit size is not specified, represented with a 64 bit int in native byte order
    Setup(R"({"name":"md","value":1})");
    int64_t v64 = 0;
    REQUIRE_RC( KMDataNodeReadB64 ( m_node, &v64 ) );
    REQUIRE_EQ( (int64_t)1, v64 );

    REQUIRE_RC_FAIL( KMDataNodeReadB8 ( m_node, &v64 ) ); // trying to read too little
    uint128_t v128;
    REQUIRE_RC_FAIL( KMDataNodeReadB128 ( m_node, &v128 ) );// trying to read too much

    v64=0;
    REQUIRE_RC( KMDataNodeReadAsI64 ( m_node, &v64 ) );
    REQUIRE_EQ( (int64_t)1, v64 );

    uint16_t v16 = 0;
    REQUIRE_RC_FAIL( KMDataNodeReadAsU16 ( m_node, &v16 ) );

    uint64_t vu64 = 0;
    REQUIRE_RC( KMDataNodeReadAsU64 ( m_node, &vu64 ) );
    REQUIRE_EQ( (uint64_t)1, vu64 );
}

FIXTURE_TEST_CASE(KMetanode_Read_SizedInt, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":{"size":4,"int":1}})");

    uint8_t u8 = 0;
    REQUIRE_RC_FAIL( KMDataNodeReadB8 ( m_node, &u8 ) );

    uint16_t u16 = 0;
    REQUIRE_RC_FAIL( KMDataNodeReadB16 ( m_node, &u16 ) );

    uint32_t u32 = 0;
    REQUIRE_RC( KMDataNodeReadB32 ( m_node, &u32 ) );
    REQUIRE_EQ( (uint32_t)1, u32 );

    uint64_t u64 = 0;
    REQUIRE_RC_FAIL( KMDataNodeReadB64 ( m_node, &u64 ) );

    uint128_t u128 = {0,0};
    REQUIRE_RC_FAIL( KMDataNodeReadB8 ( m_node, &u128 ) );
}

FIXTURE_TEST_CASE(KMetanode_ReadAs_SizedInt, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":{"size":4,"int":1}})");

    int16_t i16 = 0;
    REQUIRE_RC_FAIL( KMDataNodeReadAsI16 ( m_node, &i16 ) );

    uint16_t u16 = 0;
    REQUIRE_RC_FAIL( KMDataNodeReadAsU16 ( m_node, &u16 ) );

    int32_t i32 = 0;
    REQUIRE_RC( KMDataNodeReadAsI32 ( m_node, &i32 ) );
    REQUIRE_EQ( (int32_t)1, i32 );

    uint32_t u32 = 0;
    REQUIRE_RC( KMDataNodeReadAsU32 ( m_node, &u32 ) );
    REQUIRE_EQ( (uint32_t)1, u32 );

    int64_t i64 = 0;
    REQUIRE_RC( KMDataNodeReadAsI64 ( m_node, &i64 ) );
    REQUIRE_EQ( (int64_t)1, i64 );

    uint64_t u64 = 0;
    REQUIRE_RC( KMDataNodeReadAsU64 ( m_node, &u64 ) );
    REQUIRE_EQ( (uint64_t)1, u64 );

    double f64 = 0;
    REQUIRE_RC( KMDataNodeReadAsF64 ( m_node, &f64 ) );
    uint64_t One = 1;
    float fl = *(float*)&One; // internally, 4-byte values are converted into float and then to double (see KMDataNodeReadAsF64 in kdb/metanode-base.c)
    REQUIRE_EQ( (double)fl, f64 );
}

FIXTURE_TEST_CASE(KMetanode_OpenRead_NodeNull, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    REQUIRE_RC_FAIL( KMDataNodeOpenNodeRead ( m_node, nullptr, "" ) );
}

FIXTURE_TEST_CASE(KMetanode_OpenRead_Node_NotFound, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    const KMDataNode * node = (const KMDataNode *)1;
    REQUIRE_RC_FAIL( KMDataNodeOpenNodeRead ( m_node, &node, "/notthere" ) );
    REQUIRE_NULL( node );
}

FIXTURE_TEST_CASE(KMetanode_OpenRead_Node_Self_NULL, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    const KMDataNode * node = nullptr;
    REQUIRE_RC( KMDataNodeOpenNodeRead ( m_node, &node, nullptr ) );
    REQUIRE_EQ( m_node, node );
    REQUIRE_RC( KMDataNodeRelease( node ) );
}
FIXTURE_TEST_CASE(KMetanode_OpenRead_Node_Self_Dot, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2"} ]})");
    const KMDataNode * node = nullptr;
    REQUIRE_RC( KMDataNodeOpenNodeRead ( m_node, &node, "." ) );
    REQUIRE_EQ( m_node, node );
    REQUIRE_RC( KMDataNodeRelease( node ) );
}

FIXTURE_TEST_CASE(KMetanode_OpenRead_Node_Self_Child, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2","value":2} ]})");
    const KMDataNode * node = nullptr;
    REQUIRE_RC( KMDataNodeOpenNodeRead ( m_node, &node, "./ch2" ) );

    uint64_t vu64 = 0;
    REQUIRE_RC( KMDataNodeReadAsU64 ( node, &vu64 ) );
    REQUIRE_EQ( (uint64_t)2, vu64 );

    REQUIRE_RC( KMDataNodeRelease( node ) );
}

FIXTURE_TEST_CASE(KMetanode_ReadAttr_Size_NULL, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "a1", m_buffer, sizeof( m_buffer ), nullptr ) );
}
FIXTURE_TEST_CASE(KMetanode_ReadAttr_Name_NULL, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    size_t size;
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, nullptr, m_buffer, sizeof( m_buffer ), &size ) );
}
FIXTURE_TEST_CASE(KMetanode_ReadAttr_Name_Empty, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    size_t size;
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "", m_buffer, sizeof( m_buffer ), &size ) );
}
FIXTURE_TEST_CASE(KMetanode_ReadAttr_Buffer_NULL, KTextMetanode_ApiFixture)
{   // error if bsize > 0
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    size_t size;
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "a1", nullptr, sizeof( m_buffer ), &size ) );
}
FIXTURE_TEST_CASE(KMetanode_ReadAttr_NotFound, KTextMetanode_ApiFixture)
{   // error if bsize > 0
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    size_t size = 1;
    m_buffer[0]='a';
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "a3", m_buffer, sizeof( m_buffer ), &size ) );
    REQUIRE_EQ( (size_t)0, size );
    REQUIRE_EQ( (char)0, m_buffer[0] );
}

FIXTURE_TEST_CASE(KMetanode_ReadAttr, KTextMetanode_ApiFixture)
{   // error if bsize > 0
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    size_t size = 1;
    m_buffer[0]='a';
    REQUIRE_RC( KMDataNodeReadAttr ( m_node, "a2", m_buffer, sizeof( m_buffer ), &size ) );
    REQUIRE_EQ( string("v2"), string( m_buffer, size ) );
}
FIXTURE_TEST_CASE(KMetanode_ReadAttr_BufShort, KTextMetanode_ApiFixture)
{   // error if bsize > 0
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    size_t size = 1;
    m_buffer[0]='a';
    REQUIRE_RC_FAIL( KMDataNodeReadAttr ( m_node, "a2", m_buffer, 1, &size ) );
    REQUIRE_EQ( (size_t)2, size );
}

FIXTURE_TEST_CASE(KMetanode_ReadAttrAs_SizedInt, KTextMetanode_ApiFixture)
{   // numeric attribute values are always stored as strings and converted on read as needed
    Setup(R"({"name":"md","attributes":{"a1":1}})");

    int16_t i16 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsI16 ( m_node, "a1", &i16 ) );
    REQUIRE_EQ( (int16_t)1, i16 );

    uint16_t u16 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsU16 ( m_node, "a1", &u16 ) );
    REQUIRE_EQ( (uint16_t)1, u16 );

    int32_t i32 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsI32 ( m_node, "a1", &i32 ) );
    REQUIRE_EQ( (int32_t)1, i32 );

    uint32_t u32 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsU32 ( m_node, "a1", &u32 ) );
    REQUIRE_EQ( (uint32_t)1, u32 );

    int64_t i64 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsI64 ( m_node, "a1", &i64 ) );
    REQUIRE_EQ( (int64_t)1, i64 );

    uint64_t u64 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsU64 ( m_node, "a1", &u64 ) );
    REQUIRE_EQ( (uint64_t)1, u64 );

    double f64 = 0;
    REQUIRE_RC( KMDataNodeReadAttrAsF64 ( m_node, "a1", &f64 ) );
    REQUIRE_EQ( (double)1, f64 );
}

FIXTURE_TEST_CASE(KMetanode_Compare_Param_NULL, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","attributes":{"a1":1}})");
    bool equal;
    REQUIRE_RC_FAIL( KMDataNodeCompare( m_node, nullptr, &equal ) );
    REQUIRE_RC_FAIL( KMDataNodeCompare( m_node, m_node, nullptr ) );
}

FIXTURE_TEST_CASE(KMetanode_Compare_Different, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","attributes":{"a1":1}})");
    KTextMetanode_ApiFixture other;
    other.Setup(R"({"name":"md","attributes":{"a1":2}})");
    bool equal = true;
    REQUIRE_RC( KMDataNodeCompare( m_node, other.m_node, &equal ) );
    REQUIRE( ! equal );
}
FIXTURE_TEST_CASE(KMetanode_Compare_Same, KTextMetanode_ApiFixture)
{
          Setup(R"({"name":"md","attributes":{"a1":1},"children":[{"name":"child"}]})");
    KTextMetanode_ApiFixture other;
    other.Setup(R"({"name":"md","attributes":{"a1":1},"children":[{"name":"child"}]})");
    bool equal = false;
    REQUIRE_RC( KMDataNodeCompare( m_node, other.m_node, &equal ) );
    REQUIRE( equal );
}

FIXTURE_TEST_CASE(KMetanode_Addr_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","attributes":{"a1":1}})");
    size_t size;
    REQUIRE_RC_FAIL( KMDataNodeAddr( m_node, nullptr, &size ) );
}

FIXTURE_TEST_CASE(KMetanode_Addr, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":{"size":4,"int":1}})");
    const void * addr;
    size_t size;
    REQUIRE_RC( KMDataNodeAddr( m_node, &addr, &size ) );
    REQUIRE_EQ( (size_t)4, size );
    REQUIRE_EQ( (uint32_t)1, *(uint32_t*)addr );
}
FIXTURE_TEST_CASE(KMetanode_Addr_SizeNull, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md","value":{"size":4,"int":1}})");
    const void * addr;
    REQUIRE_RC( KMDataNodeAddr( m_node, &addr, nullptr ) );
    REQUIRE_EQ( (uint32_t)1, *(uint32_t*)addr );
}

FIXTURE_TEST_CASE(KMetanode_ListAttr_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    REQUIRE_RC_FAIL( KMDataNodeListAttr( m_node, nullptr ) );
}

FIXTURE_TEST_CASE(KMetanode_ListAttr, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","attributes":{"a1":"v1","a2":"v2"}})");
    KNamelist * names;
    REQUIRE_RC( KMDataNodeListAttr( m_node, &names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)2, count );
    REQUIRE( KNamelistContains( names, "a1" ) );
    REQUIRE( KNamelistContains( names, "a2" ) );
    KNamelistRelease( names );
}

FIXTURE_TEST_CASE(KMetanode_ListChildren_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2","value":2} ]})");
    REQUIRE_RC_FAIL( KMDataNodeListChildren( m_node, nullptr ) );
}

FIXTURE_TEST_CASE(KMetanode_ListChildren, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"mn","children":[ {"name":"ch1"}, {"name":"ch2","value":2} ]})");
    KNamelist * names;
    REQUIRE_RC( KMDataNodeListChildren( m_node, &names ) );

    uint32_t count = 0;
    REQUIRE_RC( KNamelistCount ( names, & count ) );
    REQUIRE_EQ( (uint32_t)2, count );
    REQUIRE( KNamelistContains( names, "ch1" ) );
    REQUIRE( KNamelistContains( names, "ch2" ) );
    KNamelistRelease( names );
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

const char UsageDefaultName[] = "Test_KDBText_Metanode";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextMetanodeTestSuite(argc, argv);
    return rc;
}

}
