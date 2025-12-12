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
* Common code for KFileMD5ReadObserver tests
*/

#include <kfs/bzip.h> /* KFileMakeBzip2ForWrite */
#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */
#include <kfs/gzip.h> /* KFileMakeGzipForWrite */
#include <kfs/md5.h> /* KFileMakeMD5ReadObserver */

#include <klib/rc.h> // RC
#include <kproc/timeout.h> // TimeoutInit
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path.h> /* VPathRelease */

#include <ktst/unit_test.hpp> /* TEST_SUITE */

#define FREE(ptr) do { free((void*)ptr); ptr = nullptr; } while (false)

#define RELEASE(type, obj) do { REQUIRE_RC(type##Release(obj)); \
    obj = nullptr; } while (false)

#define ERR_HEAD "The file was not read to the end; it was read to byte "

char SRC_FILE[]("../vdb/db/VDB-3418.sra");
const char *DST_FILE(nullptr);

typedef enum {
    eEmpty,
    ePlain,
    eBz2,
    eGzip,
    eHttp,
} EType;

class ObserverTest:protected ncbi::NK::TestCase {
    static VFSManager *s_mgr;
    static const VPath *s_path;
    static const std::string s_full_digest;
    static const std::string s_empty_digest;
    static std::string m_digest;
    static const std::string *s_digest;
    static KDirectory *s_cwd;
    static EType s_type;

    TestCase *_dad;

public:
    const KFileMD5ReadObserver *md5;
    const KFile *file;
    uint64_t size;
    bool sizeUnknown;
    char *buf;
    uint64_t bufSize;
    const char *error;
    uint8_t digest[16];
    struct timeout_t tm;

    ObserverTest(TestCase *dad, const std::string &name)
        : TestCase(name)
        , _dad(dad)
        , md5(NULL)
        , file(nullptr)
        , size(0)
        , sizeUnknown(false)
        , buf(NULL)
        , bufSize(0)
        , error(nullptr)
    {
        memset(digest, 0, sizeof digest);
        TimeoutInit(&tm, 300000);
    }

    ~ObserverTest() {
        free(buf);

        assert(_dad);
        _dad->ErrorCounterAdd(GetErrorCounter());
    }

    static void Prepare(EType type) {
        rc_t rc(KDirectoryNativeDir(&s_cwd));

        s_type = type;
        if (s_type == eEmpty) {
            strcpy(SRC_FILE, "empty");

            KFile* f(nullptr);
            if (rc == 0)
                rc = KDirectoryCreateFile(s_cwd, &f, false, 0400, kcmInit,
                    SRC_FILE);

            rc_t r2(KFileRelease(f));
            if (rc == 0 && r2 != 0)
                rc = r2;
        }
    }

    static rc_t Begin(EType type = ePlain) { /* prepare input test file */
        rc_t rc(0);

        if (s_cwd == nullptr)
            rc = KDirectoryNativeDir(&s_cwd);

        if (s_type == ePlain)
            s_type = type;

        if (type == eEmpty) {
            s_digest = &s_empty_digest;
            return rc;
        }
        else if (type == ePlain)
            return rc;

        else if (s_type == eHttp) {
            rc = VFSManagerMake(&s_mgr);
            if (rc != 0) {
                std::cerr << "cannot VFSManagerMake\n";
                return rc;
            }
            rc = VFSManagerResolveRemote(s_mgr, "SRR053325", &s_path, nullptr);
            if (rc != 0) {
                std::cerr << "cannot resolve SRR053325\n";
                return rc;
            }

            const uint8_t *md5(VPathGetMd5(s_path));
            if (md5 == nullptr) {
                std::cerr << "cannot resolve SRR053325\n";
                return RC(rcFS, rcFile, rcReading, rcString, rcNull);
            }

            char digest[64] = "";
            int size(64), total(0);
            for (int i = 0; i < 16; ++i) {
                int len
                    = snprintf(&digest[total], size - total, "%02x", md5[i]);
                assert(len == 2);
                total += len;
            }
            digest[total] = '\0';
            m_digest = digest;
            s_digest = &m_digest;

            return rc;
        }

        else {
            const KFile *input(nullptr);
            if (rc == 0)
                rc = KDirectoryOpenFileRead(s_cwd, &input, "%s", SRC_FILE);

            if (type == eBz2)
                DST_FILE = "VDB-3418.sra.bz2";
            else if (type == eGzip)
                DST_FILE = "VDB-3418.sra.gz";
            else { assert(0); return rc; }

            KFile *f(nullptr);
            if (rc == 0)
                rc = KDirectoryCreateFile(s_cwd, &f,
                    false, 0644, kcmInit, "%s", DST_FILE);

            KFile *output(nullptr);
            if (rc == 0) {
                if (type == eBz2)
                    rc = KFileMakeBzip2ForWrite(&output, f);
                else if (type == eGzip)
                    rc = KFileMakeGzipForWrite(&output, f);
                else { assert(0); return rc; }
            }
            KFileRelease(f); f = nullptr;

            uint64_t fileSize(0);
            if (rc == 0)
                rc = KFileSize(input, &fileSize);
            void *inputBuffer(nullptr);
            if (rc == 0) {
                inputBuffer = malloc(fileSize);
                if (inputBuffer == nullptr)
                    rc = RC(rcFS, rcFile, rcCreating, rcMemory, rcExhausted);
            }

            size_t actual(0);
            if (rc == 0)
                rc = KFileReadAll(input, 0, inputBuffer, fileSize, &actual);
            if (rc == 0 && actual != fileSize)
                rc = RC(rcFS, rcFile, rcCreating, rcSize, rcUnequal);
            KFileRelease(input); input = nullptr;

            if (rc == 0)
                rc = KFileWrite(output, 0, inputBuffer, fileSize, NULL);

            free(inputBuffer); inputBuffer = nullptr;
            KFileRelease(output); output = nullptr;

            return rc;
        }
    }

    static rc_t End() {
        rc_t rc(0);
        
        if (DST_FILE != nullptr)
            rc = KDirectoryRemove(s_cwd, false, "%s", DST_FILE);

        if (s_type == eEmpty) {
            rc_t r2(KDirectoryRemove(s_cwd, true, SRC_FILE));
            if (rc == 0 && r2 != 0)
                rc = r2;
        }

        KDirectoryRelease(s_cwd); s_cwd = nullptr;
        VFSManagerRelease(s_mgr); s_mgr = nullptr;
        VPathRelease(s_path); s_path = nullptr;

        return rc;
    }

    void Start(bool failures = false // test how functions react
    )                                // to invalid arguments
    {
        if (s_type == ePlain || s_type == eEmpty)
            REQUIRE_RC(KDirectoryOpenFileRead(s_cwd, &file, SRC_FILE));
        else if (s_type == eHttp)
            REQUIRE_RC(VFSManagerOpenFileRead(s_mgr, &file, s_path));
        else {
            const KFile *f(nullptr);
            REQUIRE_RC(KDirectoryOpenFileRead(s_cwd, &f, DST_FILE));
            if (s_type == eBz2)
                REQUIRE_RC(KFileMakeBzip2ForRead(&file, f));
            else if (s_type == eGzip)
                REQUIRE_RC(KFileMakeGzipForRead(&file, f));
            else
                REQUIRE_EQ(0, ~0);
            RELEASE(KFile, f);
        }

        if (failures) {
            REQUIRE_RC_FAIL(KFileMakeMD5ReadObserver(file, nullptr));
            REQUIRE_RC_FAIL(KFileMakeMD5ReadObserver(nullptr, &md5));
            REQUIRE_NULL(md5);
        }

        REQUIRE_RC(KFileMakeMD5ReadObserver(file, &md5));

        rc_t rc(KFileSize(file, &size));
        if (rc != 0) {
            sizeUnknown = true;
            size = 12887839;
        }
        bufSize = size;
        if (bufSize < 33)
            bufSize = 33;
        buf = reinterpret_cast<char*>(malloc(bufSize + 1));
        REQUIRE(buf);
    }

    void Finish(const std::string &aError = "", // if not empty
                                       //  - is an error message, expect failure
        bool failures = false) // the same as in Start()
    {
        bool success = aError == "";

        RELEASE(KFile, file);

        if (failures) {
            REQUIRE_RC_FAIL(KFileMD5ReadObserverGetDigest(nullptr, digest,
                &error));
            REQUIRE_NULL(error);
            if (success)
                REQUIRE_RC(KFileMD5ReadObserverGetDigest(md5, digest, nullptr));
            else
                REQUIRE_RC_FAIL(
                    KFileMD5ReadObserverGetDigest(md5, digest, nullptr));
        }

        if (success) {
            REQUIRE_RC(KFileMD5ReadObserverGetDigest(md5, digest, &error));
            REQUIRE_NULL(error);
            REQUIRE(digest[0] != '\0');

            int total = 0;
            for (int i = 0; i < 16; ++i) {
                int len
                    = snprintf(&buf[total], bufSize - total, "%02x", digest[i]);
                assert(len == 2);
                total += len;
            }
            buf[total] = '\0';

            assert(s_digest);
            REQUIRE_EQ(std::string(buf), *s_digest);
        }

        else {
            REQUIRE_RC_FAIL(
                KFileMD5ReadObserverGetDigest(md5, digest, &error));
            REQUIRE_NOT_NULL(error);
            REQUIRE(digest[0] == '\0');
            REQUIRE_EQ(std::string(error), aError);
            FREE(error);
        }

        RELEASE(KFileMD5ReadObserver, md5);
    }
};

VFSManager *ObserverTest::s_mgr(nullptr);
const VPath *ObserverTest::s_path(nullptr);
const std::string ObserverTest::s_full_digest(
    "7d66f3f346db0f916a8c723d40087b6c");
const std::string ObserverTest::s_empty_digest(
    "d41d8cd98f00b204e9800998ecf8427e");
std::string ObserverTest::m_digest;
const std::string* ObserverTest::s_digest = &s_full_digest;
KDirectory *ObserverTest::s_cwd(nullptr);
EType ObserverTest::s_type(ePlain);
