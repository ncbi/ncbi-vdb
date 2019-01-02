/*==============================================================================
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

#include "json-priv.hpp"

#include <assert.h>

namespace ncbi
{
    std :: string string_to_json ( const std :: string & string )
    {
        std :: string quoted = "\"";
        size_t i, pos, count = string . size ();
        
        for ( i = pos = 0; i < count ; ++ i )
        {
            char ch = string [ i ];
            
            if ( isascii ( ch ) )
            {
                const char * esc = nullptr;
                switch ( ch )
                {
                    case '"':
                        esc = "\\\"";
                        break;
                    case '\\':
                        esc = "\\\\";
                        break;
                    case '\b':
                        esc = "\\b";
                        break;
                    case '\f':
                        esc = "\\f";
                        break;
                    case '\n':
                        esc = "\\n";
                        break;
                    case '\r':
                        esc = "\\r";
                        break;
                    case '\t':
                        esc = "\\t";
                        break;
                    default:
                    {
                        if ( ! isprint ( ch ) )
                        {
                            // unicode escape hex sequence
                            char buff [ 32 ];
                            size_t len = snprintf( buff, sizeof buff, "\\u%04x",
                                                  ( unsigned int ) ( unsigned char ) ch );

                            assert ( len == 6 );
                            
                            quoted += string . substr ( pos, i - pos );
                            pos = i + 1;
                        }
                    }
                }
                if ( esc != nullptr )
                {
                    quoted += string . substr ( pos, i - pos );
                    quoted += esc;
                    
                    pos = i + 1;
                }
            }
        }
        
        if ( pos < count )
            quoted += string . substr ( pos, count - pos );
       
        quoted +=  "\"";
        
        return quoted;
    }

    std :: string JSONPrimitive :: toJSON () const
    {
        return toString ();
    }
    
    std :: string JSONBoolean :: toString () const
    {
        return std :: string ( value ? "true" : "false" );
    }

    std :: string JSONInteger :: toString () const
    {
        return std :: to_string ( value );
    }

    std :: string JSONString :: toJSON () const
    {
        return string_to_json ( value );
    }
}
