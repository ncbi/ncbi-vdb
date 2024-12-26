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

#include <kapp/vdbapp.h>

#include <ktst/unit_test.hpp>

#include <kapp/main.h>

#include <kproc/procmgr.h>
#include <klib/klib-priv.h>
#include <kns/manager.h>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(VDBAppTestSuite);

TEST_CASE(NotQuitting)
{
    REQUIRE( ! Quitting() );
}

TEST_CASE(NoHangUp)
{
    REQUIRE_RC( Hangup() );
}
TEST_CASE(SignalNoHup_ignored)
{
    REQUIRE_RC( SignalNoHup() );
}

const ver_t AppVersion = 12;
TEST_CASE(ReportInitialized)
{   // main() initialized report module
    ver_t version = 0;
    ReportGetVersion( & version );
    REQUIRE_EQ( AppVersion, version );
}

TEST_CASE(ProcMgrInitialized)
{
    KProcMgr * mgr = nullptr;
    REQUIRE_RC( KProcMgrMakeSingleton( & mgr ) );
    REQUIRE_NOT_NULL( mgr );
    REQUIRE_RC( KProcMgrRelease( mgr ) );
}

TEST_CASE(UserAgentInitialized)
{
    const char * agent = nullptr;
    REQUIRE_RC( KNSManagerGetUserAgent( & agent ) );
    REQUIRE_NOT_NULL( agent );
    //cout<<agent;
    REQUIRE_NE( string::npos, string(agent).find("sra-toolkit Test_VDBAPP") );
}

TEST_CASE(KnsQuittingInitialized)
{
    REQUIRE_NOT_NULL( (void*)KNSManagerGetQuitting( nullptr ) );
}

#if WIN32
TEST_CASE(SignalHup_ignored)
{
    REQUIRE_RC_FAIL( SignalHup() ); // would not return on Unix
}

TEST_CASE(WCharConversion)
{
    wchar_t * wargv[] = {L"wide string 0", L"wide string 1"};
    int wargc = 2;
    VDB::VdbApp app(wargc, wargv);
    REQUIRE_EQ( string("wide string 0"), string(app.GetArgV()[0]) );
    REQUIRE_EQ( string("wide string 1"), string(app.GetArgV()[1]) );
}

TEST_CASE(WCharConversion_BadArgs)
{
    wchar_t* wargv[] = { L"wide string 0", L"wide string 1", NULL };
    int wargc = 20;
    VDB::VdbApp app(wargc, wargv);
    REQUIRE_RC_FAIL( app.getRc() );
}

#endif

#if WINDOWS && UNICODE
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    VDB::VdbApp app( argc, argv, AppVersion );
    return VDBAppTestSuite(argc, app.GetArgV());
}
