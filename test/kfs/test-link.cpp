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
* ==============================================================================
*  Unit tests for KDirectoryCreateLink
*/

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */

#include <ktst/unit_test.hpp> /* TEST_SUITE */

#include <string>

using std::string;

TEST_SUITE(LinkSuite)

#define DIR "tmp"
#define DIR2 "tmp2"

struct Fixture {
    KDirectory * wd;

    Fixture() : wd(NULL) {
        rc_t rc = KDirectoryNativeDir(&wd);
        if (rc != 0)
            throw rc;

        rc = Clean();
        if (rc != 0)
            throw rc;
    }

    ~Fixture() { assert(!wd); }

    rc_t Compare(const string & src, const char * dest) {
        const KFile * fn = NULL;
        rc_t rc = KDirectoryOpenFileRead(wd, &fn, dest);
        if (rc != 0)
            return rc;

        char in[99] = "";
        size_t num_read = 0;
        rc = KFileRead(fn, 0, in, sizeof in, &num_read);
        if (rc != 0)
            return rc;

        if (string(in) != src)
            return 69;

        return KFileRelease(fn);
    }

    rc_t Fini() {
        rc_t rc = Clean();

        rc_t r2 = KDirectoryRelease(wd);
        if (r2 != 0 && rc == 0)
            rc = r2;
        wd = NULL;

        return rc;
    }

private:
    rc_t Clean() {
        KDirectoryRemove(wd, true, DIR);
        KDirectoryRemove(wd, true, DIR2);
        return 0;
    }
};

FIXTURE_TEST_CASE(Test_Link, Fixture) {
    const KFile * fn = NULL;
#define NEW "l"

    // link files do not exist
    REQUIRE_RC_FAIL(KDirectoryOpenFileRead(wd, &fn, "%s/%s", DIR, NEW));
    REQUIRE_RC_FAIL(KDirectoryOpenFileRead(wd, &fn, "%s/%s", DIR2, NEW));

    // create a next file
#define NXT "n"
    KFile * f2 = NULL;
    REQUIRE_RC(KDirectoryCreateFile(wd, &f2, false, 0660, kcmParents,
        "%s/%s", DIR, NXT));

    // create src file
#define OLD "f"
    KFile * fo = NULL;
    REQUIRE_RC(KDirectoryCreateFile(wd, &fo, false, 0660, kcmParents,
        "%s/%s", DIR, OLD));
    const char out[] = "0123456789\n";
    REQUIRE_RC(KFileWrite(fo, 0, out, sizeof out, NULL));

    // create link in the same directory
    REQUIRE_RC(KDirectoryCreateLink(wd, 0, 0, DIR "/" OLD, DIR "/" NEW));
    REQUIRE_RC(Compare(out, DIR "/" NEW));

    // cannot create link to existing file
    REQUIRE_RC_FAIL(KDirectoryCreateLink(wd, 0, 0, DIR "/" OLD, DIR "/" NXT));

    // cannot create link in non-existing directory without kcmParents
    REQUIRE_RC_FAIL(KDirectoryCreateLink(wd, 0, 0, DIR "/" OLD, "/9"));
    REQUIRE_RC_FAIL(KDirectoryCreateLink(wd, 0, 0, DIR "/" OLD,
        DIR2 "/" NEW));

    // create link in a different directory
    REQUIRE_RC(KDirectoryCreateLink(wd, 0770, kcmParents, DIR "/" OLD,
        DIR2 "/" NEW));
    REQUIRE_RC(Compare(out, DIR2 "/" NEW));

    REQUIRE_RC(KFileRelease(f2));
    REQUIRE_RC(KFileRelease(fo));

    REQUIRE_RC(Fini());
}

extern "C" {
    ver_t CC KAppVersion(void) { return 0; }
    rc_t CC KMain(int argc, char *argv[]) {
        return LinkSuite(argc, argv);
    }
}
