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
    bool JWK :: forSigning () const
    {
        try
        {
            return props -> getValue ( "use" ) . toString () . compare ( "sig" ) == 0;
        }
        catch ( ... )
        {
        }
        return false;
    }

    bool JWK :: forEncryption () const
    {
        try
        {
            return props -> getValue ( "use" ) . toString () . compare ( "enc" ) == 0;
        }
        catch ( ... )
        {
        }
        return false;
    }

    bool JWK :: isSymmetric () const
    {
        return false;
    }

    bool JWK :: isPrivate () const
    {
        return false;
    }

    bool JWK :: isRSA () const
    {
        return false;
    }

    bool JWK :: isEllipticCurve () const
    {
        return false;
    }

    const HMAC_JWKey * JWK :: toHMAC () const
    {
        throw JWTException ( __func__, __LINE__, "invalid cast of JWK" );
    }

    const RSAPrivate_JWKey * JWK :: toRSAPrivate () const
    {
        throw JWTException ( __func__, __LINE__, "invalid cast of JWK" );
    }

    const RSAPublic_JWKey * JWK :: toRSAPublic () const
    {
        throw JWTException ( __func__, __LINE__, "invalid cast of JWK" );
    }

    const EllipticCurvePrivate_JWKey * JWK :: toEllipticCurvePrivate () const
    {
        throw JWTException ( __func__, __LINE__, "invalid cast of JWK" );
    }

    const EllipticCurvePublic_JWKey * JWK :: toEllipticCurvePublic () const
    {
        throw JWTException ( __func__, __LINE__, "invalid cast of JWK" );
    }


    // "kty"
    //  MANDATORY in a JWK (section 4.1)
    //  legal values "oct", "RSA", "EC"
    std :: string JWK :: getType () const
    {
        return props -> getValue ( "kty" ) . toString ();
    }

    // "kid"
    //  optional (section 4.5), but
    //  our library currently makes it MANDATORY
    std :: string JWK :: getID () const
    {
        return props -> getValue ( "kid" ) . toString ();
    }

    // "alg"
    //  identifies the algorithm (section 4.4)
    std :: string JWK :: getAlg () const
    {
        return props -> getValue ( "alg" ) . toString ();
    }

    // "use"
    //  only for public keys (section 4.2)
    //  legal values are "sig" (signature) and "enc" (encryption)
    std :: string JWK :: getUse () const
    {
        return props -> getValue ( "use" ) . toString ();
    }

    std :: string JWK :: toJSON () const
    {
        return props -> toJSON ();
    }

    std :: string JWK :: readableJSON ( unsigned int indent ) const
    {
        return props -> readableJSON ( indent );
    }

    // primitive memory management
    const JWK * JWK :: duplicate () const
    {
        ++ refcount;
        return this;
    }

    void JWK :: release () const
    {
        if ( -- refcount == 0 )
            delete this;
    }

    void JWK :: checkProperties ( const JSONObject * props )
    {
        if ( props == nullptr )
            throw JWTException ( __func__, __LINE__, "null properties object" );

        if ( props -> exists ( "use" ) )
        {
            std :: string use = props -> getValue ( "use" ) . toString ();
            if ( use . compare ( "sig" ) != 0 &&
                 use . compare ( "enc" ) != 0 )
            {
                throw JWTException ( __func__, __LINE__, "illegal 'use' property value" );
            }
        }

        if ( props -> exists ( "alg" ) )
        {
            std :: string alg = props -> getValue ( "alg" ) . toString ();
            if ( ! gJWAFactory . acceptJWKAlgorithm ( alg ) )
                throw JWTException ( __func__, __LINE__, "unknown 'alg' property value" );
        }
    }

    JWK :: ~ JWK ()
    {
        props -> invalidate ();
        delete props;
    }

    JWK :: JWK ( JSONObject * _props )
        : props ( _props )
        , refcount ( 1 )
    {
    }


    bool JWKSet :: isEmpty () const
    {
        return map . empty ();
    }

    unsigned long int JWKSet :: count () const
    {
        return map . size ();
    }

    bool JWKSet :: contains ( const std :: string & kid ) const
    {
        auto it = map . find ( kid );

        if ( it == map . end () )
                return false;

        return true;
    }

    std :: vector < std :: string > JWKSet :: getKeyIDs () const
    {
        std :: vector < std :: string > ids;

        for ( auto it = map . cbegin(); it != map . cend (); ++ it )
            ids . push_back ( it -> first );

        return ids;
    }

    void JWKSet :: addKey ( const JWK * jwk )
    {
        if ( jwk != nullptr )
        {
            std :: string kid = jwk -> getID ();
            auto it = map . find ( kid );
            if ( it != map . end () )
            {
                std :: string what ( "duplicate key id: '" );
                what += kid;
                what += "'";
                throw JWTException ( __func__, __LINE__, what . c_str () );
            }

            JSONObject * props = ( JSONObject * ) jwk -> props -> clone ();
            try
            {
                map . emplace ( kid, jwk );
                kset -> getValue ( "keys" ) . toArray () . appendValue ( props );
            }
            catch ( ... )
            {
                delete props;
                throw;
            }
        }
    }

    const JWK * JWKSet :: getKey ( const std :: string & kid ) const
    {
        auto it = map . find ( kid );
        if ( it == map . end () )
        {
            std :: string what ( "key not found: id = '" );
            what += kid;
            what += "'";
            throw JWTException ( __func__, __LINE__, what . c_str () );
        }

        return it -> second;
    }

    void JWKSet :: removeKey ( const std :: string & kid )
    {
        throw JWTException ( __func__, __LINE__, "UNIMPLEENTED" );
    }

    JWKSet :: JWKSet ( const JWKSet & ks )
        : kset ( nullptr )
    {
        kset = ( JSONObject * ) ks . kset -> clone ();
        if ( ! ks . map . empty () )
        {
            for ( auto it = ks . map . begin (); it != ks . map . end (); )
            {
                const JWK * jwk = it -> second -> duplicate ();
                try
                {
                    map . emplace ( it -> first, jwk );
                }
                catch ( ... )
                {
                    jwk -> release ();
                    throw;
                }
            }
        }
    }

    JWKSet & JWKSet :: operator = ( const JWKSet & ks )
    {
        delete kset;
        kset = ( JSONObject * ) ks . kset -> clone ();

        if ( ! map . empty () )
        {
            for ( auto it = map . begin(); it != map . end (); )
            {
                it -> second -> release ();
                it = map . erase ( it );
            }
        }

        if ( ! ks . map . empty () )
        {
            for ( auto it = ks . map . begin (); it != ks . map . end (); )
            {
                const JWK * jwk = it -> second -> duplicate ();
                try
                {
                    map . emplace ( it -> first, jwk );
                }
                catch ( ... )
                {
                    jwk -> release ();
                    throw;
                }
            }
        }

        return * this;
    }

    JWKSet :: JWKSet ()
        : kset ( nullptr )
    {
        kset = JSONObject :: make ();
        kset -> setValueOrDelete ( "keys", JSONArray :: make () );
    }

    JWKSet :: ~ JWKSet ()
    {
        if ( ! map . empty () )
        {
            for ( auto it = map . begin(); it != map . end (); )
            {
                it -> second -> release ();
                it = map . erase ( it );
            }
        }

        kset -> invalidate ();
        delete kset;
    }

}
