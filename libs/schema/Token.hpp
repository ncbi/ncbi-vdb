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

#ifndef _hpp_Token_
#define _hpp_Token_

#include "SchemaScanner.hpp"

namespace ncbi
{
    namespace SchemaParser
    {
        class Token
        {
        public:
            typedef SchemaScanner :: TokenType TokenType;

        public:
            Token ( const SchemaToken& );
            Token ( TokenType );
            Token ( const Token& );
            ~Token ();

            TokenType GetType () const { return m_type; }
            const char* GetValue () const               { return m_value == 0 ? "" : m_value; }
            const char* GetLeadingWhitespace () const   { return m_ws == 0 ? "" : m_ws; }

            bool IsFake() const { return m_value == 0; }

        private:
            TokenType m_type;
            char* m_value;
            char* m_ws;
        };
    }
}

#endif
