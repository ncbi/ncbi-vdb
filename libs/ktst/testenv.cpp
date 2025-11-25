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

#include <ktst/unit_test.hpp>

#include <klib/rc.h>
#include <klib/text.h>
#include <kapp/vdbapp.h>
#include <kfg/config.h>

#include <sstream>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <mutex>

using namespace ::ncbi::NK;
using std::string;

//const int TestEnv::TEST_CASE_TIMED_OUT=14; //SIGALRM
//const int TestEnv::TEST_CASE_FAILED=255;

bool TestEnv::in_child_process = false;

namespace ncbi::NK
{
    rc_t
    UsageSummary(const char* progname)
    {
        std::cout
            << "Usage:\n"
            << progname
            << " [OPTIONS]\n";
        return 0;
    }

    rc_t
    Usage( const Args * args )
    {
        rc_t rc = 0;

        const char * progname = "";

        if (args == NULL)
            rc = RC(rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
        else
            rc = ArgsProgram(args, nullptr, &progname);
        UsageSummary ( progname );
        HelpOptionsStandard();
        std::cout <<
            "Testing framework options:\n"
            "  -debug                   Print recognized command line arguments (should be specified first)\n"
            "  -catch_system_errors     Allows to switch between catching and ignoring system errors (signals)\n"
            "  -l=(log_level)           Specifies log level\n"
            "\tall                        report all log messages including the passed test notification\n"
            "\ttest_suite                 show test suite messages\n"
            "\tmessage                    show user messages\n"
            "\twarning                    report warnings issued by user\n"
            "\terror                      report all error conditions (default)\n"
            "\tfatal_error                report user or system originated fatal errors (for example, memory access violation)\n"
            "\tnothing                    do not report any information\n";
        return rc;
    }

    rc_t
    DefaultArgsHandler(int argc, char * argv[])
    {
        rc_t rc = VdbInitialize(argc, argv, 0); // this may be not the first call to VdbInitialize, but should not be a problem.
        if ( rc == 0 )
        {
            SetUsage( ncbi::NK::Usage );
            SetUsageSummary( ncbi::NK::UsageSummary );
            Args * args = nullptr;
            rc = ArgsMakeAndHandle(&args, argc, argv, 0, nullptr, 0);
            ArgsWhack(args);
        }
        return rc;
    }
}

TestEnv::TestEnv(int argc, char* argv[], ArgsHandler* argsHandler)
    : catch_system_errors(true)
    , argc2(0)
    , argv2(NULL)
{
    rc_t rc = process_args(argc, argv, argsHandler);
    if (rc != 0) {
        throw rc;
    }
}

TestEnv::~TestEnv ()
{
    if (argv2 != NULL) {
        for (int i = 0; i < argc2; ++i) {
            free(argv2[i]);
            argv2[i] = NULL;
        }
        free(argv2);
    }
}

char TestEnv::lastLocation[] = "<init>";
LogLevel::E TestEnv::verbosity = LogLevel::e_error;
bool TestEnv::verbositySet = false;

void CC TestEnv::TermHandler() noexcept
{
    SigHandler(SIGTERM);
}

void CC TestEnv::SigHandler(int sig) noexcept
{
    switch (sig)
    {
        case SIGABRT:
            LOG(ncbi::NK::LogLevel::e_fatal_error,
                "fatal error: signal: SIGABRT(Abort call)\n");
            break;
        case SIGFPE:
            LOG(ncbi::NK::LogLevel::e_fatal_error,
                "fatal error: signal: SIGFPE(Floating point exception)\n");
            break;
        case SIGILL:
            LOG(ncbi::NK::LogLevel::e_fatal_error,
                "fatal error: signal: SIGILL(Illegal instruction)\n");
            break;
        case SIGSEGV:
            LOG(ncbi::NK::LogLevel::e_fatal_error,
                "fatal error: signal: SIGSEGV(Segmentation fault)\n");
            break;
        default:
            LOG(ncbi::NK::LogLevel::e_fatal_error,
                "fatal error: signal: " << sig << "\n");
            break;
    }
    LOG(ncbi::NK::LogLevel::e_fatal_error,
        ncbi::NK::TestEnv::lastLocation << ": last checkpoint\n");
    LOG(ncbi::NK::LogLevel::e_fatal_error, "Test is aborted\n");
    exit(sig);
}

rc_t TestEnv::process_args(int argc, char* argv[], ArgsHandler* argsHandler)
{
    size_t arg2 = 9;
    argv2 = static_cast<char**>(calloc(arg2, sizeof *argv2));
    if (argv2 == NULL)
    {   return RC (rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted); }
    argv2[argc2] = string_dup_measure(argv[argc2], NULL);
    if (argv2[argc2] == NULL)
    {   return RC (rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted); }
    ++argc2;

    rc_t rc = 0;

    bool debug = false;
    LogLevel::E detected = LogLevel::e_undefined;
    char arg_catch_system_errors[] = "-catch_system_errors";
    char arg_log_level          [] = "-l=";
    for (int i = 1; i < argc; ++i)
    {
        if (verbositySet) {
            break;
        }
        if (strncmp(argv[i], arg_log_level, strlen(arg_log_level))
            == 0)
        {
            char* a = argv[i] + strlen(arg_log_level);
            if (strcmp(a, "test_suite") == 0)
                detected = LogLevel::e_test_suite;
            else if (strcmp(a, "all"    ) == 0)
                detected = LogLevel::e_all;
            else if (strcmp(a, "message") == 0)
                detected = LogLevel::e_message;
            else if (strcmp(a, "warning") == 0)
                detected = LogLevel::e_warning;
            else if (strcmp(a, "error"  ) == 0)
                detected = LogLevel::e_error;
            else if (strcmp(a, "nothing") == 0)
                detected = LogLevel::e_nothing;
            else if (strcmp(a, "fatal_error") == 0)
                detected = LogLevel::e_fatal_error;
            if (detected != LogLevel::e_undefined)
            {
                verbosity = detected;
                if (debug)
                {
                    LOG(LogLevel::e_nothing,
                        "debug: log_level was set to " << a << std::endl);
                }
            }
            else
            {
                verbosity = LogLevel::e_error;
                if (debug)
                {
                    LOG(LogLevel::e_nothing,
                        "debug: log_level was set to error\n");
                }
            }
        }
        else if (strncmp(argv[i], arg_catch_system_errors,
            strlen(arg_catch_system_errors)) == 0)
        {
            if (debug)
            {
                LOG(LogLevel::e_nothing,
                    "debug: arg_catch_system_errors was set to true\n");
            }
        }
        else if (strcmp(argv[i], "-debug") == 0)
        {
            debug = true;
            LOG(LogLevel::e_nothing, "debug: debug was set to true\n");
        }
        else {
            if ( (size_t)argc2 >= arg2 ) {
                arg2 *= 2;
                char ** tmp = static_cast < char ** > (
                    realloc ( argv2, arg2 * sizeof *argv2 ) );
                if ( tmp == NULL )
                    return RC (
                        rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted );
                argv2 = tmp;
            }
            argv2 [ argc2 ] = string_dup_measure ( argv [ i ], NULL );
            if ( argv2 [ argc2 ] == NULL )
                return RC ( rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted );
            ++ argc2;
        }
    }

    if (verbosity == LogLevel::e_undefined)
    {
        verbosity = LogLevel::e_error;
        if (debug)
        {
            LOG(LogLevel::e_nothing,
                "debug: log_level was set to error\n");
        }
    }

    if (rc == 0) {
        if (argsHandler)
        {	rc = argsHandler(argc2, argv2); }
    }

    // ignore ~/.ncbi/user-settings.mkfg
    KConfigDisableUserSettings();

    return rc;
}

std::mutex saveLocation_mtx;
void ::ncbi::NK::saveLocation(const char* file, unsigned int line)
{
    std::ostringstream s;
    s << file << "(" << line << ")";
    saveLocation_mtx.lock();
    strncpy( TestEnv::lastLocation, s.str().c_str(), sizeof(TestEnv::lastLocation) - 1 );
    TestEnv::lastLocation [ sizeof(TestEnv::lastLocation) - 1 ] = 0;
    saveLocation_mtx.unlock();
}

void ::ncbi::NK::_REPORT_CRITICAL_ERROR_(const string& msg, const char* file, unsigned int line, bool is_msg)
{
    LOG(ncbi::NK::LogLevel::e_error, file << "(" << line << "): ");
    LOG(ncbi::NK::LogLevel::e_error, "fatal ");
    LOG(ncbi::NK::LogLevel::e_error, "error in fixture: ");
    if (is_msg) {
        LOG(ncbi::NK::LogLevel::e_error, msg);
    }
    else {
        LOG(ncbi::NK::LogLevel::e_error, "critical ");
        LOG(ncbi::NK::LogLevel::e_error, "check " << msg << " failed");
    }
    LOG(ncbi::NK::LogLevel::e_error, std::endl);
    throw ncbi::NK::execution_aborted();
 }

ncbi::NK::TestRunner*
ncbi::NK::GetTestSuite(void)
{
    static ncbi::NK::TestRunner t;
    return &t;
}

bool TestEnv::Sleep(unsigned int seconds)
{
    return TestEnv::SleepMs(seconds*1000);
}

