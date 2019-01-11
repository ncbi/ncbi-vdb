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

#include <jwt/jwa.hpp>
#include <jwt/jwk.hpp>
#include <jwt/jwt.hpp>
#include "base64-priv.hpp"

#include <mbedtls/md.h>
#include <mbedtls/error.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    static
    JwtString mbedtls_error ( int err )
    {
        char buffer [ 256 ];
        vdb_mbedtls_strerror ( err, buffer, sizeof buffer );
        return JwtString ( buffer );
    }

    static
    JWTException MBEDTLSException ( const char * func, unsigned int line, int err, const char * msg )
    {
        JwtString what ( msg );
        what += ": ";
        what += mbedtls_error ( err );
        return JWTException ( func, line, what . c_str () );
    }

    struct HMAC_Signer : JWASigner
    {
        virtual JwtString sign ( const void * data, size_t bytes ) const
        {
            // hash the data
            vdb_mbedtls_md_hmac_update ( & ctx, ( const unsigned char * ) data, bytes );

            // extract the digest - maximum size is 512 bits
            unsigned char digest [ 512 / 8 ];
            assert ( sizeof digest >= dsize );
            vdb_mbedtls_md_hmac_finish ( & ctx, digest );

            // reset the context for the next signature
            vdb_mbedtls_md_hmac_reset ( & ctx );

            // encode as base64url
            return encodeBase64URL ( digest, dsize );
        }

        virtual JWASigner * clone () const
        {
            return new HMAC_Signer ( alg, nam, key, md_type );
        }

        HMAC_Signer ( const JwtString & name, const JwtString & alg,
                const JWK * key, mbedtls_md_type_t type )
            : JWASigner ( name, alg, key )
            , ctx ( cctx )
            , md_type ( type )
        {
            // simple context initialization
            vdb_mbedtls_md_init ( & ctx );

            // get info from the type
            const mbedtls_md_info_t * info = vdb_mbedtls_md_info_from_type ( md_type );
            dsize = vdb_mbedtls_md_get_size ( info );

            // TBD - verify key size against digest size

            // allocates internal structures and binds to "info"
            int status = vdb_mbedtls_md_setup ( & ctx, info, 1 );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to setup HMAC context" );

            // bind the key to the context
            JwtString kval = static_cast < const HMAC_JWKey * > ( key ) -> getValue ();
            status = vdb_mbedtls_md_hmac_starts ( & ctx, ( const unsigned char * ) kval . data (), kval . size () );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to bind key to HMAC context" );
        }

        ~ HMAC_Signer ()
        {
            vdb_mbedtls_md_free ( & ctx );
        }

        size_t dsize;
        mbedtls_md_context_t cctx, & ctx;
        mbedtls_md_type_t md_type;
    };

    struct HMAC_Verifier : JWAVerifier
    {
        virtual bool verify ( const void * data, size_t bytes, const JwtString & sig_base64 ) const
        {
            // hash the data
            vdb_mbedtls_md_hmac_update ( & ctx, ( const unsigned char * ) data, bytes );

            // extract the digest - maximum size is 512 bits
            unsigned char digest [ 512 / 8 ];
            assert ( sizeof digest >= dsize );
            vdb_mbedtls_md_hmac_finish ( & ctx, digest );

            // reset the context for the next signature
            vdb_mbedtls_md_hmac_reset ( & ctx );

            // decode the base64url signature
            Base64Payload signature = decodeBase64URL ( sig_base64 );

            // test: the lengths must match
            if ( signature . size () != dsize )
                return false;

            // the digest must match
            if ( memcmp ( digest, signature . data (), dsize ) != 0 )
                return false;

            // signature verified
            return true;
        }

        virtual JWAVerifier * clone () const
        {
            return new HMAC_Verifier ( nam, alg, key, md_type );
        }

        HMAC_Verifier ( const JwtString & name, const JwtString & alg,
                const JWK * key, mbedtls_md_type_t type )
            : JWAVerifier ( name, alg, key )
            , ctx ( cctx )
            , md_type ( type )
        {
            // simple context initialization
            vdb_mbedtls_md_init ( & ctx );

            // get info from the type
            const mbedtls_md_info_t * info = vdb_mbedtls_md_info_from_type ( md_type );
            dsize = vdb_mbedtls_md_get_size ( info );

            // selects the digest algorithm and allocates internal structures
            int status = vdb_mbedtls_md_setup ( & ctx, info, 1 );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to setup HMAC context" );

            // bind the key to the context
            JwtString kval = static_cast < const HMAC_JWKey * > ( key ) -> getValue ();
            status = vdb_mbedtls_md_hmac_starts ( & ctx, ( const unsigned char * ) kval . data (), kval . size () );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to bind key to HMAC context" );
        }

        ~ HMAC_Verifier ()
        {
            vdb_mbedtls_md_free ( & ctx );
        }

        size_t dsize;
        mbedtls_md_context_t cctx, & ctx;
        mbedtls_md_type_t md_type;
    };

    struct HMAC_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const JwtString & name,
            const JwtString & alg, const JWK * key ) const
        {
            if ( key -> getType () . compare ( "oct" ) != 0 )
                throw JWTException ( __func__, __LINE__, "bad key type" );

            return new HMAC_Signer ( name, alg, key, md_type );
        }

        HMAC_SignerFact ( const JwtString & alg, mbedtls_md_type_t type )
            : md_type ( type )
        {
            gJWAFactory . registerSignerFact ( alg, this );
        }

        mbedtls_md_type_t md_type;
    };

    struct HMAC_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const JwtString & name,
            const JwtString & alg, const JWK * key ) const
        {
            if ( key -> getType () . compare ( "oct" ) != 0 )
                throw JWTException ( __func__, __LINE__, "bad key type" );

            return new HMAC_Verifier ( name, alg, key, md_type );
        }

        HMAC_VerifierFact ( const JwtString & alg, mbedtls_md_type_t type )
            : md_type ( type )
        {
            gJWAFactory . registerVerifierFact ( alg, this );
        }

        mbedtls_md_type_t md_type;
    };

    static struct HMAC_Registry
    {
        HMAC_Registry ( const JwtString & alg, mbedtls_md_type_t md_type )
            : signer_fact ( alg, md_type )
            , verifier_fact ( alg, md_type )
        {
        }

        HMAC_SignerFact signer_fact;
        HMAC_VerifierFact verifier_fact;

    } hs256 ( "HS256", MBEDTLS_MD_SHA256 ),
      hs384 ( "HS384", MBEDTLS_MD_SHA384 ),
      hs512 ( "HS512", MBEDTLS_MD_SHA512 );

    void includeJWA_hmac ( bool always_false )
    {
        if ( always_false )
        {
            JwtString empty;
            hs256 . signer_fact . make ( empty, empty, nullptr );
            hs384 . signer_fact . make ( empty, empty, nullptr );
            hs512 . signer_fact . make ( empty, empty, nullptr );
        }
    }
}
