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

#include <cassert>

namespace ncbi
{

    bool JWKSet :: isEmpty () const
    {
        SLocker lock ( busy );
        return map . empty ();
    }

    unsigned long int JWKSet :: count () const
    {
        SLocker lock ( busy );
        return ( unsigned long int ) map . size ();
    }

    bool JWKSet :: hasVerificationKeys () const
    {
        SLocker lock ( busy );
        return num_verification_keys != 0;
    }

    bool JWKSet :: contains ( const String & kid ) const
    {
        SLocker lock ( busy );
        auto it = map . find ( kid );
        return it != map . end ();
    }

    std :: vector < String > JWKSet :: getKeyIDs () const
    {
        SLocker lock ( busy );

        std :: vector < String > kids;

        for ( auto it = map . begin (); it != map . end (); ++ it )
        {
            kids . push_back ( it -> first );
        }

        return kids;
    }

    void JWKSet :: addKey ( const JWKRef & key )
    {
        String kid = key -> getID ();

        XLocker lock ( busy );

        auto it = map . find ( kid );
        if ( it != map . end () )
        {
            throw JWKUniqueConstraintViolation (
                XP ( XLOC )
                << "key-id '"
                << kid
                << "' exists"
                );
        }

        // locate the keys array from our props
        JSONArray & keys = kset -> getValue ( "keys" ) . toArray ();

        // obtain the new array index
        unsigned int idx = keys . count ();

        // clone the object
        JSONValueRef cpy ( key -> props -> clone () );

        // append to array
        keys . appendValue ( cpy );

        // verify that the insertion index was correct ( due to non-atomicity )
        assert ( keys [ idx ] . toObject () . getValue ( "kid" ) . toString () . compare ( kid ) == 0 );

        // insert idx and key into map under kid
        std :: pair < unsigned long int, JWKRef > entry ( idx, key );
        map . emplace ( kid, entry );

        num_verification_keys += ( count_t ) key -> forVerifying ();
    }

    JWKRef JWKSet :: getKey ( const String & kid ) const
    {
        SLocker lock ( busy );
        auto it = map . find ( kid );
        if ( it == map . end () )
        {
            throw JWKKeyNotFound (
                XP ( XLOC )
                << "key-id '"
                << kid
                << "' not found"
                );
        }
        return it -> second . second;
    }

    void JWKSet :: removeKey ( const String & kid )
    {
        XLocker lock ( busy );
        auto it = map . find ( kid );
        if ( it != map . end () )
        {
            JSONArray & keys = kset -> getValue ( "keys" ) . toArray ();
            assert ( keys [ it -> second . first ] . toObject () . getValue ( "kid" ) . toString () . compare ( kid ) == 0 );
            num_verification_keys -= ( count_t ) it -> second . second -> forVerifying ();
            keys . removeValue ( it -> second . first );
            map . erase ( it );
        }
    }

    JWKSetRef JWKSet :: clone () const
    {
        return JWKSetRef ( new JWKSet ( * this ) );
    }

    void JWKSet :: invalidate ()
    {
        XLocker lock ( busy );
        map . clear ();
        kset -> invalidate ();
        num_verification_keys = 0;
    }

    JWKSet & JWKSet :: operator = ( const JWKSet & ks )
    {
        XLocker lock1 ( busy );
        SLocker lock2 ( ks . busy );

        map . clear ();
        kset = ks . kset -> cloneObject ();
        num_verification_keys = 0;

        extractKeys ();

        return * this;
    }

    JWKSet :: JWKSet ( const JWKSet & ks )
        : num_verification_keys ( 0 )
    {
        SLocker lock ( ks . busy );
        kset = ks . kset -> cloneObject ();
        extractKeys ();
    }

    JWKSet :: ~ JWKSet () noexcept
    {
        map . clear ();
        kset -> invalidate ();
        num_verification_keys = 0;
    }

    void JWKSet :: extractKeys ()
    {
        const JSONArray & keys = kset -> getValue ( "keys" ) . toArray ();
        unsigned long int i, count = keys . count ();
        for ( i = 0; i < count; ++ i )
        {
            const JSONObject & key = keys [ i ] . toObject ();
            JSONObjectRef cpy ( key . cloneObject () );
            JWKRef jwk ( new JWK ( cpy ) );
            String kid = key . getValue ( "kid" ) . toString ();
            std :: pair < unsigned long int, JWKRef > entry ( i, jwk );
            map . emplace ( kid, entry );
            num_verification_keys += ( count_t ) jwk -> forVerifying ();
        }
    }

    JWKSet :: JWKSet ( const JSONObjectRef & _kset )
        : kset ( _kset )
        , num_verification_keys ( 0 )
    {
        extractKeys ();
    }

}
