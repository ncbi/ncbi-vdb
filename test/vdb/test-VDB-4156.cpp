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


#include <klib/log.h>
#include <klib/symbol.h>

#include <vdb/vdb-priv.h>

#include <../libs/vdb/schema-priv.h>
#include <../libs/vdb/schema-parse.h>
#include <../libs/vdb/dbmgr-priv.h>
#include <../libs/vdb/linker-priv.h>

#include <kdb/manager.h>
#include <kdb/meta.h>
#include <kdb/database.h>
#include <kdb/table.h>

#include "WVDB_Fixture.hpp"

#include <sysalloc.h>

#include <sstream>
#include <cstdlib>

using namespace std;

TEST_SUITE( KDBVDBTestSuite )

const string DName = "VDB-4156";
const string ScratchDir = "./db/";

// this test case is not very useful but is here as a blueprint for other write-side tests
FIXTURE_TEST_CASE ( DumpToKMDataNode, WVDB_Fixture)
{
    m_databaseName = ScratchDir + DName;
    RemoveDatabase();

    KDirectory * Dir;
    REQUIRE_RC ( KDirectoryNativeDir ( & Dir ) );

    KDBManager * Mgr;
    REQUIRE_RC ( KDBManagerMakeUpdate ( & Mgr, Dir ) );

    KDatabase * Db;
    REQUIRE_RC ( KDBManagerCreateDB ( Mgr, & Db, kcmInit, "%s", m_databaseName . c_str () ) );

    KMetadata * Md;
    REQUIRE_RC ( KDatabaseOpenMetadataUpdate ( Db, & Md ) );

    KMDataNode * Node;
    REQUIRE_RC ( KMetadataOpenNodeUpdate ( Md, & Node, "schema" ) );

    VDBManager * VMgr;
    REQUIRE_RC ( VDBManagerMakeUpdate ( & VMgr, Dir ) );

    VSchema * Schema;
    REQUIRE_RC ( VDBManagerMakeSchema ( VMgr, & Schema ) );

        /* Should not fall with SIGSEGV */
    ::VSchemaDumpToKMDataNode ( ( const VSchema * ) Schema, Node, "just_a_name" );

    REQUIRE_RC ( VSchemaRelease ( Schema ) );
    REQUIRE_RC ( VDBManagerRelease ( VMgr ) );
    REQUIRE_RC ( KMDataNodeRelease ( Node ) );
    REQUIRE_RC ( KMetadataRelease ( Md ) );
    REQUIRE_RC ( KDatabaseRelease ( Db ) );
    REQUIRE_RC ( KDBManagerRelease ( Mgr ) );
    REQUIRE_RC ( KDirectoryRelease ( Dir ) );

    RemoveDatabase();
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-wvdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=KDBVDBTestSuite(argc, argv);
    return rc;
}

}
