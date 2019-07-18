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

#include "schema-lex.h"

/* fake declarations to make the schema-tokens.h compile */
typedef struct ParseTree ParseTree;
typedef struct ErrorReport ErrorReport;

#include "schema-tokens.h"

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

static void AddWhitespace ( SchemaScanBlock* p_sb, const char* p_ws, size_t p_length );

#define AddWs() AddWhitespace ( ( SchemaScanBlock * ) yyextra, yytext, yyleng )

%}

%option never-interactive nounistd yylineno

%option bison-locations

%option prefix="schema_yy"

/* re-entrant scanner */
%option reentrant

 /* maintain state stack */
%option stack

 /* let parser handle EOF */
%option noyywrap

 /* start-state for slash-star comments */
%x CMT_SLASH_STAR
%x CMT_MULTI_LINE

%x VERSION_STATE

sstr_char   ([^\\'\f\r\n$]|$[^(\\])
dstr_char   ([^\\"\f\r\n$]|$[^(\\])
alpha       [a-zA-Z_]
alphanum    [a-zA-Z_0-9]

%%

 /* literals */

";" { return yytext [ 0 ]; }
"$" { return yytext [ 0 ]; }
"," { return yytext [ 0 ]; }
"{" { return yytext [ 0 ]; }
"}" { return yytext [ 0 ]; }
"=" { return yytext [ 0 ]; }
"(" { return yytext [ 0 ]; }
")" { return yytext [ 0 ]; }
"#" { return yytext [ 0 ]; }
"<" { return yytext [ 0 ]; }
">" { return yytext [ 0 ]; }
"*" { return yytext [ 0 ]; }
"/" { return yytext [ 0 ]; }
":" { return yytext [ 0 ]; }
"[" { return yytext [ 0 ]; }
"]" { return yytext [ 0 ]; }
"|" { return yytext [ 0 ]; }
"." { return yytext [ 0 ]; }
"+" { return yytext [ 0 ]; }
"-" { return yytext [ 0 ]; }
"@" { return yytext [ 0 ]; }

"..." { return ELLIPSIS; }

[1-9][0-9]*                 { return DECIMAL; }
0[0-7]*                     { return OCTAL; }
0[xX][0-9a-fA-F]+           { return HEX; }
[0-9]+\.[0-9]*[eE][0-9]+    { return EXP_FLOAT; }
\.[0-9]+[eE][0-9]+          { return EXP_FLOAT; }
[0-9]+\.[0-9]*              { return FLOAT_; }
\.[0-9]+                    { return FLOAT_; }

 /* single quoted strings */
'{sstr_char}*'              { return STRING; }
'{sstr_char}*               { return UNTERM_STRING; }
'(\\.|{sstr_char})+'        { return ESCAPED_STRING; }
'(\\.|{sstr_char})+         { return UNTERM_ESCAPED_STRING; }

 /* double quoted strings */
\"{dstr_char}*\"            { return STRING; }
\"{dstr_char}*              { return UNTERM_STRING; }
\"(\\.|{dstr_char})+\"      { return ESCAPED_STRING; }
\"(\\.|{dstr_char})+        { return UNTERM_ESCAPED_STRING; }

 /* keywords */
"__no_header"     { return KW___no_header; }
"__row_length"    { return KW___row_length; }
"__untyped"       { return KW___untyped; }
"alias"           { return KW_alias; }
"column"          { return KW_column; }
"const"           { return KW_const; }
"control"         { return KW_control; }
"database"        { return KW_database; }
"decode"          { return KW_decode; }
"default"         { return KW_default; }
"encode"          { return KW_encode; }
"extern"          { return KW_extern; }
"false"           { return KW_false; }
"fmtdef"          { return KW_fmtdef; }
"function"        { return KW_function; }
"include"         { return KW_include; }
"limit"           { return KW_limit; }
"physical"        { return KW_physical; }
"read"            { return KW_read; }
"readonly"        { return KW_readonly; }
"return"          { return KW_return; }
"schema"          { return KW_schema; }
"static"          { return KW_static; }
"table"           { return KW_table; }
"template"        { return KW_template; }
"trigger"         { return KW_trigger; }
"true"            { return KW_true; }
"type"            { return KW_type; }
"typedef"         { return KW_typedef; }
"typeset"         { return KW_typeset; }
"validate"        { return KW_validate; }

"version"                       { yy_push_state ( VERSION_STATE, yyscanner ); return KW_version; }
<VERSION_STATE>1\.0             { yy_pop_state ( yyscanner ); return VERS_1_0; }
<VERSION_STATE>1                { yy_pop_state ( yyscanner ); return VERS_1_0; }
<VERSION_STATE>2\.0             { yy_pop_state ( yyscanner ); return VERS_2_0; }
<VERSION_STATE>2                { yy_pop_state ( yyscanner ); return VERS_2_0; }
 /* reject versions we do not know about
    <VERSION_STATE>[0-9]+\.[0-9]+   { yy_pop_state ( yyscanner ); return FLOAT_; }
    <VERSION_STATE>[0-9]+           { yy_pop_state ( yyscanner ); return DECIMAL; }
 */

"view"            { return KW_view; }
"virtual"         { return KW_virtual; }
"void"            { return KW_void; }
"write"           { return KW_write; }

{alpha}{alphanum}*         { return IDENTIFIER_1_0; }
[0-9]{alpha}{alphanum}+   { return IDENTIFIER_1_0; } /* for things like 4na */
\.{alpha}{alphanum}*       { return PHYSICAL_IDENTIFIER_1_0; }

#\ *[0-9]+\.[0-9]+\.[0-9]+     { return VERSION; }
#\ *[0-9]+\.[0-9]+             { return VERSION; }
#\ *[0-9]+                     { return VERSION; }

 /* multi-line comments */
<*>\/\*                                         { yy_push_state ( CMT_SLASH_STAR, yyscanner ); AddWs(); }
<CMT_SLASH_STAR,CMT_MULTI_LINE>[^*\n]+          { AddWs(); }
<CMT_SLASH_STAR,CMT_MULTI_LINE>\*+[^*/\n]+      { AddWs(); }
<CMT_SLASH_STAR,CMT_MULTI_LINE>\**\n            { BEGIN CMT_MULTI_LINE; AddWs(); }
<CMT_SLASH_STAR>\*+\/                           { yy_pop_state ( yyscanner ); AddWs(); }
<CMT_MULTI_LINE>\*+\/                           { yy_pop_state ( yyscanner ); AddWs(); }

 /* line comments */
<*>\/\/.*                                       { AddWs(); }

 /* ignored tokens */
<*>[ \t\f\v\r\n]                                { AddWs(); }

 /* unrecognized input */
<*>.                                       { return UNRECOGNIZED; }

%%

#include <klib/text.h>

void
SchemaScan_yylex_init ( SchemaScanBlock* sb, const char *str, size_t len )
{
    yylex_init ( & sb -> scanner );
    sb -> buffer = yy_scan_bytes ( ( yyconst char * ) str, len, sb -> scanner );
    sb -> whitespace = NULL;
    ( ( struct yyguts_t * ) sb -> scanner ) -> yyextra_r = sb; /* back pointer to the scan block, not touched by flex */
}

void
SchemaScan_yylex_destroy ( SchemaScanBlock* sb )
{
    yy_delete_buffer ( sb -> buffer, sb -> scanner );
    if ( sb -> scanner )
    {
        yylex_destroy ( sb -> scanner );
    }
    sb -> scanner = NULL;
    sb -> buffer = NULL;
    free ( sb -> whitespace );
}

void
SchemaScan_set_debug ( SchemaScanBlock* sb, int on )
{
    yyset_debug ( on ? 1 : 0, sb->scanner );
}

enum yytokentype
SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb )
{
    enum yytokentype ret = ( enum yytokentype ) yylex ( lvalp, llocp, sb -> scanner );
    lvalp -> type       = ret;
    lvalp -> value      = ( ( struct yyguts_t * ) sb -> scanner ) -> yytext_r;
    lvalp -> value_len  = ( ( struct yyguts_t * ) sb -> scanner ) -> yyleng_r;
    lvalp -> leading_ws = sb -> whitespace; /* will be freed by the new owner */
    lvalp -> file       = sb -> file_name;
    lvalp -> line       = llocp -> first_line;
    lvalp -> column     = llocp -> first_column;
    lvalp -> subtree    = NULL;

    sb -> whitespace = NULL;

    return ret;
}

void
AddWhitespace ( SchemaScanBlock* p_sb, const char* p_ws, size_t p_length )
{
    if ( p_sb -> whitespace == NULL )
    {
        p_sb -> whitespace = malloc ( p_length + 1 );
        if ( p_sb -> whitespace != NULL )
        {
            string_copy ( p_sb -> whitespace, p_length + 1, p_ws, p_length ); /* will NUL-terminate */
        }
    }
    else
    {
        size_t old_size = string_size ( p_sb -> whitespace );
        size_t new_size = old_size + p_length + 1;
        p_sb -> whitespace = realloc ( p_sb -> whitespace, new_size );
        if ( p_sb -> whitespace != NULL )
        {
            string_copy ( p_sb -> whitespace + old_size, p_length + 1, p_ws, p_length ); /* will NUL-terminate */
        }
    }
}
