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

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/sha256.h>

#ifndef MBEDTLS_THREADING_C
#error "shared entropy source must be thread-safe: enable MBEDTLS_THREADING_C."
#endif

#include <iostream>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <errno.h>

#define USE_DETERMINISTIC_SIGN 1

namespace ncbi
{

    struct ECDSA_Curve_Map
    {
        const mbedtls_ecp_curve_info *  getCurveInfo ( const String & curve_name ) const
        {
            if ( curve_name . isEmpty () )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "bad elliptic curve name"
                    << xcause
                    << "name is empty"
                    );
            }

            if ( only_ascii && ! curve_name . isAscii () )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "bad elliptic curve name"
                    << xcause
                    << "name contains non-ASCII characters"
                    );
            }

            if ( max_length < curve_name . length () )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "unrecognized elliptic curve name"
                    << xcause
                    << "name length exceeds maximum recognized curve name length"
                    );
            }

            auto it = map . find ( curve_name );
            if ( it == map . end () )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "unrecognized elliptic curve name '"
                    << curve_name
                    << '\''
                    );
            }

            const mbedtls_ecp_curve_info * info
                = mbedtls_ecp_curve_info_from_name ( NULTerminatedString ( it -> second ) . c_str () );
            if ( info == nullptr )
            {
                throw UnsupportedCurve (
                    XP ( XLOC )
                    << "unsupported elliptic curve name '"
                    << curve_name
                    << "' ( '"
                    << it -> second
                    << "' )"
                    << xcause
                    << "curve is not supported by crypto-library"
                    );
            }

            return info;
        }

        void loadGroup ( mbedtls_ecp_group & grp, const String & curve_name ) const
        {
            const mbedtls_ecp_curve_info * info = getCurveInfo ( curve_name );

            // load the group from the id
            int status = mbedtls_ecp_group_load ( & grp, info -> grp_id );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to load EC group '"
                    << curve_name
                    << "' ( '"
                    << info -> name
                    << "' )"
                    << xcause
                    << crypterr ( status )
                    );
            }
        }

        ECDSA_Curve_Map ()
            : max_length ( 0 )
            , only_ascii ( true )
        {
            struct { const char * from, * to; }
            nist_map [] =
            {
                { "K-163", "sect163k1" },
                { "B-163", "sect163r2" },
                { "K-233", "sect233k1" },
                { "B-233", "sect233r1" },
                { "K-283", "sect283k1" },
                { "B-283", "sect283r1" },
                { "K-409", "sect409k1" },
                { "B-409", "sect409r1" },
                { "K-571", "sect571k1" },
                { "B-571", "sect571r1" },
                { "P-192", "secp192r1" },
                { "P-224", "secp224r1" },
                { "P-256", "secp256r1" },
                { "P-384", "secp384r1" },
                { "P-521", "secp521r1" }
            },
            ansi_map [] =
            {
                { "prime192v1", "secp192r1" },
                { "prime256v1", "secp256r1" }
            };

            count_t i;
            for ( i = 0; i < sizeof nist_map / sizeof nist_map [ 0 ]; ++ i )
            {
                String from ( nist_map [ i ] . from );
                String to ( nist_map [ i ] . to );
                map . emplace ( from, to );
                if ( from . length () > max_length )
                    max_length = from . length ();
            }

            for ( i = 0; i < sizeof ansi_map / sizeof ansi_map [ 0 ]; ++ i )
            {
                String from ( ansi_map [ i ] . from );
                String to ( ansi_map [ i ] . to );
                map . emplace ( from, to );
                if ( from . length () > max_length )
                    max_length = from . length ();
            }

            const mbedtls_ecp_curve_info * info_array = mbedtls_ecp_curve_list ();
            for ( i = 0; info_array [ i ] . name != nullptr; ++ i )
            {
                String name ( info_array [ i ] . name );
                map . emplace ( name, name );
                if ( ! name . isAscii () )
                    only_ascii = false;
                if ( name . length () > max_length )
                    max_length = name . length ();
            }
        }

        ~ ECDSA_Curve_Map ()
        {
        }

        std :: map < String, String > map;
        count_t max_length;
        bool only_ascii;
    };

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
     +-------------------+-------------------------------+
     | "alg" Param Value | Digital Signature Algorithm   |
     +-------------------+-------------------------------+
     | ES256             | ECDSA using P-256 and SHA-256 |
     | ES384             | ECDSA using P-384 and SHA-384 |
     | ES512             | ECDSA using P-521 and SHA-512 |
     +-------------------+-------------------------------+
     */

    struct safe_ecp_group : mbedtls_ecp_group
    {
        safe_ecp_group ()
        { mbedtls_ecp_group_init ( this ); }

        ~ safe_ecp_group ()
        { mbedtls_ecp_group_free ( this ); }
    };

    struct safe_ecp_point : mbedtls_ecp_point
    {
        safe_ecp_point ()
        { mbedtls_ecp_point_init ( this ); }

        ~ safe_ecp_point ()
        { mbedtls_ecp_point_free ( this ); }
    };


    struct ECDSA_Signer : JWASigner
    {
        void readKey ( mbedtls_ecp_group & grp, mbedtls_mpi & d, const JWK & key ) const
        {
            // get the NIST curve name
            String curve_name = getKeyProp ( key, "crv" );

            // load the group
            cmap -> loadGroup ( grp, curve_name );

            // read the private "d"
            mpiRead ( d, getKeyProp ( key, "d" ) );

            // check it against curve
            int status = mbedtls_ecp_check_privkey ( & grp, & d );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to validate EC key data"
                    << xcause
                    << crypterr ( status )
                    );
            }
        }

        virtual Payload sign ( const JWK & key,
            const void * data, size_t bytes ) const override
        {
            if ( ! key . isEllipticCurve () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not for elliptical curve"
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

            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );
            
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

            // load group and extract private key
            safe_mpi d, r, s;
            safe_ecp_group grp;
            readKey ( grp, d, key );

            // produce a signature
#if USE_DETERMINISTIC_SIGN
            status = mbedtls_ecdsa_sign_det (
                & grp
                , & r, & s
                , & d
                , checksum, dsize
                , md_type
                );
#else
            status = mbedtls_ecdsa_sign (
                & grp
                , & r, & s
                , & d
                , checksum, dsize
                , mbedtls_ctr_drbg_random, ( void * ) & ctr_drbg
                );
#endif
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to compute EC signature"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // concatenate "r" and "s" into signature
            size_t nsize = ( grp . nbits + 7 ) / 8;
            Payload signature ( nsize + nsize );
            const char * which = "R";
            status = mbedtls_mpi_write_binary (
                & r
                , ( unsigned char * ) signature . data ()
                , nsize
                );
            if ( status == 0 )
            {
                which = "S";
                status = mbedtls_mpi_write_binary (
                    & s
                    , ( unsigned char * ) signature . data () + nsize
                    , nsize
                    );
            }
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to transcribe EC signature '"
                    << which
                    << "' coordinate to buffer of "
                    << nsize
                    << " bytes"
                    << xcause
                    << crypterr ( status )
                    );
            }

            signature . setSize ( nsize + nsize );
            return signature;
        }
        
        ECDSA_Signer ( const SRef < const ECDSA_Curve_Map > _cmap, mbedtls_md_type_t type )
            : cmap ( _cmap )
            , md_type ( type )
        {
#if ! USE_DETERMINISTIC_SIGN
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
                status = errno;
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

#endif // ! USE_DETERMINISTIC_SIGN
        }

#if ! USE_DETERMINISTIC_SIGN        
        virtual ~ ECDSA_Signer () override
        {
            mbedtls_entropy_free ( & ent_ctx );
            mbedtls_ctr_drbg_free ( & ctr_drbg );
        }
#endif
        
        SRef < const ECDSA_Curve_Map > cmap;
#if ! USE_DETERMINISTIC_SIGN        
        mbedtls_entropy_context ent_ctx;        
        mbedtls_ctr_drbg_context ctr_drbg;
#endif
        mbedtls_md_type_t md_type;
    };
    
    struct ECDSA_Verifier : JWAVerifier
    {
        void readKey ( mbedtls_ecp_group & grp, mbedtls_ecp_point & Q, const JWK & key ) const
        {
            // get the NIST curve name
            String curve_name = getKeyProp ( key, "crv" );

            // load the group
            cmap -> loadGroup ( grp, curve_name );

            // read the public "x" and "y" into a prepared point with "z" = 1
            mpiRead ( Q . X, getKeyProp ( key, "x" ) );
            mpiRead ( Q . Y, getKeyProp ( key, "y" ) );
            int status = mbedtls_mpi_lset ( & Q . Z, 1 );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to finish EC public key data"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // check it against curve
            status = mbedtls_ecp_check_pubkey ( & grp, & Q );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to validate EC key data"
                    << xcause
                    << crypterr ( status )
                    );
            }
        }

        virtual bool verify ( const JWK & key, const void * data, size_t bytes,
            const Payload & signature ) const override
        {
            if ( ! key . isEllipticCurve () )
            {
                throw JWABadKeyType (
                    XP ( XLOC )
                    << "bad key type"
                    << xcause
                    << "key is not for elliptical curve"
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

            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );
            
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

            // load group and extract public key
            safe_ecp_point Q;
            safe_ecp_group grp;
            readKey ( grp, Q, key );

            // extract signature into components
            safe_mpi r, s;
            status = mbedtls_mpi_read_binary (
                & r
                , ( const unsigned char * ) signature . data ()
                , signature . size () / 2
                );
            if ( status == 0 )
            {
                status = mbedtls_mpi_read_binary (
                    & s
                    , ( const unsigned char * ) signature . data () + signature . size () / 2
                    , signature . size () / 2
                    );
            }
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to extract signature coordinates"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // verify the signature
            status = mbedtls_ecdsa_verify (
                & grp
                , checksum, dsize
                , & Q
                , & r
                , & s
                );

            return status == 0;
        }

        virtual size_t expectedSignatureSize () const override
        {
            // this is weird - one of the curves produces slightly more
            // but we can key by the supported hash sizes.
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            switch ( mbedtls_md_get_size ( info ) )
            {
            case 32:
                return 32 * 2;
            case 48:
                return 48 * 2;
            case 64:
                return 66 * 2;
            default:
                return 0;
            }
        }
        
        ECDSA_Verifier ( const SRef < const ECDSA_Curve_Map > _cmap, mbedtls_md_type_t type )
            : cmap ( _cmap )
            , md_type ( type )
        {
        }
        
        SRef < const ECDSA_Curve_Map > cmap;
        mbedtls_md_type_t md_type;
    };
    
    
    static struct ECDSA_Registry
    {
        ECDSA_Registry ()
        {
            // there are multiple standards for specifying curve names
            // mbedtls uses different ones from JWK
            SRef < const ECDSA_Curve_Map > cmap ( new ECDSA_Curve_Map () );

            String alg = "ES256";
            gJWARegistry . registerSigner ( alg, new ECDSA_Signer ( cmap, MBEDTLS_MD_SHA256 ) );
            gJWARegistry . registerVerifier ( alg, new ECDSA_Verifier ( cmap, MBEDTLS_MD_SHA256 ) );

            alg = "ES384";
            gJWARegistry . registerSigner ( alg, new ECDSA_Signer ( cmap, MBEDTLS_MD_SHA384 ) );
            gJWARegistry . registerVerifier ( alg, new ECDSA_Verifier ( cmap, MBEDTLS_MD_SHA384 ) );

            alg = "ES512";
            gJWARegistry . registerSigner ( alg, new ECDSA_Signer ( cmap, MBEDTLS_MD_SHA512 ) );
            gJWARegistry . registerVerifier ( alg, new ECDSA_Verifier ( cmap, MBEDTLS_MD_SHA512 ) );
        }

        void avoidDeadStrip ()
        {
            gJWARegistry . doNothing ();
        }

    } ecdsa_registry;
    
    void includeJWA_ecdsa ( bool always_false )
    {
        if ( always_false )
            ecdsa_registry . avoidDeadStrip ();
    }
}
