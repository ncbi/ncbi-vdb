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

#include <klib/symbol.h>
#include <klib/printf.h>

#include <kfs/directory.h>
#include <kfs/mmap.h>

using namespace ncbi::SchemaParser;
using namespace std;

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
WhackMessage ( void *item, void *data )
{
    free ( item );
}

void
ErrorReport :: Clear ()
{
    VectorWhack ( & m_errors, WhackMessage, 0 );
}

void
ErrorReport :: ReportError ( const char* p_fmt, ... )
{
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    :: VectorAppend ( & m_errors, 0, string_dup_measure ( buf, 0 ) );
}

