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
#include <mbedtls/error.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{

    struct HMAC_Signer : JWASigner
    {
        virtual Payload sign ( const JWK & key,
            const void * data, size_t bytes ) const override
        {
            if ( ! key . isSymmetric () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not symmetric"
                    );
            }

            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            Payload mac ( dsize );

            String k = getKeyProp ( key, "k" );
            Payload oct = Base64 :: urlDecode ( k, Base64 :: strict_charset );

            int status = mbedtls_md_hmac (
                info
                , ( const unsigned char * ) oct . data (), oct . size ()
                , ( const unsigned char * ) data, bytes
                , mac . data ()
                );

            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to calculate HMAC"
                    << xcause
                    << crypterr ( status )
                    );
            }

            mac . setSize ( dsize );
            return mac;
        }

        HMAC_Signer ( mbedtls_md_type_t type )
            : md_type ( type )
        {
        }

        mbedtls_md_type_t md_type;
    };

    struct HMAC_Verifier : JWAVerifier
    {
        virtual bool verify ( const JWK & key, const void * data, size_t bytes,
            const Payload & signature ) const override
        {
            if ( ! key . isSymmetric () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not symmetric"
                    );
            }

            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            unsigned char mac [ 512 / 8 ];
            assert ( sizeof mac >= dsize );

            String k = getKeyProp ( key, "k" );
            Payload oct = Base64 :: urlDecode ( k, Base64 :: strict_charset );

            int status = mbedtls_md_hmac (
                info
                , ( const unsigned char * ) oct . data (), oct . size ()
                , ( const unsigned char * ) data, bytes
                , mac
                );

            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to calculate HMAC"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // test: the lengths must match
            if ( signature . size () != dsize )
                return false;

            // the digest must match
            if ( memcmp ( mac, signature . data (), dsize ) != 0 )
                return false;

            // signature verified
            return true;
        }

        virtual size_t expectedSignatureSize () const override
        {
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            return mbedtls_md_get_size ( info );
        }

        HMAC_Verifier ( mbedtls_md_type_t type )
            : md_type ( type )
        {
        }

        mbedtls_md_type_t md_type;
    };

    static struct HMAC_Registry
    {
        HMAC_Registry ()
        {
            String alg = "HS256";
            gJWARegistry . registerSigner ( alg, new HMAC_Signer ( MBEDTLS_MD_SHA256 ) );
            gJWARegistry . registerVerifier ( alg, new HMAC_Verifier ( MBEDTLS_MD_SHA256 ) );

            alg = "HS384";
            gJWARegistry . registerSigner ( alg, new HMAC_Signer ( MBEDTLS_MD_SHA384 ) );
            gJWARegistry . registerVerifier ( alg, new HMAC_Verifier ( MBEDTLS_MD_SHA384 ) );

            alg = "HS512";
            gJWARegistry . registerSigner ( alg, new HMAC_Signer ( MBEDTLS_MD_SHA512 ) );
            gJWARegistry . registerVerifier ( alg, new HMAC_Verifier ( MBEDTLS_MD_SHA512 ) );
        }

        void avoidDeadStrip ()
        {
            gJWARegistry . doNothing ();
        }
        
    } hmac_registry;

    void includeJWA_hmac ( bool always_false )
    {
        if ( always_false )
            hmac_registry . avoidDeadStrip ();
    }
}
