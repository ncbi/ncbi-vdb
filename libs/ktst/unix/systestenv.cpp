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

#include <ktst/unit_test_suite.hpp>

#include <sstream>

#include <csignal> // sigaction, not necessarily in csignal :-/
#include <cstdlib>
#include <cstring>

#include <time.h>
#include <unistd.h> /* fork */

#include <sys/wait.h>
#include <sys/types.h>

using namespace std;
using namespace ncbi::NK;

#undef REPORT_ERROR
#define REPORT_ERROR(msg) _REPORT_CRITICAL_ERROR_("TestEnv::" msg, __FILE__, __LINE__, true);

static void alarmHandler(int)
{
    cerr << "child process timed out" << endl;
    
    exit(TestEnv::TEST_CASE_TIMED_OUT);
}

int TestEnv::RunProcessTestCase(TestCase& obj, void(TestCase::*meth)(), int timeout)
{
    int pid=fork();
    if (pid == -1)
    {
        REPORT_ERROR("RunProcessTestCase: fork() failed");
    }
    if (pid == 0)
    {   /* child process */
        if (timeout != 0)
        {
            struct sigaction act;
            memset(&act, 0, sizeof act);
            act.sa_handler = alarmHandler;
            act.sa_flags = SA_RESETHAND;
            sigaction(SIGALRM , &act, NULL);        
            alarm(timeout);
        }
        try 
        {
            in_child_process = true;
            (obj.*meth)();
        }   
        catch (const exception& ex)
        {
            cerr << obj.GetName() << " threw " << ex.what() << endl;
            exit(TEST_CASE_FAILED);
        }
        catch (const ncbi::NK::execution_aborted&)
        {
            cerr << obj.GetName() << " aborted " << endl;
            exit(TEST_CASE_FAILED);
        }
        catch (...)
        {
            cerr << obj.GetName() << " threw something " << endl;
            exit(TEST_CASE_FAILED);  
        }
        exit(0);
    }
    /* parent process */
    int status;
    if (waitpid(pid, &status, 0) != pid) /* suspend until the child terminates */
    {   
        REPORT_ERROR("RunProcessTestCase: wait() failed");
    }
    if (!WIFEXITED(status)) 
    {   
        REPORT_ERROR("RunProcessTestCase: child exited abnormally");
    }
    
    return WEXITSTATUS(status); /* exit status of the child process */
}

bool TestEnv::SleepMs(unsigned int milliseconds)
{
    time_t seconds = milliseconds / 1000;
    long nanoseconds = (milliseconds % 1000) * 1000 * 1000;
    struct timespec time = { seconds, nanoseconds };
    return nanosleep(&time, NULL) == 0; 
}

void TestEnv::set_handlers(void)
{
    struct sigaction act;
    memset(&act, 0, sizeof act);
    act.sa_handler = SigHandler;
    act.sa_flags = SA_RESETHAND;
    if (sigaction(SIGFPE , &act, NULL) != 0)    REPORT_ERROR("set_handlers: sigaction() failed");
    if (sigaction(SIGILL , &act, NULL) != 0)    REPORT_ERROR("set_handlers: sigaction() failed");
    if (sigaction(SIGSEGV , &act, NULL) != 0)   REPORT_ERROR("set_handlers: sigaction() failed");
}

string TestEnv::GetPidString()
{
    ostringstream str;
    str << getpid();
    return str.str();
}

string TestEnv::FormatLocation(const string& p_file, uint64_t p_line)
{
    ostringstream out;
    out << p_file << ":" << p_line << ": ";
    return out.str();
}
