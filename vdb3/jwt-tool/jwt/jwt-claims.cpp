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

#include <cassert>

namespace ncbi
{

    long long int JWTClaimSet :: getValidAsOf () const noexcept
    {
        const JSONObject & cs = * ucs . claims;
        if ( cs . exists ( "nbf" ) )
            return cs . getValue ( "nbf" ) . toInteger ();
        if ( cs . exists ( "iat" ) )
            return cs . getValue ( "iat" ) . toInteger ();
        return 0;
    }

    long long int JWTClaimSet :: getDuration () const noexcept
    {
        const JSONObject & cs = * ucs . claims;
        if ( cs . exists ( "exp" ) )
        {
            long long int exp = cs . getValue ( "exp" ) . toInteger ();
            return exp - getValidAsOf ();
        }

        return duration;
    }

    String JWTClaimSet :: toJSON () const
    {
        JSONObjectRef claims_cpy = ucs . claims -> cloneObject ();
        JWTMgr :: finalizeClaims ( * claims_cpy, duration, JWTMgr :: now () );
        return claims_cpy -> toJSON ();
    }

    String JWTClaimSet :: readableJSON ( unsigned int indent ) const
    {
        JSONObjectRef claims_cpy = ucs . claims -> cloneObject ();
        JWTMgr :: finalizeClaims ( * claims_cpy, duration, JWTMgr :: now () );
        return claims_cpy -> readableJSON ( indent );
    }

    JWTClaimSet & JWTClaimSet :: operator = ( const JWTClaimSet & cs )
    {
        ucs = cs . ucs;
        duration = cs . duration;

        return * this;
    }

    JWTClaimSet & JWTClaimSet :: operator = ( JWTClaimSet && cs )
    {
        ucs = std :: move ( cs . ucs );
        duration = cs . duration;

        return * this;
    }

    JWTClaimSet :: JWTClaimSet ( const JWTClaimSet & cs )
        : ucs ( cs . ucs )
        , duration ( cs . duration )
    {
    }

    JWTClaimSet :: JWTClaimSet ( JWTClaimSet && cs )
        : ucs ( std :: move ( cs . ucs ) )
        , duration ( cs . duration )
    {
    }

    JWTClaimSet :: ~ JWTClaimSet ()
    {
    }

    JWTClaimSet :: JWTClaimSet ( const JSONObjectRef & jose, const JSONObjectRef & claims )
        : ucs ( jose, claims )
        , duration ( -1 )
    {
    }

    JWTClaimSet :: JWTClaimSet ( const UnverifiedJWTClaims & claims )
        : ucs ( claims )
        , duration ( -1 )
    {
    }
}
