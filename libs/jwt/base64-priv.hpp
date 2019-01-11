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

#ifndef _hpp_ncbi_oauth_base64_priv_
#define _hpp_ncbi_oauth_base64_priv_

#include <jwt/jwt-string.hpp>

namespace ncbi
{
    class Base64Payload
    {
    public:

        unsigned char * data ()
        { return buff; }
        const unsigned char * data () const
        { return buff; }

        size_t size () const
        { return sz; }

        size_t capacity () const
        { return cap; }

        void setSize ( size_t amt );
        void increaseCapacity ( size_t amt = 256 );

        Base64Payload ();
        Base64Payload ( size_t initial_capacity );
        Base64Payload ( const Base64Payload & payload );
        Base64Payload & operator = ( const Base64Payload & payload );
        ~ Base64Payload ();

    private:

        mutable unsigned char * buff;
        mutable size_t sz, cap;
    };

    const JwtString encodeBase64 ( const void * data, size_t bytes );
    const Base64Payload decodeBase64 ( const JwtString &encoding );
    const JwtString decodeBase64String ( const JwtString &encoding );

    const JwtString encodeBase64URL ( const void * data, size_t bytes );
    const Base64Payload decodeBase64URL ( const JwtString &encoding );
    const JwtString decodeBase64URLString ( const JwtString &encoding );

} // namespace ncbi

#endif /* _hpp_ncbi_oauth_base64_priv_ */
