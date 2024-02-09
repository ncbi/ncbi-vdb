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
    REQUIRE_EQ( string("1"), m_node->getValue() );
}
FIXTURE_TEST_CASE(KDBTextMetanode_Make_Value_String, KDBTextMetanode_Fixture)
{
    Setup(R"({"name":"mn","value":"qq"})");
    REQUIRE_RC( m_node -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("qq"), m_node->getValue() );
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

#if 0
//TODO: the rest

// API

class KTextMetanode_ApiFixture
{
public:
    KTextMetanode_ApiFixture()
    {
    }
    ~KTextMetanode_ApiFixture()
    {
        KMetanodeRelease( m_md );
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
            m_md = (const KMetanode*)md;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
            throw;
        }

    }

    KJsonValue * m_json = nullptr;
    const KMetanode * m_md = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KMetanode_AddRelease, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");

    REQUIRE_RC( KMetanodeAddRef( m_md ) );
    REQUIRE_RC( KMetanodeRelease( m_md ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KMetanode_ByteOrder_Null, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");
    REQUIRE_RC_FAIL( KMetanodeByteOrder( m_md, nullptr ) );
}
FIXTURE_TEST_CASE(KMetanode_ByteOrder, KTextMetanode_ApiFixture)
{
    Setup(R"({"name":"md"})");
    bool reversed = true;
    REQUIRE_RC( KMetanodeByteOrder( m_md, &reversed ) );
    REQUIRE( ! reversed );
}

#endif

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
