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

#ifndef NCBI_NK_UNIT_TEST__SUITE_HPP
#define NCBI_NK_UNIT_TEST__SUITE_HPP

// turn on INT64_C, UINT64_C etc.
#define __STDC_CONSTANT_MACROS
#include <klib/defs.h>
#undef memcpy

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <chrono>

// it's generally a bad idea to make the test suite rely upon code under test
#define ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED 0

////////////////////////////////////////////////////////////////////////////////
// these macros are available outside of test cases' code

#define LOG(log_level, msg) \
    (log_level >= ncbi::NK::TestEnv::verbosity ? (std::cerr << msg) : std::cerr)

/*#define TESTMESSAGE(M) \
{ ncbi::NK::saveLocation(__FILE__,__LINE__); \
    LOG(ncbi::NK::LogLevel::e_message, M); }*/

#define TEST_MESSAGE(M) \
{ ncbi::NK::saveLocation(__FILE__,__LINE__); \
    LOG(ncbi::NK::LogLevel::e_message, M << std::endl); }

#define TEST_CHECKPOINT(M) TEST_MESSAGE(M)

// report failure from a fixture
#define FAIL( msg_ )     \
      ncbi::NK::_REPORT_CRITICAL_ERROR_( (msg_), __FILE__, __LINE__, true )

#define GET_TEST_SUITE ncbi::NK::GetTestSuite

#define REQUIRE_THROW(code) \
    do {   \
        bool threw = false; \
        try { code; } catch(...) { threw = true; } \
        if (!threw) FAIL("expected exception not thrown"); \
    } while (0)

#define THROW_ON_RC(call) \
    do { \
        if ( ( rc_t ) ( call ) != ( rc_t ) 0 ) \
            throw std::logic_error ( std::string ( __func__ ) + ": " + #call + " failed" ); \
    } while (0)

#define THROW_ON_FALSE(call) \
    do { \
        if ( ! ( call ) ) \
            throw std::logic_error ( std::string ( __func__ ) + ": " + #call + " == false" ); \
    } while (0)

////////////////////////////////////////////////////////////////////////////////

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
struct Args;
#endif

namespace ncbi { namespace NK {

typedef int counter_t;

class Empty {};

class execution_aborted {};

class LogLevel {
public:
    enum E {
        e_undefined,
        e_all,
        e_test_suite,
        e_message,
        e_warning,
        e_error,
        e_fatal_error,
        e_nothing
    };
};

extern void saveLocation(const char* file, int line);
extern void _REPORT_CRITICAL_ERROR_(const std::string& msg, const char* file, int line, bool is_msg);

template<class T> const T abs(const T& a) { return a >= 0 ? a : -a; }

class TestCase;

class TestEnv {
public:
    typedef rc_t ArgsHandler(int argc, char* argv[]);

    TestEnv(int argc, char* argv[], ArgsHandler *argsHandler = NULL);
    ~TestEnv(void);

    static void set_handlers(void);

    static std::string lastLocation;
    static LogLevel::E verbosity;
    static bool verbositySet;
    bool catch_system_errors;

    static int RunProcessTestCase(TestCase&, void(TestCase::*)(), int);

    // Sleep functions return false if sleep was interrupted
    static bool Sleep(unsigned int seconds);
    static bool SleepMs(unsigned int milliseconds);

    static const int TEST_CASE_TIMED_OUT=14;
    static const int TEST_CASE_FAILED=255;

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
    static struct Args* GetArgs() { return args; }
#endif

    static rc_t UsageSummary(const char* progname);
#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
    static rc_t Usage(const Args* args);
#else
    static rc_t Usage(const char *progname);
#endif

    static bool in_child_process;
    static std::string GetPidString();

    static std::string FormatLocation(const std::string& p_file, uint64_t p_line);

    static void SetVerbosity(LogLevel::E v)
    {   verbosity = v; verbositySet = true; }

private:
    static void TermHandler();

    static void SigHandler(int sig);

    rc_t process_args(int argc, char* argv[], ArgsHandler* argsHandler);

    int argc2;
    char** argv2;

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
    static struct Args* args;
#endif
};


class TestCase {
    void Init(const std::string& name);

public:
    typedef void ( TestCase ::* TestMethod ) ();

protected:
    TestCase(const std::string &name) { Init(name); }

public:
    // explicit destruction, to be used before calling exit() in out-of-process test runner
    virtual void clear() {}

public:
    ncbi::NK::counter_t GetErrorCounter(void) { return _ec; }
    const char * GetName(void) const { return _name . c_str (); }
    void ErrorCounterAdd(ncbi::NK::counter_t ec) { _ec += ec; }

protected:
    void report_error(const char* msg, const char* file, int line, bool is_msg = false, bool isCritical = false);

    void report_passed(const char* msg, const char* file, int line);

    template<class T1>
    void report_error2(const char* e1, const char* e2, T1 t1, T1 t2,
        const char* file, int line, const char* eq, const char* ne,
        bool isCritical = false)
    {
        ncbi::NK::saveLocation(file, line);
        ++_ec;
        LOG(LogLevel::e_error, TestEnv::FormatLocation(file, line));
        if (isCritical) {
            LOG(LogLevel::e_error, "fatal ");
        }
        LOG(LogLevel::e_error, "error in \"" << _name << "\": ");
        if (isCritical) {
            LOG(LogLevel::e_error, "critical ");
        }
        LOG(LogLevel::e_error, "check " << e1 << " " << eq << " " << e2
            << " failed [" << t1 << " " << ne << " " << t2 << "]");
        LOG(LogLevel::e_error, std::endl);
        if (isCritical)
        { throw ncbi::NK::execution_aborted(); }
    }
    // pointers reported as ints (otherwise << may crash if given an invalid pointer)
    template<class T1>
    void report_error2(const char* e1, const char* e2, const T1* t1, const T1* t2,
        const char* file, int line, const char* eq, const char* ne,
        bool isCritical = false)
    {
        report_error2(e1, e2, (uint64_t)t1, (uint64_t)t2,file, line, eq, ne,isCritical);
    }
    template<class T1>
    void report_error2(const char* e1, const char* e2, T1* t1, T1* t2,
        const char* file, int line, const char* eq, const char* ne,
        bool isCritical = false)
    {
        report_error2(e1, e2, (uint64_t)t1, (uint64_t)t2,file, line, eq, ne,isCritical);
    }


    template<class T1, class T2>
    void report_passed2(const char* e1, const char* e2,
        const T1& t1, const T2& t2,
        const char* file, int line, const char* eq, const char* ne)
    {
        ncbi::NK::saveLocation(file, line);
        LOG(LogLevel::e_all, file << "(" << line << "): info: "
            "check " << e1 << " " << eq << " " << e2 << " passed" << std::endl);
    }

    template<class T1, class T2, class T3>
    void report_passed_close(const char* e1, const char* e2,
        const T1& t1, const T2& t2, const T3& tolerance,
        const char* file, int line)
    {
        ncbi::NK::saveLocation(file, line);
        LOG(LogLevel::e_all, file << "(" << line << "): "
            "info: difference between "
            << e1 << "{" << t1 << "} and " << e2 << "{" << t2 << "} "
            "doesn't exceed " << tolerance << std::endl);
    }

    template<class T1, class T2, class T3, class T4>
    void report_error_close(const char* e1, const char* e2,
        const T1& t1, const T2& t2, const T3& tolerance, const T4& diff,
        const char* file, int line, bool isCritical = false)
    {
        ncbi::NK::saveLocation(file, line);
        ++_ec;
        LOG(LogLevel::e_error, file << "(" << line << "): ");
        if (isCritical) {
            LOG(LogLevel::e_error, "fatal ");
        }
        LOG(LogLevel::e_error, "error in \"" << _name << "\": "
            "difference{" << diff << "} between "
            << e1 << "{" << t1 << "} and " << e2 << "{" << t2 << "} "
            "exceeds " << tolerance << std::endl);
        if (isCritical)
        { throw ncbi::NK::execution_aborted(); }
    }

    void _REPORT_CRITICAL_ERROR_(const std::string& msg, const char* file, int line, bool is_msg = false);

#define CHECK(exp)       \
  ( (exp)                      \
      ? report_passed((#exp), __FILE__, __LINE__) \
      : report_error ((#exp), __FILE__, __LINE__) )

#define REPORT_ERROR(exp)       \
        report_error (exp, __FILE__, __LINE__)

/* TODO #define CHECK_MESSAGE(exp, M)       \
  std::ostringstream s, s << M, \
  ( (exp)                      \
      ? report_passed(s.str(),__FILE__,__LINE__) \
      : report_error (s.str(),__FILE__,__LINE__) )*/

#define REQUIRE(exp)     \
  ( (exp)                      \
      ? report_passed        ((#exp), __FILE__, __LINE__) \
      : _REPORT_CRITICAL_ERROR_((#exp), __FILE__, __LINE__) )

#define CHECK_CLOSE(left, right, tolerance) \
( (ncbi::NK::abs((left) - (right)) < (tolerance)) \
?report_passed_close(#left,#right,(left),(right),(tolerance),__FILE__,__LINE__)\
: report_error_close(#left,#right,(left),(right),(tolerance), \
    ncbi::NK::abs((left) - (right)), __FILE__, __LINE__) )

#define REQUIRE_CLOSE(left, right, tolerance) \
( (ncbi::NK::abs((left) - (right)) < (tolerance)) \
?report_passed_close(#left,#right,(left),(right),(tolerance),__FILE__,__LINE__)\
: report_error_close(#left,#right,(left),(right),(tolerance), \
    ncbi::NK::abs((left) - (right)), __FILE__, __LINE__, true) )

    template<class T>
    void AssertEqual(const T& e1, const T& e2, const char* e1str, const char* e2str, const char* file, unsigned int line, bool critical=false)
    {
        if (e1 == e2)
        {
            report_passed2(e1str, e2str, e1, e2, file, line, "==", "!=");
        }
        else
        {
            report_error2 (e1str, e2str, e1, e2, file, line, "==", "!=", critical);
        }
    }
#define CHECK_EQUAL(e1, e2)     AssertEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, false)
#define CHECK_EQ(e1, e2)        CHECK_EQUAL(e1,e2)
#define REQUIRE_EQUAL(e1, e2)   AssertEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, true)
#define REQUIRE_EQ(e1, e2)      REQUIRE_EQUAL(e1,e2)

    template<class T>
    void AssertNotEqual(const T& e1, const T& e2, const char* e1str, const char* e2str, const char* file, unsigned int line, bool critical=false)
    {
        if (e1 != e2)
        {
            report_passed2(e1str, e2str, e1, e2, file, line, "!=", "==");
        }
        else
        {
            report_error2 (e1str, e2str, e1, e2, file, line, "!=", "==", critical);
        }
    }
#define CHECK_NE(e1, e2)     AssertNotEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, false)
#define REQUIRE_NE(e1, e2)   AssertNotEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, true)

    template<class T>
    void AssertGreaterOrEqual(const T& e1, const T& e2, const char* e1str, const char* e2str, const char* file, unsigned int line, bool critical=false)
    {
        if (e1 >= e2)
        {
            report_passed2(e1str, e2str, e1, e2, file, line, ">=", "<");
        }
        else
        {
            report_error2 (e1str, e2str, e1, e2, file, line, ">=", "<", critical);
        }
    }
#define CHECK_GE(e1, e2)    AssertGreaterOrEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, false)
#define REQUIRE_GE(e1, e2)  AssertGreaterOrEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, true)

    template<class T>
    void AssertGreater(const T& e1, const T& e2, const char* e1str, const char* e2str, const char* file, unsigned int line, bool critical=false)
    {
        if (e1 > e2)
        {
            report_passed2(e1str, e2str, e1, e2, file, line, ">", "<=");
        }
        else
        {
            report_error2 (e1str, e2str, e1, e2, file, line, ">", "<=", critical);
        }
    }
#define CHECK_GT(e1, e2)    AssertGreater((e1),(e2),#e1,#e2,__FILE__,__LINE__, false)
#define REQUIRE_GT(e1, e2)  AssertGreater((e1),(e2),#e1,#e2,__FILE__,__LINE__, true)

    template<class T>
    void AssertLessOrEqual(const T& e1, const T& e2, const char* e1str, const char* e2str, const char* file, unsigned int line, bool critical=false)
    {
        if (e1 <= e2)
        {
            report_passed2(e1str, e2str, e1, e2, file, line, "<=", ">");
        }
        else
        {
            report_error2 (e1str, e2str, e1, e2, file, line, "<=", ">", critical);
        }
    }
#define CHECK_LE(e1, e2)    AssertLessOrEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, false)
#define REQUIRE_LE(e1, e2)  AssertLessOrEqual((e1),(e2),#e1,#e2,__FILE__,__LINE__, true)

    template<class T>
    void AssertLess(const T& e1, const T& e2, const char* e1str, const char* e2str, const char* file, unsigned int line, bool critical=false)
    {
        if (e1 < e2)
        {
            report_passed2(e1str, e2str, e1, e2, file, line, "<", ">=");
        }
        else
        {
            report_error2 (e1str, e2str, e1, e2, file, line, "<", ">=", critical);
        }
    }
#define CHECK_LT(e1, e2)    AssertLess((e1),(e2),#e1,#e2,__FILE__,__LINE__, false)
#define REQUIRE_LT(e1, e2)  AssertLess((e1),(e2),#e1,#e2,__FILE__,__LINE__, true)

    void report_rc(rc_t rc, const char* callStr, const char* file, int line, int successExpected, bool isCritical = false);

#define CHECK_RC(exp)   report_rc((exp), #exp, __FILE__, __LINE__, true, false)
#define REQUIRE_RC(exp) report_rc((exp), #exp, __FILE__, __LINE__, true, true)
#define CHECK_RC_FAIL(exp)   report_rc((exp), #exp, __FILE__, __LINE__, false, false)
#define REQUIRE_RC_FAIL(exp) report_rc((exp), #exp, __FILE__, __LINE__, false, true)

    template<class T>
    void AssertNull(const T* e1, const char* e1str,const char* file, unsigned int line, bool critical=false)
    {
        if (e1 == 0)
        {
            report_passed2(e1str, "NULL", e1, (const T*)0, file, line, "==", "!=");
        }
        else
        {
            report_error2 (e1str, "NULL", e1, (const T*)0, file, line, "==", "!=", critical);
        }
    }
#define CHECK_NULL(e1)   AssertNull((e1), #e1, __FILE__,__LINE__, false)
#define REQUIRE_NULL(e1) AssertNull((e1), #e1, __FILE__,__LINE__, true)

    template<class T>
    void AssertNotNull(const T* e1, const char* e1str,const char* file, unsigned int line, bool critical=false)
    {
        if (e1 != 0)
        {
            report_passed2(e1str, "NULL", e1, (const T*)0, file, line, "!=", "==");
        }
        else
        {
            report_error2 (e1str, "NULL", e1, (const T*)0, file, line, "!=", "==", critical);
        }
    }
#define CHECK_NOT_NULL(e1)   AssertNotNull((e1), #e1, __FILE__,__LINE__, false)
#define REQUIRE_NOT_NULL(e1) AssertNotNull((e1), #e1, __FILE__,__LINE__, true)

private:
    std::string _name;
    ncbi::NK::counter_t _ec;
};


class SharedTest : protected TestCase {
    TestCase * _dad;

protected:
    SharedTest ( TestCase * dad, const char * name )
        : TestCase ( std::string ( dad -> GetName () ) + "." + name )
        , _dad ( dad )
    {
        assert ( _dad );
    }

    ~SharedTest ( void ) {
        _dad -> ErrorCounterAdd ( GetErrorCounter () );
    }
};


class TestInvoker {
protected:
    TestInvoker(const std::string& name) : _name(name), _ec(0) {}
    virtual ~TestInvoker(void) {}
public:
    virtual void Run(void* globalFixtute) throw () = 0;
    const std::string& GetName(void) const { return _name; }
    ncbi::NK::counter_t GetErrorCounter(void) { return _ec; }
protected:
    void SetErrorCounter(ncbi::NK::counter_t ec)
    { _ec = ec; }
private:
    const std::string _name;
    ncbi::NK::counter_t _ec;
};

class TestRunner {
    typedef std::vector<ncbi::NK::TestInvoker*> T;
    typedef T::const_iterator TCI;

public:
    TestRunner();

    int    argc;
    char** argv;

    void ReportTestNumber(void);
    void SetArgs(int argc, char* argv[]);
    void Add(ncbi::NK::TestInvoker* t);
    counter_t Run(void* globalFixtute) const throw ();

private:
    T _cases;
};

extern ncbi::NK::TestRunner* GetTestSuite();

template<class TFixture>
ncbi::NK::counter_t Main(int argc, char* argv[],
                         const char* suite_name)
{
    ncbi::NK::counter_t ec = 0;
    ncbi::NK::TestRunner* t = ncbi::NK::GetTestSuite();
    assert(t);
    t->SetArgs(argc, argv);
    t->ReportTestNumber();

    TFixture globalFixtute;
    LOG(ncbi::NK::LogLevel::e_test_suite,
        "Entering test suite \"" << suite_name << "\"\n");
    auto start = std::chrono::high_resolution_clock::now();

    ec = t->Run(&globalFixtute);
    LOG(ncbi::NK::LogLevel::e_test_suite,
        "Leaving test suite \"" << suite_name << "\"\n");

    switch (ec)
    {
    case 0:
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
            LOG(ncbi::NK::LogLevel::e_nothing, "\n*** No errors detected (" << duration.count() << " s total)\n" );
        }
        break;
    case 1:
        LOG(ncbi::NK::LogLevel::e_nothing, "\n*** " << ec <<
            " failure detected in test suite \"" << suite_name << "\"\n");
        break;
    default:
        LOG(ncbi::NK::LogLevel::e_nothing, "\n*** " << ec <<
            " failures detected in test suite \"" << suite_name << "\"\n");
        break;
    }
    return ec;
}

} } // namespace ncbi::NK


#endif// NCBI_NK_UNIT_TEST__SUITE_HPP
