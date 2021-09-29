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

#include <ncbi/jwa.hpp>
#include <ncbi/jwk.hpp>
#include "jwa-registry.hpp"
#include <ncbi/secure/base64.hpp>

#include <mbedtls/md.h>
#include <mbedtls/rsa.h>
#include <mbedtls/error.h>
#include <mbedtls/bignum.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#ifndef MBEDTLS_THREADING_C
#error "shared entropy source must be thread-safe: enable MBEDTLS_THREADING_C."
#endif

#include <cstring>
#include <cassert>
#include <unistd.h>
#include <errno.h>

namespace ncbi
{

#ifndef HAVE_MPI_READ
#define HAVE_MPI_READ 1
    static
    void mpiRead ( mbedtls_mpi & mpi, const String & val )
    {
        // the string is in base64url encoding
        Payload raw = Base64 :: urlDecode ( val, Base64 :: strict_charset );

        // presumably, it can just be read into the multi-precision number
        int status = mbedtls_mpi_read_binary ( & mpi, ( const unsigned char * ) raw . data (), raw . size () );
        if ( status != 0 )
        {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to read RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
        }
    }
#endif
    
#ifndef HAVE_SAFE_MPI
#define HAVE_SAFE_MPI 1
    struct safe_mpi : mbedtls_mpi
    {
        safe_mpi ()
        { mbedtls_mpi_init ( this ); }

        ~ safe_mpi ()
        { mbedtls_mpi_free ( this ); }
    };
#endif

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

    struct safe_rsa_context : mbedtls_rsa_context
    {
        safe_rsa_context ( int padding, int hash_id = 0 )
        { mbedtls_rsa_init ( this, padding, hash_id ); }

        ~ safe_rsa_context ()
        { mbedtls_rsa_free ( this ); }
    };

    struct RSA_Signer : JWASigner
    {
        static void readKey ( mbedtls_rsa_context & ctx, const JWK & key )
        {
            safe_mpi N, P, Q, D, E;

            // modulus
            mpiRead ( N, getKeyProp ( key, "n" ) );
            mpiRead ( E, getKeyProp ( key, "e" ) );
            mpiRead ( D, getKeyProp ( key, "d" ) );
            mpiRead ( P, getKeyProp ( key, "p" ) );
            mpiRead ( Q, getKeyProp ( key, "q" ) );

            int status = mbedtls_rsa_import ( & ctx, & N, & P, & Q, & D, & E );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to import RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
            }

            status = mbedtls_rsa_complete ( & ctx );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to complete RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
            }

            status = mbedtls_rsa_check_privkey ( & ctx );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to validate RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
            }
        }

        virtual Payload sign ( const JWK & key,
            const void * data, size_t bytes ) const override
        {
            if ( ! key . isRSA () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not RSA"
                    );
            }

            if ( ! key . forSigning () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not for signing"
                    );
            }

            // the RSA context must be initialized
            safe_rsa_context ctx ( MBEDTLS_RSA_PKCS_V15 );

            // get hash info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            // bind the key to the context
            readKey ( ctx, key );

            // generate the checksum
            unsigned char checksum [ 512 / 8 ];
            assert ( sizeof checksum >= dsize );
            int status = mbedtls_md ( info, ( const unsigned char * ) data, bytes, checksum );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to calculate SHA-"
                    << ( dsize * 8 )
                    << " checksum"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // compute the signature
            Payload sig ( ctx . len );
            //status = mbedtls_rsa_pkcs1_sign (
            status = mbedtls_rsa_rsassa_pkcs1_v15_sign (
                & ctx
                , mbedtls_ctr_drbg_random, ( void * ) & ctr_drbg
                , MBEDTLS_RSA_PRIVATE
                , md_type
                , ( unsigned int ) dsize, checksum
                , sig . data ()
                );

            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to compute RSA signature"
                    << xcause
                    << crypterr ( status )
                    );
            }

            sig . setSize ( ctx . len );
            return sig;
        }
        
        RSA_Signer ( mbedtls_md_type_t type )
            : md_type ( type )
        {
            mbedtls_entropy_init ( & ent_ctx );
            mbedtls_ctr_drbg_init ( & ctr_drbg );

            char pers [ 4096 ];
            int status, pers_size = snprintf (
                pers, sizeof pers
                , "%s[%d]"
                , __func__
                , getpid ()
                );

            if ( pers_size < 0 )
            {
                status = errno;
                throw InternalError (
                    XP ( XLOC )
                    << "failed to create personalization"
                    << xcause
                    << syserr ( status )
                    );
            }

            if ( ( size_t ) pers_size >= sizeof pers )
                pers [ pers_size = sizeof pers - 1 ] = 0;

            status = mbedtls_ctr_drbg_seed (
                & ctr_drbg
                , mbedtls_entropy_func, & ent_ctx
                , ( const unsigned char * ) pers
                , ( size_t ) pers_size
                );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to seed RNG"
                    << xcause
                    << crypterr ( status )
                    );
            }

#if ENABLE_PRNG_PREDICTION_RESISTANCE
            mbedtls_ctr_drbg_set_prediction_resistance (
                & ctr_drbg
                , MBEDTLS_CTR_DRBG_PR_ON
                );
#endif
        }

        virtual ~ RSA_Signer () override
        {
            mbedtls_entropy_free ( & ent_ctx );
            mbedtls_ctr_drbg_free ( & ctr_drbg );
        }
        
        mbedtls_entropy_context ent_ctx;        
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_md_type_t md_type;
    };
    
    struct RSA_Verifier : JWAVerifier
    {
        static void readKey ( mbedtls_rsa_context & ctx, const JWK & key )
        {
            safe_mpi N, E;

            mpiRead ( N, getKeyProp ( key, "n" ) );
            mpiRead ( E, getKeyProp ( key, "e" ) );

            int status = mbedtls_rsa_import ( & ctx, & N, 0, 0, 0, & E );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to import RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
            }

            status = mbedtls_rsa_complete ( & ctx );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to complete RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
            }

            status = mbedtls_rsa_check_pubkey ( & ctx );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to validate RSA key data"
                    << xcause
                    << crypterr ( status )
                    );
            }
        }

        virtual bool verify ( const JWK & key, const void * data, size_t bytes,
            const Payload & signature ) const override
        {
            if ( ! key . isRSA () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not RSA"
                    );
            }

            if ( ! key . forVerifying () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not for verifying"
                    );
            }


            // the RSA context must be initialized
            safe_rsa_context ctx ( MBEDTLS_RSA_PKCS_V15 );

            // get hash info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            // bind the key to the context
            readKey ( ctx, key );

            // generate the checksum
            unsigned char checksum [ 512 / 8 ];
            assert ( sizeof checksum >= dsize );
            int status = mbedtls_md ( info, ( const unsigned char * ) data, bytes, checksum );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to calculate SHA-"
                    << ( dsize * 8 )
                    << " checksum"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // must be the expected size
            if ( signature . size () != ctx . len )
                return false;

            // verify the signature
            status = mbedtls_rsa_rsassa_pkcs1_v15_verify (
                & ctx
                , nullptr, nullptr
                , MBEDTLS_RSA_PUBLIC
                , md_type
                , ( unsigned int ) dsize, checksum
                , signature . data ()
                );

            // status must be 0 for a match
            return status == 0;
        }

        virtual size_t expectedSignatureSize () const override
        {
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            return mbedtls_md_get_size ( info );
        }
        
        RSA_Verifier ( mbedtls_md_type_t type )
            : md_type ( type )
        {
        }

        mbedtls_md_type_t md_type;
    };
    
    static struct RSA_Registry
    {
        RSA_Registry ()
        {
            String alg = "RS256";
            gJWARegistry . registerSigner ( alg, new RSA_Signer ( MBEDTLS_MD_SHA256 ) );
            gJWARegistry . registerVerifier ( alg, new RSA_Verifier ( MBEDTLS_MD_SHA256 ) );

            alg = "RS384";
            gJWARegistry . registerSigner ( alg, new RSA_Signer ( MBEDTLS_MD_SHA384 ) );
            gJWARegistry . registerVerifier ( alg, new RSA_Verifier ( MBEDTLS_MD_SHA384 ) );

            alg = "RS512";
            gJWARegistry . registerSigner ( alg, new RSA_Signer ( MBEDTLS_MD_SHA512 ) );
            gJWARegistry . registerVerifier ( alg, new RSA_Verifier ( MBEDTLS_MD_SHA512 ) );
        }

        void avoidDeadStrip ()
        {
            gJWARegistry . doNothing ();
        }
        
    } rsa_registry;

    void includeJWA_rsa ( bool always_false )
    {
        if ( always_false )
            rsa_registry . avoidDeadStrip ();
    }
}
