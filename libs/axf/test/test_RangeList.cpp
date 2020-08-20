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
#include "../range-list.hpp"

#undef TESTING

static bool verbose = false;
#define VERBOSE if (!verbose) {} else std::cerr

int main(int argc, char **argv)
{
    {
        auto const envvar = getenv("VERBOSE");
        auto const verbose_arg = (argc > 1 && std::string(argv[1]) == "-v");
        auto const verbose_env = (envvar != NULL && std::string(envvar) == "1");

        verbose = verbose_arg || verbose_env;
    }
    srandom(time(0));

    if (Range::test())
        std::cerr << "Range::test ok" << std::endl;
    else
        return 1;

    if (RangeList::test())
        std::cerr << "RangeList::test ok" << std::endl;
    else
        return 1;

    return 0;
}

Range Range::randomSubRange() const {
    double r1 = random() / (double)(RAND_MAX);
    double r2 = random() / (double)(RAND_MAX);
    unsigned const sublen = r1 * (end - start);
    unsigned const offset = r2 * ((end - start) - sublen);
    auto const result = Range(start + offset, start + offset + sublen);
    assert (result.empty() || this->contains(result));
    return result;
}

bool RangeList::test() {
    auto const first = 10;
    auto const list = RangeList()
        .append(Range(first + 0, first + 20))
        .append(Range(first + 30, first + 50))
        .append(Range(first + 70, first + 90));

    VERBOSE << "list: " << list << std::endl;
    {
        auto const testcount = 1000000; assert(testcount > 200 * 200);

        VERBOSE << "testing " << testcount << " random sub-ranges for intersection" << std::endl;

        auto const wholeRange = Range(0, 200);
        for (auto _ = 0; _ < testcount; ++_) {
            auto const sub = wholeRange.randomSubRange();
            auto const intersecting = list.intersecting(sub);
            auto i = list.begin();
            auto j = intersecting.begin();

            if (sub.empty()) {
                if (intersecting.begin() != intersecting.end()) {
                    std::cerr << "intersect test failed: " << sub << " should not intersect" << std::endl;
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
                    std::cerr << "intersect test failed: " << *i << " and " << sub << " should not intersect" << std::endl;
                    return false;
                }
            }
            // verify that every range in the intersecting list does intersect
            for ( ; i != list.end() && j != intersecting.end(); ++i, ++j) {
                assert(i == j);
                if (!i->intersects(sub)) {
                    std::cerr << "intersect test failed: " << *i << " and " << sub << " should intersect" << std::endl;
                    return false;
                }
            }
            // verify that every range after the intersecting list does NOT intersect
            for ( ; i != list.end(); ++i) {
                if (i->intersects(sub)) {
                    std::cerr << "intersect test failed: " << *i << " and " << sub << " should not intersect" << std::endl;
                    return false;
                }
            }
        }
    }
    { // before the first range
        auto const testvalue = Range(0, first);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (!intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }
    { // entirely within the first range
        auto const testvalue = Range(first + 10, first + 20);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
        if (!intersection.begin()->contains(testvalue.start) || !intersection.begin()->contains(testvalue.end - 1)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
    }
    { // starts within the first range
        auto const testvalue = Range(first + 10, first + 30);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
        if (!intersection.begin()->contains(testvalue.start)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
    }
    { // starts within the first range, ends within the second range
        auto const testvalue = Range(first + 10, first + 40);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
        if (!(std::distance(intersection.begin(), intersection.end()) == 2)) {
            std::cerr << "intersect test failed: " << testvalue << " should intersect with 2 ranges, not "<< std::distance(intersection.begin(), intersection.end()) << std::endl;
            return false;
        }
        if (!intersection.begin()->contains(testvalue.start)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
        auto j = intersection.begin(); ++j;
        if (!j->contains(testvalue.end - 1)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
    }
    { // starts within the first range, ends within the third range
        auto const testvalue = Range(first + 10, first + 80);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
        if (!intersection.begin()->contains(testvalue.start)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
        auto j = intersection.begin(); ++j; ++j;
        if (!j->contains(testvalue.end - 1)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
    }
    { // entirely between first and second ranges
        auto const testvalue = Range(first + 20, first + 30);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (!intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }
    { // starts within the first gap, ends within the second gap
        auto const testvalue = Range(first + 25, first + 60);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }
    { // ends in second range
        auto const testvalue = Range(first + 20, first + 40);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
        if (!intersection.begin()->contains(testvalue.end - 1)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
    }
    { // entirely in second range
        auto const testvalue = Range(first + 30, first + 40);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
    }
    { // starts in second range
        auto const testvalue = Range(first + 49, first + 69);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does intersect the Range List" << std::endl;
            return false;
        }
        if (!intersection.begin()->contains(testvalue.start)) {
            std::cerr << "intersect test failed: wrong range @ line " << __LINE__ << std::endl;
            return false;
        }
    }
    { // beyond second range
        auto const testvalue = Range(first + 50, first + 70);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (!intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }
    { // beyond last range
        auto const testvalue = Range(first + 100, first + 170);
        auto const intersection = list.intersect(testvalue);

        VERBOSE << "intersected with " << testvalue << ": " << intersection << std::endl;
        if (!intersection.empty()) {
            std::cerr << "intersect test failed: " << testvalue << " does not intersect the Range List" << std::endl;
            return false;
        }
    }
    return true;
}

bool Range::test() {
    auto const r = Range(0, 20);
    {
        if (!(r == r.intersectedWith(r))) {
            std::cerr << "intersectedWith test failed: a range intersected with itself must equal itself" << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = Range(10, 30);
        auto const isect = r.intersectedWith(testvalue);
        if (isect.empty()) {
            std::cerr << "intersectedWith test failed: the intersection of " << r << " and " << testvalue << " should not be empty" << std::endl;
            return false;
        }
        if (isect.start != testvalue.start || isect.end != r.end) {
            std::cerr << "intersectedWith test failed: the intersection of " << r << " and " << testvalue << " should not be " << isect << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = Range(20, 30);
        auto const isect = r.intersectedWith(testvalue);
        if (!isect.empty()) {
            std::cerr << "intersectedWith test failed: the intersection of " << r << " and " << testvalue << " should be empty" << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = Range(10, 30);
        if (!(r.intersectedWith(testvalue) == testvalue.intersectedWith(r))) {
            std::cerr << "intersectedWith test failed: intersection is commutative" << std::endl;
            return false;
        }
    }
    {
        auto const testvalue = Range(20, 30);
        if (!(r.intersectedWith(testvalue) == testvalue.intersectedWith(r))) {
            std::cerr << "intersectedWith test failed: intersection is commutative even if the intersection is empty" << std::endl;
            return false;
        }
    }
    {
        if (!r.contains((r.end - r.start) / 2 + r.start)) {
            std::cerr << "contains test failed: range must contain midpoint" << std::endl;
            return false;
        }
        if (r.contains(r.end)) {
            std::cerr << "contains test failed: contains endpoint" << std::endl;
            return false;
        }
    }
    {
        if (!r.contains(Range(7, 15))) {
            std::cerr << "contains test failed: Range(0, 20) does contain Range(7, 15)" << std::endl;
            return false;
        }
        if (r.contains(Range(10, 30))) {
            std::cerr << "contains test failed: Range(0, 20) does not contain Range(10, 30)" << std::endl;
            return false;
        }
        if (r.contains(Range(30, 40))) {
            std::cerr << "contains test failed: Range(0, 20) does not contain Range(30, 40)" << std::endl;
            return false;
        }
        if (r.contains(Range(20, 30))) {
            std::cerr << "contains test failed: Range(0, 20) does not contain Range(20, 30)" << std::endl;
            return false;
        }
    }
    {
        auto const r2 = Range(7, 15);

        if (!r.intersects(r)) {
            std::cerr << "intersects test failed: Range(0, 20) does intersect itself" << std::endl;
            return false;
        }
        if (!r.intersects(r2)) {
            std::cerr << "intersects test failed: Range(0, 20) does intersect Range(7, 15)" << std::endl;
            return false;
        }
        if (!r2.intersects(r)) {
            std::cerr << "intersects test failed: Range(0, 20) does intersect Range(7, 15)" << std::endl;
            return false;
        }
        if (!r.intersects(Range(10, 30))) {
            std::cerr << "intersects test failed: Range(0, 20) does intersect Range(10, 30)" << std::endl;
            return false;
        }
        if (r.intersects(Range(20, 40))) {
            std::cerr << "intersects test failed: Range(0, 20) does not intersect Range(20, 40)" << std::endl;
            return false;
        }
        if (r.intersects(Range(30, 50))) {
            std::cerr << "intersects test failed: Range(0, 20) does not contain Range(30, 50)" << std::endl;
            return false;
        }
    }
    return true;
}
