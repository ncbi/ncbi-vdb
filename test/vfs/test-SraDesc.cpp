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
* =========================================================================== */

#include <cmath>

#include "../libs/vfs/SraDesc.c"

#include <kfg/config.h> /* KConfigDisableUserSettings */

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */

#include <klib/printf.h> /* string_printf */

#include <ktst/unit_test.hpp> /* KMain */

TEST_SUITE(SraDescTestSuite);

#define ALL
#define DIR "tmp"

struct SraDescTestFixture {
    KDirectory * dir = NULL;
    KFile * f = NULL;

    SraDescTestFixture() : dir(0), f(0) {
        rc_t rc = KDirectoryNativeDir(&dir); if (rc != 0) throw rc;
        KDirectoryRemove(dir, true, "%s", DIR);
    }

    ~SraDescTestFixture() { Fini(); }

    rc_t Fini() {
        rc_t rc = KFileRelease(f); f = 0;

        KDirectoryRemove(dir, true, "%s", DIR);

        rc_t r2 = KDirectoryRelease(dir); dir = 0;
        if (r2 != 0 && rc == 0)
            rc = r2;

        return rc;
    }
};

#ifdef ALL
FIXTURE_TEST_CASE(BinFileTest, SraDescTestFixture) { // load from binary file
    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s/sdb", DIR));

    char b[512] = "";
    size_t num_writ = 0;
    REQUIRE_RC(string_printf(b, sizeof b, &num_writ, "NCBIRrDs"));
    b[num_writ++] = 255;

    union {
        uint64_t u;
        uint8_t b[8];
    } n;
    n.u = 1234567890;

    for (int i = 0; i < 8; ++i)
        b[num_writ++] = n.b[i];
    for (int i = 0; i < 8; ++i)
        b[num_writ++] = n.b[i];

    REQUIRE_RC(KFileWrite(f, 0, b, num_writ, NULL));

    SraDesc sd;
    REQUIRE(memset(&sd, 0, sizeof sd));
    sd._sdType = eBin;

    REQUIRE_RC(SraDescLoadPath(&sd, dir, "tmp/sdb"));

    SraDesc e;
    SraDescSet(&e, eQualDefault, n.u, eBin);

    REQUIRE_RC(SraDescCmp(&sd, &e));

    REQUIRE_RC(Fini());
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(TextFileTest, SraDescTestFixture) { // load from textual file
    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s/sdt", DIR));

    char b[512] = "";
    size_t num_writ = 0;
    REQUIRE_RC(string_printf(b, sizeof b, &num_writ,
        "NCBIRrDs\n"
        "255\n"
        "1234567890\n"
        "0123456789abcdef0123456789abcdef\n"));
    REQUIRE_RC(KFileWrite(f, 0, b, num_writ, NULL));

    SraDesc sd;
    REQUIRE(memset(&sd, 0, sizeof sd));
    sd._sdType = eTextual;
    REQUIRE_RC(SraDescLoadPath(&sd, dir, "tmp/sdt"));

    SraDesc e;
    SraDescSet(&e, eQualDefault, 1234567890, eTextual);

    REQUIRE_RC(SraDescCmp(&sd, &e));

    REQUIRE_RC(Fini());
}
#endif

#ifdef ALL
// save textual; convert to bin; convert to text
FIXTURE_TEST_CASE(ConvertTest, SraDescTestFixture) {
    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s/sd.dst", DIR));

    char b[512] = "";
    size_t num_writ = 0;
    REQUIRE_RC(string_printf(b, sizeof b, &num_writ,
        "NCBIRrDs\n"
        "255\n"
        "1234567890\n"
        "0123456789abcdef0123456789abcdef\n"));
    REQUIRE_RC(KFileWrite(f, 0, b, num_writ, NULL));

    SraDesc sd;
    REQUIRE(memset(&sd, 0, sizeof sd));
    sd._sdType = eTextual;
    REQUIRE_RC(SraDescLoadPath(&sd, dir, "tmp/sd.dst"));

    SraDesc e;
    SraDescSet(&e, eQualDefault, 1234567890, eTextual);

    REQUIRE_RC(SraDescCmp(&sd, &e));

    bool recognized = false;
    REQUIRE_RC(SraDescConvert(dir, "tmp/sd.dst", &recognized));
    REQUIRE(recognized);

    sd._sdType = e._sdType = eBin;
    REQUIRE_RC(SraDescLoadPath(&sd, dir, "tmp/sd.dsc"));

    REQUIRE_RC(SraDescCmp(&sd, &e));

    REQUIRE_RC(Fini());
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(LoalFullQualityTest, SraDescTestFixture) {
    const char sra[] = "0.sra";
    char path[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(dir, true, path, sizeof path,
        "%s/%s", DIR, sra));
    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s", path));

    char b[512] = "";
    REQUIRE_RC(KFileWrite(f, 0, b, sizeof b, NULL));
    REQUIRE_RC(KFileRelease(f)); f = 0;

    SraDesc sd;
    VQuality q(eQualFull);
    SraDescSet(&sd, q, sizeof b, eBin);

    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s.dsc", path));

    REQUIRE_RC(SraDescSave(&sd, f));

    VPath * p = NULL;
    REQUIRE_RC(VPathMake(&p, path));
    REQUIRE_NOT_NULL(p);
    REQUIRE_EQ(p->quality, (int32_t)eQualFull);

    REQUIRE_RC(VPathLoadQuality(p));
    REQUIRE_EQ(p->quality, q);

    REQUIRE_RC(VPathRelease(p));

    REQUIRE_RC(Fini());
}
#endif

#ifdef ALL
FIXTURE_TEST_CASE(LoalObsoleteQualityTest, SraDescTestFixture) {
    const char sra[] = "0.sra";
    char path[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(dir, true, path, sizeof path,
        "%s/%s", DIR, sra));
    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s", path));

    char b[512] = "";
    REQUIRE_RC(KFileWrite(f, 0, b, sizeof b, NULL));
    REQUIRE_RC(KFileRelease(f)); f = 0;

    SraDesc sd;
    VQuality q(eQualFull);
    SraDescSet(&sd, q, sizeof b - 1, eBin);

    REQUIRE_RC(KDirectoryCreateFile(dir, &f, false,
        0664, kcmInit | kcmParents, "%s.dsc", path));
    REQUIRE_RC(SraDescSave(&sd, f));

    VPath * p = NULL;
    REQUIRE_RC(VPathMake(&p, path));
    REQUIRE_NOT_NULL(p);
    REQUIRE_EQ(p->quality, (int32_t)eQualFull);

    REQUIRE_RC(VPathLoadQuality(p));
    REQUIRE_EQ(p->quality, (int32_t)eQualLast);

    REQUIRE_RC(VPathRelease(p));

    REQUIRE_RC(Fini());
}
#endif

extern "C" {
    ver_t CC KAppVersion(void) { return 0; }
    rc_t CC KMain(int argc, char * argv[]) {
        KConfigDisableUserSettings();
        return SraDescTestSuite(argc, argv);
    }
}
