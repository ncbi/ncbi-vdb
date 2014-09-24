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

#ifndef _h_ktst_unit_test_
#define _h_ktst_unit_test_

#include <ktst/unit_test_suite.hpp>

////////////////////////////////////////////////////////////////////////////////
namespace ncbi { namespace NK {

// a utility template used to pass test character string case names to FixtureTestCase
template<class T>
struct TestCaseTraits
{
    static const char* name;
};

// create an executable test case class with the given fixture.
template <class TCaseTraits, class TFixture>
class FixtureTestCase : public ncbi::NK::TestCase, public TFixture { 
public:
    typedef TCaseTraits Traits;

    // Represents the global test fixture. Cast to the correct data type in the test code
    typedef void AUTO_TEST_CASE_FIXTURE;

public: 
    FixtureTestCase(void* globalFixture) 
    : TestCase(TCaseTraits::name), _globalFixture (static_cast<AUTO_TEST_CASE_FIXTURE*>(globalFixture)) 
    {
    }

protected: 
    AUTO_TEST_CASE_FIXTURE* GET_GLOBAL_FIXTURE(void) const { return _globalFixture; } 
    AUTO_TEST_CASE_FIXTURE* _globalFixture; 

    const TFixture* GET_FIXTURE(void) const { return this; } 
    TFixture* GET_FIXTURE(void) { return this; } 
}; 

// Create an invoker object for a test case with a given fixture.
// The invoker object is static and registers itself with the global test suite during initialization.
// When executed, the invoker will instantiate a fixture and a test case object and execute object's test method 
// on the current thread.
template <class TCase, class TFixture>
class TestCaseInvoker : ncbi::NK::TestInvoker { 
public: 
    TestCaseInvoker() : TestInvoker(TCase::Traits::name) 
    { 
        ncbi::NK::GetTestSuite()->Add(this); 
    } 
private: 
    virtual void Run(void* globalFixture) throw ()
    { 
        try
        { 
            TCase t(globalFixture); 
            t.test_method(); 
            SetErrorCounter(t.GetErrorCounter()); 
        }
        catch (const execution_aborted&)
        {
            SetErrorCounter(1);
        } 
        catch (const std::exception& ex)
        {
            if (!ncbi::NK::TestEnv::lastLocation.empty())
            {
                LOG(ncbi::NK::LogLevel::e_fatal_error, ncbi::NK::TestEnv::lastLocation << ": last checkpoint before exception \"" << ex.what() << "\"\n");
            }        
            SetErrorCounter(1);
        } 
        catch (...) // a non-framework exception escaped
        { 
            if (!ncbi::NK::TestEnv::lastLocation.empty())
            {
                LOG(ncbi::NK::LogLevel::e_fatal_error, ncbi::NK::TestEnv::lastLocation << ": last checkpoint before an unknown exception\n");
            }        
            SetErrorCounter(1);
        } 
    } 
    static TestCaseInvoker instance;
}; 

// Create an out-of-thread invoker object for a test case with a given fixture.
// The invoker object is static and registers itself with the global test suite during initialization.
// When executed, the invoker will instantiate a fixture and a test case object and execute object's test method 
// as a child process (Unix) or on a separate thread(Windows).
template <class TCase, class TFixture, int rc, int timeout>
class ProcessTestCaseInvoker : ncbi::NK::TestInvoker { 
public: 
    ProcessTestCaseInvoker() : TestInvoker(TCase::Traits::name) 
    { 
        ncbi::NK::GetTestSuite()->Add(this); 
    } 
private: 
    virtual void Run(void* globalFixture) throw ()
    { 
        try
        {
            TCase t(globalFixture); 
            TestCase :: TestMethod method = static_cast <TestCase :: TestMethod> ( & TCase::test_method );
            if (ncbi::NK::TestEnv::RunProcessTestCase(t, method, timeout) != rc)
            {
                SetErrorCounter(1);
            }
            else
            {
                SetErrorCounter(t.GetErrorCounter());
            }
        }
        catch (const execution_aborted&)
        {
            SetErrorCounter(1);
        } 
        catch (const std::exception& ex)
        {
            if (!ncbi::NK::TestEnv::lastLocation.empty())
            {
                LOG(ncbi::NK::LogLevel::e_fatal_error, ncbi::NK::TestEnv::lastLocation << ": last checkpoint before exception \"" << ex.what() << "\"\n");
            }        
            SetErrorCounter(1);
        } 
        catch (...) // a non-framework exception escaped
        { 
            if (!ncbi::NK::TestEnv::lastLocation.empty())
            {
                LOG(ncbi::NK::LogLevel::e_fatal_error, ncbi::NK::TestEnv::lastLocation << ": last checkpoint before an unknown exception\n");
            }        
            SetErrorCounter(1);
        } 
    }
}; 

}} // namespace

// macros to be used to instantiate test cases

// user code should not not use this directly. 
#define DEFINE_TEST_CASE(testcase, F, rc, timeout) \
    struct testcase { \
        class dummy{}; \
        typedef ncbi::NK::FixtureTestCase< ::ncbi::NK::TestCaseTraits<dummy>, F> Case;\
        struct Instance : public Case { \
            Instance(void* g) : Case(g) { ncbi::NK::saveLocation(__FILE__,__LINE__); }\
            ~Instance() { ncbi::NK::saveLocation(__FILE__,__LINE__); }\
            void test_method(); } ; \
        class Invoker : public ::ncbi::NK::TestCaseInvoker<Instance, F> {}; \
        class ProcessInvoker : public ::ncbi::NK::ProcessTestCaseInvoker<Instance, F, rc, timeout> {}; \
    };\
    namespace ncbi { namespace NK { template<> const char* TestCaseTraits< ::testcase::dummy > :: name = #testcase; } }

// define an in-thread test case with a fixture
#define FIXTURE_TEST_CASE(testcase, F) \
    DEFINE_TEST_CASE(testcase, F, 0, 0) \
    static testcase::Invoker testcase##Invoker;\
    void testcase::Instance::test_method()

// define an in-thread test case without a fixture
#define TEST_CASE(test_name) FIXTURE_TEST_CASE(test_name, ncbi::NK::Empty)

// define an out-of-thread test case with a fixture, expected exit code, and a timeout (in seconds)
#define PROCESS_FIXTURE_TEST_CASE(testcase, F, rc, timeout) \
    DEFINE_TEST_CASE(testcase, F, rc, timeout) \
    static testcase::ProcessInvoker testcase##Invoker;\
    void testcase::Instance::test_method()

// define an out-of-thread test case with an expected exit code and a timeout (in seconds), no fixture
#define PROCESS_TEST_CASE(test_name, rc, timeout) PROCESS_FIXTURE_TEST_CASE(test_name, ncbi::NK::Empty, rc, timeout)

// fix param type
#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
#define DECLARE_EXTERN_C_ENTRYPOINTS                             \
    extern "C"                                                   \
    {                                                            \
        rc_t CC UsageSummary ( const char *progname )            \
        { return ncbi::NK::TestEnv::UsageSummary ( progname ); } \
                                                                 \
        rc_t CC Usage ( const Args *args )                       \
        { return ncbi::NK::TestEnv::Usage ( args ); }            \
    }
#else
#define DECLARE_EXTERN_C_ENTRYPOINTS
#endif

// define a suite of test cases with a global fixture 
// Note: Typed access to global fixture from the test cases' code is not currently implemented
#define FIXTURE_TEST_SUITE( suite_name, F ) \
DECLARE_EXTERN_C_ENTRYPOINTS \
typedef F AUTO_TEST_CASE_FIXTURE; \
int suite_name(int argc, char* argv[]) { \
    try { \
        ncbi::NK::TestEnv args(argc, argv); \
        if (args.catch_system_errors) { \
            args.set_handlers(); \
        } \
    } catch (...) { return 1; } \
    ncbi::NK::counter_t ec = ncbi::NK::Main<AUTO_TEST_CASE_FIXTURE>(argc, argv, #suite_name); \
    return ec == 0 ? 0 : -ec; /* positive rc represents the signal that killed the process */ \
} 

// define a suite of test cases with a global fixture
// with callback to handle application command line arguments
#define FIXTURE_TEST_SUITE_WITH_ARGS_HANDLER( suite_name, F, Handler ) \
DECLARE_EXTERN_C_ENTRYPOINTS \
typedef F AUTO_TEST_CASE_FIXTURE; \
int suite_name(int argc, char* argv[]) { \
    try { \
        ncbi::NK::TestEnv args(argc, argv, Handler); \
        if (args.catch_system_errors) { \
            args.set_handlers(); \
        } \
    } catch (...) { return 1; } \
    ncbi::NK::counter_t ec = ncbi::NK::Main<AUTO_TEST_CASE_FIXTURE>(argc, argv, #suite_name); \
    return ec == 0 ? 0 : -ec; /* positive rc represents the signal that killed the process */ \
}

// define a suite of test cases with a global fixture; command line parsing, Usage and UsageSummary functions are supplied by the user's code
#define FIXTURE_TEST_SUITE_WITH_USAGE( suite_name, F ) \
typedef F AUTO_TEST_CASE_FIXTURE; \
int suite_name(int argc, char* argv[]) { \
    try { \
        ncbi::NK::TestEnv args(argc, argv); \
        if (args.catch_system_errors) { \
            args.set_handlers(); \
        } \
    } catch (...) { return 1; } \
    ncbi::NK::counter_t ec = ncbi::NK::Main<AUTO_TEST_CASE_FIXTURE>(argc, argv, #suite_name); \
    return ec == 0 ? 0 : -ec; /* positive rc represents the signal that killed the process */ \
} 

#define TEST_SUITE( suite_name ) FIXTURE_TEST_SUITE(suite_name, ncbi::NK::Empty)
#define TEST_SUITE_WITH_USAGE( suite_name ) FIXTURE_TEST_SUITE_WITH_USAGE(suite_name, ncbi::NK::Empty)
#define TEST_SUITE_WITH_ARGS_HANDLER( suite_name, Handler ) FIXTURE_TEST_SUITE_WITH_ARGS_HANDLER(suite_name, ncbi::NK::Empty, Handler)

#endif// _h_ktst_unit_test_
