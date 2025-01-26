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

#include <kapp/vdbapp.h>

using namespace VDB;

VdbApp::VdbApp(int argc, char* argv[], ver_t vers) 
    : m_argc( argc ), m_argv( argv ), m_argvOwned ( false )
{ 
    m_rc = VdbInitialize(argc, argv, vers); 
}

#if WINDOWS && UNICODE
#include "win/main-priv-win.h"
VdbApp::VdbApp(int argc, wchar_t* argv[], ver_t vers)
    : m_argc( argc ), m_argvOwned ( false )
{
    int status = ConvertWArgsToUtf8(argc, argv, &m_argv, true);
    if (status != 0)
    {
        m_rc = RC(rcApp, rcArgv, rcParsing, rcParam, rcFailed);
    }
    else
    {
        m_argvOwned = true;
        m_rc = VdbInitialize(argc, m_argv, vers);
    }
}
#endif

VdbApp::~VdbApp() 
{ 
    VdbTerminate(m_rc); 
    if (m_argvOwned)
    {
        int i = m_argc;
        while ( -- i >= 0 )
        {
            free ( m_argv [ i ] );
        }
        free ( m_argv );        
    }
}
