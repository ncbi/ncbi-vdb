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

TEST_SUITE( KDBNodeDropTestSuite )

const string DName = "NodeDrop";

FIXTURE_TEST_CASE (NodeDropTest, WVDB_Fixture)
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

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "A/B/C"));
    REQUIRE_RC(KMDataNodeWriteCString(Node, "1"));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "D/E/F"));
    REQUIRE_RC(KMDataNodeWriteCString(Node, "2"));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, "GG/H/I"));
    REQUIRE_RC(KMDataNodeWriteCString(Node, "3"));
    REQUIRE_RC(KMDataNodeRelease(Node));

    // delete top level node
    const KMDataNode* rNode;
    REQUIRE_RC(KMetadataOpenNodeRead(Md, &rNode, "A"));
    REQUIRE_RC(KMDataNodeRelease(rNode));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, NULL));
    REQUIRE_RC(KMDataNodeDropChild(Node,"A"));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_EQ(GetRCState(KMetadataOpenNodeRead(Md, &rNode, "A")), rcNotFound);

    // delete lower level node
    REQUIRE_RC(KMetadataOpenNodeRead(Md, &rNode, "D/E"));
    REQUIRE_RC(KMDataNodeRelease(rNode));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, NULL));
    REQUIRE_RC(KMDataNodeDropChild(Node, "D/E"));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_EQ(GetRCState(KMetadataOpenNodeRead(Md, &rNode, "D/E")),
        rcNotFound);
    REQUIRE_RC(KMetadataOpenNodeRead(Md, &rNode, "D"));
    REQUIRE_RC(KMDataNodeRelease(rNode));

    // ignore slashes
    REQUIRE_RC(KMetadataOpenNodeRead(Md, &rNode, "GG"));
    REQUIRE_RC(KMDataNodeRelease(rNode));

    REQUIRE_RC(KMetadataOpenNodeUpdate(Md, &Node, NULL));
    REQUIRE_RC(KMDataNodeDropChild(Node, "//GG//"));
    REQUIRE_RC(KMDataNodeRelease(Node));

    REQUIRE_EQ(GetRCState(KMetadataOpenNodeRead(Md, &rNode, "GG")), rcNotFound);

    REQUIRE_RC ( KMetadataRelease ( Md ) );
    REQUIRE_RC ( KDatabaseRelease ( Db ) );
    REQUIRE_RC ( KDBManagerRelease ( Mgr ) );
    REQUIRE_RC ( KDirectoryRelease ( Dir ) );

    RemoveDatabase();
}

//////////////////////////////////////////// Main
int main ( int argc, char *argv [] )
{
    return KDBNodeDropTestSuite(argc, argv);
}
