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

#include <ncbi/jwt.hpp>

namespace ncbi
{

    bool UnverifiedJWTClaims :: hasIssuer () const noexcept
    {
        return claims -> exists ( "iss" );
    }

    bool UnverifiedJWTClaims :: hasSubject () const noexcept
    {
        return claims -> exists ( "sub" );
    }

    bool UnverifiedJWTClaims :: hasAudience () const noexcept
    {
        return claims -> exists ( "aud" );
    }

    bool UnverifiedJWTClaims :: hasExpiration () const noexcept
    {
        return claims -> exists ( "exp" );
    }

    bool UnverifiedJWTClaims :: hasNotBefore () const noexcept
    {
        return claims -> exists ( "nbf" );
    }

    bool UnverifiedJWTClaims :: hasIssuedAt () const noexcept
    {
        return claims -> exists ( "iat" );
    }

    bool UnverifiedJWTClaims :: hasID () const noexcept
    {
        return claims -> exists ( "jti" );
    }

    StringOrURI UnverifiedJWTClaims :: getIssuer () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "iss" ) . toString ();
    }

    StringOrURI UnverifiedJWTClaims :: getSubject () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "sub" ) . toString ();
    }

    std :: vector < StringOrURI > UnverifiedJWTClaims :: getAudience () const
    {
        std :: vector < StringOrURI > rtn;

        const JSONObject & cs = * claims;
        const JSONValue & audv = cs . getValue ( "aud" );

        if ( audv . isArray () )
        {
            const JSONArray & auda = audv . toArray ();
            unsigned long int i, count = auda . count ();
            for ( i = 0; i < count; ++ i )
            {
                rtn . push_back ( auda [ i ] . toString () );
            }
        }
        else
        {
            rtn . push_back ( audv . toString () );
        }

        return rtn;
    }

    long long int UnverifiedJWTClaims :: getExpiration () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "exp" ) . toInteger ();
    }

    long long int UnverifiedJWTClaims :: getNotBefore () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "nbf" ) . toInteger ();
    }

    long long int UnverifiedJWTClaims :: getIssuedAt () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "iat" ) . toInteger ();
    }

    String UnverifiedJWTClaims :: getID () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "jti" ) . toString ();
    }

    std :: vector < String > UnverifiedJWTClaims :: getNames () const
    {
        return claims -> getNames ();
    }

    bool UnverifiedJWTClaims :: hasClaim ( const String & name ) const noexcept
    {
        return claims -> exists ( name );
    }

    const JSONValue & UnverifiedJWTClaims :: getClaim ( const String & name ) const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( name );
    }

    std :: vector < String > UnverifiedJWTClaims :: getHdrNames () const
    {
        return jose -> getNames ();
    }

    bool UnverifiedJWTClaims :: hasHeader ( const String & name ) const noexcept
    {
        return jose -> exists ( name );
    }

    const JSONValue & UnverifiedJWTClaims :: getHeader ( const String & name ) const
    {
        const JSONObject & cs = * jose;
        return cs . getValue ( name );
    }

    const std :: vector < JWKRef > & UnverifiedJWTClaims :: getVerificationKeySeq () const noexcept
    {
        return vkeys;
    }

    String UnverifiedJWTClaims :: toJSON () const
    {
        JSONObjectRef claims_cpy = claims -> cloneObject ();
        JWTMgr :: finalizeClaims ( * claims_cpy, 10, JWTMgr :: now () );
        return claims_cpy -> toJSON ();
    }

    String UnverifiedJWTClaims :: readableJSON ( unsigned int indent ) const
    {
        JSONObjectRef claims_cpy = claims -> cloneObject ();
        JWTMgr :: finalizeClaims ( * claims_cpy, 10, JWTMgr :: now () );
        return claims_cpy -> readableJSON ( indent );
    }

    UnverifiedJWTClaims & UnverifiedJWTClaims :: operator = ( const UnverifiedJWTClaims & ucs )
    {
        JSONObjectRef _jose = ucs . jose -> cloneObject ();
        JSONObjectRef _claims = ucs . claims -> cloneObject ();
        vkeys = ucs . vkeys;
        claims = _claims;
        jose = _jose;

        return * this;
    }

    UnverifiedJWTClaims & UnverifiedJWTClaims :: operator = ( UnverifiedJWTClaims && ucs )
    {
        jose = std :: move ( ucs . jose );
        claims = std :: move ( ucs . claims );
        vkeys = std :: move ( ucs . vkeys );

        return * this;
    }

    UnverifiedJWTClaims :: UnverifiedJWTClaims ( const UnverifiedJWTClaims & ucs )
        : jose ( ucs . jose -> cloneObject () )
        , claims ( ucs . claims -> cloneObject () )
        , vkeys ( ucs . vkeys )
    {
    }

    UnverifiedJWTClaims :: UnverifiedJWTClaims ( UnverifiedJWTClaims && ucs )
        : jose ( std :: move ( ucs . jose ) )
        , claims ( std :: move ( ucs . claims ) )
        , vkeys ( std :: move ( ucs . vkeys ) )
    {
    }

    UnverifiedJWTClaims :: ~ UnverifiedJWTClaims ()
    {
    }

    UnverifiedJWTClaims :: UnverifiedJWTClaims ( const JSONObjectRef & _jose, const JSONObjectRef & _claims )
        : jose ( _jose )
        , claims ( _claims )
    {
    }

    UnverifiedJWTClaims :: UnverifiedJWTClaims ( const std :: vector < JWKRef > & keys,
            const JSONObjectRef & _jose, const JSONObjectRef & _claims )
        : jose ( _jose )
        , claims ( _claims )
        , vkeys ( keys )
    {
    }
}
