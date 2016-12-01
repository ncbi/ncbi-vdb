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

#include "Token.hpp"

namespace ncbi
{
    namespace SchemaParser
    {
        class ParseTree
        {
        public:
            virtual ~ParseTree ();

            void AddChild(ParseTree*);

            unsigned int ChildrenCount() const;
            ParseTree* GetChild(unsigned int idx);
            const ParseTree* GetChild(unsigned int idx) const;

        protected:
            ParseTree ();
        };

        class TokenNode : public ParseTree
        {
        public:
            TokenNode(const Token& token);
            virtual ~TokenNode();

            const Token& GetToken() const { return m_token; }

        private:
            Token m_token;
        };

        class RuleNode : public ParseTree
        {
        public:
            RuleNode(const char* name);
            virtual ~RuleNode();

        private:
            char* m_name;
        };
    }
}

#endif
