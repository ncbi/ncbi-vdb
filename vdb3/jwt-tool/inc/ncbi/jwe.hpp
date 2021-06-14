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

#pragma once

#include <ncbi/secure/string.hpp>
#include <ncbi/jwk.hpp>

/**
 * @file ncbi/jwe.hpp
 * @brief JSON Web Encryption - RFC 7516
 */

namespace ncbi
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class Payload;


    /*=====================================================*
     *                      TYPEDEFS                       *
     *=====================================================*/

    /**
     * @typedef JWE
     * @brief JSON Web Encryption - RFC 7516, section 2
     *
     * "A data structure representing an encrypted and
     *  integrity-protected message."
     */
    typedef String JWE;


    /*=====================================================*
     *                       JWEMgr                        *
     *=====================================================*/
    
    /**
     * @class JWEMgr
     * @brief JWE Management
     *
     * Globally accessible factory functions for encrypting
     * and extracting ( decrypting ) payloads.
     */
    class JWEMgr
    {
    public:

    private:

        /**
         * decryptCompact
         * @overload decrypt a JWE and extracts its payload
         * @param jose a return parameter for full JOSE header
         * @param payload a return parameter for JWE payload
         * @param keys a set of decryption/validation keys
         * @param jwe the compact JWE to be verified
         * @param dots location of the dots within "jwe"
         * @return a JWKRef with the key that identifies source
         */
        static JWKRef decryptCompact ( OUT JSONObjectRef & jose, OUT Payload & payload,
            const JWKSet & keys, const JWE & jwe, const count_t dots [ 5 ] );

        JWEMgr ();

        friend class JWTMgr;
    };


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/
}
