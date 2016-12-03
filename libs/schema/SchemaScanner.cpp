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

using namespace ncbi::SchemaParser;

SchemaScanner :: SchemaScanner ( const char * source, bool p_debug )
{
    SchemaScan_yylex_init ( & m_scanBlock, source );
    if ( p_debug )
    {
        SchemaScan_set_debug ( & m_scanBlock, 1 );
    }
    memset ( & m_lastToken, 0, sizeof m_lastToken );
}

SchemaScanner :: ~SchemaScanner ()
{
    SchemaScan_yylex_destroy ( & m_scanBlock );
}

// direct access to flex-generated scanner
class ParseTree; // need this declared for the following header to compile
#include "schema-tokens.h"

extern "C" {
    extern enum yytokentype SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb );
}

SchemaScanner :: TokenType
SchemaScanner :: Scan()
{
    YYLTYPE loc; //TODO: make a data member, or place in SchemaToken
    return SchemaScan_yylex ( & m_lastToken, & loc, & m_scanBlock );
}

