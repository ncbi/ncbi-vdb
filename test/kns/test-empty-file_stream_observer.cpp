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
* Tests of KStreamMD5ReadObserver for empty file
*/

#include "ObserverTest.hpp" // EmptyObserverTest

TEST_SUITE(ReadObserverTestSuite)

TEST_CASE(testNotRead) {
    EmptyObserverTest t(this, "NotRead", true);

    t.Finish(false);
}

TEST_CASE(testKStreamRead) {
    EmptyObserverTest t(this, "KStreamRead");

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamTimedRead) {
    EmptyObserverTest t(this, "KStreamTimedRead");

    size_t num_read(~0);
    REQUIRE_RC(KStreamTimedRead(t.str, t.buf, 1, &num_read, &t.tm));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamReadAll) {
    EmptyObserverTest t(this, "KStreamReadAll");

    size_t num_read(~0);
    REQUIRE_RC(KStreamReadAll(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamTimedReadAll) {
    EmptyObserverTest t(this, "KStreamTimedReadAll");

    size_t num_read(~0);
    REQUIRE_RC(KStreamTimedReadAll(t.str, t.buf, 1, &num_read, &t.tm));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamReadExactly) {
    EmptyObserverTest t(this, "KStreamReadExactly");

    REQUIRE_RC_FAIL(KStreamReadExactly(nullptr, t.buf, 1));
    REQUIRE_RC_FAIL(KStreamReadExactly(t.str, nullptr, 1));

    REQUIRE_RC(KStreamReadExactly(t.str, t.buf, 0));

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    REQUIRE(rc == 0);

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamReadExactlyCompleted) {
    EmptyObserverTest t(this, "KStreamReadExactlyCompleted");

    REQUIRE_RC_FAIL(KStreamReadExactly(nullptr, t.buf, 1));

    REQUIRE_RC_FAIL(KStreamReadExactly(t.str, nullptr, 1));

    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, nullptr));

    REQUIRE_RC(KStreamReadExactly(t.str, t.buf, 0));

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    REQUIRE(rc == 0);

    REQUIRE_RC_FAIL(KStreamReadExactly(t.str, t.buf, 1));

    t.Finish(false,
        SILENT_RC(rcNS, rcStream, rcReading, rcTransfer, rcIncomplete));
}

TEST_CASE(testKStreamTimedReadExactly) {
    EmptyObserverTest t(this, "KStreamTimedReadExactly");

    REQUIRE_RC_FAIL(KStreamTimedReadExactly(nullptr, t.buf, 1, &t.tm));
    REQUIRE_RC_FAIL(KStreamTimedReadExactly(t.str, nullptr, 1, &t.tm));
    REQUIRE_RC_FAIL(KStreamTimedReadExactly(t.str, t.buf, 0, &t.tm));

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    REQUIRE(rc == 0);

    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish();
}

TEST_CASE(testKStreamTimedReadExactlyCompleted) {
    EmptyObserverTest t(this, "KStreamTimedReadExactlyCompleted");

    REQUIRE_RC_FAIL(KStreamTimedReadExactly(nullptr, t.buf, 1, &t.tm));
    REQUIRE_RC_FAIL(KStreamTimedReadExactly(t.str, nullptr, 1, &t.tm));
    REQUIRE_RC_FAIL(KStreamTimedReadExactly(t.str, t.buf, 0, &t.tm));

    rc_t rc(1);
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    REQUIRE(rc == 0);

    REQUIRE_RC_FAIL(KStreamTimedReadExactly(t.str, t.buf, 1, &t.tm));
    REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(t.md5, t.digest, &rc));
    rc_t e(SILENT_RC(rcNS, rcStream, rcReading, rcTransfer, rcIncomplete));
    REQUIRE_EQ(rc, e);
   
    size_t num_read(~0);
    REQUIRE_RC(KStreamRead(t.str, t.buf, 1, &num_read));
    REQUIRE(num_read == 0);

    t.Finish(false, e);
}

int main(int argc, char* argv[]) {
    rc_t rc(ReadObserverTestSuite(argc, argv));
    return (rc == 0) ? 0 : IF_EXITCODE(rc, 3);
}
