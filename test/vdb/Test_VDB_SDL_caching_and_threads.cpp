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
 * ========================================================================== */

#include <kapp/vdbapp.h> // Quitting
#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h> // VDBManagerRelease
#include <fstream> // ifstream

using std::cerr;
using std::endl;

TEST_SUITE(Test_VDB_SDL_caching_and_threadsSuite)

TEST_CASE(Test) {
    const VDBManager* mgr(nullptr);
    REQUIRE_RC(VDBManagerMakeRead(&mgr, nullptr));

    std::ifstream
        acc_list("accessionsFor-Test_VDB_SDL_caching_and_threads.txt");
    std::string accession;
    size_t count(0);

    while (acc_list >> accession) {
        const VDatabase* vdb(nullptr);

        cerr << ++count << ": opening " << accession << ": " << std::flush;
        REQUIRE_RC(VDBManagerOpenDBRead(
            mgr, &vdb, 0, "%.*s", accession.size(), accession.data()));
        cerr << " opened" << endl;

        REQUIRE_RC(VDatabaseRelease(vdb));

        if (Quitting())
            break;
    }

    REQUIRE_RC(VDBManagerRelease(mgr));
}

int main(int argc, char* argv[]) {
    const char name[]("TEST_VDB_SDL_CACHING_AND_THREADS");
    if (getenv(name) == NULL) {
        cerr << name << " was not set: skipping the test." << endl;
        return 0;
    }

    KConfigDisableUserSettings();

    return Test_VDB_SDL_caching_and_threadsSuite(argc, argv);
}
