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

#include <kdb/meta.h>

#include <klib/rc.h>
#include <klib/json.h>
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
    REQUIRE_EQ( string("ch1"), c[0].getName() );
    REQUIRE_EQ( string("ch2"), c[1].getName() );
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
FIXTURE_TEST_CASE(KMetanode_Read_int, KTextMetanode_ApiFixture)
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
