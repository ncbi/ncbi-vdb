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
#include <mbedtls/rsa.h>
#include <mbedtls/error.h>
#include <mbedtls/bignum.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    static
    std :: string mbedtls_error ( int err )
    {
        char buffer [ 256 ];
        vdb_mbedtls_strerror ( err, buffer, sizeof buffer );
        return std :: string ( buffer );
    }

    static
    JWTException MBEDTLSException ( const char * func, unsigned int line, int err, const char * msg )
    {
        std :: string what ( msg );
        what += ": ";
        what += mbedtls_error ( err );
        return JWTException ( func, line, what . c_str () );
    }

    static
    void mpiRead ( mbedtls_mpi & mpi, const std :: string & val )
    {
        // the string is in base64url encoding
        Base64Payload raw = decodeBase64URL ( val );

        // presumably, it can just be read into the multi-precision number
        int status = vdb_mbedtls_mpi_read_binary ( & mpi, ( const unsigned char * ) raw . data (), raw . size () );
        if ( status != 0 )
            throw MBEDTLSException ( __func__, __LINE__, status, "failed to read RSA key data" );
    }

    /*
     +-------------------+---------------------------------+
     | "alg" Param Value | Digital Signature Algorithm     |
     +-------------------+---------------------------------+
     | RS256             | RSASSA-PKCS1-v1_5 using SHA-256 |
     | RS384             | RSASSA-PKCS1-v1_5 using SHA-384 |
     | RS512             | RSASSA-PKCS1-v1_5 using SHA-512 |
     +-------------------+---------------------------------+
     The RSASSA-PKCS1-v1_5 SHA-256 digital signature for a JWS is
     validated as follows: submit the JWS Signing Input, the JWS
     Signature, and the public key corresponding to the private key used
     by the signer to the RSASSA-PKCS1-v1_5-VERIFY algorithm using SHA-256
     as the hash function.
     */

    struct RSA_Signer : JWASigner
    {
        virtual std :: string sign ( const void * data, size_t bytes ) const
        {
            // start digest computation
            int status = vdb_mbedtls_md_starts ( & sha_ctx );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to start SHA computation" );

            // compute checksum
            status = vdb_mbedtls_md_update ( & sha_ctx, ( unsigned char * ) data, bytes );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to compute SHA checksum" );

            unsigned char digest [ 512 / 8 ];
            assert ( sizeof digest >= dsize );
            status = vdb_mbedtls_md_finish ( & sha_ctx, digest );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to compute SHA digest" );

            unsigned char sig [ MBEDTLS_MPI_MAX_SIZE ];
            status = vdb_mbedtls_rsa_pkcs1_sign ( & rsa_ctx,
                                              nullptr, nullptr, // TBD - RNG needed?
                                              MBEDTLS_RSA_PRIVATE,
                                              md_type,
                                              ( unsigned int ) dsize,
                                              digest,
                                              sig
                );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to compute RSA signature" );

            // encode as base64url
            return encodeBase64URL ( sig, rsa_ctx . len );
        }

        virtual JWASigner * clone () const
        {
            return new RSA_Signer ( nam, alg, key, md_type );
        }

        void readKey ( const RSAPrivate_JWKey * key )
        {
            mbedtls_mpi N, P, Q, D, E;
            vdb_mbedtls_mpi_init ( & N );
            vdb_mbedtls_mpi_init ( & P );
            vdb_mbedtls_mpi_init ( & Q );
            vdb_mbedtls_mpi_init ( & D );
            vdb_mbedtls_mpi_init ( & E );

            try
            {
                mpiRead ( N, key -> getModulus () );
                mpiRead ( P, key -> getFirstPrimeFactor () );
                mpiRead ( Q, key -> getSecondPrimeFactor () );
                mpiRead ( D, key -> getPrivateExponent () );
                mpiRead ( E, key -> getExponent () );

                int status = vdb_mbedtls_rsa_import ( & rsa_ctx, & N, & P, & Q, & D, & E );
                if ( status != 0 )
                    throw MBEDTLSException ( __func__, __LINE__, status, "failed to import RSA key data" );

                status = vdb_mbedtls_rsa_complete ( & rsa_ctx );
                if ( status != 0 )
                    throw MBEDTLSException ( __func__, __LINE__, status, "failed to complete RSA key data" );

                status = vdb_mbedtls_rsa_check_privkey ( & rsa_ctx );
                if ( status != 0 )
                    throw MBEDTLSException ( __func__, __LINE__, status, "failed to validate RSA key data" );
            }
            catch ( ... )
            {
                vdb_mbedtls_mpi_free ( & N );
                vdb_mbedtls_mpi_free ( & P );
                vdb_mbedtls_mpi_free ( & Q );
                vdb_mbedtls_mpi_free ( & D );
                vdb_mbedtls_mpi_free ( & E );
                throw;
            }

            vdb_mbedtls_mpi_free ( & N );
            vdb_mbedtls_mpi_free ( & P );
            vdb_mbedtls_mpi_free ( & Q );
            vdb_mbedtls_mpi_free ( & D );
            vdb_mbedtls_mpi_free ( & E );
        }

        RSA_Signer ( const std :: string & name, const std :: string & alg,
                     const JWK * key, mbedtls_md_type_t type )
            : JWASigner ( name, alg, key )
            , rsa_ctx ( rsa_cctx )
            , sha_ctx ( sha_cctx )
            , md_type ( type )
        {
            // the RSA context must be initialized
            vdb_mbedtls_rsa_init  ( & rsa_ctx, MBEDTLS_RSA_PKCS_V15, 0 );

            // the SHA context must be initialized
            vdb_mbedtls_md_init ( & sha_ctx );

            // get hash info from the type
            const mbedtls_md_info_t * info = vdb_mbedtls_md_info_from_type ( md_type );
            dsize = vdb_mbedtls_md_get_size ( info );

            // allocate internal structures and bind to "info"
            int status = vdb_mbedtls_md_setup ( & sha_ctx, info, 0 );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to setup RSA/SHA context" );

            // we are supposed to have validated this as an RSA key
            const RSAPrivate_JWKey * priv = key -> toRSAPrivate ();
            try
            {
                readKey ( priv );
            }
            catch ( ... )
            {
                priv -> release ();
                throw;
            }
            priv -> release ();
        }

        ~ RSA_Signer ()
        {
            vdb_mbedtls_rsa_free ( & rsa_ctx );
            vdb_mbedtls_md_free ( & sha_ctx );
        }

        size_t dsize;
        mbedtls_rsa_context rsa_cctx, & rsa_ctx;
        mbedtls_md_context_t sha_cctx, & sha_ctx;
        mbedtls_md_type_t md_type;
    };

    struct RSA_Verifier : JWAVerifier
    {
        virtual bool verify ( const void * data, size_t bytes, const std :: string & sig_base64 ) const
        {
            // start digest computation
            int status = vdb_mbedtls_md_starts ( & sha_ctx );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to start SHA computation" );

            // compute checksum
            status = vdb_mbedtls_md_update ( & sha_ctx, ( unsigned char * ) data, bytes );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to compute SHA checksum" );

            unsigned char digest [ 512 / 8 ];
            assert ( sizeof digest >= dsize );
            status = vdb_mbedtls_md_finish ( & sha_ctx, digest );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to compute SHA digest" );

            Base64Payload signature = decodeBase64URL ( sig_base64 );
            if ( signature . size () != rsa_ctx . len )
                return false;

            status = vdb_mbedtls_rsa_pkcs1_verify ( & rsa_ctx,
                                                nullptr, nullptr, // TBD - RNG needed?
                                                MBEDTLS_RSA_PUBLIC,
                                                md_type,
                                                ( unsigned int ) dsize,
                                                digest,
                                                signature . data ()
                );
            if ( status != 0 )
                return false;

            return true;
        }

        virtual JWAVerifier * clone () const
        {
            return new RSA_Verifier ( nam, alg, key, md_type );
        }

        void readKey ( const RSAPublic_JWKey * key )
        {
            mbedtls_mpi N, E;
            vdb_mbedtls_mpi_init ( & N );
            vdb_mbedtls_mpi_init ( & E );

            try
            {
                mpiRead ( N, key -> getModulus () );
                mpiRead ( E, key -> getExponent () );

                int status = vdb_mbedtls_rsa_import ( & rsa_ctx, & N, 0, 0, 0, & E );
                if ( status != 0 )
                    throw MBEDTLSException ( __func__, __LINE__, status, "failed to import RSA key data" );

                status = vdb_mbedtls_rsa_complete ( & rsa_ctx );
                if ( status != 0 )
                    throw MBEDTLSException ( __func__, __LINE__, status, "failed to complete RSA key data" );

                status = vdb_mbedtls_rsa_check_pubkey ( & rsa_ctx );
                if ( status != 0 )
                    throw MBEDTLSException ( __func__, __LINE__, status, "failed to validate RSA key data" );
            }
            catch ( ... )
            {
                vdb_mbedtls_mpi_free ( & N );
                vdb_mbedtls_mpi_free ( & E );
                throw;
            }

            vdb_mbedtls_mpi_free ( & N );
            vdb_mbedtls_mpi_free ( & E );
        }

        RSA_Verifier ( const std :: string & name, const std :: string & alg,
                       const JWK * key, mbedtls_md_type_t type )
            : JWAVerifier ( name, alg, key )
            , rsa_ctx ( rsa_cctx )
            , sha_ctx ( sha_cctx )
            , md_type ( type )
        {
            // the RSA context must be initialized
            vdb_mbedtls_rsa_init  ( & rsa_ctx, MBEDTLS_RSA_PKCS_V15, 0 );

            // the SHA context must be initialized
            vdb_mbedtls_md_init ( & sha_ctx );

            // get hash info from the type
            const mbedtls_md_info_t * info = vdb_mbedtls_md_info_from_type ( md_type );
            dsize = vdb_mbedtls_md_get_size ( info );

            // allocate internal structures and bind to "info"
            int status = vdb_mbedtls_md_setup ( & sha_ctx, info, 0 );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to setup RSA/SHA context" );

            // we are supposed to have validated this as an RSA key
            const RSAPublic_JWKey * pub = key -> toRSAPublic ();
            try
            {
                readKey ( pub );
            }
            catch ( ... )
            {
                pub -> release ();
                throw;
            }
            pub -> release ();
        }

        ~ RSA_Verifier ()
        {
            vdb_mbedtls_rsa_free ( & rsa_ctx );
            vdb_mbedtls_md_free ( & sha_ctx );
        }

        size_t dsize;
        mbedtls_rsa_context rsa_cctx, & rsa_ctx;
        mbedtls_md_context_t sha_cctx, & sha_ctx;
        mbedtls_md_type_t md_type;
    };

    struct RSA_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const std :: string & name,
            const std :: string & alg, const JWK * key ) const
        {
            if ( key -> getType () . compare ( "RSA" ) != 0 )
                throw JWTException ( __func__, __LINE__, "bad key type" );

            return new RSA_Signer ( name, alg, key, md_type );
        }

        RSA_SignerFact ( const std :: string & alg, mbedtls_md_type_t type )
            : md_type ( type )
        {
            gJWAFactory . registerSignerFact ( alg, this );
        }

        mbedtls_md_type_t md_type;
    };

    struct RSA_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const std :: string & name,
            const std :: string & alg, const JWK * key ) const
        {
            if ( key -> getType () . compare ( "RSA" ) != 0 )
                throw JWTException ( __func__, __LINE__, "bad key type" );

            return new RSA_Verifier ( name, alg, key, md_type );
        }

        RSA_VerifierFact ( const std :: string & alg, mbedtls_md_type_t type )
            : md_type ( type )
        {
            gJWAFactory . registerVerifierFact ( alg, this );
        }

        mbedtls_md_type_t md_type;
    };

    static struct RSA_Registry
    {
        RSA_Registry ( const std :: string & alg, mbedtls_md_type_t md_type )
            : signer_fact ( alg, md_type )
            , verifier_fact ( alg, md_type )
        {
        }

        RSA_SignerFact signer_fact;
        RSA_VerifierFact verifier_fact;

    } rs256 ( "RS256", MBEDTLS_MD_SHA256 ),
      rs384 ( "RS384", MBEDTLS_MD_SHA384 ),
      rs512 ( "RS512", MBEDTLS_MD_SHA512 );

    void includeJWA_rsa ( bool always_false )
    {
        if ( always_false )
        {
            std :: string empty;
            rs256 . signer_fact . make ( empty, empty, nullptr );
            rs384 . signer_fact . make ( empty, empty, nullptr );
            rs512 . signer_fact . make ( empty, empty, nullptr );
        }
    }
}
