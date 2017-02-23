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

// hide an unfortunately named C function
#define typename __typename
#include "../../libs/vdb/schema-parse.h"
#undef typename

#define KW_TOKEN(v,k) SchemaToken v = { KW_##k, #k, strlen(#k), 0, 0 }

struct KSymbol;
struct SDatatype;

// intrinsic type ids as defined in libs/vdb/schema-int.c
const uint32_t U8_id     = 9;
const uint32_t U16_id    = 10;
const uint32_t U32_id    = 11;

class AST_Fixture
{
public:
    AST_Fixture();
    ~AST_Fixture();

    void PrintTree ( const ParseTree& p_tree );

    AST * MakeAst ( const char* p_source,
                    bool p_debugParse = false,
                    bool p_printTree = false,
                    bool p_debugAst = false );

    void VerifyErrorMessage ( const char* p_source, const char* p_expectedError );

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

    SchemaParser    m_parser;
    ParseTree *     m_parseTree;
    ASTBuilder      m_builder;
    AST*            m_ast;
};

#endif