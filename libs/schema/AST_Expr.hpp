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

#ifndef _hpp_AST_Expr_
#define _hpp_AST_Expr_

#include "AST.hpp"

namespace ncbi
{
    namespace SchemaParser
    {
        class AST_Expr : public AST
        {
        public:
            static AST_Expr * Make ( ctx_t ctx, const Token* ); // literal constant
            static AST_Expr * Make ( ctx_t ctx, AST_FQN* );      // fully qualified name
            static AST_Expr * Make ( ctx_t ctx, AST_Expr* );     // first sub-expression in a conditional ( ex1 | ex2 | ... )
            static AST_Expr * Make ( ctx_t ctx, Token :: TokenType );    // '@' etc

            // these methods report problems
            SExpression * EvaluateConst ( ctx_t ctx, ASTBuilder & ) const;
            SExpression * MakeExpression ( ctx_t ctx, ASTBuilder & ) const;
            SExpression * MakeSymExpr ( ctx_t ctx, ASTBuilder & , const KSymbol * p_sym ) const;
            SExpression * MakeUnsigned ( ctx_t ctx, ASTBuilder & ) const;

        protected:
            AST_Expr ( const Token* );
            AST_Expr ();
            ~AST_Expr(); // call AST::Destroy() instead

        private:
            SExpression * MakeFloat ( ctx_t ctx, ASTBuilder & ) const;
            SExpression * MakeString ( ctx_t ctx, ASTBuilder & p_builder ) const;
            SExpression * MakeEscapedString ( ctx_t ctx, ASTBuilder & p_builder ) const;
            SExpression * MakeVectorConstant ( ctx_t ctx, ASTBuilder & p_builder ) const;
            SExpression * MakeBool ( ctx_t ctx, ASTBuilder & ) const;
            SExpression * MakeNegate ( ctx_t ctx, ASTBuilder & ) const;
            SExpression * MakeCast ( ctx_t ctx, ASTBuilder & p_builder ) const;
            SExpression * MakeMember ( ctx_t ctx, ASTBuilder & p_builder ) const;
            SExpression * MakeJoin ( ctx_t ctx, ASTBuilder & p_builder ) const;
        };

        // these conversion function will assert if the argument is NULL or not an AST_Expr,
        // otherwise guarantee to return a non-NULL AST_Expr*
        extern AST_Expr * ToExpr ( AST * p_ast);
        extern const AST_Expr * ToExpr ( const AST * p_ast);
    }
}

#endif
