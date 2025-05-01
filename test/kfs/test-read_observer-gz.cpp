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
* Tests of KFileMD5ReadObserver for gz file
*/

#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */
#include <kfs/gzip.h> /* KFileMakeGzipForRead */
#include <kfs/md5.h> /* KFileMakeMD5ReadObserver */

#include <klib/printf.h> // snprintf
#include <klib/rc.h> // RC
#include <kproc/timeout.h> // TimeoutInit
#include <ktst/unit_test.hpp> /* TEST_SUITE */

#include <zlib.h> // z_stream

using std::string;

#define FREE(ptr) do { free((void*)ptr); ptr = nullptr; } while (false)

#define RELEASE(type, obj) do { REQUIRE_RC(type##Release(obj)); \
    obj = nullptr; } while (false)

#define ERR_HEAD "The file was not read to the end; it was read to byte "

TEST_SUITE(ReadObserverTestSuite)

const char SRC_FILE[]("../vdb/db/VDB-3418.sra");
const char DST_FILE[]("VDB-3418.sra.gz");

class Test : protected ncbi::NK::TestCase {
    TestCase *_dad;
public:
    const KFileMD5ReadObserver *md5;
    const KFile *file;
    uint64_t size;
    bool sizeUnknown;
    char *buf;
    const char *error;
    struct timeout_t tm;
    uint8_t digest[16];

    static KDirectory *cwd;

public:
    Test(TestCase *dad, const std::string &name)
        : TestCase(name)
        , _dad(dad)
        , md5(nullptr)
        , file(nullptr)
        , size(0)
        , sizeUnknown(false)
        , buf(nullptr)
        , error(nullptr)
    {
        TimeoutInit(&tm, 300000);
        memset(digest, 0, sizeof digest);
    }

    ~Test() {
        free(buf);

        assert(_dad);
        _dad->ErrorCounterAdd(GetErrorCounter());
    }

    void Start(bool failures = false // test how functions react
    )                                // to invalid arguments
    {
        const KFile *f(nullptr);
        REQUIRE_RC(KDirectoryOpenFileRead(cwd, &f, DST_FILE));
        REQUIRE_RC(KFileMakeGzipForRead(&file, f));
        RELEASE(KFile, f);

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
        buf = reinterpret_cast<char*>(malloc(size + 1));
        REQUIRE(buf);
    }

    void Finish(const string &aError = "",// if not empty - is an error message,
                                          // expect failure
        bool failures = false)   // the same as in Start()
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
                    = snprintf(&buf[total], size - total, "%02x", digest[i]);
                assert(len == 2);
                total += len;
            }
            buf[total] = '\0';
            REQUIRE_EQ(string(buf), string("7d66f3f346db0f916a8c723d40087b6c"));
        }
        else {
            REQUIRE_RC_FAIL(
                KFileMD5ReadObserverGetDigest(md5, digest, &error));
            REQUIRE_NOT_NULL(error);
            REQUIRE(digest[0] == '\0');
            REQUIRE_EQ(string(error), aError);
            FREE(error);
        }

        RELEASE(KFileMD5ReadObserver, md5);
    }

    static rc_t Begin() { /* create gz input test file */
        rc_t rc(0);

        rc = KDirectoryNativeDir(&cwd);

        const KFile *input(nullptr);
        if (rc == 0)
            rc = KDirectoryOpenFileRead(cwd, &input, "%s", SRC_FILE);

        KFile *f(nullptr);
        if (rc == 0)
            rc = KDirectoryCreateFile(cwd, &f,
                false, 0644, kcmInit, "%s", DST_FILE);

        KFile *output(nullptr);
        if (rc == 0)
            rc = KFileMakeGzipForWrite(&output, f);
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

    static rc_t End() {
        rc_t rc(KDirectoryRemove(cwd, false, "%s", DST_FILE));
        KDirectoryRelease(cwd); cwd = nullptr;
        return rc;
    }
};

KDirectory *Test::cwd(nullptr);

////////////////////////////////////////////////////////////////////////////////
// KFileReadAll

TEST_CASE(ReadAllExactly) {
    Test t(this, "ReadAllExactly");
    t.Start(true);
    size_t num_read(0);
    // request exactly all file; EOF cannot be detected by file size
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, t.size, &num_read));
    REQUIRE_EQ(num_read, t.size);
    if (t.sizeUnknown) {
        // EOF was not detected by file size
        REQUIRE_RC_FAIL(
            KFileMD5ReadObserverGetDigest(t.md5, t.digest, &t.error));
        REQUIRE_EQ(string(t.error), string(ERR_HEAD "12887839."));
        FREE(t.error);
        // detect EOF
        REQUIRE_RC(KFileReadAll(t.file, t.size, t.buf, 1, &num_read));
    }
    t.Finish("", true);
}

TEST_CASE(ReadAllExactlyPlus1) {
    Test t(this, "ReadAllExactlyPlus1");
    t.Start();
    size_t num_read(0);
    // request all file + 1 byte
    // will read one byte less than requested;
    // EOF will be detected by last read that returns 0 bytes
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, t.size + 1, &num_read));
    t.Finish();
}

TEST_CASE(ReadAllSkip0) {
    Test t(this, "ReadAllSkip0");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 1, t.buf, t.size, &num_read));
    // the first byte was not read
    t.Finish(ERR_HEAD "0.", true);
}

TEST_CASE(ReadAllSkip1) {
    Test t(this, "ReadAllSkip1");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, 1, &num_read));
//  REQUIRE_RC(KFileReadAll(t.file, 1, t.buf, 1, &num_read));
    REQUIRE_RC(KFileReadAll(t.file, 2, t.buf, t.size, &num_read));
    // the second byte was not read
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(ReadAllSkipLast) {
    Test t(this, "ReadAllSkipLast");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, t.size - 1, &num_read));
//  REQUIRE_RC(KFileReadAll(t.file, t.size - 1, t.buf, 2, &num_read));
    // the last byte was not read
    t.Finish(ERR_HEAD "12887838.");
}

// gzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadAllTwice) {
    Test t(this, "ReadAllTwice");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, 1234567, &num_read));
    // this call will request the part that was already completely read
    REQUIRE_RC_FAIL(KFileReadAll(t.file, 1, t.buf, 123456, &num_read));
    t.Finish(ERR_HEAD "1234567.", true);
}

// gzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadAllTwicePartially) {
    Test t(this, "ReadAllTwicePartially");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, 2000000, &num_read));
    // this call will request the part that was already read and new data
    REQUIRE_RC_FAIL(KFileReadAll(t.file, 1000000, t.buf, t.size, &num_read));
    t.Finish(ERR_HEAD "2000000.");
}

////////////////////////////////////////////////////////////////////////////////
// KFileRead

TEST_CASE(Read) {
    Test t(this, "Read");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 2, &num_read));
    t.Finish();
}

TEST_CASE(ReadSkip0) {
    Test t(this, "ReadSkip0");
    t.Start();
    size_t num_read(0);
    uint64_t pos(1);
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 2, &num_read));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(ReadSkip1) {
    Test t(this, "ReadSkip1");
    t.Start();
    size_t num_read(0);
    uint64_t pos(0);
    REQUIRE_RC(KFileRead(t.file, pos, t.buf, 1, &num_read));
    pos += num_read;
//  REQUIRE_RC(KFileRead(t.file, pos, t.buf, 1, &num_read));
    for (++pos; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 3, &num_read));
    t.Finish(ERR_HEAD "1.");
}

// gzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadTwice) {
    Test t(this, "ReadTwice");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0;         num_read > 0 && pos < 1234567; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 100000, &num_read));
    for (size_t n = 0; n > 0; )
    {   REQUIRE_RC_FAIL(KFileRead(t.file, 1, t.buf, 10000, &n)); break; }
    for (            ; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 100000, &num_read));
    t.Finish();
}

// gzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadTwicePartially) {
    Test t(this, "ReadTwicePartially");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0;         num_read > 0 && pos < 2000000; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 100000, &num_read));
    for (pos = 1000000; num_read > 0; pos += num_read)
        REQUIRE_RC_FAIL(KFileRead(t.file, pos, t.buf, 100000, &num_read));
    t.Finish(ERR_HEAD "2000000.", true);
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedRead

TEST_CASE(TimedRead) {
    Test t(this, "TimedRead");
    t.Start(true);
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0; pos += num_read)
        // KFileTimedRead is not supported for bzip KFile
        REQUIRE_RC_FAIL(KFileTimedRead(t.file, 0, t.buf, 99, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0.", true);
}

#if 0
TEST_CASE(TimedReadSkip0) {
    Test t(this, "TimedReadSkip0");
    t.Start(true);
    size_t num_read(0);
    uint64_t pos(1);
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(TimedReadSkip1) {
    Test t(this, "TimedReadSkip1");
    t.Start(true);
    size_t num_read(0);
    uint64_t pos(0);
    REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 1, &num_read, &t.tm));
    pos += num_read;
//REQUIRE_RC(KFileTimedRead(t.file, pos,t.buf,1,&num_read,&t.tm));pos+=num_read;
    for (++pos; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(TimedReadTwice) {
    Test t(this, "TimedReadTwice");
    t.Start(true);
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0 && pos < 1234567; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    for (pos = 1; num_read > 0 && pos < 123456; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    for (pos = 1234567; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadTwicePartially) {
    Test t(this, "TimedReadTwicePartially");
    t.Start(true);
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0 && pos < 2000000; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    for (pos = 1000000; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 90000, &num_read, &t.tm));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadAll

TEST_CASE(TimedReadAllExactly) {
    Test t(this, "TimedReadAllExactly");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, t.size, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllExactlyPlus1) {
    Test t(this, "TimedReadAllExactlyPlus1");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, t.size + 1, &num_read,
        &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllSkip0) {
    Test t(this, "TimedReadAllSkip0");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 1, t.buf, t.size, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(TimedReadAllSkip1) {
    Test t(this, "TimedReadAllSkip1");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, 1, &num_read, &t.tm));
    //REQUIRE_RC(KFileTimedReadAll(t.file, 1, t.buf, 1, &num_read, &t.tm));
    REQUIRE_RC(KFileTimedReadAll(t.file, 2, t.buf, t.size, &num_read, &t.tm));
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(TimedReadAllSkipLast) {
    Test t(this, "TimedReadAllSkipLast");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, t.size - 1, &num_read,
        &t.tm));
//REQUIRE_RC(KFileTimedReadAll(t.file, t.size - 1, t.buf, 1, &num_read, &t.tm));
    t.Finish(ERR_HEAD "12887838.");
}

TEST_CASE(TimedReadAllTwice) {
    Test t(this, "TimedReadAllTwice");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, 123, &num_read, &t.tm));
    REQUIRE_RC(KFileTimedReadAll(t.file, 1, t.buf, 12, &num_read, &t.tm));
    REQUIRE_RC(KFileTimedReadAll(t.file, 123, t.buf, t.size, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllTwicePartially) {
    Test t(this, "TimedReadAllTwicePartially");
    t.Start(true);
    size_t num_read(0);
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, 2000000, &num_read, &t.tm));
    REQUIRE_RC(KFileTimedReadAll(t.file, 999, t.buf, t.size, &num_read, &t.tm));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileReadExactly

TEST_CASE(ReadExactly) {
    Test t(this, "ReadExactly");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size));
    t.Finish("", true);
}

TEST_CASE(ReadExactlySkip0) {
    Test t(this, "ReadExactlySkip0");
    t.Start(true);
    //REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, t.size - 1));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(ReadExactlySkip1) {
    Test t(this, "ReadExactlySkip1");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
    //REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 2, t.buf, t.size - 2));
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(ReadExactlySkipLast) {
    Test t(this, "ReadExactlySkipLast");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size - 1));
    //REQUIRE_RC(KFileReadExactly(t.file, t.size - 1, t.buf, 1));
    t.Finish(ERR_HEAD "12887838.");
}

TEST_CASE(ReadExactlyTwice) {
    Test t(this, "ReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1234567));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 123456));
    REQUIRE_RC(KFileReadExactly(t.file, 1234567, t.buf, t.size - 1234567));
    t.Finish();
}

TEST_CASE(ReadExactlyTwicePartially) {
    Test t(this, "ReadExactlyTwicePartially");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 2000000));
    REQUIRE_RC(KFileReadExactly(t.file, 1000000, t.buf, t.size - 1000000));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadExactly

TEST_CASE(TimedReadExactly) {
    Test t(this, "TimedReadExactly");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlySkip0) {
    Test t(this, "TimedReadExactlySkip0");
    t.Start(true);
    //REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, t.size - 1, &t.tm));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(TimedReadExactlySkip1) {
    Test t(this, "ReadExactlySkip1");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
    //REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 2, t.buf, t.size - 2, &t.tm));
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(TimedReadExactlySkipLast) {
    Test t(this, "TimedReadExactlySkipLast");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size - 1, &t.tm));
    //REQUIRE_RC(KFileTimedReadExactly(t.file, t.size - 1, t.buf, 1, &t.tm));
    t.Finish(ERR_HEAD "12887838.");
}

TEST_CASE(TimedReadExactlyTwice) {
    Test t(this, "TimedReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1234567, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 123456, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1234567,
                             t.buf, t.size - 1234567, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlyTwicePartially) {
    Test t(this, "TimedReadExactlyTwicePartially");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 2000000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 999, t.buf, t.size - 999, &t.tm));
    t.Finish();
}
#endif

extern "C" {
    rc_t CC KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings();

        rc_t rc(Test::Begin());

        if (rc == 0)
            rc = ReadObserverTestSuite(argc, argv);

        rc_t r2(Test::End());
        if (rc == 0 && r2 != 0)
            rc = r2;

        return rc;
    }
}

/******************************************************************************/
