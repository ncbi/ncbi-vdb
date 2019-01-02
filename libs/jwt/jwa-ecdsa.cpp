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

#if 0

#include <jwt/jwa.hpp>
#include <jwt/jwt.hpp>
#include "base64-priv.hpp"

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/sha256.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    /*
     +-------------------+-------------------------------+
     | "alg" Param Value | Digital Signature Algorithm   |
     +-------------------+-------------------------------+
     | ES256             | ECDSA using P-256 and SHA-256 |
     | ES384             | ECDSA using P-384 and SHA-384 |
     | ES512             | ECDSA using P-521 and SHA-512 |
     +-------------------+-------------------------------+
     */
    struct ECDSA_Signer : JWASigner
    {
        virtual std :: string sign ( const void * data, size_t bytes ) const
        {
            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            // Compute hash
            unsigned char hash [ 32 ];
            if ( mbedtls_md ( info, ( const unsigned char * ) data, bytes, hash ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to compute hash" );

            size_t dlen;
            unsigned char digest [ 512 / 8 ];
            if ( mbedtls_ecdsa_write_signature ( &ctx, md_type, hash, sizeof hash,
                                                digest, & dlen, mbedtls_ctr_drbg_random, & ctr_drbg ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to extract digest" );

            // encode as base64url
            return encodeBase64URL ( digest, dsize );
        }

        virtual JWASigner * clone () const
        {
            return new ECDSA_Signer ( alg, nam, key, md_type );
        }

        ECDSA_Signer ( const std :: string & name, const std :: string & alg,
                      const std :: string & key, mbedtls_md_type_t type )
        : JWASigner ( name, alg, key )
        , ctx ( cctx )
        , ctr_drbg ( cctr_drbg )
        , entropy ( centropy )
        , md_type ( type )
        {
            mbedtls_ecdsa_init( & ctx );
            mbedtls_ctr_drbg_init( & ctr_drbg );
            mbedtls_entropy_init( & entropy );

            if ( ( mbedtls_ctr_drbg_seed ( &ctr_drbg, mbedtls_entropy_func,
                                          &entropy, NULL, NULL ) ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to generate random seed" );
        }

        ~ ECDSA_Signer ()
        {
            mbedtls_ecdsa_free ( & ctx );
            mbedtls_ctr_drbg_free ( & ctr_drbg );
            mbedtls_entropy_free( & entropy );
        }

        mbedtls_ecdsa_context cctx, & ctx;
        mbedtls_ctr_drbg_context cctr_drbg, & ctr_drbg;
        mbedtls_entropy_context centropy, & entropy;
        mbedtls_md_type_t md_type;
    };

    struct ECDSA_Verifier : JWAVerifier
    {
        virtual bool verify ( const void * data, size_t bytes, const std :: string & sig_base64 ) const
        {
            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            // Compute hash
            unsigned char hash [ 32 ];
            if ( mbedtls_md ( info, ( const unsigned char * ) data, bytes, hash ) != 0 )
                return false;

            unsigned char digest [ 512 / 8 ];
            if ( mbedtls_ecdsa_read_signature ( &ctx, hash, sizeof hash,
                                                ( const unsigned char * ) digest, sizeof digest ) != 0 )
                return false;

            Base64Payload signature = decodeBase64URL ( sig_base64 );

            if ( signature . size () != dsize )
                return false;

            // the digest must match
            if ( memcmp ( digest, signature . data (), dsize ) != 0 )
                return false;


            return true;
        }

        virtual JWAVerifier * clone () const
        {
            return new ECDSA_Verifier ( nam, alg, key, md_type );
        }

        ECDSA_Verifier ( const std :: string & name, const std :: string & alg,
                        const std :: string & key, mbedtls_md_type_t type )
        : JWAVerifier ( name, alg, key )
        , ctx ( cctx )
        , md_type ( type )
        {
            mbedtls_ecdsa_init( & ctx );
        }

        ~ ECDSA_Verifier ()
        {
            mbedtls_ecdsa_free ( & ctx );
        }

        size_t dsize;
        mbedtls_ecdsa_context cctx, & ctx;
        mbedtls_md_type_t md_type;
    };

    struct ECDSA_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const std :: string & name,
                                  const std :: string & alg, const std :: string & key ) const
        {
            return new ECDSA_Signer ( name, alg, key, md_type );
        }

        ECDSA_SignerFact ( const std :: string & alg, mbedtls_md_type_t type )
        {
            gJWAFactory . registerSignerFact ( alg, this );
        }

        mbedtls_md_type_t md_type;
    };

    struct ECDSA_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const std :: string & name,
                                    const std :: string & alg, const std :: string & key ) const
        {
            return new ECDSA_Verifier ( name, alg, key, md_type );
        }

        ECDSA_VerifierFact ( const std :: string & alg, mbedtls_md_type_t md_type )
        {
            gJWAFactory . registerVerifierFact ( alg, this );
        }

        mbedtls_md_type_t md_type;
    };


    static struct ECDSA_Registry
    {
        ECDSA_Registry ( const std :: string & alg, mbedtls_md_type_t md_type )
        : signer_fact ( alg, md_type )
        , verifier_fact ( alg, md_type )
        {
        }

        ECDSA_SignerFact signer_fact;
        ECDSA_VerifierFact verifier_fact;

    } es256 ( "ES256", MBEDTLS_MD_SHA256 ),
      es384 ( "ES384", MBEDTLS_MD_SHA384 ),
      es512 ( "ES512", MBEDTLS_MD_SHA512 );

    void includeJWA_ecdsa ( bool always_false )
    {
        if ( always_false )
        {
            std :: string empty;
            es256 . signer_fact . make ( empty, empty, empty );
            es384 . signer_fact . make ( empty, empty, empty );
            es512 . signer_fact . make ( empty, empty, empty );
        }
    }
}
#endif
