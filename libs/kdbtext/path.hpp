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

#pragma once

#include <string>
#include <queue>
#include <cstdarg>

#include <klib/rc.h>

struct VPath;

namespace KDBText
{
    // converts a string path with '/' as the separator into a vector of individual elements
    class Path : public std::queue< std::string >
    {
    public:
        static rc_t PrintToString( const char *fmt, va_list args, std::string & out );

    public:
        Path( const std::string & p_source );
        Path( const char *fmt, va_list args );
        Path( const struct VPath * path );

    private:
        void fromString( const std::string & p_source );
    };
}
