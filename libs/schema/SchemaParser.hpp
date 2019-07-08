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

#ifndef _hpp_SchemaParser_
#define _hpp_SchemaParser_

#include <kfs/file.h>

#include "ErrorReport.hpp"

namespace ncbi
{
    namespace SchemaParser
    {
        class ParseTree;

        class SchemaParser
        {
        public:
            SchemaParser ();
            ~SchemaParser ();

            bool ParseString ( const char * input, bool debug = false );
            bool ParseFile ( const struct KFile * file, const char * fileName = 0 );

            const ParseTree* GetParseTree () const { return m_root; }
                  ParseTree* MoveParseTree (); // Transfer ownership to caller; destroy with delete

            const ErrorReport & GetErrors () const { return m_errors; }

        private:
            bool m_debug;
            ParseTree* m_root;

            ErrorReport m_errors;
        };
    }
}

#endif
