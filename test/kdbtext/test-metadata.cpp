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
* Unit tests for KDBMetadata for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/metadata.hpp"

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextMetadataTestSuite);

class KDBTextMetadata_Fixture
{
public:
    KDBTextMetadata_Fixture()
    {
    }
    ~KDBTextMetadata_Fixture()
    {
        delete m_meta;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_meta = new Metadata( json );
    }

    KJsonValue * m_json = nullptr;
    Metadata * m_meta = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KDBTextMetadata_Make_Empty, KDBTextMetadata_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_meta -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextMetadata_Make, KDBTextMetadata_Fixture)
{
    Setup(R"({"name":"md"})");
    REQUIRE_RC( m_meta -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("md"), m_meta->getName() );
}

//TODO: the rest

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

const char UsageDefaultName[] = "Test_KDBText_Metadata";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextMetadataTestSuite(argc, argv);
    return rc;
}

}
