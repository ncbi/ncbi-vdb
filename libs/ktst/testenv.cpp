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

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED

#include <kapp/main.h> // KAppVersion
#include <kapp/args.h> // ArgsWhack
#include <klib/out.h> // KOutMsg

#endif

#include <klib/rc.h>
#include <sstream> 
#include <csignal>
#include <cstdlib>
#include <cstring>

using namespace ::ncbi::NK;
using std::string;

//const int TestEnv::TEST_CASE_TIMED_OUT=14; //SIGALRM
//const int TestEnv::TEST_CASE_FAILED=255;

bool TestEnv::in_child_process = false;

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
struct Args* TestEnv::args = 0;
#endif

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

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
    ArgsWhack(args);
    args = NULL;
#endif
}

string TestEnv::lastLocation;
LogLevel::E TestEnv::verbosity = LogLevel::e_error;
bool TestEnv::verbositySet = false;

void CC TestEnv::TermHandler() 
{
    SigHandler(SIGTERM);
}

void CC TestEnv::SigHandler(int sig) 
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
    if (!ncbi::NK::TestEnv::lastLocation.empty())
    {
        LOG(ncbi::NK::LogLevel::e_fatal_error,
            ncbi::NK::TestEnv::lastLocation << ": last checkpoint\n");
    }
    LOG(ncbi::NK::LogLevel::e_fatal_error, "Test is aborted\n");
    exit(sig);
}


#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED

#define OPTION_DBG "verbose_cmd_line"
#define ALIAS_DBG "a"
static const char * dbg_usage[] = { "print command line argument processing information", NULL };

#define OPTION_CSE "catch_system_errors"
#define ALIAS_CSE "c"
static const char * cse_usage[] = { "[yes|y|no|n] catch system errors, default is yes", NULL };

#define OPTION_LOG "test_log_level"
#define ALIAS_LOG "t"
static const char * log_usage[] = { "test log level, one of:",
    "'all':       report all log messages",
    "             including the passed test notification;",
    "'test_suite': show test suite messages;",
    "'message':   show user messages",
    "'warning':   report warnings issued by user;",
    "'error':     report all error conditions (default);",
    "'fatal_error': report user or system originated fatal errors",
    "             (for example, memory access violation);",
    "'nothing':   do not report any information", NULL };

OptDef Options[] = {
      { OPTION_DBG, ALIAS_DBG, NULL, dbg_usage, 1, false, false }
    , { OPTION_CSE, ALIAS_CSE, NULL, cse_usage, 1, true , false }
    , { OPTION_LOG, ALIAS_LOG, NULL, log_usage, 1, true , false }
};

#endif

rc_t TestEnv::process_args(int argc, char* argv[], ArgsHandler* argsHandler) 
{
    int arg2 = 9;
    argv2 = static_cast<char**>(calloc(arg2, sizeof *argv2));
    if (argv2 == NULL)
    {   return RC (rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted); }
    argv2[argc2] = strdup(argv[argc2]);
    if (argv2[argc2] == NULL)
    {   return RC (rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted); }
    ++argc2;

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED

    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 1,
        Options, sizeof Options / sizeof (OptDef));
    if (rc)
    {   return rc; }

    bool debug = false;
    LogLevel::E detected = LogLevel::e_undefined;
    do {
        uint32_t pcount = 0;

        rc = ArgsOptionCount(args, OPTION_DBG, &pcount);
        if (rc)
        {   return rc; }
        if (pcount) {
            debug = true;
            LOG(LogLevel::e_nothing, "debug: debug was set to true\n");
        }

        rc = ArgsOptionCount(args, OPTION_CSE, &pcount);
        if (rc)
        {   return rc; }
        if (pcount) {
            const char* pc = NULL;
            rc = ArgsOptionValue(args, OPTION_CSE, 0, (const void **)&pc);
            if (rc)
            {   return rc; }
            if (!strcmp(pc, "n") || !strcmp(pc, "no")) {
                catch_system_errors = false;
                if (debug) {
                    LOG(LogLevel::e_nothing,
                       "debug: arg_catch_system_errors was set to false\n");
                }
            }
            else {
                if (debug) {
                    LOG(LogLevel::e_nothing,
                       "debug: arg_catch_system_errors was set to true\n");
                }
            }
        }

        rc = ArgsOptionCount(args, OPTION_LOG, &pcount);
        if (rc)
        {   return rc; }
        if (pcount) {
            const char* a = NULL;
            rc = ArgsOptionValue(args, OPTION_LOG, 0, (const void **)&a);
            if (rc)
            {   return rc; }
            if (!strcmp(a, "test_suite"))
            { detected = LogLevel::e_test_suite; }
            else if (strcmp(a, "all"    ) == 0)
            { detected = LogLevel::e_all; }
            else if (strcmp(a, "message") == 0)
            { detected = LogLevel::e_message; }
            else if (strcmp(a, "warning") == 0)
            { detected = LogLevel::e_warning; }
            else if (strcmp(a, "error"  ) == 0)
            { detected = LogLevel::e_error; }
            else if (strcmp(a, "nothing") == 0)
            { detected = LogLevel::e_nothing; }
            else if (strcmp(a, "fatal_error") == 0)
            { detected = LogLevel::e_fatal_error; }
            if (detected != LogLevel::e_undefined) {
                verbosity = detected;
                if (debug) {
                    LOG(LogLevel::e_nothing,
                        "debug: log_level was set to " << a << std::endl);
                }
            }
            else {
                verbosity = LogLevel::e_error;
                if (debug) {
                    LOG(LogLevel::e_nothing,
                        "debug: log_level was set to error\n");
                }
            }
        }
    } while (0);

    if (verbosity == LogLevel::e_undefined) {
        verbosity = LogLevel::e_error;
        if (debug) {
            LOG(LogLevel::e_nothing,
                "debug: log_level was set to error\n");
        }
    }
#else

    rc_t rc = 0;

    bool debug = false;
    LogLevel::E detected = LogLevel::e_undefined;
    char arg_catch_system_errors[] = "-catch_system_errors=";
    char arg_log_level          [] = "-l=";
    char arg_app_args           [] = "-app_args=";
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
            char* a = argv[i] + strlen(arg_catch_system_errors);
            if (strcmp(a, "n") == 0 || strcmp(a, "no") == 0)
            {
                catch_system_errors = false;
                if (debug)
                {
                    LOG(LogLevel::e_nothing,
                       "debug: arg_catch_system_errors was set to false\n");
                }
            }
            else
            {
                if (debug)
                {
                    LOG(LogLevel::e_nothing,
                       "debug: arg_catch_system_errors was set to true\n");
                }
            }
        }
        else if (strcmp(argv[i], "-debug") == 0)
        {
            debug = true;
            LOG(LogLevel::e_nothing, "debug: debug was set to true\n");
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            if (debug)
            {
                LOG(LogLevel::e_nothing, "debug: help was set to true\n");
            }
            std::cout << "Usage:\n"
                      << argv[0]
                      << " [-debug] [-catch_system_errors=[yes|y|no|n]]\n"
                         "      [-app_args='<value>'] [-l=<value>] [-h]\n"
                "where:\n"
                "debug              - Print recognized command line arguments\n"
                "                     (should be specified first)\n"
                "catch_system_errors -  Allows to switch between catching\n"
                "                     and ignoring system errors (signals)\n"
                "app_args           - Allows to pass command line arguments\n"
                "                     to application handler\n"
                "    (see unit_test.hpp/FIXTURE_TEST_SUITE_WITH_ARGS_HANDLER)\n"
                "\n"
                "l (log_level)       - Specifies log level\n"
                "\tall        - report all log messages\n"
                "\t             including the passed test notification\n"
                "\ttest_suite - show test suite messages\n"
                "\tmessage    - show user messages\n"
                "\twarning    - report warnings issued by user\n"
                "\terror      - report all error conditions (default)\n"
                "\tfatal_error- report user or system originated fatal errors\n"
                "\t             (for example, memory access violation)\n"
                "\tnothing    - do not report any information\n"
                "\n"
                "h (help)            - this help message\n";
            exit(0);
        }
        else if (strncmp(argv[i], arg_app_args, strlen(arg_app_args)) == 0)
        {
            char* a = argv[i] + strlen(arg_app_args);
            if (debug)
            {
                LOG(LogLevel::e_nothing,
                    string("debug: arg_app_args was detected: ") + a + "\n");
            }
            char* pch = strtok(a ," ");
            while (pch != NULL) {
                if (argc2 >= arg2) {
                    arg2 *= 2;
                    char** tmp = static_cast<char**>(
                        realloc(argv2, arg2 * sizeof *argv2));
                    if (tmp == NULL) {
                        return RC(
                            rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted);
                    }
                    argv2 = tmp;
                }
                argv2[argc2] = strdup(pch);
                if (argv2[argc2] == NULL) {
                    return RC(
                        rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted);
                }
                ++argc2;
                pch = strtok(NULL, " ");
            }
        }
        else {
            if ( argc2 >= arg2 ) {
                arg2 *= 2;
                char ** tmp = static_cast < char ** > (
                    realloc ( argv2, arg2 * sizeof *argv2 ) );
                if ( tmp == NULL )
                    return RC (
                        rcApp, rcArgv, rcAccessing, rcMemory, rcExhausted );
                argv2 = tmp;
            }
            argv2 [ argc2 ] = strdup ( argv [ i ] );
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
#endif

    if (rc == 0) {
        if (argsHandler)
        {	rc = argsHandler(argc2, argv2); }
    }

    return rc;
}

void ::ncbi::NK::saveLocation(const char* file, int line) 
{
    std::ostringstream s;
    s << file << "(" << line << ")";
    TestEnv::lastLocation = s.str();
}

void ::ncbi::NK::_REPORT_CRITICAL_ERROR_(const string& msg, const char* file, int line, bool is_msg)
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

rc_t CC TestEnv::UsageSummary(const char* progname) {
#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
    return KOutMsg(
        "Usage:\n"
        "  %s [--verbose_cmd_line] [--catch_system_errors [yes|y|no|n]] "
        "[-t <value>] [-h] [...]\n", progname);
#else
    std::cout
        << "Usage:\n"
        << progname
        << " [-debug] [-catch_system_errors=[yes|y|no|n]] "
        "[-l=<value>] [-h] [...]\n";
    return 0;
#endif
}

#if ALLOW_TESTING_CODE_TO_RELY_UPON_CODE_BEING_TESTED
rc_t CC TestEnv::Usage(const Args* args)
{
    const char* progname = UsageDefaultName;
    const char* fullpath = UsageDefaultName;

    rc_t rc = (args == NULL) ?
        RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull):
        ArgsProgram(args, &fullpath, &progname);

    if (rc != 0)
        progname = fullpath = UsageDefaultName;

    UsageSummary(progname);

    KOutMsg("\nOptions:\n");
    
    HelpOptionLine(ALIAS_DBG, OPTION_DBG, NULL, dbg_usage);
    HelpOptionLine(ALIAS_CSE, OPTION_CSE, NULL, cse_usage);
    HelpOptionLine(ALIAS_LOG, OPTION_LOG, NULL, log_usage);
    KOutMsg("\n");
    HelpOptionsStandard();
    HelpVersion(fullpath, KAppVersion());
    
    return rc;
}
#else
rc_t CC TestEnv::Usage(const char *progname)
{
    UsageSummary ( progname );
    std::cout <<
        "where:\n"
        "debug - Print recognized command line arguments (should be specified first)\n"
        "catch_system_errors - "
        "Allows to switch between catching and ignoring system errors (signals)\n"
        "l (log_level) - Specifies log level\n"
        "\tall        - report all log messages\n"
        "\t             including the passed test notification\n"
        "\ttest_suite - show test suite messages\n"
        "\tmessage    - show user messages\n"
        "\twarning    - report warnings issued by user\n"
        "\terror      - report all error conditions (default)\n"
        "\tfatal_error- report user or system originated fatal errors\n"
        "\t             (for example, memory access violation)\n"
        "\tnothing    - do not report any information\n"
        "h (help) - this help message\n";
    return 0;
}
#endif

bool TestEnv::Sleep(unsigned int seconds)
{
    return TestEnv::SleepMs(seconds*1000);
}

