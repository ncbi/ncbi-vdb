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
* #include <atomic.h> // atomic_t
* call atomic32_() functions
*/

#include <kproc/thread.h> // KThread
#include <ktst/unit_test.hpp> // TEST_SUITE
#include <atomic.h> // atomic_t

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
        S* p = 0;
        Fixture* f = (Fixture*)data;

        for (int i = 0; i < M * 2; ++i) {
            long l = atomic32_read(&f->_a);

            for (p = 0; !p;)
                p = reinterpret_cast<S*>(atomic_read_ptr(&f->_p));
            cout << p->c;

            atomic_set_ptr(&f->_p, 0);
            delete p;

            float d(0);
            atomic32_get_var(&d, &f->_f);

            int ii(0);
            atomic32_get_var(&ii, &f->_i);

            cout << l << ii << d;

            TestEnv::SleepMs(1);
        }

        p = reinterpret_cast<S*>(atomic_read_ptr(&f->_p));
        delete p;

        return 0;
    }

    // thread test of atomic writes to detect data races
    static rc_t wf(KThread const* const thread, void* data) noexcept {
        Fixture* f = (Fixture*)data;

        for (int i = 0; i < M; ++i) {
            atomic32_set(&f->_a, i);
            atomic32_test_and_set(&f->_a, 1, i);
            atomic32_add(&f->_a, 2);
            atomic32_read_and_add(&f->_a, 1);
            atomic32_add_and_read(&f->_a, 1);
            atomic32_inc(&f->_a);
            atomic32_dec(&f->_a);
            atomic32_dec_and_test(&f->_a);
            atomic32_inc_and_test(&f->_a);

            long l = atomic32_test_and_inc(&f->_a);
            atomic32_set(&f->_a, i + l + 1);

            l = atomic32_add_if_lt(&f->_a, 1, 9);
            atomic32_read_and_add_lt(&f->_a, 1, l + 2);

            l = atomic32_add_if_le(&f->_a, 1, 9);
            atomic32_read_and_add_le(&f->_a, 1, l + 3);

            atomic32_read_and_add_eq(&f->_a, 1, 5);
            l = atomic32_add_if_eq(&f->_a, 1, 6);
            atomic32_read_and_add_ne(&f->_a, 1, 0);
            l = atomic32_add_if_ne(&f->_a, 1, 1);
            atomic32_read_and_add_ge(&f->_a, 1, 2);
            l = atomic32_add_if_ge(&f->_a, 1, 3);
            atomic32_read_and_add_gt(&f->_a, 1, 4);
            l = atomic32_add_if_gt(&f->_a, 1, 5);
            atomic32_read_and_add_odd(&f->_a, 1);
            atomic32_read_and_add_even(&f->_a, 1);

            /* N.B. - THESE FUNCTIONS ARE FOR 64 BIT PTRS ONLY */
            S* p = 0;
            do {
                p = reinterpret_cast<S*>(atomic_read_ptr(&f->_p));
            } while (p);

            p = new S(i);
            atomic_set_ptr(&f->_p, p);

            p = new S(i + 9);
            for (void* o = p; o;)
                o = atomic_test_and_set_ptr(&f->_p, p, 0);

            atomic32_set_var(&f->_i, i);

            TestEnv::SleepMs(1);
        }

        return 0;
    }

protected:
    atomic32_t _a;
    int _i = 0;
    float _f = 0;
    long _l = 0;
    atomic_ptr_t _p;

    Fixture() {
        _a.counter = 0;
        _p.ptr = 0;
    }

    ~Fixture() {
        KThreadRelease(r);
        KThreadRelease(w);

        delete reinterpret_cast<S*>(_p.ptr);
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
    REQUIRE(atomic32_read(&_a) == 0);
    REQUIRE(_a.counter == 0);

    atomic32_set(&_a, -1);
    REQUIRE(_a.counter == -1);

    atomic32_add(&_a, 1);
    REQUIRE(_a.counter == 0);

    REQUIRE(atomic32_read_and_add(&_a, 1) == 0);
    REQUIRE(_a.counter == 1);

    REQUIRE(atomic32_add_and_read(&_a, 1) == 2);

    atomic32_inc(&_a);
    REQUIRE(_a.counter == 3);

    atomic32_dec(&_a);
    REQUIRE(_a.counter == 2);

    REQUIRE(!atomic32_dec_and_test(&_a)); // 1
    REQUIRE(atomic32_dec_and_test(&_a)); // 0

    atomic32_set(&_a, -2);
    REQUIRE(!atomic32_inc_and_test(&_a)); // -1
    REQUIRE(atomic32_inc_and_test(&_a)); // 0
    REQUIRE(atomic32_test_and_inc(&_a)); // 1

    REQUIRE(atomic32_test_and_set(&_a, 2, 2) == 1); // 1
    REQUIRE(_a.counter == 1);

    REQUIRE(atomic32_test_and_set(&_a, 3, 1) == 1); // 3
    REQUIRE(_a.counter == 3);

    REQUIRE(atomic32_read_and_add_lt(&_a, 3, 4) == 3); // 6
    REQUIRE(_a.counter == 6);
    REQUIRE(atomic32_read_and_add_lt(&_a, 3, 4) == 6); // 6
    REQUIRE(_a.counter == 6);
    REQUIRE(!atomic32_add_if_lt(&_a, 3, 4));
    REQUIRE(_a.counter == 6);
    REQUIRE(atomic32_add_if_lt(&_a, 3, 7));
    REQUIRE(_a.counter == 9);

    REQUIRE(atomic32_read_and_add_le(&_a, 1, 8) == 9); // 9
    REQUIRE(_a.counter == 9);
    REQUIRE(atomic32_read_and_add_le(&_a, 1, 9) == 9); // 10
    REQUIRE(_a.counter == 10);
    REQUIRE(!atomic32_add_if_le(&_a, 1, 9));
    REQUIRE(_a.counter == 10);
    REQUIRE(atomic32_add_if_le(&_a, 1, 10));
    REQUIRE(_a.counter == 11);

    REQUIRE(atomic32_read_and_add_eq(&_a, 1, 10) == 11);
    REQUIRE(_a.counter == 11);
    REQUIRE(atomic32_read_and_add_eq(&_a, 1, 11) == 11);
    REQUIRE(_a.counter == 12);
    REQUIRE(!atomic32_add_if_eq(&_a, 1, 10));
    REQUIRE(_a.counter == 12);
    REQUIRE(atomic32_add_if_eq(&_a, 1, 12));
    REQUIRE(_a.counter == 13);

    REQUIRE(atomic32_read_and_add_ne(&_a, 1, 11) == 13);
    REQUIRE(_a.counter == 14);
    REQUIRE(atomic32_read_and_add_ne(&_a, 1, 14) == 14);
    REQUIRE(_a.counter == 14);
    REQUIRE(atomic32_add_if_ne(&_a, 1, 10));
    REQUIRE(_a.counter == 15);
    REQUIRE(!atomic32_add_if_ne(&_a, 1, 15));
    REQUIRE(_a.counter == 15);

    REQUIRE(atomic32_read_and_add_ge(&_a, 1, 16) == 15);
    REQUIRE(_a.counter == 15);
    REQUIRE(atomic32_read_and_add_ge(&_a, 1, 15) == 15);
    REQUIRE(_a.counter == 16);
    REQUIRE(atomic32_add_if_ge(&_a, 1, 16));
    REQUIRE(_a.counter == 17);
    REQUIRE(!atomic32_add_if_ge(&_a, 1, 19));
    REQUIRE(_a.counter == 17);

    REQUIRE(atomic32_read_and_add_gt(&_a, 1, 17) == 17);
    REQUIRE(_a.counter == 17);
    REQUIRE(atomic32_read_and_add_gt(&_a, 1, 16) == 17);
    REQUIRE(_a.counter == 18);
    REQUIRE(atomic32_add_if_gt(&_a, 1, 16));
    REQUIRE(_a.counter == 19);
    REQUIRE(!atomic32_add_if_gt(&_a, 1, 19));
    REQUIRE(_a.counter == 19);

    REQUIRE(atomic32_read_and_add_odd(&_a, 1) == 19);
    REQUIRE(_a.counter == 20);
    REQUIRE(atomic32_read_and_add_odd(&_a, 1) == 20);
    REQUIRE(_a.counter == 20);

    REQUIRE(atomic32_read_and_add_even(&_a, 1) == 20);
    REQUIRE(_a.counter == 21);
    REQUIRE(atomic32_read_and_add_even(&_a, 1) == 21);
    REQUIRE(_a.counter == 21);

    REQUIRE_NULL(_p.ptr);

    /* N.B. - THESE FUNCTIONS ARE FOR 64 BIT PTRS ONLY */
    S* s(reinterpret_cast <S*>(atomic_read_ptr(&_p)));
    REQUIRE_NULL(s);

    atomic_set_ptr(&_p, new S);
    REQUIRE_NOT_NULL(_p.ptr);
    REQUIRE(((S*)(_p.ptr))->c == '!');

    s = reinterpret_cast<S*>(atomic_read_ptr(&_p));
    REQUIRE(s->c == S::d);

    atomic_set_ptr(&_p, 0);
    REQUIRE_NULL(_p.ptr);

    ++s->c;
    REQUIRE_NULL(atomic_test_and_set_ptr(&_p, s, 0));
    REQUIRE_NOT_NULL(_p.ptr);

    REQUIRE(atomic_test_and_set_ptr(&_p, 0, s) == s);
    REQUIRE_NULL(_p.ptr);

    delete s;

    REQUIRE(_i == 0);
    atomic32_set_var(&_i, 1);
    REQUIRE(_i == 1);

    int i(2);
    atomic32_get_var(&i, &_i);
    REQUIRE(i == 1);

    REQUIRE_RC(go());
}

int main(int argc, char* argv[]) { return KAtomicTestSuite(argc, argv); }
