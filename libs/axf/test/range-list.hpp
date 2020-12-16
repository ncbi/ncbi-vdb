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

#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>

extern "C" {
#include "../range-list.h"
}

class cRange : public Range {
    cRange(Range const &other) : Range(other) {}
public:
    cRange(unsigned const start, unsigned const end)
    {
        assert(start <= end);
        this->start = start;
        this->end = end;
    }

    /// returns a new range that is translated to be relative to the start of another range
    cRange offsetFrom(cRange const &origin) const {
        assert(origin.start <= start);
        return cRange(start - origin.start, end - origin.start);
    }

    /// the distance between the start and end of the range
    unsigned size() const { return end - start; }

    /// true if the distance between the start and end of the range == 0
    bool empty() const { return start == end; }

    bool operator ==(cRange const &other) const {
        return start == other.start && end == other.end;
    }

    /// true if the point within this range
    bool contains(unsigned const point) const {
        return start <= point && point < end;
    }

    /// true if the range is a subrange of this range
    bool contains(cRange const &other) const {
        return start <= other.start && other.end <= end;
    }

    /// true if the intersection of this range and another range is not empty
    bool intersects(cRange const &other) const {
        return (contains(other.start) || other.contains(start));
    }

    /// a new range that is the intersection of this range and another range; may be empty
    cRange intersectedWith(cRange const &other) const {
        Range result;
        intersectRanges(&result, this, &other);
        return result;
    }

    /// a new range that is the intersection of this range and another range; may NOT be empty
    cRange nonEmptyIntersection(cRange const &other) const {
        auto const &result = this->intersectedWith(other);
        assert(!result.empty());
        return result;
    }

#if TESTING
public:
    cRange randomSubRange() const;
    static bool test();

    friend std::ostream &operator <<(std::ostream &os, cRange const &r) {
        return os << r.start << "..<" << r.end;
    }
#endif
};

class cRangeList : RangeList {
public:
    cRangeList()
    {
        ranges = 0;
        allocated = 0;
        count = 0;
    }
    explicit cRangeList(unsigned const n)
    {
        ranges = 0;
        count = 0;
        allocated = 0;
        if (n > 0) {
            ranges = (Range *)malloc(n * sizeof(Range));
            if (ranges == NULL)
                throw std::bad_alloc();
        }
        allocated = n;
    }
    ~cRangeList() {
        free(ranges);
    }
    using ConstIter = cRange const *;

    void add(unsigned const position) {
        extendRangeList(this, position);
    }
    cRangeList &append(cRange const &r) {
        auto const p = appendRange(this, &r);
        if (p)
            return *this;
        throw std::bad_alloc();
    }
    size_t size() const { return count; }
    bool empty() const { return count == 0; }
    cRange fullRange() const {
        return cRange(  count == 0 ? 0 : ranges[0].start
                      , count == 0 ? 0 : ranges[count - 1].end);
    }

    // these allow ranged-for to work
    ConstIter begin() const { return static_cast<ConstIter>(ranges); }
    ConstIter end() const { return begin() + size(); }

    /// the type returned by the intersecting function
    using Intersecting = struct {
        ConstIter begin_, end_;

        // these allow ranged-for to work
        ConstIter begin() const { return begin_; }
        ConstIter end() const { return end_; }
    };

    /// @return the ranges in the list which intersect the query range
    Intersecting intersecting(cRange const &r) const {
        Range const *s = 0;
        Range const *e = 0;

        intersectRangeList(this, &s, &e, &r);
        assert(s <= e);

        return { static_cast<ConstIter>(s)
                , static_cast<ConstIter>(e) };
    }

    cRangeList intersect(cRange const &r) const {
        auto const intersecting = this->intersecting(r);
        auto const N = std::distance(intersecting.begin(), intersecting.end());
        auto result = cRangeList(N);

        for (auto && i : intersecting) {
            result.append(i.nonEmptyIntersection(r));
        }
        return result;
    }

    bool check() const {
        return 0 != checkRangeList(this);
    }

    cRange const &operator [](int i) const {
        return *static_cast<cRange *>(ranges + i);
    }

#if TESTING
public:
    static bool test();

    friend std::ostream &operator <<(std::ostream &os, cRangeList const &list) {
        os << "[ ";
        for (auto && r : list) {
            os << r << " ";
        }
        return os << "]";
    }
#endif
};
