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
* Common code for KStreamMD5ReadObserver tests
*/

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */
#include <kfs/md5.h> /* KStreamMD5ReadObserverRelease */
#include <kns/stream.h> /* KStreamRelease */
#include <kproc/timeout.h> // TimeoutInit
#include <ktst/unit_test.hpp> /* TestCase */
#include <klib/rc.h> // IF_EXITCODE
#include "../../libs/kns/stream-priv.h" /* rcStream */

static rc_t correctKStreamMakeFromBuffer(
    const KStream** stream, const char* buffer, size_t size)
{   return KStreamMakeFromBuffer((KStream**)stream, buffer, size); }

class ObserverTest : protected ncbi::NK::TestCase {
    TestCase* _dad;
    bool _failures;
    bool _stdin;
    bool _empty;
    KDirectory* _d = nullptr;
    char const* _path = "tmp";

public:
    const KStream* str = nullptr;
    const KStreamMD5ReadObserver* md5 = nullptr;
    char buf[99] = "1";
    uint8_t digest[16];

protected:
    ObserverTest(bool aStdin, bool empty,
        TestCase* dad, const std::string& name, bool failures)
        : TestCase(name)
        , _dad(dad)
        , _failures(failures)
        , _stdin(aStdin)
        , _empty(empty)
    {
        digest[0] = '\0';

        if (_failures)
            REQUIRE_RC_FAIL(KStreamMakeFromBuffer(nullptr, buf, 1));

        if (_stdin) {
            REQUIRE_RC(KDirectoryNativeDir(&_d));
            REQUIRE_RC(KDirectoryRemove(_d, true, _path));
            KFile* f(nullptr);
            REQUIRE_RC(
                KDirectoryCreateFile(_d, &f, false, 0400, kcmInit, _path));
            if (!_empty)
                REQUIRE_RC(KFileWrite(f, 0, buf, 1, NULL));
            REQUIRE_RC(KFileRelease(f));
            REQUIRE_NOT_NULL(freopen(_path, "r", stdin));
            REQUIRE_RC(KStreamMakeStdIn(&str));
        }
        else {
            if (_empty)
                REQUIRE_RC(correctKStreamMakeFromBuffer(&str, nullptr, 0));
            else
                REQUIRE_RC(correctKStreamMakeFromBuffer(&str, buf, 1));
        }

        if (_failures) {
            REQUIRE_RC_FAIL(KStreamMakeMD5ReadObserver(str, nullptr));
            REQUIRE_RC_FAIL(KStreamMakeMD5ReadObserver(nullptr, &md5));
        }

        REQUIRE_RC(KStreamMakeMD5ReadObserver(str, &md5));

        if (_failures) {
            size_t num_read(~0);
            REQUIRE_RC_FAIL(KStreamRead(nullptr, buf, 1, &num_read));
            REQUIRE_RC_FAIL(KStreamRead(str, nullptr, 1, &num_read));
            REQUIRE_RC_FAIL(KStreamRead(str, buf, 0, &num_read));
            REQUIRE_RC_FAIL(KStreamRead(str, buf, 1, nullptr));
        }
    }

    ~ObserverTest() {
        assert(_dad);
        _dad->ErrorCounterAdd(GetErrorCounter());

        if (_stdin) {
            KDirectoryRemove(_d, true, _path);

            KDirectoryRelease(_d);
            _d = nullptr;
        }
    }

public:
    void Finish(bool success = true, rc_t aRc = 0) {
        if (_failures) {
            REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(nullptr, digest,
                nullptr));
            REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(md5, nullptr,
                nullptr));
        }

        if (success) {
            REQUIRE_RC(KStreamMD5ReadObserverGetDigest(md5, digest, nullptr));
            REQUIRE(digest[0] != '\0');

            int total = 0;
            for (int i = 0; i < 16; ++i) {
                int len = snprintf(&buf[total], sizeof buf - total, "%02x",
                    digest[i]);
                assert(len == 2);
                total += len;
            }
            buf[total] = '\0';

            std::string e("c4ca4238a0b923820dcc509a6f75849b");
            if (_empty)
                e = "d41d8cd98f00b204e9800998ecf8427e";

            REQUIRE_EQ(std::string(buf), e);
        }
        else {
            REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(md5, digest,
                nullptr));

            rc_t rc = ~0;
            REQUIRE_RC_FAIL(KStreamMD5ReadObserverGetDigest(md5, digest, &rc));
            REQUIRE_EQ(rc, aRc);
        }

        REQUIRE_RC(KStreamMD5ReadObserverRelease(md5));
        md5 = nullptr;

        REQUIRE_RC(KStreamRelease(str));
        str = nullptr;
    }
};

class EmptyObserverTest : public ObserverTest {
public:
    EmptyObserverTest
    (bool stdin, TestCase* dad, const std::string& name, bool failures = false)
        : ObserverTest(stdin, true, dad, name, failures)
    {}
};

class FilledObserverTest : public ObserverTest {
public:
    FilledObserverTest
    (bool stdin, TestCase* dad, const std::string& name, bool failures = false)
        : ObserverTest(stdin, false, dad, name, failures)
    {}
};
