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
* Unit tests for schema parser
*/

#include <ktst/unit_test.hpp>

#include <sstream>
#include <fstream>
#include <cstring>

#include <kfs/directory.h>
#include <kfs/file.h>

#include "../../libs/schema/SchemaParser.hpp"
#include "../../libs/schema/ParseTree.hpp"

using namespace ncbi::SchemaParser;
#include "../../libs/schema/schema-tokens.h"

using namespace std;
using namespace ncbi::NK;

const string DataDir = "./actual";

TEST_SUITE ( SchemaParserTestSuite );

#define KW_TOKEN(v,k) Token v ( KW_##k, #k)

KW_TOKEN( st  ,virtual );
KW_TOKEN( st0 ,void );
KW_TOKEN( st1 ,write );
KW_TOKEN( st2 ,version );
KW_TOKEN( st3 ,include );

// Token
TEST_CASE(Token_Construct)
{
    REQUIRE_EQ ( ( int ) KW_virtual, st . GetType() );
    REQUIRE_EQ ( string ( "virtual" ), string ( st . GetValue() ) );
    REQUIRE ( ! st . IsFake () );
}

TEST_CASE(Token_Construct_WithLocation)
{
    Token :: Location loc ("qq", 1, 2 );
    Token t ( KW_virtual, "virtual", loc );
    REQUIRE_EQ ( string ( loc . m_file ), string ( t . GetLocation () . m_file ) );
    REQUIRE_EQ ( loc . m_line, t . GetLocation () . m_line );
    REQUIRE_EQ ( loc . m_column, t . GetLocation () . m_column );
}

TEST_CASE(Token_Construct_Fake)
{
    Token t ( 42 );
    REQUIRE ( t . IsFake () );
}

// ParseTree
bool
operator == ( const Token & l, const Token & r)
{
    return l . GetType() == r . GetType() && string ( l . GetValue() ) == string ( r . GetValue() );
}

TEST_CASE(ParseTree_Construct)
{
    ParseTree t ( st );
    REQUIRE ( t . GetToken() == Token ( st ) );
}

TEST_CASE(ParseTree_AddChild)
{
    ParseTree t = Token ( PT_PARSE );
    t. AddChild ( new ParseTree ( st0 ) );
    t. AddChild ( new ParseTree ( st1 ) );

    REQUIRE ( t . GetChild ( 0 ) -> GetToken () == Token ( st0 ) );
    REQUIRE ( t . GetChild ( 1 ) -> GetToken () == Token ( st1 ) );
}
TEST_CASE(ParseTree_ChildrenCount)
{
    ParseTree t = Token ( PT_PARSE );
    t. AddChild ( new ParseTree ( st0 ) );
    t. AddChild ( new ParseTree ( st1 ) );

    REQUIRE_EQ ( 2u, t . ChildrenCount () );
}

TEST_CASE(ParseTree_MoveChildren)
{
    ParseTree source = Token ( PT_PARSE );
    source. AddChild ( new ParseTree ( st0 ) );
    source. AddChild ( new ParseTree ( st1 ) );

    class TestTree : public ParseTree
    {
    public:
        using ParseTree :: MoveChildren;
        TestTree(const Token & p_t) : ParseTree( p_t ) {};
    } target ( st );

    target . MoveChildren ( source );

    REQUIRE_EQ ( 0u, source . ChildrenCount () );

    REQUIRE_EQ ( 2u, target . ChildrenCount () );
    REQUIRE ( target . GetChild ( 0 ) -> GetToken () == Token ( st0 ) );
    REQUIRE ( target . GetChild ( 1 ) -> GetToken () == Token ( st1 ) );
}

TEST_CASE(ParseTree_Location)
{
    // start with a fake token, its empty location is used as the tree's location
    ParseTree source = Token ( PT_PARSE );
    REQUIRE_EQ ( string (), string ( source . GetLocation() . m_file ) );
    REQUIRE_EQ ( 0u, source . GetLocation() . m_line );
    REQUIRE_EQ ( 0u, source . GetLocation() . m_column );

    // add a real token, make sure its location is used as the tree's location
    SchemaToken st1 = { KW_view, "view", 4, 0, 0, "file", 1, 2 };
    source . AddChild ( new ParseTree ( st1 ) );
    REQUIRE_EQ ( string ( "file" ), string ( source . GetLocation() . m_file ) ); // first real token
    REQUIRE_EQ ( 1u, source . GetLocation() . m_line );
    REQUIRE_EQ ( 2u, source . GetLocation() . m_column );

    // add more real tokens, make sure the location does not change
    SchemaToken st2 = { KW_view, "view", 4, 0, 0, "file", 2, 3 };
    source . AddChild ( new ParseTree ( st2 ) );
    REQUIRE_EQ ( 1u, source . GetLocation() . m_line );
    REQUIRE_EQ ( 2u, source . GetLocation() . m_column );
}

// SchemaParser

TEST_CASE(SchemaParser_ParseString)
{
    SchemaParser p;
    REQUIRE ( p . ParseString ( "" ) );
    REQUIRE_NOT_NULL ( p . GetParseTree () );
}

TEST_CASE(SchemaParser_ParseFile)
{
    const string FileName = DataDir + "/" + GetName ();
    {
        ofstream out ( FileName . c_str () );
        out << " ";
    }

    const KFile * f;
    KDirectory *wd;
    REQUIRE_RC ( KDirectoryNativeDir ( & wd ) );
    REQUIRE_RC ( KDirectoryOpenFileRead ( wd, & f, "%s", FileName . c_str () ) );
    KDirectoryRelease ( wd );

    SchemaParser p;
    REQUIRE ( p . ParseFile ( f ) );
    REQUIRE_NOT_NULL ( p . GetParseTree () );
    KFileRelease ( f );
}

TEST_CASE(SchemaParser_MoveParseTree)
{
    SchemaParser p;
    p . ParseString ( "" );
    ParseTree * root = p . MoveParseTree ();
    REQUIRE_NULL ( p . GetParseTree () );
    delete root;
}

// Grammar

static
void
PrintParseTree ( const ParseTree * p_t, ostream& p_out )
{
    if ( p_t == 0 )
    {
        p_out << " NULL";
        return;
    }

    p_out << p_t -> GetToken () . GetLeadingWhitespace () << p_t -> GetToken () . GetValue ();

    for ( uint32_t i = 0 ; i < p_t -> ChildrenCount (); ++ i )
    {
        PrintParseTree ( p_t -> GetChild ( i ), p_out );
    }
}

static
void
ParseAndVerify ( const char* p_source, bool p_debug = false )
{
    SchemaParser p;
    if ( ! p . ParseString ( p_source, p_debug ) )
    {
        throw logic_error ( string ( "ParseAndVerify : ParseString() failed: " ) + p . GetErrors () . GetMessageText ( 0 ) );
    }
    ostringstream str;
    PrintParseTree ( p . GetParseTree (), str );
    if ( str . str () != p_source )
    {
        cout << "expected: '" << p_source << "'" << endl
             << "actual:   '" << str . str () << "'" << endl;
        throw logic_error ( "ParseAndVerify failed: " );
    }
}

TEST_CASE ( EmptyInput )
{
    ParseAndVerify ( "" );
}

TEST_CASE ( Version1 )
{
    ParseAndVerify ( "version 1; include \"qq\";" );
}

TEST_CASE ( MultipleDecls )
{
    ParseAndVerify ( "version 1; include \"qq\"; include \"aa\";" );
}

TEST_CASE ( Typedef )
{
    ParseAndVerify ( "typedef ns:oldName newName;" );
}
TEST_CASE ( Keywords_as_identifiers )
{
    ParseAndVerify (
        "typedef ns:database:decode:encode:read:table:type:view:write newName;" );
}

TEST_CASE ( TypedefDim )
{
    ParseAndVerify ( "typedef oldName newName [ 12 ];" );
}
TEST_CASE ( TypedefVarDim )
{
    ParseAndVerify ( "typedef oldName newName [ * ];" );
}

TEST_CASE ( TypedefMultipleNames )
{
    ParseAndVerify ( "typedef oldName neawName1, newName2 [ 12 ], newName3;" );
}

TEST_CASE ( Typeset )
{
    ParseAndVerify ( "typeset newName { a, b[1], c };" );
}

TEST_CASE ( Format )
{
    ParseAndVerify ( "fmtdef newName;" );
}
TEST_CASE ( FormatRename )
{
    ParseAndVerify ( "fmtdef oldName newName;" );
}

TEST_CASE ( Const )
{
    ParseAndVerify ( "const t c = 1;" );
}
TEST_CASE ( ConstDim )
{
    ParseAndVerify ( "const t[2] c = 1;" );
}

TEST_CASE ( Alias )
{
    ParseAndVerify ( "alias a b;" );
}

TEST_CASE ( Extern_AndUntyped )
{
    ParseAndVerify ( "extern function __untyped fn ();" );
}

TEST_CASE ( Function_RowLength )
{
    ParseAndVerify ( "function __row_length fn ();" );
}

TEST_CASE ( Function_Naked )
{
    ParseAndVerify ( "function t fn ( a b );" );
}

TEST_CASE ( Function_ArrayReturn )
{
    ParseAndVerify ( "function t[3] fn ( a b );" );
}

TEST_CASE ( Function_Schema )
{
    ParseAndVerify ( "function < type b,  a / fmt c > t fn ( a b );" );
}

TEST_CASE ( Function_Factory )
{
    ParseAndVerify ( "function t fn < a b, c d > ( a b );" );
}

TEST_CASE ( Function_NoFormals )
{
    ParseAndVerify ( "function t fn ();" );
}

TEST_CASE ( Function_Formals_OptionalOnly )
{
    ParseAndVerify ( "function t fn ( * a b );" );
}
TEST_CASE ( Function_Formals_MandatoryAndOptional_NoComma )
{
    ParseAndVerify ( "function t fn ( a b * a b );" );
}
TEST_CASE ( Function_Formals_MandatoryAndOptional )
{
    ParseAndVerify ( "function t fn ( a b, * a b );" );
}
TEST_CASE ( Function_Formals_MandatoryAndVariadic )
{
    ParseAndVerify ( "function t fn ( a b, ...  );" );
}
TEST_CASE ( Function_Formals_MandatoryOptionalAndVariadic )
{
    ParseAndVerify ( "function t fn ( a b, * a b, ...  );" );
}
TEST_CASE ( Function_Formals_Control )
{
    ParseAndVerify ( "function t fn ( control a b );" );
}

TEST_CASE ( Function_Prologue_Rename )
{
    ParseAndVerify ( "function t fn ( a b ) = fn;" );
}
TEST_CASE ( Function_Prologue_Script_Return )
{
    ParseAndVerify ( "schema t fn ( a b ) { return 1; };" );
}
TEST_CASE ( Function_Prologue_Script_Assign )
{
    ParseAndVerify ( "schema function t fn ( a b ) { a c = 1; };" );
}

TEST_CASE ( Script )
{
    ParseAndVerify ( "schema t fn ( a b );" );
}
TEST_CASE ( Script_Function )
{
    ParseAndVerify ( "schema function t fn ( a b );" );
}

TEST_CASE ( Validate )
{
    ParseAndVerify ( "validate function t fn ( a b );" );
}

TEST_CASE ( Physical_Shorthand )
{
    ParseAndVerify ( "physical t fn #1.0 = { return 1; };" );
}

TEST_CASE ( Physical_Longhand )
{
    ParseAndVerify (
        "physical t fn #1.0 { decode { return 1; } ; encode { return 1; } ; __row_length = f () };" );
}

TEST_CASE ( Physical_noheader )
{
    ParseAndVerify (
        "physical __no_header t fn #1.0 = { return a; };" );
}

TEST_CASE ( Table_NoParents )
{
    ParseAndVerify ( "table t #1.1.1 { t a = 1; };" );
}
TEST_CASE ( Table_Parents )
{
    ParseAndVerify ( "table t #1.1.1 = t1, t2, t3 { t a = 1; };" );
}
TEST_CASE ( Table_ParentsWithVersion )
{
    ParseAndVerify ( "table t #1.1.1 = t1, t2, t3#1.2.3 { t a = 1; };" );
}

TEST_CASE ( Table_Empty )
{
    ParseAndVerify ( "table t #1 {};" );
}
TEST_CASE ( Table_ProdStmt )
{
    ParseAndVerify ( "table t #1 { t a = 1; };" );
}
TEST_CASE ( Table_ProdStmt_FunCallSchemaNoFormals )
{
    ParseAndVerify ( "table t #1 { t p = < t, 1 > fn < 1 > (); };" );
}

TEST_CASE ( Table_Column )
{
    ParseAndVerify ( "table t #1 { column t c; };" );
}
TEST_CASE ( Table_Column_PhysicalEncoding)
{
    ParseAndVerify ( "table t #1 { column <1> p c; };" );
}
TEST_CASE ( Table_Column_PhysicalEncoding_WithVersion )
{
    ParseAndVerify ( "table t #1 { column <1> p#1.2.3 c; };" );
}
TEST_CASE ( Table_Column_PhysicalEncoding_WithFactory )
{
    ParseAndVerify ( "table t #1 { column <1> p <1> c; };" );
}
TEST_CASE ( Table_Column_PhysicalEncoding_WithVersionAndFactory )
{
    ParseAndVerify ( "table t #1 { column <1> p#1 <1> c; };" );
}
TEST_CASE ( Table_Column_Default )
{
    ParseAndVerify ( "table t #1 { default column t c; };" );
}
TEST_CASE ( Table_Column_Extern_WithPhysical )
{
    ParseAndVerify ( "table t #1 { extern column < U32 > izip_encoding #1 CHANNEL; };" );
}
TEST_CASE ( Table_Column_Extern_WithNakedPhysical )
{
    ParseAndVerify ( "table t #1 { extern column bool_encoding #1 HIGH_QUALITY; };" );
}
TEST_CASE ( Table_Column_Extern_WithPhysicalFactory )
{
    ParseAndVerify ( "table t #1 {  column F32_4ch_encoding < 24 > BASE_FRACTION; };" );
}

TEST_CASE ( Table_Column_Readonly )
{
    ParseAndVerify ( "table t #1 { readonly column t c; };" );
}
TEST_CASE ( Table_Column_AllMods )
{
    ParseAndVerify ( "table t #1 { default extern readonly column t c; };" );
}

TEST_CASE ( Table_Column_limit )
{
    ParseAndVerify ( "table t #1 { column limit = 1; };" );
}
TEST_CASE ( Table_Column_default_limit )
{
    ParseAndVerify ( "table t #1 { column default limit = 1; };" );
}

TEST_CASE ( Table_Column_withBody_1 )
{
    ParseAndVerify ( "table t #1 { column t c { read = 1 | 2; validate = 2 | 3; limit = 100; }; };" );
}
TEST_CASE ( Table_Column_withBody_2 )
{
    ParseAndVerify ( "table t #1 { column t c { read = 1 | 2; validate = 2 | 3; limit = 100; } };" );
}
TEST_CASE ( Table_Column_withExpr )
{
    ParseAndVerify ( "table t #1 { column t c = 0 | 1; };" );
}

TEST_CASE ( Table_DefaultView )
{
    ParseAndVerify ( "table t #1 { default view \"QQ\"; };" );
}

TEST_CASE ( Table_PhysMbr_Static )
{
    ParseAndVerify ( "table t #1 { static t .c; };" );
}
TEST_CASE ( Table_PhysMbr_StaticWithInit )
{
    ParseAndVerify ( "table t #1 { static t .c = 1; };" );
}
TEST_CASE ( Table_PhysMbr_Physical )
{
    ParseAndVerify ( "table t #1 { physical t .c = 1; };" );
}
TEST_CASE ( Table_PhysMbr_PhysicalWithVersion )
{
    ParseAndVerify ( "table t #1 { physical column NCBI #1 .CLIP_ADAPTER_LEFT; };" );
}

TEST_CASE ( Table_PhysMbr_StaticPhysical )
{
    ParseAndVerify ( "table t #1 { static physical t .c = 1; };" );
}
TEST_CASE ( Table_PhysMbr_WithColumn )
{
    ParseAndVerify ( "table t #1 { static column t .c = 1; };" );
}

TEST_CASE ( Table_PhysMbr_WithSchema )
{
    ParseAndVerify ( "table t #1 { static column <1, 2> t .c = 1; };" );
}
TEST_CASE ( Table_PhysMbr_WithVersion )
{
    ParseAndVerify ( "table t #1 { static column t#1 .c = 1; };" );
}
TEST_CASE ( Table_PhysMbr_WithFactory )
{
    ParseAndVerify ( "table t #1 { static column t<1> .c = 1; };" );
}
TEST_CASE ( Table_PhysMbr_WithAll )
{
    ParseAndVerify ( "table t #1 { static column <1, t = a:b> t #2.3.4 < 5 > .c = 1; };" );
}

TEST_CASE ( Table_Untyped )
{
    ParseAndVerify ( "table t #1 { __untyped = a:b(); };" );
}

TEST_CASE ( Database_Empty )
{
    ParseAndVerify ( "database a:b #1.2.3 {};" );
}
TEST_CASE ( Database_WithParent )
{
    ParseAndVerify ( "database a:b #1.2.3 = ns:dad #4.5.6 {};" );
}

TEST_CASE ( Database_DbMember )
{
    ParseAndVerify ( "database d#1 { database ns : db DB; };" );
}
TEST_CASE ( Database_DbMember_WithTemplate )
{
    ParseAndVerify ( "database d#1 { template database ns : db DB; };" );
}

TEST_CASE ( Database_TableMember )
{
    ParseAndVerify ( "database d#1 { table ns : tbl T; };" );
}
TEST_CASE ( Database_TableMember_WithTemplate )
{
    ParseAndVerify ( "database d#1 { template table ns : tbl T; };" );
}

TEST_CASE ( Database_MultipleMembers )
{
    ParseAndVerify ( "database d#1 { database ns : db DB; table ns : tbl T; };" );
}

TEST_CASE ( Include )
{
    ParseAndVerify ( "include 'insdc/sra.vschema';" );
}

// Expressions
TEST_CASE ( CastExpr )
{
    ParseAndVerify ( "function t f() { return (a)b; };" );
}

TEST_CASE ( AtExpr )
{
    ParseAndVerify ( "function t f() { return @; };" );
}

TEST_CASE ( HexExpr )
{
    ParseAndVerify ( "function t f() { return 0x1290ABEF; };" );
}

TEST_CASE ( FloatExpr )
{
    ParseAndVerify ( "function t f() { return 1.2; };" );
}

TEST_CASE ( ExpFloatExpr )
{
    ParseAndVerify ( "function t f() { return 1E2; };" );
}

TEST_CASE ( StringExpr )
{
    ParseAndVerify ( "function t f() { return \"qq\"; };" );
}

TEST_CASE ( EscapedStringExpr )
{
    ParseAndVerify ( "function t f() { return \"q\\q\"; };" );
}

TEST_CASE ( ConstVectExpr_Empty )
{
    ParseAndVerify ( "function t f() { return [ ]; };" );
}
TEST_CASE ( ConstVectExpr )
{
    ParseAndVerify ( "function t f() { return [ 1, 2, 3 ]; };" );
}

TEST_CASE ( BoolExpr_True )
{
    ParseAndVerify ( "function t f() { return true; };" );
}
TEST_CASE ( BoolExpr_False )
{
    ParseAndVerify ( "function t f() { return false; };" );
}

TEST_CASE ( NegateExpr_Ident )
{
    ParseAndVerify ( "function t f() { return -a; };" );
}
TEST_CASE ( NegateExpr_Int )
{
    ParseAndVerify ( "function t f() { return -1; };" );
}
TEST_CASE ( NegateExpr_Float )
{
    ParseAndVerify ( "function t f() { return -1.0; };" );
}

TEST_CASE ( FuncExpr_NoSchemaNoFactNoParms )
{
    ParseAndVerify ( "function t f() { return f(); };" );
}
TEST_CASE ( FuncExpr_NoSchemaFactNoParms )
{
    ParseAndVerify ( "function t f() { return f<1>(); };" );
}
TEST_CASE ( FuncExpr_NoSchemaNoFactParms )
{
    ParseAndVerify ( "function t f() { return f(1); };" );
}
TEST_CASE ( FuncExpr_NoSchemaFactParms )
{
    ParseAndVerify ( "function t f() { return f<a, b>(c, d); };" );
}
TEST_CASE ( FuncExpr_Version )
{
    ParseAndVerify ( "function t f() { return f#1<a, b>(c, d); };" );
}

TEST_CASE ( PhysicalIdent )
{
    ParseAndVerify ( "function t f() { return .b; };" );
}

// Version 2

TEST_CASE ( VersionOther )
{
    REQUIRE ( ! SchemaParser () . ParseString ( "version 3.14; $" ) );
}

TEST_CASE ( Version2_Decl1 )
{
    ParseAndVerify ( "version 2; include \"qq\";" );
}

TEST_CASE ( View_InVers1 )
{
    REQUIRE ( ! SchemaParser () . ParseString ( "view X#1 < T t > {};" ) );
}

TEST_CASE ( Decls_1_And_2 )
{
    ParseAndVerify ( "version 2;  include \"qq\"; view X#1< T t > { U8 p = 1; };" );
}

TEST_CASE ( View_OneParam_OneStmt )
{
    ParseAndVerify ( "version 2; view X#1 < T t > { U8 p = 1; };" );
}

TEST_CASE ( View_Params_Stmts )
{
    ParseAndVerify ( "version 2; view X#1 < T1 t1, T2 t2 > { U8 p = 1; ; column U32 c = p; };" );
}

TEST_CASE ( View_OneParent )
{
    ParseAndVerify ( "version 2; view X#1 < T t > = V < t > { U8 p = 1; };" );
}

TEST_CASE ( View_TwoParents )
{
    ParseAndVerify ( "version 2; view X#1 < T t > = V#1<t>,W<t> { U8 p = 1; };" );
}

TEST_CASE ( View_OneParam_EmptyBody )
{
    ParseAndVerify ( "version 2; view X#1 < T t > {};" );
}

TEST_CASE ( View_NoPhysicalColumns )
{
    REQUIRE ( ! SchemaParser () . ParseString ( "version 2; view X#1 < T t > { column U8 .PHYS = 1; };" ) );
}

TEST_CASE ( View_MemberExpression )
{
    ParseAndVerify ( "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t . c1; }" );
}

TEST_CASE ( View_JoinExpression )
{
    ParseAndVerify ( "version 2; table T#1 { column U8 c1 = 1; }; view W#1 <T t> { column U8 c2 = t [ 1 ] . c1; }" );
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

const char UsageDefaultName[] = "wb-test-schema-parser";

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
    return SchemaParserTestSuite(argc, argv);
}

}

