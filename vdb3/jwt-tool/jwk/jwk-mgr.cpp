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

#include <ncbi/jwk.hpp>
#include <ncbi/jwa.hpp>
#include <ncbi/secure/base64.hpp>

#include <mbedtls/pk.h>
#include <mbedtls/error.h>

#include <cassert>

#include <uuid/uuid.h>

namespace ncbi
{
    void JWKMgr :: InvalKey :: make ()
    {
        // not extremely worried about thread safety here
        // this is a one shot that is guaranteed to be
        // called before process "main()".
        if ( key == nullptr )
        {
            JSONObjectRef props = JSON :: makeObject ();

            static struct { const char * name, * value; } contents [] =
            {
                { "alg", "none" },
                { "kty", "none" },
                { "use", "none" },
                { "kid", "none" }
            };

            for ( count_t i = 0; i < sizeof contents / sizeof contents [ 0 ]; ++ i )
            {
                props -> setValue ( contents [ i ] . name,
                    JSON :: makeString ( contents [ i ] . value ) );
            }

            key = new JWK ( props );
        }
    }

    JWKMgr :: InvalKey :: InvalKey ()
    {
        // cause the object to be create before "main()" is called
        make ();
    }

    JWKMgr :: InvalKey :: ~ InvalKey ()
    {
    }

    JWKMgr :: InvalKey JWKMgr :: inval;

    JWKRef JWKMgr :: getInvalidKey () noexcept
    {
        // just in case we're called before "main()"
        inval . make ();
        return inval . key;
    }

    JWKSetRef JWKMgr :: makeJWKSet ()
    {
        JSONObjectRef obj = JSON :: makeObject ();
        obj -> addValue ( "keys", JSON :: makeArray () . release () );
        return JWKSetRef ( new JWKSet ( obj ) );
    }

    static
    void validateHMAC ( const JSONObject & props )
    {
        // key string
        if ( ! props . getValue ( "k" ) . isString () )
        {
            throw MalformedJWK (
                XP ( XLOC )
                << "expected '"
                << 'k'
                << "' value as string for JWK"
                );
        }
    }

    static
    void validateRSA ( const JSONObject & props )
    {
        const char * pub_props [] = { "n", "e" };

        size_t i;
        for ( i = 0; i < sizeof pub_props / sizeof pub_props [ 0 ]; ++ i )
        {
            if ( ! props . getValue ( pub_props [ i ] ) . isString () )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "expected '"
                    << pub_props [ i ]
                    << "' value as string for JWK"
                    );
            }
        }

        const char * priv_props [] = { "d", "p", "q" };

        assert ( sizeof priv_props / sizeof priv_props [ 0 ] != 0 );
        if ( props . exists ( priv_props [ 0 ] ) )
        {
            for ( i = 0; i < sizeof priv_props / sizeof priv_props [ 0 ]; ++ i )
            {
                if ( ! props . getValue ( priv_props [ i ] ) . isString () )
                {
                    throw MalformedJWK (
                        XP ( XLOC )
                        << "expected '"
                        << priv_props [ i ]
                        << "' value as string for JWK"
                        );
                }
            }
        }
    }

    static
    void validateEC ( const JSONObject & props )
    {
        const char * pub_props [] = { "crv", "x", "y" };

        size_t i;
        for ( i = 0; i < sizeof pub_props / sizeof pub_props [ 0 ]; ++ i )
        {
            if ( ! props . getValue ( pub_props [ i ] ) . isString () )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "expected '"
                    << pub_props [ i ]
                    << "' value as string for JWK"
                    );
            }
        }

        const char * priv_props [] = { "d" };

        assert ( sizeof priv_props / sizeof priv_props [ 0 ] != 0 );
        if ( props . exists ( priv_props [ 0 ] ) )
        {
            for ( i = 0; i < sizeof priv_props / sizeof priv_props [ 0 ]; ++ i )
            {
                if ( ! props . getValue ( priv_props [ i ] ) . isString () )
                {
                    throw MalformedJWK (
                        XP ( XLOC )
                        << "expected '"
                        << priv_props [ i ]
                        << "' value as string for JWK"
                        );
                }
            }
        }
    }

    void JWKMgr :: validateJWK ( const JSONObject & props )
    {
        // key type
        String kty = props . getValue ( "kty" ) . toString ();

        // check the alg
        if ( props . exists ( "alg" ) )
        {
            String alg = props . getValue ( "alg" ) . toString ();
            if ( ! JWAMgr :: acceptJWKAlgorithm ( kty, alg ) )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "unsupported alg value for JWK: '"
                    << alg
                    << '\''
                    );
            }
        }

        // examine the usage
        if ( props . exists ( "key_ops" ) )
        {
            const JSONArray & ops = props . getValue ( "key_ops" ) . toArray ();
            unsigned int i, count = ops . count ();
            for ( i = 0; i < count; ++ i )
            {
                String op = ops [ i ] . toString ();
                if ( op != "sign"       &&
                     op != "verify"     &&
                     op != "encrypt"    &&
                     op != "decrypt"    &&
                     op != "wrapKey"    &&
                     op != "unwrapKey"  &&
                     op != "deriveKey"  &&
                     op != "deriveBits" )
                {
                    throw MalformedJWK (
                        XP ( XLOC )
                        << "unrecognized key_ops[] value for JWK: '"
                        << op
                        << '\''
                        );
                }
            }
        }
        else if ( props . exists ( "use" ) )
        {
            String use = props . getValue ( "use" ) . toString ();
            if ( use != "sig" &&
                 use != "enc" )
            {
                throw MalformedJWK (
                    XP ( XLOC )
                    << "unrecognized use value for JWK: '"
                    << use
                    << '\''
                    );
            }
        }

        // examine the type
        if ( kty == "oct" )
            validateHMAC ( props );
        else if ( kty == "RSA" )
            validateRSA ( props );
        else if ( kty == "EC" )
            validateEC ( props );
        else
        {
            throw MalformedJWK (
                XP ( XLOC )
                << "unrecognized kty value for JWK: '"
                << kty
                << '\''
                );
        }

        // check the kid
        if ( ! props . exists ( "kid" ) )
            throw MalformedJWK ( XP ( XLOC ) << "expected kid value for JWK" );
    }

    void JWKMgr :: validateJWKSet ( const JSONObject & kset )
    {
        std :: set < String > kid_set;

        const JSONArray & keys = kset . getValue ( "keys" ) . toArray ();
        unsigned long int i, count = keys . count ();
        for ( i = 0; i < count; ++ i )
        {
            const JSONObject & props = keys [ i ] . toObject ();
            validateJWK ( props );

            String kid = props . getValue ( "kid" ) . toString ();
            auto it = kid_set . emplace ( kid );
            if ( ! it . second )
            {
                throw MalformedJWKSet (
                    XP ( XLOC )
                    << "duplicate kid in JWKSet: '"
                    << kid
                    << '\''
                    );
            }
        }
    }

    JWKRef JWKMgr :: parseJWK ( const String & json_text )
    {
        // keys have known depths
        JSON :: Limits lim;
        lim . recursion_depth = 20; // TBD - get real limit

        JSONObjectRef props = JSON :: parseObject ( lim, json_text );

        validateJWK ( * props );

        return JWKRef ( new JWK ( props ) );
    }

    JWKSetRef JWKMgr :: parseJWKSet ( const String & json_text )
    {
        // key sets have known depths
        JSON :: Limits lim;
        lim . recursion_depth = 22; // TBD - get real limit

        JSONObjectRef props = JSON :: parseObject ( lim, json_text );

        validateJWKSet ( * props );

        return JWKSetRef ( new JWKSet ( props ) );
    }


    // code to perform some mbedtls magic
    static
    void writeKeyParameter ( JSONObject & props, const char * mbr, mbedtls_mpi & mpi )
    {
        // the MPI ( Multi-precision Integer ) represents some parameter
        // it is intended to be represented in base64url format
        // extract it first into a buffer
        unsigned char buff [ 4096 ], * bp = buff;
        size_t buff_size = sizeof buff;

        // test the actual size of mpi
        size_t mpi_size = mbedtls_mpi_size ( & mpi );

        // allocate a temporary buffer if necessary
        if ( mpi_size > buff_size )
        {
            bp = new unsigned char [ mpi_size ];
            buff_size = mpi_size;
        }
        try
        {
            // write mpi into our buffer
            int status = mbedtls_mpi_write_binary ( & mpi, bp, mpi_size );
            if ( status != 0 )
            {
                throw CryptoException (
                    XP ( XLOC )
                    << "failed to write key parameter"
                    << xcause
                    << crypterr ( status )
                    );
            }

            // base64url encode the thing
            String encoded = Base64 :: urlEncode ( ( void * ) bp, mpi_size );

            // write it into the props
            props . setValue ( mbr, JSON :: makeString ( encoded ) );
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

    JWKRef JWKMgr :: parsePEM ( const String & pem_text,
        const String & use, const String & alg, const String & kid )
    {
        return parsePEM ( pem_text, "", use, alg, kid );
    }

    JWKRef JWKMgr :: parsePEM ( const String & pem_text, const String & pwd,
        const String & use, const String & alg, const String & kid )
    {
        count_t i, start, end;

        // this is a NEVER TERMINATING LOOP
        // by which I mean that it terminates either upon error
        // or upon success, neither of which are at standard exit
        // points of the loop.
        for ( i = end = 0; ; ++ i )
        {
            // locate the start of opening delimiter line
            start = pem_text . find ( "-----BEGIN ", end );
            if ( start == String :: npos )
                throw MalformedPEM ( XP ( XLOC ) << "invalid PEM text" );

            // locate the start of the label
            count_t label_start = start + sizeof "-----BEGIN " - 1;

            // locate the end of opening delimiter line
            // we are interested in cases when this ends in " KEY-----"
            // but PEM text can contain multiple entries.
            // regardless, the line MUST end in "-----"
            count_t key_start = pem_text . find ( "-----", label_start );
            if ( key_start == String :: npos )
                throw MalformedPEM ( XP ( XLOC ) << "invalid PEM text" );

            // convert into potential start of base64-encoded key string
            // the pem-type keyword we want will be " KEY-----" - 1 bytes behind
            key_start += sizeof "-----" - 1;

            // locate the start of the next delimiter line
            // which should be a closing delimiter line
            // which would also be the end of the key
            count_t key_end = pem_text . find ( "-----", key_start );
            if ( key_end == String :: npos )
                throw MalformedPEM ( XP ( XLOC ) << "invalid PEM text" );

            // this should be an "-----END " ... line
            count_t end_label = pem_text . find ( "END ", key_end + 5 );
            if ( end_label != key_end + 5 )
                throw MalformedPEM ( XP ( XLOC ) << "invalid PEM text" );

            // the start of the ending label
            end_label += sizeof "END " - 1;

            // locate the end of this delimiter
            count_t end = pem_text . find ( "-----", end_label );
            if ( end == String :: npos )
                throw MalformedPEM ( XP ( XLOC ) << "invalid PEM text" );
            end += 5;

            // the delimiter lines should match other than "BEGIN" and "END"
            if ( pem_text . subString ( label_start, key_start - label_start ) !=
                 pem_text . subString ( end_label, end - end_label ) )
                throw MalformedPEM ( XP ( XLOC ) << "invalid PEM text" );

            // seems like a legitimate PEM entry - see if it's a KEY entry
            const char type_str [] = " KEY-----";
            const size_t type_str_len = sizeof type_str - 1;
            size_t type_start = key_start - type_str_len;
            if ( pem_text . subString ( type_start, type_str_len ) == type_str )
            {
                // THIS BLOCK WILL EITHER FAIL OR RETURN A KEY

                JWKRef jwk;
                
                // get the label
                String label = pem_text . subString ( label_start, type_start - label_start );

                // get the full PEM text of this entry
                String key_text = pem_text . subString ( start, end - start );

                // learn whether the key claims to be public or private
                bool key_is_public = false;

                int status = 0;
                mbedtls_pk_context pk;
                mbedtls_pk_init ( & pk );

                // catch exceptions to undo effect of mbedtls_pk_init()
                try
                {
                    // look for a label we support
                    if ( label == "RSA PRIVATE" || label == "EC PRIVATE" )
                    {
                        // NB - mbedtls states:
                        //  "Avoid calling mbedtls_pem_read_buffer() on non-null-terminated string"
                        // NB - incredibly, the key size they want passed in must include the NUL!!!
                        NULTerminatedString zkt ( key_text );
                        NULTerminatedString zpw ( pwd );
                        status = mbedtls_pk_parse_key ( & pk,
                            ( const unsigned char * ) zkt . c_str (), zkt . size () + 1,
                            ( const unsigned char * ) zpw . c_str (), zpw . size () );
                    }
                    else if ( label == "RSA PUBLIC" || label == "PUBLIC" )
                    {
                        // key claims to be public
                        key_is_public = true;
                        
                        // NB - mbedtls states
                        //  "Avoid calling mbedtls_pem_read_buffer() on non-null-terminated string"
                        // NB - incredibly, the key size they want passed in must include the NUL!!!
                        NULTerminatedString zkt ( key_text );
                        status = mbedtls_pk_parse_public_key ( & pk,
                            ( const unsigned char * ) zkt . c_str (), zkt . size () + 1 );
                    }
                    else
                    {
                        // this is not a label we support
                        mbedtls_pk_free ( & pk );
                        continue;
                    }

                    // check for mbedtls errors
                    if ( status != 0 )
                    {
                        throw CryptoException (
                            XP ( XLOC )
                            << "failed to parse PEM key"
                            << xcause
                            << crypterr ( status )
                            );
                    }

                    // extract the components
                    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
                    mbedtls_mpi_init ( & N );
                    mbedtls_mpi_init ( & P );
                    mbedtls_mpi_init ( & Q );
                    mbedtls_mpi_init ( & D );
                    mbedtls_mpi_init ( & E );
                    mbedtls_mpi_init ( & DP );
                    mbedtls_mpi_init ( & DQ );
                    mbedtls_mpi_init ( & QP );
                    
                    try
                    {
                        JSONObjectRef props_ref = JSON :: makeObject ();
                        JSONObject & props = * props_ref;

                        // create a catch block to invalidate stored props
                        try
                        {
                            // set some JSON properties
                            props . setValue ( "use", JSON :: makeString ( use ) );
                            props . setValue ( "alg", JSON :: makeString ( alg ) );
                            props . setValue ( "kid", JSON :: makeString ( kid ) );
                            
                            if ( mbedtls_pk_get_type ( & pk ) == MBEDTLS_PK_RSA )
                            {
                                // set type property
                                props . setValue ( "kty", JSON :: makeString ( "RSA" ) );
                                
                                // extract the RSA context
                                mbedtls_rsa_context * rsa = mbedtls_pk_rsa ( pk );

                                // handle RSA PUBLIC key
                                if ( key_is_public )
                                {
                                    // extract the public-only portions
                                    status = mbedtls_rsa_export ( rsa, & N, nullptr, nullptr, nullptr, & E );
                                    if ( status != 0 )
                                    {
                                        throw CryptoException (
                                            XP ( XLOC )
                                            << "mbedtls_rsa_export failed to obtain key parameters"
                                            << xcause
                                            << crypterr ( status )
                                            );
                                    }

                                    // write the key parameters into JSON
                                    writeKeyParameter ( props, "n", N );
                                    writeKeyParameter ( props, "e", E );

                                    // create the key
                                    // NB - MUST be last step within try block
                                    jwk = new JWK ( props_ref );
                                }
                                else
                                {
                                    // extract the full RSA portions
                                    status = mbedtls_rsa_export ( rsa, & N, & P, & Q, & D, & E );
                                    if ( status != 0 )
                                    {
                                        throw CryptoException (
                                            XP ( XLOC )
                                            << "mbedtls_rsa_export failed to obtain key parameters"
                                            << xcause
                                            << crypterr ( status )
                                            );
                                    }
                                    status = mbedtls_rsa_export_crt ( rsa, & DP, & DQ, & QP );
                                    if ( status != 0 )
                                    {
                                        throw CryptoException (
                                            XP ( XLOC )
                                            << "mbedtls_rsa_export_crt failed to obtain key parameters"
                                            << xcause
                                            << crypterr ( status )
                                            );
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
                                    jwk = new JWK ( props_ref );
                                }
                            }
                            else if ( 0 )
                            {
                            }
                            else
                            {
                                // exception case
                                throw InternalError (
                                    XP ( XLOC )
                                    << "INTERNAL ERROR - unknown mbedtls key type: "
                                    << mbedtls_pk_get_type ( & pk )
                                    );
                            }
                        }
                        catch ( ... )
                        {
                            props . invalidate ();
                            throw;
                        }
                    }
                    catch ( ... )
                    {
                        mbedtls_mpi_free ( & N );
                        mbedtls_mpi_free ( & P );
                        mbedtls_mpi_free ( & Q );
                        mbedtls_mpi_free ( & D );
                        mbedtls_mpi_free ( & E );
                        mbedtls_mpi_free ( & DP );
                        mbedtls_mpi_free ( & DQ );
                        mbedtls_mpi_free ( & QP );
                        throw;
                    }
                    
                    mbedtls_mpi_free ( & N );
                    mbedtls_mpi_free ( & P );
                    mbedtls_mpi_free ( & Q );
                    mbedtls_mpi_free ( & D );
                    mbedtls_mpi_free ( & E );
                    mbedtls_mpi_free ( & DP );
                    mbedtls_mpi_free ( & DQ );
                    mbedtls_mpi_free ( & QP );
                    
                }
                catch ( ... )
                {
                    mbedtls_pk_free ( & pk );
                    throw;
                }
                
                mbedtls_pk_free ( & pk );

                return jwk;
            }
        }
    }

    String JWKMgr :: makeID ()
    {
        // this is nice enough:
        // the size of a binary UUID buffer is defined to accompany
        // the function that operates upon it. Of course, it does
        // not allow for any changes in this size over time and the
        // effects of dynamic linking or system updates. The functions
        // could have been protected against this by accepting a runtime
        // indication of what the software thinks is the buffer size.
        //
        // normally, we are supposed to simply say
        //    uuid_t uuid;
        // but while safe for the time period around which the
        // library was compiled, it is unsafe over time.
        //
        // to build in some buffer against this, we declare
        // a larger buffer.
        const size_t uuid_size = sizeof ( uuid_t );
        const size_t oversize_uuid_buffer_space = uuid_size * 4;
        unsigned char uuid [ oversize_uuid_buffer_space ];
        assert ( sizeof uuid % 16 == 0 );
        uuid_generate ( uuid );

        // this is absolutely AWFUL, especially for secure software.
        // passing in a simple pointer to a buffer with no indication of
        // buffer size? Again, there is the assumption that everyone KNOWS
        // the length of an ASCII representatio of a UUID ( 36 bytes
        // plus 1 NUL byte ) and that works for the current time period.
        const size_t ascii_UUID_char_count = 36;
        const size_t oversize_UUID_buffer_space = ascii_UUID_char_count * 4 + 1;
        const size_t aligned_buffer_size = ( ( oversize_UUID_buffer_space + 15 ) / 16 ) * 16;
        char buffer [ aligned_buffer_size ];
        assert ( sizeof buffer % 16 == 0 );
        uuid_unparse ( uuid, buffer );

        // had the function above returned the actual string size, assumed
        // to be 36, we could use it when constructing this string object.
        // but it's safer to scan for first NUL and use that.
        return String ( buffer );
    }
}
