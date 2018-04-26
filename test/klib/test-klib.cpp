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
* Unit tests for klib interface
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
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace std;

//#define BENCHMARK

TEST_SUITE(KlibTestSuite);

///////////////////////////////////////////////// text
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

    std::set<uint64_t> set;

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
    printf("  Hashed %lu strings, %lu collisions\n", set.size(), collisions);
    REQUIRE_EQ(inserts, set.size());
}

TEST_CASE(Klib_KHashSet)
{
    const char* str1 = "Tu estas probando este hoy, no manana";
    const char* str2 = "Tu estas probando este hoy, no mananX";
    size_t size = strlen(str1);

    KHashTable hset;
    rc_t rc = KHashTableInit(&hset, 8, 0, 0, 0.0, true);
    REQUIRE_RC(rc);

    size_t sz = KHashTableCount(&hset);
    REQUIRE_EQ(sz, (size_t)0);

    double lf = KHashTableGetLoadFactor(&hset);
    REQUIRE_EQ(lf, (double)0.0);

    uint64_t hash = KHash(str1, size);
    rc = KHashTableAdd(&hset, str1, hash, NULL);
    REQUIRE_RC(rc);

    sz = KHashTableCount(&hset);
    REQUIRE_EQ(sz, (size_t)1);

    bool found;
    found = KHashTableFind(&hset, str1, hash, NULL);
    REQUIRE_EQ(found, true);

    found = KHashTableFind(&hset, str2, hash, NULL);
    REQUIRE_EQ(found, false);

    KHashTableWhack(&hset, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashMap)
{
    const char* str1 = "Tu estas probando este hoy, no manana";
    const char* str2 = "Tu estas probando este hoy, no mananX";

    KHashTable hmap;
    rc_t rc = KHashTableInit(&hmap, 8, 8, 0, 0, true);
    REQUIRE_RC(rc);

    size_t sz = KHashTableCount(&hmap);
    REQUIRE_EQ(sz, (size_t)0);

    uint64_t hash = 1;
    uint64_t val1 = 123;
    rc = KHashTableAdd(&hmap, str1, hash, &val1);
    REQUIRE_RC(rc);

    sz = KHashTableCount(&hmap);
    REQUIRE_EQ(sz, (size_t)1);

    rc = KHashTableAdd(&hmap, str1, hash, &val1);
    REQUIRE_RC(rc);

    sz = KHashTableCount(&hmap);
    REQUIRE_EQ(sz, (size_t)1);

    bool found;
    uint64_t val;
    found = KHashTableFind(&hmap, str1, hash, &val);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(val, (uint64_t)123);

    uint64_t val2 = 124;
    rc = KHashTableAdd(&hmap, str1, hash, &val2);
    REQUIRE_RC(rc);

    sz = KHashTableCount(&hmap);
    REQUIRE_EQ(sz, (size_t)1);

    found = KHashTableFind(&hmap, str1, hash, &val);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(val, (uint64_t)124);

    found = KHashTableFind(&hmap, str2, hash, &val);
    REQUIRE_EQ(found, false);

    uint64_t val3 = 125;
    rc = KHashTableAdd(&hmap, str2, hash, &val3);
    REQUIRE_RC(rc);

    found = KHashTableFind(&hmap, str2, hash, &val);
    REQUIRE_EQ(found, true);
    REQUIRE_EQ(val, (uint64_t)125);

    sz = KHashTableCount(&hmap);
    REQUIRE_EQ(sz, (size_t)2);

    KHashTableWhack(&hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashMapInts)
{
    rc_t rc;

    KHashTable hmap;
    rc = KHashTableInit(&hmap, 8, 8, 0, 0, false);
    REQUIRE_RC(rc);

    // Test probing, constant hash value
    uint64_t hash = random();
    hash = random() % 5;
    size_t count = 0;
    for (uint64_t i = 0; i != 100; i++) {
        uint64_t j = i * 3;
        rc = KHashTableAdd(&hmap, (void*)&i, hash, (void*)&j);
        REQUIRE_RC(rc);
        ++count;
        size_t sz = KHashTableCount(&hmap);
        REQUIRE_EQ(sz, count);
    }

    for (uint64_t i = 0; i != 100; i++) {
        bool found;
        uint64_t val;
        found = KHashTableFind(&hmap, (void*)&i, hash, &val);
        REQUIRE_EQ(found, true);
        REQUIRE_EQ(val, i * 3);
    }

    for (uint64_t i = 0; i != 10000; i++) {
        bool found;
        uint64_t key = i + 9999999;
        uint64_t hash = KHash((char*)&key, 8);
        found = KHashTableFind(&hmap, (void*)&key, hash, NULL);
        REQUIRE_EQ(found, false);
    }

    KHashTableWhack(&hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashMapStrings)
{
    rc_t rc;

    KHashTable hmap;
    rc = KHashTableInit(&hmap, 8, 8, 0, 0.0, true);
    REQUIRE_RC(rc);

    // TODO: Whack with destructors
    KHashTableWhack(&hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashMapInts2)
{
    rc_t rc;

    KHashTable hmap;
    rc = KHashTableInit(&hmap, 8, 8, 0, 0.0, false);
    REQUIRE_RC(rc);

    size_t count = 0;
    for (uint64_t i = 0; i != 1000; i++) {
        uint64_t j = i * 3;
        uint64_t hash = KHash((char*)&i, 8);
        rc = KHashTableAdd(&hmap, (void*)&i, hash, (void*)&j);
        REQUIRE_RC(rc);
        count++;
        size_t sz = KHashTableCount(&hmap);
        REQUIRE_EQ(sz, count);
    }

    for (uint64_t i = 0; i != 1000; i++) {
        bool found;
        uint64_t val;
        uint64_t hash = KHash((char*)&i, 8);
        found = KHashTableFind(&hmap, (void*)&i, hash, &val);
        REQUIRE_EQ(found, true);
        REQUIRE_EQ(val, i * 3);
    }

    for (uint64_t i = 0; i != 1000; i++) {
        bool found;
        uint64_t key = i + 9999999;
        uint64_t hash = KHash((char*)&key, 8);
        found = KHashTableFind(&hmap, (void*)&key, hash, NULL);
        REQUIRE_EQ(found, false);
    }

    KHashTableWhack(&hmap, NULL, NULL, NULL);
}

TEST_CASE(Klib_HashMapValid)
{
    rc_t rc;

    KHashTable hmap;
    rc = KHashTableInit(&hmap, 4, 4, 0, 0.0, false);
    REQUIRE_RC(rc);

    std::unordered_map<uint32_t, uint32_t> map;

    for (int i = 0; i != (4 * 1024 * 1024); ++i) {
        uint32_t key = random();
        uint32_t value = i;

        auto pair = std::make_pair(key, value);
        map.erase(key);
        map.insert(pair);
        uint64_t hash = KHash((char*)&key, 4);
        rc = KHashTableAdd(&hmap, (void*)&key, hash, (void*)&value);
    }

    size_t mapcount = map.size();
    size_t hmapcount = KHashTableCount(&hmap);
    REQUIRE_EQ(mapcount, hmapcount);

    for (int i = 0; i != 10000; ++i) {
        uint32_t key = random() % 1000;
        uint64_t hash = KHash((char*)&key, 4);
        uint32_t hvalue = 0;
        bool hfound = KHashTableFind(&hmap, (void*)&key, hash, &hvalue);

        auto mapfound = map.find(key);
        if (mapfound == map.end()) {
            REQUIRE_EQ(hfound, false);
        } else {
            REQUIRE_EQ(hfound, true);
            uint32_t mvalue = mapfound->second;
            REQUIRE_EQ(hvalue, mvalue);
        }
    }
    KHashTableWhack(&hmap, NULL, NULL, NULL);
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

TEST_CASE(Klib_stdunorderedSetBench)
{
    std::unordered_set<uint64_t> hset;

    for (unsigned long numelem = 4; numelem < (1ULL << 26); numelem *= 2) {
        hset.clear();

        stopwatch();
        uint64_t val = 0;
        for (size_t i = 0; i != numelem; i++) {
            hset.insert(val);
            val += 1;
        }
        size_t sz = hset.size();
        REQUIRE_EQ(sz, (size_t)numelem);
        printf("std::unordered_set ");
        printf("required %lu ms to insert %lu\n", stopwatch() / 1000,
               numelem);

        stopwatch();
        long loops = 1000000;
        val = 0;
        uint64_t c = 0;
        for (long loop = 0; loop != loops; loop++) {
            c += hset.count(val);
            val += 1;
        }
        unsigned long us = stopwatch();

        printf("Found %lu,", c);
        uint64_t lps = 1000000 * loops / us;
        printf("numelem=%lu\t%lu lookups/sec, ", numelem, lps);
        printf("load factor %f,", hset.load_factor());
        printf("buckets %d,", hset.bucket_count());

        stopwatch();
        loops = 1000000;
        c = 0;
        for (long loop = 0; loop != loops; loop++) {
            c += hset.count(val);
            val = random();
        }
        us = stopwatch();

        printf("Random found %lu,", c);
        lps = 1000000 * loops / us;
        printf("numelem=%lu\t%lu lookups/sec, ", numelem, lps);
        printf("load factor %f,", hset.load_factor());
        printf("buckets %d,", hset.bucket_count());
        printf("\n");
    }
    printf("\n");
}

TEST_CASE(Klib_JudyBench)
{
    KVector* kv;
    for (unsigned long numelem = 4; numelem != (1ULL << 26); numelem *= 2) {
        KVectorMake(&kv);

        stopwatch();
        uint64_t val = 0;
        for (size_t i = 0; i != numelem; i++) {
            val += 1;
            KVectorSetU64(kv, val, i);
        }

        stopwatch();
        unsigned long loops = 1000000;
        val = 0;
        unsigned long c = 0;
        for (long loop = 0; loop != loops; loop++) {
            size_t found;
            KVectorGetU64(kv, val, &found);

            c += found;
            val += 1;
        }
        unsigned long us = stopwatch();

        printf("Judy Found %lu,", c);
        uint64_t lps = 1000000 * loops / us;
        printf("numelem=%lu\t%lu lookups/sec, ", numelem, lps);

        stopwatch();
        loops = 1000000;
        c = 0;
        for (long loop = 0; loop != loops; loop++) {
            size_t found;
            KVectorGetU64(kv, val, &found);
            val = random();
        }
        us = stopwatch();

        printf("Random %lu,", c);
        lps = 1000000 * loops / us;
        printf("numelem=%lu\t%lu lookups/sec, ", numelem, lps);
        printf("\n");

        KVectorRelease(kv);
    }
    printf("\n");
}

TEST_CASE(Klib_HashMapBench)
{
    KHashTable hset;

    for (unsigned long numelem = 4; numelem != (1ULL << 26); numelem *= 2) {
        // rc_t rc = KHashTableInit(&hset, 8, 0, 1ULL << 26, 0.0, false);
        rc_t rc = KHashTableInit(&hset, 8, 0, 0, 0.0, false);
        REQUIRE_RC(rc);

        size_t sz = KHashTableCount(&hset);
        REQUIRE_EQ(sz, (size_t)0);

        stopwatch();
        uint64_t val = 0;
        for (size_t i = 0; i != numelem; i++) {
            uint64_t hash = KHash((char*)&val, 8);
            rc = KHashTableAdd(&hset, &val, hash, (void*)NULL);
            // Don't invoke REQUIRE_RC, affects benchmark
            //            REQUIRE_RC(rc);
            val += 1;
        }
        sz = KHashTableCount(&hset);
        REQUIRE_EQ(sz, (size_t)numelem);
        printf("KHashTable ");
        printf("required %lu ms to insert %lu\n", stopwatch() / 1000,
               numelem);

        stopwatch();
        unsigned long loops = 1000000;
        val = 0;
        unsigned long c = 0;
        for (long loop = 0; loop != loops; loop++) {
            uint64_t hash = KHash((char*)&val, 8);
            bool found = KHashTableFind(&hset, &val, hash, NULL);
            c += found;
            val += 1;
        }
        unsigned long us = stopwatch();

        printf("Found %lu,", c);
        uint64_t lps = 1000000 * loops / us;
        printf("numelem=%lu\t%lu lookups/sec, ", numelem, lps);
        double lf = KHashTableGetLoadFactor(&hset);
        printf("load factor %f,", lf);

        if (numelem <= loops)
            REQUIRE_EQ(c, numelem);
        else
            REQUIRE_EQ(c, loops);

        stopwatch();
        loops = 1000000;
        c = 0;
        for (long loop = 0; loop != loops; loop++) {
            uint64_t hash = KHash((char*)&val, 8);
            bool found = KHashTableFind(&hset, &val, hash, NULL);
            val = random();
        }
        us = stopwatch();

        printf("Random %lu,", c);
        lps = 1000000 * loops / us;
        printf("numelem=%lu\t%lu lookups/sec, ", numelem, lps);
        lf = KHashTableGetLoadFactor(&hset);
        printf("load factor %f,", lf);
        printf("\n");

        KHashTableWhack(&hset, NULL, NULL, NULL);
    }
    printf("\n");
}

TEST_CASE(Klib_KHash_Speed)
{
    char key[8192];
    long loops = 1000000;

    for (uint64_t i = 0; i != sizeof(key); i++) key[i] = random();

    long len = 4;
    while (len < 10000) {
        stopwatch();
        for (uint64_t i = 0; i != loops; i++) KHash(key, len);

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
    long loops = 1000000;

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
    long loops = 1000000;
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

TEST_CASE(Klib_text_string_len)
{
    // this is ASCII with a character count of 37, byte count of 37
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    uint32_t length = string_len(str, size);
    REQUIRE_EQ(length, (uint32_t)size);

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    length = string_len(str, size);
    REQUIRE_EQ(length, (uint32_t)(size - 4));
}

TEST_CASE(Klib_text_string_measure)
{
    // this is ASCII with a character count of 37, byte count of 37
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    size_t measure;
    uint32_t length = string_measure(str, &measure);
    REQUIRE_EQ(measure, size);
    REQUIRE_EQ(length, (uint32_t)size);

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    length = string_measure(str, &measure);
    REQUIRE_EQ(measure, size);
    REQUIRE_EQ(length, (uint32_t)(size - 4));
}

TEST_CASE(Klib_text_string_copy)
{
    char buff64[64];
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    size_t copied = string_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    char buff32[32];
    copied = string_copy(buff32, sizeof buff32, str, size);
    REQUIRE_EQ(copied, (size_t)(sizeof buff32));

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    copied = string_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    // this is the same string with a split UTF-8 character
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303";
    size = strlen(str);
    copied = string_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size - 1);
    REQUIRE_EQ((char)0, buff64[copied]);

    // test a copy with an embedded NUL
    const char EN[] = "This is the case of an\000embedded NUL byte";
    size = sizeof EN - 1;
    REQUIRE_EQ(size, (size_t)40);
    size = strlen(EN);
    REQUIRE_EQ(size, (size_t)22);
    copied = string_copy(buff64, sizeof buff64, EN, sizeof EN - 1);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
}

TEST_CASE(Klib_text_string_copy_measure)
{
    char buff64[64];
    const char* str = "Tu estas probando este hoy, no manana";
    size_t size = strlen(str);
    size_t copied = string_copy_measure(buff64, sizeof buff64, str);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    char buff32[32];
    copied = string_copy_measure(buff32, sizeof buff32, str);
    REQUIRE_EQ(copied, (size_t)(sizeof buff32));

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana";
    size = strlen(str);
    copied = string_copy_measure(buff64, sizeof buff64, str);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);

    // this is the same string with a split UTF-8 character
    str = "T\303\272 est\303\241s probando \303\251ste hoy, no ma\303";
    size = strlen(str);
    copied = string_copy_measure(buff64, sizeof buff64, str);
    REQUIRE_EQ(copied, size - 1);
    REQUIRE_EQ((char)0, buff64[copied]);

    // test a copy with an embedded NUL
    const char EN[] = "This is the case of an\000embedded NUL byte";
    size = sizeof EN - 1;
    REQUIRE_EQ(size, (size_t)40);
    size = strlen(EN);
    REQUIRE_EQ(size, (size_t)22);
    copied = string_copy_measure(buff64, sizeof buff64, EN);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
}

TEST_CASE(Klib_text_tolower_copy)
{
    char buff64[64];
    const char* str = "Tu Estas Probando Este Hoy, No Manana";
    size_t size = strlen(str);
    size_t copied = tolower_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
    int diff = strcmp(buff64, "tu estas probando este hoy, no manana");
    REQUIRE_EQ(diff, 0);

    char buff32[32];
    copied = tolower_copy(buff32, sizeof buff32, str, size);
    REQUIRE_EQ(copied, (size_t)(sizeof buff32));
    diff = memcmp(buff64, "tu estas probando este hoy, no manana", copied);
    REQUIRE_EQ(diff, 0);

    // this is UTF-8 with a character count of 37, byte count of 41
    str = "T\303\272 Est\303\241s Probando \303\211ste Hoy, No Ma\303\261ana";
    size = strlen(str);
    copied = tolower_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
#if 0
    std::cout
        << buff64
        << '\n'
        ;
    diff = strcmp ( buff64, "t\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana" );
    REQUIRE_EQ ( diff, 0 );
#endif

    // this is the same string with a split UTF-8 character
    str = "T\303\272 Est\303\241s Probando \303\211ste Hoy, No Ma\303";
    size = strlen(str);
    copied = tolower_copy(buff64, sizeof buff64, str, size);
    REQUIRE_EQ(copied, size - 1);
    REQUIRE_EQ((char)0, buff64[copied]);
#if 0
    diff = memcmp ( buff64, "t\303\272 est\303\241s probando \303\251ste hoy, no ma\303\261ana", copied );
    REQUIRE_EQ ( diff, 0 );
#endif

    // test a copy with an embedded NUL
    const char EN[] = "This Is The Case of an\000embedded NUL byte";
    size = sizeof EN - 1;
    REQUIRE_EQ(size, (size_t)40);
    size = strlen(EN);
    REQUIRE_EQ(size, (size_t)22);
    copied = tolower_copy(buff64, sizeof buff64, EN, sizeof EN - 1);
    REQUIRE_EQ(copied, size);
    REQUIRE_EQ((char)0, buff64[copied]);
    diff = memcmp(buff64, "this is the case of an\000embedded nul byte",
                  copied);
    REQUIRE_EQ(diff, 0);
}

TEST_CASE(KLib_text_StringToI64)
{
    rc_t rc;
    String str;
    int64_t val;

    CONST_STRING(&str, "12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   ++12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   ++12345678 ");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcTransfer);
    REQUIRE_EQ(GetRCState(rc), rcIncomplete);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   ++c12345678 ");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), (RCObject)rcData);
    REQUIRE_EQ(GetRCState(rc), rcInsufficient);
    REQUIRE_EQ(val, (int64_t)0);

    CONST_STRING(&str, "   -++-12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)12345678);

    CONST_STRING(&str, "   -++12345678");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (int64_t)-12345678);

    CONST_STRING(&str, "");
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, (int64_t)0);

    CONST_STRING(&str, "0");
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, (int64_t)0);

    CONST_STRING(&str, "1");
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, (int64_t)1);

    CONST_STRING(&str, "9223372036854775805");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(9223372036854775805));

    CONST_STRING(&str, "9223372036854775807");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));

    CONST_STRING(&str, "9223372036854775808");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));

    CONST_STRING(&str, "92233720368547758071");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, INT64_C(9223372036854775807));

    CONST_STRING(&str, "-9223372036854775805");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(-9223372036854775805));

    CONST_STRING(&str, "-9223372036854775807");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807));

    // the value -9223372036854775808 causes complaint
    // because it is first parsed as signed, which overflows
    // and then is negated after the complaint
    // use -9223372036854775807 - 1
    // WHY AREN'T INT64_MIN/MAX WORKING?

    CONST_STRING(&str, "-9223372036854775808");
    val = StringToI64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807) - INT64_C(1));

    CONST_STRING(&str, "-9223372036854775809");
    val = StringToI64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807) - INT64_C(1));
    val = StringToI64(&str, 0);
    REQUIRE_EQ(val, INT64_C(-9223372036854775807) - INT64_C(1));
}

TEST_CASE(KLib_text_StringToU64)
{
    rc_t rc;
    String str;
    uint64_t val;

    CONST_STRING(&str, "12345678");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (uint64_t)12345678);

    CONST_STRING(&str, "   12345678");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, (uint64_t)12345678);

    CONST_STRING(&str, "   ++12345678");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), (RCObject)rcData);
    REQUIRE_EQ(GetRCState(rc), rcInsufficient);
    REQUIRE_EQ(val, (uint64_t)0);

    CONST_STRING(&str, "   12345678 ");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcTransfer);
    REQUIRE_EQ(GetRCState(rc), rcIncomplete);
    REQUIRE_EQ(val, (uint64_t)12345678);

    CONST_STRING(&str, "");
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, (uint64_t)0);

    CONST_STRING(&str, "0");
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, (uint64_t)0);

    CONST_STRING(&str, "1");
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, (uint64_t)1);

    CONST_STRING(&str, "18446744073709551610");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, UINT64_C(18446744073709551610));

    CONST_STRING(&str, "18446744073709551615");
    val = StringToU64(&str, &rc);
    REQUIRE_RC(rc);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));

    CONST_STRING(&str, "18446744073709551616");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));
    val = StringToU64(&str, 0);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));

    CONST_STRING(&str, "184467440737095516151");
    val = StringToU64(&str, &rc);
    REQUIRE_RC_FAIL(rc);
    REQUIRE_EQ(GetRCObject(rc), rcRange);
    REQUIRE_EQ(GetRCState(rc), rcExcessive);
    REQUIRE_EQ(val, UINT64_C(18446744073709551615));
}

///////////////////////////////////////////////// ksort

static char UserData[] = "User data";

// this is used by qsort and (indirectly) ksort
int CC baseCompare(const void* a, const void* b)
{
    const char* pa = (const char*)a;
    const char* pb = (const char*)b;
    if (*pa < *pb) {
        return -1;
    }
    if (*pa > *pb) {
        return 1;
    }
    return 0;
}
// this is used by ksort
int64_t CC compare(const void* a, const void* b, void* data)
{
    // if data is not pointing where we waint it to, do not sort
    const char* pdata = (const char*)data;
    if (pdata != 0
        && string_cmp(pdata, string_size(pdata), UserData,
                      string_size(UserData), (uint32_t)string_size(UserData))
            != 0) {
        return 0;
    }
    return baseCompare(a, b);
}

TEST_CASE(KLib_ksort_no_elements)
{
    char arr[1] = {13};
    ksort(arr, 0, sizeof(char), compare, UserData);
    // do not crash or change the target
    REQUIRE_EQ(arr[0], (char)13);
}
TEST_CASE(KLib_ksort_one_element)
{
    char arr[1] = {13};
    ksort(arr, 1, sizeof(char), compare, UserData);
    // do not crash or change the target
    REQUIRE_EQ(arr[0], (char)13);
}

TEST_CASE(KLib_ksort_simple)
{
    const int Size = 5;
    char arr[Size] = {2, 4, 1, 0, 3};
    ksort(arr, Size, sizeof(char), compare, UserData);
    REQUIRE_EQ(arr[0], (char)0);
    REQUIRE_EQ(arr[1], (char)1);
    REQUIRE_EQ(arr[2], (char)2);
    REQUIRE_EQ(arr[3], (char)3);
    REQUIRE_EQ(arr[4], (char)4);
}
TEST_CASE(KLib_ksort_vs_qsort)
{
    const int Size = 5;
    char karr[Size] = {2, 4, 1, 0, 3};
    char qarr[Size];
    memmove(qarr, karr, sizeof(karr));

    ksort(karr, Size, sizeof(char), compare, 0); // do not pass any user data
    qsort(qarr, Size, sizeof(char), baseCompare);
    REQUIRE_EQ(memcmp(karr, qarr, sizeof(karr)), 0);
}

// an example of a bad function (compating pointers not values, in reverse
// order) that causes an implementaion of ksort to crash.
int64_t CC badCompare(const void* a, const void* b, void* data)
{
    const char* pa = (const char*)a;
    const char* pb = (const char*)b;
    if (pa < pb) {
        return 1;
    }
    if (pa > pb) {
        return -1;
    }
    return 0;
}
TEST_CASE(KLib_ksort_problem)
{
    const int Size = 5;
    {
        char arr[Size] = {2, 4, 1, 0, 3};
        ksort(arr, Size, sizeof(char), badCompare, 0);
        // we just do not want this to crash since the compare function is not
        // working properly and the eventual sort order is undefined
    }
}

///////////////////////////////////////////////// macro based ksort

TEST_CASE(KSORT_simple)
{
    char karr[] = {2, 4, 1, 0, 3};
    const int ElemSize = sizeof(karr[0]);
    const int Size = sizeof(karr) / ElemSize;
    char qarr[Size];
    memmove(qarr, karr, sizeof(karr));

#define CMP(a, b) (*(char*)(a) < *(char*)(b) ? -1 : *(char*)(a) > *(char*)(b))
#define SWAP(a, b, offset, size)
    KSORT(karr, Size, ElemSize, 0, ElemSize);
    ksort(qarr, Size, ElemSize, compare, 0);
    REQUIRE_EQ(memcmp(karr, qarr, sizeof(karr)), 0);
#undef CMP
#undef SWAP
}

static int64_t CC cmp_int64_t(const void* a, const void* b, void* data)
{
    const int64_t* ap = (const int64_t*)a;
    const int64_t* bp = (const int64_t*)b;

    if (*ap < *bp) return -1;
    return *ap > *bp;
}

TEST_CASE(KSORT_int64)
{
    int64_t karr[] = {2, 4, 1};
    const int ElemSize = sizeof(karr[0]);
    const int Size = sizeof(karr) / ElemSize;
    int64_t qarr[Size];
    memmove(qarr, karr, sizeof(karr));

    ksort_int64_t(karr, Size);
    ksort(qarr, Size, ElemSize, cmp_int64_t, 0);
    REQUIRE_EQ(memcmp(karr, qarr, sizeof(karr)), 0);
}

///////////////////////////////////////////////// string_printf
TEST_CASE(KLib_print_uint64)
{
    char dst[1024];
    size_t num_writ;
    uint64_t val = UINT64_C(1152921504606846976);
    REQUIRE_RC(string_printf(dst, sizeof(dst), &num_writ, "%lu", val));
    REQUIRE_EQ(string(dst, num_writ), string("1152921504606846976"));
}

#ifndef WINDOWS
TEST_CASE(KLib_print_problem)
{
    char dst[1024];
    size_t num_writ;
    double d = 1.0;
    REQUIRE_RC(string_printf(dst, sizeof(dst), &num_writ, "%.6f", d));
    REQUIRE_EQ(num_writ, strlen(dst));
    REQUIRE_EQ(string("1.000000"), string(dst, num_writ));
}
#endif

///////////////////////////////////////////////// KDataBuffer

TEST_CASE(KDataBuffer_Make)
{
    KDataBuffer src;

    REQUIRE_RC(KDataBufferMake(&src, 13, 9876));

    REQUIRE(src.ignore != 0);
    REQUIRE(src.base != 0);
    REQUIRE_EQ((uint64_t)13, src.elem_bits);
    REQUIRE_EQ((uint64_t)9876, src.elem_count);
    REQUIRE_EQ((bitsz_t)(13 * 9876), KDataBufferBits(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_MakeBytes)
{
    KDataBuffer src;

    REQUIRE_RC(KDataBufferMakeBytes(&src, 12));

    REQUIRE_EQ((uint64_t)8, src.elem_bits);
    REQUIRE_EQ((uint64_t)12, src.elem_count);
    REQUIRE_EQ((bitsz_t)(12 * 8), KDataBufferBits(&src));
    REQUIRE_EQ((size_t)12, KDataBufferBytes(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_MakeBits)
{
    KDataBuffer src;

    REQUIRE_RC(KDataBufferMakeBits(&src, 8));

    REQUIRE_EQ((uint64_t)1, src.elem_bits);
    REQUIRE_EQ((uint64_t)8, src.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * 8), KDataBufferBits(&src));
    REQUIRE_EQ((size_t)1, KDataBufferBytes(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_MakeBits1)
{
    KDataBuffer src;
    const size_t BIT_SZ = 7896;
    REQUIRE_RC(KDataBufferMakeBits(&src, BIT_SZ));

    REQUIRE_EQ((uint64_t)1, src.elem_bits);
    REQUIRE_EQ((uint64_t)BIT_SZ, src.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * BIT_SZ), KDataBufferBits(&src));
    REQUIRE_EQ((size_t)((1 * BIT_SZ + 7) / 8), KDataBufferBytes(&src));
    REQUIRE_EQ((uint8_t)src.bit_offset, (uint8_t)0);

    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_Sub)
{
    KDataBuffer src;
    KDataBuffer sub;

    const size_t BIT_SZ = 7896;
    REQUIRE_RC(KDataBufferMakeBits(&src, BIT_SZ));

    REQUIRE_RC(KDataBufferSub(&src, &sub, 800, 900));

    REQUIRE_EQ(src.elem_bits, sub.elem_bits);
    REQUIRE_EQ((uint64_t)900, sub.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * 900), KDataBufferBits(&sub));
    REQUIRE_EQ((uint8_t*)sub.base, (uint8_t*)src.base + (800 >> 3));
    REQUIRE_EQ((uint64_t)sub.bit_offset, (uint64_t)0);

    KDataBufferWhack(&src);
    KDataBufferWhack(&sub);
}

TEST_CASE(KDataBuffer_Sub1)
{
    KDataBuffer src;
    KDataBuffer sub;

    const size_t BIT_SZ = 7896;
    REQUIRE_RC(KDataBufferMakeBits(&src, BIT_SZ));

    REQUIRE_RC(KDataBufferSub(&src, &sub, 801, 900));

    REQUIRE_EQ(src.elem_bits, sub.elem_bits);
    REQUIRE_EQ((uint64_t)900, sub.elem_count);
    REQUIRE_EQ((bitsz_t)(1 * 900), KDataBufferBits(&sub));
    REQUIRE_EQ((uint8_t*)sub.base, (uint8_t*)src.base + (800 >> 3));
    REQUIRE_EQ((uint64_t)sub.bit_offset, (uint64_t)1);

    KDataBufferWhack(&src);
    KDataBufferWhack(&sub);
}

TEST_CASE(KDataBuffer_MakeWritable)
{
    KDataBuffer src;
    KDataBuffer copy;

    REQUIRE_RC(KDataBufferMakeBytes(&src, 12));

    REQUIRE_RC(KDataBufferMakeWritable(&src, &copy));

    REQUIRE_EQ(src.elem_bits, copy.elem_bits);
    REQUIRE_EQ((uint64_t)12, copy.elem_count);
    REQUIRE_EQ(copy.base, src.base);
    REQUIRE_EQ((uint64_t)copy.bit_offset, (uint64_t)0);

    KDataBufferWhack(&src);
    KDataBufferWhack(&copy);
}

TEST_CASE(KDataBuffer_MakeWritable1)
{
    KDataBuffer src;
    KDataBuffer sub;
    KDataBuffer copy;

    REQUIRE_RC(KDataBufferMakeBytes(&src, 256));

    REQUIRE_RC(KDataBufferSub(&src, &sub, 8, 12));

    REQUIRE_RC(KDataBufferMakeWritable(&sub, &copy));

    REQUIRE_EQ(src.elem_bits, copy.elem_bits);
    REQUIRE_EQ((uint64_t)12, sub.elem_count);
    REQUIRE_EQ((uint64_t)12, copy.elem_count);
    REQUIRE_NE(sub.base, copy.base);
    REQUIRE_EQ((uint64_t)copy.bit_offset, (uint64_t)0);

    KDataBufferWhack(&src);
    KDataBufferWhack(&sub);
    KDataBufferWhack(&copy);
}

TEST_CASE(KDataBuffer_Resize)
{
    KDataBuffer src;
    uint32_t blob_size = 4096;
    REQUIRE_RC(KDataBufferMake(&src, 8, blob_size));

    /* make sub-buffer from input */
    KDataBuffer dst;
    uint32_t hdr_size = 7;
    REQUIRE_RC(KDataBufferSub(&src, &dst, hdr_size, blob_size));
    /* cast from 8 into 2 bits */
    REQUIRE_RC(KDataBufferCast(&dst, &dst, 2, true));
    /* resize to 4 times the original number of elements */
    REQUIRE_RC(KDataBufferResize(&dst, (blob_size - hdr_size) * 4));

    KDataBufferWhack(&dst);
    KDataBufferWhack(&src);
}

TEST_CASE(KDataBuffer_Cast_W32Assert)
{
    KDataBuffer src;
    REQUIRE_RC(KDataBufferMake(&src, 64, 1));
    REQUIRE_RC(KDataBufferCast(&src, &src, 64,
                               true)); /* used to throw am assert on Win32 */
    KDataBufferWhack(&src);
}

//////////////////////////////////////////// Log
TEST_CASE(KLog_Formatting)
{
    unsigned long status = 161;
    REQUIRE_RC(pLogErr(klogInfo, 0, "$(E) - $(C)", "E=%!,C=%u", status,
                       status)); // fails on Windows
}

TEST_CASE(KLog_LevelExplain)
{
    char buf[5];
    size_t num_writ = 0;
    REQUIRE_RC(KLogLevelExplain(klogInfo, buf, sizeof(buf), &num_writ));
    REQUIRE_EQ(string("info"), string(buf));
    REQUIRE_EQ(num_writ, strlen(buf));
}
TEST_CASE(KLog_LevelExplainNoRoomforNul)
{
    char buf[4];
    size_t num_writ = 0;
    REQUIRE_RC(KLogLevelExplain(klogInfo, buf, sizeof(buf), &num_writ));
    REQUIRE_EQ(string("info"), string(buf, 4));
    REQUIRE_EQ(num_writ, (size_t)4);
}
TEST_CASE(KLog_LevelExplainInsufficientBuffer)
{
    char buf[3];
    size_t num_writ = 0;
    REQUIRE_RC_FAIL(KLogLevelExplain(klogInfo, buf, sizeof(buf), &num_writ));
    REQUIRE_EQ(num_writ, (size_t)0);
}

TEST_CASE(IsUserAnAdminTest)
{
// TeamCity agents run as admin on some systems but not the others
#if defined(WINDOWS)
    if (getenv("TEAMCITY_VERSION") != 0) {
        REQUIRE(is_iser_an_admin());
    } else {
        REQUIRE(!is_iser_an_admin());
    }
#else
    // Linux or not under TeamCity
    REQUIRE(!is_iser_an_admin());
#endif
}

static const size_t BufSize = 1024;
// implementation of KWrtWriter for testing purposes
rc_t CC TestWrtWriter(void* data, const char* buffer, size_t bufsize,
                      size_t* num_writ)
{
    size_t nw = string_copy((char*)data, BufSize, buffer, bufsize);
    if (num_writ != 0) *num_writ = nw;
    return 0;
}

TEST_CASE(KLog_DefaultFormatterNull)
{
    REQUIRE_RC(KLogFmtWriterGet()(NULL, NULL, 0, NULL, 0, NULL));
}
TEST_CASE(KLog_DefaultFormatterBasic)
{
    char buffer[BufSize];
    KWrtHandler testHandler = {TestWrtWriter, (void*)buffer};
    wrt_nvp_t args[] = {// ignored
                        {"", ""}};
    wrt_nvp_t envs[] = {
        // has to be sorted by key, apparently...
        {"app", "test"},     {"message", "msg"},    {"reason", "rc"},
        {"severity", "bad"}, {"timestamp", "time"}, {"version", "v1"},
    };
    REQUIRE_RC(KLogFmtWriterGet()(NULL, &testHandler,
                                  sizeof(args) / sizeof(args[0]), args,
                                  sizeof(envs) / sizeof(envs[0]), envs));
    REQUIRE_EQ(string("time test.v1 bad: rc - msg\n"),
               string((const char*)testHandler.data));
}

// implementation of KFmtWriter for testing purposes
rc_t CC TestFmtWriter(void* self, KWrtHandler* writer, size_t argc,
                      const wrt_nvp_t args[], size_t envc,
                      const wrt_nvp_t envs[])
{
    char buffer[BufSize];
    size_t num_writ;
    string_printf(buffer, sizeof(buffer), &num_writ, "%s",
                  wrt_nvp_find_value(envc, envs, "message"));

    size_t size;
    return LogFlush(writer, buffer, string_measure(buffer, &size));
}
TEST_CASE(KLog_ParamFormatting)
{
    char buffer[BufSize];
    REQUIRE_RC(KLogHandlerSet(TestWrtWriter, buffer));
    REQUIRE_RC(KLogFmtHandlerSet(TestFmtWriter, 0, buffer));
    REQUIRE_RC(pLogMsg(klogErr, "message with $(PARAM1) $(P2) etc.",
                       "PARAM1=%s,P2=%s", "parameter1", "param2"));
    REQUIRE_EQ(string("message with parameter1 param2 etc."), string(buffer));
}

//////////////////////////////////////////// num-gen
#define SHOW_UNIMPLEMENTED 0
class NumGenFixture
{
  public:
    NumGenFixture() : m_ng(0), m_it(0) {}
    ~NumGenFixture()
    {
        if (m_it && num_gen_iterator_destroy(m_it))
            throw logic_error(
                "NumGenFixture: num_gen_iterator_destroy failed");

        if (m_ng && num_gen_destroy(m_ng) != 0)
            throw logic_error("NumGenFixture: num_gen_destroy failed");
    }

    struct num_gen* m_ng;
    const struct num_gen_iter* m_it;
};

FIXTURE_TEST_CASE(num_gen_Make, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE_NOT_NULL(m_ng);
}
FIXTURE_TEST_CASE(num_gen_MakeFromStr, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_str(&m_ng, "1"));
    REQUIRE_NOT_NULL(m_ng);
}

FIXTURE_TEST_CASE(num_gen_MakeFromEmptyStr, NumGenFixture)
{
    REQUIRE_RC_FAIL(num_gen_make_from_str(&m_ng, ""));
    REQUIRE_NULL(m_ng);
}

FIXTURE_TEST_CASE(num_gen_MakeFromRange, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE_NOT_NULL(m_ng);
}
FIXTURE_TEST_CASE(num_gen_MakeFromEmptyRange, NumGenFixture)
{
    REQUIRE_RC_FAIL(num_gen_make_from_range(&m_ng, 1, 0));
    REQUIRE_NULL(m_ng);
}

FIXTURE_TEST_CASE(num_gen_Empty, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE(num_gen_empty(m_ng));
}

FIXTURE_TEST_CASE(num_gen_Clear, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE(!num_gen_empty(m_ng));

    REQUIRE_RC(num_gen_clear(m_ng));

    REQUIRE(num_gen_empty(m_ng));
}

#if SHOW_UNIMPLEMENTED
FIXTURE_TEST_CASE(num_gen_EmptyRange, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 0));
    REQUIRE_NOT_NULL(m_ng);
    REQUIRE(num_gen_empty(m_ng));
}
#endif

FIXTURE_TEST_CASE(num_gen_IteratorMake, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));
}

FIXTURE_TEST_CASE(num_gen_IteratorCount, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));

    uint64_t count;
    REQUIRE_RC(num_gen_iterator_count(m_it, &count));
    REQUIRE_EQ((uint64_t)2, count);
}
FIXTURE_TEST_CASE(num_gen_IteratorCountEmpty, NumGenFixture)
{
    REQUIRE_RC(num_gen_make(&m_ng));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));

    uint64_t count;
    REQUIRE_RC(num_gen_iterator_count(m_it, &count));
    REQUIRE_EQ((uint64_t)0, count);
}

FIXTURE_TEST_CASE(num_gen_IteratorNext, NumGenFixture)
{
    REQUIRE_RC(num_gen_make_from_range(&m_ng, 1, 2));
    REQUIRE_RC(num_gen_iterator_make(m_ng, &m_it));

    int64_t value;
    rc_t rc = 0;
    REQUIRE(num_gen_iterator_next(m_it, &value, &rc));
    REQUIRE_EQ((int64_t)1, value);
    REQUIRE_RC(rc);

    REQUIRE(num_gen_iterator_next(m_it, &value, &rc));
    REQUIRE_EQ((int64_t)2, value);
    REQUIRE_RC(rc);

    REQUIRE(!num_gen_iterator_next(m_it, &value, &rc));
    REQUIRE_RC(rc);
}

// TODO:
// rc_t num_gen_parse( struct num_gen * self, const char * src );
// rc_t num_gen_add( struct num_gen * self, const uint64_t first, const
// uint64_t count );
// rc_t num_gen_trim( struct num_gen * self, const int64_t first, const
// uint64_t count );
// rc_t num_gen_as_string( const struct num_gen * self, char * buffer, size_t
// buffsize, size_t * written, bool full_info );
// rc_t num_gen_contains_value( const struct num_gen * self, const uint64_t
// value );
// rc_t num_gen_range_check( struct num_gen * self, const int64_t first, const
// uint64_t count );
// rc_t num_gen_iterator_percent( const struct num_gen_iter * self, uint8_t
// fract_digits, uint32_t * value );

// Error reporting
#if _DEBUGGING
TEST_CASE(GetUnreadRCInfo_LogRC)
{ // bug report: only 1 RC is reported at the end of KMane in _DEBUGGING mode

    // create a couple of RCs
    RC(rcApp, rcFile, rcConstructing, rcFile, rcNull);
    RC(rcXF, rcFunction, rcExecuting, rcParam, rcInvalid);

    rc_t rc;
    uint32_t lineno;
    const char *filename, *function;
    REQUIRE(GetUnreadRCInfo(&rc, &filename, &function, &lineno));
    // bug: call to pLogErr invokes GetRCFunction (and others alike),
    // which as a side effect changes "last read RC" to equal "last written
    // RC"
    // causing the subsequent call to GetUnreadRCInfo to return "no more
    // unread RCs"
    pLogErr(klogWarn, rc, "$(filename):$(lineno) within $(function)",
            "filename=%s,lineno=%u,function=%s", filename, lineno, function);
    REQUIRE(GetUnreadRCInfo(&rc, &filename, &function, &lineno));
}
#endif

//////////////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-klib";

rc_t CC KMain(int argc, char* argv[])
{
    srandom(time(NULL));
    KConfigDisableUserSettings();
    rc_t rc = KlibTestSuite(argc, argv);
    return rc;
}
}
