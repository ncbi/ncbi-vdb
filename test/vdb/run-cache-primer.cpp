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

#include <kfg/kfg-priv.h> /* KConfigMakeLocal */

#include <ktst/unit_test.hpp>

#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h> /* VDBManagerRelease */
#include <vdb/table.h> /* VTableRelease */
#include <vdb/vdb-priv.h> // VDBManagerMakeReadWithVFSManager

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> // VFSManagerMakeFromKfg
#include <vfs/path.h> /* VFSManagerMakePath */

#include "../../libs/kfg/kfg-priv.h" // KConfigSetUserSettingsDisabled
#include "../../libs/vfs/manager-priv.h" // VFSManagerSdlCacheClear

#define ALL

TEST_SUITE(Test_cache_sdlSuite)

TEST_CASE(UseAfterClean) {
    char root[]("/repository/user/main/public/root");

    char path[4096]("");
    sprintf(path, "%s/NCBI", getenv("HOME"));

    bool useUserConfig = getenv("VDB6457_UseUserConfig") != nullptr;
    bool useDfltConfig = getenv("VDB6457_WriteConfigInApp") != nullptr;
    bool run = useUserConfig | useDfltConfig;

    if (!run) {
        std::cerr << "=======================================================\n"
            "'export VDB6457_UseUserConfig=' to cache in the path\n"
            "defined in the value of " << root <<
            "\nnode of ~/.ncbi/user-settings.mkfg\n\n"
            << "'export VDB6457_WriteConfigInApp=' to cache in\n"
            << path << " directory\n"
            "=======================================================\n";
        return;
    }

    if (useUserConfig)
        KConfigSetUserSettingsDisabled(false);

    KConfig* cfg = NULL;
    REQUIRE_RC(KConfigMakeLocal(&cfg, nullptr));

    REQUIRE_RC(KConfigWriteString(cfg,
        "repository/user/main/public/apps/sra/volumes/sraFlat", "sra"));
    if (!useUserConfig)
        REQUIRE_RC(KConfigWriteString(cfg, root, path));

    VFSManager* vfs(nullptr);
    REQUIRE_RC(VFSManagerMakeFromKfg(&vfs, cfg));

    const VDBManager* vdb(nullptr);
    REQUIRE_RC(VDBManagerMakeReadWithVFSManager(&vdb, nullptr, vfs));

    /***************************************************************************
    the following is a test of SetCacheRoot,
    you can update "path" to a string you want to test.
    However, calling VDBManagerSetCacheRoot
    will not update location of where run is cached.*/
    sprintf(path, "%s/QWERTY", getenv("HOME"));
    VPath* p(nullptr);
    REQUIRE_RC(VFSManagerMakePath(vfs, &p, path));
    REQUIRE_RC(VDBManagerSetCacheRoot(vdb, p));
    REQUIRE_RC(VPathRelease(p));
    /**************************************************************************/

    const VTable* tb(nullptr);
    REQUIRE_RC(VDBManagerOpenTableRead(vdb, &tb, nullptr, "SRR053325"));

    KNamelist* names(nullptr);
    REQUIRE_RC(VTableListReadableColumns(tb, &names));
    REQUIRE_RC(KNamelistRelease(names));

    REQUIRE_RC(VTableRelease(tb));

    REQUIRE_RC(VDBManagerRelease(vdb));

    REQUIRE_RC(VFSManagerRelease(vfs));

    REQUIRE_RC(KConfigRelease(cfg));
}

int main(int argc, char * argv[]) {
    return Test_cache_sdlSuite(argc, argv);
}
