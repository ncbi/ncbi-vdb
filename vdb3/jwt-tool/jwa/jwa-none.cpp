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

#include <ncbi/jwa.hpp>
#include "jwa-registry.hpp"

namespace ncbi
{

    struct NONE_Signer : JWASigner
    {
        virtual Payload sign ( const JWK & key,
            const void * data, size_t bytes ) const override
        {
            return Payload ();
        }

        NONE_Signer ()
        {
        }
    };

    struct NONE_Verifier : JWAVerifier
    {
        virtual bool verify ( const JWK & key, const void * data, size_t bytes,
            const Payload & binary_signature ) const override
        {
            return true;
        }

        virtual size_t expectedSignatureSize () const override
        {
            return 0;
        }

        NONE_Verifier ()
        {
        }
    };


    static struct NONE_Registry
    {
        NONE_Registry ()
        {
            String alg ( "none" );
            gJWARegistry . registerSigner ( alg, new NONE_Signer () );
            gJWARegistry . registerVerifier ( alg, new NONE_Verifier () );
        }

        void avoidDeadStrip ()
        {
            gJWARegistry . doNothing ();
        }
        
    } none_registry;

    void includeJWA_none ( bool always_false )
    {
        if ( always_false )
            none_registry . avoidDeadStrip ();
    }
}
