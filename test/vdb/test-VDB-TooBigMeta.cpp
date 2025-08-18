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

#include <kdb/manager.h>
#include <kdb/meta.h>
#include <kdb/database.h>
#include <kdb/table.h>

#include "WVDB_Fixture.hpp"

#include <sysalloc.h>

#include <sstream>
#include <cstdlib>

using namespace std;

TEST_SUITE(TooBigMetaTestSuite)

const string DName = "TooBigMeta";

FIXTURE_TEST_CASE (TooBigMetaTest, WVDB_Fixture)
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

    size_t size(8193);
    const void* buffer(malloc(size));
    REQUIRE(buffer);

    // Created metadata will overflow buffer in WMetadataFlush.
    // It will be flushed to disk
    // and check that we will be able to load Metadata will not be performed.
    KMDataNode * Node;
    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "A"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "B"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "C"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "D"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "E"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "F"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "G"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "H"));
    REQUIRE_RC(KMDataNodeWrite(Node, buffer, size));
    REQUIRE_RC(KMDataNodeRelease(Node));

    free(buffer);

    REQUIRE_RC ( KMetadataRelease ( Md ) );
    REQUIRE_RC ( KDatabaseRelease ( Db ) );
    REQUIRE_RC ( KDBManagerRelease ( Mgr ) );
    REQUIRE_RC ( KDirectoryRelease ( Dir ) );

    RemoveDatabase();
}

//////////////////////////////////////////// Main
int main ( int argc, char *argv [] )
{
    return TooBigMetaTestSuite(argc, argv);
}
