/*==============================================================================
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
* ==============================================================================
* Tests of cache of names resolve results / SDL responses.
*/

#include <ktst/unit_test.hpp>

#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h> /* VDBManagerRelease */
#include <vdb/table.h> /* VTableRelease */
#include <vdb/vdb-priv.h> // VDBManagerMakeReadWithVFSManager

#include <vfs/manager.h> /* VFSManagerRelease */

#include "../../libs/vfs/manager-priv.h" // VFSManagerSdlCacheClear

#define ALL

TEST_SUITE(Test_cache_sdlSuite)

TEST_CASE(UseAfterClean) {
    VFSManager* vfs(nullptr);
    REQUIRE_RC(VFSManagerMake(&vfs));

    const VDBManager* vdb(nullptr);
    REQUIRE_RC(VDBManagerMakeReadWithVFSManager(&vdb, nullptr, vfs));

    const VDatabase* db(nullptr);
    REQUIRE_RC(VDBManagerOpenDBRead(vdb, &db, nullptr, "SRR619505")); // cSra

    const VTable* tb(nullptr);
    REQUIRE_RC(VDBManagerOpenTableRead(vdb, &tb, nullptr, "SRR053325"));

    // clear cache of SDL responses
    REQUIRE_RC(VFSManagerSdlCacheClear(vfs));

    // VDatabase remains open after cache clear
    KNamelist* names(nullptr);
    REQUIRE_RC(VDatabaseListTbl(db, &names));
    REQUIRE_RC(KNamelistRelease(names));

    // VTable remains open after cache clear
    REQUIRE_RC(VTableListReadableColumns(tb, &names));
    REQUIRE_RC(KNamelistRelease(names));

    REQUIRE_RC(VTableRelease(tb));

    REQUIRE_RC(VDatabaseRelease(db));

    REQUIRE_RC(VDBManagerRelease(vdb));

    REQUIRE_RC(VFSManagerRelease(vfs));
}

int main(int argc, char * argv[]) {
    return Test_cache_sdlSuite(argc, argv);
}
