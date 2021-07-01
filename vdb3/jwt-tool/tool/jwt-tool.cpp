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

#include "jwt-tool.hpp"

#include "cmdline.hpp"
#include "logging.hpp"

namespace ncbi
{
    static LocalLogger local_logger;
    Log log ( local_logger );

    JWTTool :: JWTTool ( const ParamBlock & params )
        : keySetFilePaths ( params . keySetFilePaths )
        , privKeyFilePath ( params . privKeyFilePath )
        , inputParams ( params . inputParams )
    {
    }
    
    JWTTool :: ~ JWTTool () noexcept
    {
    }

    void JWTTool :: run ()
    {
        // initialize and let any exceptions pass out
        init ();

        // we've been initilized
        try
        {
            exec ();
        }
        catch ( ... )
        {
            cleanup ();
            throw;
        }

        cleanup ();
    }

    static
    void handle_params ( ParamBlock & params, int argc, char * argv [] )
    {
        Cmdline cmdline ( argc, argv );

        // to the cmdline parser, all params are optional
        // we will enforce their presence manually
        cmdline . startOptionalParams ();

        // gather all params into a single vector
        cmdline . addParam ( params . inputParams, 0, 256, "token(s)", "optional list of tokens to process" );

        cmdline . addListOption ( params . keySetFilePaths, ',', 256,
            "K", "key-sets", "path-to-JWKS", "provide one or more sets of public JWKs" );

        // pre-parse to look for any configuration file path
        cmdline . parse ( true );

        // configure params from file

        // normal parse
        cmdline . parse ();

        params . validate ();
    }

    static
    int run ( int argc, char * argv [] )
    {
        try
        {
            // enable local logging to stderr
            local_logger . init ( argv [ 0 ] );

            // create params
            ParamBlock params;
            handle_params ( params, argc, argv );

            // run the task object
            JWTTool jwt_tool ( params );
            jwt_tool . run ();

            log . msg ( LOG_INFO )
                << "exiting. "
                << endm
                ;
        }
        catch ( Exception & x )
        {
            log . msg ( LOG_ERR )
                << "EXIT: exception - "
                << x . what ()
                << endm
                ;
            return x . status ();
        }
        catch ( std :: exception & x )
        {
            log . msg ( LOG_ERR )
                << "EXIT: exception - "
                << x . what ()
                << endm
                ;
            throw;
        }
        catch ( ReturnCodes x )
        {
            log . msg ( LOG_NOTICE )
                << "EXIT: due to exception"
                << endm
                ;
            return x;
        }
        catch ( ... )
        {
            log . msg ( LOG_ERR )
                << "EXIT: unknown exception"
                << endm
                ;
            throw;
        }
        
        return 0;
    }
}

extern "C"
{
    int main ( int argc, char * argv [] )
    {
        int status = 1;

        /* STATE:
             fd 0 is probably open for read
             fd 1 is probably open for write
             fd 2 is hopefully open for write
             launched as child process of some shell,
             or perhaps directly as a child of init
        */

        try
        {
            // run the tool within namespace
            status = ncbi :: run ( argc, argv );
        }
#if 0
        catch ( ReturnCodes x )
        {
            status = x;
        }
#endif
        catch ( ... )
        {
        }
        
        return status;
    }
}
