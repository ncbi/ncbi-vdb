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

#include "vdbapp-priv.h"

#include <klib/text.h>

#include <string>
#include <stdexcept>

using namespace std;
using namespace VDB;

Application::Application(int argc, char* argv[], const char * exe_name)
    : m_argc( (unsigned int)argc ), m_argv( argv ), m_argvOwned ( false )
{
    SetUsageDefaultName( exe_name );
    m_rc = VdbInitialize(argc, argv, 0);
}

#if WINDOWS && UNICODE
#include <kapp/win/main-priv-win.h>
Application::Application(int argc, wchar_t* argv[], const char * exe_name)
    : m_argc( (unsigned int)argc ), m_argvOwned ( false )
{
    int status = ConvertWArgsToUtf8(argc, argv, &m_argv, true);
    if (status != 0)
    {
        m_rc = RC(rcApp, rcArgv, rcParsing, rcParam, rcFailed);
    }
    else
    {
        m_argvOwned = true;
        SetUsageDefaultName( exe_name );
        m_rc = VdbInitialize(argc, m_argv, 0);
    }
}
#endif

Application::~Application()
{
    VdbTerminate(m_rc);
    if (m_argvOwned)
    {
        for ( unsigned int i = 0; i < m_argc; ++i )
        {
            free(m_argv[i]);
        }
        free ( m_argv );
    }
}

bool
Application::IsStandardOption( unsigned int index, bool & skipNext ) const
{
    string opt = m_argv[ index ];
    skipNext = false;
    if ( opt == "-h" || opt == "--help" )
    {
        return true;
    }
    if ( opt == "-V" || opt == "--version" )
    {
        return true;
    }
    if ( opt == "-L" || opt == "--log-level" )
    {
        skipNext = true;
        return true;
    }
    if ( opt.substr(0, 2) == "-v" || opt == "--verbose" )
    {
        return true;
    }
    if ( opt.substr(0, 2) == "-+" )
    {
        return true;
    }
    return false;
}

rc_t
Application::HandleStandardOptions()
{
    Args * args = nullptr;
    // take care of the standard options
    ignore_unknown_arguments = true;
    rc_t rc = ArgsMakeStandardOptions( &args );
    if ( rc == 0 )
    {
        rc = ArgsParse( args, (int)m_argc, (const char **)m_argv );
        ArgsRelease( args );
    }
    ignore_unknown_arguments = false;

    // filter out standard options
    char ** new_argv = (char**) malloc( m_argc * sizeof( *new_argv ) );
    if ( m_argv == 0 )
    {
        throw std::bad_alloc();
    }

    unsigned int new_argc = 0;
    unsigned int i = 0;
    while ( i < m_argc )
    {
        bool skip = false;
        if( i > 0 && IsStandardOption( i, skip ) )
        {
            if ( skip )
            {
                ++i;
            }
        }
        else
        {
            new_argv[ new_argc ] = string_dup( m_argv[i], string_size( m_argv [ i ] ) );
            ++new_argc;
        }
        ++i;
    }

    m_argc = new_argc;
    // we do not own the original m_argv, replace it with a filtered copy
    m_argv = new_argv;
    m_argvOwned = true;

    if ( rc != 0 )
    {
        setRc( rc );
    }
    return rc;
}
