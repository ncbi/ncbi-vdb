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

#ifndef _hpp_ParseTree_
#define _hpp_ParseTree_

#include <klib/vector.h>

#include "Token.hpp"

namespace ncbi
{
    namespace SchemaParser
    {
        class ParseTree
        {
        public:
            ParseTree ( const Token& token );
            virtual ~ParseTree ();

            void AddChild ( ParseTree * ); // make sure is allocated with new

            const Token & GetToken () const { return m_token; }

            uint32_t ChildrenCount () const;

            const ParseTree* GetChild ( uint32_t idx ) const;
                  ParseTree* GetChild ( uint32_t idx );

            const Token :: Location & GetLocation () const { return * m_location; } // location of the leading real token

        protected:
            void MoveChildren ( ParseTree& );
            void SetToken ( const Token & p_token ) {  m_token = p_token; }

        private:
            Token                       m_token;
            Vector                      m_children;
            const Token :: Location *   m_location;
        };

        class ParseTreeScanner
        {
        public:
            ParseTreeScanner ( const ParseTree& p_root, const char * p_sourceFileName = "" );
            ~ParseTreeScanner ();

            // returns token type, SchemaScanner::EndSource at the end of the walk; the token itself is in p_token
            // for container nodes, this will create a bracketed TAG '(' contents ')' sequence.
            Token :: TokenType NextToken ( const Token*& p_token );

            const char * GetSourceFileName () const { return m_source; }

        private:
            void PushNode ( const ParseTree* );

        private:
            char *  m_source;
            Vector  m_stack;
        };
    }
}

#endif
