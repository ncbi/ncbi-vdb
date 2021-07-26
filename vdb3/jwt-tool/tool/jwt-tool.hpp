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

#include <ncbi/secure/except.hpp>
#include <ncbi/secure/string.hpp>
#include <ncbi/jwk.hpp>
#include <ncbi/jwt.hpp>

#include "logging.hpp"

#include <vector>

namespace ncbi
{
    
    struct ParamBlock
    {
        void validate ()
            {
                // Params
                if ( inputParams . empty () )
                    throw InvalidArgument (
                        XP ( XLOC, rc_param_err )
                        << "Missing input parameters"
                        );
                
                
                // Options
                if ( keySetFilePaths . empty () )
                    throw InvalidArgument (
                        XP ( XLOC, rc_param_err )
                        << "Required public key set"
                        );
            }
        
        ParamBlock ()
            {
            }
        
        ~ ParamBlock ()
            {
            }
        
        std :: vector <String> keySetFilePaths;
        std :: vector <String> privKeyFilePath;
        std :: vector <String> inputParams;
    };
    
    class JWTTool
    {
    public:
        JWTTool ( const ParamBlock & params );
        ~ JWTTool () noexcept;
        
        
        void run ();

    private:
        void init ();
        void exec ();
        void cleanup () noexcept;

        void loadKeySet ( const String & path );

        void decodeJWT ( const JWT & jwt );
        
        std :: vector <String> keySetFilePaths;
        std :: vector <String> privKeyFilePath;
        std :: vector <String> inputParams;

        JWKSetRef pubKeys;
    };
}
