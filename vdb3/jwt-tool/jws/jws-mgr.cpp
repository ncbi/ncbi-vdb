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

#include <ncbi/jws.hpp>
#include <ncbi/jwk.hpp>
#include <ncbi/jwa.hpp>
#include <ncbi/uri.hpp>
#include <ncbi/secure/payload.hpp>
#include <ncbi/secure/base64.hpp>

#include <sys/time.h>
#include <ctype.h>
#include <cassert>

#include <string.h>

namespace ncbi
{

    const size_t max_jose_alg_length = 16;
    const size_t max_jose_kid_length = 256;

    // hard-coded policy defaults
    const bool dflt_kid_required = false;
    const bool dflt_kid_gen = dflt_kid_required | true;

    // if true, require the JOSE header to be trivially simple,
    // meaning it is a single-level JSON object with no sub-objects
    // and nothing that prevents it from being parsed by a simple scan
    const bool dflt_require_simple_hdr = true;

    // if the JWS has a 'kid', require it to be
    // prestored in the keyset, without dynamic lookup
    const bool dflt_require_prestored_kid = true;

    // require that the key used to verify signature is
    // prestored in the keyset, without dynamic lookup
    // when not found by 'kid'
    const bool dflt_require_prestored_key = true;

    JWSMgr :: Policy JWSMgr :: dflt;
    JWSMgr :: Reserved JWSMgr :: reserved;



    JWS JWSMgr :: signCompact ( const JWK & key,
        const void * payload, size_t bytes )
    {
        // 1. create an empty JSON Object to act as the JOSE header.
        JSONObjectRef jose = JSON :: makeObject ();

        // 2. continue with private JWS Compact Signing Process 2
        return signCompact ( key, * jose, payload, bytes );
    }

    JWS JWSMgr :: signCompact ( const JWK & key, INOUT JSONObject & jose,
        const void * payload, size_t bytes )
    {
        // 1. STAT attempt to sign
        // this means to record in process-global statistics

        // early test of parameters
        // in anticipation of step #7
        if ( payload == nullptr )
            throw JWSNullPayload ( XP ( XLOC ) << "null payload pointer" );
        if ( bytes == 0 )
            throw JWSEmptyPayload ( XP ( XLOC ) << "no payload to sign" );

        // 2. REQUIRE that the key be appropriate for signing
        //    and an accepted algorithm for signatures.
        if ( ! key . forSigning () )
        {
            throw JWSIncompatibleKey (
                XP ( XLOC )
                << "cannot sign payload"
                << xcause
                << "key is not valid for signing"
                );
        }
        String alg = key . getAlg ();
        JWASignerRef signer = JWAMgr :: getSigner ( alg );

        // 3. REQUIRE that the key have a usable 'kid'
        String kid = key . getID ();
        if ( kid . isEmpty () )
        {
            throw JWSIncompatibleKey (
                XP ( XLOC )
                << "cannot sign payload"
                << xcause
                << "key has empty 'kid'"
                );
        }

        // 4. add 'alg' and 'kid' members with values taken from key.
        jose . setValue ( "alg", JSON :: makeString ( alg ) );
        if ( dflt . kid_gen )
            jose . setValue ( "kid", JSON :: makeString ( kid ) );

        // 5. validate JOSE Header
        verifyHeader ( jose );

        // 6. serialize the JOSE header into a JSON string
        String hdr_json = jose . toJSON ();

        // 7. base64url-encode each the JOSE JSON string and the payload blob
        String hdr_base64 = Base64 :: urlEncode ( hdr_json . data (), hdr_json . size () );
        String pay_base64 = Base64 :: urlEncode ( payload, bytes );

        // 8. concatenate the base64url strings with a '.' delimiter
        StringBuffer jws;
        jws += hdr_base64;
        jws += '.';
        jws += pay_base64;

        // 9. create a base64url-encoded signature of the result of step #8
        Payload signature = signer -> sign ( key, jws . data (), jws . size () );
        String sig_base64 = Base64 :: urlEncode ( signature . data (), signature . size () );

        // 10. concatenate the base64url-encoded signature to its contents with a '.'
        jws += '.';
        jws += sig_base64;

        // 11. ACCEPT
        return jws . stealString ();
    }

    JWKRef JWSMgr :: extract ( OUT JSONObjectRef & jose, OUT Payload & payload,
        const JWKSet & val_keys, const JWS & jws )
    {
        // start by returning NO header or payload
        // do this before any exceptions can be generated
        jose = nullptr;
        payload . wipe ();

        // 1. STAT attempt to decode JWS
        // this means to record in process-global statistics

        // 2. REJECT empty JWS
        if ( jws . isEmpty () )
        {
            // 2a. upstream, this exception should be associated with source
            throw MalformedJWS ( XP ( XLOC ) << "empty JWS" );
        }

        // 3. detect class of first character:
        UTF32 ch = jws [ 0 ];

        // 3a. non-ASCII
        if ( ch >= 128 )
            throw MalformedJWS ( XP ( XLOC ) << "badly formed JWS" );

        // 3b. base64url => required for and only legal in compact
        // 3c. neither space nor '{' => illegal for JSON serialization

        // 4. REJECT if first character is not base64url
        if ( ! isalnum ( ch ) && ch != '-' && ch != '_' )
        {
            // 4a. upstream, this exception should be associated with source
            throw JWSUnsupported (
                XP ( XLOC )
                << "JWS JSON Serialization is not currently supported"
                );
        }

        // 5. continue with private JWS Compact Decoding Process
        return extractCompact ( jose, payload, val_keys, jws );
    }

    JWKRef JWSMgr :: extractCompact ( OUT JSONObjectRef & jose, OUT Payload & payload,
        const JWKSet & keys, const JWS & jws )
    {
        // 1. STAT attempt to decode compact JWS
        // this means to record in process-global statistics

        // 2. REQUIRE that the putative JWS string be 100% ASCII
        if ( ! jws . isAscii () )
        {
            throw MalformedJWS (
                XP ( XLOC )
                << "JWS contains non-ASCII characters"
                );
        }

        // 3. count and map the location of the dots in the string
        count_t dots [ 5 ];
        count_t num_dots = locateDelimiters ( dots, jws );

        // 4. REJECT any count of dots != 2
        if ( num_dots != 2 )
        {
            throw MalformedJWS (
                XP ( XLOC )
                << "JWS contains "
                << ( num_dots + 1 )
                << " '.' delimited sections - expected 3"
                );
        }

        // 5. create a JSON Object to act as JOSE header and a payload
        // already passed in as an output parameter

        // 6. continue with private JWS Compact Decoding Process 2 (private)
        return extractCompact ( jose, payload, keys, jws, dots );
    }

    JWKRef JWSMgr :: extractCompact ( JSONObjectRef & jose, Payload & payload,
        const JWKSet & keys, const JWS & jws, const count_t dots [ 5 ] )
    {
        // start by returning NO header or payload
        // do this before any exceptions can be generated
        jose = nullptr;
        payload . wipe ();

        // 1. REQUIRE a set of keys containing at least 1 key suitable
        //    for signature verification.
        if ( ! keys . hasVerificationKeys () )
        {
            throw JWSBadKeySet (
                XP ( XLOC )
                << "supplied key set contains no keys suitable for verification"
                );
        }

        // 2. use dot map to split out the signed contents and signature
        String content = jws . subString ( 0, dots [ 1 ] );
        String sig_base64 = jws . subString ( dots [ 1 ] + 1 );

        // 3. use dot map to extract the JOSE header component
        String hdr_base64 = jws . subString ( 0, dots [ 0 ] );

        // 4. decode base64url header into JSON string
        // 4a. REJECT illegal characters
        // 4b. REJECT line wrapping
        String hdr_json = Base64 :: urlDecodeText ( hdr_base64, Base64 :: strict_charset );

        // 5. decode base64url signature into Payload
        // 5a. REJECT illegal characters
        // 5b. REJECT line wrapping
        Payload signature = Base64 :: urlDecode ( sig_base64, Base64 :: strict_charset );

        // 6. try Scan Protected Header
        // 6a. must find 'alg'
        // 6b. may find 'kid'
        String alg, kid;
        bool has_alg = scanProtectedHeader ( alg, kid, hdr_json );

        // 7. if Scan Protected Header failed
        JWKRef key;
        JWAVerifierRef verifier;
        if ( ! has_alg )
        {
            // 7a. it represents potential DOS
            // 7b. update and read STAT for result
            // 7c. REJECT if ratio of unsuccessful scans:scans >= threshold
            // 7d. REJECT if policy requires a simple header
            if ( dflt . require_simple_hdr )
            {
                throw MalformedJWS (
                    XP ( XLOC )
                    << "JWS header has unrecognized format"
                    );
            }
        }
        else
        {
            // 8. if Scan Protected Header gave 'alg'
            assert ( ! alg . isEmpty () );
            assert ( alg . isAscii () );

            // 8a. REJECT if 'alg' is unrecognized
            if ( ! JWAMgr :: acceptJWSAlgorithm ( alg ) )
            {
                throw MalformedJWS (
                    XP ( XLOC )
                    << "JWS contains unrecognized alg: '"
                    << alg
                    << '\''
                    );
            }

            // 8b. REJECT if no JWA verifier is found
            try
            {
                verifier = JWAMgr :: getVerifier ( alg );
            }
            catch ( ... )
            {
                throw JWSAlgUnsupported (
                    XP ( XLOC )
                    << "JWS uses unsupported alg: '"
                    << alg
                    << '\''
                    );
            }

            // 9. if Scan Protected Header gave 'kid'
            if ( kid . isEmpty () )
            {
                // we can throw an exception now if 'kid' was not found
                if ( dflt . require_prestored_kid )
                {
                    throw JWSMissingKeyIdInHeader (
                        XP ( XLOC )
                        << "JOSE header missing 'kid' required by local policy"
                        );
                }
            }
            else
            {
                // 9a. query key-set for 'kid'
                // 9b. REJECT if key not found and policy requires it to be present
                // 9c. SKIP if key not found and policy does not require it to be present
                if ( dflt . require_prestored_kid || keys . contains ( kid ) )
                {
                    try
                    {
                        key = keys . getKey ( kid );
                    }
                    catch ( ... )
                    {
                        throw JWSKeyNotFound (
                            XP ( XLOC )
                            << "key with id '"
                            << kid
                            << "' not found in key set"
                            );
                    }

                    // 9d. REQUIRE found key 'alg' to match header 'alg'
                    if ( alg != key -> getAlg () )
                    {
                        throw JWSIncompatibleKey (
                            XP ( XLOC )
                            << "algorithm differs for key with id '"
                            << kid
                            << "' : key alg '"
                            << key -> getAlg ()
                            << "' != token alg '"
                            << alg
                            << '\''
                            );
                    }

                    // 9e. REQUIRE found key to be for verification
                    // NB - this test will also be performed by verifier
                    if ( ! key -> forVerifying () )
                    {
                        throw JWSIncompatibleKey (
                            XP ( XLOC )
                            << "key with id '"
                            << kid
                            << "' is not usable for signature verification"
                            );
                    }

                    // 9f. REQUIRE signature match using JWA Signature Validation
                    if ( ! verifier -> verify ( * key, content . data (), content . size (), signature ) )
                    {
                        // 9g. detect JWS truncation
                        if ( verifier -> expectedSignatureSize () != signature . size () )
                            throw JWSSignatureTruncated ( XP ( XLOC ) << "signature truncated" );

                        throw JWSSignatureMismatch ( XP ( XLOC ) << "signature mismatch" );
                    }
                }
            }
        }

        // 10. for each key in key-set if step #9 was not successful
        // this is detected by checking for a JWK
        if ( key == nullptr )
        {
            std :: vector < String > kids = keys . getKeyIDs ();
            size_t i, count = kids . size ();
            for ( i = 0; i < count; ++ i )
            {
                // retrieve JWK
                key = keys . getKey ( kids [ i ] );

                // limit to verification keys
                if ( key -> forVerifying () )
                {
                    // 10a. filter by 'alg' if known
                    if ( ! has_alg || alg == key -> getAlg () )
                    {
                        // 10b. locate JWA verifier for key if not known
                        // meaning this is a brute force scan
                        if ( ! has_alg )
                            verifier = JWAMgr :: getVerifier ( key -> getAlg () );

                        // 10c. attempt JWA Signature Verification with key
                        if ( verifier -> verify ( * key, content . data (), content . size (), signature ) )
                        {
                            // 10d. SKIP remaining keys if signature matches
                            break;
                        }
                    }
                }

                // forget the key
                key = nullptr;
            }
        }

        // 11. REJECT if no key was found
        if ( key == nullptr )
        {
            // 11a. logging here is very important
            // TBD

            throw JWSNoKeyAvailable (
                XP ( XLOC )
                << "no key that verifies signature"
                );

        }

        /********************************************
         *              AT THIS POINT               *
         * THE JWS IS AS TRUSTED AS IT WILL EVER BE *
         ********************************************/

        // 12. parse JOSE header from JSON string
        jose = JSON :: parseObject ( hdr_json );

        // 13. Validate JOSE Header
        verifyHeader ( * jose );

        // 14. optionally decode base64url on payload
        // this is based upon the #$%^U 'b64' header
        String pay_text = content . subString ( dots [ 0 ] + 1 );
        if ( jose -> exists ( "b64" ) && jose -> getValue ( "b64" ) . toBoolean () )
        {
            payload = Payload ( pay_text . size () );
            memmove ( payload . data (), pay_text . data (), pay_text . size () );
            payload . setSize ( pay_text . size () );
        }
        else
        {
            // either there is no 'b64' or there is but it says 'false'
            payload = Base64 :: urlDecode ( pay_text, Base64 :: strict_charset );
        }

        // 15. ACCEPT JWS
        return key;
    }

    const JWSMgr :: Policy & JWSMgr :: getPolicy () noexcept
    {
        return dflt;
    }

    void JWSMgr :: setPolicy ( const Policy & p )
    {
        // the assignment operator takes care of invariants
        dflt = p;
    }

    bool JWSMgr :: hdrReserved ( const String & name ) noexcept
    {
        return reserved . headerExists ( name );
    }

    count_t JWSMgr :: locateDelimiters ( OUT count_t dots [ 5 ], const String & encoded )
    {
        assert ( encoded . isAscii () );
        const ASCII * ptr = encoded . data ();
        size_t sz = encoded . size ();
        count_t num_delimiters = 0;

        for ( size_t i = 0; i < sz; ++ i )
        {
            if ( ptr [ i ] == '.' )
            {
                dots [ num_delimiters ] = ( count_t ) i;
                if ( ++ num_delimiters == 5 )
                    break;
            }
        }

        return num_delimiters;
    }

    bool JWSMgr :: scanProtectedHeader ( String & alg, String & kid, const String & jose )
    {
        // it's "legal" for a JOSE header to have non-ASCII,
        // but we don't expect it and will skip it if so
        if ( ! jose . isAscii () )
            return false;

        bool seen_alg, seen_kid;
        seen_alg = seen_kid = false;

        // count curly and square bracket nesting
        long int curly, square;
        curly = square = 0;

        // raw pointer to the data so it happens in front of our eyes
        const ASCII * hdr = jose . data ();
        size_t i, sz = jose . size ();

        // endpoints to tokens
        size_t tstart, tend;
        size_t nstart, nend;
        tstart = tend = nstart = nend = 0;

        // tell us if previous token was string
        bool tok_is_string = false;

        // tell us if previous token was name
        bool prev_tok_is_name = false;

        for ( i = 0; i < sz; ++ i )
        {
            // 1. IGNORE white space while scanning (not within tokens)
            if ( isspace ( hdr [ i ] ) )
                continue;

            switch ( curly )
            {
            case 0:
                switch ( hdr [ i ] )
                {
                // 2. increment brace counter on each'{', decrement on '}'
                case '{':
                    curly = 1;
                    break;
                default:
                    // 3. SKIP scan unless leading '{' (nothing before 0->1 transition)
                    return false;
                }
                break;

            case 1:
                switch ( hdr [ i ] )
                {
                // 2. increment brace counter on each'{', decrement on '}'
                case '}':
                    // should be end of header
                    curly = -1;
                    break;
                case '[':
                    // beginning of an array
                    if ( ++ square != 1 )
                        return false;
                    tok_is_string = false;
                    break;
                case ']':
                    // end of an array
                    if ( -- square != 0 )
                        return false;
                    tok_is_string = false;
                    break;
                case ',':
                    // separator between members
                    tok_is_string = prev_tok_is_name = false;
                    break;
                case ':':
                    // separator between (n,v)
                    prev_tok_is_name = tok_is_string;
                    if ( prev_tok_is_name )
                    {
                        nstart = tstart;
                        nend = tend;
                    }
                    tok_is_string = false;
                    break;
                case '+':
                case '-':
                case '.':
                    // apparently numeric
                    tok_is_string = false;
                    break;
                case '"':
                    tstart = i + 1;
                    for ( ++ i; i < sz; ++ i )
                    {
                        if ( hdr [ i ] == '"' )
                            break;
                    }
                    tend = i;
                    tok_is_string = tstart < tend;
                    if ( tok_is_string && prev_tok_is_name )
                    {
                        if ( nend - nstart == 3 )
                        {
                            // 5. identify "alg": followed by a string of up to 16 characters
                            if ( memcmp ( & hdr [ nstart ], "alg", 3 ) == 0 )
                            {
                                // 6. SKIP scan if "alg": not observed exactly once
                                if ( seen_alg )
                                    return false;

                                if ( tend - tstart > max_jose_alg_length )
                                    return false;

                                alg = jose . subString ( tstart, tend - tstart );
                                seen_alg = true;
                            }

                            // 7. identify "kid": followed by a string of up to 256 characters
                            else if ( memcmp ( & hdr [ nstart ], "kid", 3 ) == 0 )
                            {
                                // 8. SKIP scan if "kid": observed more than once
                                if ( seen_kid )
                                    return false;

                                if ( tend - tstart > max_jose_kid_length )
                                    return false;

                                kid = jose . subString ( tstart, tend - tstart );
                                seen_kid = true;
                            }
                        }
                    }
                    break;
                default:
                    if ( ! isalnum ( hdr [ i ] ) )
                    {
                        // nothing else is acceptable
                        return false;
                    }
                }
                break;

            default:
                // 4. SKIP scan upon seeing brace count of 2 or -1
                // 10. SKIP scan if any other character observed after '}'
                return false;
            }
        }

        // 9. SKIP scan unless trailing '}' (nothing after 1->0 transition of counter)
        if ( curly >= 0 )
            return false;

        return seen_alg;
    }

    void JWSMgr :: verifyHeader ( const JSONObject & jose )
    {
        try
        {
            verifyHeader2 ( jose );
        }
        catch ( JWSInvalidHeader & x )
        {
            throw;
        }
        catch ( Exception & x )
        {
            throw JWSInvalidHeader ( x );
        }
        catch ( ... )
        {
            throw JWSInvalidHeader (
                XP ( XLOC )
                << "unknown error"
                );
        }
    }

    void JWSMgr :: verifyHeader2 ( const JSONObject & jose )
    {
        // RFC7515 Section 4.1.1 - "alg" REQUIRED string
        String alg = jose . getValue ( "alg" ) . toString ();
        if ( ! JWAMgr :: acceptJWSAlgorithm ( alg ) )
        {
            throw JWSInvalidHeader (
                XP ( XLOC )
                << "JWS JOSE algorithm '"
                << alg
                << "' not accepted"
                );
        }

        // RFC7515 Section 4.1.4 - "kid" OPTIONAL string
        String kid;
        if ( dflt . kid_required || jose . exists ( "kid" ) )
        {
            kid = jose . getValue ( "kid" ) . toString ();
            if ( kid . isEmpty () )
            {
                throw JWSInvalidHeader (
                    XP ( XLOC )
                    << "JWS JOSE 'kid' member is empty"
                    );
            }
        }

        // RFC7515 Section 4.1.9 - "typ" OPTIONAL string
        if ( jose . exists ( "typ" ) )
        {
            String typ = jose . getValue ( "typ" ) . toString ();
            if ( typ . isEmpty () )
            {
                throw JWSInvalidHeader (
                    XP ( XLOC )
                    << "JWS JOSE 'typ' member is empty"
                    );
            }
        }

        // RFC7515 Section 4.1.10 - "cty" OPTIONAL string
        if ( jose . exists ( "cty" ) )
        {
            String cty = jose . getValue ( "cty" ) . toString ();
            if ( cty . isEmpty () )
            {
                throw JWSInvalidHeader (
                    XP ( XLOC )
                    << "JWS JOSE 'cty' member is empty"
                    );
            }
        }

        // RFC7515 Section 4.1.11 - "crit" OPTIONAL array
        if ( jose . exists ( "crit" ) )
        {
            // "Recipients MAY consider the JWS to be invalid if the
            //  critical list contains any Header Parameter names defined
            //  by this specification or [JWA] for use with JWS or if any
            //  other constraints on its use are violated."

            std :: set < String > dups;
            const JSONArray & crit = jose . getValue ( "crit" ) . toArray ();

            // "Producers MUST NOT use the empty list '[]'
            //  as the 'crit' value."
            count_t count = crit . count ();
            if ( count == 0 )
            {
                throw JWSInvalidHeader (
                    XP ( XLOC )
                    << "JWS JOSE 'crit' member is empty"
                    );
            }

            for ( count_t i = 0; i < count; ++ i )
            {
                // read the name of the critical header
                String name = crit [ i ] . toString ();

                // "If any of the listed extension Header Parameters are
                //  not understood and supported by the recipient, then
                //  the JWS is invalid."
                if ( ! reserved . headerUnderstood ( name ) )
                {
                    throw JWSInvalidHeader (
                        XP ( XLOC )
                        << "JWS JOSE 'crit' member '"
                        << name
                        << "' is not understood"
                        );
                }

                // "Producers MUST NOT include Header Parameter names defined
                //  by [RFC7515] or [JWA] for use with JWS, duplicate
                //  names, or names that do not occur as Header Parameter names
                //  within the JOSE Header in the 'crit' list."
                if ( reserved . headerReserved ( name ) )
                {
                    throw JWSInvalidHeader (
                        XP ( XLOC )
                        << "JWS JOSE 'crit' member '"
                        << name
                        << "' is a reserved header name"
                        );
                }

                if ( dups . find ( name ) != dups . end () )
                {
                    throw JWSInvalidHeader (
                        XP ( XLOC )
                        << "JWS JOSE 'crit' member '"
                        << name
                        << "' appears multiple times"
                        );
                }
                dups . emplace ( name );

                if ( ! jose . exists ( name ) )
                {
                    throw JWSInvalidHeader (
                        XP ( XLOC )
                        << "JWS JOSE 'crit' member '"
                        << name
                        << "' does not exist"
                        );
                }
            }
        }

        // RFC7797 Section 3 - "b64" OPTIONAL string
        if ( jose . exists ( "b64" ) )
        {
            if ( ! jose . getValue ( "b64" ) . isBoolean () )
            {
                throw JWSInvalidHeader (
                    XP ( XLOC )
                    << "JWS JOSE 'b64' member is not Boolean"
                    );
            }
        }

        // RFC7515 Section 4.1.2 - "jku" OPTIONAL URL
        // RFC7515 Section 4.1.3 - "jwk" OPTIONAL JWK
        // RFC7515 Section 4.1.5 - "x5u" OPTIONAL X.509 URL
        // RFC7515 Section 4.1.6 - "x5c" OPTIONAL X.509 certificate chain
        // RFC7515 Section 4.1.7 - "x5t" OPTIONAL X.509 certificate SHA-1 thumbprint
        // RFC7515 Section 4.1.8 - "x5t#256" OPTIONAL X.509 certificate SHA-256 thumbprint
        // all of these are considered... sorry.
    }

    JWSMgr :: Policy :: Policy ()
        : kid_required ( dflt_kid_required )
        , kid_gen ( dflt_kid_gen )
        , require_simple_hdr ( dflt_require_simple_hdr )
        , require_prestored_kid ( dflt_require_prestored_kid )
        , require_prestored_key ( dflt_require_prestored_key )
    {
    }

    JWSMgr :: Policy & JWSMgr :: Policy :: operator = ( const Policy & p )
    {
        kid_required = p . kid_required;

        kid_gen = p . kid_required | p . kid_gen;

        require_simple_hdr = p . require_simple_hdr;
        require_prestored_kid = p . require_prestored_kid;
        require_prestored_key = p . require_prestored_key;

        return * this;
    }

    JWSMgr :: Policy :: Policy ( const Policy & p )
        : kid_required ( p . kid_required )
        , kid_gen ( p . kid_required | p . kid_gen )
        , require_simple_hdr ( p . require_simple_hdr )
        , require_prestored_kid ( p . require_prestored_kid )
        , require_prestored_key ( p . require_prestored_key )
    {
    }

    JWSMgr :: Policy :: ~ Policy ()
    {
    }

    bool JWSMgr :: Reserved :: headerExists ( const String & name ) const noexcept
    {
        return headerReserved ( name ) || headerUnderstood ( name );
    }

    bool JWSMgr :: Reserved :: headerReserved ( const String & name ) const noexcept
    {
        return hdrs . find ( name ) != hdrs . end ();
    }

    bool JWSMgr :: Reserved :: headerUnderstood ( const String & name ) const noexcept
    {
        return understood . find ( name ) != understood . end ();
    }

    JWSMgr :: Reserved :: Reserved ()
    {
        const char * used_names [] =
        {
            "alg", "kid", "typ", "cty", "crit"
        };

        const char * unused_names [] =
        {
            "jku", "jwk", "x5u", "x5c", "x5t", "x5t#256"
        };

        const char * ext_names [] =
        {
            "b64"
        };

        for ( size_t i = 0; i < sizeof used_names / sizeof used_names [ 0 ]; ++ i )
        {
            hdrs . emplace ( String ( used_names [ i ] ) );
            understood . emplace ( String ( used_names [ i ] ) );
        }

        for ( size_t i = 0; i < sizeof unused_names / sizeof unused_names [ 0 ]; ++ i )
        {
            hdrs . emplace ( String ( unused_names [ i ] ) );
        }

        for ( size_t i = 0; i < sizeof ext_names / sizeof ext_names [ 0 ]; ++ i )
        {
            understood . emplace ( String ( ext_names [ i ] ) );
        }
    }

    JWSMgr :: Reserved :: ~ Reserved ()
    {
    }
}
