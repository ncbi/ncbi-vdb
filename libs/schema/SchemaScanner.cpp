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

#include "SchemaScanner.hpp"

#include <string.h>

#include <klib/text.h>

using namespace ncbi::SchemaParser;

void
SchemaScanner :: Init ( const char * p_source, size_t p_size, bool p_debug )
{
    m_scanBlock . scanner = 0;
    m_scanBlock . buffer = 0;
    m_scanBlock . whitespace = 0;
    m_scanBlock . file_name = "<unknown>";
    SchemaScan_yylex_init ( & m_scanBlock, p_source, p_size );
    if ( p_debug )
    {
        SchemaScan_set_debug ( & m_scanBlock, 1 );
    }
}

SchemaScanner :: SchemaScanner ( const char * p_source, size_t p_size, bool p_debug )
{
    Init ( p_source, p_size, p_debug );
}

SchemaScanner :: SchemaScanner ( const char * p_source, bool p_debug )
{
    Init ( p_source, string_size ( p_source ), p_debug );
}

SchemaScanner :: ~SchemaScanner ()
{
    SchemaScan_yylex_destroy ( & m_scanBlock );
}

// direct access to flex-generated scanner

// need these declared for the following header to compile
class ParseTree;
class ErrorReport;
#include "schema-tokens.h"

extern "C" {
    extern enum yytokentype SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb );
}

Token
SchemaScanner :: NextToken ()
{
    YYLTYPE loc;
    SchemaToken t;
    memset ( & t, 0, sizeof t );
    SchemaScan_yylex ( & t, & loc, & m_scanBlock );
    return Token ( t );
}

