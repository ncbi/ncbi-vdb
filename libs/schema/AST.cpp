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

#include "AST.hpp"

#include <klib/symbol.h>
#include <klib/printf.h>
#include <klib/rc.h>

#include "../vdb/schema-priv.h"
// hide an unfortunately named C function typename()
#define typename __typename
#include "../../libs/vdb/schema-parse.h"
#undef typename

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

// AST

SchemaToken st = { PT_EMPTY, NULL, 0, 0, 0 };

AST :: AST ()
: ParseTree ( st )
{
}

AST :: AST ( const Token * p_token )
: ParseTree ( * p_token )
{
}

AST :: ~AST ()
{
}

void
AST :: AddNode ( AST * p_child )
{
    AddChild ( p_child );
}

void
AST :: AddNode ( const Token * p_child )
{
    AddChild ( new AST ( p_child ) );
}

// ASTBuilder

ASTBuilder :: ASTBuilder ()
:   m_debug ( false ),
    m_intrinsic ( 0 ),
    m_schema ( 0 )
{
    VectorInit ( & m_errors, 0, 1024 );

    rc_t rc = VSchemaMakeIntrinsic ( & m_intrinsic );
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMakeIntrinsic failed: %R", rc  );
        m_intrinsic = 0;
    }
    else
    {
        rc = KSymTableInit ( & m_symtab, 0 );
        if ( rc != 0 )
        {
            ReportError ( "KSymTableInit failed: %R", rc );
            VSchemaRelease ( m_intrinsic );
            m_intrinsic = 0;
        }
    }
}


void
ASTBuilder :: DebugOn ()
{
    m_debug = true;
}

static
void CC
WhackMessage ( void *item, void *data )
{
    free ( item );
}

ASTBuilder :: ~ASTBuilder ()
{
    KSymTableWhack ( & m_symtab);
    VSchemaRelease ( m_schema );
    VSchemaRelease ( m_intrinsic );
    VectorWhack ( & m_errors, WhackMessage, 0 );
}

void
ASTBuilder :: ReportError ( const char* p_fmt, ... )
{
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    VectorAppend ( & m_errors, 0, string_dup_measure ( buf, 0 ) );
}

bool
ASTBuilder :: Init()
{
    if ( m_schema != 0 )    // in case we are called more than once
    {
        KSymTableWhack ( & m_symtab);
        VSchemaRelease ( m_schema );
    }
    rc_t rc = VSchemaMake ( & m_schema, m_intrinsic );
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMake failed: %R", rc );
        m_schema = 0;
        return false;
    }

    rc = init_symtab ( & m_symtab, m_schema ); // this pushes the global scope
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMake failed: %R", rc );
        return false;
    }

    return true;
}

AST *
ASTBuilder :: Build ( const ParseTree& p_root )
{
    if ( m_intrinsic == 0 || ! Init () )
    {
        return 0;
    }

    AST* ret = 0;
    AST_debug = m_debug;
    ParseTreeScanner scanner ( p_root );
    if ( AST_parse ( ret, * this, scanner ) == 0 )
    {
        return ret;
    }

    //TODO: report error(s)
    return 0;
}

void
ASTBuilder :: Declare ( const Token& p_ident, KSymbol* )
{

}

void
ASTBuilder :: DeclareType ( const Token& p_ident, const KSymbol* p_super, const AST* p_dimension )
{
    assert ( p_dimension == 0 ); //TODO
    const uint32_t dimension = 1;

    /* allocate a datatype */
    SDatatype * dt = static_cast < SDatatype * > ( malloc ( sizeof * dt ) ); // VSchema's tables dispose of objects with free()
    if ( dt == NULL )
    {
        ReportError ( "malloc failed: %R", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
    }
    else
    {
        /* insert into type vector */
        rc_t rc = VectorAppend ( & m_schema -> dt, & dt -> id, dt );
        if ( rc != 0 )
        {
            ReportError ( "VectorAppend failed: %R", rc );
            free ( dt );
        }
        else
        {
            /* capture supertype */
            assert ( p_super != NULL );
            assert ( p_super -> type == eDatatype );
            const SDatatype * super = static_cast < const SDatatype * > ( p_super -> u . obj );

            // create a symtab entry
            String name;
            StringInitCString ( & name, p_ident . GetValue() );
            KSymbol* symbol;
            rc_t rc = KSymTableCreateSymbol ( & m_symtab, & symbol, & name, eDatatype, dt );
            if ( rc == 0 )
            {
                /* fill it out from super-type */
                dt -> super     = super;
                dt -> byte_swap = super -> byte_swap;
                dt -> name      = symbol;
                dt -> size      = super -> size * dimension;
                dt -> dim       = dimension;
                dt -> domain    = super -> domain;
            }
            else if ( GetRCState ( rc ) == rcExists )
            {
                ReportError ( "Object already declared: '%s'", p_ident . GetValue() ); //TODO: add location of the original declaration
            }
            else
            {
                ReportError ( "KSymTableCreateSymbol failed: %R", rc );
            }
        }
    }
}

KSymbol*
ASTBuilder :: Resolve ( const Token& p_ident )
{
    assert ( p_ident . GetType () == IDENTIFIER_1_0 );
    String name;
    StringInitCString ( & name, p_ident . GetValue () );
    KSymbol * ret = KSymTableFind ( & m_symtab, & name );
    if ( ret == 0 )
    {
        ReportError ( "Undeclared identifier: '%s'", p_ident . GetValue () ); //TODO: add location
    }
    return ret;
}

// AST nodes

AST_Schema :: AST_Schema ()
:   m_versMajor ( 0 ),
    m_versMinor ( 0 )
{
}


AST_Schema :: AST_Schema ( const Token * p_token, AST* p_decls /*NULL OK*/, unsigned int p_version )
:   AST ( p_token ),
    m_versMajor ( p_version ),
    m_versMinor ( 0 )
{
    if ( p_decls != 0 )
    {
        MoveChildren ( * p_decls );
        delete p_decls;
    }
}

AST_Schema :: ~AST_Schema()
{
}

void
AST_Schema :: SetVersion ( const char* ) // version specified as n.m; checked for valid version number here
{
    //TODO
}

AST_TypeDef :: AST_TypeDef ( ASTBuilder & p_builder, const Token * p_token, AST* p_baseType, AST* p_newTypes )
:   AST ( p_token )
{
    assert ( p_baseType != 0 );
    AddNode ( p_baseType );
    assert ( p_newTypes != 0 );
    AddNode ( p_newTypes );

    assert ( p_baseType -> ChildrenCount () == 1 );
    KSymbol * baseType = p_builder . Resolve ( p_baseType -> GetChild ( 0 ) -> GetToken () ); // will report unknown name
    if ( baseType != 0 )
    {
        if ( baseType -> type != eDatatype )
        {
            p_builder . ReportError ( "Not a datatype: '%s'", p_baseType -> GetChild ( 0 ) -> GetToken () . GetValue () ); //TODO: add location
            //TODO: recover? pretend it is "any"?
            return;
        }

        uint32_t count = p_newTypes -> ChildrenCount ();
        for ( uint32_t i = 0; i < count; ++i )
        {
            const AST* newType = p_newTypes -> GetChild ( i );
            if ( newType -> GetToken () . GetType () == PT_ARRAY )
            {
                assert ( newType -> ChildrenCount () == 2 );
                const Token & newT = newType -> GetChild ( 0 ) -> GetToken ();
                assert ( newT . GetType () == IDENTIFIER_1_0 ); //TODO: PHYSICAL_IDENTIFIER (allowed??); FQN (token-less list)
                p_builder . DeclareType ( newT, baseType, newType -> GetChild ( 1 ) ); //TODO: support *; will report duplicate definition
            }
            else // scalar
            {
                assert ( newType -> ChildrenCount () == 1 );
                const Token & newT = newType -> GetChild ( 0 ) -> GetToken ();
                assert ( newT . GetType () == IDENTIFIER_1_0 ); //TODO: PHYSICAL_IDENTIFIER (allowed??); FQN (token-less list)
                p_builder . DeclareType ( newT, baseType, 0 ); // will report duplicate definition
            }
        }
    }
}

AST_TypeDef :: ~AST_TypeDef()
{
}

AST_ArrayDef :: AST_ArrayDef ( const Token * p_token, AST* p_typeName, AST* p_dimension )
:   AST ( p_token )
{
    assert ( p_typeName != 0 );
    AddNode ( p_typeName );
    if ( p_dimension != 0 )
    {
        AddNode ( p_dimension );
    }
}

AST_ArrayDef :: ~AST_ArrayDef()
{
}