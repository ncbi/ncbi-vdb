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
* Unit tests for HTTP interfaces
*/

#include "HttpFixture.hpp"

#include <ktst/unit_test.hpp>
#include <kfg/config.h>
#include <kns/manager.h>
#include <kns/http.h>
#include <kfs/directory.h>
#include <kfs/file.h>

using namespace std;
using namespace ncbi::NK;

rc_t CC
TestStream ::  Whack ( KSTREAM_IMPL *self )
{
    if ( TestEnv::verbosity == LogLevel::e_message )
        cout << "TestStream::Whack() called" << endl;
    return 0;
}

rc_t CC
TestStream :: Read ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read )
{
    if ( TestEnv::verbosity == LogLevel::e_message )
        cout << "TestStream::Read() called" << endl;
    * num_read = 0;
    return 0;
}

rc_t CC
TestStream :: Write ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ )
{
    if ( TestEnv::verbosity == LogLevel::e_message )
        cout << "TestStream::Write() called" << endl;
    * num_writ = size;
    m_requests . push_back ( string ( (const char*)buffer, size) );
    return 0;
}

rc_t CC
TestStream :: TimedRead ( const KSTREAM_IMPL *self, void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    if ( TestEnv::verbosity == LogLevel::e_message )
        cout << "TestStream::TimedRead() called" << endl;

    string response;
    if ( m_responses.size()> 0)
    {
        response = m_responses.front();
        m_responses.pop_front();
    }
    else
    {
        throw logic_error ( "TestStream: TimedRead is out of responses" );
    }

    if ( response.size() >= bsize )
    {
        memmove(buffer, response.c_str(), bsize);
        * num_read = bsize;
        response = response.substr(bsize);
    }
    else
    {
        memmove(buffer, response.c_str(), response.size());
        * num_read = response.size();
        response.clear();
    }
    if ( TestEnv::verbosity == LogLevel::e_message )
        cout << "TestStream::TimedRead returned \"" << string((const char*)buffer, * num_read) << "\"" << endl;

    return 0;
}

rc_t CC
TestStream :: TimedWrite ( KSTREAM_IMPL *self, const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    if ( TestEnv::verbosity == LogLevel::e_message )
        cout << "TestStream::TimedWrite(\"" << string((const char*)buffer, size) << "\") called" << endl;
    * num_writ = size;
    m_requests . push_back ( string ( (const char*)buffer, size) );
    return 0;
}

void
TestStream :: AddResponse ( const string& p_str, bool end_binary )
{
    if (end_binary)
        m_responses.push_back(p_str);
    else
        m_responses.push_back(std::string(p_str.c_str(), p_str.size() + 1));
}

list<string> TestStream :: m_requests;
list<string> TestStream :: m_responses;

KStream_vt_v1 TestStream::vt =
{
    1, 1,
    TestStream::Whack,
    TestStream::Read,
    TestStream::Write,
    TestStream::TimedRead,
    TestStream::TimedWrite
};

HttpFixture :: HttpFixture()
:   m_mgr ( nullptr ),
    m_file ( nullptr ),
    m_req ( nullptr )
{
    if ( KNSManagerMake ( & m_mgr ) != 0 )
        throw logic_error ( "HttpFixture: KNSManagerMake failed" );

    if ( KStreamInit ( & m_stream, ( const KStream_vt* ) & TestStream::vt, "TestStream", "", true, true ) != 0 )
        throw logic_error ( "HttpFixture: KStreamInit failed" );

    TestStream::m_requests.clear();
    TestStream::m_responses.clear();
}

HttpFixture :: ~HttpFixture()
{
    if ( KNSManagerRelease ( m_mgr ) != 0 )
        cerr << "HttpFixture::~HttpFixture KNSManagerRelease failed" << endl;

    if ( KFileRelease ( m_file ) != 0 )
        cerr << "HttpFixture::~HttpFixture KFileRelease failed" << endl;

    if ( KClientHttpRequestRelease ( m_req ) != 0 )
        cerr << "HttpFixture::~HttpFixture KClientHttpRequestRelease failed" << endl;

    if ( ! TestStream::m_responses.empty() )
        cerr << "HttpFixture::~HttpFixture not all TestStream::m_responses have been consumed" << endl;
}

KConfig*
HttpFixture :: MakeConfig( const char* name, const char* contents )
{
    KDirectory* wd;
    if ( KDirectoryNativeDir ( & wd ) != 0 )
        throw logic_error("KfgFixture: KDirectoryNativeDir failed");

    {
        KFile* file;
        if (KDirectoryCreateFile(wd, &file, true, 0664, kcmInit, name) != 0)
            throw logic_error("MakeConfig: KDirectoryCreateFile failed");

        size_t num_writ=0;
        if (KFileWrite(file, 0, contents, strlen(contents), &num_writ) != 0)
            throw logic_error("MakeConfig: KFileWrite failed");

        if (KFileRelease(file) != 0)
            throw logic_error("MakeConfig: KFileRelease failed");
    }

    KConfig* ret;
    {
        if (KConfigMake ( & ret, wd ) != 0)
            throw logic_error("MakeConfig: KConfigMake failed");

        KFile* file;
        if (KDirectoryOpenFileRead(wd, (const KFile**)&file, name) != 0)
            throw logic_error("MakeConfig: KDirectoryOpenFileRead failed");

        if (KConfigLoadFile ( ret, name, file) != 0)
            throw logic_error("MakeConfig: KConfigLoadFile failed");

        if (KFileRelease(file) != 0)
            throw logic_error("MakeConfig: KFileRelease failed");
    }

    if (KDirectoryRemove(wd, true, name) != 0)
        throw logic_error("MakeConfig: KDirectoryRemove failed");
    if (KDirectoryRelease(wd) != 0)
        throw logic_error("MakeConfig: KDirectoryRelease failed");

    return ret;
}

struct KStream *
HttpFixture :: Reconnect ()
{
    return & m_stream;
}

string
HttpFixture :: MakeURL(const string & base)
{
    return string("http://") + base + ".com/blah";
}

KStream HttpFixture :: m_stream;

