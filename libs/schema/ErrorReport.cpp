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

#include <new>

#include <kfc/xc.h>
#include <kfc/except.h>

#include <klib/symbol.h>
#include <klib/printf.h>

#include <kfs/directory.h>
#include <kfs/mmap.h>

using namespace ncbi::SchemaParser;
using namespace std;

///////////////////////////////////////// InternalError exception

InternalError :: InternalError(const char * p_text)
: m_text ( string_dup( p_text, string_size ( p_text ) ) )
{
}

InternalError :: ~InternalError()
{
    free ( m_text );
}

const char*
InternalError :: what() const
{
    return m_text;
}

///////////////////////////////////////// Error Report :: Error

ErrorReport :: Error *
ErrorReport :: Error :: Make ( ctx_t ctx, const char * p_message, const ErrorReport :: Location & p_location )
{
    void * ret = malloc ( sizeof ( ErrorReport :: Error ) );
    if ( ret == 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        SYSTEM_ERROR ( xcNoMemory, "" );
        return 0;
    }
    return new ( ret ) ErrorReport :: Error ( p_message, p_location );
}

void
ErrorReport :: Error :: Destroy ( ErrorReport :: Error * self )
{
    if ( self != 0 )
    {
        self -> ~Error();
        free ( self );
    }
}

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
ErrorReport :: Error :: Format ( ctx_t ctx, char * p_buf, size_t p_bufSize ) const
{
    if ( p_buf == 0 )
    {
        return false;
    }
    rc_t rc = string_printf ( p_buf, p_bufSize, 0,
                           "%s:%u:%u %s",
                           m_file, m_line, m_column, m_message );
    if ( rc != 0 )
    {
        FUNC_ENTRY ( ctx, rcSRA, rcSchema, rcParsing );
        INTERNAL_ERROR ( xcUnexpected, "string_printf, rc=%R", rc );
        return false;
    }
    return true;
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
WhackError ( void *item, void *data ) noexcept
{
    ErrorReport :: Error :: Destroy ( reinterpret_cast < ErrorReport :: Error * > ( item ) );
}

void
ErrorReport :: Clear ()
{
    VectorWhack ( & m_errors, WhackError, 0 );
}

void
ErrorReport :: ReportError ( ctx_t ctx, const Location & p_location, const char* p_fmt, ... )
{
    FUNC_ENTRY ( ctx, rcSRA, rcSchema, rcParsing );
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    rc_t rc = string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcUnexpected, "string_vprintf, rc=%R", rc );
    }
    else
    {
        rc = :: VectorAppend ( & m_errors, 0, Error :: Make ( ctx, buf, p_location ) );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcUnexpected, "VectorAppend, rc=%R", rc );
        }
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

