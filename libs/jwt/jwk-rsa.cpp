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

#include <iostream>

namespace ncbi
{

    const RSAPrivate_JWKey * RSAPrivate_JWKey :: make ( unsigned int key_bits,
        const std :: string & use, const std :: string & alg, const std :: string & kid )
    {
        JSONObject * props = JSONObject :: make ();
        try
        {
            props -> setValueOrDelete ( "kty", JSONValue :: makeString ( "RSA" ) );
            props -> setValueOrDelete ( "kid", JSONValue :: makeString ( kid ) );
            props -> setValueOrDelete ( "alg", JSONValue :: makeString ( alg ) );
            props -> setValueOrDelete ( "use", JSONValue :: makeString ( use ) );

            // TBD - create key and get properties

            return make ( props );
        }
        catch ( ... )
        {
            props -> invalidate ();
            delete props;
            throw;
        }
    }

    bool RSAPrivate_JWKey :: isRSA () const
    {
        return true;
    }

    bool RSAPrivate_JWKey :: isPrivate () const
    {
        return true;
    }

    const RSAPrivate_JWKey * RSAPrivate_JWKey :: toRSAPrivate () const
    {
        return reinterpret_cast < const RSAPrivate_JWKey * > ( duplicate () );
    }

    const RSAPublic_JWKey * RSAPrivate_JWKey :: toRSAPublic () const
    {
        return RSAPublic_JWKey :: derive ( this );
    }

    // modulus "n"
    std :: string RSAPrivate_JWKey :: getModulus () const
    {
        return props -> getValue ( "n" ) . toString ();
    }

    // exponent "e"
    std :: string RSAPrivate_JWKey :: getExponent () const
    {
        return props -> getValue ( "e" ) . toString ();
    }

    // private exponent "d"
    std :: string RSAPrivate_JWKey :: getPrivateExponent () const
    {
        return props -> getValue ( "d" ) . toString ();
    }

    // first prime factor "p"
    std :: string RSAPrivate_JWKey :: getFirstPrimeFactor () const
    {
        return props -> getValue ( "p" ) . toString ();
    }

    // second prime factor "q"
    std :: string RSAPrivate_JWKey :: getSecondPrimeFactor () const
    {
        return props -> getValue ( "q" ) . toString ();
    }

    // first factor CRT exponent "dp"
    std :: string RSAPrivate_JWKey :: getFirstFactorCRTExponent () const
    {
        return props -> getValue ( "dp" ) . toString ();
    }

    // second factor CRT exponent "dq"
    std :: string RSAPrivate_JWKey :: getSecondFactorCRTExponent () const
    {
        return props -> getValue ( "dq" ) . toString ();
    }

    // first CRT coefficient "qi"
    std :: string RSAPrivate_JWKey :: getFirstCRTCoefficient () const
    {
        return props -> getValue ( "qi" ) . toString ();
    }

    // other primes "oth"
    //  prime factor "r"
    //  factor CRT exponent "d"
    //  factor CRT coefficient "t"
    size_t RSAPrivate_JWKey :: numOtherPrimes () const
    {
        if ( props -> exists ( "oth" ) )
        {
            try
            {
                return props -> getValue ( "oth" ) . toArray () . count ();
            }
            catch ( ... )
            {
            }
        }
        return 0;
    }

    const JSONObject & RSAPrivate_JWKey :: getOtherPrime ( unsigned int idx ) const
    {
        return props -> getValue ( "oth" ) . toArray () . getValue ( idx ) . toObject ();
    }

    RSAPrivate_JWKey * RSAPrivate_JWKey :: make ( JSONObject * props )
    {
        checkProperties ( props );

        if ( props -> getValue ( "kty" ) . toString () . compare ( "RSA" ) != 0 )
            throw JWTException ( __func__, __LINE__, "not RSA key properties" );

        if ( ! props -> exists ( "n" ) ||
             ! props -> exists ( "e" ) ||
             ! props -> exists ( "d" ) ||
             ! props -> exists ( "p" ) ||
             ! props -> exists ( "q" ) )
        {
            throw JWTException ( __func__, __LINE__, "missing RSA private key parameter(s)" );
        }

        return new RSAPrivate_JWKey ( props );
    }

    // "kty" = "RSA"
    RSAPrivate_JWKey :: RSAPrivate_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }


    const RSAPublic_JWKey * RSAPublic_JWKey :: derive ( const RSAPrivate_JWKey * priv )
    {
        JSONObject * props = JSONObject :: make ();
        try
        {
            props -> setValueOrDelete ( "kty", JSONValue :: makeString ( priv -> getType () ) );
            props -> setValueOrDelete ( "kid", JSONValue :: makeString ( priv -> getID () ) );
            props -> setValueOrDelete ( "alg", JSONValue :: makeString ( priv -> getAlg () ) );
            props -> setValueOrDelete ( "n", JSONValue :: makeString ( priv -> getModulus () ) );
            props -> setValueOrDelete ( "e", JSONValue :: makeString ( priv -> getExponent () ) );
            return make ( props );
        }
        catch ( ... )
        {
            props -> invalidate ();
            delete props;
            throw;
        }
    }

    bool RSAPublic_JWKey :: isRSA () const
    {
        return true;
    }

    const RSAPublic_JWKey * RSAPublic_JWKey :: toRSAPublic () const
    {
        return reinterpret_cast < const RSAPublic_JWKey * > ( duplicate () );
    }

    // modulus "n"
    std :: string RSAPublic_JWKey :: getModulus () const
    {
        return props -> getValue ( "n" ) . toString ();
    }

    // exponent "e"
    std :: string RSAPublic_JWKey :: getExponent () const
    {
        return props -> getValue ( "e" ) . toString ();
    }

    RSAPublic_JWKey * RSAPublic_JWKey :: make ( JSONObject * props )
    {
        // TBD - check kty, alg, n, e
        return new RSAPublic_JWKey ( props );
    }

    // "kty" = "RSA"
    RSAPublic_JWKey :: RSAPublic_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }

}
