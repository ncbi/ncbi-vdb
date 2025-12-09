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
* Tests of KFileMD5ReadObserver for bz2 file
*/

#include "../vfs/observer-test.hpp" // ObserverTest

TEST_SUITE(ReadObserverTestSuite)

using std::string;

////////////////////////////////////////////////////////////////////////////////
// KFileReadAll

TEST_CASE(ReadAllExactly) {
    ObserverTest t(this, "ReadAllExactly");
    t.Start(true);
    size_t num_read(0);
    // request exactly all file; EOF cannot be detected by file size
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, t.size, &num_read));
    REQUIRE(num_read == t.size);
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
    ObserverTest t(this, "ReadAllExactlyPlus1");
    t.Start();
    size_t num_read(0);
    // request all file + 1 byte
    // will read one byte less than requested;
    // EOF will be detected by last read that returns 0 bytes
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, t.size + 1, &num_read));
    t.Finish();
}

TEST_CASE(ReadAllSkip0) {
    ObserverTest t(this, "ReadAllSkip0");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 1, t.buf, t.size, &num_read));
    // the first byte was not read
    t.Finish(ERR_HEAD "0.", true);
}

TEST_CASE(ReadAllSkip1) {
    ObserverTest t(this, "ReadAllSkip1");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, 1, &num_read));
//  REQUIRE_RC(KFileReadAll(t.file, 1, t.buf, 1, &num_read));
    REQUIRE_RC(KFileReadAll(t.file, 2, t.buf, t.size, &num_read));
    // the second byte was not read
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(ReadAllSkipLast) {
    ObserverTest t(this, "ReadAllSkipLast");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, t.size - 1, &num_read));
//  REQUIRE_RC(KFileReadAll(t.file, t.size - 1, t.buf, 2, &num_read));
    // the last byte was not read
    t.Finish(ERR_HEAD "12887838.");
}

// bzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadAllTwice) {
    ObserverTest t(this, "ReadAllTwice");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC(KFileReadAll(t.file, 0, t.buf, 1234567, &num_read));
    // this call will request the part that was already completely read
    REQUIRE_RC_FAIL(KFileReadAll(t.file, 1, t.buf, 123456, &num_read));
    t.Finish(ERR_HEAD "1234567.", true);
}

// bzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadAllTwicePartially) {
    ObserverTest t(this, "ReadAllTwicePartially");
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
    ObserverTest t(this, "Read");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 2, &num_read));
    t.Finish();
}

TEST_CASE(ReadSkip0) {
    ObserverTest t(this, "ReadSkip0");
    t.Start();
    size_t num_read(0);
    uint64_t pos(1);
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 2, &num_read));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(ReadSkip1) {
    ObserverTest t(this, "ReadSkip1");
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

// bzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadTwice) {
    ObserverTest t(this, "ReadTwice");
    t.Start();
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0     ; num_read > 0 && pos < 1234567; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 100000, &num_read));
    for (size_t n = 0; n > 0; )
    {   REQUIRE_RC_FAIL(KFileRead(t.file, 1, t.buf, 10000, &n)); break; }
    for (            ; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, 100000, &num_read));
    t.Finish();
}

// bzip KFile does not allow re-reading the part that was previously read
TEST_CASE(ReadTwicePartially) {
    ObserverTest t(this, "ReadTwicePartially");
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
// KFileTimedRead is not supported for bzip KFile

TEST_CASE(TimedRead) {
    ObserverTest t(this, "TimedRead");
    t.Start(true);
    size_t num_read(1);
    REQUIRE_RC_FAIL(KFileTimedRead(t.file, 0, t.buf, 99, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0.", true);
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadAll is not supported for bzip KFile

TEST_CASE(TimedReadAllExactly) {
    ObserverTest t(this, "TimedReadAllExactly");
    t.Start();
    size_t num_read(0);
    REQUIRE_RC_FAIL(KFileTimedReadAll(t.file, 0, t.buf, 999, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0.");
}

////////////////////////////////////////////////////////////////////////////////
// KFileReadExactly

TEST_CASE(ReadExactly) {
    ObserverTest t(this, "ReadExactly");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size));
    // EOF was not detected by file size
    REQUIRE_RC_FAIL(KFileMD5ReadObserverGetDigest(t.md5, t.digest, &t.error));
    REQUIRE_EQ(string(t.error), string(ERR_HEAD "12887839."));
    FREE(t.error);
    size_t num_read(0);
    REQUIRE_RC(KFileRead(t.file, t.size, t.buf, 1, &num_read));
    t.Finish("", true);
}

#if 0
TEST_CASE(ReadExactlySkip0) {
    ObserverTest t(this, "ReadExactlySkip0");
    t.Start(true);
    //REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, t.size - 1));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(ReadExactlySkip1) {
    ObserverTest t(this, "ReadExactlySkip1");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
    //REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 2, t.buf, t.size - 2));
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(ReadExactlySkipLast) {
    ObserverTest t(this, "ReadExactlySkipLast");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size - 1));
    //REQUIRE_RC(KFileReadExactly(t.file, t.size - 1, t.buf, 1));
    t.Finish(ERR_HEAD "12887838.");
}

TEST_CASE(ReadExactlyTwice) {
    ObserverTest t(this, "ReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1234567));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 123456));
    REQUIRE_RC(KFileReadExactly(t.file, 1234567, t.buf, t.size - 1234567));
    t.Finish();
}

TEST_CASE(ReadExactlyTwicePartially) {
    ObserverTest t(this, "ReadExactlyTwicePartially");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 2000000));
    REQUIRE_RC(KFileReadExactly(t.file, 1000000, t.buf, t.size - 1000000));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadExactly

TEST_CASE(TimedReadExactly) {
    ObserverTest t(this, "TimedReadExactly");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlySkip0) {
    ObserverTest t(this, "TimedReadExactlySkip0");
    t.Start(true);
    //REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, t.size - 1, &t.tm));
    t.Finish(ERR_HEAD "0.");
}

TEST_CASE(TimedReadExactlySkip1) {
    ObserverTest t(this, "ReadExactlySkip1");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
    //REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 2, t.buf, t.size - 2, &t.tm));
    t.Finish(ERR_HEAD "1.");
}

TEST_CASE(TimedReadExactlySkipLast) {
    ObserverTest t(this, "TimedReadExactlySkipLast");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size - 1, &t.tm));
    //REQUIRE_RC(KFileTimedReadExactly(t.file, t.size - 1, t.buf, 1, &t.tm));
    t.Finish(ERR_HEAD "12887838.");
}

TEST_CASE(TimedReadExactlyTwice) {
    ObserverTest t(this, "TimedReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1234567, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 123456, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1234567,
                             t.buf, t.size - 1234567, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlyTwicePartially) {
    ObserverTest t(this, "TimedReadExactlyTwicePartially");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 2000000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 999, t.buf, t.size - 999, &t.tm));
    t.Finish();
}
#endif

int main(int argc, char *argv[])
{
    rc_t rc(ObserverTest::Begin(eBz2));

    if (rc == 0)
        rc = ReadObserverTestSuite(argc, argv);

    rc_t r2(ObserverTest::End());
    if (rc == 0 && r2 != 0)
        rc = r2;

    return (rc == 0) ? 0 : IF_EXITCODE(rc, 3);
}

/******************************************************************************/
