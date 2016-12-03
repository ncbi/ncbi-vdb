/*===========================================================================
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

/**
* Command line test for schema tokenizer.
* Scans the input file and outputs each token to stdout on a single line
*/

#include "../../libs/schema/SchemaScanner.hpp"
#include "../../libs/schema/ParseTree.hpp"

using namespace std;
using namespace ncbi::SchemaParser;

#include "../../libs/schema/schema-tokens.h"

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/out.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

const char UsageDefaultName[] = "test-schema-scan";

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ( "Usage:\n" "\t%s [options] schema-file\n\n", progname );
}

rc_t CC Usage( const Args* args )
{
    return 0;
}

bool
NextToken ( SchemaScanner & p_s)
{
    SchemaScanner :: TokenType t = p_s . Scan ();
    if ( t == END_SOURCE )
    {
        return false;
    }
    if ( t == UNRECOGNIZED )
    {
        throw runtime_error ( string ( "Unrecognized token " ) +
                        string ( p_s . LastTokenValue ()  . value, p_s . LastTokenValue ()  . value_len ) );
    }
    return true;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    if ( argc < 2 )
    {
        cout << "Usage:\n\t" << argv[0] << " schema-file" << endl;
        return 1;
    }
    try
    {
        for ( int i = 0 ; i < argc - 1; ++i )
        {
            stringstream buffer;
            ifstream in ( argv [ i + 1 ] );
            if ( ! in . good () )
            {
                throw runtime_error ( string ( "Invalid file " ) + argv [ i + 1 ] );
            }
            buffer << in.rdbuf();

            SchemaScanner s ( buffer . str () . c_str () );
            while ( NextToken ( s ) )
            {
                cout << s . LastTokenValue () . leading_ws
                     << string ( s . LastTokenValue ()  . value, s . LastTokenValue ()  . value_len )
                     << endl;
            }
        }
    }
    catch ( exception& ex)
    {
        cerr << " Exception: " << ex . what () << endl;
        return 2;
    }
    catch ( ... )
    {
        cerr << " Unknown exception" << endl;
        return 3;
    }
    return 0;
}

}

