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

#include "ErrorReport.hpp"

#include <stdexcept>

#include <klib/symbol.h>
#include <klib/printf.h>

#include <kfs/directory.h>
#include <kfs/mmap.h>

using namespace ncbi::SchemaParser;
using namespace std;

///////////////////////////////////////// Error Report :: Error

ErrorReport :: Error :: Error( const char * p_message, const ErrorReport :: Location & p_location )
:   m_message ( string_dup_measure ( p_message, 0 ) )
{
    m_file = string_dup_measure ( p_location . m_file, 0 );
    m_line = p_location . m_line;
    m_column = p_location . m_column;
}

ErrorReport :: Error :: ~Error()
{
    free ( m_message );
    free ( m_file );
}

bool
ErrorReport :: Error :: Format ( char * p_buf, size_t p_bufSize ) const
{
    if ( p_buf == 0 )
    {
        return false;
    }
    return string_printf ( p_buf, p_bufSize, 0,
                           "%s:%u:%u %s",
                           m_file, m_line, m_column, m_message ) == 0;
}

///////////////////////////////////////// Error Report

ErrorReport :: ErrorReport ()
{
    VectorInit ( & m_errors, 0, 1024 );
}

ErrorReport :: ~ErrorReport ()
{
    Clear();
}

static
void CC
WhackError ( void *item, void *data )
{
    ErrorReport :: Error * it = reinterpret_cast < ErrorReport :: Error * > ( item );
    delete ( it ); // was allocated with new (see below)
}

void
ErrorReport :: Clear ()
{
    VectorWhack ( & m_errors, WhackError, 0 );
}

void
ErrorReport :: ReportError ( const Location & p_location, const char* p_fmt, ... )
{
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    rc_t rc = :: VectorAppend ( & m_errors, 0, new Error ( buf, p_location ) );
    if ( rc != 0 )
    {
        throw logic_error ( "ReportError() : VectorAppend() failed" );
    }
}

void
ErrorReport :: ReportInternalError ( const char * p_source, const char* p_fmt, ... )
{
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    Location loc ( p_source, 0, 0 );
    rc_t rc = :: VectorAppend ( & m_errors, 0, new Error ( buf, loc ) );
    if ( rc != 0 )
    {
        throw logic_error ( "ReportError() : VectorAppend() failed" );
    }
}

const ErrorReport :: Error *
ErrorReport :: GetError ( uint32_t p_idx ) const
{
    return reinterpret_cast < const Error * > ( VectorGet ( & m_errors, p_idx ) );
}

const char *
ErrorReport :: GetMessageText ( uint32_t p_idx ) const
{
    const Error * err = reinterpret_cast < const Error * > ( VectorGet ( & m_errors, p_idx ) );
    return err == 0 ? 0 : err -> m_message;
}

