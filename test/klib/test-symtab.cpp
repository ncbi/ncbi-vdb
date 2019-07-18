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
* Unit tests for KSymTable
*/

#include <klib/symtab.h>

#include <stdexcept>

#include <ktst/unit_test.hpp>

#include <klib/container.h>
#include <klib/text.h>
#include <klib/symbol.h>
#include <klib/token.h>

using namespace std;

TEST_SUITE(KSymtabTestSuite);

class SymtabFixture
{
public:
    SymtabFixture()
    : m_sym ( NULL )
    {
        BSTreeInit ( & m_intrinsic );
        if ( KSymTableInit ( & m_self, & m_intrinsic ) != 0 )
        {
            throw logic_error ( "KSymTableInit failed" );
        }
        CONST_STRING( & m_int, "int" );
        // push int into the intrinsic scope
        KSymTableCreateSymbol ( & m_self, NULL, & m_int, 1, NULL );

        BSTreeInit ( & m_global );
        if ( KSymTablePushScope ( & m_self, & m_global ) != 0 )
        {
            throw logic_error ( "KSymTablePushScope failed" );
        }
    }
    ~SymtabFixture()
    {
        KSymTableWhack ( & m_self ); // will take care of intrinsic scope
        free ( m_sym );
    }

    KSymTable   m_self;
    BSTree      m_intrinsic;
    BSTree      m_global;
    String      m_int;

    // for use in test cases
    KSymbol *   m_sym;
    String      m_name;
};

TEST_CASE ( CreateDestroy )
{
    KSymTable self;
    REQUIRE_RC ( KSymTableInit ( & self, NULL ) );
    KSymTableWhack ( & self );
}

TEST_CASE ( CreateDestroy_WithIntrinsic )
{
    BSTree intrinsic;
    KSymTable self;
    REQUIRE_RC ( KSymTableInit ( & self, & intrinsic ) );
    KSymTableWhack ( & self );
}

TEST_CASE ( PushPopScope )
{
    KSymTable self;
    REQUIRE_RC ( KSymTableInit ( & self, NULL ) );

    BSTree scope;
    BSTreeInit ( & scope );
    REQUIRE_RC ( KSymTablePushScope ( & self, & scope ) );
    KSymTablePopScope ( & self );
    BSTreeWhack ( & scope, NULL, NULL );

    KSymTableWhack ( & self );
}

FIXTURE_TEST_CASE ( CreateNamespace, SymtabFixture )
{
    CONST_STRING( & m_name, "NameSpace" );
    REQUIRE_RC ( KSymTableCreateNamespace ( & m_self, & m_sym, & m_name ) );
    REQUIRE_NOT_NULL ( m_sym );
    REQUIRE_EQ ( m_self . ns, m_sym -> dad );
    REQUIRE ( StringEqual ( & m_name, & m_sym -> name ) );
    REQUIRE_EQ ( ( uint32_t ) eNamespace, m_sym -> type );
}

FIXTURE_TEST_CASE ( CreateNamespace_Exists, SymtabFixture )
{
    CONST_STRING( & m_name, "NameSpace" );
    REQUIRE_RC ( KSymTableCreateNamespace ( & m_self, & m_sym, & m_name ) );
    KSymbol * new_sym;
    REQUIRE_RC ( KSymTableCreateNamespace ( & m_self, & new_sym, & m_name ) );
    REQUIRE_EQ ( m_sym, new_sym );
}

FIXTURE_TEST_CASE ( CreateNamespace_NotNamespace, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    REQUIRE_RC ( KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 25, NULL ) );
    KSymbol * new_sym;
    REQUIRE_RC_FAIL ( KSymTableCreateNamespace ( & m_self, & new_sym, & m_name ) );
    REQUIRE_NULL ( new_sym );
}

FIXTURE_TEST_CASE ( PushPopNamespace, SymtabFixture )
{
    CONST_STRING( & m_name, "NameSpace" );
    REQUIRE_RC ( KSymTableCreateNamespace ( & m_self, & m_sym, & m_name ) );

    REQUIRE_RC ( KSymTablePushNamespace ( & m_self, m_sym ) );
    REQUIRE_EQ ( m_sym, m_self . ns );

    KSymTablePopNamespace ( & m_self );
    REQUIRE_NULL ( m_self . ns );
}

FIXTURE_TEST_CASE ( CreateSymbol_NoDefinition, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    uint32_t type = 25;
    REQUIRE_RC ( KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, type, NULL ) );
    REQUIRE_NULL ( m_sym -> u . obj );
    REQUIRE_EQ ( m_self . ns, m_sym -> dad );
    REQUIRE ( StringEqual ( & m_name, & m_sym -> name ) );
    REQUIRE_EQ ( type, m_sym -> type );
}

FIXTURE_TEST_CASE ( CreateSymbol_WithDefinition, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    uint32_t type = 25;
    const void* def = (void*)0x12345678;
    REQUIRE_RC ( KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, type, def ) );
    REQUIRE_EQ ( def, m_sym -> u . obj );
    REQUIRE_EQ ( m_self . ns, m_sym -> dad );
    REQUIRE ( StringEqual ( & m_name, & m_sym -> name ) );
    REQUIRE_EQ ( type, m_sym -> type );
}

FIXTURE_TEST_CASE ( FindSymbol_ByName, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );
    REQUIRE_EQ ( m_sym, KSymTableFind ( & m_self, & m_name ) );
}
FIXTURE_TEST_CASE ( FindSymbol_ByName_NotFound, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );
    String new_name;
    CONST_STRING( & new_name, "vvar" );
    REQUIRE_NULL ( KSymTableFind ( & m_self, & new_name ) );
}

FIXTURE_TEST_CASE ( FindSymbol_BySymbol, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );
    REQUIRE_EQ ( m_sym, KSymTableFindSymbol ( & m_self, m_sym ) );
}
FIXTURE_TEST_CASE ( FindSymbol_BySymbol_NotFound, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );

    String new_name;
    CONST_STRING( & new_name, "vvar" );
    KSymbol * new_sym;
    REQUIRE_RC ( KSymbolMake ( & new_sym, & new_name, 1, NULL ) );
    REQUIRE_NULL ( KSymTableFindSymbol ( & m_self, new_sym ) );

    KSymbolWhack ( & new_sym -> n, NULL );
}

FIXTURE_TEST_CASE ( FindIntrinsic_NotFound, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    REQUIRE_NULL ( KSymTableFindIntrinsic ( & m_self, & m_name ) );
}
FIXTURE_TEST_CASE ( FindIntrinsic_Found, SymtabFixture )
{
    REQUIRE_EQ ( m_sym, KSymTableFindIntrinsic ( & m_self, & m_int ) );
}

FIXTURE_TEST_CASE ( FindGlobal_NotFound, SymtabFixture )
{
    REQUIRE_NULL ( KSymTableFindGlobal ( & m_self, & m_int ) );
}
FIXTURE_TEST_CASE ( FindGlobal_Found, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    // push var into the global scope
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );

    // push another var into a new scope
    BSTree scope;
    BSTreeInit ( & scope );
    REQUIRE_RC ( KSymTablePushScope ( & m_self, & scope ) );
    KSymbol * new_sym;
    REQUIRE_RC ( KSymTableCreateSymbol ( & m_self, & new_sym, & m_name, 1, NULL ) );

    // make sure KSymTableFindGlobal finds the global var
    REQUIRE_EQ ( m_sym, KSymTableFindGlobal ( & m_self, & m_name ) );

    KSymbolWhack ( & new_sym -> n, NULL );
}

FIXTURE_TEST_CASE ( FindShallow_NotFound, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    // push var into the global scope
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );

    // push another scope
    BSTree scope;
    BSTreeInit ( & scope );
    REQUIRE_RC ( KSymTablePushScope ( & m_self, & scope ) );

    // only looking in the innermost scope
    REQUIRE_NULL ( KSymTableFindShallow ( & m_self, & m_name ) );
}

FIXTURE_TEST_CASE ( FindShallow_Found, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );
    REQUIRE_EQ ( m_sym, KSymTableFindShallow ( & m_self, & m_name ) );
}

FIXTURE_TEST_CASE ( FindNext_NotFound, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );
    REQUIRE_EQ ( m_sym, KSymTableFind ( & m_self, & m_name ) );
    uint32_t scopeNum = 99;
    REQUIRE_NULL ( KSymTableFindNext ( & m_self, m_sym, & scopeNum ) );
    REQUIRE_EQ ( 0u, scopeNum );
}
FIXTURE_TEST_CASE ( FindNext_Found, SymtabFixture )
{
    CONST_STRING( & m_name, "var" );
    KSymTableCreateSymbol ( & m_self, & m_sym, & m_name, 1, NULL );

    // push another var into a new scope
    BSTree scope;
    BSTreeInit ( & scope );
    REQUIRE_RC ( KSymTablePushScope ( & m_self, & scope ) );
    KSymbol * new_sym;
    REQUIRE_RC ( KSymTableCreateSymbol ( & m_self, & new_sym, & m_name, 1, NULL ) );

    REQUIRE_EQ ( new_sym, KSymTableFind ( & m_self, & m_name ) );   // local var
    uint32_t scopeNum = 0;
    REQUIRE_EQ ( m_sym, KSymTableFindNext ( & m_self, new_sym, & scopeNum ) );  // global var
    REQUIRE_EQ ( 2u, scopeNum );    // 2 is the global scope
    REQUIRE_NULL ( KSymTableFindNext ( & m_self, m_sym, & scopeNum ) );
    REQUIRE_EQ ( 0u, scopeNum );

    KSymbolWhack ( & new_sym -> n, NULL );
}

//////////////////////////////////////////////////// Main
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

const char UsageDefaultName[] = "test-symtab";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KSymtabTestSuite(argc, argv);
    return rc;
}

}
