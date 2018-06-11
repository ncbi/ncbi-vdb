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

#ifndef _hpp_AST_
#define _hpp_AST_

#include "ParseTree.hpp"

#include <klib/symtab.h>

#include <vdb/schema.h>

struct SExpression;
struct VTypedecl;

namespace ncbi
{
    namespace SchemaParser
    {
        class ASTBuilder;

        class AST : public ParseTree
        {
        public:
            AST ( const Token* token );
            AST ( Token :: TokenType tokenType ); // no-value token
            explicit AST (); // no token; a synthesized node

            // convenience overloads; chilren nodes cannot be NULL
            AST ( const Token* token, AST* );
            AST ( const Token* token, AST*, AST* );
            AST ( const Token* token, AST*, AST*, AST* );
            AST ( const Token* token, AST*, AST*, AST*, AST * );
            AST ( const Token* token, AST*, AST*, AST*, AST *, AST * );
            AST ( const Token* token, AST*, AST*, AST*, AST *, AST *, AST* );

            void AddNode ( AST * ); // allocate with new
            void AddNode ( const Token * );

            const AST* GetChild ( uint32_t idx ) const  { return static_cast < const AST * > ( ParseTree :: GetChild ( idx ) ); }
                  AST* GetChild ( uint32_t idx )        { return static_cast <       AST * > ( ParseTree :: GetChild ( idx ) ); }

            Token :: TokenType GetTokenType () const { return GetToken () . GetType (); }
            const char* GetTokenValue () const { return GetToken () . GetValue (); }
        };

        class AST_FQN : public AST
        {
        public:
            AST_FQN ( const Token* ); // always PT_IDENT

            void SetVersion ( const char* ); // version specified as "#maj[.min[.rel]]]"
            uint32_t GetVersion () const { return m_version; } // encoded as ( maj << 24 ) | ( min << 16 ) | ( rel )

            uint32_t NamespaceCount() const;
            void GetNamespace ( uint32_t idx, String & ) const;
            void GetIdentifier ( String & ) const;

            // reconstruct the full name "ns1:ns2:...:ident", 0-terminated. Returns size in bytes
            void GetFullName ( char* buf, size_t bufSize ) const;
            void GetPartialName ( char* buf, size_t bufSize, uint32_t lastMember ) const;

        private:
            uint32_t m_version;
        };

        // these conversion function will assert if the argument is NULL or not an AST_FQN,
        // otherwise guarantee to return a non-NULL AST_FQN*
        extern AST_FQN * ToFQN ( AST * p_ast);
        extern const AST_FQN * ToFQN ( const AST * p_ast);

        class AST_Expr : public AST
        {
        public:
            AST_Expr ( const Token* );  // literal constant
            AST_Expr ( AST_FQN* );      // fully qualified name
            AST_Expr ( AST_Expr* );     // first sub-expression in a conditional ( ex1 | ex2 | ... )
            AST_Expr ( Token :: TokenType );    // '@' etc

            // these methods report problems
            SExpression * EvaluateConst ( ASTBuilder & ) const;
            SExpression * MakeExpression ( ASTBuilder & ) const;
            SExpression * MakeSymExpr ( ASTBuilder & , const KSymbol * p_sym ) const;
            SExpression * MakeUnsigned ( ASTBuilder & ) const;

        private:
            SExpression * MakeFloat ( ASTBuilder & ) const;
            SExpression * MakeString ( ASTBuilder & p_builder ) const;
            SExpression * MakeEscapedString ( ASTBuilder & p_builder ) const;
            SExpression * MakeVectorConstant ( ASTBuilder & p_builder ) const;
            SExpression * MakeBool ( ASTBuilder & ) const;
            SExpression * MakeNegate ( ASTBuilder & ) const;
            SExpression * MakeCast ( ASTBuilder & p_builder ) const;
            SExpression * MakeMember ( ASTBuilder & p_builder ) const;
            SExpression * MakeJoin ( ASTBuilder & p_builder ) const;
        };

        // these conversion function will assert if the argument is NULL or not an AST_Expr,
        // otherwise guarantee to return a non-NULL AST_Expr*
        extern AST_Expr * ToExpr ( AST * p_ast);
        extern const AST_Expr * ToExpr ( const AST * p_ast);

    }
}

#endif
