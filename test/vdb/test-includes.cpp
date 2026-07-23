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
#include <os-native.h> // setenv

using namespace std;

TEST_SUITE( VdbTestIncludesSuite )

class IncludeFixture
{
public:
    IncludeFixture()
    {
        THROW_ON_RC( VDBManagerMakeRead(&mgr, nullptr) );
        THROW_ON_RC( VDBManagerMakeSchema( mgr, &schema ) );
    }

    ~IncludeFixture()
    {
        VSchemaRelease(schema);
        VDBManagerRelease(mgr);
    }

    const VDBManager *mgr = nullptr;
    VSchema *schema = nullptr;
};

FIXTURE_TEST_CASE( DirectInclude, IncludeFixture )
{   // relative path to the working directory
    const string Text = "version 2; include 'kfg/includes/test.vschema';";
    REQUIRE_RC( VSchemaParseText ( schema, "test", Text.data(), Text.size() ) );

    VTypedecl resolved;
    REQUIRE_RC( VSchemaResolveTypedecl ( schema, &resolved, "type1" ) ); // defined in kfg/test.vschema
}

FIXTURE_TEST_CASE( IncludeViaKonfig, IncludeFixture )
{
    // kfg/includes comes from the konfig (see main())
    const string Text = "version 2; include 'test.vschema';";
    REQUIRE_RC( VSchemaParseText ( schema, "test", Text.data(), Text.size() ) );

    VTypedecl resolved;
    REQUIRE_RC( VSchemaResolveTypedecl ( schema, &resolved, "type1" ) ); // defined in kfg/test.vschema
}

FIXTURE_TEST_CASE( IncludeViaAPI, IncludeFixture )
{
    // override konfig's includes via an API call
    REQUIRE_RC( VSchemaAddIncludePath( schema, "kfg/includes/includeViaAPI" ) );

    const string Text = "version 2; include 'test.vschema';";
    REQUIRE_RC( VSchemaParseText ( schema, "test", Text.data(), Text.size() ) );

    VTypedecl resolved;
    REQUIRE_RC( VSchemaResolveTypedecl ( schema, &resolved, "type2" ) ); // defined in kfg/includes/includeViaAPI/test.vschema
}

TEST_CASE( IncludeViaEnv )
{
    const VDBManager *mgr = nullptr;
    VSchema *schema = nullptr;
    REQUIRE_RC( VDBManagerMakeRead(&mgr, nullptr) );

    // override everything via an environment variable
    setenv( "VDB_SCHEMA_INCLUDES", "kfg/includes/includeViaEnv", 1);
    REQUIRE_RC( VDBManagerMakeSchema( mgr, &schema ) );

    // overrride konfig's includes via an API call, should not override the env var
    REQUIRE_RC( VSchemaAddIncludePath( schema, "kfg/includes/includeViaAPI" ) );

    const string Text = "version 2; include 'test.vschema';";
    REQUIRE_RC( VSchemaParseText ( schema, "test", Text.data(), Text.size() ) );

    VTypedecl resolved;
    REQUIRE_RC( VSchemaResolveTypedecl ( schema, &resolved, "type3" ) ); // defined in kfg/includes/includesEnv/test.vschema

    REQUIRE_RC( VSchemaRelease(schema) );
    REQUIRE_RC( VDBManagerRelease(mgr) );
}

//TODO: ':'-separated list of directories

//////////////////////////////////////////// Main
int main( int argc, char *argv [] )
{
    setenv( "VDB_CONFIG", "kfg/includes", 1);
    return VdbTestIncludesSuite(argc, argv);
}
