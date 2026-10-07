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
* Tests of atomic operations.
*
* #include <atomic64.h> // atomic_t
* call atomic64_() functions
*/

#include <kproc/thread.h> // KThread
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <atomic64.h> // atomic_t

TEST_SUITE(KAtomicTestSuite)

using namespace ncbi::NK;
using std::cout;

struct S {
    char c;

    S(char i = 0) : c(i + d) {}

    static const char d = '!';
};

class Fixture {
    static const int M = 9;
    KThread* r = nullptr;
    KThread* w = nullptr;

    // test of atomic reads to detect data races within the thread
    static rc_t rf(KThread const* const thread, void* data) noexcept {
        Fixture* f = (Fixture*)data;

        for (int i = 0; i < M * 2; ++i) {
            long l = atomic64_read(&f->_a);

            long ll(0);
            atomic64_get_var(&ll, &f->_l);

            double d(0);
            atomic64_get_var(&d, &f->_d);
            int ii(0);
            atomic32_get_var(&ii, &f->_i);

            cout << l << ll << ii << d;

            TestEnv::SleepMs(1);
        }

        return 0;
    }

    // thread test of atomic writes to detect data races
    static rc_t wf(KThread const* const thread, void* data) noexcept {
        Fixture* f = (Fixture*)data;

        for (int i = 0; i < M; ++i) {
            atomic64_set(&f->_a, i);
            atomic64_test_and_set(&f->_a, 1, i);
            atomic64_add(&f->_a, 2);
            atomic64_read_and_add(&f->_a, 1);
            atomic64_add_and_read(&f->_a, 1);
            atomic64_inc(&f->_a);
            atomic64_dec(&f->_a);
            atomic64_dec_and_test(&f->_a);
            atomic64_inc_and_test(&f->_a);

            long l = atomic64_test_and_inc(&f->_a);
            atomic64_set(&f->_a, i + l + 1);

            l = atomic64_add_if_lt(&f->_a, 1, 9);
            atomic64_read_and_add_lt(&f->_a, 1, l + 2);

            l = atomic64_add_if_le(&f->_a, 1, 9);
            atomic64_read_and_add_le(&f->_a, 1, l + 3);

            atomic64_read_and_add_eq(&f->_a, 1, 5);
            l = atomic64_add_if_eq(&f->_a, 1, 6);
            atomic64_read_and_add_ne(&f->_a, 1, 0);
            l = atomic64_add_if_ne(&f->_a, 1, 1);
            atomic64_read_and_add_ge(&f->_a, 1, 2);
            l = atomic64_add_if_ge(&f->_a, 1, 3);
            atomic64_read_and_add_gt(&f->_a, 1, 4);
            l = atomic64_add_if_gt(&f->_a, 1, 5);
            atomic64_read_and_add_odd(&f->_a, 1);
            atomic64_read_and_add_even(&f->_a, 1);

            l = f->_l;
            ++l;
            atomic64_set_var(&f->_l, &l);

            double d(f->_d);
            d += .1;
            atomic64_set_var(&f->_d, &d);
            atomic32_set_var(&f->_i, i);

            TestEnv::SleepMs(1);
        }

        return 0;
    }

protected:
    atomic64_t _a;
    int _i = 0;
    double _d = 0;
    long _l = 0;

    Fixture() {
        _a.counter = 0;
    }

    ~Fixture() {
        KThreadRelease(r);
        KThreadRelease(w);
    }

    rc_t go() {
        KThreadMake(&r, rf, reinterpret_cast<void*>(this));
        KThreadMake(&w, wf, reinterpret_cast<void*>(this));

        KThreadWait(r, nullptr);
        KThreadWait(w, nullptr);

        std::cout << "\n";

        return 0;
    }
};

// tests of atomic functions without threads
FIXTURE_TEST_CASE(Test, Fixture) {
    REQUIRE(atomic64_read(&_a) == 0);
    REQUIRE(_a.counter == 0);

    atomic64_set(&_a, -1);
    REQUIRE(_a.counter == -1);

    atomic64_add(&_a, 1);
    REQUIRE(_a.counter == 0);

    REQUIRE(atomic64_read_and_add(&_a, 1) == 0);
    REQUIRE(_a.counter == 1);

    REQUIRE(atomic64_add_and_read(&_a, 1) == 2);

    atomic64_inc(&_a);
    REQUIRE(_a.counter == 3);

    atomic64_dec(&_a);
    REQUIRE(_a.counter == 2);

    REQUIRE(!atomic64_dec_and_test(&_a)); // 1
    REQUIRE(atomic64_dec_and_test(&_a)); // 0

    atomic64_set(&_a, 3); // 3
    REQUIRE(_a.counter == 3);

    REQUIRE(atomic64_read_and_add_lt(&_a, 3, 4) == 3); // 6
    REQUIRE(_a.counter == 6);
    REQUIRE(atomic64_read_and_add_lt(&_a, 3, 4) == 6); // 6
    REQUIRE(_a.counter == 6);
    REQUIRE(!atomic64_add_if_lt(&_a, 3, 4));
    REQUIRE(_a.counter == 6);
    REQUIRE(atomic64_add_if_lt(&_a, 3, 7));
    REQUIRE(_a.counter == 9);

    REQUIRE(atomic64_read_and_add_le(&_a, 1, 8) == 9); // 9
    REQUIRE(_a.counter == 9);
    REQUIRE(atomic64_read_and_add_le(&_a, 1, 9) == 9); // 10
    REQUIRE(_a.counter == 10);
    REQUIRE(!atomic64_add_if_le(&_a, 1, 9));
    REQUIRE(_a.counter == 10);
    REQUIRE(atomic64_add_if_le(&_a, 1, 10));
    REQUIRE(_a.counter == 11);

    REQUIRE(atomic64_read_and_add_eq(&_a, 1, 10) == 11);
    REQUIRE(_a.counter == 11);
    REQUIRE(atomic64_read_and_add_eq(&_a, 1, 11) == 11);
    REQUIRE(_a.counter == 12);
    REQUIRE(!atomic64_add_if_eq(&_a, 1, 10));
    REQUIRE(_a.counter == 12);
    REQUIRE(atomic64_add_if_eq(&_a, 1, 12));
    REQUIRE(_a.counter == 13);

    REQUIRE(atomic64_read_and_add_ne(&_a, 1, 11) == 13);
    REQUIRE(_a.counter == 14);
    REQUIRE(atomic64_read_and_add_ne(&_a, 1, 14) == 14);
    REQUIRE(_a.counter == 14);
    REQUIRE(atomic64_add_if_ne(&_a, 1, 10));
    REQUIRE(_a.counter == 15);
    REQUIRE(!atomic64_add_if_ne(&_a, 1, 15));
    REQUIRE(_a.counter == 15);

    REQUIRE(atomic64_read_and_add_ge(&_a, 1, 16) == 15);
    REQUIRE(_a.counter == 15);
    REQUIRE(atomic64_read_and_add_ge(&_a, 1, 15) == 15);
    REQUIRE(_a.counter == 16);
    REQUIRE(atomic64_add_if_ge(&_a, 1, 16));
    REQUIRE(_a.counter == 17);
    REQUIRE(!atomic64_add_if_ge(&_a, 1, 19));
    REQUIRE(_a.counter == 17);

    REQUIRE(atomic64_read_and_add_gt(&_a, 1, 17) == 17);
    REQUIRE(_a.counter == 17);
    REQUIRE(atomic64_read_and_add_gt(&_a, 1, 16) == 17);
    REQUIRE(_a.counter == 18);
    REQUIRE(atomic64_add_if_gt(&_a, 1, 16));
    REQUIRE(_a.counter == 19);
    REQUIRE(!atomic64_add_if_gt(&_a, 1, 19));
    REQUIRE(_a.counter == 19);

    REQUIRE(atomic64_read_and_add_odd(&_a, 1) == 19);
    REQUIRE(_a.counter == 20);
    REQUIRE(atomic64_read_and_add_odd(&_a, 1) == 20);
    REQUIRE(_a.counter == 20);

    REQUIRE(atomic64_read_and_add_even(&_a, 1) == 20);
    REQUIRE(_a.counter == 21);
    REQUIRE(atomic64_read_and_add_even(&_a, 1) == 21);
    REQUIRE(_a.counter == 21);

    REQUIRE(_l == 0);
    long nl(9);
    atomic64_get_var(&nl, &_l);
    REQUIRE(nl == 0);

    nl = 42;
    atomic64_set_var(&_l, &nl);
    REQUIRE(_l == 42);

    nl = 0;
    atomic64_get_var(&nl, &_l);
    REQUIRE(nl == 42);

    REQUIRE(_d == 0);
    double nd(9);
    atomic64_get_var(&nd, &_d);
    REQUIRE(nd == 0);

    nd = 3.14159;
    atomic64_set_var(&_d, &nd);
    REQUIRE(_d == 3.14159);
    REQUIRE(nd == 3.14159);

    nd = 0;
    atomic64_get_var(&nd, &_d);
    REQUIRE(nd == 3.14159);

    REQUIRE(_i == 0);
    atomic32_set_var(&_i, 1);
    REQUIRE(_i == 1);

    int i(2);
    atomic32_get_var(&i, &_i);
    REQUIRE(i == 1);

    REQUIRE_RC(go());
}

int main(int argc, char* argv[]) { return KAtomicTestSuite(argc, argv); }
