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

#include "path.hpp"

#include <klib/printf.h>
#include <klib/data-buffer.h>
#include <klib/text.h>

#include <vfs/path.h>

#include <sstream>

using namespace std;
using namespace KDBText;

rc_t
Path::PrintToString( const char *fmt, va_list args, string & out )
{
    KDataBuffer buf;
    rc_t rc = KDataBufferMake ( & buf, 8, 0 );
    if ( rc != 0 )
    {
        return false;
    }

    rc = KDataBufferVPrintf ( & buf, fmt, args );
    if ( rc != 0 )
    {
        return rc;
    }

    out = string((const char *) (buf . base)); // will be 0-terminated

    rc = KDataBufferWhack ( & buf );
    if ( rc != 0 )
    {
        return rc;
    }

    return rc;
}

void
Path::fromString( const string & p_source )
{
    string word;
    istringstream in( p_source );
    while( getline(in, word, '/') )
    {
        push( word );
    }
}

Path::Path( const std::string & p_source )
{
    fromString( p_source );
}

Path::Path( const char *fmt, va_list args )
{
    string p;
    PrintToString( fmt, args, p );
    fromString( p );
}

Path::Path( const struct VPath * p_path )
{
    const String * str;
    rc_t rc = VPathMakeString ( p_path, &str );
    if ( rc == 0 )
    {
        fromString( string( str -> addr, str -> size ) );
        StringWhack( str );
    }
}
