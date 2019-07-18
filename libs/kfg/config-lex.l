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
    #include "kfg-lex.h"
    #include <sysalloc.h>
    #include "kfg-parse.h"
    #include "config-tokens.h"

    #define YYSTYPE KFGToken

    /* code to populate token with internal values */
    #define YY_USER_ACTION              \
        yylval -> tokenText     = yytext;   \
        yylval -> tokenLength   = yyleng;   \
        yylval -> line_no       = yylineno; \
        yylval -> column_no     = yycolumn; \
        yycolumn += yyleng;

    #define REM_LQUOTE \
        ++ yylval -> tokenText; \
        -- yylval -> tokenLength; \
        ++ yylval -> column_no;

    #define REM_RQUOTE \
        -- yylval -> tokenLength;
%}

%option never-interactive nounistd yylineno

%option prefix="vdb_kfg_yy"

/* re-entrant scanner */
%option reentrant

/* bison options */
%option bison-bridge

 /* maintain state stack */
%option stack

 /* let parser handle EOF */
%option noyywrap

 /* start-state for slash-star comments */
%x CMT_SLASH_STAR
%x CMT_MULTI_LINE

 /* states for strings */
%x IN_DBL_STRING
%x IN_SNG_STRING

 /* in-string variable reference */
%x IN_VAR_REF

 /* node/name in a path */
path_node                       [A-Za-z_0-9][-.A-Za-z_0-9]*

pathname    {path_node}(\/{path_node})*

abs_key     \/{pathname}
rel_key     {pathname}

ref_start   \$\(
var_ref     {ref_start}{pathname}\)

sstr_char   ([^\\'\f\r\n$]|$[^(\\])
dstr_char   ([^\\"\f\r\n$]|$[^(\\])

%%
 /* multi-line comments */
\/\*                                            { yy_push_state ( CMT_SLASH_STAR, yyscanner ); }
<CMT_SLASH_STAR,CMT_MULTI_LINE>[^*\n]+          /* ignore */
<CMT_SLASH_STAR,CMT_MULTI_LINE>\*+[^*/\n]+      /* ignore */
<CMT_SLASH_STAR,CMT_MULTI_LINE>\**\n            { BEGIN CMT_MULTI_LINE; }
<CMT_SLASH_STAR>\*+\/                           { yy_pop_state ( yyscanner ); }
<CMT_MULTI_LINE>\*+\/                           { yy_pop_state ( yyscanner ); return kfgEND_LINE; }

 /* line comments */
#.*                                     /* ignore */

 /* ignored tokens */
[ \t\f\v\r]                             /* ignore */

 /* end of line is significant */
\n                                      { return kfgEND_LINE; }

 /* normal, POSIX-style paths */
\/{pathname}                            { return kfgABS_PATH; }
{pathname}                              { return kfgREL_PATH; }

 /* variable references inside strings */
<IN_VAR_REF>{var_ref}                   { yy_pop_state ( yyscanner ); return kfgVAR_REF; }
<IN_VAR_REF>[^)\n]*                     { yy_pop_state ( yyscanner ); return kfgUNTERM_VAR_REF; }
<IN_VAR_REF><<EOF>>                     { yy_pop_state ( yyscanner ); return kfgUNTERM_VAR_REF; }

 /* single quoted strings */
'{sstr_char}*'                          { REM_LQUOTE; REM_RQUOTE; return kfgSTRING; }
'{sstr_char}*                           { REM_LQUOTE; return kfgUNTERM_STRING; }
'(\\.|$\\|$\\$|{sstr_char})+'           { REM_LQUOTE; REM_RQUOTE; return kfgESCAPED_STRING; }
'(\\.|$\\|$\\$|{sstr_char})+            { REM_LQUOTE; return kfgUNTERM_ESCAPED_STRING; }

 /* single quoted strings with variable references*/
'{sstr_char}*/{ref_start}               { REM_LQUOTE; yy_push_state ( IN_SNG_STRING, yyscanner ); yy_push_state ( IN_VAR_REF, yyscanner ); return kfgSTRING; }
'(\\.|{sstr_char})+/{ref_start}         { REM_LQUOTE; yy_push_state ( IN_SNG_STRING, yyscanner ); yy_push_state ( IN_VAR_REF, yyscanner ); return kfgESCAPED_STRING; }

<IN_SNG_STRING>{ref_start}              { yy_push_state ( IN_VAR_REF, yyscanner ); yyless(0); }
<IN_SNG_STRING>{sstr_char}+/{ref_start} { yy_push_state ( IN_VAR_REF, yyscanner ); return kfgSTRING; }
<IN_SNG_STRING>{sstr_char}*'            { REM_RQUOTE; yy_pop_state ( yyscanner ); return kfgSTRING; }

 /* double quoted strings */
\"{dstr_char}*\"                        { REM_LQUOTE; REM_RQUOTE; return kfgSTRING; }
\"{dstr_char}*                          { REM_LQUOTE; return kfgUNTERM_STRING; }
\"(\\.|$\\|$\\$|{dstr_char})+\"         { REM_LQUOTE; REM_RQUOTE; return kfgESCAPED_STRING; }
\"(\\.|$\\|$\\$|{dstr_char})+           { REM_LQUOTE; return kfgUNTERM_ESCAPED_STRING; }

 /* double quoted strings with variable references*/
\"{dstr_char}*/{ref_start}              { REM_LQUOTE; yy_push_state ( IN_DBL_STRING, yyscanner ); yy_push_state ( IN_VAR_REF, yyscanner ); return kfgSTRING; }
\"(\\.|{dstr_char})+/{ref_start}        { REM_LQUOTE; yy_push_state ( IN_DBL_STRING, yyscanner ); yy_push_state ( IN_VAR_REF, yyscanner ); return kfgESCAPED_STRING; }

<IN_DBL_STRING>{ref_start}              { yy_push_state ( IN_VAR_REF, yyscanner ); yyless(0); }
<IN_DBL_STRING>{dstr_char}+/{ref_start} { yy_push_state ( IN_VAR_REF, yyscanner ); return kfgSTRING; }
<IN_DBL_STRING>{dstr_char}*\"           { REM_RQUOTE; yy_pop_state ( yyscanner ); return kfgSTRING; }

 /* literals */
=                                       { return kfgASSIGN; }

{var_ref}                               { return kfgVAR_REF; }

 /* unrecognized input */
<*>.|\n                                 { return kfgUNRECOGNIZED; }

%%

bool KFGScan_yylex_init(KFGScanBlock* sb, const char *str)
{
    if (yylex_init(&sb->scanner) != 0)
    {
        return false; /*RC ( rcKFG, rcMgr, rcParsing, rcMemory, rcExhausted );*/
    }
    sb->buffer=yy_scan_string((yyconst char*)str, sb->scanner);

    {   /* initialize line/col counters */
        /* yylineno and yycolumn are macros using variable yyg, so we have to prepare it for them */
        struct yyguts_t * yyg = (struct yyguts_t*)sb->scanner;
        yylineno=1;
        yycolumn=1;
    }

    return true;
}

void KFGScan_yylex_destroy(KFGScanBlock* sb)
{
    yy_delete_buffer(sb->buffer, sb->scanner);
    if (sb->scanner)
    {
        yylex_destroy(sb->scanner);
    }
    sb->scanner=0;
    sb->buffer=0;
}

void KFGScan_set_debug(bool on, KFGScanBlock* sb)
{
    yyset_debug(on ? 1 : 0, sb->scanner);
}

int KFGScan_yylex(KFGSymbol* sym, KFGScanBlock* sb)
{
    sym->pb.tokenId=yylex(&sym->pb, sb->scanner);
    sb->lastToken= &sym->pb;

    switch (sym->pb.tokenId)
    {
    case kfgVAR_REF:
        if (!sb->look_up_var(sb->self, &sym->pb))
        {
            /* undefined variable reference is handled silently, to behave like a shell script */
            /* sb->report_error(sb, "variable not found");*/
            sym->pb.tokenLength = 0;
        }
        sym->pb.tokenId = kfgSTRING;
        break;
    case kfgUNTERM_VAR_REF: /* convert bad var ref into a string */
        sb->report_error(sb, "unterminated variable reference inside a string (use '\\$(' to avoid this message)");
        sym->pb.tokenId = kfgSTRING;
        break;
    case kfgUNTERM_STRING:
    case kfgUNTERM_ESCAPED_STRING:
        sb->report_error(sb, "unterminated string");
        break;
    default:
        break;
    }

    return sym->pb.tokenId;
}

