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

/**
* Unit tests for schema tokenizer
*/

#include <ktst/unit_test.hpp>

#include "../../libs/schema/SchemaScanner.hpp"

using namespace ncbi::SchemaParser;
typedef struct ParseTree ParseTree; // need this fake to make schema-tokens.h
#include "../../libs/schema/schema-tokens.h"

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaLexTestSuite );

TEST_CASE ( EmptyInput )
{
    REQUIRE_EQ ( (int)END_SOURCE, SchemaScanner ( "" ) . Scan () );
}

TEST_CASE ( Whitspace )
{
    SchemaScanner s ( "    " );
    REQUIRE_EQ ( (int)END_SOURCE, s . Scan () );
    REQUIRE_EQ ( string ( "    " ), string ( s . LastTokenValue () . leading_ws ) );
}

TEST_CASE ( NotNulTerminated )
{
    SchemaScanner s ( "  bad", 2, false ); // only first 2 bytes looked at
    REQUIRE_EQ ( (int)END_SOURCE, s . Scan () );
    REQUIRE_EQ ( string ( "  " ), string ( s . LastTokenValue () . leading_ws ) );
}

TEST_CASE ( Unrecognized )
{
    REQUIRE_EQ ( (int)UNRECOGNIZED, SchemaScanner ( "ъ" ) . Scan () );
}

#define REQUIRE_LITERAL(name, lit) \
    TEST_CASE ( name ) { REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) lit [ 0 ], SchemaScanner ( lit ) . Scan () ); }

REQUIRE_LITERAL ( Semicolon,     ";" )
REQUIRE_LITERAL ( Dollar,        "$" )
REQUIRE_LITERAL ( Comma,         "," )
REQUIRE_LITERAL ( LeftBrace,     "{" )
REQUIRE_LITERAL ( RightBrace,    "}" )
REQUIRE_LITERAL ( Equals,        "=" )
REQUIRE_LITERAL ( LeftParen,     "(" )
REQUIRE_LITERAL ( RightParen,    ")" )
REQUIRE_LITERAL ( Hash,          "#" )
REQUIRE_LITERAL ( LeftAngle,     "<" )
REQUIRE_LITERAL ( RightAngle,    ">" )
REQUIRE_LITERAL ( Asterisk,      "*" )
REQUIRE_LITERAL ( Slash,         "/" )
REQUIRE_LITERAL ( Colon,         ":" )
REQUIRE_LITERAL ( LeftSquare,    "[" )
REQUIRE_LITERAL ( RightSquare,   "]" )
REQUIRE_LITERAL ( Pipe,          "|" )
REQUIRE_LITERAL ( Dor,           "." )
REQUIRE_LITERAL ( Plus,          "+" )
REQUIRE_LITERAL ( Minus,         "-" )
REQUIRE_LITERAL ( At,            "@" )

TEST_CASE ( Ellipsis )
{
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) ELLIPSIS, SchemaScanner ( "..." ) . Scan () );
}

#define REQUIRE_TOKEN(expected, scanner) \
    REQUIRE_EQ ( string ( expected ), string ( scanner . LastTokenValue () . value, scanner . LastTokenValue () . value_len ) )
#define REQUIRE_WS(expected, scanner) \
    REQUIRE_EQ ( string ( expected ), string ( scanner . LastTokenValue () . leading_ws ) )

#define REQUIRE_TERMINAL(name, token, term) \
TEST_CASE ( name ) \
{ \
    SchemaScanner s ( term ); \
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) token, s . Scan () ); \
    REQUIRE_TOKEN ( term, s ); \
}

REQUIRE_TERMINAL ( Decimal,             DECIMAL,        "123456789" )
REQUIRE_TERMINAL ( Octal,               OCTAL,          "01234567" )
REQUIRE_TERMINAL ( Hex_0x,              HEX,            "0x01abCDef23" )
REQUIRE_TERMINAL ( Hex1_0X,             HEX,            "0X01abCDef23" )
REQUIRE_TERMINAL ( Float,               FLOAT,          ".1415" )
REQUIRE_TERMINAL ( Float_NoLeadDigits,  FLOAT,          "3.1415" )
REQUIRE_TERMINAL ( Float_NoTrailDigits, FLOAT,          "3." )
REQUIRE_TERMINAL ( ExpFloat_e,          EXP_FLOAT,      "3.14e1" )
REQUIRE_TERMINAL ( ExpFloat_E,          EXP_FLOAT,      "3.14E1" )
REQUIRE_TERMINAL ( DoubleQuotedString,  STRING,         "\"qwerty\"" )
REQUIRE_TERMINAL ( EscapedDQString,     ESCAPED_STRING, "\"q\\w\\nerty\"" )
REQUIRE_TERMINAL ( SingleQuotedString,  STRING,         "'qwerty'" )
REQUIRE_TERMINAL ( EscapedSQString,     ESCAPED_STRING, "\"q\\w\\nerty\"" )

REQUIRE_TERMINAL ( Identifier_OneChar,  IDENTIFIER_1_0, "T" )
REQUIRE_TERMINAL ( Identifier,          IDENTIFIER_1_0, "abc123_" )
REQUIRE_TERMINAL ( Identifier_StartsWithDigit, IDENTIFIER_1_0,  "1abc123_" ) // "2na" is used in 1.0 schemas
REQUIRE_TERMINAL ( PhysicalIdentifier,  PHYSICAL_IDENTIFIER_1_0, ".T" )

REQUIRE_TERMINAL ( Version_Maj,         VERSION, "#1" )
REQUIRE_TERMINAL ( Version_MajMin,      VERSION, "#1.2" )
REQUIRE_TERMINAL ( Version_MajMinRel,   VERSION, "#1.2.3" )

#define REQUIRE_KEYWORD(word) \
TEST_CASE ( kw_##word ) \
{ \
    SchemaScanner s ( #word ); \
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) KW_##word, s . Scan () ); \
    REQUIRE_TOKEN ( #word, s ); \
}

REQUIRE_KEYWORD(__no_header)
REQUIRE_KEYWORD(__row_length)
REQUIRE_KEYWORD(__untyped)
REQUIRE_KEYWORD(alias)
REQUIRE_KEYWORD(column)
REQUIRE_KEYWORD(const)
REQUIRE_KEYWORD(control)
REQUIRE_KEYWORD(database)
REQUIRE_KEYWORD(decode)
REQUIRE_KEYWORD(default)
REQUIRE_KEYWORD(encode)
REQUIRE_KEYWORD(extern)
REQUIRE_KEYWORD(false)
REQUIRE_KEYWORD(fmtdef)
REQUIRE_KEYWORD(function)
REQUIRE_KEYWORD(include)
REQUIRE_KEYWORD(limit)
REQUIRE_KEYWORD(physical)
REQUIRE_KEYWORD(read)
REQUIRE_KEYWORD(readonly)
REQUIRE_KEYWORD(return)
REQUIRE_KEYWORD(schema)
REQUIRE_KEYWORD(static)
REQUIRE_KEYWORD(table)
REQUIRE_KEYWORD(template)
REQUIRE_KEYWORD(trigger)
REQUIRE_KEYWORD(true)
REQUIRE_KEYWORD(type)
REQUIRE_KEYWORD(typedef)
REQUIRE_KEYWORD(typeset)
REQUIRE_KEYWORD(validate)
REQUIRE_KEYWORD(version)
REQUIRE_KEYWORD(view)
REQUIRE_KEYWORD(virtual)
REQUIRE_KEYWORD(void)
REQUIRE_KEYWORD(write)

TEST_CASE ( Comment )
{
    SchemaScanner s ( "/**/abc" );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) IDENTIFIER_1_0, s . Scan () );
    REQUIRE_TOKEN ( "abc", s );
    REQUIRE_WS ( "/**/", s );
}

TEST_CASE ( LineComment )
{
    SchemaScanner s ( "//qed\nabc" );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) IDENTIFIER_1_0, s . Scan () );
    REQUIRE_TOKEN ( "abc", s ); \
    REQUIRE_WS ( "//qed\n", s );
}

TEST_CASE ( MultiLineComment )
{
    SchemaScanner s ( "/*\n\n*/abc" );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) IDENTIFIER_1_0, s . Scan () );
    REQUIRE_TOKEN ( "abc", s ); \
    REQUIRE_WS ( "/*\n\n*/", s );
}

TEST_CASE ( WhiteSpace )
{
    SchemaScanner s ( " \t\f\v\r\nabc \t\f\v\r\n" );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) IDENTIFIER_1_0, s . Scan () );
    REQUIRE_TOKEN ( "abc", s ); \
}

TEST_CASE ( VERS_1 )
{
    SchemaScanner s ( "version 1;" );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) KW_version, s . Scan () );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) VERS_1_0, s . Scan () );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) ';', s . Scan () );
}

TEST_CASE ( VERS_1_comment )
{
    SchemaScanner s ( "version /*!!*/ 1;" );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) KW_version, s . Scan () );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) VERS_1_0, s . Scan () );
    REQUIRE_EQ ( ( SchemaScanner ::  TokenType ) ';', s . Scan () );
}

//TODO: unterminated strings

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/out.h>

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

const char UsageDefaultName[] = "wb-test-schema-lex";

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ( "Usage:\n" "\t%s [options] -o path\n\n", progname );
}

rc_t CC Usage( const Args* args )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    return SchemaLexTestSuite(argc, argv);
}

}

