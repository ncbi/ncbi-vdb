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

#include <jwt/jwa.hpp>
#include <jwt/jwk.hpp>
#include <jwt/jwt.hpp>

#include "jwt-map-impl.hpp"
#include "jwt-set-impl.hpp"

#include <iostream>
#include <cstring>
#include <cassert>

/*
  This unit implements a global singleton algorithm factory.

  The factory will be constructed at static construction time,
  i.e. before running "main".

  Specific algorithms, defined in other units, will ALSO be
  constructed at static construction time, and will register
  themselves with the global singleton.

  The specific algorithms may be constructed BEFORE the factory,
  making for a little bit of a tricky case solved as usual by
  depending upon clearing of BSS.

  The factory is used to create an algorithm that is bound to
  its parameters, e.g. a signing or verification key.
 */

namespace ncbi
{

    /* JWAKeyHolder
     *  holds a key
     *  performs a brute force overwrite of key memory when destroyed
     */

    const JwtString & JWAKeyHolder :: authority_name () const
    {
        return nam;
    }

    const JwtString & JWAKeyHolder :: algorithm () const
    {
        return alg;
    }

    JwtString JWAKeyHolder :: keyID () const
    {
        return key -> getID ();
    }

    JWAKeyHolder :: JWAKeyHolder ( const JwtString & _nam,
            const JwtString & _alg, const JWK * _key )
        : nam ( _nam )
        , alg ( _alg )
        , key ( nullptr )
    {
        key = _key -> duplicate ();
    }

    JWAKeyHolder :: ~ JWAKeyHolder ()
    {
        key -> release ();
        key = nullptr;
    }

    /* JWASigner
     *  bound to a key
     *  holds the algorithm for reporting
     *  performs the operation of generating a signature
     */

    JWASigner :: JWASigner ( const JwtString & name,
            const JwtString & alg, const JWK * key )
        : JWAKeyHolder ( name, alg, key )
    {
    }

    /* JWAVerifier
     *  bound to a key
     *  holds the algorithm for reporting
     *  performs the operation of verifying a signature
     */

    JWAVerifier :: JWAVerifier ( const JwtString & name,
            const JwtString & alg, const JWK * key )
        : JWAKeyHolder ( name, alg, key )
    {
    }

    /* JWASignerFact
     *  creates a JWASigner
     *  binds the signer to a key
     */

    JWASignerFact :: ~ JWASignerFact ()
    {
    }

    /* JWAVerifierFact
     *  creates a JWAVerifier
     *  binds the verifier to a key
     */

    JWAVerifierFact :: ~ JWAVerifierFact ()
    {
    }

    /* JWAFactory
     *  makes JWAs, which are algorithms of some sort
     *  these are basically interface objects
     *  with polymorphic implementations
     */

    JWASigner * JWAFactory :: makeSigner ( const JwtString & name,
            const JwtString & alg, const JWK * key ) const
    {
        // NB - expect this to be called after static constructors run
        assert ( maps != nullptr );

        // check the key
        if ( key == nullptr )
            throw JWTException ( __func__, __LINE__, "NULL key reference" );
        if ( ! key -> forSigning () )
            throw JWTException ( __func__, __LINE__, "invalid key for signing" );
        if ( ! key -> isSymmetric () && ! key -> isPrivate () )
            throw JWTException ( __func__, __LINE__, "invalid key for signing" );

        // find factory factory
        auto it = const_cast < const Maps * > ( maps ) -> signer_facts . find ( alg );
        if ( it == const_cast < const Maps * > ( maps ) -> signer_facts . cend () )
        {
            JwtString msg ( "signing algorithm '" );
            msg += alg;
            msg += "' not registered.";
            throw JWTException ( __func__, __LINE__, msg . c_str () );
        }

        // pull out the factory factory
        const JWASignerFact * fact = it -> second;
        assert ( fact != nullptr );

        // create the signer
        return fact -> make ( name, alg, key );
    }

    JWAVerifier * JWAFactory :: makeVerifier ( const JwtString & name,
            const JwtString & alg, const JWK * key ) const
    {
        // NB - expect this to be called after static constructors run
        assert ( maps != nullptr );

        // check the key
        if ( key == nullptr )
            throw JWTException ( __func__, __LINE__, "NULL key reference" );
        if ( ! key -> forSigning () )
            throw JWTException ( __func__, __LINE__, "invalid key for signature verification" );

        // find factory factory
        auto it = const_cast < const Maps * > ( maps ) -> verifier_facts . find ( alg );
        if ( it == const_cast < const Maps * > ( maps ) -> verifier_facts . cend () )
        {
            JwtString msg ( "signing algorithm '" );
            msg += alg;
            msg += "' not registered.";
            throw JWTException ( __func__, __LINE__, msg . c_str () );
        }

        // pull out the factory factory
        const JWAVerifierFact * fact = it -> second;
        assert ( fact != nullptr );

        // create the verifier
        return fact -> make ( name, alg, key );
    }

    void JWAFactory :: registerSignerFact ( const JwtString & alg, JWASignerFact * fact )
    {
        // NB - can be called BEFORE constructor runs
        makeMaps ();

        // test algorithm for known/acceptable
        auto ok = maps -> sign_accept . find ( alg );
        assert ( ok != maps -> sign_accept . end () );
        if ( ok != maps -> sign_accept . end () )
        {
            // factory should be alright...
            assert ( fact != nullptr );
            if ( fact != nullptr )
            {
                auto it = maps -> signer_facts . find ( alg );
                if ( it == maps -> signer_facts . end () )
                {
                    maps -> signer_facts . emplace ( alg, fact );
                }
                else if ( it -> second != fact )
                {
                    delete it -> second;
                    it -> second = fact;
                }
            }
        }
    }

    bool JWAFactory :: acceptJWKAlgorithm ( const JwtString & alg ) const
    {
        {
            auto found = maps -> verifier_facts . find ( alg );
            if ( found != maps -> verifier_facts . end () )
                return true;
        }
        {
            auto found = maps -> signer_facts . find ( alg );
            if ( found != maps -> signer_facts . end () )
                return true;
        }
        return false;
    }

    void JWAFactory :: registerVerifierFact ( const JwtString & alg, JWAVerifierFact * fact )
    {
        // NB - can be called BEFORE constructor runs
        makeMaps ();

        auto ok = maps -> sign_accept . find ( alg );
        assert ( ok != maps -> sign_accept . end () );
        if ( ok != maps -> sign_accept . end () )
        {
            // factory should be alright...
            assert ( fact != nullptr );
            if ( fact != nullptr )
            {
                auto it = maps -> verifier_facts . find ( alg );
                if ( it == maps -> verifier_facts . end () )
                {
                    maps -> verifier_facts . emplace ( alg, fact );
                }
                else if ( it -> second != fact )
                {
                    delete it -> second;
                    it -> second = fact;
                }
            }
        }
    }

    JWAFactory :: JWAFactory ()
    {
        // NB - may already be constructed BEFORE this constructor runs
        // depends upon zeroed out static data
        makeMaps ();

        // include algorithms for static linking
        //includeJWA_none ( false );
        includeJWA_hmac ( false );
        includeJWA_rsa  ( false );
    }

    JWAFactory :: ~ JWAFactory ()
    {
        delete maps;
        maps = nullptr;
    }

    void JWAFactory :: makeMaps ()
    {
        if ( maps == nullptr )
        {
            maps = new Maps;
        }
    }

    JWAFactory :: Maps :: Maps ()
    {
        // TEMPORARY - for initial testing
        sign_accept . emplace ( "none" );

        // don't accept registration of ANY other algorithms by name
        sign_accept . emplace ( "HS256" );
        sign_accept . emplace ( "HS384" );
        sign_accept . emplace ( "HS512" );
        sign_accept . emplace ( "RS256" );
        sign_accept . emplace ( "RS384" );
        sign_accept . emplace ( "RS512" );
        sign_accept . emplace ( "ES256" );
        sign_accept . emplace ( "ES384" );
        sign_accept . emplace ( "ES512" );
        sign_accept . emplace ( "PS256" );
        sign_accept . emplace ( "PS384" );
        sign_accept . emplace ( "PS512" );
    }

    JWAFactory :: Maps :: ~ Maps ()
    {
    }

    // global singleton
    JWAFactory gJWAFactory;

}
