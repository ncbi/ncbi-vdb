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

#include "file-observer-test.hpp" // ObserverTest

TEST_SUITE(ReadObserverTestSuite)

using std::string;

////////////////////////////////////////////////////////////////////////////////
// KFileReadAll

TEST_CASE(ReadAllExactly) {
    ObserverTest t(this, "ReadAllExactly");
    t.Start(true);
    size_t num_read(0), sz(t.size);
    uint64_t pos(0);
    // request exactly all file; EOF is detected by file size
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    t.Finish("", true);
}

TEST_CASE(ReadAllExactlyPlus1) {
    ObserverTest t(this, "ReadAllExactlyPlus1");
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
    ObserverTest t(this, "ReadAllSkip0");
    t.Start(true);
    size_t num_read(0), sz(t.size);
    uint64_t pos(1);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileReadAll(t.file, pos, t.buf, sz, &num_read));
    // the first byte was not read
    t.Finish(ERR_HEAD "0 of 31838.", true);
}

TEST_CASE(ReadAllSkip1) {
    ObserverTest t(this, "ReadAllSkip1");
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
    ObserverTest t(this, "ReadAllSkipLast");
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
    ObserverTest t(this, "ReadAllTwice");
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
    ObserverTest t(this, "ReadAllTwicePartially");
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
//  REQUIRE_RC(KFileRead(t.file, 0, t.buf, 1, &num_read));
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileRead(t.file, pos, t.buf, t.size / 2, &num_read));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(ReadSkip1) {
    ObserverTest t(this, "ReadSkip1");
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
    ObserverTest t(this, "ReadTwice");
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
    ObserverTest t(this, "ReadTwicePartially");
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
    ObserverTest t(this, "TimedRead");
    t.Start(true);
    size_t num_read(1);
    uint64_t pos(0);
    for (pos = 0; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 10000, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadSkip0) {
    ObserverTest t(this, "TimedReadSkip0");
    t.Start(true);
    size_t num_read(0);
    uint64_t pos(1);
    //REQUIRE_RC(KFileRead(t.file, 0, t.buf, 1, &num_read));
    for (pos = 1; num_read > 0; pos += num_read)
        REQUIRE_RC(KFileTimedRead(t.file, pos, t.buf, 10000, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(TimedReadSkip1) {
    ObserverTest t(this, "TimedReadSkip1");
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
    ObserverTest t(this, "TimedReadTwice");
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
    ObserverTest t(this, "TimedReadTwicePartially");
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
    ObserverTest t(this, "TimedReadAllExactly");
    t.Start();
    size_t num_read(0), sz(t.size);
    uint64_t pos(0);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllExactlyPlus1) {
    ObserverTest t(this, "TimedReadAllExactlyPlus1");
    t.Start();
    size_t num_read(0), sz(t.size + 1);
    uint64_t pos(0);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadAllSkip0) {
    ObserverTest t(this, "TimedReadAllSkip0");
    t.Start(true);
    size_t num_read(0), sz(t.size);
    //REQUIRE_RC(KFileTimedReadAll(t.file, 0, t.buf, 1, &num_read, &t.tm));
    uint64_t pos(1);
    for (; pos < t.size; pos += num_read, sz -= num_read)
        REQUIRE_RC(KFileTimedReadAll(t.file, pos, t.buf, sz, &num_read, &t.tm));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(TimedReadAllSkip1) {
    ObserverTest t(this, "TimedReadAllSkip1");
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
    ObserverTest t(this, "TimedReadAllSkipLast");
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
    ObserverTest t(this, "TimedReadAllTwice");
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
    ObserverTest t(this, "TimedReadAllTwicePartially");
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
    ObserverTest t(this, "ReadExactly");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size));
    t.Finish();
}

TEST_CASE(ReadExactlySkip0) {
    ObserverTest t(this, "ReadExactlySkip0");
    t.Start();
//  REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, t.size - 1));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(ReadExactlySkip1) {
    ObserverTest t(this, "ReadExactlySkip1");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 1));
//  REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 1));
    REQUIRE_RC(KFileReadExactly(t.file, 2, t.buf, t.size - 2));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(ReadExactlySkipLast) {
    ObserverTest t(this, "ReadExactlySkipLast");
    t.Start();
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, t.size - 1));
//  REQUIRE_RC(KFileReadExactly(t.file, t.size - 1, t.buf, 1));
    t.Finish(ERR_HEAD "31837 "
                   "of 31838.");
}

TEST_CASE(ReadExactlyTwice) {
    ObserverTest t(this, "ReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 20000));
    REQUIRE_RC(KFileReadExactly(t.file, 1, t.buf, 10000));
    REQUIRE_RC(KFileReadExactly(t.file, 20000, t.buf, t.size - 20000));
    t.Finish();
}

TEST_CASE(ReadExactlyTwicePartially) {
    ObserverTest t(this, "ReadExactlyTwicePartially");
    t.Start(true);
    REQUIRE_RC(KFileReadExactly(t.file, 0, t.buf, 20000));
    REQUIRE_RC(KFileReadExactly(t.file, 10000, t.buf, t.size - 10000));
    t.Finish();
}

////////////////////////////////////////////////////////////////////////////////
// KFileTimedReadExactly

TEST_CASE(TimedReadExactly) {
    ObserverTest t(this, "TimedReadExactly");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlySkip0) {
    ObserverTest t(this, "TimedReadExactlySkip0");
    t.Start();
//  REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, t.size - 1, &t.tm));
    t.Finish(ERR_HEAD "0 of 31838.");
}

TEST_CASE(TimedReadExactlySkip1) {
    ObserverTest t(this, "ReadExactlySkip1");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 1, &t.tm));
//  REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 1, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 2, t.buf, t.size - 2, &t.tm));
    t.Finish(ERR_HEAD "1 of 31838.");
}

TEST_CASE(TimedReadExactlySkipLast) {
    ObserverTest t(this, "TimedReadExactlySkipLast");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, t.size - 1, &t.tm));
//  REQUIRE_RC(KFileTimedReadExactly(t.file, t.size - 1, t.buf, 1, &t.tm));
    t.Finish(ERR_HEAD "31837 "
                   "of 31838.");
}

TEST_CASE(TimedReadExactlyTwice) {
    ObserverTest t(this, "TimedReadExactlyTwice");
    t.Start(true);
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 20000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 1, t.buf, 10000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 20000,
                             t.buf, t.size - 20000, &t.tm));
    t.Finish();
}

TEST_CASE(TimedReadExactlyTwicePartially) {
    ObserverTest t(this, "TimedReadExactlyTwicePartially");
    t.Start();
    REQUIRE_RC(KFileTimedReadExactly(t.file, 0, t.buf, 20000, &t.tm));
    REQUIRE_RC(KFileTimedReadExactly(t.file, 999, t.buf, t.size - 999, &t.tm));
    t.Finish();
}

int main(int argc, char *argv[])
{
    rc_t rc(ObserverTest::Begin(eHttp));

    if (rc == 0)
        rc = ReadObserverTestSuite(argc, argv);

    rc_t r2(ObserverTest::End());
    if (rc == 0 && r2 != 0)
        rc = r2;

    return (rc == 0) ? 0 : IF_EXITCODE(rc, 3);
}

/******************************************************************************/
