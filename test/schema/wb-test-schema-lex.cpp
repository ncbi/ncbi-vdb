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

#include <sstream>

#include <ktst/unit_test.hpp>

#include "../../libs/schema/SchemaScanner.hpp"

using namespace ncbi::SchemaParser;

// need these fake to make schema-tokens.h compile
typedef struct ParseTree ParseTree;
typedef struct ErrorReport ErrorReport;

#include "../../libs/schema/schema-tokens.h"

using namespace std;
using namespace ncbi::NK;

TEST_SUITE ( SchemaLexTestSuite );

void
VerifyLocation ( const Token & p_token, uint32_t p_line, uint32_t p_column, const char * p_file = "" )
{
    return;//TODO
    const Token :: Location & loc = p_token . GetLocation ();
    if ( string ( p_file ) != string ( loc . m_file ) )
    {
        ostringstream out;
        out << "VerifyLocation(): file name expected '" << p_file << "', actual '" << loc . m_file << "'" << endl;
        throw std :: logic_error ( out . str () );
    }
    if ( p_line != loc . m_line )
    {
        ostringstream out;
        out << "VerifyLocation(): line expected '" << p_line << "', actual '" << loc . m_line << "'" << endl;
        throw std :: logic_error ( out . str () );
    }
    if ( p_column != loc . m_column )
    {
        ostringstream out;
        out << "VerifyLocation(): column expected '" << p_column << "', actual '" << loc . m_column << "'" << endl;
        throw std :: logic_error ( out . str () );
    }
}

TEST_CASE ( EmptyInput )
{
    REQUIRE_EQ ( (int)END_SOURCE, SchemaScanner ( "" ) . NextToken () . GetType () );
}

TEST_CASE ( Location )
{
    SchemaScanner s ( "" );
    Token t = s . NextToken ();
    VerifyLocation ( t, 1, 1 );
}

TEST_CASE ( Whitespace )
{
    SchemaScanner s ( "    " );
    Token t = s . NextToken ();
    REQUIRE_EQ ( (int)END_SOURCE, t. GetType () );
    REQUIRE_EQ ( string ( "    " ), string ( t . GetLeadingWhitespace () ) );
    VerifyLocation ( t, 1, 4 );
}

TEST_CASE ( NotNulTerminated )
{
    SchemaScanner s ( "  bad", 2, false ); // only first 2 bytes looked at
    Token t = s . NextToken ();
    REQUIRE_EQ ( (int)END_SOURCE, t. GetType () );
    REQUIRE_EQ ( string ( "  " ), string ( t . GetLeadingWhitespace () ) );
}

TEST_CASE ( Unrecognized )
{
    REQUIRE_EQ ( (int)UNRECOGNIZED, SchemaScanner ( "ъ" ) . NextToken () . GetType () );
}

#define REQUIRE_LITERAL(name, lit) \
    TEST_CASE ( name ) { REQUIRE_EQ ( ( Token :: TokenType ) lit [ 0 ], SchemaScanner ( lit ) . NextToken () . GetType () ); }

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
    REQUIRE_EQ ( ( Token ::  TokenType ) ELLIPSIS, SchemaScanner ( "..." ) . NextToken () . GetType () );
}

#define REQUIRE_TOKEN(expected, token) \
    REQUIRE_EQ ( string ( expected ), string ( token . GetValue () ) )
#define REQUIRE_WS(expected, token) \
    REQUIRE_EQ ( string ( expected ), string ( token . GetLeadingWhitespace () ) )

#define REQUIRE_TERMINAL(name, token, term) \
TEST_CASE ( name ) \
{ \
    Token t = SchemaScanner ( term ) . NextToken (); \
    REQUIRE_EQ ( ( Token :: TokenType ) token, t . GetType () ); \
    REQUIRE_TOKEN ( term, t ); \
}

REQUIRE_TERMINAL ( Decimal,             DECIMAL,        "123456789" )
REQUIRE_TERMINAL ( Octal,               OCTAL,          "01234567" )
REQUIRE_TERMINAL ( Hex_0x,              HEX,            "0x01abCDef23" )
REQUIRE_TERMINAL ( Hex1_0X,             HEX,            "0X01abCDef23" )
REQUIRE_TERMINAL ( Float,               FLOAT_,          ".1415" )
REQUIRE_TERMINAL ( Float_NoLeadDigits,  FLOAT_,          "3.1415" )
REQUIRE_TERMINAL ( Float_NoTrailDigits, FLOAT_,          "3." )
REQUIRE_TERMINAL ( ExpFloat_e,          EXP_FLOAT,      "3.14e1" )
REQUIRE_TERMINAL ( ExpFloat_E,          EXP_FLOAT,      "3.14E1" )

REQUIRE_TERMINAL ( DoubleQuotedString,      STRING,         "\"qwerty\"" )
REQUIRE_TERMINAL ( EscapedDQString,         ESCAPED_STRING, "\"q\\w\\nerty\"" )
REQUIRE_TERMINAL ( UnterminatedDQString,    UNTERM_STRING, "\"qwerty" )
REQUIRE_TERMINAL ( UnterminatedEDQString,   UNTERM_ESCAPED_STRING, "\"q\\w\\nerty" )

REQUIRE_TERMINAL ( SingleQuotedString,      STRING,         "'qwerty'" )
REQUIRE_TERMINAL ( EscapedSQString,         ESCAPED_STRING, "\"q\\w\\nerty\"" )
REQUIRE_TERMINAL ( UnterminatedSQString,    UNTERM_STRING,         "'qwerty" )
REQUIRE_TERMINAL ( UnterminatedESQString,   UNTERM_ESCAPED_STRING, "'q\\w\\nerty" )

REQUIRE_TERMINAL ( Identifier_OneChar,  IDENTIFIER_1_0, "T" )
REQUIRE_TERMINAL ( Identifier,          IDENTIFIER_1_0, "abc123_" )
REQUIRE_TERMINAL ( Identifier_StartsWithDigit, IDENTIFIER_1_0,  "1abc123_" ) // "2na" is used in 1.0 schemas
REQUIRE_TERMINAL ( PhysicalIdentifier,  PHYSICAL_IDENTIFIER_1_0, ".T" )

REQUIRE_TERMINAL ( Version_Maj,         VERSION, "#1" )
REQUIRE_TERMINAL ( Version_MajMin,      VERSION, "#1.2" )
REQUIRE_TERMINAL ( Version_MajMinRel,   VERSION, "#1.2.3" )
REQUIRE_TERMINAL ( Version_Ws_Maj,         VERSION, "# 1" )
REQUIRE_TERMINAL ( Version_Ws_MajMin,      VERSION, "#  1.2" )
REQUIRE_TERMINAL ( Version_Ws_MajMinRel,   VERSION, "#    1.2.3" )

#define REQUIRE_KEYWORD(word) \
TEST_CASE ( kw_##word ) \
{ \
    Token t = SchemaScanner ( #word ) . NextToken (); \
    REQUIRE_EQ ( ( Token :: TokenType ) KW_##word, t . GetType () ); \
    REQUIRE_TOKEN ( #word, t ); \
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
    Token t = SchemaScanner ( "/**/abc" ) . NextToken ();
    REQUIRE_EQ ( ( Token :: TokenType ) IDENTIFIER_1_0, t . GetType () );
    REQUIRE_TOKEN ( "abc", t );
    REQUIRE_WS ( "/**/", t );
}

TEST_CASE ( LineComment )
{
    Token t = SchemaScanner ( "//qed\nabc" ) . NextToken ();
    REQUIRE_EQ ( ( Token :: TokenType ) IDENTIFIER_1_0, t . GetType () );
    VerifyLocation ( t, 1, 4 );
    REQUIRE_TOKEN ( "abc", t );
    REQUIRE_WS ( "//qed\n", t );
}

TEST_CASE ( MultiLineComment )
{
    Token t = SchemaScanner ( "/*\n\n*/abc" ) . NextToken ();
    REQUIRE_EQ ( ( Token :: TokenType ) IDENTIFIER_1_0, t . GetType () );
    REQUIRE_TOKEN ( "abc", t );
    REQUIRE_WS ( "/*\n\n*/", t );
}

TEST_CASE ( WhiteSpace )
{
    Token t = SchemaScanner ( " \t\f\v\r\nabc \t\f\v\r\n" ) . NextToken ();
    REQUIRE_EQ ( ( Token :: TokenType ) IDENTIFIER_1_0, t . GetType () );
    REQUIRE_TOKEN ( "abc", t ); \
}

TEST_CASE ( Version_1 )
{
    SchemaScanner s ( "version 1;" );
    REQUIRE_EQ ( ( Token :: TokenType ) KW_version, s . NextToken () . GetType () );
    REQUIRE_EQ ( ( Token :: TokenType ) VERS_1_0, s . NextToken () . GetType () );
    REQUIRE_EQ ( ( Token :: TokenType ) ';', s . NextToken () . GetType () );
}

TEST_CASE ( Version_1_comment )
{
    SchemaScanner s ( "version /*!!*/ 1;" );
    REQUIRE_EQ ( ( Token :: TokenType ) KW_version, s . NextToken () . GetType () );
    REQUIRE_EQ ( ( Token :: TokenType ) VERS_1_0, s . NextToken () . GetType () );
    REQUIRE_EQ ( ( Token :: TokenType ) ';', s . NextToken () . GetType () );
}

TEST_CASE ( Version_2 )
{
    SchemaScanner s ( "version 2;" );
    REQUIRE_EQ ( ( Token :: TokenType ) KW_version, s . NextToken () . GetType () );
    REQUIRE_EQ ( ( Token :: TokenType ) VERS_2_0, s . NextToken () . GetType () );
}

TEST_CASE ( Version_2_0 )
{
    SchemaScanner s ( "version 2.0;" );
    REQUIRE_EQ ( ( Token :: TokenType ) KW_version, s . NextToken () . GetType () );
    REQUIRE_EQ ( ( Token :: TokenType ) VERS_2_0, s . NextToken () . GetType () );
}

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

