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
* Tests of KFileMD5ReadObserver for HTTP file
*/

#include <kfg/config.h> /* KConfigDisableUserSettings */
#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */
#include <kfs/md5.h> /* KFileMakeMD5ReadObserver */

#include <klib/printf.h> // snprintf
#include <kproc/timeout.h> // TimeoutInit
#include <ktst/unit_test.hpp> /* TEST_SUITE */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path.h> /* VPathRelease */

using std::string;

void FREE(const void *ptr) { free((void*)ptr); }

#define RELEASE(type, obj) do { REQUIRE_RC(type##Release(obj)); \
    obj = nullptr; } while (false)

#define ERR_HEAD "The file was not read to the end; it was read to byte "

TEST_SUITE(ReadObserverTestSuite)

class Test : protected ncbi::NK::TestCase {
    TestCase *_dad;
    const VPath *_path;
    const KFileMD5ReadObserver *_md5;
public:
    const KFile *file;
    uint64_t size;
    char *buf;
    struct timeout_t tm;

public:
    Test(TestCase *dad, const std::string &name)
        : TestCase(name)
        , _dad(dad)
        , _path(nullptr)
        , _md5(nullptr)
        , file(nullptr)
        , size(0)
        , buf(nullptr)
    {
        VFSManager *mgr(nullptr);
        REQUIRE_RC(VFSManagerMake(&mgr));
        REQUIRE_RC(VFSManagerResolveRemote(mgr, "SRR053325", &_path, nullptr));
        REQUIRE_RC(VFSManagerOpenFileRead(mgr, &file, _path));
        RELEASE(VFSManager, mgr);

        TimeoutInit(&tm, 300000);
    }

    ~Test() {
        free(buf);
    
        assert(_dad);
        _dad->ErrorCounterAdd(GetErrorCounter());
    }

    void Start(bool failures = false // test how functions react
    )                                // to invalid invalid arguments
    {
        if (failures) {
            REQUIRE_RC_FAIL(KFileMakeMD5ReadObserver(file, nullptr));
            REQUIRE_RC_FAIL(KFileMakeMD5ReadObserver(nullptr, &_md5));
            REQUIRE_NULL(_md5);
        }

        REQUIRE_RC(KFileMakeMD5ReadObserver(file, &_md5));
        
        REQUIRE_RC(KFileSize(file, &size));
        buf = reinterpret_cast<char*>(malloc(size + 1));
        REQUIRE(buf);
    }

    void Finish(const string &aError = "",// if not empty - is an error message,
                                          // expect failure
        bool failures = false )   // the same as in Start()
    {
        bool success = aError == "";

        RELEASE(KFile, file);
        
        uint8_t digest[16];
        memset(digest, 0, sizeof digest);
        const char *error(nullptr);
        
        if (failures) {
            REQUIRE_RC_FAIL(KFileMD5ReadObserverGetDigest(nullptr, digest,
                &error));
            REQUIRE_NULL(error);
            if (success)
                REQUIRE_RC(KFileMD5ReadObserverGetDigest(_md5, digest,
                    nullptr));
            else
                REQUIRE_RC_FAIL(
                    KFileMD5ReadObserverGetDigest(_md5, digest, nullptr));
        }

        if (success) {
            const uint8_t *md5(VPathGetMd5(_path));
            REQUIRE_NOT_NULL(md5);
            char e[64]("");

            REQUIRE_RC(KFileMD5ReadObserverGetDigest(_md5, digest, &error));
            REQUIRE_NULL(error);
            REQUIRE(digest[0] != '\0');
        
            int total = 0;
            for (int i = 0; i < 16; ++i) {
                int len
                    = snprintf(&buf[total], size - total, "%02x", digest[i]);
                assert(len == 2);
                len = snprintf(&e[total], size - total, "%02x", md5[i]);
                assert(len == 2);
                total += len;
            }
            buf[total] = '\0';
            e[total] = '\0';

            REQUIRE_EQ(string(buf), string(e));
        }
        else {
            REQUIRE_RC_FAIL(
                KFileMD5ReadObserverGetDigest(_md5, digest, &error));
            REQUIRE_NOT_NULL(error);
            REQUIRE(digest[0] == '\0');
            REQUIRE_EQ(string(error), aError);
            FREE(error);
        }

        RELEASE(VPath, _path);
        RELEASE(KFileMD5ReadObserver, _md5);
    }
};

////////////////////////////////////////////////////////////////////////////////
// KFileReadAll

TEST_CASE(ReadAllExactly) {
    Test t(this, "ReadAllExactly");
    t.Start(true);
    size_t num_read(0), sz(t.size);
    uint64_t pos(0);
    // request exactly all file; EOF is detected by file size
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    t.Finish("", true);
}

TEST_CASE(ReadAllExactlyPlus1) {
    Test t(this, "ReadAllExactlyPlus1");
    t.Start();
    size_t num_read(0), sz(t.size + 1);
    uint64_t pos(0);
    // request all file + 1 byte
    // will read one byte less than requested;
    // EOF can be detected by last read that returns 0 bytes
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    t.Finish();
}

TEST_CASE(ReadAllSkip0) {
    Test t(this, "ReadAllSkip0");
    t.Start(true);
    size_t num_read(0), sz(t.size);
    uint64_t pos(1);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    // the first byte was not read
    t.Finish(ERR_HEAD "0 of 31838.", true);
}

TEST_CASE(ReadAllSkip1) {
    Test t(this, "ReadAllSkip1");
    t.Start();
    size_t num_read(0), sz(t.size);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, 1, &num_read));
//  REQUIRE_RC(KFileReadAll(t.file, 1, t.buf, 1, &num_read));
    uint64_t pos(2);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    // the second byte was not read
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(ReadAllSkipLast) {
    Test t(this, "ReadAllSkipLast");
    t.Start();
    size_t num_read(0), sz(t.size - 1);
    uint64_t pos(0);
    for (; pos < t.size - 1; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
//  REQUIRE_RC(KFileReadAll(t.file, t.size - 1, t.buf, 1, &num_read));
    // the last byte was not read
    t.Finish(ERR_HEAD "31837 "
                   "of 31838.");
}

TEST_CASE(ReadAllTwice) {
    Test t(this, "ReadAllTwice");
    t.Start();
    size_t num_read(0), sz(10000);
    uint64_t pos(0);
    for (; pos < 10000; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    // this call will request the part that was already completely read
    for (pos = 1, sz = 5000; pos < 5000; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    for (pos = 5000, sz = t.size - 5000;
        pos < t.size; pos += num_read, sz -= num_read)
    {   REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read)); }
    t.Finish();
}

TEST_CASE(ReadAllTwicePartially) {
    Test t(this, "ReadAllTwicePartially");
    t.Start();
    size_t num_read(0), sz(20000);
    uint64_t pos(0);
    for (; pos < 20000; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    for (pos = 2000, sz = t.size - 2000;
        pos < t.size; pos += num_read, sz -= num_read)
    {   REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read)); }
    // this call requested the part that was already read and new data
    t.Finish();
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
//  REQUIRE_RC(KFileRead(t.file, 0, t.buf, 1, &num_read));
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 2, &num_read));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(ReadSkip1) {
    Test t(this, "ReadSkip1");
    t.Start();
    size_t num_read(0);
    uint64_t pos(0);
    REQUIRE_RC(KFileRead(t.file, pos, t.buf, 1, &num_read));
    pos += num_read;
//  REQUIRE_RC(KFileRead(t.file, pos, t.buf, 1, &num_read)); pos += num_read;
    for (++pos; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 3, &num_read));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(ReadTwice) {
    Test t(this, "ReadTwice");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0;         num_read > 0 && pos < 20000; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 2000, &num_read));
    for (pos = 1;         num_read > 0 && pos < 10000; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 1000, &num_read));
    for (pos = 20000; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 20000, &num_read));
    t.Finish();
}

TEST_CASE(ReadTwicePartially) {
    Test t(this, "ReadTwicePartially");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0;          num_read > 0 && pos < 20000; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 10000, &num_read));
    for (pos = 10000; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 20000, &num_read));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedRead

TEST_CASE(TimedRead) {
    Test t(this, "TimedRead");
    t.Start(true);
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 10000, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadSkip0) {
    Test t(this, "TimedReadSkip0");
    t.Start(true);
    size_t num_read(0);
    uint64_t pos(1);
    //REQUIRE_RC(KFileRead(t.file, 0, t.buf, 1, &num_read));
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 10000, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(TimedReadSkip1) {
    Test t(this, "TimedReadSkip1");
    t.Start();
    size_t num_read(0);
    uint64_t pos(0);
    REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 1, &num_read, &t.tm));
    pos += num_read;
//REQUIRE_RC(KFileTimedRead(t.file, pos,t.buf,1,&num_read,&t.tm));pos+=num_read;
    for (++pos; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 10000, &num_read, &t.tm));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(TimedReadTwice) {
    Test t(this, "TimedReadTwice");
    t.Start(true);
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0 && pos < 20000; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 2000, &num_read, &t.tm));
    for (pos = 1; num_read > 0 && pos < 10000; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 1000, &num_read, &t.tm));
    for (pos = 20000; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 20000, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadTwicePartially) {
    Test t(this, "TimedReadTwicePartially");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0 && pos < 20000; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 10000, &num_read, &t.tm));
    for (pos = 10000; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 20000, &num_read, &t.tm));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadAll

TEST_CASE(TimedReadAllExactly) {
    Test t(this, "TimedReadAllExactly");
    t.Start();
    size_t num_read(0), sz(t.size);
    uint64_t pos(0);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllExactlyPlus1) {
    Test t(this, "TimedReadAllExactlyPlus1");
    t.Start();
    size_t num_read(0), sz(t.size + 1);
    uint64_t pos(0);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllSkip0) {
    Test t(this, "TimedReadAllSkip0");
    t.Start(true);
    size_t num_read(0), sz(t.size);
    //REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, 1, &num_read, &t.tm));
    uint64_t pos(1);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(TimedReadAllSkip1) {
    Test t(this, "TimedReadAllSkip1");
    t.Start(true);
    size_t num_read(0), sz(t.size);;
    REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, 1, &num_read, &t.tm));
//  REQUIRE_RC(KFileTimedReadAll(t.file, 1, t.buf, 1, &num_read, &t.tm));
    uint64_t pos(2);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(TimedReadAllSkipLast) {
    Test t(this, "TimedReadAllSkipLast");
    t.Start();
    size_t num_read(0), sz(t.size - 1);
    uint64_t pos(0);
    for (; pos < t.size - 1; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
//REQUIRE_RC(KFileTimedReadAll(t.file, t.size - 1, t.buf, 1, &num_read, &t.tm));
    t.Finish(ERR_HEAD "31837 "
                   "of 31838.");
}

TEST_CASE(TimedReadAllTwice) {
    Test t(this, "TimedReadAllTwice");
    t.Start();
    size_t num_read(0), sz(10000);
    uint64_t pos(0);
    for (; pos < 10000; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    for (pos = 1, sz = 5000; pos < 5000; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    for (pos = 5000, sz = t.size - 5000;
        pos < t.size; pos += num_read, sz -= num_read)
    {
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    }
    t.Finish();
}

TEST_CASE(TimedReadAllTwicePartially) {
    Test t(this, "TimedReadAllTwicePartially");
    t.Start();
    size_t num_read(0), sz(20000);
    uint64_t pos(0);
    for (; pos < 20000; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    for (pos = 2000, sz = t.size - 2000;
        pos < t.size; pos += num_read, sz -= num_read)
    {
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    }
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileReadExactly

TEST_CASE(ReadExactly) {
    Test t(this, "ReadExactly");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size));
    t.Finish();
}

TEST_CASE(ReadExactlySkip0) {
    Test t(this, "ReadExactlySkip0");
    t.Start();
//  REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, t.size - 1));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(ReadExactlySkip1) {
    Test t(this, "ReadExactlySkip1");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
//  REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 2, t.buf, t.size - 2));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(ReadExactlySkipLast) {
    Test t(this, "ReadExactlySkipLast");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size - 1));
//  REQUIRE_RC(KFileReadExactly(t.file, t.size - 1, t.buf, 1));
    t.Finish(ERR_HEAD "31837 "
                   "of 31838.");
}

TEST_CASE(ReadExactlyTwice) {
    Test t(this, "ReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 20000));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 10000));
    REQUIRE_RC(KFileReadExactly(t.file, 20000, t.buf, t.size - 20000));
    t.Finish();
}

TEST_CASE(ReadExactlyTwicePartially) {
    Test t(this, "ReadExactlyTwicePartially");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 20000));
    REQUIRE_RC(KFileReadExactly(t.file, 10000, t.buf, t.size - 10000));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadExactly

TEST_CASE(TimedReadExactly) {
    Test t(this, "TimedReadExactly");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlySkip0) {
    Test t(this, "TimedReadExactlySkip0");
    t.Start();
//  REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, t.size - 1, &t.tm));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(TimedReadExactlySkip1) {
    Test t(this, "ReadExactlySkip1");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
//  REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 2, t.buf, t.size - 2, &t.tm));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(TimedReadExactlySkipLast) {
    Test t(this, "TimedReadExactlySkipLast");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size - 1, &t.tm));
//  REQUIRE_RC(KFileTimedReadExactly(t.file, t.size - 1, t.buf, 1, &t.tm));
    t.Finish(ERR_HEAD "31837 "
                   "of 31838.");
}

TEST_CASE(TimedReadExactlyTwice) {
    Test t(this, "TimedReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 20000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 10000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 20000,
                             t.buf, t.size - 20000, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlyTwicePartially) {
    Test t(this, "TimedReadExactlyTwicePartially");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 20000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 999, t.buf, t.size - 999, &t.tm));
    t.Finish();
}

extern "C" {
    int main(int argc, char *argv[]) {
        KConfigDisableUserSettings();

        return ReadObserverTestSuite(argc, argv);
    }
}

/******************************************************************************/
