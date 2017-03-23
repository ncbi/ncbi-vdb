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

#include <string>
#include <iostream>

#include <klib/symbol.h>

#include <vdb/manager.h>

#include "AST_Fixture.hpp"

using namespace std;

AST_Fixture :: AST_Fixture()
:   m_debugParse ( false ),
    m_printTree ( false ),
    m_debugAst ( false ),
    m_parseTree ( 0 ),
    m_ast ( 0 ),
    m_schema ( 0 ),
    m_newParse ( true )
{
}
AST_Fixture :: ~AST_Fixture()
{
    delete m_ast;
    delete m_parseTree;
    VSchemaRelease ( m_schema );
}

void
AST_Fixture :: PrintTree ( const ParseTree& p_tree )
{
    ParseTreeScanner sc ( p_tree );
    const Token* tok;
    SchemaScanner :: TokenType tt;
    unsigned int indent = 0;
    do
    {
        tt = sc . NextToken ( tok );
        string v = tok -> GetValue ();
        if ( v . empty () )
        {
            switch (tt)
            {
            #define case_(t) case t:  v = #t; break
            case_ ( PT_PARSE );
            case_ ( PT_SOURCE );
            case_ ( PT_SCHEMA_1_0 );
            case_ ( PT_TYPEDEF );
            case_ ( PT_TYPESET );
            case_ ( PT_TYPESETDEF );
            case_ ( PT_IDENT );
            case_ ( PT_ASTLIST );
            case_ ( PT_ARRAY );
            case_ ( END_SOURCE );
            case_ ( PT_FUNCTION );
            case_ ( PT_FUNCDECL );
            case_ ( PT_EMPTY );
            case_ ( PT_SCHEMASIG );
            case_ ( PT_SCHEMAFORMAL );
            case_ ( PT_RETURNTYPE );
            case_ ( PT_FUNCSIG );
            case_ ( PT_FUNCPROLOGUE );
            case_ ( PT_FUNCPARAMS );
            case_ ( PT_FORMALPARAM );
            case_ ( PT_RETURN );
            case_ ( PT_UINT );
            case_ ( PT_VERSNAME );
            case_ ( PT_PHYSPROLOGUE );
            case_ ( PT_PHYSSTMT );
            case_ ( PT_PHYSBODYSTMT );
            case_ ( PT_FQN );
            case_ ( PT_TABLE );
            case_ ( PT_TABLEBODY );
            case_ ( PT_TYPEDCOL );
            case_ ( PT_COLUMN );
            case_ ( PT_COLDECL );
            case_ ( PT_PHYSENCEXPR );
            case_ ( PT_FACTPARMS );
            case_ ( PT_PHYSENCREF );
            case_ ( PT_COLSTMT );
            case_ ( PT_DBBODY );
            case_ ( PT_DBMEMBER );

            default:
                if ( tt < 256 )
                {
                    v = string ( 1, ( char ) tt );
                }
                break;
            }
        }
        if ( tt == ')' && indent != 0 )
        {
            --indent;
        }
        cout << string ( indent * 2, ' ' ) << v << " (" << tt << ")" << endl;
        if ( tt == '(' )
        {
            ++indent;
        }
    }
    while ( tt != END_SOURCE );
    cout << string ( indent * 2, ' ' ) << ") (41)" << endl;
}

AST_FQN *
AST_Fixture :: MakeFqn ( const char* p_text ) // p_text = (ident:)+ident
{
    SchemaToken id = { PT_IDENT, 0, 0, 0, 0 };
    Token ident ( id );
    AST_FQN * ret = new AST_FQN ( & ident );

    std::string s ( p_text );

    while ( s . length () > 0 )
    {
        std::string token;
        size_t pos = s . find(':');
        if (pos != std::string::npos)
        {
            token = s . substr ( 0, pos );
            s . erase(0, pos + 1);
        }
        else
        {
            token = s;
            s . clear ();
        }
        SchemaToken name = { IDENTIFIER_1_0, token . c_str () , token . length (), 0, 0 };
        Token tname ( name );
        ret -> AddNode ( & tname );
    }

    return ret;
}

AST *
AST_Fixture :: MakeAst ( const char* p_source )
{
    if ( ! m_parser . ParseString ( p_source, m_debugParse ) )
    {
        throw std :: logic_error ( "AST_Fixture::MakeAst : ParseString() failed" );
    }
    if ( m_parseTree != 0 )
    {
        delete m_parseTree;
    }
    m_parseTree = m_parser . MoveParseTree ();
    if ( m_parseTree == 0 )
    {
        throw std :: logic_error ( "AST_Fixture::MakeAst : MoveParseTree() returned 0" );
    }
    if ( m_printTree )
    {
        PrintTree ( * m_parseTree );
    }
    if ( m_ast != 0 )
    {
        delete m_ast;
    }
    m_ast = m_builder . Build ( * m_parseTree, m_debugAst );
    if ( m_builder . GetErrorCount() != 0)
    {
        throw std :: logic_error ( string ( "AST_Fixture::MakeAst : ASTBuilder::Build() failed: " ) + string ( m_builder . GetErrorMessage ( 0 ) ) );
    }
    else if ( m_ast == 0 )
    {
        throw std :: logic_error ( "AST_Fixture::MakeAst : ASTBuilder::Build() failed, no message!" );
    }
    return m_ast;
}

void
AST_Fixture :: VerifyErrorMessage ( const char* p_source, const char* p_expectedError )
{
    if ( m_newParse )
    {
        if ( ! m_parser . ParseString ( p_source ) )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyErrorMessage : ParseString() failed" );
        }
        m_parseTree = m_parser . MoveParseTree ();
        if ( m_parseTree == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyErrorMessage : MoveParseTree() returned 0" );
        }
        delete m_builder . Build ( * m_parseTree );
        if ( m_builder . GetErrorCount() == 0 )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyErrorMessage : no error" );
        }
        if ( string ( m_builder . GetErrorMessage ( 0 ) ) != string ( p_expectedError ) )
        {
            throw std :: logic_error ( "AST_Fixture::VerifyErrorMessage : expected '" + string ( p_expectedError ) +
                                                                        "', received '" + string ( m_builder . GetErrorMessage ( 0 ) ) + "'" );
        }
    }
    else if ( OldParse ( p_source ) )
    {
        throw std :: logic_error ( "AST_Function_Fixture::VerifyErrorMessage : no error" );
    }
}

const KSymbol*
AST_Fixture :: VerifySymbol ( const char* p_name, uint32_t p_type )
{
    AST_FQN * ast = MakeFqn ( p_name );
    const KSymbol* sym = m_builder . Resolve ( * ast );

    if ( sym == 0 )
    {
        throw std :: logic_error ( "AST_Fixture::VerifySymbol : symbol not found" );
    }
    else
    {
        if ( ToCppString ( sym -> name ) !=
                ast -> GetChild ( ast -> ChildrenCount() - 1 ) -> GetTokenValue () )
        {
            throw std :: logic_error ( "AST_Fixture::VerifySymbol : object name mismatch" );
        }
        else if ( sym -> type != p_type )
        {
            throw std :: logic_error ( "AST_Fixture::VerifySymbol : wrong object type" );
        }
    }

    delete ast;

    return sym;
}

const SDatatype*
AST_Fixture :: VerifyDatatype ( const char* p_name, const char* p_baseName, uint32_t p_dim, uint32_t p_size )
{
    const KSymbol* sym = VerifySymbol ( p_name, eDatatype );
    const SDatatype* ret = static_cast < const SDatatype* > ( sym -> u . obj );
    if ( ret -> super == 0 )
    {
        throw std :: logic_error ( "AST_Fixture::VerifyDatatype : super == 0" );
    }
    if ( string ( p_baseName ) != ToCppString ( ret -> super -> name -> name ) )
    {
        throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong baseName" );
    }
    if ( p_dim != ret -> dim )
    {
        throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong dim" );
    }
    if ( p_size != ret -> size )
    {
        throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong size" );
    }
    if ( ret -> super -> domain != ret -> domain )
    {
        throw std :: logic_error ( "AST_Fixture::VerifyDatatype : wrong domain" );
    }
    return ret;
}

bool
AST_Fixture :: OldParse ( const char* p_source )
{
    const VDBManager *mgr;
    if ( VDBManagerMakeRead ( & mgr, 0 ) != 0 )
    {
        throw std :: logic_error ( "AST_Function_Fixture::ParseFunction : VDBManagerMakeRead() failed" );
    }
    if ( VDBManagerMakeSchema ( mgr, & m_schema ) != 0 )
    {
        throw std :: logic_error ( "AST_Function_Fixture::ParseFunction : VDBManagerMakeSchema() failed" );
    }
    // expect an error, do not need to see it
    KWrtHandler* h =  KLogLibHandlerGet ();
    //KLogLibHandlerSet ( NULL, NULL );
    bool ret = VSchemaParseText ( m_schema, 0, p_source, string_size ( p_source ) ) == 0;
    KLogLibHandlerSet ( h -> writer, h -> data );

    VDBManagerRelease ( mgr );

    return ret;
}
