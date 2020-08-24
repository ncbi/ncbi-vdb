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

#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vfs/path.h>
#include "refseq.hpp"
#include <cctype>
#include <memory>

static inline VCursor const *createCursor(VTable const *const tbl)
{
    VCursor const *curs = NULL;
    rc_t const rc = VTableCreateCachedCursorRead(tbl, &curs, 0);
    if (rc) throw rc;
    return curs;
}

struct CursorAddResult {
    char const *name;
    uint32_t colID;
};

static inline CursorAddResult addColumn(char const *name, VCursor const *curs)
{
    uint32_t index = 0;
    rc_t const rc = VCursorAddColumn(curs, &index, name);
    if (rc) throw rc;
    return { name, index };
}

struct RowRange {
    int64_t first;
    uint64_t count;
};

static inline RowRange getRowRange(VCursor const *curs)
{
    int64_t first = 0;
    uint64_t count = 0;
    rc_t const rc = VCursorIdRange(curs, 0, &first, &count);
    if (rc) throw rc;
    return { first, count };
}

using ReadResult = struct { void const *data; uint32_t count; uint32_t bits; uint32_t offset; };
static inline ReadResult read(CursorAddResult const &colInfo, int64_t const row, VCursor const *const curs)
{
    void const *data = NULL;
    uint32_t count = 0;
    uint32_t bits = 0;
    uint32_t offset = 0;
    rc_t const rc = VCursorCellDataDirect(curs, row, colInfo.colID, &bits, &data, &offset, &count);
    if (rc) throw rc;
    return { data, count, bits, offset };
}

bool inline readBool(CursorAddResult const &colInfo, int64_t const row, VCursor const *const curs)
{
    auto const rr = read(colInfo, row, curs);
    assert(rr.bits == sizeof(bool) * 8);
    assert(rr.offset == 0);
    assert(rr.count == 1);
    return *reinterpret_cast<bool const *>(rr.data);
}

static inline int32_t readI32(CursorAddResult const &colInfo, int64_t const row, VCursor const *const curs)
{
    auto const rr = read(colInfo, row, curs);
    assert(rr.bits == sizeof(int32_t) * 8);
    assert(rr.offset == 0);
    assert(rr.count == 1);
    return *reinterpret_cast<int32_t const *>(rr.data);
}

using ReadStringResult = struct {
    uint8_t const *value; uint32_t length;
    char operator [](decltype(length) i) const { return i < length ? value[i] : 'N'; }
};
static inline ReadStringResult readString(CursorAddResult const &colInfo, int64_t const row, VCursor const *const curs)
{
    auto const rr = read(colInfo, row, curs);
    assert(rr.bits == 8);
    assert(rr.offset == 0);
    return { reinterpret_cast<uint8_t const *>(rr.data), rr.count };
}

class AccumulatorBase {
protected:
    unsigned position;
    int n;
    int value;

    virtual void accumulate_bin(int const base, std::vector<uint8_t> *bases) = 0;
    virtual void accumulateN(std::vector<uint8_t> *bases, RangeList *Ns) = 0;
public:
    AccumulatorBase() : position(0), n(0), value(0) {}
    virtual ~AccumulatorBase() {}

    virtual void add(uint8_t const base, std::vector<uint8_t> *bases, RangeList *Ns) = 0;
    virtual unsigned finish(std::vector<uint8_t> *bases) = 0;
};

template <int SHIFT, int A_VALUE, int C_VALUE, int G_VALUE, int T_VALUE, bool NRA>
class Accumulator final : public AccumulatorBase {
    enum { limit = 8 / SHIFT };

    void accumulate_bin(int const base, std::vector<uint8_t> *bases) override
    {
        value = (value << SHIFT) | base;
        n += 1;
        if (n == limit) {
            bases->push_back(value);
            value = 0;
            n = 0;
        }
        position += 1;
    }
    void accumulateN(std::vector<uint8_t> *bases, RangeList *Ns) override
    {
        auto const pos = position;
        accumulate_bin(0, bases);
        if (NRA) Ns->add(pos);
    }
public:
    void add(uint8_t const base, std::vector<uint8_t> *bases, RangeList *Ns) override
    {
        if (SHIFT == 4) {
            accumulate_bin(base, bases);
            return;
        }

        switch (base) {
        case 1: accumulate_bin(A_VALUE, bases); return;
        case 2: accumulate_bin(C_VALUE, bases); return;
        case 4: accumulate_bin(G_VALUE, bases); return;
        case 8: accumulate_bin(T_VALUE, bases); return;
        }
        accumulateN(bases, Ns);
    }

    unsigned finish(std::vector<uint8_t> *bases) override
    {
        if (n > 0) {
            while (n < limit) {
                value <<= SHIFT;
                n += 1;
            }
            bases->push_back(value);
        }
        return position;
    }
};

static inline std::unique_ptr<AccumulatorBase> makeAccumulator(bool circular)
{
    using Accumulator_2na = Accumulator<2, 0, 1, 2, 3, true>;
    using Accumulator_4na = Accumulator<4, 1, 2, 4, 8, false>;
    auto result = circular ? std::unique_ptr<AccumulatorBase>(new Accumulator_4na())
                           : std::unique_ptr<AccumulatorBase>(new Accumulator_2na());
    return result;
}

RefSeq RefSeq::load(VPath const *const url, VTable const *const tbl)
{
    VPathRelease(url);

    CursorAddResult cols[3];
    rc_t rc = 0;
    auto const curs = createCursor(tbl);

    cols[0] = addColumn("(BOOL)CIRCULAR", curs);
    cols[1] = addColumn("(I32)SEQ_LEN", curs);
    cols[2] = addColumn("(INSDC:4na:bin)READ", curs);

    rc = VCursorOpen(curs);
    assert(rc == 0);

    VTableRelease(tbl);

    auto const rowRange = getRowRange(curs);
    auto const circular = readBool(cols[0], rowRange.first, curs);
    RangeList Ns;
    std::vector<uint8_t> bases;
    unsigned length = 0;

    {
        auto const accum = makeAccumulator(circular);

        for (auto i = decltype(rowRange.count)(0); i < rowRange.count; ++i) {
            auto const row = rowRange.first + i;
            auto const seqLen = readI32(cols[1], row, curs);
            auto const read = readString(cols[2], row, curs);

            for (auto j = decltype(seqLen)(0); j < seqLen; ++j) {
                accum->add(read[j], &bases, &Ns);
            }
        }
        length = accum->finish(&bases);
    }
    VCursorRelease(curs);

    return RefSeq(std::move(bases), length, circular, std::move(Ns));
}
