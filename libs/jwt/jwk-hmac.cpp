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
    const HMAC_JWKey * HMAC_JWKey :: make ( unsigned int key_bits,
        const std :: string & use, const std :: string & alg, const std :: string & kid )
    {
        JSONObject * props = JSONObject :: make ();
        try
        {
            props -> setValueOrDelete ( "kty", JSONValue :: makeString ( "oct" ) );
            props -> setValueOrDelete ( "kid", JSONValue :: makeString ( kid ) );
            props -> setValueOrDelete ( "alg", JSONValue :: makeString ( alg ) );
            props -> setValueOrDelete ( "use", JSONValue :: makeString ( use ) );

            // TBD - create key and set value
            props -> setValueOrDelete ( "k", JSONValue :: makeString ( kid ) );

            return make ( props );
        }
        catch ( ... )
        {
            props -> invalidate ();
            delete props;
            throw;
        }
    }

    bool HMAC_JWKey :: isSymmetric () const
    {
        return true;
    }

    const HMAC_JWKey * HMAC_JWKey :: toHMAC () const
    {
        return reinterpret_cast < const HMAC_JWKey * > ( duplicate () );
    }

    // get/set symmetric key "k"
    std :: string HMAC_JWKey :: getValue () const
    {
        return props -> getValue ( "k" ) . toString ();
    }

    HMAC_JWKey * HMAC_JWKey :: make ( JSONObject * props )
    {
        // TBD - inspect props for being a proper HMAC JWK
        return new HMAC_JWKey ( props );
    }

    // "kty" = "oct"
    HMAC_JWKey :: HMAC_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }

}
