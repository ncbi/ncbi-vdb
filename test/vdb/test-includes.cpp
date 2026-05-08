// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#include <kfg/config.h>
#include <kfs/directory.h>
#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <vfs/path.h>
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>

#include <ktst/unit_test.hpp>

using namespace std;

TEST_SUITE( VdbTestIncludesSuite )

TEST_CASE( DirectInclude )
{
    const VDBManager *mgr = nullptr;
    REQUIRE_RC( VDBManagerMakeRead(&mgr, nullptr) );

    VSchema *schema = nullptr;
    REQUIRE_RC( VDBManagerMakeSchema( mgr, &schema ) );

    const string Text = "version 2; include 'kfg/includes/test.vschema';";
    REQUIRE_RC( VSchemaParseText ( schema, "test", Text.data(), Text.size() ) );

    REQUIRE_RC( VSchemaRelease(schema) );

    REQUIRE_RC( VDBManagerRelease(mgr) );
}

TEST_CASE( IncludeViaKonfig )
{
    const VDBManager *mgr = nullptr;
    REQUIRE_RC( VDBManagerMakeRead(&mgr, nullptr) );

    VSchema *schema = nullptr;
    REQUIRE_RC( VDBManagerMakeSchema( mgr, &schema ) );

    # kfg/includes comes from the konfig
    const string Text = "version 2; include 'test.vschema';";
    REQUIRE_RC( VSchemaParseText ( schema, "test", Text.data(), Text.size() ) );

    REQUIRE_RC( VSchemaRelease(schema) );

    REQUIRE_RC( VDBManagerRelease(mgr) );
}

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    setenv( "VDB_CONFIG", "kfg/includes", 1);
    return VdbTestIncludesSuite(argc, argv);
}
