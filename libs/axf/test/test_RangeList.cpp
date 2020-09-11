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

#include <iostream>
#include <string>
#include <cstdlib> // for abort, getenv, random

#define TESTING 1
#include "range-list.hpp"

#undef TESTING

static int verbose = 0;
#define VERBOSE(N) if (verbose < N) {} else std::cerr
// << "[ " <<__FILE__ << ":" << __LINE__ << " ] "
#define ERROR std::cerr << "[ " <<__FILE__ << ":" << __LINE__ << " ] "

int main(int argc, char **argv)
{
    {
        auto const envvar = getenv("VERBOSE");
        if (envvar)
            verbose = std::stoi(std::string(envvar));
        std::cerr << "verbose: " << verbose << std::endl;
    }
    srandom(time(0));

    if (cRange::test())
        std::cerr << "cRange::test ok" << std::endl;
    else
        return 1;

    if (cRangeList::test())
        std::cerr << "cRangeList::test ok" << std::endl;
    else
        return 1;

    return 0;
}

/// Generate a random sub-range.
/// @return a range that is fully contained or is equal to self.
cRange cRange::randomSubRange() const {
    double r1 = random() / (double)(RAND_MAX);
    double r2 = random() / (double)(RAND_MAX);
    unsigned const sublen = r1 * (end - start);
    unsigned const offset = r2 * ((end - start) - sublen);
    auto const result = cRange(start + offset, start + offset + sublen);
    assert (result.empty() || this->contains(result));
    return result;
}

/// Checks that a large number of randomly generated ranges correctly intersect the list.
/// @param list the list to be tested.
/// @param wholeRange the range from which the random sub-ranges will be generated.
static bool randomTest(cRangeList const &list, cRange const &wholeRange)
{
    auto const testcount = 1000000; assert(testcount > wholeRange.end * wholeRange.end);

    VERBOSE(1) << "testing " << testcount << " random sub-ranges for intersection (" << (testcount / ((double)wholeRange.end * wholeRange.end)) << "x oversampling)" << std::endl;

    for (auto _ = 0; _ < testcount; ++_) {
        auto const sub = wholeRange.randomSubRange();
        auto const intersecting = list.intersecting(sub);
        auto i = list.begin();
        auto j = intersecting.begin();

        if (verbose >= 5) {
            VERBOSE(5) << "intersected with " << sub << ": [ ";
            for (auto && r : intersecting)
                std::cerr << r << " ";
            std::cerr << "]" << std::endl;
        }
        if (sub.empty()) {
            if (intersecting.begin() != intersecting.end()) {
                ERROR << "intersect test failed: " << sub << " should not intersect" << std::endl;
                std::cerr << "[ ";
                for (auto && r : intersecting)
                    std::cerr << r << " ";
                std::cerr << "]" << std::endl;
                return false;
            }
            continue;
        }
        // verify that every range before the intersecting list does NOT intersect
        for ( ; i != list.end(); ++i) {
            if (i == j) break;
            if (i->intersects(sub)) {
                ERROR << "intersect test failed: " << *i << " and " << sub << " should not intersect" << std::endl;
                return false;
            }
            else {
                VERBOSE(4) << *i << " and " << sub << " do not intersect" << std::endl;
            }
        }
        // verify that every range in the intersecting list does intersect
        for ( ; i != list.end() && j != intersecting.end(); ++i, ++j) {
            assert(i == j);
            if (!i->intersects(sub)) {
                ERROR << "intersect test failed: " << *i << " and " << sub << " should intersect" << std::endl;
                return false;
            }
            else {
                VERBOSE(4) << *i << " and " << sub << " do intersect" << std::endl;
            }
        }
        // verify that every range after the intersecting list does NOT intersect
        for ( ; i != list.end(); ++i) {
            if (i->intersects(sub)) {
                ERROR << "intersect test failed: " << *i << " and " << sub << " should not intersect" << std::endl;
                return false;
            }
            else {
                VERBOSE(4) << *i << " and " << sub << " do not intersect" << std::endl;
            }
        }
    }
    return true;
}

/// Checks for the expected number of intersecting ranges.
static bool testIntersection(cRangeList const &list, cRange const &testvalue, unsigned intersects)
{
    VERBOSE(1) << testvalue;
    auto const intersection = list.intersect(testvalue);

    VERBOSE(1) << " -> " << intersection << std::endl;
    return intersection.size() == intersects;
}

static bool test_append_0()
{
    cRangeList list;

    list.append(cRange(95739, 100000));

    list.add(0);

    return true;
}

/// The first set of tests only check for the expected number of intersecting ranges.
/// The last test checks that a large number of randomly generated ranges correctly intersect the list.
bool cRangeList::test() {
    auto const first = 10;
    cRangeList list;

    list.append(cRange(first + 0, first + 20))
        .append(cRange(first + 30, first + 50))
        .append(cRange(first + 70, first + 90));

    VERBOSE(1) << "list: " << list << std::endl;

    test_append_0();
    
    { // before the first range
        auto const testvalue = cRange(0, first);
        if (!testIntersection(list, testvalue, 0)) {
            ERROR << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }

    { // entirely within the first range
        auto const testvalue = cRange(first + 10, first + 20);
        if (!testIntersection(list, testvalue, 1)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // starts within the first range
        auto const testvalue = cRange(first + 10, first + 30);
        if (!testIntersection(list, testvalue, 1)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // starts within the first range, ends within the second range
        auto const testvalue = cRange(first + 10, first + 40);
        if (!testIntersection(list, testvalue, 2)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // starts within the first range, ends within the third range
        auto const testvalue = cRange(first + 10, first + 80);
        if (!testIntersection(list, testvalue, 3)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // entirely between first and second ranges
        auto const testvalue = cRange(first + 20, first + 30);
        if (!testIntersection(list, testvalue, 0)) {
            ERROR << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }

    { // starts within the first gap, ends within the second gap
        auto const testvalue = cRange(first + 25, first + 60);
        if (!testIntersection(list, testvalue, 1)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // ends in second range
        auto const testvalue = cRange(first + 20, first + 40);
        if (!testIntersection(list, testvalue, 1)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // entirely in second range
        auto const testvalue = cRange(first + 30, first + 40);
        if (!testIntersection(list, testvalue, 1)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // starts in second range
        auto const testvalue = cRange(first + 49, first + 69);
        if (!testIntersection(list, testvalue, 1)) {
            ERROR << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }

    { // beyond second range
        auto const testvalue = cRange(first + 50, first + 70);
        if (!testIntersection(list, testvalue, 0)) {
            ERROR << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }

    { // beyond last range
        auto const testvalue = cRange(first + 100, first + 170);
        if (!testIntersection(list, testvalue, 0)) {
            ERROR << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }

    if (!randomTest(list, cRange(0, 200)))
        return false;

    list.add(first + 90);
    VERBOSE(2) << "list: " << list << std::endl;
    if (!list.check() || list[2].end != first + 91) {
        ERROR << "extend last test failed: " << list << std::endl;
        return false;
    }
    list.add(first + 50);
    VERBOSE(2) << "list: " << list << std::endl;
    if (!list.check() || list[1].end != first + 51) {
        ERROR << "extend middle test failed: " << list << std::endl;
        return false;
    }
    list.add(first + 100);
    VERBOSE(2) << "list: " << list << std::endl;
    if (!list.check() || list.size() != 4 || (list[3].start != first + 100 && list[3].end != first + 101)) {
        ERROR << "extend new at end test failed: " << list << std::endl;
        return false;
    }
    list.add(first + 60);
    VERBOSE(2) << "list: " << list << std::endl;
    if (!list.check() || list.size() != 5 || (list[2].start != first + 60 && list[2].end != first + 61)) {
        ERROR << "extend new test failed: " << list << std::endl;
        return false;
    }
    list.add(5);
    VERBOSE(2) << "list: " << list << std::endl;
    if (!list.check() || list.size() != 6 || (list[0].start != 5 && list[0].end != 6)) {
        ERROR << "extend new at front test failed: " << list << std::endl;
        return false;
    }
    list.add(first + 91);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 92);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 93);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 94);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 95);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 96);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 97);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 98);
    VERBOSE(3) << "list: " << list << std::endl;
    list.add(first + 99);
    VERBOSE(2) << "list: " << list << std::endl;
    if (list.size() != 5 || (list[4].start != first + 70 && list[4].end != 101)) {
        ERROR << "extend to collapse test failed: " << list << std::endl;
        return false;
    }
    return true;
}

bool cRange::test() {
    auto const r = cRange(0, 20);
    {
        if (!(r == r.intersectedWith(r))) {
            ERROR << "intersectedWith test failed: a range intersected with itself must equal itself" << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = cRange(10, 30);
        auto const isect = r.intersectedWith(testvalue);
        if (isect.empty()) {
            ERROR << "intersectedWith test failed: the intersection of " << r << " and " << testvalue << " should not be empty" << std::endl;
            return false;
        }
        if (isect.start != testvalue.start || isect.end != r.end) {
            ERROR << "intersectedWith test failed: the intersection of " << r << " and " << testvalue << " should not be " << isect << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = cRange(20, 30);
        auto const isect = r.intersectedWith(testvalue);
        if (!isect.empty()) {
            ERROR << "intersectedWith test failed: the intersection of " << r << " and " << testvalue << " should be empty" << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = cRange(10, 30);
        if (!(r.intersectedWith(testvalue) == testvalue.intersectedWith(r))) {
            ERROR << "intersectedWith test failed: intersection is commutative" << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = cRange(20, 30);
        if (!(r.intersectedWith(testvalue) == testvalue.intersectedWith(r))) {
            ERROR << "intersectedWith test failed: intersection is commutative even if the intersection is empty" << std::endl;
            return false;
        }
    }
    {
        if (!r.contains((r.end - r.start) / 2 + r.start)) {
            ERROR << "contains test failed: range must contain midpoint" << std::endl;
            return false;
        }
        if (r.contains(r.end)) {
            ERROR << "contains test failed: contains endpoint" << std::endl;
            return false;
        }
    }
    {
        if (!r.contains(cRange(7, 15))) {
            ERROR << "contains test failed: Range(0, 20) does contain Range(7, 15)" << std::endl;
            return false;
        }
        if (r.contains(cRange(10, 30))) {
            ERROR << "contains test failed: Range(0, 20) does not contain Range(10, 30)" << std::endl;
            return false;
        }
        if (r.contains(cRange(30, 40))) {
            ERROR << "contains test failed: Range(0, 20) does not contain Range(30, 40)" << std::endl;
            return false;
        }
        if (r.contains(cRange(20, 30))) {
            ERROR << "contains test failed: Range(0, 20) does not contain Range(20, 30)" << std::endl;
            return false;
        }
    }
    {
        auto const r2 = cRange(7, 15);

        if (!r.intersects(r)) {
            ERROR << "intersects test failed: Range(0, 20) does intersect itself" << std::endl;
            return false;
        }
        if (!r.intersects(r2)) {
            ERROR << "intersects test failed: Range(0, 20) does intersect Range(7, 15)" << std::endl;
            return false;
        }
        if (!r2.intersects(r)) {
            ERROR << "intersects test failed: Range(0, 20) does intersect Range(7, 15)" << std::endl;
            return false;
        }
        if (!r.intersects(cRange(10, 30))) {
            ERROR << "intersects test failed: Range(0, 20) does intersect Range(10, 30)" << std::endl;
            return false;
        }
        if (r.intersects(cRange(20, 40))) {
            ERROR << "intersects test failed: Range(0, 20) does not intersect Range(20, 40)" << std::endl;
            return false;
        }
        if (r.intersects(cRange(30, 50))) {
            ERROR << "intersects test failed: Range(0, 20) does not contain Range(30, 50)" << std::endl;
            return false;
        }
    }
    return true;
}
