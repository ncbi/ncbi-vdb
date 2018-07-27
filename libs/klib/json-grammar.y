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
    #include "json-priv.h"

    #include <klib/text.h>
    #include <klib/printf.h>

    #define Json_lex JsonScan_yylex

    extern enum yytokentype JsonScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, JsonScanBlock* sb );

    void Json_error ( YYLTYPE *                 p_llocp,
                      struct KJsonObject **     p_root,
                      struct JsonScanBlock *    p_sb,
                      const char *              p_msg )
    {
        p_sb -> error = string_dup ( p_msg, string_size ( p_msg ) );
    }
    void Json_RC ( YYLTYPE *                 p_llocp,
                   struct KJsonObject **     p_root,
                   struct JsonScanBlock *    p_sb,
                   rc_t                      p_rc )
    {
        char msg [ 1024 ];
        string_printf ( msg, sizeof ( msg ), NULL, "line %u, col %u: %R", p_llocp->last_line, p_llocp -> last_column, p_rc );
        p_sb -> error = string_dup ( msg, string_size ( msg ) );
    }

#define CHECK_RC( call ) \
{\
    rc_t rc = ( call );\
    if ( rc != 0 )\
    {\
        Json_RC ( & yyloc, root, sb, rc );\
        YYERROR;\
    }\
}

%}

%name-prefix "Json_"
%parse-param { struct KJsonObject ** root }
%param { struct JsonScanBlock* sb }

%define api.value.type {JsonToken}

%define parse.error verbose
%locations

%define api.pure full

%token END_SOURCE   0 "end of source"
%token UNRECOGNIZED "unrecognized token"
%token jsonSTRING   "string"
%token jsonNUMBER   "number"
%token jsonTRUE     "true"
%token jsonFALSE    "false"
%token jsonNULL     "null"

%start parse

%%

parse
    : object END_SOURCE { * root = ( KJsonObject * ) KJsonValueToObject ( $1 . node ); return 0; }
    ;

object
    : '{' '}'
        {
            KJsonObject * obj;
            CHECK_RC ( KJsonMakeObject ( & obj ) );
            $$ . node = ( KJsonValue *) KJsonObjectToValue ( obj );
        }
    | '{' members '}'
        { $$ = $2; }
    ;

members
    : jsonSTRING ':' value
        {
            KJsonObject * obj;
            CHECK_RC ( KJsonMakeObject ( & obj ) );
            CHECK_RC ( KJsonObjectAddMember ( obj, $1 . value, $1 . value_len, $3 . node ) );
            $$ . node = ( KJsonValue *) KJsonObjectToValue ( obj );
        }
    | members ',' jsonSTRING ':' value
        {
            KJsonObject * obj = ( KJsonObject * ) KJsonValueToObject ( $1 . node );
            CHECK_RC ( KJsonObjectAddMember ( obj, $3 . value, $3 . value_len, $5 . node ) );
            $$ = $1;
        }
    ;

value
    : jsonSTRING    { CHECK_RC ( KJsonMakeString ( & $$ . node, $1 . value, $1 . value_len ) ); }
    | jsonNUMBER
    | object        { $$ = $1; }
    | array         { $$ = $1; }
    | jsonTRUE
    | jsonFALSE
    | jsonNULL      { CHECK_RC ( KJsonMakeNull ( & $$ . node ) ); }
    ;

array
    : '[' ']'
        {
            KJsonArray * arr;
            CHECK_RC ( KJsonMakeArray ( & arr ) );
            $$ . node = ( KJsonValue *) KJsonArrayToValue ( arr );
        }
    | '[' elements ']' { $$ = $2; }
    ;

elements
    : value
        {
            KJsonArray * arr;
            CHECK_RC ( KJsonMakeArray ( & arr ) );
            CHECK_RC ( KJsonArrayAddElement ( arr, $1 . node ) );
            $$ . node = ( KJsonValue *) KJsonArrayToValue ( arr );
        }
    | elements ',' value
        {
            KJsonArray * arr = ( KJsonArray * ) KJsonValueToArray ( $1 . node );
            CHECK_RC ( KJsonArrayAddElement ( arr, $3 . node ) );
            $$ = $1;
        }
    ;
