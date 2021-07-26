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
 *  Author: Kurt Rodarmer
 *
 * ===========================================================================
 *
 */

#include <ncbi/uri.hpp>

#include <cassert>

namespace ncbi
{
#if 0

    const size_t uri_size_limit = 64 * 1024;
    const size_t scheme_length_limit = 32;

    UriRsrcID URIMgr :: parse ( const URI & uri_str )
    {
        // RFC 3986 Section 2.
        // "The ABNF notation defines its terminal values to be non-negative
        //  integers (codepoints) based on the US-ASCII coded character set [ASCII]."
        if ( ! uri_str . isAscii () )
            throw URIInvalid ( XP ( XLOC ) << "URI has non-ASCII characters" );

        // the overall URI should be a reasonable size
        if ( uri_str . size () > uri_size_limit )
        {
            throw SizeViolation (
                XP ( XLOC )
                << "URI has size "
                << uri_str . size ()
                << " bytes which exceeds the limit of "
                << uri_size_limit
                );
        }

        // start to split into parts, beginning with schema
        count_t delim = uri_str . find ( ':' );
        if ( delim == uri_str . npos )
            throw URIInvalid ( XP ( XLOC ) << "URI has no scheme" );

        // check scheme length
        if ( delim > scheme_length_limit )
        {
            throw LengthViolation (
                XP ( XLOC )
                << "URI has schema of length "
                << delim
                << " characters which exceeds the limit of "
                << scheme_length_limit
                );
        }

        // the structured URI will be built piece by piece
        UriRsrcID uri;

        // start off with the scheme
        uri . scheme . setName ( uri_str . subString ( 0, delim ) );
        switch ( uri . scheme . getID () )
        {
        case URIScheme :: file:
        case URIScheme :: ftp:
        case URIScheme :: urn:
            break;
        default:
            return parseGeneral ( uri_str, uri, delim + 1 );
        }

        throw UnsupportedException (
            XP ( XLOC )
            << "the URI scheme '"
            << uri . scheme . getName ()
            << "' is not currently supported"
            );
    }

    void URIMgr :: parseGeneral ( const URI & uri_str, UriRsrcID & uri, count_t left )
    {
        count_t xright = uri_str . length ();

        // RFC 3986 Section 3
        // URI = scheme ':' hier-part [ '?' query ] [ '#' fragment ]

        // look for fragment
        count_t delim = uri_str . rfind ( '#', xright, xright - left );
        if ( delim != uri_str . npos )
        {
            uri . frag . parse ( uri_str . subString ( delim + 1 ) );
            xright = delim;
        }

        // look for query
        delim = uri_str . rfind ( '?', xright, xright - left );
        if ( delim != uri_str . npos )
        {
            uri . query . parse ( uri_str . subString ( delim + 1, xright - delim - 1 ) );
            xright = delim;
        }

        // the hierarchical part
        // RFC 3986 Section 3
        // hier-part
        //    = '//' authority [ path-absolute ]
        //    | path-absolute
        //    | path-rootless
        //    | path-empty
        //    ;
        // path-absolute
        //    = '/' segment-nz *( '/' segment )
        //    ;
        // path-rootless
        //    = segment-nz *( '/' segment )
        //    ;
        // path-empty
        //    =
        //    ;
        // segment-nz
        //    = 1*pchar
        //    ;
        // segment
        //    = *pchar
        //    ;
    }
#endif

    void URIMgr :: validateURI ( const URI & uri_str )
    {
#if 0
        try
        {
            parse ( uri_str );
        }
        catch ( URIInvalid & x )
        {
            throw;
        }
        catch ( InvalidArgument & x )
        {
            throw URIInvalid (
                XP ( XLOC )
                << xwhat
                << x . what ()
                << xcause
                << x . cause ()
                << xsuggest
                << x . suggestion ()
                );
        }
#endif
    }

    void URIMgr :: validateStringOrURI ( const StringOrURI & str )
    {
        if ( str . isEmpty () )
            throw URIInvalidString ( XP ( XLOC ) << "string is empty" );
        if ( str . find ( ':' ) != str . npos )
            validateURI ( str );
    }
}
