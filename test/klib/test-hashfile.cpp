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
* ===========================================================================
*
*/
/**
* Unit tests for hash files
*/

#include <ktst/unit_test.hpp>

#include <arch-impl.h>
#include <atomic.h>
#include <cstdlib>
#include <cstring>
#include <kfs/defs.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <klib/data-buffer.h>
#include <klib/hashfile.h>
#include <klib/hashtable.h>
#include <klib/log.h>
#include <klib/misc.h> /* is_user_admin() */
#include <klib/num-gen.h>
#include <klib/printf.h>
#include <klib/sort.h>
#include <klib/text.h>
#include <klib/time.h>
#include <klib/vector.h>
#include <kproc/lock.h>
#include <kproc/thread.h>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

#define RANDS_SIZE 2000000
static uint64_t RANDS[RANDS_SIZE];

static volatile bool KEEPRUNNING = true;
static atomic64_t FINDCOUNT;
static KDirectory* DIR = NULL;
static KFile* BACKING = NULL;
static KHashFile* HMAP = NULL;

/* Very fast RNG */
static __inline uint64_t fastrng(uint64_t* state0)
{
    *state0 += 3800322248010097311ULL;
    const uint64_t s0 = *state0;
    const uint64_t rot = s0 >> 60;
    *state0 *= 12898287266413564321ULL;
    uint64_t xs = uint64_ror(s0, 18) + s0;
    xs = uint64_ror(xs, rot);
    return xs;
}

//#define BENCHMARK

#ifdef BENCHMARK
static KLock* KLOCK = NULL;
static KHashTable* HTABLE = NULL;
// Number of microseconds since last called
static unsigned long stopwatch(void)
{
    static unsigned long start = 0;
    struct timeval tv_cur;

    gettimeofday(&tv_cur, NULL);
    unsigned long finish = tv_cur.tv_sec * 1000000 + tv_cur.tv_usec;
    unsigned long elapsed = finish - start;
    start = finish;
    return elapsed;
}
#endif // BENCHMARK

TEST_SUITE(KHashFileTestSuite);

TEST_CASE(Klib_KHashFileSet)
{
    rc_t rc;
    const char* str1 = "Tu estas probando este hoy, no manana";
    const char* str2 = "Tu estas probando este hoy, no mananX";
    size_t size = strlen(str1);

    KHashFile* hset = NULL;
    rc = KHashFileMake(NULL, NULL);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ((void*)hset, (void*)NULL);

    rc = KHashFileMake(&hset, NULL);
    REQUIRE_RC(rc);
    REQUIRE_NE((void*)hset, (void*)NULL);

    rc = KHashFileReserve(hset, 100);
    REQUIRE_RC(rc);
    size_t sz = KHashFileCount(hset);
    REQUIRE_EQ(sz, (size_t)0);

    uint64_t hash = KHash(str1, size);
    rc = KHashFileAdd(hset, str1, strlen(str1), hash, NULL, 0);
    REQUIRE_RC(rc);

    sz = KHashFileCount(hset);
    REQUIRE_EQ(sz, (size_t)1);

    rc = KHashFileAdd(hset, str1, strlen(str1), hash, NULL, 0);
    REQUIRE_RC(rc);

    sz = KHashFileCount(hset);
    REQUIRE_EQ(sz, (size_t)1);

    bool found;
    found = KHashFileFind(hset, str1, strlen(str1), hash, NULL, NULL);
    REQUIRE_EQ(found, true);

    found = KHashFileFind(hset, str2, strlen(str2), hash, NULL, NULL);
    REQUIRE_EQ(found, false);

    KHashFileDispose(hset);
}

TEST_CASE(Klib_hashfileMap)
{
    rc_t rc;
    const char* str1 = "Tu estas probando este hoy, no manana";
    const char* str2 = "Tu estas probando este hoy, no mananX";

    KHashFile* hmap;
    rc = KHashFileMake(&hmap, BACKING);
    REQUIRE_RC(rc);

    size_t sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)0);

    uint64_t hash = 1;
    uint64_t val1 = 123;
    rc = KHashFileAdd(hmap, str1, strlen(str1), hash, &val1, sizeof(val1));
    REQUIRE_RC(rc);

    sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)1);

    rc = KHashFileAdd(hmap, str1, strlen(str1), hash, &val1, sizeof(val1));
    REQUIRE_RC(rc);

    sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)1);

    bool found;
    uint64_t val;
    uint64_t len = 0;
    found = KHashFileFind(hmap, str1, strlen(str1), hash, NULL, 0);
    REQUIRE_EQ(found, true);
    found = KHashFileFind(hmap, str1, strlen(str1), hash, &val, &len);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(len, sizeof(val));
    len = 0;
    REQUIRE_EQ(val, (uint64_t)123);

    uint64_t val2 = 124;
    rc = KHashFileAdd(hmap, str1, strlen(str1), hash, &val2, sizeof(val2));
    REQUIRE_RC(rc);

    sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)1);

    found = KHashFileFind(hmap, str1, strlen(str1), hash, &val, &len);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(len, sizeof(val));
    len = 0;
    REQUIRE_EQ(val, (uint64_t)124);

    found = KHashFileFind(hmap, str2, strlen(str2), hash, NULL, 0);
    REQUIRE_EQ(found, false);

    uint64_t val3 = 125;
    rc = KHashFileAdd(hmap, str2, strlen(str2), hash, &val3, sizeof(val3));
    REQUIRE_RC(rc);

    found = KHashFileFind(hmap, str2, strlen(str2), hash, &val, &len);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(len, sizeof(val));
    len = 0;
    REQUIRE_EQ(val, (uint64_t)125);

    sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)2);

    KHashFileDispose(hmap);
}

TEST_CASE(Klib_hashfiledups)
{
    rc_t rc;

    KHashFile* hmap;
    rc = KHashFileMake(&hmap, BACKING);
    REQUIRE_RC(rc);

    uint64_t key1 = random();
    uint64_t hash1 = KHash((const char*)&key1, sizeof(uint64_t));
    uint64_t key2 = random();
    uint64_t hash2 = hash1;
    for (size_t i = 0; i != 100000; ++i) {
        rc = KHashFileAdd(hmap, (const char*)&key1, 8, hash1,
                          (const char*)&key1, 8);
        REQUIRE_RC(rc);
        rc = KHashFileAdd(hmap, (const char*)&key2, 8, hash2,
                          (const char*)&key1, 8);
        REQUIRE_RC(rc);
    }

    size_t sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)2);

    KHashFileDispose(hmap);
}

TEST_CASE(Klib_hashfileMapDeletes)
{
    rc_t rc;

    uint64_t state = random();

    KHashFile* hmap;
    rc = KHashFileMake(&hmap, BACKING);
    REQUIRE_RC(rc);

    size_t sz = KHashFileCount(hmap);
    REQUIRE_EQ(sz, (size_t)0);

    std::vector<std::string> strs, vals;
    const size_t loops = 10000;
    for (size_t i = 0; i != loops; ++i) {
        strs.push_back(string(1 + (fastrng(&state) % 500),
                              char(32 + fastrng(&state) % 90)));
        vals.push_back(string(1 + (fastrng(&state) % 500),
                              char(32 + fastrng(&state) % 90)));
    }

    std::unordered_map<std::string, std::string> map;
    for (size_t i = 0; i != strs.size(); ++i) {
        const auto key = strs[i];
        const auto val = vals[i];
        auto pair = std::make_pair(key, val);
        map.erase(key);
        map.insert(pair);

        uint64_t hash = KHash(key.data(), key.size());
        KHashFileDelete(hmap, key.data(), key.size(), hash);
        rc = KHashFileAdd(hmap, key.data(), key.size(), hash, val.data(),
                          val.size());
        REQUIRE_RC(rc);

        if (fastrng(&state) % 2) {
            map.erase(key);
            bool found = KHashFileDelete(hmap, key.data(), key.size(), hash);
            REQUIRE_EQ(found, true);
        }

        sz = KHashFileCount(hmap);
        REQUIRE_EQ(sz, (size_t)map.size());
    }

    for (auto it : map) {
        const auto key = it.first;
        const auto value = it.second;

        uint64_t hash = KHash(key.data(), key.size());
        char val[500];
        size_t len = 0;
        bool found
            = KHashFileFind(hmap, key.data(), key.size(), hash, &val, &len);
        REQUIRE_EQ(found, true);
        REQUIRE_EQ(value.size(), len);
        REQUIRE_EQ((int)0, memcmp(value.data(), val, len));
    }

    KHashFileDispose(hmap);
}

static rc_t inserter(const KThread* thread, void* data)
{
    rc_t rc;
    uint64_t state = random();
    while (KEEPRUNNING) {
        size_t idx = fastrng(&state) % RANDS_SIZE;
        uint64_t key = RANDS[idx];
        uint64_t val = key + 1;
        uint64_t hash = KHash((const char*)&key, 8);
        rc = KHashFileAdd(HMAP, &key, sizeof(key), hash, &val, sizeof(val));
        if (rc != 0) {
            fprintf(stderr, "Add failed\n");
            // REQUIRE_RC(rc);
        }
        if (val != key + 1) {
            fprintf(stderr, "val touched\n");
            // REQUIRE_EQ((uint64_t)val,(uint64_t)key+1);
        }
    }
    return 0;
}

static rc_t deleter(const KThread* thread, void* data)
{
    uint64_t state = random();
    while (KEEPRUNNING) {
        size_t idx = fastrng(&state) % RANDS_SIZE;
        uint64_t key = RANDS[idx];
        uint64_t hash = KHash((const char*)&key, 8);
        KHashFileDelete(HMAP, &key, sizeof(key), hash);
    }

    return 0;
}

static rc_t finder(const KThread* thread, void* data)
{
    uint64_t state = random();
    while (KEEPRUNNING) {
        size_t idx = fastrng(&state) % RANDS_SIZE;
        uint64_t key = RANDS[idx];
        uint64_t val = 0;
        size_t val_size = 9;
        uint64_t hash = KHash((const char*)&key, 8);
        atomic64_inc(&FINDCOUNT);
        bool found
            = KHashFileFind(HMAP, &key, sizeof(key), hash, &val, &val_size);
        if (found) {
            if (val_size != sizeof(val)) {
                fprintf(stderr, "bad find size\n");
            }
            if (val != key + 1) {
                fprintf(stderr, "bad find\n");
            }
            val_size = 9;
        } else {
            if (val != 0) {
                fprintf(stderr, "bad not found\n");
            }
        }
    }

    return 0;
}

static rc_t notfinder(const KThread* thread, void* data)
{
    uint64_t state = random();
    while (KEEPRUNNING) {
        uint64_t key = fastrng(&state);
        uint64_t val = 9;
        size_t val_size = 9;
        uint64_t hash = KHash((const char*)&key, 8);
        bool found
            = KHashFileFind(HMAP, &key, sizeof(key), hash, &val, &val_size);
        if (found) {
            fprintf(stderr, "false found\n");
        } else {
            if (val != 9 || val_size != 9) {
                fprintf(stderr, "bad not not found\n");
            }
        }
    }

    return 0;
}

#ifdef BENCHMARK
static rc_t bench_inserter(const KThread* thread, void* data)
{
    uint64_t state = random();
    rc_t rc;
    while (KEEPRUNNING) {
        uint64_t rnd = fastrng(&state);
        uint64_t key = rnd;
        uint64_t val = key + 1;
        uint64_t hash = KHash((const char*)&key, 8);
        rc = KHashFileAdd(HMAP, &key, sizeof(key), hash, &val, sizeof(val));
        if (rc != 0) {
            fprintf(stderr, "Add failed\n");
            // REQUIRE_RC(rc);
        }
    }

    return 0;
}

TEST_CASE(Klib_Bench_hashfilethreads)
{
    KEEPRUNNING = true;
    KHashFileMake(&HMAP, BACKING);

    const size_t NUM_THREADS = 100;
    for (long i = 0; i != NUM_THREADS; ++i) {
        KThread* thrd;
        KThreadMake(&thrd, bench_inserter, (void*)i);
    }

    size_t oldcnt = KHashFileCount(HMAP);
    stopwatch();
    fprintf(stderr, "10B\n");
    while (KHashFileCount(HMAP) < 10 * 1000 * 1000 * 1000ULL) {
        size_t cnt = KHashFileCount(HMAP);
        unsigned long us = stopwatch();
        double ips = (double)(cnt - oldcnt) / us;
        oldcnt = cnt;
        fprintf(stderr,
                " %lu threads required %lu ms to insert %lu (%.1f "
                "Minserts/sec)\n",
                NUM_THREADS, us / 1000, cnt, ips);
        KSleepMs(1000);
    }
    fprintf(stderr, "Done\n");
    KEEPRUNNING = false;
    KSleepMs(1000);
    KHashFileDispose(HMAP);
}

TEST_CASE(Klib_Bench_hashfiletune)
{
    rc_t rc;
    uint8_t numts[] = {1, 5, 10, 20, 50, 100};
    for (size_t nt = 0; nt != 6; ++nt) {
        const size_t NUM_THREADS = numts[nt];
        time_t starttime = time(NULL);

        KEEPRUNNING = true;
        rc = KHashFileMake(&HMAP, BACKING);
        REQUIRE_RC(rc);
        KHashFileReserve(HMAP, 100000000);
        REQUIRE_RC(rc);

        for (size_t i = 0; i != NUM_THREADS; ++i) {
            KThread* thrd;
            KThreadMake(&thrd, bench_inserter, (void*)i);
        }

        size_t oldcnt = KHashFileCount(HMAP);
        stopwatch();
        while (KHashFileCount(HMAP) < 100000000) {
            size_t cnt = KHashFileCount(HMAP);
            unsigned long us = stopwatch();
            double ips = (double)(cnt - oldcnt) / us;
            oldcnt = cnt;
            fprintf(stderr,
                    "%lu threads required %lu ms to insert %lu (%.1f "
                    "Minserts/sec)\n",
                    NUM_THREADS, us / 1000, cnt, ips);
            KSleepMs(1000);
        }

        atomic64_set(&FINDCOUNT, 0);
        KEEPRUNNING = false;
        KSleepMs(1000);

        KEEPRUNNING = true;
        for (size_t i = 0; i != NUM_THREADS; ++i) {
            KThread* thrd;
            KThreadMake(&thrd, finder, (void*)i);
        }

        for (size_t loops = 0; loops != 10; ++loops) {
            KSleepMs(1000);
            fprintf(stderr, "FINDCOUNT is %lu\n", atomic64_read(&FINDCOUNT));
        }
        KEEPRUNNING = false;
        KSleepMs(1000);

        KHashFileDispose(HMAP);
        time_t endtime = time(NULL);
        fprintf(stderr, "time required %lu\n", endtime - starttime);
    }
}

static rc_t table_bench_inserter(const KThread* thread, void* data)
{
    uint64_t state = random();
    rc_t rc;
    while (KEEPRUNNING) {
        uint64_t rnd = fastrng(&state);
        uint64_t key = rnd;
        uint64_t val = key + 1;
        uint64_t hash = KHash((const char*)&key, 8);
        KLockAcquire(KLOCK);
        rc = KHashTableAdd(HTABLE, &key, hash, &val);
        if (rc != 0) {
            fprintf(stderr, "Add failed\n");
        }
        KLockUnlock(KLOCK);
    }

    return 0;
}

static rc_t table_bench_finder(const KThread* thread, void* data)
{
    uint64_t state = random();
    while (KEEPRUNNING) {
        uint64_t rnd = fastrng(&state);
        uint64_t key = rnd;
        uint64_t val = 0;
        uint64_t hash = KHash((const char*)&key, 8);
        KLockAcquire(KLOCK);
        KHashTableFind(HTABLE, &key, hash, &val);
        KLockUnlock(KLOCK);
        atomic64_inc(&FINDCOUNT);
    }

    return 0;
}

TEST_CASE(Klib_Bench_hashtableetune)
{
    rc_t rc;
    KLockMake(&KLOCK);
    uint8_t numts[] = {1, 5, 10, 20, 50, 100};
    for (size_t nt = 0; nt != 6; ++nt) {
        const size_t NUM_THREADS = numts[nt];
        time_t starttime = time(NULL);

        KEEPRUNNING = true;
        rc = KHashTableMake(&HTABLE, 8, 8, 100 * 1000 * 1000, 0, raw);
        REQUIRE_RC(rc);

        for (size_t i = 0; i != NUM_THREADS; ++i) {
            KThread* thrd;
            KThreadMake(&thrd, table_bench_inserter, (void*)i);
        }

        size_t oldcnt = KHashTableCount(HTABLE);
        stopwatch();
        while (KHashTableCount(HTABLE) < 100000000) {
            size_t cnt = KHashTableCount(HTABLE);
            unsigned long us = stopwatch();
            double ips = (double)(cnt - oldcnt) / us;
            oldcnt = cnt;
            fprintf(stderr,
                    "table %lu threads required %lu ms to insert %lu (%.1f "
                    "Minserts/sec)\n",
                    NUM_THREADS, us / 1000, cnt, ips);
            KSleepMs(1000);
        }
        KEEPRUNNING = false;
        KSleepMs(1000);
        time_t endtime = time(NULL);
        fprintf(stderr, "time required %lu\n", endtime - starttime);

        atomic64_set(&FINDCOUNT, 0);
        KEEPRUNNING = true;
        for (size_t i = 0; i != NUM_THREADS; ++i) {
            KThread* thrd;
            KThreadMake(&thrd, table_bench_finder, (void*)i);
        }
        KSleepMs(10000);
        KEEPRUNNING = false;
        KSleepMs(1000);
        KHashTableDispose(HTABLE, NULL, NULL, NULL);
    }
    KLockRelease(KLOCK);
}
#endif // BENCHMARK

TEST_CASE(Klib_hashfilethreads)
{
    atomic64_set(&FINDCOUNT, 0);
    KEEPRUNNING = true;
    KHashFileMake(&HMAP, BACKING);

    Vector threads;
    VectorInit(&threads, 0, 0);
    const size_t NUM_THREADS = 40;
    for (long i = 0; i != NUM_THREADS; ++i) {
        KThread* thrd;
        KThreadMake(&thrd, inserter, (void*)i);
        VectorAppend(&threads, NULL, thrd);
        KThreadMake(&thrd, deleter, (void*)i);
        VectorAppend(&threads, NULL, thrd);
        KThreadMake(&thrd, finder, (void*)i);
        VectorAppend(&threads, NULL, thrd);
        KThreadMake(&thrd, notfinder, (void*)i);
        VectorAppend(&threads, NULL, thrd);
    }

    for (size_t loops = 0; loops != 10; ++loops) {
        fprintf(stderr, "Count is %lu\n", KHashFileCount(HMAP));
        KSleepMs(1000);
    }
    KEEPRUNNING = false;
    KSleepMs(1000);

    for (long i = 0; i != VectorLength(&threads); ++i) {
        KThread* thrd = (KThread*)VectorGet(&threads, i);
        KThreadRelease(thrd);
    }
    VectorWhack(&threads, NULL, NULL);
    KHashFileDispose(HMAP);
}

TEST_CASE(Klib_HashFileIterator)
{
    const int loops = 10000;
    rc_t rc;

    KHashFile* hmap;
    rc = KHashFileMake(&hmap, NULL);
    REQUIRE_RC(rc);
    uint32_t key;
    uint32_t value;

    std::unordered_map<uint32_t, uint32_t> map;
    for (int iter = 0; iter != 2; ++iter) {
        for (int i = 0; i != loops; ++i) {
            key = random() % loops;
            value = key + 1;

            auto pair = std::make_pair(key, value);
            map.erase(key);
            map.insert(pair);

            uint64_t hash = KHash((char*)&key, 4);
            KHashFileDelete(hmap, (void*)&key, 4, hash);
            rc = KHashFileAdd(hmap, (void*)&key, 4, hash, (void*)&value, 4);

            size_t mapcount = map.size();
            size_t hmapcount = KHashFileCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }

        for (int i = 0; i != loops; ++i) {
            key = random() % loops;

            map.erase(key);

            uint64_t hash = KHash((char*)&key, 4);
            KHashFileDelete(hmap, (void*)&key, 4, hash);
            bool found
                = KHashFileFind(hmap, (void*)&key, 4, hash, NULL, NULL);
            REQUIRE_EQ(found, false);

            size_t mapcount = map.size();
            size_t hmapcount = KHashFileCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }

        for (int i = 0; i != loops; ++i) {
            key = random() % loops;
            value = (uint32_t)random();

            auto pair = std::make_pair(key, value);
            map.erase(key);
            map.insert(pair);

            uint64_t hash = KHash((char*)&key, 4);
            KHashFileDelete(hmap, (void*)&key, 4, hash);
            rc = KHashFileAdd(hmap, (void*)&key, 4, hash, (void*)&value, 4);

            size_t mapcount = map.size();
            size_t hmapcount = KHashFileCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }

        size_t mapcount = map.size();
        size_t hmapcount = KHashFileCount(hmap);
        REQUIRE_EQ(mapcount, hmapcount);

        size_t founds = 0;
        key = loops + 1;
        size_t key_size = 0;
        size_t value_size = 0;
        KHashFileIteratorMake(hmap);
        while (KHashFileIteratorNext(hmap, &key, &key_size, &value,
                                     &value_size)) {
            auto mapfound = map.find(key);
            if (mapfound == map.end()) {
                fprintf(stderr, "no key=%d\n", key);
                REQUIRE_EQ(key_size, (size_t)0);
                REQUIRE_EQ(value_size, (size_t)0);
                REQUIRE_EQ(true, false);
            } else {
                REQUIRE_EQ(key_size, (size_t)4);
                REQUIRE_EQ(value_size, (size_t)4);
                uint32_t mvalue = mapfound->second;
                REQUIRE_EQ(value, mvalue);
                ++founds;
            }
            key_size = 0;
            value_size = 0;
        }
        mapcount = map.size();
        hmapcount = KHashFileCount(hmap);
        REQUIRE_EQ(founds, hmapcount);

        KHashFileIteratorMake(hmap);
        while (KHashFileIteratorNext(hmap, &key, NULL, NULL, NULL)) {
            map.erase(key);
            uint64_t hash = KHash((char*)&key, 4);
            KHashFileDelete(hmap, (void*)&key, 4, hash);
        }
        mapcount = map.size();
        hmapcount = KHashFileCount(hmap);
        REQUIRE_EQ(mapcount, hmapcount);
        REQUIRE_EQ(mapcount, (size_t)0);
    }
    KHashFileDispose(hmap);
}

extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-hashfile";

rc_t CC KMain(int argc, char* argv[])
{
    rc_t rc;
    srandom(time(NULL));
    uint64_t state = random();

    for (size_t i = 0; i != RANDS_SIZE; ++i) {
        RANDS[i] = fastrng(&state);
    }

    rc = KDirectoryNativeDir(&DIR);
    if (rc) return rc;

    const char* fname = tmpnam(NULL);
    rc = KDirectoryCreateFile(DIR, &BACKING, true, 0600, kcmInit, fname);
    if (rc) return rc;

    KConfigDisableUserSettings();

    rc = KHashFileTestSuite(argc, argv);
    if (rc) return rc;

    rc = KDirectoryRemove(DIR, true, "%s", fname);
    if (rc) return rc;

    rc = KFileRelease(BACKING);
    if (rc) return rc;
    rc = KDirectoryRelease(DIR);
    if (rc) return rc;

    return 0;
}
}
