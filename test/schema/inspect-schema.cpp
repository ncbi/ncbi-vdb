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
// #include <vdb/manager.h>
// #include <vfs/manager.h>
// #include <vfs/path.h>

#include <iostream>
// #include <fstream>
// #include <sstream>
#include <stdexcept>
// #include <algorithm>

using namespace std;
//using namespace ncbi::SchemaParser;

rc_t CC UsageSummary (const char * progname) {
    return KOutMsg (
        "Summary:\n"
        "  Inspect schema AST\n"
        "\n""Usage:\n"
        "  %s [options] \n",
        progname);
 }

const char UsageDefaultName[] = "vdb-inspect-schema";

rc_t CC Usage(const Args* args) {
    rc_t rc = 0 ;

    const char* progname = UsageDefaultName;
    const char* fullpath = UsageDefaultName;

    if (args == NULL)
    {    rc = RC(rcExe, rcArgv, rcAccessing, rcSelf, rcNull); }
    else
    {    rc = ArgsProgram(args, &fullpath, &progname); }

    UsageSummary(progname);

    KOutMsg ("\nOptions:\n");

    // HelpOptionLine (ALIAS_ALL, OPTION_ALL, NULL, USAGE_ALL);
    // HelpOptionLine (ALIAS_REF, OPTION_REF, NULL, USAGE_REF);
    // HelpOptionLine (ALIAS_BAM, OPTION_BAM, NULL, USAGE_BAM);
    // HelpOptionLine (ALIAS_QUA, OPTION_QUA, NULL, USAGE_QUA);
    // HelpOptionLine (ALIAS_HEA, OPTION_HEA, NULL, USAGE_HEA);
    // HelpOptionLine (ALIAS_NGC, OPTION_NGC, "path",USAGE_NGC);

    KOutMsg ("\n");

    HelpOptionsStandard ();

    KOutMsg("\n");

    HelpVersion (fullpath, GetKAppVersion());

    return rc;
}

void
run( int argc, char *argv [] )
{
    // int failed = 0;
    // if ( argc < 2 )
    // {
    //     cout << "Usage:\n\t" << argv[0] << " [-Ipath ... ] schema-file " << endl;
    //     return 1;
    // }
    // struct KDirectory * wd;
    // if ( KDirectoryNativeDir ( & wd ) != 0 )
    // {
    //     throw runtime_error ( "KDirectoryNativeDir failed" );
    // }

    // struct VDBManager const * vdb;
    // if ( VDBManagerMakeRead ( & vdb, wd ) != 0 )
    // {
    //     throw runtime_error ( "VDBManagerMakeRead failed" );
    // }

    // for ( int i = 0 ; i < argc - 1; ++i )
    // {
    //     const char * arg = argv [ i + 1 ];
    //     if ( arg [ 0 ] == '-' )
    //     {
    //         switch ( arg [ 1 ] )
    //         {
    //             case 'I':
    //                 if ( VDBManagerAddSchemaIncludePath ( vdb, arg + 2 ) )
    //                 {
    //                     throw runtime_error ( string ( "VDBManagerAddSchemaIncludePath(" ) + ( arg + 2 ) + ") failed" );
    //                 }
    //                 break;
    //             default:
    //                 cout << "Unknown option " << arg << endl;
    //                 break;
    //         }
    //         continue;
    //     }
    // }

    // VDBManagerRelease ( vdb );
    // KDirectoryRelease ( wd );

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

