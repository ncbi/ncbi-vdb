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

// need this fake to make schema-tokens.h compile
typedef struct ErrorReport ErrorReport;

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

static
bool
MatchStrings ( const string& p_source, const string p_print )
{
    const size_t Context = 20;
    for ( size_t i = 0; i < p_source . length (); ++ i )
    {
        if ( i >= p_print . length () )
        {
            cout << "premature end of print after '" << p_source . substr ( i > Context ? i - Context : 0 ) << "', " << ( i - p_print . length () + 1 ) << " character(s) missing" << endl;
            return false;
        }
        if ( p_source [ i ] != p_print [ i ] )
        {
            cout << "mismatch at " << i << ", after " << p_source . substr ( i > Context ? i - Context : 0, Context ) << endl;
            cout << "source: '" << p_source . substr ( i, Context ) << "'" << endl;
            cout << "print : '" << p_print . substr ( i, Context ) << "'" << endl;
            return false;
        }
    }
    if ( p_print . length () > p_source . length () )
    {
        cout << "extra characters printed: " << p_print . substr ( p_source . length () ) << endl;
        return false;
    }
    return true;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    int failed = 0;
    if ( argc < 2 )
    {
        cout << "Usage:\n\t" << argv[0] << " schema-file" << endl;
        return 1;
    }
    try
    {
        cout << "Scanning " << argc - 1 << " schema-files" << endl;
        for ( int i = 0 ; i < argc - 1; ++i )
        {
            stringstream buffer;
            ifstream in ( argv [ i + 1 ] );
            if ( ! in . good () )
            {
                throw runtime_error ( string ( "Invalid file " ) + argv [ i + 1 ] );
            }
            buffer << in.rdbuf();

            stringstream out;
            SchemaScanner s ( buffer . str () . c_str () );
            while ( true )
            {
                Token t = s . NextToken ();
                out << t . GetLeadingWhitespace ();
                if ( t . GetType () == END_SOURCE )
                {
                    break;
                }
                if ( t . GetType () == UNRECOGNIZED )
                {
                    throw runtime_error ( string ( "Unrecognized token " ) + t . GetValue () );
                }
                out << t . GetValue ();
            }

            if ( ! MatchStrings ( buffer . str (), out . str () ) )
            {
                cout << string ( "Printout mismatch: " ) + argv [ i + 1 ] << endl;
                ++ failed;
            }
        }
        cout << "Failed: " << failed << endl;
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
    return failed == 0 ? 0 : 4;
}

}

