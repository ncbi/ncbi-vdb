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
* VSchema diff test.
* Parses input files with 2 schema parsers (old/new), matches the dumps of resulting VSchema objects, reports discrepancies
*/

#include "../../libs/schema/SchemaParser.hpp"
#include "../../libs/schema/ParseTree.hpp"
#include "../../libs/vdb/schema-priv.h"

using namespace std;
using namespace ncbi::SchemaParser;

//////////////////////////////////////////// Main
#include <kfc/except.h>
#include <kapp/vdbapp.h>
#include <kfg/config.h>
#include <klib/out.h>
#include <kfs/directory.h>
#include <vdb/schema.h>
#include <vdb/manager.h>
#include <vfs/manager.h>
#include <vfs/path.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include "../libs/schema/ASTBuilder.hpp"

rc_t CC FlushSchema ( void *fd, const void * buffer, size_t size )
{
    ostream & out = * static_cast < ostream * > ( fd );
    out . write ( static_cast < const char * > ( buffer ), size );
    out . flush ();
    return 0;
}

void
DumpSchema ( const VSchema * p_schema, ostream & p_out )
{
    if ( VSchemaDump ( p_schema, sdmPrint, 0, FlushSchema, & p_out ) != 0 )
    {
        throw runtime_error ( "DumpSchema failed" );
    }
}

int main ( int argc, char *argv [] )
{
    VDB::Application app( argc, argv, "" );

    int failed = 0;
    if ( argc < 2 )
    {
        cout << "Usage:\n\t" << argv[0] << " [-Ipath ... ] [-p1|2] schema-file" << endl;
        return 1;
    }
    try
    {
        struct KDirectory * wd;
        if ( KDirectoryNativeDir ( & wd ) != 0 )
        {
            throw runtime_error ( "KDirectoryNativeDir failed" );
        }

        struct VDBManager const * vdb;
        if ( VDBManagerMakeRead ( & vdb, wd ) != 0 )
        {
            throw runtime_error ( "VDBManagerMakeRead failed" );
        }

        SchemaParserVersion parser_version = SchemaParser_default;

        for ( int i = 0 ; i < argc - 1; ++i )
        {
            const char * arg = argv [ i + 1 ];
            if ( arg [ 0 ] == '-' )
            {
                switch ( arg [ 1 ] )
                {
                    case 'I':
                        if ( VDBManagerAddSchemaIncludePath ( vdb, arg + 2 ) )
                        {
                            throw runtime_error ( string ( "VDBManagerAddSchemaIncludePath(" ) + ( arg + 2 ) + ") failed" );
                        }
                        break;
                    case 'p':
                        if ( string( arg + 2 ) == "1" )
                        {
                            parser_version = SchemaParser_v1;
                        }
                        else if ( string( arg + 2 ) == "2" )
                        {
                            parser_version = SchemaParser_v2;
                        }
                        else
                        {
                            throw runtime_error ( string ( "wrong parser version:" ) + ( arg + 2 ) );
                        }

                        break;
                    default:
                        cout << "Unknown option " << arg << endl;
                        break;
                }
                continue;
            }

            VFSManager * vfs;
            if ( VFSManagerMake ( & vfs ) != 0 )
            {
                throw runtime_error ( "VFSManagerMake failed" );
            }
            struct VPath * path;
            if ( VFSManagerMakeSysPath ( vfs, & path, arg ) != 0 )
            {
                throw runtime_error ( "VFSManagerMakeSysPath failed" );
            }

            {
                VSchema * schema;
                if ( VDBManagerMakeSchema ( vdb, & schema ) != 0 )
                {
                    throw runtime_error ( "VDBManagerMakeSchema failed" );
                }

                VSchemaSetParserVersion( schema, parser_version );

                if ( VSchemaParseFile ( schema, "%s", arg ) != 0 )
                {
                    throw runtime_error ( string(arg) + ": VSchemaParseFile (v1) failed" );
                }

                ostringstream out;
                DumpSchema ( schema, out );
                cout << out . str () << endl;

                VSchemaRelease ( schema );
            }

            VPathRelease ( path );
            VFSManagerRelease ( vfs );
        }

        VDBManagerRelease ( vdb );
        KDirectoryRelease ( wd );
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

