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

#ifndef _hpp_ErrorReport_
#define _hpp_ErrorReport_

#include <klib/vector.h>

#include "Token.hpp"

namespace ncbi
{
    namespace SchemaParser
    {
        class ErrorReport
        {
        public:
            typedef Token :: Location Location;

            struct Error
            {
                char *    m_message;
                char *    m_file;
                uint32_t  m_line;
                uint32_t  m_column;

                Error( const char * p_message, const ErrorReport :: Location & p_location );
                ~Error();

                bool Format ( char * p_buf, size_t p_bufSize ) const;
            };

        public:
            ErrorReport ();
            ~ErrorReport ();

            void ReportError ( const Location & p_loc, const char* p_fmt, ... );
            void ReportInternalError ( const char * p_source, const char* p_fmt, ... );

            uint32_t GetCount() const { return VectorLength ( & m_errors ); }

            const Error * GetError ( uint32_t p_idx ) const;
            const char *  GetMessageText ( uint32_t p_idx ) const; // if not interested in location

            void Clear ();

        private:
            Vector      m_errors;
        };
    }
}

#endif
