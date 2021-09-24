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
#include <ncbi/jws.hpp>

#include <climits>
#include <cassert>

namespace ncbi
{

    bool JWTClaimSetBuilder :: hasIssuer () const
    {
        SLocker lock ( busy );
        return cs . hasIssuer ();
    }

    bool JWTClaimSetBuilder :: hasSubject () const
    {
        SLocker lock ( busy );
        return cs . hasSubject ();
    }

    bool JWTClaimSetBuilder :: hasAudience () const
    {
        SLocker lock ( busy );
        return cs . hasAudience ();
    }

    bool JWTClaimSetBuilder :: hasExpiration () const
    {
        SLocker lock ( busy );
        return cs . hasExpiration ();
    }

    bool JWTClaimSetBuilder :: hasNotBefore () const
    {
        SLocker lock ( busy );
        return cs . hasNotBefore ();
    }

    bool JWTClaimSetBuilder :: hasID () const
    {
        SLocker lock ( busy );
        return cs . hasID ();
    }

    void JWTClaimSetBuilder :: setIssuer ( const StringOrURI & iss )
    {
        URIMgr :: validateStringOrURI ( iss );

        XLocker lock ( busy );
        cs . ucs . claims -> setValue ( "iss", JSON :: makeString ( iss ) );
    }

    void JWTClaimSetBuilder :: setSubject ( const StringOrURI & sub )
    {
        URIMgr :: validateStringOrURI ( sub );

        XLocker lock ( busy );
        cs . ucs . claims -> setValue ( "sub", JSON :: makeString ( sub ) );
    }

    void JWTClaimSetBuilder :: addAudience ( const StringOrURI & aud )
    {
        URIMgr :: validateStringOrURI ( aud );

        XLocker lock ( busy );

        // first time audience gets added as simple string
        if ( ! cs . ucs . claims -> exists ( "aud" ) )
            cs . ucs . claims -> setValue ( "aud", JSON :: makeString ( aud ) );
        else
        {
            // get previous entry
            JSONValue * p = & cs . ucs . claims -> getValue ( "aud" );

            // if it's not an array, convert to an array
            if ( ! p -> isArray () )
            {
                // remove the string
                JSONValueRef r = cs . ucs . claims -> removeValue ( "aud" );

                // add in an array
                cs . ucs . claims -> addValue ( "aud", JSON :: makeArray () . release () );

                // put the string back into the array as first element
                p = & cs . ucs . claims -> getValue ( "aud" );
                JSONArray & a = p -> toArray ();
                a . appendValue ( r );
            }

            JSONArray & auda = p -> toArray ();
            auda . appendValue ( JSON :: makeString ( aud ) );
        }
    }

    void JWTClaimSetBuilder :: setDuration ( long long int dur_seconds )
    {
        // enforce bounds and some policy
        if ( dur_seconds != LLONG_MAX )
        {
            if ( dur_seconds + ( long long int ) JWTMgr :: dflt . zero_dur_allowed <= 0 )
            {
                throw JWTBadDurationValue (
                    XP ( XLOC )
                    << "invalid JWT duration"
                    );
            }
        }

        XLocker lock ( busy );
        cs . duration = dur_seconds;

        // invalidate absolute expiration
        if ( cs . hasExpiration () )
            cs . ucs . claims -> deleteValue ( "exp" );
    }

    void JWTClaimSetBuilder :: setAbsExpiration ( long long int exp_seconds )
    {
        // enforce bounds
        if ( exp_seconds < 0 )
        {
            throw JWTBadTimestamp (
                XP ( XLOC )
                << "invalid JWT 'exp' timestamp"
                );
        }

        XLocker lock ( busy );
        cs . ucs . claims -> setValue ( "exp", JSON :: makeInteger ( exp_seconds ) );

        // invalidate relative duration
        cs . duration = -1;
    }

    void JWTClaimSetBuilder :: setNotBefore ( long long int nbf_seconds )
    {
        if ( nbf_seconds < 0 )
        {
            throw JWTBadTimestamp (
                XP ( XLOC )
                << "invalid JWT 'nbf' timestamp"
                );
        }

        XLocker lock ( busy );
        cs . ucs . claims -> setValue ( "nbf", JSON :: makeInteger ( nbf_seconds ) );
    }

    void JWTClaimSetBuilder :: setID ( const String & unique_jti )
    {
        if ( unique_jti . isEmpty () )
        {
            throw JWTInvalidClaims (
                XP ( XLOC )
                << "empty JWT identifier"
                );
        }

        XLocker lock ( busy );
        cs . ucs . claims -> setValue ( "jti", JSON :: makeString ( unique_jti ) );
    }

    StringOrURI JWTClaimSetBuilder :: getIssuer () const
    {
        SLocker lock ( busy );
        return cs . getIssuer ();
    }

    StringOrURI JWTClaimSetBuilder :: getSubject () const
    {
        SLocker lock ( busy );
        return cs . getSubject ();
    }

    std :: vector < StringOrURI > JWTClaimSetBuilder :: getAudience () const
    {
        SLocker lock ( busy );
        return cs . getAudience ();
    }

    long long int JWTClaimSetBuilder :: getDuration () const
    {
        SLocker lock ( busy );
        return cs . duration;
    }

    long long int JWTClaimSetBuilder :: getAbsExpiration () const
    {
        SLocker lock ( busy );
        return cs . getExpiration ();
    }

    long long int JWTClaimSetBuilder :: getNotBefore () const
    {
        SLocker lock ( busy );
        return cs . getNotBefore ();
    }

    String JWTClaimSetBuilder :: getID () const
    {
        SLocker lock ( busy );
        return cs . getID ();
    }

    void JWTClaimSetBuilder :: setClaim ( const String & name, const JSONValueRef & value )
    {
        if ( reserved . claimExists ( name ) )
        {
            throw JWTInvalidClaims (
                XP ( XLOC )
                << "the claim name '"
                << name
                << "' cannot be set"
                << xcause
                << "the name is reserved"
                << xsuggest
                << "use formatted setter"
                );
        };

        XLocker lock ( busy );
        cs . ucs . claims -> setValue ( name, value );
    }

    void JWTClaimSetBuilder :: deleteClaim ( const String & name )
    {
        XLocker lock ( busy );
        cs . ucs . claims -> deleteValue ( name );
    }

    std :: vector < String > JWTClaimSetBuilder :: getNames () const
    {
        SLocker lock ( busy );
        return cs . getNames ();
    }

    bool JWTClaimSetBuilder :: hasClaim ( const String & name ) const
    {
        SLocker lock ( busy );
        return cs . hasClaim ( name );
    }

    const JSONValue & JWTClaimSetBuilder :: getClaim ( const String & name ) const
    {
        SLocker lock ( busy );
        return cs . getClaim ( name );
    }

    void JWTClaimSetBuilder :: setHeader ( const String & name, const JSONValueRef & value )
    {
        if ( reserved . headerExists ( name ) )
        {
            throw JWTInvalidClaims (
                XP ( XLOC )
                << "the header member '"
                << name
                << "' cannot be set"
                << xcause
                << "the name is reserved"
                << xsuggest
                << "use formatted setter if available"
                );
        };

        XLocker lock ( busy );
        cs . ucs . jose -> setValue ( name, value );
    }

    std :: vector < String > JWTClaimSetBuilder :: getHdrNames () const
    {
        SLocker lock ( busy );
        return cs . getHdrNames ();
    }

    bool JWTClaimSetBuilder :: hasHeader ( const String & name ) const
    {
        SLocker lock ( busy );
        return cs . hasHeader ( name );
    }

    const JSONValue & JWTClaimSetBuilder :: getHeader ( const String & name ) const
    {
        SLocker lock ( busy );
        return cs . getHeader ( name );
    }

    JWTClaimSetRef JWTClaimSetBuilder :: toClaimSet () const
    {
        return new JWTClaimSet ( cs );
    }

    JWTClaimSetRef JWTClaimSetBuilder :: stealClaimSet ()
    {
        XLocker lock ( busy );
        JWTClaimSetRef ref = new JWTClaimSet
            ( cs . ucs . jose . release (), cs . ucs . claims . release () );
        ref -> duration = cs . duration;
        cs . duration = -1;
        return ref;
    }

    String JWTClaimSetBuilder :: toJSON () const
    {
        SLocker lock ( busy );
        return cs . toJSON ();
    }

    String JWTClaimSetBuilder :: readableJSON ( unsigned int indent ) const
    {
        SLocker lock ( busy );
        return cs . readableJSON ( indent );
    }

    JWTClaimSetBuilder & JWTClaimSetBuilder :: operator = ( const JWTClaimSetBuilder & csb )
    {
        XLocker lock1 ( busy );
        SLocker lock2 ( csb . busy );

        cs = csb . cs;
        return * this;
    }

    JWTClaimSetBuilder & JWTClaimSetBuilder :: operator = ( JWTClaimSetBuilder && csb )
    {
        XLocker lock1 ( busy );
        XLocker lock2 ( csb . busy );

        cs = std :: move ( csb . cs );

        return * this;
    }

    JWTClaimSetBuilder :: JWTClaimSetBuilder ( const JWTClaimSetBuilder & csb )
        : cs ( JSONObjectRef ( nullptr ), JSONObjectRef ( nullptr ) )
    {
        SLocker lock ( csb . busy );
        cs = csb . cs;
    }

    JWTClaimSetBuilder :: JWTClaimSetBuilder ( JWTClaimSetBuilder && csb )
        : cs ( JSONObjectRef ( nullptr ), JSONObjectRef ( nullptr ) )
    {
        XLocker lock ( csb . busy );
        cs = std :: move ( csb . cs );
    }

    JWTClaimSetBuilder :: ~ JWTClaimSetBuilder ()
    {
    }

    JWTClaimSetBuilder :: JWTClaimSetBuilder ( const JSONObjectRef & jose, const JSONObjectRef & claims )
        : cs ( jose, claims )
    {
    }

    JWTClaimSetBuilder :: Reserved JWTClaimSetBuilder :: reserved;


    bool JWTClaimSetBuilder :: Reserved :: claimExists ( const String & name ) const noexcept
    {
        return claims . find ( name ) != claims . end ();
    }

    bool JWTClaimSetBuilder :: Reserved :: headerExists ( const String & name ) const noexcept
    {
        return JWSMgr :: hdrReserved ( name );
    }

    JWTClaimSetBuilder :: Reserved :: Reserved ()
    {
        const char * clist [] =
        {
            "iss", "sub", "aud", "exp", "nbf", "iat", "jti"
        };

        for ( size_t i = 0; i < sizeof clist / sizeof clist [ 0 ]; ++ i )
        {
            claims . emplace ( String ( clist [ i ] ) );
        }
    }

    JWTClaimSetBuilder :: Reserved :: ~ Reserved ()
    {
    }
}
