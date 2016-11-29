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

#include "../../libs/schema/SchemaParser.hpp"

using namespace std;
using namespace ncbi::NK;
using namespace ncbi::SchemaParser;

TEST_SUITE ( SchemaParserTestSuite );

TEST_CASE ( EmptyInput )
{
    REQUIRE ( SchemaParser () . ParseString ( "" ) );
}

TEST_CASE ( Version1 )
{
    REQUIRE ( SchemaParser () . ParseString ( "version 1; include \"qq\";" ) );
}

TEST_CASE ( Typedef )
{
    REQUIRE ( SchemaParser () . ParseString ( "typedef oldName newName;" ) );
}
TEST_CASE ( TypedefDim )
{
    REQUIRE ( SchemaParser () . ParseString ( "typedef oldName newName [ 12 ];" ) );
}
TEST_CASE ( TypedefMultipleNames )
{
    REQUIRE ( SchemaParser () . ParseString ( "typedef oldName neawName1, newName2 [ 12 ], newName3;" ) );
}

TEST_CASE ( Typeset )
{
    REQUIRE ( SchemaParser () . ParseString ( "typeset newName { a, b[1], c };" ) );
}

TEST_CASE ( Format )
{
    REQUIRE ( SchemaParser () . ParseString ( "fmtdef newName;" ) );
}
TEST_CASE ( FormatRename )
{
    REQUIRE ( SchemaParser () . ParseString ( "fmtdef oldName newName;" ) );
}

TEST_CASE ( Const )
{
    REQUIRE ( SchemaParser () . ParseString ( "const t c = 1;" ) );
}
TEST_CASE ( ConstDim )
{
    REQUIRE ( SchemaParser () . ParseString ( "const t[2] c = 1;" ) );
}

TEST_CASE ( Alias )
{
    REQUIRE ( SchemaParser () . ParseString ( "alias a b;" ) );
}

TEST_CASE ( Extern_AndUntyped )
{
    REQUIRE ( SchemaParser () . ParseString ( "extern function __untyped fn ();" ) );
}

TEST_CASE ( Function_RowLength )
{
    REQUIRE ( SchemaParser () . ParseString ( "function __row_length fn ();" ) );
}

TEST_CASE ( Function_Naked )
{   //TODO: verify that paramter-less functions are not allowed
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b );" ) );
}

TEST_CASE ( Function_Schema )
{
    REQUIRE ( SchemaParser () . ParseString ( "function < type b,  a / fmt c > t fn ( a b );" ) );
}

TEST_CASE ( Function_Factory )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn < a b > ( a b );" ) );
}

TEST_CASE ( Function_NoFormals )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ();" ) );
}
TEST_CASE ( Function_Formals_OptionalOnly )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( * a b );" ) );
}
TEST_CASE ( Function_Formals_MandatoryAndOptional_NoComma )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b * a b );" ) );
}
TEST_CASE ( Function_Formals_MandatoryAndOptional )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b, * a b );" ) );
}
TEST_CASE ( Function_Formals_MandatoryAndVariadic )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b, ...  );" ) );
}
TEST_CASE ( Function_Formals_MandatoryOptionalAndVariadic )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b, * a b, ...  );" ) );
}
TEST_CASE ( Function_Formals_Control )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( control a b );" ) );
}

TEST_CASE ( Function_Prologue_Rename )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b ) = fn;" ) );
}
TEST_CASE ( Function_Prologue_Script_Return )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b ) { return 1; };" ) );
}
TEST_CASE ( Function_Prologue_Script_AssignFormat )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b ) { a / b c = 1; };" ) );
}
TEST_CASE ( Function_Prologue_Script_Assign )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b ) { a c = 1; };" ) );
}
TEST_CASE ( Function_Prologue_Script_Trigger )
{
    REQUIRE ( SchemaParser () . ParseString ( "function t fn ( a b ) { trigger c = 1; };" ) );
}

TEST_CASE ( Script )
{
    REQUIRE ( SchemaParser () . ParseString ( "schema t fn ( a b );" ) );
}
TEST_CASE ( Script_Function )
{
    REQUIRE ( SchemaParser () . ParseString ( "schema function t fn ( a b );" ) );
}

TEST_CASE ( Validate )
{
    REQUIRE ( SchemaParser () . ParseString ( "validate function t fn ( a b );" ) );
}

TEST_CASE ( Physical_Shorthand )
{
    REQUIRE ( SchemaParser () . ParseString ( "physical t fn #1.0 = { return 1; };" ) );
}

TEST_CASE ( Physical_Longhand )
{
    REQUIRE ( SchemaParser () . ParseString (
        "physical t fn #1.0 { decode { return 1; } ; encode { return 1; } ; __row_length = f () };" ) );
}

TEST_CASE ( Table_NoParents )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1.1.1 { t a = 1; };" ) );
}
TEST_CASE ( Table_Parents )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1.1.1 = t1, t2, t3 { t a = 1; };" ) );
}

TEST_CASE ( Table_ProdStmt )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { t a = 1; };" ) );
}
TEST_CASE ( Table_ProdStmt_NoFormals )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { t p = < t > fn < 1 > (); };" ) );
}

TEST_CASE ( Table_Column )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column t c; };" ) );
}
TEST_CASE ( Table_Column_PhysicalEncoding )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column <1> p t c; };" ) );
}
TEST_CASE ( Table_Column_PhysicalEncoding_WithVersion )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column <1> p#1.2.3 t c; };" ) );
}
TEST_CASE ( Table_Column_PhysicalEncoding_WithFactory )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column <1> p <1> t c; };" ) );
}
TEST_CASE ( Table_Column_PhysicalEncoding_WithVersionAndFactory )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column <1> p#1 <1> t c; };" ) );
}
TEST_CASE ( Table_Column_Default )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { default column t c; };" ) );
}
TEST_CASE ( Table_Column_Extern )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { extern column t c; };" ) );
}
TEST_CASE ( Table_Column_Readonly )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { readonly column t c; };" ) );
}
TEST_CASE ( Table_Column_AllMods )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { default extern readonly column t c; };" ) );
}

TEST_CASE ( Table_Column_default )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column default t c; };" ) );
}
TEST_CASE ( Table_Column_limit )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column limit = 1; };" ) );
}
TEST_CASE ( Table_Column_default_limit )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { column default limit = 1; };" ) );
}

TEST_CASE ( Table_Column_withBody )
{
    REQUIRE ( SchemaParser () . ParseString (
        "table t #1 { column t c { read = 1 | 2; validate = 2 | 3; limit = 100}; };" ) );
}
TEST_CASE ( Table_Column_withExpr )
{
    REQUIRE ( SchemaParser () . ParseString (
        "table t #1 { column t c = 0; };" ) );
}

TEST_CASE ( Table_DefaultView )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { default view \"QQ\"; };" ) );
}

TEST_CASE ( Table_PhysMbr_Static )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static t .c = 1; };" ) );
}
TEST_CASE ( Table_PhysMbr_Physical )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { physical t .c = 1; };" ) );
}
TEST_CASE ( Table_PhysMbr_StaticPhysical )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static physical t .c = 1; };" ) );
}
TEST_CASE ( Table_PhysMbr_WithColumn )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static column t .c = 1; };" ) );
}

TEST_CASE ( Table_PhysMbr_WithSchema )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static column <1> t .c = 1; };" ) );
}
TEST_CASE ( Table_PhysMbr_WithVErsion )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static column t#1 .c = 1; };" ) );
}
TEST_CASE ( Table_PhysMbr_WithFactory )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static column t<1> .c = 1; };" ) );
}
TEST_CASE ( Table_PhysMbr_WithAll )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { static column <1>t#2.3.4<5> .c = 1; };" ) );
}

TEST_CASE ( Table_Untyped )
{
    REQUIRE ( SchemaParser () . ParseString ( "table t #1 { __untyped = a:b(); };" ) );
}

TEST_CASE ( Database_Empty )
{
    REQUIRE ( SchemaParser () . ParseString ( "database a:b #1.2.3 {};" ) );
}
TEST_CASE ( Database_WithParent )
{
    REQUIRE ( SchemaParser () . ParseString ( "database a:b #1.2.3 = ns:dad #4.5.6 {};" ) );
}

TEST_CASE ( Database_DbMember )
{
    REQUIRE ( SchemaParser () . ParseString ( "database d#1 { database ns : db DB; };" ) );
}
TEST_CASE ( Database_DbMember_WithTemplate )
{
    REQUIRE ( SchemaParser () . ParseString ( "database d#1 { template database ns : db DB; };" ) );
}

TEST_CASE ( Database_TableMember )
{
    REQUIRE ( SchemaParser () . ParseString ( "database d#1 { table ns : tbl T; };" ) );
}
TEST_CASE ( Database_TableMember_WithTemplate )
{
    REQUIRE ( SchemaParser () . ParseString ( "database d#1 { template table ns : tbl T; };" ) );
}

TEST_CASE ( Include )
{
    REQUIRE ( SchemaParser () . ParseString ( "include 'insdc/sra.vschema';" ) );
}

// Version 2

TEST_CASE ( VersionOther )
{
    REQUIRE ( SchemaParser () . ParseString ( "version 3.14; $" ) ); //TODO
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

