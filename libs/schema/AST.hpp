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

#include <klib/text.h>

struct SExpression;
struct KSymbol;

namespace ncbi
{
    namespace SchemaParser
    {
        class ASTBuilder;

        class AST : public ParseTree
        {
        public:
            static const char * TokenTypeToString( Token :: TokenType );

        public:
            static AST * Make ( ctx_t ctx,  const Token* token );
            static AST * Make ( ctx_t ctx, Token :: TokenType tokenType ); // no-value token
            static AST * Make ( ctx_t ctx ); // no token; a synthesized node

            static void Destroy ( AST * );

            // convenience overloads; chilren nodes cannot be NULL
            static AST * Make ( ctx_t ctx, const Token* token, AST* );
            static AST * Make ( ctx_t ctx, const Token* token, AST*, AST* );
            static AST * Make ( ctx_t ctx, const Token* token, AST*, AST*, AST* );
            static AST * Make ( ctx_t ctx, const Token* token, AST*, AST*, AST*, AST * );
            static AST * Make ( ctx_t ctx, const Token* token, AST*, AST*, AST*, AST *, AST * );
            static AST * Make ( ctx_t ctx, const Token* token, AST*, AST*, AST*, AST *, AST *, AST* );

            void AddNode ( ctx_t ctx, AST * );
            void AddNode ( ctx_t ctx, const Token * );

            const AST* GetChild ( uint32_t idx ) const  { return static_cast < const AST * > ( ParseTree :: GetChild ( idx ) ); }
                  AST* GetChild ( uint32_t idx )        { return static_cast <       AST * > ( ParseTree :: GetChild ( idx ) ); }

            Token :: TokenType GetTokenType () const { return GetToken () . GetType (); }
            const char* GetTokenValue () const { return GetToken () . GetValue (); }

        protected:
            AST ( const Token* token );
            AST ( Token :: TokenType tokenType ); // no-value token
            explicit AST (); // no token; a synthesized node
            virtual ~AST(); // call Destroy() instead
        };

        class AST_FQN : public AST
        {
        public:
            static AST_FQN * Make ( ctx_t ctx, const Token* ); // always PT_IDENT

            void SetVersion ( const char* ); // version specified as "#maj[.min[.rel]]]"
            uint32_t GetVersion () const { return m_version; } // encoded as ( maj << 24 ) | ( min << 16 ) | ( rel )

            uint32_t NamespaceCount() const;
            void GetNamespace ( uint32_t idx, String & ) const;
            void GetIdentifier ( String & ) const;

            // reconstruct the full name "ns1:ns2:...:ident", 0-terminated. Returns size in bytes
            void GetFullName ( char* buf, size_t bufSize ) const;
            void GetPartialName ( char* buf, size_t bufSize, uint32_t lastMember ) const;
            void GetVersionedName ( char* buf, size_t bufSize ) const;

        protected:
            AST_FQN ( const Token* ); // always PT_IDENT
            ~AST_FQN(); // call AST::Destroy() instead

        private:
            uint32_t m_version;
        };

        // these conversion function will assert if the argument is NULL or not an AST_FQN,
        // otherwise guarantee to return a non-NULL AST_FQN*
        extern AST_FQN * ToFQN ( AST * p_ast);
        extern const AST_FQN * ToFQN ( const AST * p_ast);

    }
}

#endif
