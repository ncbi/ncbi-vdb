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

#include <csignal> 
#include <sstream> 
#include <windows.h>
#include <process.h>

using namespace std;
using namespace ncbi::NK;

#undef REPORT_ERROR
#define REPORT_ERROR(msg) _REPORT_CRITICAL_ERROR_(string("TestEnv::") + msg, __FILE__, __LINE__, true)

/* signal handlers for a single-test case thread */
void CC SigSubHandler(int sig)
{
    _endthreadex(sig);
}
void CC TermSubHandler() 
{
    SigSubHandler(SIGTERM);
}

struct TestCaseCall
{
    TestCaseCall(TestCase& obj, void(TestCase::*meth)())
        : object(&obj), method(meth)
    {
    }

    TestCase* object;
    void(TestCase::*method)();
};

unsigned __stdcall ThreadProc(void* call)
{
    signal(SIGABRT, SigSubHandler);
    signal(SIGFPE, SigSubHandler);
    signal(SIGILL, SigSubHandler);
    signal(SIGINT, SigSubHandler);
    signal(SIGSEGV, SigSubHandler);
    signal(SIGTERM, SigSubHandler);
    set_terminate(TermSubHandler);

    try
    {
        TestCaseCall* c=(TestCaseCall*)call;
        ((c->object)->*(c->method))();
    }
    catch (...)
    {
        return TestEnv::TEST_CASE_FAILED;
    }
    return 0;
}

int TestEnv::RunProcessTestCase(TestCase& obj, void(TestCase::*meth)(), int timeout)
{
    TestCaseCall call(obj, meth);
    TestEnv::in_child_process = true;
    HANDLE thread = (HANDLE)_beginthreadex( NULL, 0, ThreadProc, &call, 0, NULL );
    if (thread == NULL)
    {
        REPORT_ERROR("TestEnv::RunProcessTestCase: failed to start a test case thread");
    }

    // make sure to restore main process's signal handlers before re-throwing an exception
    DWORD rc=0;
    DWORD result=WaitForSingleObject( (HANDLE)thread, timeout == 0 ? INFINITE : timeout*1000);
    try
    {
        switch (result)
        {
        case WAIT_OBJECT_0:
            if (GetExitCodeThread(thread, &rc) == 0)
            {
                DOUBLE error = GetLastError();
                ostringstream str;
                str << "RunProcessTestCase failed(" << error << ")" << endl;
                REPORT_ERROR(str.str());
            }
            break;
        case WAIT_TIMEOUT:
            if (!CloseHandle(thread))
                REPORT_ERROR("CloseHandle failed");
            cerr << "child process timed out" << endl;            
            rc=TEST_CASE_TIMED_OUT;
            break;
        default:
            REPORT_ERROR("WaitForSingleObject failed");
            break;
        }
    }
    catch (const exception& ex)
    {
        REPORT_ERROR(obj.GetName() + " threw " + ex.what());
        rc=TEST_CASE_FAILED;
    }
    catch (const ncbi::NK::execution_aborted&)
    {
        REPORT_ERROR(obj.GetName() + " aborted ");
        rc=TEST_CASE_FAILED;
    }
    catch (...)
    {
        REPORT_ERROR(obj.GetName() + " threw something ");
        rc=TEST_CASE_FAILED;
        set_handlers(); 
        throw;
    }
#undef CALL_FAILED
    set_handlers(); 
    in_child_process = false;
    return (int)rc;
}

bool TestEnv::SleepMs(unsigned int milliseconds)
{
    ::Sleep((DWORD)milliseconds);
    return true;
}

void TestEnv::set_handlers(void) 
{
    signal(SIGABRT, SigHandler);
    signal(SIGFPE, SigHandler);
    signal(SIGILL, SigHandler);
    signal(SIGINT, SigHandler);
    signal(SIGSEGV, SigHandler);
    signal(SIGTERM, SigHandler);
    set_terminate(TermHandler);
}

string TestEnv::GetPidString()
{
    ostringstream str;
    str << GetCurrentProcessId();
    return str.str();
}

string TestEnv::FormatLocation(const string& p_file, uint64_t p_line)
{
    ostringstream out;
    out << p_file << "(" << p_line << "): ";
    return out.str();
}
