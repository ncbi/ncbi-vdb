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
* Unit tests for Kfg interface
*/

#include <kfg/extern.h>

#include <map>
#include <string>
#include <cstring>
#include <sstream>

#include <ktst/unit_test.hpp>

#include <klib/namelist.h>

using namespace std;

TEST_SUITE(KfgWbTestSuite);

//////////////////////////////////////////// tests for flex-generated scanner
extern "C" {
#include "../libs/kfg/kfg-parse.h"
#include "../libs/kfg/config-tokens.h"
}

// simple symbol table
class SymbolTable : public map<string, string>
{
protected:
    void Add(const char* name, const char* value)
    {
        (*this)[name]=value;
    }

    static bool lookup_var(void * self, KFGToken* pb)
    { 
        const SymbolTable* tab=(const SymbolTable*)self;
        map<string, string>::const_iterator var=tab->find(string(pb->tokenText+2, pb->tokenLength-3));
        if (var == tab->end())
        {
            return false;
        }
        pb->tokenText = var->second.c_str();
        pb->tokenLength = var->second.length();
        pb->tokenId = kfgVAR_REF;
        return true;
    }
};

// test fixture for scanner tests
class KfgScanFixture : public SymbolTable
{
public:
    KfgScanFixture() 
    {
        sb.self=this;
        sb.file="test";
        sb.look_up_var=lookup_var;
        sb.buffer=0;
        sb.scanner=0;
        sb.report_error=report_error;
    }
    ~KfgScanFixture() 
    {
        KFGScan_yylex_destroy(&sb);
    }
    void InitScan(const char* input, bool trace=false)
    {
        KFGScan_yylex_init(&sb, input);
        KFGScan_set_debug(trace, &sb);
    }
    int Scan()
    {
        errorMsg.clear();

        KFGScan_yylex(&sym, &sb);

        const char* token=sb.lastToken->tokenText;
        if (sb.lastToken->tokenId != kfgEND_INPUT && token != 0)
        {
            tokenText=string(token, sb.lastToken->tokenLength);
        }
        else
        {
            tokenText.clear();
        }
        return sym.pb.tokenId;
    }
    static void report_error(KFGScanBlock* sb, const char* msg)
    {
        ((KfgScanFixture*)sb->self)->errorMsg=msg;
    }

    string MatchToken(const char* token, int expectedId, const char* expectedText, int nextToken=kfgEND_INPUT, bool trace=false, bool keepError=true)
    {
        InitScan(token, trace);
        int id=Scan();
        if (id != expectedId)
        {
            ostringstream os;
            os << "Scan() returned " << id << ", expected" << expectedId;
            return os.str();
        }
        if (expectedText == 0)
        {
            expectedText=token;
        }
        if (tokenText != expectedText)
        {
            ostringstream os;
            os << "Scan() returned " << tokenText.c_str() << ", expected" << expectedText;
            return os.str();
        }

        // save the errorMsg returned by Scan() from being overwritten, in case the test case is interested in checking it rather than the next one.
        string errorMsgSaved=errorMsg;
        id=Scan();
        if (id != nextToken)
        {
            ostringstream os;
            os << "post-Scan() returned " << id << ", expected" << nextToken;
            return os.str();
        }
        if (keepError)
        {
            errorMsg=errorMsgSaved;
        }
        return "";
    }
    string MatchToken(const char* token, int expectedId, bool trace=false)
    {
        return MatchToken(token, expectedId, 0, kfgEND_INPUT, trace);
    }

    KFGSymbol sym;
    KFGScanBlock sb;
    string tokenText;
    string errorMsg;
};

FIXTURE_TEST_CASE(KfgScanMultiLineComments, KfgScanFixture)
{   // C comments with an \n inside are recognized as EOL
    InitScan("/***\n\n\n*/");
    REQUIRE_EQUAL(Scan(), (int)kfgEND_LINE);
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanLineComments, KfgScanFixture)
{
    InitScan(
        "/*********/\n"
        "#some more stuff\n"
        "/*========*/");
    REQUIRE_EQUAL(Scan(), (int)kfgEND_LINE); // C comments without \n inside are skipped
    REQUIRE_EQUAL(Scan(), (int)kfgEND_LINE); // line comments skipped
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);// not folowed by /n
}

FIXTURE_TEST_CASE(KfgScanNestedMultiLineComments, KfgScanFixture)
{   
    InitScan("/***\n"
             "/**/\n" // the comment ends here
             "*/");
    REQUIRE_EQUAL(Scan(), (int)kfgEND_LINE);
    REQUIRE_EQUAL(Scan(), (int)kfgEND_LINE);
    REQUIRE_EQUAL(Scan(), (int)kfgUNRECOGNIZED);
}

FIXTURE_TEST_CASE(KfgScanIgnoredSymbols, KfgScanFixture)
{
    InitScan("\v\r\t\f");
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanEndOfLine, KfgScanFixture)
{
    InitScan("\v\r\t\f\n");
    REQUIRE_EQUAL(Scan(), (int)kfgEND_LINE);
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanStrings, KfgScanFixture)
{   // strings with white space in between
    InitScan("'str1' \"str2\"");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING);          REQUIRE_EQUAL(tokenText, std::string("str1"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING);          REQUIRE_EQUAL(tokenText, std::string("str2"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
} 

FIXTURE_TEST_CASE(KfgScanString1, KfgScanFixture)
{   // ( in string
    REQUIRE_EQ(MatchToken("'str1('", kfgSTRING, "str1("), string());
}
FIXTURE_TEST_CASE(KfgScanString2, KfgScanFixture)
{   // $ in string
    REQUIRE_EQ(MatchToken("'a$str2\\\"'", kfgESCAPED_STRING, "a$str2\\\""), string());
}
FIXTURE_TEST_CASE(KfgScanString3, KfgScanFixture)
{   // ) in string
    REQUIRE_EQ(MatchToken("\"str)3\"", kfgSTRING, "str)3"), string());
}
FIXTURE_TEST_CASE(KfgScanString4, KfgScanFixture)
{   // escaped \n in string
    REQUIRE_EQ(MatchToken("\"str4\\n'\"", kfgESCAPED_STRING, "str4\\n'"), string());
}
FIXTURE_TEST_CASE(KfgScanString5, KfgScanFixture)
{   // escaped \" in string
    REQUIRE_EQ(MatchToken("\"\\\"\"", kfgESCAPED_STRING, "\\\""), string());
}

FIXTURE_TEST_CASE(KfgScanVarRefInStringEscapes1, KfgScanFixture)
{   // escaped $ in string, case 1
    REQUIRE_EQ(MatchToken("'\\$'", kfgESCAPED_STRING, "\\$"), string());
}
FIXTURE_TEST_CASE(KfgScanVarRefInStringEscapes2, KfgScanFixture)
{   // escaped $ in string, case 2
    REQUIRE_EQ(MatchToken("\"$\\$1\"", kfgESCAPED_STRING, "$\\$1"), string());
}
FIXTURE_TEST_CASE(KfgScanVarRefInStringEscapes3, KfgScanFixture)
{   // escaped $( in string
    REQUIRE_EQ(MatchToken("\"$\\$(ref)\"", kfgESCAPED_STRING, "$\\$(ref)"), string());
}
FIXTURE_TEST_CASE(KfgScanVarRefInStringEscapes4, KfgScanFixture)  
{   // unterminated $( in double quoted string
    REQUIRE_EQ(MatchToken("\"$(qq\"qq", kfgSTRING, "", kfgSTRING), string());
    REQUIRE_EQUAL(tokenText, string("$(qq\"qq"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}
FIXTURE_TEST_CASE(KfgScanVarRefInStringEscapes5, KfgScanFixture)  
{   // unterminated $( in single quoted string
    REQUIRE_EQ(MatchToken("'$(qq'qq", kfgSTRING, "", kfgSTRING), string());
    REQUIRE_EQUAL(tokenText, string("$(qq\'qq"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}
FIXTURE_TEST_CASE(KfgScanVarRefInStringEscapes6, KfgScanFixture)  
{   // unterminated $( eats up the rest of the line
    REQUIRE_EQ(MatchToken("'$(qq zzz", kfgSTRING, "", kfgSTRING), string());
    REQUIRE_EQUAL(tokenText, string("$(qq zzz"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

// EOL in strings
FIXTURE_TEST_CASE(KfgScanUnterminatedString1, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("'st\n", kfgUNTERM_STRING, "st", kfgEND_LINE), string(""));
    REQUIRE(errorMsg.length() > 0);
}
FIXTURE_TEST_CASE(KfgScanUnterminatedString2, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("\"st\n", kfgUNTERM_STRING, "st", kfgEND_LINE), string(""));
    REQUIRE(errorMsg.length() > 0);
}
FIXTURE_TEST_CASE(KfgScanUnterminatedString3, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("'s\\t\n", kfgUNTERM_ESCAPED_STRING, "s\\t", kfgEND_LINE), string(""));
    REQUIRE(errorMsg.length() > 0);
}
FIXTURE_TEST_CASE(KfgScanUnterminatedString4, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("\"s\\t\n", kfgUNTERM_ESCAPED_STRING, "s\\t", kfgEND_LINE), string(""));
    REQUIRE(errorMsg.length() > 0);
}
// EOF in strings
FIXTURE_TEST_CASE(KfgScanUnterminatedString5, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("'st", kfgUNTERM_STRING, "st"), string(""));
    REQUIRE(errorMsg.length() > 0);
}
FIXTURE_TEST_CASE(KfgScanUnterminatedString6, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("\"st", kfgUNTERM_STRING, "st"), string(""));
    REQUIRE(errorMsg.length() > 0);
}
FIXTURE_TEST_CASE(KfgScanUnterminatedString7, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("'s\\t", kfgUNTERM_ESCAPED_STRING, "s\\t"), string(""));
    REQUIRE(errorMsg.length() > 0);
}
FIXTURE_TEST_CASE(KfgScanUnterminatedString8, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("\"s\\t", kfgUNTERM_ESCAPED_STRING, "s\\t"), string(""));
    REQUIRE(errorMsg.length() > 0);
}

FIXTURE_TEST_CASE(KfgScanSimplePathNames, KfgScanFixture)
{
    InitScan("a _aBc 9-9 _--.* ");
    REQUIRE_EQUAL(Scan(), (int)kfgREL_PATH); REQUIRE_EQUAL(tokenText, string("a"));
    REQUIRE_EQUAL(Scan(), (int)kfgREL_PATH); REQUIRE_EQUAL(tokenText, string("_aBc"));
    REQUIRE_EQUAL(Scan(), (int)kfgREL_PATH); REQUIRE_EQUAL(tokenText, string("9-9"));
    REQUIRE_EQUAL(Scan(), (int)kfgREL_PATH); REQUIRE_EQUAL(tokenText, string("_--."));
    REQUIRE_EQUAL(Scan(), (int)kfgUNRECOGNIZED); 
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanLongPathNames, KfgScanFixture)
{
    InitScan("/a/b/c a/_.- ");
    REQUIRE_EQUAL(Scan(), (int)kfgABS_PATH); REQUIRE_EQUAL(tokenText, string("/a/b/c"));
    REQUIRE_EQUAL(Scan(), (int)kfgREL_PATH); REQUIRE_EQUAL(tokenText, string("a/_.-"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanLiterals, KfgScanFixture)
{
    REQUIRE_EQ(MatchToken("=", kfgASSIGN), string());
}

FIXTURE_TEST_CASE(KfgScanVarRefSimple, KfgScanFixture)
{
    InitScan("$(ref)");
    Add("ref", "value");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanVarRefUndefined, KfgScanFixture)
{
    InitScan("$(reff)");
    Add("ref", "value");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE(errorMsg.length() == 0); // undefined vars not reported
    REQUIRE_EQUAL(tokenText, string(""));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT); 
}

FIXTURE_TEST_CASE(KfgScanVarRefInDoubleString, KfgScanFixture)
{
    InitScan("\"-$(ref)+\"");
    Add("ref", "value");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("-"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("+"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanVarRefInSingleString, KfgScanFixture)
{
    InitScan("'-$(ref)+'");
    Add("ref", "value");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("-"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("+"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanMultipleVarRefsInString1, KfgScanFixture)
{
    InitScan("'-$(ref1)+$(ref2)-'");
    Add("ref1", "value1");
    Add("ref2", "value2");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("-"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value1"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("+"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value2"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("-"));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

FIXTURE_TEST_CASE(KfgScanMultipleVarRefsInString2, KfgScanFixture)
{
    InitScan("'$(ref1)$(ref2)'");
    Add("ref1", "value1");
    Add("ref2", "value2");
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string(""));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value1"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string("value2"));
    REQUIRE_EQUAL(Scan(), (int)kfgSTRING); REQUIRE_EQUAL(tokenText, string(""));
    REQUIRE_EQUAL(Scan(), (int)kfgEND_INPUT);
}

//// bad token cases
FIXTURE_TEST_CASE(KfgScanVarRefEndFile, KfgScanFixture)
{   
    InitScan("$(ref");
    REQUIRE_EQUAL(Scan(), (int)kfgUNRECOGNIZED); 
}

FIXTURE_TEST_CASE(KfgScanVarRefInStringEndFile, KfgScanFixture)
{   // broken var refs inside strings are converted into strings
    REQUIRE_EQ(MatchToken("'$(ref", kfgSTRING, "", kfgSTRING, false, false), string(""));
    REQUIRE_EQUAL(tokenText, string("$(ref"));
    REQUIRE(errorMsg.length() > 0);
}

FIXTURE_TEST_CASE(KfgScanVarRefInStringEndLine, KfgScanFixture)
{   // broken var refs inside strings are converted into strings
    REQUIRE_EQ(MatchToken("\"$(ref\n", kfgSTRING, "", kfgSTRING, false, false), string(""));
    REQUIRE_EQUAL(tokenText, string("$(ref"));
    REQUIRE(errorMsg.length() > 0);
}

//////////////////////////////////////////// tests for bison-generated parser

// test fixture for parser tests
class KfgParseFixture : public SymbolTable
{
public:
    KfgParseFixture() 
    {
    }
    ~KfgParseFixture() 
    {
    }
    rc_t Parse(const char* input)
    {
        KFGParseBlock pb; 
        KFGScanBlock sb;

        KFGScan_yylex_init(&sb, input);

        pb.write_nvp=write_nvp;
        
        sb.self=this;
        sb.file="test";
        sb.look_up_var=lookup_var;
        sb.report_error=report_error;
        errorMsg.clear();
        KFG_parse(&pb, &sb);

        KFGScan_yylex_destroy(&sb);

        return 0;
    }

    static rc_t write_nvp(void * cfg, const char* name, size_t nameLen, VNamelist* values)
    {
        uint32_t count;
        rc_t rc=VNameListCount(values, &count);
        if (rc != 0)
        {
            return rc;
        }
        string value;
        for (uint32_t i=0; i < count; ++i)
        {
            const char* val;
            rc=VNameListGet(values, i, &val);
            if (rc != 0)
            {
                return rc;
            }
            value+=string(val, strlen(val));
        }

        KfgParseFixture* config=(KfgParseFixture*)cfg;
        (*config)[string(name, nameLen)] = value;

        return 0;
    }

    static void report_error(KFGScanBlock* sb, const char* msg)
    {
        KfgParseFixture* obj=((KfgParseFixture*)sb->self);
        obj->errorMsg   = msg;
        obj->errorLine  = sb->lastToken->line_no;
        obj->errorCol   = sb->lastToken->column_no;
        const char* token=sb->lastToken->tokenText;
        obj->errorToken =string(token, sb->lastToken->tokenLength);
    }

    size_t errorLine;
    size_t errorCol;
    string errorToken;
    string errorMsg;
};

FIXTURE_TEST_CASE(KfgParseEmptyFile, KfgParseFixture)
{
    REQUIRE_RC(Parse(""));
    REQUIRE_EQ(size(), (size_t)0);
    REQUIRE_RC(Parse("\n\n"));
    REQUIRE_EQ(size(), (size_t)0);
}

FIXTURE_TEST_CASE(KfgParseOneNameValue, KfgParseFixture)
{
    REQUIRE_RC(Parse("name = \"value\""));  // no \n before EOF
    REQUIRE_EQ(size(), (size_t)1);
    REQUIRE_EQ((*this)["name"], string("value"));
}

FIXTURE_TEST_CASE(KfgParseMultNameValue, KfgParseFixture)
{
    REQUIRE_RC(Parse(
        "name1 = \"value1\"\n"
        "name2 = \"value2\"/**\n\n**/"    // comment with \n inside serves as a line end
        "name3 = \"value3\"\n"
        ));
    REQUIRE_EQ(size(), (size_t)3);
    REQUIRE_EQ((*this)["name1"], string("value1"));
    REQUIRE_EQ((*this)["name2"], string("value2"));
    REQUIRE_EQ((*this)["name3"], string("value3"));
}

FIXTURE_TEST_CASE(KfgParseAbsPath, KfgParseFixture)
{
    REQUIRE_RC(Parse("/id/1name/1 = \"value\"\n")); 
    REQUIRE_EQ(size(), (size_t)1);
    REQUIRE_EQ((*this)["/id/1name/1"], string("value"));
}

FIXTURE_TEST_CASE(KfgParseRelPath, KfgParseFixture)
{
    REQUIRE_RC(Parse("1/name1/id = \"value\"\n")); 
    REQUIRE_EQ(size(), (size_t)1);
    REQUIRE_EQ((*this)["1/name1/id"], string("value"));
}

FIXTURE_TEST_CASE(KfgParseEscString, KfgParseFixture)
{
    REQUIRE_RC(Parse("1/name1/id = \"v\\tlue\"\n")); 
    REQUIRE_EQ(size(), (size_t)1);
    REQUIRE_EQ((*this)["1/name1/id"], string("v\tlue")); 
}

FIXTURE_TEST_CASE(KfgParseVarRefSimple, KfgParseFixture)
{
    REQUIRE_RC(Parse(
        "ref='value'\n"
        "var = $(ref)"
        )); 
    REQUIRE_EQ((*this)["var"], string("value"));
}

FIXTURE_TEST_CASE(KfgParseVarRefSimpleUndefined, KfgParseFixture)
{
    REQUIRE_RC(Parse(
        "ref='value'\n"
        "var1 = $(reff)\n"
        "var2 = $(ref)\n"
        "var3 = $(var1)"
        )); 
    REQUIRE_EQ((*this)["var1"], string("")); 
    REQUIRE_EQ((*this)["var2"], string("value")); // and recovered from
    REQUIRE_EQ((*this)["var3"], string("")); // var1 is usable 
}

FIXTURE_TEST_CASE(KfgParseSyntaxRecovery1, KfgParseFixture)
{
    REQUIRE_RC(Parse(
        "a='q'nn\n" // syntax error (name) and skip to eol
        "name='val'" // this should parse
        )); 
    REQUIRE_EQ(errorLine, (size_t)1); 
    REQUIRE_EQ(errorCol, (size_t)6);  
    REQUIRE_EQ(errorToken, string("nn")); 
    REQUIRE_EQ(errorMsg, string("syntax error")); 
    REQUIRE_EQ((*this)["name"], string("val")); 
}

FIXTURE_TEST_CASE(KfgParseSyntaxRecovery2, KfgParseFixture)
{
    REQUIRE_RC(Parse(
        "a='q'%\n"  // syntax error (unrecognized character) and skip to eol
        "name='val'" // this should parse
        )); 
    REQUIRE_EQ(errorLine, (size_t)1); 
    REQUIRE_EQ(errorCol, (size_t)6);  
    REQUIRE_EQ(errorToken, string("%")); 
    REQUIRE_EQ(errorMsg, string("syntax error")); 
    REQUIRE_EQ((*this)["name"], string("val")); 
}

FIXTURE_TEST_CASE(KfgParseVarRefPath, KfgParseFixture)
{
    REQUIRE_RC(Parse(
        "ref/sub='value'\n"
        "var = $(ref/sub)"
        )); 
    REQUIRE_EQ((*this)["var"], string("value"));
}

//////////////////////////////////////////// Main
extern "C"
{
#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "wb-test-kfg";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc = KfgWbTestSuite(argc, argv);
    return rc;
}

}
