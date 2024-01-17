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
* Unit tests for KDBColumn for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/columnblob.hpp"

using namespace std;
using namespace KDBText;

TEST_SUITE(KTextColumnBlobTestSuite);

const char * TestColumn = R"({"name":"col",
            "type":"ascii",
            "data":
                [
                    {"row":1,"value":"AGCT"},
                    {"row":2,"value":"AGCT"}
                ]})";

class KTextColumnBlob_Fixture
{
public:
    KTextColumnBlob_Fixture()
    {
    }
    ~KTextColumnBlob_Fixture()
    {
        delete m_blob;
    }

    void Setup( const string & data = "abcdef" )
    {
        m_blob = new ColumnBlob( data.c_str(), data.size(), nullptr, 1, 1 );
    }

    ColumnBlob * m_blob = nullptr;
};

FIXTURE_TEST_CASE(KTextColumnBlob_Make_Empty, KTextColumnBlob_Fixture)
{
    Setup();
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

const char UsageDefaultName[] = "Test_KDBText_Column";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KTextColumnBlobTestSuite(argc, argv);
    return rc;
}

}
