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

#include "range-list.hpp"
#include <vector>
#include <string>

struct RefSeq {
private:
    using Bases = std::vector<uint8_t>;

    Bases bases;
    RangeList Ns;
    unsigned length; ///< this may be slightly less than bases.size() * (circular ? 2 : 4)
    bool circular;

    RefSeq(Bases && bases, unsigned const length, bool const circular, RangeList && Ns)
    : bases(bases)
    , Ns(Ns)
    , length(length)
    , circular(circular)
    {
        if (circular) {
            assert(bases.size() == (length + 1) / 2);
        }
        else {
            assert(bases.size() == (length + 3) / 4);
            if (!Ns.empty()) {
                assert(Range(0, length).contains(Ns.fullRange()));
            }
        }
    }

    void fillNs(uint8_t *const dst, Range const &activeRange) const {
        for (auto && range : Ns.intersecting(activeRange)) {
            auto const sub = range.intersectedWith(activeRange).offsetFrom(activeRange);
            memset(dst + sub.start, 15, sub.size());
        }
    }

    // packed 2na to unpacked 4na
    void unpack_2na(uint8_t dst[4], unsigned const position) const {
        auto const packed = bases[position / 4];
        auto const b2na_1 = packed >> 6;
        auto const b2na_2 = (packed >> 4) & 0x03;
        auto const b2na_3 = (packed >> 2) & 0x03;
        auto const b2na_4 = packed & 0x03;

        dst[0] = 1 << b2na_1;
        dst[1] = 1 << b2na_2;
        dst[2] = 1 << b2na_3;
        dst[3] = 1 << b2na_4;
    }

    unsigned partial_unpack_2na(uint8_t *const dst, unsigned const offset, unsigned const limit, unsigned const pos) const
    {
        auto const j = pos % 4;
        uint8_t temp[4];
        unsigned n;

        unpack_2na(temp, pos);
        for (n = 0; (offset + n) < limit && (j + n) < 4; ++n)
            dst[offset + n] = temp[j + n];

        return n;
    }

    void getBases_2na(uint8_t *const dst, unsigned const start, unsigned const len) const
    {
        unsigned pos = start;
        unsigned i = 0;

        if (pos % 4 != 0) {
            auto const n = partial_unpack_2na(dst, i, len, pos);
            i += n; pos += n;
        }
        while ((i + 4) <= len) {
            unpack_2na(dst + i, pos);
            i += 4;
            pos += 4;
        }
        if (i < len) {
            auto const n = partial_unpack_2na(dst, i, len, pos);
            i += n; pos += n;
        }
        assert(i == len);
        assert(start + len == pos);

        // 2na will have 'A's in place of 'N's, put the 'N's back
        fillNs(dst, Range(start, start + len));
    }

    unsigned getBases_4na(uint8_t *const dst, unsigned const start, unsigned const len) const
    {
        unsigned i = 0;
        unsigned j = start % length;

        if (j % 2 == 1 && i < len) {
            auto const b4na_2 = bases[j >> 1];
            auto const b4na2 = b4na_2 & 0x0F;
            dst[i++] = b4na2;
            j = (j + 1) % length;
        }
        while ((i + 2) <= len) {
            auto const b4na_2 = bases[j >> 1];
            auto const b4na1 = b4na_2 >> 4;
            auto const b4na2 = b4na_2 & 0x0F;
            dst[i++] = b4na1;
            dst[i++] = b4na2;
            j = (j + 2) % length;
        }
        if (i < len) {
            auto const b4na_2 = bases[j >> 1];
            auto const b4na1 = b4na_2 >> 4;
            auto const b4na2 = b4na_2 & 0x0F;
            dst[i++] = (j % 2) == 0 ? b4na1 : b4na2;
        }
        assert(i == len);
        return i;
    }

public:
    static bool isScheme(std::string const &scheme) {
        return scheme == std::string("NCBI:refseq:tbl:reference");
    }
    unsigned getBases(uint8_t *const dst, unsigned const start, unsigned const len) const
    {
        if (!circular) {
            auto const actlen = (start + len) < length ? len : start < length ? length - start : 0;
            if (actlen > 0)
                getBases_2na(dst, start, actlen);
            return actlen;
        }
        else
            return getBases_4na(dst, start, len);
    }

#if TESTING
private:
    static RefSeq loadFromStream(std::istream && input);
    static RefSeq loadFromStreamCircular(std::istream && input);
public:
    static bool test();
#else
public:
    static RefSeq load(struct VPath const *const url, struct VTable const *const tbl);
#endif
};
