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
* Unit tests for hash and hashtables
*/

#include <ktst/unit_test.hpp>

#include <klib/data-buffer.h>
#include <klib/hashtable.h>
#include <klib/log.h>
#include <klib/misc.h> /* is_user_admin() */
#include <klib/num-gen.h>
#include <klib/printf.h>
#include <klib/sort.h>
#include <klib/text.h>
#include <klib/vector.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace std;

/* #define BENCHMARK */

TEST_SUITE(KHashTableTestSuite);

TEST_CASE(Klib_KHash)
{
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);

    uint64_t hash = KHash(str, size);
    REQUIRE_NE(hash, (uint64_t)0);

    uint64_t hash2 = KHash(str, size);
    REQUIRE_EQ(hash, hash2);
}

TEST_CASE(Klib_KHash_unique)
{
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);

    uint64_t hash1 = KHash(str, size);
    uint64_t hash2 = KHash(str, size - 1);
    REQUIRE_NE(hash1, hash2);
}

TEST_CASE(Klib_KHash_Adjacent)
{
    uint64_t hash1, hash2, diff;

    uint64_t val = 0x1234567890ABCDE0;

    hash1 = KHash((char*)&val, 8);
    ++val;
    hash2 = KHash((char*)&val, 8);
    diff = labs(hash2 - hash1);
    REQUIRE_LE(diff, (uint64_t)7);

    const char* str1 = "string01";
    const char* str2 = "string02";
    size_t size = strlen(str1);
    hash1 = KHash(str1, size);
    hash2 = KHash(str2, size);
    diff = labs(hash2 - hash1) & 0xfffff;
    if (diff > 7) {
        fprintf(stderr, "%lx %lx\n", hash1, hash2);
        REQUIRE_LE(diff, (uint64_t)7);
    }

    str1 = "str01";
    str2 = "str02";
    size = strlen(str1);
    hash1 = KHash(str1, size);
    hash2 = KHash(str2, size);
    diff = labs(hash2 - hash1);
    REQUIRE_LE(diff, (uint64_t)7);
}

TEST_CASE(Klib_KHash_Collide)
{
    // We fill a buffer with random bytes, and then increment each byte once
    // and verify no collisions occur for all lengths.
    char buf[37];
    for (size_t l = 0; l != sizeof(buf); l++) buf[l] = (char)random();

    std::unordered_set<uint64_t> set;

    size_t inserts = 0;
    size_t collisions = 0;
    for (size_t l = 0; l != sizeof(buf); l++)
        for (size_t j = 0; j != l; j++)
            for (size_t k = 0; k != 255; k++) {
                buf[j] += 1;
                uint64_t hash = KHash(buf, l);
                size_t count = set.count(hash);
                if (count) {
                    collisions++;
                    fprintf(stderr,
                            "Collision at %lu on hash of len %lu is %lx: "
                            "%lu elements %lx\n",
                            j, l, hash, set.size(), *(uint64_t*)buf);
                }
                set.insert(hash);
                inserts++;
            }
    REQUIRE_EQ(inserts, set.size());
}

TEST_CASE(Klib_KHashTableSet)
{
    rc_t rc;
    const char* str1 = "Tu estas probando este hoy, no manana";
    const char* str2 = "Tu estas probando este hoy, no mananX";
    size_t size = strlen(str1);

    KHashTable* hset = NULL;
    rc = KHashTableMake(NULL, 8, 0, 0, 1.0, cstr);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ((void*)hset, (void*)NULL);
    rc = KHashTableMake(&hset, 8, 0, 0, -1.0, cstr);
    REQUIRE_RC_FAIL(rc);
    rc = KHashTableMake(&hset, 8, 0, 0, 1.0, cstr);
    REQUIRE_RC_FAIL(rc);
    rc = KHashTableMake(&hset, 0, 0, 0, 1.0, raw);
    REQUIRE_RC_FAIL(rc);
    rc = KHashTableMake(&hset, 4, 0, 0, 1.0, cstr);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ((void*)hset, (void*)NULL);

    rc = KHashTableMake(&hset, 8, 0, 0, 0.0, cstr);
    REQUIRE_RC(rc);
    REQUIRE_NE((void*)hset, (void*)NULL);

    size_t sz = KHashTableCount(hset);
    REQUIRE_EQ(sz, (size_t)0);

    uint64_t hash = KHash(str1, size);
    rc = KHashTableAdd(hset, str1, hash, NULL);
    REQUIRE_RC(rc);
    rc = KHashTableAdd(hset, str1, hash, NULL);
    REQUIRE_RC(rc);

    sz = KHashTableCount(hset);
    REQUIRE_EQ(sz, (size_t)1);

    bool found;
    found = KHashTableFind(hset, str1, hash, NULL);
    REQUIRE_EQ(found, true);

    found = KHashTableFind(hset, str2, hash, NULL);
    REQUIRE_EQ(found, false);

    KHashTableDispose(hset, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMap)
{
    const char* str1 = "Tu estas probando este hoy, no manana";
    const char* str2 = "Tu estas probando este hoy, no mananX";

    KHashTable* hmap;
    rc_t rc = KHashTableMake(&hmap, 8, 8, 0, 0, cstr);
    REQUIRE_RC(rc);

    size_t sz = KHashTableCount(hmap);
    REQUIRE_EQ(sz, (size_t)0);

    uint64_t hash = 1;
    uint64_t val1 = 123;
    rc = KHashTableAdd(hmap, str1, hash, &val1);
    REQUIRE_RC(rc);

    sz = KHashTableCount(hmap);
    REQUIRE_EQ(sz, (size_t)1);

    rc = KHashTableAdd(hmap, str1, hash, &val1);
    REQUIRE_RC(rc);

    sz = KHashTableCount(hmap);
    REQUIRE_EQ(sz, (size_t)1);

    bool found;
    uint64_t val;
    found = KHashTableFind(hmap, str1, hash, &val);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(val, (uint64_t)123);

    uint64_t val2 = 124;
    rc = KHashTableAdd(hmap, str1, hash, &val2);
    REQUIRE_RC(rc);

    sz = KHashTableCount(hmap);
    REQUIRE_EQ(sz, (size_t)1);

    found = KHashTableFind(hmap, str1, hash, &val);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(val, (uint64_t)124);

    found = KHashTableFind(hmap, str2, hash, &val);
    REQUIRE_EQ(found, false);

    uint64_t val3 = 125;
    rc = KHashTableAdd(hmap, str2, hash, &val3);
    REQUIRE_RC(rc);

    found = KHashTableFind(hmap, str2, hash, &val);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(val, (uint64_t)125);

    sz = KHashTableCount(hmap);
    REQUIRE_EQ(sz, (size_t)2);

    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapInts)
{
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, 8, 8, 0, 0, raw);
    REQUIRE_RC(rc);

    // Test probing, constant hash value
    uint64_t hash = random();
    hash = random() % 5;

    for (uint64_t i = 0; i != 100; i++) {
        bool found;
        uint64_t val;
        found = KHashTableFind(hmap, (void*)&i, hash, &val);
        REQUIRE_EQ(found, false);
    }

    size_t count = 0;
    for (uint64_t i = 0; i != 100; i++) {
        uint64_t j = i * 3;
        rc = KHashTableAdd(hmap, (void*)&i, hash, (void*)&j);
        REQUIRE_RC(rc);
        ++count;
        size_t sz = KHashTableCount(hmap);
        REQUIRE_EQ(sz, count);
    }

    for (uint64_t i = 0; i != 100; i++) {
        bool found;
        uint64_t val;
        found = KHashTableFind(hmap, (void*)&i, hash, &val);
        REQUIRE_EQ(found, true);
        REQUIRE_EQ(val, i * 3);
    }

    for (uint64_t i = 0; i != 10000; i++) {
        bool found;
        uint64_t key = i + 9999999;
        uint64_t hash = KHash((char*)&key, 8);
        found = KHashTableFind(hmap, (void*)&key, hash, NULL);
        REQUIRE_EQ(found, false);
    }

    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapStrings)
{
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, sizeof(char*), sizeof(char*), 0, 0.0, cstr);
    REQUIRE_RC(rc);
    const char* JOJOA[]
        = {"JOJO01", "JOJO02", "JOJO03", "JOJO04", "JOJO05", "JOJO06",
           "JOJO07", "JOJO08", "JOJO09", "JOJO10", "JOJO11", "JOJO12"};

    static const int JOJOAQ = sizeof(JOJOA) / sizeof(char*);
    for (int llp = 0; llp < JOJOAQ; llp++) {
        const char* Key = JOJOA[llp];
        char* Val = NULL;
        uint64_t Hash = 0;
        Hash = KHash(Key, strlen(Key));
        bool found;
        found = KHashTableFind(hmap, Key, Hash, &Val);
        REQUIRE_EQ(found, false);
        Hash = KHash(Key, strlen(Key));
        rc_t rc = KHashTableAdd(hmap, Key, Hash, &Key);
        REQUIRE_RC(rc);
        Hash = KHash(Key, strlen(Key));
        found = KHashTableFind(hmap, Key, Hash, &Val);
        REQUIRE_EQ(found, true);
    }

    // TODO: Whack with destructors
    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapInts2)
{
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, 8, 8, 0, 0.0, raw);
    REQUIRE_RC(rc);

    size_t count = 0;
    for (uint64_t i = 0; i != 1000; i++) {
        uint64_t j = i * 3;
        uint64_t hash = KHash((char*)&i, 8);
        rc = KHashTableAdd(hmap, (void*)&i, hash, (void*)&j);
        REQUIRE_RC(rc);
        count++;
        size_t sz = KHashTableCount(hmap);
        REQUIRE_EQ(sz, count);
    }

    for (uint64_t i = 0; i != 1000; i++) {
        bool found;
        uint64_t val;
        uint64_t hash = KHash((char*)&i, 8);
        found = KHashTableFind(hmap, (void*)&i, hash, &val);
        REQUIRE_EQ(found, true);
        REQUIRE_EQ(val, i * 3);
    }

    for (uint64_t i = 0; i != 1000; i++) {
        bool found;
        uint64_t key = i + 9999999;
        uint64_t hash = KHash((char*)&key, 8);
        found = KHashTableFind(hmap, (void*)&key, hash, NULL);
        REQUIRE_EQ(found, false);
    }

    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapValid)
{
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, 4, 4, 0, 0.0, raw);
    REQUIRE_RC(rc);

    std::unordered_map<uint32_t, uint32_t> map;

    uint64_t hash = (uint64_t)random(); // Test probing
    const int loops = 10000;
    for (int i = 0; i != loops; ++i) {
        uint32_t key = random() % loops;
        uint32_t value = i;

        auto pair = std::make_pair(key, value);
        map.erase(key);
        map.insert(pair);
        rc = KHashTableAdd(hmap, (void*)&key, hash, (void*)&value);
        bool hfound = KHashTableFind(hmap, (void*)&key, hash, (void*)&value);
        REQUIRE_EQ(hfound, true);
    }

    size_t mapcount = map.size();
    size_t hmapcount = KHashTableCount(hmap);
    REQUIRE_EQ(mapcount, hmapcount);

    for (int i = 0; i != loops; ++i) {
        uint32_t key = random() % loops;
        uint32_t hvalue = 0;
        bool hfound = KHashTableFind(hmap, (void*)&key, hash, &hvalue);

        auto mapfound = map.find(key);
        if (mapfound == map.end()) {
            REQUIRE_EQ(hfound, false);
        } else {
            REQUIRE_EQ(hfound, true);
            uint32_t mvalue = mapfound->second;
            REQUIRE_EQ(hvalue, mvalue);
        }
    }
    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapDeletes)
{
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, 4, 4, 0, 0.95, raw);
    REQUIRE_RC(rc);

    std::unordered_map<uint32_t, uint32_t> map;

    // Test probing, constant hash value
    uint64_t hash = random();
    long int threshold = random();  // Delete some fraction of keys
    long int threshold2 = random(); // Delete some fraction of keys

    for (int iter = 0; iter != 3; ++iter) {
        const int loops = random() % 10000;
        for (int i = 0; i != loops; ++i) {
            uint32_t key = random() % loops;
            uint32_t value = i;

            auto pair = std::make_pair(key, value);
            if (random() > threshold2) {
                map.erase(key);
                map.insert(pair);

                rc = KHashTableAdd(hmap, (void*)&key, hash, (void*)&value);
                bool hfound = KHashTableFind(hmap, (void*)&key, hash, NULL);
                REQUIRE_EQ(hfound, true);
            }

            if (random() > threshold) {
                map.erase(key);
                KHashTableDelete(hmap, (void*)&key, hash);
            }

            size_t mapcount = map.size();
            size_t hmapcount = KHashTableCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }

        for (int i = 0; i != loops; ++i) {
            uint32_t key = random() % loops;
            uint32_t hvalue = 0;
            bool hfound = KHashTableFind(hmap, (void*)&key, hash, &hvalue);

            auto mapfound = map.find(key);
            if (mapfound == map.end()) {
                REQUIRE_EQ(hfound, false);
            } else {
                REQUIRE_EQ(hfound, true);
                uint32_t mvalue = mapfound->second;
                REQUIRE_EQ(hvalue, mvalue);
            }
        }
    }
    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapIterator)
{
    const int loops = 10000;
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, 4, 4, loops, 0.0, raw);
    REQUIRE_RC(rc);
    uint32_t key;
    uint32_t value;

    std::unordered_map<uint32_t, uint32_t> map;
    for (int iter = 0; iter != 2; ++iter) {
        for (int i = 0; i != loops; ++i) {
            key = random() % loops;
            value = i;
            uint64_t hash = KHash((char*)&key, 4);

            auto pair = std::make_pair(key, value);
            map.erase(key);
            map.insert(pair);
            rc = KHashTableAdd(hmap, (void*)&key, hash, (void*)&value);

            size_t mapcount = map.size();
            size_t hmapcount = KHashTableCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }
        for (int i = 0; i != loops; ++i) {
            key = random() % loops;
            uint64_t hash = KHash((char*)&key, 4);

            map.erase(key);
            KHashTableDelete(hmap, (void*)&key, hash);
            bool found = KHashTableFind(hmap, (void*)&key, hash, NULL);
            REQUIRE_EQ(found, false);

            size_t mapcount = map.size();
            size_t hmapcount = KHashTableCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }
        for (int i = 0; i != loops; ++i) {
            key = random() % loops;
            value = random();
            uint64_t hash = KHash((char*)&key, 4);

            auto pair = std::make_pair(key, value);
            map.erase(key);
            map.insert(pair);
            rc = KHashTableAdd(hmap, (void*)&key, hash, (void*)&value);

            size_t mapcount = map.size();
            size_t hmapcount = KHashTableCount(hmap);
            REQUIRE_EQ(mapcount, hmapcount);
        }

        size_t founds = 0;
        key = loops + 1;
        KHashTableIteratorMake(hmap);
        while (KHashTableIteratorNext(hmap, &key, &value)) {
            auto mapfound = map.find(key);
            if (mapfound == map.end()) {
                fprintf(stderr, "no key=%d\n", key);
                REQUIRE_EQ(true, false);
            } else {
                uint32_t mvalue = mapfound->second;
                REQUIRE_EQ(value, mvalue);
                ++founds;
            }
        }
        size_t mapcount = map.size();
        size_t hmapcount = KHashTableCount(hmap);
        REQUIRE_EQ(founds, hmapcount);

        KHashTableIteratorMake(hmap);
        while (KHashTableIteratorNext(hmap, &key, NULL)) {
            map.erase(key);
            uint64_t hash = KHash((char*)&key, 4);
            KHashTableDelete(hmap, (void*)&key, hash);
        }
        mapcount = map.size();
        hmapcount = KHashTableCount(hmap);
        REQUIRE_EQ(mapcount, hmapcount);
        REQUIRE_EQ(mapcount, (size_t)0);
    }
    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapSmallKeys)
{
    rc_t rc;

    KHashTable* hmap;
    rc = KHashTableMake(&hmap, 2, 2, 0, 0.0, raw);
    REQUIRE_RC(rc);

    std::unordered_map<uint16_t, uint16_t> map;

    uint64_t hash = (uint64_t)random(); // Test probing
    const int loops = 1000;
    for (int i = 0; i != loops; ++i) {
        uint16_t key = random() % loops;
        uint16_t value = i;

        auto pair = std::make_pair(key, value);
        map.erase(key);
        map.insert(pair);
        rc = KHashTableAdd(hmap, (void*)&key, hash, (void*)&value);
    }

    size_t mapcount = map.size();
    size_t hmapcount = KHashTableCount(hmap);
    REQUIRE_EQ(mapcount, hmapcount);

    for (int i = 0; i != loops; ++i) {
        uint16_t key = random() % loops;
        uint16_t hvalue = 0;
        bool hfound = KHashTableFind(hmap, (void*)&key, hash, &hvalue);

        auto mapfound = map.find(key);
        if (mapfound == map.end()) {
            REQUIRE_EQ(hfound, false);
        } else {
            REQUIRE_EQ(hfound, true);
            uint16_t mvalue = mapfound->second;
            REQUIRE_EQ(hvalue, mvalue);
        }
    }
    KHashTableDispose(hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashTableMapReserve)
{
    rc_t rc;

    KHashTable* hmap;
    size_t capacity = random() % 20000;
    rc = KHashTableMake(&hmap, 2, 2, capacity, 0.0, raw);
    REQUIRE_RC(rc);

    std::unordered_map<uint16_t, uint16_t> map;

    uint64_t hash = (uint64_t)random(); // Test probing
    const int loops = 1000;
    for (int i = 0; i != loops; ++i) {
        capacity = random() % 20000;
        rc = KHashTableReserve(hmap, capacity);
        REQUIRE_RC(rc);

        uint16_t key = random() % loops;
        uint16_t value = i;

        auto pair = std::make_pair(key, value);
        map.erase(key);
        map.insert(pair);
        rc = KHashTableAdd(hmap, (void*)&key, hash, (void*)&value);
    }

    capacity = random() % 20000;
    rc = KHashTableReserve(hmap, capacity);
    REQUIRE_RC(rc);

    size_t mapcount = map.size();
    size_t hmapcount = KHashTableCount(hmap);
    REQUIRE_EQ(mapcount, hmapcount);

    for (int i = 0; i != loops; ++i) {
        uint16_t key = random() % loops;
        uint16_t hvalue = 0;
        bool hfound = KHashTableFind(hmap, (void*)&key, hash, &hvalue);

        auto mapfound = map.find(key);
        if (mapfound == map.end()) {
            REQUIRE_EQ(hfound, false);
        } else {
            REQUIRE_EQ(hfound, true);
            uint16_t mvalue = mapfound->second;
            REQUIRE_EQ(hvalue, mvalue);
        }
    }
    KHashTableDispose(hmap, NULL, NULL, NULL);
}

#ifdef BENCHMARK
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

static uint64_t* benchkeys = NULL;
static const size_t numbenchkeys = 1 << 26;

static void make_benchkeys(void)
{
    if (benchkeys == NULL) {
        benchkeys = (uint64_t*)calloc(8, numbenchkeys);

        for (size_t i = 0; i != numbenchkeys; ++i)
            benchkeys[i] = random() * random() + random();
    }
}

TEST_CASE(Klib_stdunorderedSetBench)
{
    make_benchkeys();
    std::unordered_set<uint64_t> hset;

    for (unsigned long numelem = 4; numelem < (1ULL << 26); numelem *= 2) {
        hset.clear();

        stopwatch();
        for (size_t i = 0; i != numelem; i++) {
            hset.insert(benchkeys[i]);
        }
        size_t sz = hset.size();
        REQUIRE_EQ(sz, (size_t)numelem);
        printf("std::unordered_set ");
        printf("required %lu ms to insert %lu\n", stopwatch() / 1000,
               numelem);

        stopwatch();
        const long loops = 1000000;
        uint64_t c = 0;
        for (long loop = 0; loop != loops; loop++) {
            c += hset.count(loop);
        }
        unsigned long us = stopwatch();

        printf("Found %lu,", c);
        double lps = (double)loops / us;
        printf("numelem=%lu\t%.1f Mlookups/sec, ", numelem, lps);
        printf("load factor %f,", hset.load_factor());
        printf("buckets %ld,", hset.bucket_count());

        stopwatch();
        c = 0;
        for (long loop = 0; loop != loops; loop++) {
            c += hset.count(benchkeys[loop]);
        }
        us = stopwatch();

        printf("Random found %lu,", c);
        lps = (double)loops / us;
        printf("\t%.1f Mlookups/sec, ", lps);
        printf("load factor %f,", hset.load_factor());
        printf("buckets %ld,", hset.bucket_count());
        printf("\n");
    }
    printf("\n");
}

TEST_CASE(Klib_JudyBench)
{
    make_benchkeys();
    KVector* kv;
    for (unsigned long numelem = 4; numelem != (1ULL << 26); numelem *= 2) {
        KVectorMake(&kv);

        stopwatch();
        for (size_t i = 0; i != numelem; i++) {
            uint64_t key = benchkeys[i];
            uint64_t val = i + 1;
            KVectorSetU64(kv, key, val);
        }
        printf("Judy ");
        printf("required %lu ms to insert %lu\n", stopwatch() / 1000,
               numelem);

        stopwatch();
        unsigned long loops = 1000000;
        unsigned long c = 0;
        for (unsigned long loop = 0; loop != loops; loop++) {
            uint64_t key = loop;
            uint64_t val = loop + 1;
            size_t found = 0;
            rc_t rc = KVectorGetU64(kv, key, &found);
            if (found && found != val)
                fprintf(stderr, "miss %ld %ld\n", val, found);

            c += (rc == 0);
        }
        unsigned long us = stopwatch();

        printf("Judy Found %lu,", c);
        double lps = (double)loops / us;
        printf("numelem=%lu\t %.1f Mlookups/sec, ", numelem, lps);

        stopwatch();
        c = 0;
        for (unsigned long loop = 0; loop != loops; loop++) {
            uint64_t key = benchkeys[loop];
            size_t found = 0;
            KVectorGetU64(kv, key, &found);
            c += (found != 0);
        }
        us = stopwatch();

        printf("Random %lu,", c);
        lps = (double)loops / us;
        printf("\t%.1f Mlookups/sec, ", lps);
        printf("\n");

        KVectorRelease(kv);
    }
    printf("\n");
}

TEST_CASE(Klib_HashMapBench)
{
    make_benchkeys();
    KHashTable* hmap;

    for (unsigned long numelem = 4; numelem != (1ULL << 26); numelem *= 2) {
        rc_t rc = KHashTableMake(&hmap, 8, 8, 0, 0.0, raw);
        REQUIRE_RC(rc);

        size_t sz = KHashTableCount(hmap);
        REQUIRE_EQ(sz, (size_t)0);

        stopwatch();
        for (size_t i = 0; i != numelem; i++) {
            uint64_t key = benchkeys[i];
            uint64_t hash = KHash((char*)&key, 8);
            uint64_t val = i;
            rc = KHashTableAdd(hmap, &key, hash, (void*)&val);
            // Don't invoke REQUIRE_RC, affects benchmark
            //            REQUIRE_RC(rc);
        }
        sz = KHashTableCount(hmap);
        REQUIRE_EQ(sz, (size_t)numelem);
        printf("KHashTable ");
        printf("required %lu ms to insert %lu\n", stopwatch() / 1000,
               numelem);

        stopwatch();
        unsigned long loops = 1000000;
        unsigned long c = 0;
        for (unsigned long loop = 0; loop != loops; loop++) {
            uint64_t key = loop;
            uint64_t hash = KHash((char*)&key, 8);
            uint64_t val;
            bool found = KHashTableFind(hmap, &key, hash, &val);
            c += found;
        }
        unsigned long us = stopwatch();

        printf("Found %lu,", c);
        double lps = (double)loops / us;
        printf("numelem=%lu\t %.1f Mlookups/sec, ", numelem, lps);

        stopwatch();
        c = 0;
        for (unsigned long loop = 0; loop != loops; loop++) {
            uint64_t key = benchkeys[loop];
            uint64_t hash = KHash((char*)&key, 8);
            uint64_t val;
            bool found = KHashTableFind(hmap, &key, hash, &val);
            c += found;
        }
        us = stopwatch();

        if (numelem <= loops)
            REQUIRE_EQ(c, numelem);
        else
            REQUIRE_EQ(c, loops);

        printf("Random %lu,", c);
        lps = (double)loops / us;
        printf("\t%.1f Mlookups/sec, ", lps);
        printf("\n");

        KHashTableDispose(hmap, NULL, NULL, NULL);
    }
    printf("\n");
}

TEST_CASE(Klib_KHash_Speed)
{
    char key[8192];
    unsigned long loops = 1000000;

    for (uint64_t i = 0; i != sizeof(key); i++) key[i] = random();

    long len = 4;
    while (len < 10000) {
        stopwatch();
        for (unsigned long i = 0; i != loops; i++) KHash(key, len);

        unsigned long us = stopwatch();
        unsigned long hps = 1000000 * loops / us;
        unsigned long mbps = hps * len / 1048576;
        printf("KHash %lu %lu us elapsed (%lu hash/sec, %lu Mbytes/sec)\n",
               len, us, hps, mbps);

        len *= 2;
    }
}

TEST_CASE(Klib_string_hash_Speed)
{
    char key[8192];
    unsigned long loops = 1000000;

    for (uint64_t i = 0; i != sizeof(key); i++) key[i] = random();

    long len = 4;
    while (len < 10000) {
        stopwatch();
        for (uint64_t i = 0; i != loops; i++) string_hash(key, len);

        unsigned long us = stopwatch();
        unsigned long hps = 1000000 * loops / us;
        unsigned long mbps = hps * len / 1048576;
        printf(
            "string_hash %lu %lu us elapsed (%lu hash/sec, %lu Mbytes/sec)\n",
            len, us, hps, mbps);

        len *= 2;
    }
}

TEST_CASE(Klib_std_hash_Speed)
{
    unsigned long loops = 1000000;
    string str = "1234";

    std::size_t hash = 0;
    long len = 4;
    while (len < 10000) {
        stopwatch();
        for (uint64_t i = 0; i != loops; i++)
            hash += std::hash<std::string>{}(str);

        unsigned long us = stopwatch() + 1;
        unsigned long hps = 1000000 * loops / us;
        unsigned long mbps = hps * len / 1048576;
        printf(
            "std::hash %lu %lu us elapsed (%lu hash/sec, %lu Mbytes/sec)\n",
            len, us, hps, mbps);

        len *= 2;
        str += str;
    }
}

TEST_CASE(Klib_hash_hamming)
{
    char key[100];
    uint64_t mask = 0xfff;
    uint64_t hash_collisions[mask + 1];
    uint64_t khash_collisions[mask + 1];
    uint64_t rhash_collisions[mask + 1];

    for (uint64_t i = 0; i != mask + 1; i++) {
        hash_collisions[i] = 0;
        khash_collisions[i] = 0;
        rhash_collisions[i] = 0;
    }

    const char* foo1 = "ABCDE1";
    const char* foo2 = "ABCDE2";

    printf("khash of %s is %lx, %s is %lx\n", foo1, KHash(foo1, strlen(foo1)),
           foo2, KHash(foo2, strlen(foo2)));
    printf("string_hash of %s is %u, %s is %u\n", foo1,
           string_hash(foo1, strlen(foo1)), foo2,
           string_hash(foo2, strlen(foo2)));
    for (uint64_t i = 0; i != 10000000; i++) {
        sprintf(key, "ABCD%lu", i);
        uint64_t hash = string_hash(key, strlen(key));
        hash &= mask;
        hash_collisions[hash] = hash_collisions[hash] + 1;

        hash = KHash(key, strlen(key));
        hash &= mask;
        khash_collisions[hash] = khash_collisions[hash] + 1;

        hash = random();
        hash &= mask;
        rhash_collisions[hash] = rhash_collisions[hash] + 1;
    }

    uint64_t hash_max = 0;
    uint64_t khash_max = 0;
    uint64_t rhash_max = 0;
    for (uint64_t i = 0; i != mask; i++) {
        if (hash_collisions[i] > hash_max) hash_max = hash_collisions[i];
        if (khash_collisions[i] > khash_max) khash_max = khash_collisions[i];
        if (rhash_collisions[i] > rhash_max) rhash_max = rhash_collisions[i];
    }

    printf("string_hash longest probe is %lu\n", hash_max);
    printf("khash longest probe is %lu\n", khash_max);
    printf("rhash longest probe is %lu\n", rhash_max);
}

#endif // BENCHMARK

extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-hashtable";

rc_t CC KMain(int argc, char* argv[])
{
    srandom(time(NULL));
    KConfigDisableUserSettings();
    rc_t rc = KHashTableTestSuite(argc, argv);
    return rc;
}
}
