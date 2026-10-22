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
* schema AST inspection tool
*/

//////////////////////////////////////////// Main
//#include <kfc/except.h>
#include <kapp/vdbapp.h>
// #include <kfg/config.h>
 #include <klib/out.h>
// #include <kfs/directory.h>
// #include <vdb/schema.h>
#include <vdb/manager.h>
// #include <vfs/manager.h>
// #include <vfs/path.h>

#include "AST_Fixture.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
// #include <algorithm>

using namespace std;
//using namespace ncbi::SchemaParser;

rc_t CC UsageSummary (const char * progname)
{
    cout << "Usage:" << endl
         << "  " << progname << " [Options] schemafile query ..." << endl
        << "Summary:" << endl
        << "  Performs static analysis of a schema" << endl
        << endl
        << "Example:" << endl
        << "  " << progname << " --database NCBI:align:db:alignment_sorted --table REFERENCE align/seq.vschema coltofn" << endl
        << endl;
    return 0;
 }

const char UsageDefaultName[] = "vdb-inspect-schema";

rc_t CC Usage(const Args* args)
{
    UNUSED( args );
    UsageSummary ( UsageDefaultName );

    cout
        << "Query:" << endl
        << "  coltofn     " << endl
        << endl;

    cout
        << "Options:" << endl
        << "  -I|--include <paths>     " << endl
        << "  -d|--database <name>     " << endl
        << "  -t|--table <name>        " << endl
        << "  -o|--output <path>       " << endl
        << "  --json                   " << endl
        ;

    cout << endl;

    HelpOptionsStandard();
    cout << endl;

    HelpVersion ( UsageDefaultName, GetKAppVersion () );

    return 0;
}

void
ColToFn( AST * ast )
{

}

void
run( int argc, char *argv [] )
{
    struct VDBManager const * vdb;
    if ( VDBManagerMakeRead ( & vdb, nullptr ) != 0 )
    {
        throw runtime_error ( "VDBManagerMakeRead failed" );
    }

    string input;
    string action;
    unsigned int args_seen = 0;
    int i = 1;
    while ( i < argc )
    {
        const char * arg = argv [ i ];
        if ( arg [ 0 ] == '-' )
        {   // an option
            switch ( arg [ 1 ] )
            {
                case 'I':
                    ++i;
                    if ( i >= argc )
                    {
                        throw runtime_error ( "Option -I requires an argument" );
                    }
                    if ( VDBManagerAddSchemaIncludePath ( vdb, argv [ i ]) )
                    {
                        throw runtime_error ( string ( "VDBManagerAddSchemaIncludePath(" ) + argv [ i ] + ") failed" );
                    }
                    break;
                default:
                    cout << "Unknown option " << arg << endl;
                    break;
            }
            ++i;
            continue;
        }
        else // an argument
        {
            switch ( args_seen )
            {
                case 0:
                {
                    input = arg;
                    ++ args_seen;
                    break;
                }
                case 1:
                {
                    action = arg;
                    ++ args_seen;
                    break;
                }
                default:
                {
                    throw runtime_error ( "Too many arguments" );
                }
            }
            ++i;
        }
    }

    if ( args_seen < 2 )
    {
        UsageSummary( argv[0] );
        throw runtime_error ( "Missing argument(s)" );
    }

    string input_str;
    std::ifstream file( input );
    if ( ! file.is_open() )
    {
        throw runtime_error ( string( "File not found: " ) + input );
    }
    else
    {
        std::ostringstream sstr;
        sstr << file.rdbuf();
        input_str = sstr.str();
    }

    if ( action != "coltofn" )
    {
        throw runtime_error ( string( "Unknown action: " ) + action );
    }

    AST_Fixture fixture;
    AST* ast = fixture.MakeAst( input_str.c_str() );

    ColToFn( ast );

    AST :: Destroy( ast );

    VDBManagerRelease ( vdb );
}

int main ( int argc, char *argv [] )
{
    VDB::Application app( argc, argv, "" );
    app.HandleStandardOptions( Usage, UsageSummary );

    try
    {
        run ( app.getArgC(), app.getArgV() );
    }
    catch ( exception& ex)
    {
        cerr << " Error: " << ex . what () << endl;
        return 2;
    }
    catch ( ... )
    {
        cerr << " Unknown exception" << endl;
        return 3;
    }
    return 0;
}

