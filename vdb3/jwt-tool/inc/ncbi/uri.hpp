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

#pragma once

#include <ncbi/secure/except.hpp>
#include <ncbi/secure/string.hpp>

#include <vector>
#include <map>

/**
 * @file ncbi/uri.hpp
 * @brief Uniform Resource Identifier (URI) - RFC 3986
 */

namespace ncbi
{
    /**
     * @typedef URI
     * @brief a Uniform Resource Identifier - RFC 3986 Section 1.
     *
     * A string that identifies a resource.
     */
    typedef String URI;

    /**
     * @typedef StringOrURI
     * @brief a string that may be a URI
     *
     * If the string contains a ':', then it MUST be a URI
     */
    typedef String StringOrURI;

    /**
     * @class URIScheme
     * @brief represents a URI scheme
     */
    class URIScheme
    {
    public:

        enum ID
        {
            // fixed value
            unrecognized = 0,

            // version 1 set of ids
            file,
            ftp,
            http,
            https,
            urn,
            v1_end,

            // add new versions here
            // scheme = v1_end, ...

            id_end = v1_end
        };

        void setName ( const String & name );
        const String & getName () const noexcept;

        ID getID () const noexcept;

        const String & toString () const noexcept;

        URIScheme & operator = ( const URIScheme & s );
        URIScheme & operator = ( URIScheme && s );
        URIScheme ( const URIScheme & s );
        URIScheme ( URIScheme && s );

        URIScheme ();
        ~ URIScheme ();

    private:

        String name;
        ID id;

        static std :: map < String, ID > recognized;
    };

    /**
     * @class URIUser
     * @brief represents description of a user within URI
     */
    class URIUser
    {
    public:

        void setInfo ( const String & info );
        const String & getInfo () const noexcept;

        const String & toString () const noexcept;

        URIUser & operator = ( const URIUser & u );
        URIUser & operator = ( URIUser && u );
        URIUser ( const URIUser & u );
        URIUser ( URIUser && u );

        URIUser ();
        ~ URIUser ();

    private:

        String info;
    };

    /**
     * @class URIHost
     * @brief represents description of a host within URI
     */
    class URIHost
    {
    public:

        void parse ( const String & host );

        String getFQDN () const;

        const std :: vector < String > & getDomainNames () const noexcept;

        const String & getIP () const;
        void setIP ( const unsigned short int ipv6 [ 8 ] );
        void setIP ( const unsigned char ipv4 [ 4 ] );
        void setIP ( unsigned int ipv4_native );

        String toString () const;

        URIHost & operator = ( const URIHost & h );
        URIHost & operator = ( URIHost && h );
        URIHost ( const URIHost & h );
        URIHost ( URIHost && h );

        URIHost ();
        ~ URIHost ();

    private:

        std :: vector < String > fqdn; //!< stored root to leaf; left to right
        String ip;                     //!< holds numeric IP address
        unsigned short int ipv6 [ 8 ]; //!< stored native endian; upper to lower
        unsigned char ipv4 [ 4 ];      //!< stored upper to lower
        bool has_fqdn;
        bool has_ipv6;
        bool has_ipv4;
    };

    /**
     * @class URIPort
     * @brief represents description of a port within URI
     */
    class URIPort
    {
    public:

        void parse ( const String & port );

        const String & getName () const noexcept;
        unsigned int getNumber () const noexcept;

        const String & toString () const;

        URIPort & operator = ( const URIPort & p );
        URIPort & operator = ( URIPort && p );
        URIPort ( const URIPort & p );
        URIPort ( URIPort && p );

        URIPort ();
        ~ URIPort ();

    private:

        String name;
        unsigned int num;
        bool symbolic;
    };

    /**
     * @class URIAuth
     * @brief represents description of an authority within URI
     */
    class URIAuth
    {
    public:

        void parse ( const String & auth );

        URIUser & getUserInfo () noexcept;
        const URIUser & getUserInfo () const noexcept;

        URIHost & getHost () noexcept;
        const URIHost & getHost () const noexcept;

        URIPort & getPort () noexcept;
        const URIPort & getPort () const noexcept;

        String toString () const;

        URIAuth & operator = ( const URIAuth & a );
        URIAuth & operator = ( URIAuth && a );
        URIAuth ( const URIAuth & a );
        URIAuth ( URIAuth && a );

        URIAuth ();
        ~ URIAuth ();

    private:

        URIUser user;
        URIHost host;
        URIPort port;
    };

    /**
     * @class URIPath
     * @brief represents a hierarchical resource path
     */
    class URIPath
    {
    public:

        void parse ( const String & path );

        count_t count () const noexcept;

        const String & getLeaf () const noexcept;
        void setLeaf ( const String & leaf );

        const String & getSegment ( count_t idx ) const;
        void setSegment ( count_t idx, const String & seg );
        void deleteSegment ( count_t idx );

        const std :: vector < String > & getAllSegments () const noexcept;

        String toString () const;

        URIPath & operator = ( const URIPath & p );
        URIPath & operator = ( URIPath && p );
        URIPath ( const URIPath & p );
        URIPath ( URIPath && p );

        URIPath ();
        ~ URIPath ();

    private:

        std :: vector < String > segments;
    };

    /**
     * @class URIQuery
     * @brief represents a set of ( name, value ) pairs of a query
     */
    class URIQuery
    {
    public:

        void parse ( const String & query );

        void add ( const String & name, const String & value );
        void set ( const String & name, const String & value );
        String get ( const String & name ) const;
        const std :: vector < String > & getEach ( const String & name ) const;

        String toString () const;

        URIQuery & operator = ( const URIQuery & q );
        URIQuery & operator = ( URIQuery && q );
        URIQuery ( const URIQuery & q );
        URIQuery ( URIQuery && q );

        URIQuery ();
        ~ URIQuery ();

    private:

        std :: map < String, std :: vector < String > > pairs;

    };

    /**
     * @class URIFrag
     * @brief represents description of a fragment within URI
     */
    class URIFrag
    {
    public:

        void parse ( const String & frag );

        void set ( const String & frag );
        const String & get () const noexcept;

        const String & toString () const noexcept;

        URIFrag & operator = ( const URIFrag & f );
        URIFrag & operator = ( URIFrag && f );
        URIFrag ( const URIFrag & f );
        URIFrag ( URIFrag && f );

        URIFrag ();
        ~ URIFrag ();

    private:

        String frag;
    };

    /**
     * @class UniRsrcID
     * @brief structured Uniform Resource Identifier
     *
     * Intended to provide a canonical representation of a URI.
     */
    class UniRsrcID
    {
    public:

        /*=================================================*
         *                 PUBLIC MEMBERS                  *
         *=================================================*/

        //!< the scheme component
        URIScheme scheme;

        //!< the authority component
        URIAuth auth;

        //!< the path component
        URIPath path;

        //!< the query component
        URIQuery query;

        //!< the fragment component
        URIFrag frag;


        /*=================================================*
         *                   PROCESSING                    *
         *=================================================*/

        void parse ( const URI & uri );

        String toString () const;

        /*=================================================*
         *                   C++ SUPPORT                   *
         *=================================================*/

        UniRsrcID & operator = ( const UniRsrcID & u );
        UniRsrcID & operator = ( UniRsrcID && u );
        UniRsrcID ( const UniRsrcID & u );
        UniRsrcID ( UniRsrcID && u );

        UniRsrcID ();
        ~ UniRsrcID ();

    };
    
    /**
     * @class URIMgr
     * @brief URI Management
     *
     * Globally accessible factory functions
     */
    class URIMgr
    {
    public:

        /**
         * @fn parse
         * @brief parse a URI into a structure
         * @param uri a textual URI to validate
         * @return UriRsrcID a structured URI
         */
        static UniRsrcID parse ( const URI & uri );

        /**
         * @fn validateURI
         * @brief checks a std::string for conformance with RFC3986
         * @param uri a textual URI to validate
         * @exception URIInvalid
         */
        static void validateURI ( const URI & uri );

        /**
         * @fn validateStringOrURI
         * @brief checks a std::string for conformance as a StringOrURI
         * @param str a StringOrURI to validate
         * @exception URIInvalid
         * @exception URIInvalidString
         *
         * While a URI is clearly a string, RFC7515 and RFC7519
         * state that any string containing a colon ( ':' )
         * MUST be a well-formed URI
         */
        static void validateStringOrURI ( const StringOrURI & str );

    private:

        static UniRsrcID parseGeneral ( const URI & uri_str, UniRsrcID & uri, count_t left );

    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/

    DECLARE_SEC_MSG_EXCEPTION ( URIInvalid, InvalidArgument );
    DECLARE_SEC_MSG_EXCEPTION ( URIInvalidString, InvalidArgument );

}
