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
* Tests of KStreamMD5ReadObserver for not-empty file
*/

#include "ObserverTest.hpp" // FilledObserverTest

TEST_SUITE(ObserverTestSuite)

TEST_CASE(testKStreamRead) {
    FilledObserverTest t(STDIN, this, "KStreamRead");

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    REQUIRE(rc == 0);

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 1);
    REQUIRE(t.buf[0] == '1');

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    REQUIRE(rc == 0);

    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamTimedRead) {
    struct timeout_t tm;
    TimeoutInit(&tm, 300000);
    struct timeout_t* pt(&tm);
    if (STDIN)  // StdIn KStream supports KStreamTimedRead without timeout
        pt = nullptr;
    
    FilledObserverTest t(STDIN, this, "KStreamTimedRead");

    size_t num_read(~0);
    REQUIRE_RC(KStreamTimedRead(t.str, t.buf, 1, &num_read, pt));
    REQUIRE(num_read == 1);

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, nullptr));

    REQUIRE_RC(KStreamTimedRead(t.str, t.buf, 1, &num_read, pt));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamReadAll) {
    FilledObserverTest t(STDIN, this, "KStreamReadAll");

    size_t num_read(~0);
    REQUIRE_RC(KStreamReadAll(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 1);

    t.Finish(false);
}

TEST_CASE(testKStreamReadAllComplete) {
    FilledObserverTest t(STDIN, this, "KStreamReadAllComplete");

    size_t num_read(~0);
    REQUIRE_RC(KStreamReadAll(t.str, t.buf, 2, &num_read));
    REQUIRE(num_read == 1);

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, nullptr));

    REQUIRE_RC(KStreamReadAll(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamTimedReadAll) {
    struct timeout_t tm;
    TimeoutInit(&tm, 300000);
    struct timeout_t* pt(&tm);
    if (STDIN) // StdIn KStream supports KStreamTimedReadAll without timeout
        pt = nullptr;

    FilledObserverTest t(STDIN, this, "KStreamTimedReadAll");

    size_t num_read(~0);
    REQUIRE_RC(KStreamTimedReadAll(t.str, t.buf, 1, &num_read, pt));
    REQUIRE(num_read == 1);

    t.Finish(false);
}

TEST_CASE(testKStreamTimedReadAllComplete) {
    struct timeout_t tm;
    TimeoutInit(&tm, 300000);
    struct timeout_t* pt(&tm);
    if (STDIN) // StdIn KStream supports KStreamTimedReadAll without timeout
        pt = nullptr;

    FilledObserverTest t(STDIN, this, "KStreamTimedReadAllComplete");

    size_t num_read(~0);
    REQUIRE_RC(KStreamTimedReadAll(t.str, t.buf, 2, &num_read, pt));
    REQUIRE(num_read == 1);

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, nullptr));

    REQUIRE_RC(KStreamTimedReadAll(t.str, t.buf, 1, &num_read, pt));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamReadExactly) {
    FilledObserverTest t(STDIN, this, "KStreamReadExactly");

    REQUIRE_RC(KStreamReadExactly(t.str, t.buf, 1));

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, nullptr));

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamReadExactlyComplete) {
    FilledObserverTest t(STDIN, this, "KStreamReadExactlyComplete");

    REQUIRE_RC_FAIL(KStreamReadExactly(t.str, t.buf, 2));

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    rc_t e(SILENT_RC(rcNS, rcStream, rcReading, rcTransfer, rcIncomplete));
    REQUIRE_EQ(rc, e);

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish(false, e);
}

TEST_CASE(testKStreamTimedReadExactly) {
    struct timeout_t tm;
    TimeoutInit(&tm, 300000);
    struct timeout_t* pt(&tm);
    if (STDIN) // StdIn KStream supports KStreamTimedReadExactly without timeout
        pt = nullptr;

    FilledObserverTest t(STDIN, this, "KStreamTimedReadExactly");

    REQUIRE_RC(KStreamTimedReadExactly(t.str, t.buf, 1, pt));

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, nullptr));

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamTimedReadExactlyComplete) {
    struct timeout_t tm;
    TimeoutInit(&tm, 300000);
    struct timeout_t* pt(&tm);
    if (STDIN) // StdIn KStream supports KStreamTimedReadExactly without timeout
        pt = nullptr;

    FilledObserverTest t(STDIN, this, "KStreamTimedReadExactlyComplete");

    REQUIRE_RC_FAIL(KStreamTimedReadExactly(t.str, t.buf, 2, pt));

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    rc_t e(SILENT_RC(rcNS, rcStream, rcReading, rcTransfer, rcIncomplete));
    REQUIRE_EQ(rc, e);

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish(false, e);
}
