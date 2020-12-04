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

#ifndef _hpp_ASTBuilder_func_
#define _hpp_ASTBuilder_func_

#include "ASTBuilder.hpp"

#include "../vdb/schema-priv.h"

namespace ncbi
{
    namespace SchemaParser
    {

        class FunctionDeclaration // Wrapper around SFunction
        {
        public:
            FunctionDeclaration ( ASTBuilder & p_builder );
            FunctionDeclaration ( ASTBuilder & p_builder, SFunction & ); // physical function; will not be overloaded or added to symtab
            ~FunctionDeclaration ();

            BSTree * SchemaScope () { return m_self == 0 ? 0 : & m_self -> sscope; }
            BSTree * FunctionScope () { return m_self == 0 ? 0 : & m_self -> fscope; }

            bool SetName ( const AST_FQN &  p_fqn,
                        uint32_t         p_type,
                        bool             p_canOverload,
                        bool             p_validate );

            void SetSchemaParams ( const AST & p_sig );
            void SetReturnType ( STypeExpr * p_retType );
            void SetFactoryParams ( const AST & p_sig );
            void SetFormalParams ( const AST & p_sig );
            void SetPhysicalParams ();
            void SetPrologue ( const AST & p_prologue );

            void HandleScript ( const AST & p_body, const String & p_funcName );

            SFunction * GetSFunction () { return m_self; }

        private:
            bool HandleOverload ( const AST_FQN &  p_fqn, const KSymbol *  p_priorDecl );
            void AddFactoryParams ( Vector& p_sig, const AST & p_params );
            void AddFormalParams ( Vector& p_sig, const AST & p_params );
            SIndirectType * MakeSchemaParamType ( const AST_FQN & p_name );
            SIndirectConst * MakeSchemaParamConst ( const AST_FQN & p_name );
            void HandleStatement ( const AST & p_stmt );

        private:
            ASTBuilder &    m_builder;
            SFunction *     m_self;
            bool            m_destroy;
        };

    }
}

#endif
