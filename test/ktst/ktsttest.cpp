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
* Unit tests for Ktst interface
*/

#include <ktst/unit_test.hpp>

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
#include <kapp/args.h>
#endif

#include <csignal>

using namespace std;
using namespace ncbi::NK;

static rc_t argsHandler(int argc, char* argv[]);
TEST_SUITE_WITH_ARGS_HANDLER(KtstTestSuite, argsHandler);

static
rc_t 
RcSuccess()
{
    return 0;
}
static
rc_t 
RcFail()
{
    return 1;
}

TEST_CASE(Parameter)
{
#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
    uint32_t count=1;
    REQUIRE_RC(ArgsParamCount(TestEnv::GetArgs(), &count));
    REQUIRE_EQ(count, 0u);
#endif
}

TEST_CASE(Requires)
{
    REQUIRE(1);
    CHECK(1);

    CHECK_CLOSE(1.0, 2.0, 1.99);
    REQUIRE_CLOSE(1.0, -1.0, 2.01);

    CHECK_EQUAL(1, 1);    
    CHECK_EQ(2.0, 2.0);
    REQUIRE_EQUAL(-1, -1);
    REQUIRE_EQ(-11.0f, -11.0f);
    REQUIRE_NE((const char*)"qq", (const char*)1);

    CHECK_NE(2.0, 2.01);
    REQUIRE_NE(-1.f, -1.1f);

    CHECK_GE(1, 1);
    REQUIRE_GE(1, -1);

    CHECK_GT(1, 0);
    REQUIRE_GT(1, -1);

    CHECK_LE(1, 10);
    REQUIRE_LE(-1, -1);

    CHECK_LT(1.0f, 2.0f);
    REQUIRE_LT(-111.0f, 2.0f);

    CHECK_RC(RcSuccess());
    REQUIRE_RC(RcSuccess());

    CHECK_RC_FAIL(RcFail());
    REQUIRE_RC_FAIL(RcFail());

    CHECK_NULL((int*)0);
    REQUIRE_NULL((char*)0);

    CHECK_NOT_NULL((float*)1);
    REQUIRE_NOT_NULL((double*)10);
}

TEST_CASE(ParentProcess)
{
	REQUIRE(!TestEnv::in_child_process);
}
PROCESS_TEST_CASE(ChildProcessOk, 0, 0)
{
	REQUIRE(TestEnv::in_child_process);
    TEST_MESSAGE("ChildProcessOk: no failures in a child process");
    REQUIRE_EQ(0, 0); // make sure REQUIRE* macros are working
    // just exit without errors
}

PROCESS_TEST_CASE(ChildProcessFail, TestEnv::TEST_CASE_FAILED, 0)
{
    FAIL("ChildProcessFail: failure in a child process");
    // exit(255) means test case threw a C++ exception
}

PROCESS_TEST_CASE(ChildProcessAbort, SIGFPE, 0)
{
    TEST_MESSAGE("ChildProcessAbort: aborting child process");
    raise(SIGFPE);
}

PROCESS_TEST_CASE(ChildProcessTimeout, TestEnv::TEST_CASE_TIMED_OUT, 1)
{   
    TEST_MESSAGE("ChildProcessTimeout: sleeping in the child process");
    TestEnv::Sleep(2);
    TEST_MESSAGE("ChildProcessTimeout: did not time out!!");
    raise(SIGSEGV);
}

static bool argHandlerCalled = false;
static rc_t argsHandler(int argc, char* argv[]) 
{
    argHandlerCalled = true;
    return 0;
}

TEST_CASE(ArgHandlerCalled)
{
    REQUIRE(argHandlerCalled);
}


struct Shared : SharedTest {
    Shared ( TestCase * dad, bool ok = true ) : SharedTest ( dad, "Shared" ) {
        CHECK ( ok );
    }
};

TEST_CASE ( SharedSucceed ) {
    REQUIRE_EQ ( this -> GetErrorCounter (), 0 );
    Shared test ( this );
    REQUIRE_EQ ( this -> GetErrorCounter (), 0 );
}

TEST_CASE ( SharedFailed ) {
    REQUIRE_EQ ( GetErrorCounter (), 0 ); // no error when starting
    Shared test ( this, false );          // make shared SharedTest

    // ErrorCounter is adjusted when SharedTest goes out of scope
    REQUIRE_EQ ( GetErrorCounter (), 0 );

    // compensate SharedTest failure - make this test case succeed
    ErrorCounterAdd ( -1 );
}

TEST_CASE ( SharedFailedAndDetected ) {
    REQUIRE_EQ ( GetErrorCounter (), 0 ); // no error when starting

    {
        Shared test ( this, false );      // make SharedTest fail

        // ErrorCounter is adjusted when SharedTest goes out of scope
        REQUIRE_EQ ( GetErrorCounter (), 0 );
    }
    REQUIRE_EQ ( GetErrorCounter (), 1 );

    // compensate SharedTest failure - make this test case succeed
    ErrorCounterAdd ( -1 );
}

TEST_CASE ( SharedSucceedInBlock ) {
    REQUIRE_EQ ( GetErrorCounter (), 0 ); // no error when starting

    {
        Shared test ( this );             // make SharedTest succeed

        // ErrorCounter is adjusted when SharedTest goes out of scope
        REQUIRE_EQ ( GetErrorCounter (), 0 );
    }
    REQUIRE_EQ ( GetErrorCounter (), 0 );
}


//TODO: test FIXTURE_TEST_CASE, PROCESS_FIXTURE_TEST_CASE
//TODO: test GET_GLOBAL_FIXTURE
//TODO: test REQUIRE_THROW, THROW_ON_RC

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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


const char UsageDefaultName[] = "test-kfg";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=KtstTestSuite(argc, argv);
    return rc;
}

}
