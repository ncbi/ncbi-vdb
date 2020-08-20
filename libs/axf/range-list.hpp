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

struct Range {
    unsigned start, end;

    // Range() : start(0), end(0) {}

    Range(unsigned const start, unsigned const end)
    : start(start)
    , end(end)
    {
        assert(start <= end);
    }

    /// a new range of 1
    explicit Range(unsigned const pos)
    : start(pos)
    , end(pos + 1)
    {}

    // increase the size of the range by increasing the end position
    void extend() { end += 1; }

    /// returns a new range that is translated to be relative to the start of another range
    Range offsetFrom(Range const &origin) const {
        assert(origin.start <= start);
        return Range(start - origin.start, end - origin.start);
    }

    /// the distance between the start and end of the range
    unsigned size() const { return end - start; }

    /// true if the distance between the start and end of the range == 0
    bool empty() const { return start == end; }

    bool operator ==(Range const &other) const { return start == other.start && end == other.end; }

    /// true if the point within this range
    bool contains(unsigned const point) const { return start <= point && point < end; }

    /// true if the range is a subrange of this range
    bool contains(Range const &other) const { return start <= other.start && other.end <= end; }

    /// true if the intersection of this range and another range is not empty
    bool intersects(Range const &other) const {
        return (contains(other.start) || other.contains(start));
    }

    /// a new range that is the intersection of this range and another range; may be empty
    Range intersectedWith(Range const &other) const {
        auto const s = std::max(start, other.start);
        return Range(s, std::max(s, std::min(end, other.end)));
    }

    /// a new range that is the intersection of this range and another range; may NOT be empty
    Range nonEmptyIntersection(Range const &other) const {
        auto const &result = this->intersectedWith(other);
        assert(!result.empty());
        return result;
    }

#if TESTING
public:
    Range randomSubRange() const;
    static bool test();

    friend std::ostream &operator <<(std::ostream &os, Range const &r) {
        return os << r.start << "..<" << r.end;
    }
#endif
};

struct RangeList {
    using Container = std::vector<Range>;
    using ConstIter = Container::const_iterator;

private:
    Container ranges;

public:
    void add(unsigned const position) {
        if (!ranges.empty()) {
            auto &extendable = ranges.back();
            if (extendable.end == position) {
                extendable.extend();
                return;
            }
        }
        ranges.emplace_back(Range(position));
    }
    RangeList &append(Range const &r) {
        assert(ranges.empty() || ranges.back().end < r.start);
        ranges.emplace_back(r);
        return *this;
    }
    bool empty() const { return ranges.empty(); }
    Range fullRange() const {
        if (ranges.empty()) return Range(0, 0);
        return Range(ranges.front().start, ranges.back().end);
    }

    // these allow ranged-for to work
    ConstIter begin() const { return ranges.begin(); }
    ConstIter end() const { return ranges.end(); }

    /// the type returned by the intersecting function
    using Intersecting = struct {
        ConstIter begin_, end_;

        // these allow ranged-for to work
        ConstIter begin() const { return begin_; }
        ConstIter end() const { return end_; }
    };

    /// @return the ranges in the list which intersect the query range
    Intersecting intersecting(Range const &r) const {
        if (r.empty()) return { ranges.end(), ranges.end() };

        // partition_point returns the first element for which the predicate is FALSE
        auto const &beg = std::partition_point(ranges.begin(), ranges.end(), [r](Range const &e) { return !(r.start < e.end); });
        auto const &end = std::partition_point(       beg    , ranges.end(), [r](Range const &e) { return !(r.end <= e.start); });

        return { beg, end };
    }

    /// @return a new RangeList consisting of the intersection of every Range with r where that intersection is not empty
    RangeList intersect(Range const &r) const {
        auto result = RangeList();

#if 0
        // slow implementation; scan entire list
        for (auto && range : ranges) {
            auto const &i = r.intersectedWith(range);
            if (!i.empty())
                result.ranges.emplace_back(i);
        }
#else
        for (auto && range : intersecting(r)) {
            result.ranges.emplace_back(r.nonEmptyIntersection(range));
        }
#endif
        return result;
    }

#if TESTING
public:
    static bool test();

    friend std::ostream &operator <<(std::ostream &os, RangeList const &list) {
        os << "[ ";
        for (auto && r : list) {
            os << r << " ";
        }
        return os << "]";
    }
#endif
};
