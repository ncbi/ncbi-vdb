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
* Unit tests for KDBIndex for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/index.hpp"

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextIndexTestSuite);

class KDBTextIndex_Fixture
{
public:
    KDBTextIndex_Fixture()
    {
    }
    ~KDBTextIndex_Fixture()
    {
        delete m_idx;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_idx = new Index( json );
    }

    KJsonValue * m_json = nullptr;
    Index * m_idx = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KDBTextIndex_Make_Empty, KDBTextIndex_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextIndex_Make, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"idx"})");
    REQUIRE_RC( m_idx -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("idx"), m_idx->getName() );
}

// API

class KTextIndex_ApiFixture
{
public:
    KTextIndex_ApiFixture()
    {
    }
    ~KTextIndex_ApiFixture()
    {
        KIndexRelease( m_idx );
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

            Index * idx = new Index( json );
            THROW_ON_RC( idx -> inflate( m_error, sizeof m_error ) );
            m_idx = (const KIndex*)idx;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
            throw;
        }

    }

    KJsonValue * m_json = nullptr;
    const KIndex * m_idx = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KIndex_AddRelease, KTextIndex_ApiFixture)
{
    Setup(R"({"name":"idx"})");

    REQUIRE_RC( KIndexAddRef( m_idx ) );
    REQUIRE_RC( KIndexRelease( m_idx ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KIndex_Locked, KTextIndex_ApiFixture)
{   // always false for this library
    Setup(R"({"name":"idx"})");
    REQUIRE( ! KIndexLocked( m_idx ) );
}

FIXTURE_TEST_CASE(KIndex_Version_Null, KTextIndex_ApiFixture)
{   // always 0 for this library
    Setup(R"({"name":"idx"})");
    REQUIRE_RC_FAIL( KIndexVersion( m_idx, nullptr ) );
}

FIXTURE_TEST_CASE(KIndex_Version, KTextIndex_ApiFixture)
{   // always 0 for this library
    Setup(R"({"name":"idx"})");
    uint32_t version = 1;
    REQUIRE_RC( KIndexVersion( m_idx, & version ) );
    REQUIRE_EQ( (uint32_t)0, version );
}

FIXTURE_TEST_CASE(KIndex_Type_Null, KTextIndex_ApiFixture)
{   // always titText for now
    Setup(R"({"name":"idx"})");
    REQUIRE_RC_FAIL( KIndexType( m_idx, nullptr ) );
}

FIXTURE_TEST_CASE(KIndex_Type, KTextIndex_ApiFixture)
{   // always titText for now
    Setup(R"({"name":"idx"})");
    KIdxType type = 99;
    REQUIRE_RC( KIndexType( m_idx, & type ) );
    REQUIRE_EQ( (int)kitText, (int)type );
}

FIXTURE_TEST_CASE(KIndex_ConsistencyCheck, KTextIndex_ApiFixture)
{   // no op
    Setup(R"({"name":"idx"})");
    int64_t start_id;
    uint64_t id_range;
    uint64_t num_keys;
    uint64_t num_rows;
    uint64_t num_holes;
    REQUIRE_RC( KIndexConsistencyCheck( m_idx, 1, &start_id, &id_range, &num_keys, &num_rows, &num_holes ) );
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

const char UsageDefaultName[] = "Test_KDBText_Index";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextIndexTestSuite(argc, argv);
    return rc;
}

}
