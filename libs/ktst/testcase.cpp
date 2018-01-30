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

#include <klib/printf.h> 

#include <cstring> // strlen
#include <ctime> // time

using namespace ncbi::NK;

void TestCase::Init(const std::string& name)  { _name = name; _ec = 0; } 

void TestCase::report_error(const char* msg, const char* file, int line, bool is_msg, bool isCritical)
{
    ncbi::NK::saveLocation(file, line);
    ++_ec;
    LOG(LogLevel::e_error, TestEnv::FormatLocation(file, line));
    if (isCritical) 
    {
        LOG(LogLevel::e_error, "fatal ");
    }
    LOG(LogLevel::e_error, "error in \"" << _name << "\": ");
    if (is_msg) 
    {
        LOG(LogLevel::e_error, msg);
    }
    else 
    {
        if (isCritical) 
        {
            LOG(LogLevel::e_error, "critical ");
        }
        LOG(LogLevel::e_error, "check " << msg << " failed");
    }
    LOG(LogLevel::e_error, std::endl);
    if (isCritical)
    { 
        throw ncbi::NK::execution_aborted(); 
    }
}

void TestCase::report_passed(const char* msg, const char* file, int line) 
{
    ncbi::NK::saveLocation(file, line);
    LOG(LogLevel::e_all, file << "(" << line << "): info: " "check " << msg << " passed" << std::endl);
}

void TestCase::_REPORT_CRITICAL_ERROR_(const std::string& msg, const char* file, int line, bool is_msg)
{ 
    report_error( msg.c_str(), file, line, is_msg, true ); 
}

void TestCase::report_rc(rc_t rc, const char* callStr, const char* file, int line, int successExpected, bool isCritical)
{
    if ((successExpected && rc != 0) || (!successExpected && rc == 0))
    {
        const int BufSize=1024;
        char buf[BufSize];
        size_t num_writ=0;
        if (0 == string_printf (buf, BufSize, &num_writ, "Unexpected return code from %s: %R\n", callStr, rc))
        {
            report_error(buf, file, line, true, isCritical);
        }
        else
        {
            report_error("***error message too large to print***", file, line, true, isCritical);
        }
    }
    else {
        ncbi::NK::saveLocation(file, line);
        time_t t = time(NULL);
        char *pp = ctime(&t);
        size_t s = strlen(pp);
        if (s > 0) {
            --s;
        }
        pp[s] = '\0';
        LOG(LogLevel::e_all, file << "(" << line << "): [" << pp
            << "] info: " "check " << callStr << " "
            << (successExpected ? "=" : "!") << "= 0 passed" << std::endl);
    }
}

