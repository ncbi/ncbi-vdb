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

#include <jwt/json.hpp>
#include <jwt/jwt.hpp>
#include <jwt/jws.hpp>
#include <jwt/jwk.hpp>
#include "base64-priv.hpp"

#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/bignum.h>

#include <iostream>

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

    const JWK * JWK :: parse ( const JwtString & json_text )
    {
        JWK * jwk = nullptr;

        // TBD - add limits : keys have known depths
        JSONObject * props = JSONObject :: parse ( json_text );
        try
        {
            // examine the type
            JwtString kty = props -> getValue ( "kty" ) . toString ();
            if ( kty . compare ( "oct" ) == 0 )
                jwk = HMAC_JWKey :: make ( props );
            else if ( kty . compare ( "RSA" ) == 0 )
            {
                if ( props -> exists ( "d" ) )
                    jwk = RSAPrivate_JWKey :: make ( props );
                else
                    jwk = RSAPublic_JWKey :: make ( props );
            }
            else if ( kty . compare ( "ES" ) == 0 )
            {
                if ( props -> exists ( "d" ) )
                    jwk = EllipticCurvePrivate_JWKey :: make ( props );
                else
                    jwk = EllipticCurvePublic_JWKey :: make ( props );
            }
            else
            {
                JwtString what ( "bad kty value for JWK: '" );
                what += kty;
                what += "'";
                throw JWTException ( __func__, __LINE__, what . c_str () );
            }
        }
        catch ( ... )
        {
            delete props;
            throw;
        }

        return jwk;
    }


    // code to perform some mbedtls magic
    static
    void writeKeyParameter ( JSONObject * props, const char * mbr, mbedtls_mpi & mpi )
    {
        // the MPI ( Multi-precision Integer ) represents some parameter
        // it is intended to be represented in base64url format
        // extract it first into a buffer
        unsigned char buff [ 4096 ], * bp = buff;
        size_t buff_size = sizeof buff;

        // test the actual size of mpi
        size_t mpi_size = vdb_mbedtls_mpi_size ( & mpi );

        // allocate a temporary buffer if necessary
        if ( mpi_size > buff_size )
        {
            bp = new unsigned char [ mpi_size ];
            buff_size = mpi_size;
        }
        try
        {
            // write mpi into our buffer
            int status = vdb_mbedtls_mpi_write_binary ( & mpi, bp, mpi_size );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to write key parameter" );

            // base64url encode the thing
            JwtString encoded = encodeBase64URL ( ( void * ) bp, mpi_size );

            // write it into the props
            props -> setValueOrDelete ( mbr, JSONValue :: makeString ( encoded ) );
        }
        catch ( ... )
        {
            if ( bp != buff )
                delete [] bp;
            throw;
        }

        if ( bp != buff )
            delete [] bp;
    }


    // inflate from PEM text format
    const JWK * JWK :: parsePEM ( const JwtString & pem_text,
        const JwtString & use, const JwtString & alg, const JwtString & kid )
    {
        return parsePEM ( pem_text, "", use, alg, kid );
    }

    const JWK * JWK :: parsePEM ( const JwtString & pem_text, const JwtString & pwd,
        const JwtString & use, const JwtString & alg, const JwtString & kid )
    {
        size_t i, start, end;
        for ( i = end = 0; ; ++ i )
        {
            // locate the start of opening delimiter line
            start = pem_text . find ( "-----BEGIN ", end );
            if ( start == JwtString :: npos )
                throw JWTException ( __func__, __LINE__, "invalid PEM text" );

            // locate the start of the label
            size_t label_start = start + sizeof "-----BEGIN " - 1;

            // locate the end of opening delimiter line
            // we are interested in cases when this ends in " KEY-----"
            // but PEM text can contain multiple entries.
            // regardless, the line MUST end in "-----"
            size_t key_start = pem_text . find ( "-----", label_start );
            if ( key_start == JwtString :: npos )
                throw JWTException ( __func__, __LINE__, "invalid PEM text" );

            // convert into potential start of base64-encoded key string
            // the pem-type keyword we want will be " KEY-----" - 1 bytes behind
            key_start += sizeof "-----" - 1;

            // locate the start of the next delimiter line
            // which should be a closing delimiter line
            // which would also be the end of the key
            size_t key_end = pem_text . find ( "-----", key_start );
            if ( key_end == JwtString :: npos )
                throw JWTException ( __func__, __LINE__, "invalid PEM text" );

            // this should be an "-----END " ... line
            size_t end_label = pem_text . find ( "END ", key_end + 5 );
            if ( end_label != key_end + 5 )
                throw JWTException ( __func__, __LINE__, "invalid PEM text" );

            // the start of the ending label
            end_label += sizeof "END " - 1;

            // locate the end of this delimiter
            size_t end = pem_text . find ( "-----", end_label );
            if ( end == JwtString :: npos )
                throw JWTException ( __func__, __LINE__, "invalid PEM text" );
            end += 5;

            // the delimiter lines should match other than "BEGIN" and "END"
            if ( pem_text . compare ( label_start, key_start - label_start, pem_text, end_label, end - end_label ) != 0 )
                throw JWTException ( __func__, __LINE__, "invalid PEM text" );

            // seems like a legitimate PEM entry - see if it's a KEY entry
            const char type_str [] = " KEY-----";
            const size_t type_str_len = sizeof type_str - 1;
            size_t type_start = key_start - type_str_len;
            if ( pem_text . compare ( type_start, type_str_len, type_str, type_str_len ) == 0 )
            {
                const JWK * jwk = nullptr;

                // get the label
                JwtString label = pem_text . substr ( label_start, type_start - label_start );

                // get the full PEM text of this entry
                JwtString key_text = pem_text . substr ( start, end - start );

                // learn whether the key claims to be public or private
                bool key_is_public = false;

                int status = 0;
                mbedtls_pk_context pk;
                vdb_mbedtls_pk_init ( & pk );
                try
                {
                    // look for a label we support
                    if ( label . compare ( "RSA PRIVATE" ) == 0 ||
                         label . compare ( "EC PRIVATE" ) == 0 )
                    {
                        // NB - mbedtls states
                        //  "Avoid calling mbedtls_pem_read_buffer() on non-null-terminated string"
                        //  so always pass "c_str()" rather than "data()"
                        // NB - incredibly, the key size they want passed in must include the NUL!!!
                        status = vdb_mbedtls_pk_parse_key ( & pk,
                            ( const unsigned char * ) key_text . c_str (), key_text . size () + 1,
                            ( const unsigned char * ) pwd . c_str (), pwd . size () );
                    }
                    else if ( label . compare ( "RSA PUBLIC" ) == 0 ||
                              label . compare ( "PUBLIC" ) == 0 )
                    {
                        // key claims to be public
                        key_is_public = true;

                        // NB - mbedtls states
                        //  "Avoid calling mbedtls_pem_read_buffer() on non-null-terminated string"
                        //  so always pass "c_str()" rather than "data()"
                        // NB - incredibly, the key size they want passed in must include the NUL!!!
                        status = vdb_mbedtls_pk_parse_public_key ( & pk,
                            ( const unsigned char * ) key_text . c_str (), key_text . size () + 1 );
                    }
                    else
                    {
                        vdb_mbedtls_pk_free ( & pk );
                        continue;
                    }

                    if ( status != 0 )
                        throw MBEDTLSException ( __func__, __LINE__, status, "failed to parse PEM key" );

                    // extract the components
                    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
                    vdb_mbedtls_mpi_init ( & N );
                    vdb_mbedtls_mpi_init ( & P );
                    vdb_mbedtls_mpi_init ( & Q );
                    vdb_mbedtls_mpi_init ( & D );
                    vdb_mbedtls_mpi_init ( & E );
                    vdb_mbedtls_mpi_init ( & DP );
                    vdb_mbedtls_mpi_init ( & DQ );
                    vdb_mbedtls_mpi_init ( & QP );

                    try
                    {
                        JSONObject * props = JSONObject :: make ();
                        try
                        {
                            // set some JSON properties
                            props -> setValueOrDelete ( "use", JSONValue :: makeString ( use ) );
                            props -> setValueOrDelete ( "alg", JSONValue :: makeString ( alg ) );
                            props -> setValueOrDelete ( "kid", JSONValue :: makeString ( kid ) );

                            if ( vdb_mbedtls_pk_get_type ( & pk ) == MBEDTLS_PK_RSA )
                            {
                                // set type property
                                props -> setValueOrDelete ( "kty", JSONValue :: makeString ( "RSA" ) );

                                // extract the RSA context
                                mbedtls_rsa_context * rsa = vdb_mbedtls_pk_rsa ( pk );

                                // handle RSA PUBLIC key
                                if ( key_is_public )
                                {
                                    // extract the public-only portions
                                    status = vdb_mbedtls_rsa_export ( rsa, & N, nullptr, nullptr, nullptr, & E );
                                    if ( status != 0 )
                                    {
                                        throw MBEDTLSException ( __func__, __LINE__, status,
                                            "mbedtls_rsa_export failed to obtain key parameters" );
                                    }

                                    // write the key parameters into JSON
                                    writeKeyParameter ( props, "n", N );
                                    writeKeyParameter ( props, "e", E );

                                    // create the key
                                    // NB - MUST be last step within try block
                                    jwk = RSAPublic_JWKey :: make ( props );
                                }
                                else
                                {
                                    // extract the full RSA portions
                                    status = vdb_mbedtls_rsa_export ( rsa, & N, & P, & Q, & D, & E );
                                    if ( status != 0 )
                                    {
                                        throw MBEDTLSException ( __func__, __LINE__, status,
                                            "mbedtls_rsa_export failed to obtain key parameters" );
                                    }
                                    status = vdb_mbedtls_rsa_export_crt ( rsa, & DP, & DQ, & QP );
                                    if ( status != 0 )
                                    {
                                        throw MBEDTLSException ( __func__, __LINE__, status,
                                            "mbedtls_rsa_export_crt failed to obtain key parameters" );
                                    }

                                    // write the key parameters into JSON
                                    writeKeyParameter ( props, "n", N );
                                    writeKeyParameter ( props, "e", E );
                                    writeKeyParameter ( props, "d", D );
                                    writeKeyParameter ( props, "p", P );
                                    writeKeyParameter ( props, "q", Q );
                                    writeKeyParameter ( props, "dp", DP );
                                    writeKeyParameter ( props, "dq", DQ );
                                    writeKeyParameter ( props, "qi", QP );

                                    // create the key
                                    // NB - MUST be last step within try block
                                    jwk = RSAPrivate_JWKey :: make ( props );
                                }
                            }
                            else if ( 0 )
                            {
                            }
                            else
                            {
                                // exception case
                                throw JWTException ( __func__, __LINE__, "INTERNAL ERROR - unknown mbedtls key type" );
                            }
                        }
                        catch ( ... )
                        {
                            props -> invalidate ();
                            delete props;
                            throw;
                        }
                    }
                    catch ( ... )
                    {
                        vdb_mbedtls_mpi_free ( & N );
                        vdb_mbedtls_mpi_free ( & P );
                        vdb_mbedtls_mpi_free ( & Q );
                        vdb_mbedtls_mpi_free ( & D );
                        vdb_mbedtls_mpi_free ( & E );
                        vdb_mbedtls_mpi_free ( & DP );
                        vdb_mbedtls_mpi_free ( & DQ );
                        vdb_mbedtls_mpi_free ( & QP );
                        throw;
                    }

                    vdb_mbedtls_mpi_free ( & N );
                    vdb_mbedtls_mpi_free ( & P );
                    vdb_mbedtls_mpi_free ( & Q );
                    vdb_mbedtls_mpi_free ( & D );
                    vdb_mbedtls_mpi_free ( & E );
                    vdb_mbedtls_mpi_free ( & DP );
                    vdb_mbedtls_mpi_free ( & DQ );
                    vdb_mbedtls_mpi_free ( & QP );

                }
                catch ( ... )
                {
                    vdb_mbedtls_pk_free ( & pk );
                    throw;
                }

                vdb_mbedtls_pk_free ( & pk );

                return jwk;
            }
        }
    }

    // inflate from DER format
    const JWK * JWK :: parseDER ( const void * key, size_t key_size,
        const JwtString & use, const JwtString & alg, const JwtString & kid )
    {
        throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED" );
    }

    const JWK * JWK :: parseDER ( const void * key, size_t key_size, const JwtString & pwd,
        const JwtString & use, const JwtString & alg, const JwtString & kid )
    {
        throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED" );
    }

    // inflate from PEM or DER format
    const JWK * JWK :: parsePEMorDER ( const void * key, size_t key_size,
        const JwtString & use, const JwtString & alg, const JwtString & kid )
    {
        throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED" );
    }

    const JWK * JWK :: parsePEMorDER ( const void * key, size_t key_size, const JwtString & pwd,
        const JwtString & use, const JwtString & alg, const JwtString & kid )
    {
        throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED" );
    }

}
