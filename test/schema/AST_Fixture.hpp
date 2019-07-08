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
#ifndef _h_AST_Fixture_
#define _h_AST_Fixture_

/**
* Test fixture class for schema AST
*/

#include <stdint.h>

#include <string>

#include <klib/text.h>

#include "../../libs/schema/ParseTree.hpp"
#include "../../libs/schema/SchemaParser.hpp"
#include "../../libs/schema/ASTBuilder.hpp"

using namespace ncbi::SchemaParser;
#include "../../libs/schema/schema-tokens.h"

#include "../../libs/vdb/schema-parse.h"

struct KSymbol;
struct SDatatype;

// intrinsic type ids as defined in libs/vdb/schema-int.c
const uint32_t U8_id     = 9;
const uint32_t U16_id    = 10;
const uint32_t U32_id    = 11;
const uint32_t U64_id    = 12;
const uint32_t I8_id     = 13;
const uint32_t F32_id    = 17;
const uint32_t F64_id    = 18;
const uint32_t Bool_id   = 19;
const uint32_t ASCII_id  = 23;
const uint32_t UTF8_id   = 24;

class AST_Fixture
{
public:
    AST_Fixture();
    ~AST_Fixture();

    void PrintTree ( const ParseTree& p_tree );

    AST * MakeAst ( const char* p_source );

    void VerifyErrorMessage ( const char* p_source, const char* p_expectedError, uint32_t p_line = 0, uint32_t p_column = 0 );

    enum yytokentype TokenType ( const ParseTree * p_node ) const
    {
        return ( enum yytokentype ) p_node -> GetToken () . GetType ();
    }

    const struct KSymbol* VerifySymbol ( const char* p_name, uint32_t p_type );

    const struct SDatatype* VerifyDatatype ( const char* p_name, const char* p_baseName, uint32_t p_dim, uint32_t p_size );

    static std :: string ToCppString ( const String & p_str)
    {
        return std :: string ( p_str . addr, p_str . len );
    }

    static AST_FQN * MakeFqn ( const char* p_text ); // p_text = (ident:)+ident

    bool OldParse ( const char* p_source );

    const VSchema * GetSchema () const { return m_newParse ? m_builder -> GetSchema () : m_schema; }

    uint32_t Version ( uint32_t p_major, uint32_t p_minor = 0, uint32_t p_release = 0 )
    {
        return ( p_major << 24 ) + ( p_minor << 16 ) + p_release;
    }

    void CreateFile ( const char * p_name, const char * p_content );

    const STable * GetTable ( uint32_t p_idx )
    {
        return static_cast < const STable* > ( VectorGet ( & GetSchema () -> tbl, p_idx ) );
    }

    static void DumpSymbolTable ( const KSymTable & self );
    static void DumpScope ( const BSTree & scope, const char * title = "" );

    // set to true to control debugging output (all false by default)
    bool m_debugParse;
    bool m_printTree;
    bool m_debugAst;
    //

    SchemaParser    m_parser;
    ParseTree *     m_parseTree;
    ASTBuilder *    m_builder;
    AST*            m_ast;

    VSchema *   m_schema;
    bool        m_newParse;

};

// convernience wrapper for Vector ( a KVector of void * )
template <typename T>
class VdbVector
{
public:
    VdbVector( const Vector & p_v ) : m_v ( p_v ) {}
    uint32_t Count() const { return  VectorLength ( & m_v ); }
    const T * Get( uint32_t p_idx ) const
    {
        return static_cast < const T * > ( VectorGet ( & m_v, VectorStart ( & m_v ) + p_idx ) );
    }

    const Vector & m_v;
};

#endif