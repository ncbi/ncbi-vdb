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

%{

#define YYDEBUG 1

#include "json-lex.h"

#include "json-tokens.h"

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    { int i; for(i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 1; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }}

%}

%option never-interactive nounistd yylineno

%option bison-locations

%option prefix="json_yy"

/* re-entrant scanner */
%option reentrant

 /* maintain state stack */
%option stack

 /* let parser handle EOF */
%option noyywrap

 /* start-state for slash-star comments */

char        ([^"\\]|\\["\\bfnrt\/]|\\u[0-9a-fA-F]{4})
whitespace  [\t\r\n ]

digit       [0-9]
digit1to9   [1-9]
uint        ({digit1to9}{digit}+|{digit})
sint        ([\-\+]{uint})
int         ({sint}|{uint})

exp         ([eE](\-|\+)|[eE])

%%

 /* literals */
"{" { return yytext [ 0 ]; }
"}" { return yytext [ 0 ]; }
"=" { return yytext [ 0 ]; }
"[" { return yytext [ 0 ]; }
"]" { return yytext [ 0 ]; }
":" { return yytext [ 0 ]; }
"," { return yytext [ 0 ]; }
\"{char}*\"    { return jsonSTRING; }

 /* keywords */
"true"    { return jsonTRUE; }
"false"   { return jsonFALSE; }
"null"    { return jsonNULL; }

{int}\.{digit}+{exp}{digit}+    { return jsonNUMBER; }
{int}{exp}{digit}+              { return jsonNUMBER; }
{int}\.{digit}+                 { return jsonNUMBER; }
{int}                           { return jsonNUMBER; }

 {whitespace}

 /* unrecognized input */
.                                       { return UNRECOGNIZED; }

%%

void
JsonScan_yylex_init ( JsonScanBlock* sb, const char *str, size_t len )
{
    yylex_init ( & sb -> scanner );
    sb -> buffer = yy_scan_bytes ( ( yyconst char * ) str, len, sb -> scanner );
    ( ( struct yyguts_t * ) sb -> scanner ) -> yyextra_r = sb; /* back pointer to the scan block */
    sb -> error = NULL;
}

void
JsonScan_yylex_destroy ( JsonScanBlock* sb )
{
    yy_delete_buffer ( sb -> buffer, sb -> scanner );
    if ( sb -> scanner )
    {
        yylex_destroy ( sb -> scanner );
    }
    sb -> scanner = NULL;
    sb -> buffer = NULL;
    free ( sb -> error );
}

void
JsonScan_set_debug ( JsonScanBlock* sb, int on )
{
    yyset_debug ( on ? 1 : 0, sb->scanner );
}

enum yytokentype
JsonScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, JsonScanBlock* sb )
{
    enum yytokentype ret = ( enum yytokentype ) yylex ( lvalp, llocp, sb -> scanner );
    lvalp -> type       = ret;
    lvalp -> value      = ( ( struct yyguts_t * ) sb -> scanner ) -> yytext_r;
    lvalp -> value_len  = ( ( struct yyguts_t * ) sb -> scanner ) -> yyleng_r;
    if ( ret == jsonSTRING )
    {   /* clip off the quotes */
        ++ lvalp -> value;
        lvalp -> value_len -= 2;
    }
    lvalp -> node = NULL;
    return ret;
}

