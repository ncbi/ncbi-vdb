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
 *  Government have not placed any rem_striction on its use or reproduction.
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

#include <jwt/jwt.h>

#include <klib/text.h>
#include <jwt/jwk.hpp>
#include <jwt/jwt.hpp>
#include <jwt/jws.hpp>
#include <jwt/json.hpp>

//TODO: catch exceptions, convert into RC

LIB_EXPORT
JSONValue * CC JSONValue_makeString ( const String * val )
{
    assert ( val != nullptr );
    return ( JSONValue * ) ncbi :: JSONValue :: makeString ( ncbi :: JwtString ( val -> addr, val -> size) );
}

LIB_EXPORT
rc_t CC JSONValue_toString ( const JSONValue * p_self, const String ** p_str )
{
    assert ( p_self != nullptr );
    assert ( p_str != nullptr );
    const ncbi::JSONValue * self = (const ncbi::JSONValue *) p_self;
    ncbi::JwtString src = self -> toString();
    String s;
    StringInitCString( & s, src . c_str () );
    return StringCopy ( p_str, & s);
}

LIB_EXPORT
const HMAC_JWKey * CC
HMAC_JWKey_make ( unsigned int key_bits,
    const String * use, const String * alg, const String * kid )
{
    return ( const HMAC_JWKey * ) ncbi :: HMAC_JWKey :: make(key_bits, use->addr, alg->addr, kid->addr);
}

LIB_EXPORT
void CC
HMAC_JWKey_dtor ( const HMAC_JWKey * self )
{
    delete ( ncbi::HMAC_JWKey * ) self;
}

LIB_EXPORT
JWSFactory * CC JWSFactory_ctor ( const String * authority_name, const String * alg, const JWK * key )
{
    assert ( authority_name != nullptr );
    assert ( alg != nullptr );
    assert ( key != nullptr );
    return ( JWSFactory * ) new ncbi::JWSFactory( ncbi :: JwtString ( authority_name -> addr, authority_name -> size),
                                                  ncbi :: JwtString ( alg -> addr, alg -> size),
                                                  (const ncbi::JWK*)key );
}

LIB_EXPORT
void CC
JWSFactory_dtor ( JWSFactory * p_self )
{
    ncbi::JWSFactory * self = (ncbi::JWSFactory *) p_self;
    delete self;
}

LIB_EXPORT
JWTFactory * CC JWTFactory_ctor_default ()
{
    return ( JWTFactory * ) new ncbi :: JWTFactory();
}

LIB_EXPORT
JWTFactory * CC JWTFactory_ctor ( const JWSFactory * jws_fact )
{
    assert ( jws_fact );
    return ( JWTFactory * ) new ncbi :: JWTFactory( * ( const ncbi :: JWSFactory * ) jws_fact );
}

LIB_EXPORT
void CC
JWTFactory_dtor ( JWTFactory * p_self )
{
    ncbi::JWTFactory * self = (ncbi::JWTFactory *) p_self;
    delete self;
}

LIB_EXPORT
JWTClaims * CC
JWTFactory_make ( JWTFactory * p_self )
{
    assert ( p_self != nullptr );
    ncbi :: JWTFactory * self = ( ncbi :: JWTFactory * ) p_self;
    return ( JWTClaims * ) new ncbi :: JWTClaims ( self -> make () );
}

LIB_EXPORT
rc_t CC
JWTFactory_setIssuer ( JWTFactory * p_self, const StringOrURI * iss )
{
    assert ( p_self != nullptr );
    assert ( iss != nullptr );
    ncbi :: JWTFactory * self = ( ncbi :: JWTFactory * ) p_self;
    self->setIssuer ( ncbi :: JwtString ( iss -> addr, iss -> size) );
    return 0;
}

LIB_EXPORT
rc_t CC
JWTFactory_setDuration ( JWTFactory * p_self, long long int dur_seconds )
{
    assert ( p_self != nullptr );
    ncbi :: JWTFactory * self = ( ncbi :: JWTFactory * ) p_self;
    self->setDuration ( dur_seconds );
    return 0;
}

LIB_EXPORT
rc_t CC
JWTClaims_addClaimOrDeleteValue ( JWTClaims * p_self, const String * name, JSONValue * value, bool isFinal )
{
    try
    {
        assert ( p_self != nullptr );
        assert ( value != nullptr );
        ncbi :: JWTClaims * self = ( ncbi :: JWTClaims * ) p_self;
        self -> addClaimOrDeleteValue ( ncbi :: JwtString ( name -> addr, name -> size ), (ncbi::JSONValue*)value, isFinal );
        return 0;
    }
    catch (...)
    {
        return RC(rcText,rcString,rcProcessing,rcError,rcUnexpected);
    }
}

LIB_EXPORT
rc_t CC
JWTClaims_toJSON ( const JWTClaims * p_self, const String ** p_json )
{
    const ncbi::JWTClaims * self = (const ncbi::JWTClaims *) p_self;
    ncbi::JwtString src = self -> toJSON();
    String s;
    StringInitCString( & s, src . c_str () );
    return StringCopy ( p_json, & s);
}

LIB_EXPORT
void CC
JWTClaims_dtor ( JWTClaims * p_self )
{
    ncbi::JWTClaims * self = (ncbi::JWTClaims *) p_self;
    delete self;
}

