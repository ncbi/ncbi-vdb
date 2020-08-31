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

#include <klib/rc.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vfs/path.h>
#include "refseq.h"

#include <stdlib.h>
#include <assert.h>

// packed 2na to unpacked 4na
static void unpack_2na(uint8_t const *bases, uint8_t dst[4], unsigned const position)
{
    int const packed = bases[position / 4];
    int const b2na_1 = packed >> 6;
    int const b2na_2 = (packed >> 4) & 0x03;
    int const b2na_3 = (packed >> 2) & 0x03;
    int const b2na_4 = packed & 0x03;

    dst[0] = 1 << b2na_1;
    dst[1] = 1 << b2na_2;
    dst[2] = 1 << b2na_3;
    dst[3] = 1 << b2na_4;
}

static unsigned partial_unpack_2na(uint8_t const *bases, uint8_t *const dst, unsigned const offset, unsigned const limit, unsigned const pos)
{
    int const j = pos % 4;
    uint8_t temp[4];
    unsigned n;

    unpack_2na(bases, temp, pos);
    for (n = 0; (offset + n) < limit && (j + n) < 4; ++n)
        dst[offset + n] = temp[j + n];

    return n;
}

static void fillNs(RangeList const *self, uint8_t *const dst, Range const *activeRange) {
    Range const *beg = NULL;
    Range const *end = NULL;
    Range const *cur = NULL;

    intersectRangeList(self, &beg, &end, activeRange);
    for (cur = beg; cur != end; ++cur) {
        unsigned start = cur->start < activeRange->start ? activeRange->start : cur->start;
        unsigned end = cur->end > activeRange->end ? activeRange->end : cur->end;
        memset(dst + (start - activeRange->start), 15, (end - start));
    }
}

static void getBases_2na(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned pos = start;
    unsigned i = 0;

    if (pos % 4 != 0) {
        unsigned const n = partial_unpack_2na(self->bases, dst, i, len, pos);
        i += n; pos += n;
    }
    while ((i + 4) <= len) {
        unpack_2na(self->bases, dst + i, pos);
        i += 4;
        pos += 4;
    }
    if (i < len) {
        unsigned const n = partial_unpack_2na(self->bases, dst, i, len, pos);
        i += n; pos += n;
    }
    assert(i == len);
    assert(start + len == pos);

    // 2na will have 'A's in place of 'N's, put the 'N's back
    {
        Range full;
        full.start = start;
        full.end = start + len;
        fillNs(&self->Ns, dst, &full);
    }
}

static unsigned getBases_4na(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned const length = self->length;
    uint8_t const *const bases = self->bases;
    unsigned i = 0;
    unsigned j = start % length;

    if (j % 2 == 1 && i < len) {
        int const b4na_2 = bases[j >> 1];
        int const b4na2 = b4na_2 & 0x0F;
        dst[i++] = b4na2;
        j = (j + 1) % length;
    }
    while ((i + 2) <= len) {
        int const b4na_2 = bases[j >> 1];
        int const b4na1 = b4na_2 >> 4;
        int const b4na2 = b4na_2 & 0x0F;
        dst[i++] = b4na1;
        dst[i++] = b4na2;
        j = (j + 2) % length;
    }
    if (i < len) {
        int const b4na_2 = bases[j >> 1];
        int const b4na1 = b4na_2 >> 4;
        int const b4na2 = b4na_2 & 0x0F;
        dst[i++] = (j % 2) == 0 ? b4na1 : b4na2;
    }
    assert(i == len);
    return i;
}

char const *RefSeq_Scheme(void) {
    return "NCBI:refseq:tbl:reference";
}

unsigned RefSeq_getBases(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    if (!self->circular) {
        unsigned const length = self->length;
        unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;
        if (actlen > 0)
            getBases_2na(self, dst, start, actlen);
        return actlen;
    }
    else
        return getBases_4na(self, dst, start, len);
}

static inline VCursor const *createCursor(VTable const *const tbl, rc_t *prc)
{
    VCursor const *curs = NULL;
    rc_t const rc = VTableCreateCachedCursorRead(tbl, &curs, 0);
    if (prc) *prc = rc;
    return curs;
}

typedef struct CursorAddResult CursorAddResult;
struct CursorAddResult {
    char const *name;
    uint32_t colID;
};

static inline CursorAddResult *addColumn(CursorAddResult *result, char const *name, VCursor const *curs, rc_t *prc)
{
    rc_t const rc = VCursorAddColumn(curs, &result->colID, result->name = name);
    if (prc) *prc = rc;
    return rc == 0 ? result : NULL;
}

typedef struct RowRange RowRange;
struct RowRange {
    int64_t first;
    uint64_t count;
};

static inline RowRange *getRowRange(RowRange *result, VCursor const *curs, rc_t *prc)
{
    rc_t const rc = VCursorIdRange(curs, 0, &result->first, &result->count);
    if (prc) *prc = rc;
    return rc == 0 ? result : NULL;
}

typedef struct Cell Cell;
struct Cell {
    void const *data;
    uint32_t count;
    uint32_t bits;
    uint32_t offset;
};

static inline Cell *readCell(Cell *result, CursorAddResult const *colInfo, int64_t const row, VCursor const *const curs, rc_t *prc)
{
    rc_t const rc = VCursorCellDataDirect(curs, row, colInfo->colID, &result->bits, &result->data, &result->offset, &result->count);
    if (prc) *prc = rc;
    return rc == 0 ? result : NULL;
}

static bool inline readBool(CursorAddResult const *colInfo, int64_t const row, VCursor const *const curs, rc_t *prc)
{
    Cell rr, *const prr = readCell(&rr, colInfo, row, curs, prc);
    assert(rr.bits == 8);
    assert(rr.offset == 0);
    return prr ? (*(uint8_t *)rr.data != 0) : false;
}

static inline uint32_t readU32(CursorAddResult const *colInfo, int64_t const row, VCursor const *const curs, rc_t *prc)
{
    Cell rr, *const prr = readCell(&rr, colInfo, row, curs, prc);
    assert(rr.bits == sizeof(uint32_t) * 8);
    assert(rr.offset == 0);
    return prr ? *(uint32_t *)rr.data : 0;
}

static inline uint64_t readU64(CursorAddResult const *colInfo, int64_t const row, VCursor const *const curs, rc_t *prc)
{
    Cell rr, *const prr = readCell(&rr, colInfo, row, curs, prc);
    assert(rr.bits == sizeof(uint64_t) * 8);
    assert(rr.offset == 0);
    return prr ? *(uint64_t *)rr.data : 0;
}

typedef struct ReadStringResult ReadStringResult;
struct ReadStringResult {
    uint8_t const *value;
    uint32_t length;
};

static inline ReadStringResult *readString(ReadStringResult *result, CursorAddResult const *colInfo, int64_t const row, VCursor const *const curs, rc_t *prc)
{
    Cell rr, *const prr = readCell(&rr, colInfo, row, curs, prc);
    if (prr) {
        result->value = rr.data;
        result->length = rr.count;
        return result;
    }
    return NULL;
}

static rc_t loadCircular(  RefSeq *result
                          , VCursor const *const curs
                          , RowRange const *const rowRange
                          , CursorAddResult const *const seqLenInfo
                          , CursorAddResult const *const readInfo
                          , unsigned const baseCount)
{
    unsigned const allocated = (baseCount + 1) / 2;
    int accum = 0;
    int n = 0;
    unsigned j = 0; ///< current index in bases
    uint64_t i;
    unsigned position = 0;
    rc_t rc = 0;

    result->bases = malloc(allocated);
    if (result->bases == 0)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    for (i = 0; i < rowRange->count; ++i) {
        int64_t const row = rowRange->first + i;
        int32_t const seqLen = readU32(seqLenInfo, row, curs, &rc);
        int32_t ri; ///< index within current row
        ReadStringResult read;

        if (seqLen == 0 || NULL == readString(&read, readInfo, row, curs, &rc))
            return rc;
        for (ri = 0; ri < seqLen; ++ri) {
            int base = 15;
            if (ri < read.length)
                base = read.value[ri];
            assert(base >= 0 && base <= 15);

            accum = (accum << 4) | base;
            ++n;
            if (n == 2) {
                assert(j < allocated);
                if (!(j < allocated))
                    return RC(rcXF, rcFunction, rcConstructing, rcData, rcTooLong);

                result->bases[j++] = accum;
                accum = 0;
                n = 0;
            }
            ++position;
        }
    }
    if (n != 0) {
        accum = accum << 4;
        assert(j < allocated);
        if (!(j < allocated))
            return RC(rcXF, rcFunction, rcConstructing, rcData, rcTooLong);

        result->bases[j++] = accum;
    }
    assert(j == allocated);
    result->length = position;
    result->circular = true;
    return 0;
}

static rc_t load(  RefSeq *result
                  , VCursor const *const curs
                  , RowRange const *const rowRange
                  , CursorAddResult const *const seqLenInfo
                  , CursorAddResult const *const readInfo
                  , unsigned const baseCount)
{
    unsigned const allocated = (baseCount + 3) / 4;
    int accum = 0;
    int n = 0;
    unsigned j = 0; ///< current index in bases
    uint64_t i;
    unsigned position = 0;
    rc_t rc = 0;

    result->bases = malloc(allocated);
    if (result->bases == 0)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    for (i = 0; i < rowRange->count; ++i) {
        int64_t const row = rowRange->first + i;
        int32_t const seqLen = readU32(seqLenInfo, row, curs, &rc);
        int32_t ri; ///< index within current row
        ReadStringResult read;

        if (seqLen == 0 || NULL == readString(&read, readInfo, row, curs, &rc))
            return rc;
        for (ri = 0; ri < seqLen; ++ri) {
            int base = 0;
            int isN = 1;
            if (ri < read.length) {
                switch (read.value[ri]) {
                case 1: base = 0; isN = 0; break;
                case 2: base = 1; isN = 0; break;
                case 4: base = 2; isN = 0; break;
                case 8: base = 3; isN = 0; break;
                }
            }
            assert(base >= 0 && base <= 3);
            accum = (accum << 2) | base;
            ++n;
            if (n == 4) {
                assert(j < allocated);
                if (!(j < allocated))
                    return RC(rcXF, rcFunction, rcConstructing, rcData, rcTooLong);

                result->bases[j++] = accum;
                accum = 0;
                n = 0;
            }
            if (isN)
                extendRangeList(&result->Ns, position);
            ++position;
        }
    }
    if (n != 0) {
        while (n < 4) {
            accum = accum << 2;
            ++n;
        }
        assert(j < allocated);
        if (!(j < allocated))
            return RC(rcXF, rcFunction, rcConstructing, rcData, rcTooLong);

        result->bases[j++] = accum;
    }
    assert(j == allocated);
    result->length = position;
    return 0;
}

rc_t RefSeq_load(RefSeq *result, VTable const *const tbl)
{
    CursorAddResult cols[4];
    RowRange rowRange;
    rc_t rc = 0;
    VCursor const *const curs = createCursor(tbl, &rc);

    memset(result, 0, sizeof(*result));
    if (curs == NULL) return rc;

    if (!addColumn(&cols[0], "CIRCULAR", curs, &rc)) return rc;
    if (!addColumn(&cols[1], "SEQ_LEN", curs, &rc)) return rc;
    if (!addColumn(&cols[2], "(INSDC:4na:bin)READ", curs, &rc)) return rc;
    if (!addColumn(&cols[3], "TOTAL_SEQ_LEN", curs, &rc)) return rc;

    rc = VCursorOpen(curs);
    assert(rc == 0);

    if (getRowRange(&rowRange, curs, &rc) != NULL) {
        uint64_t const baseCount = readU64(&cols[3], rowRange.first, curs, &rc);
        if (baseCount > 0) {
            bool const circular = readBool(&cols[0], rowRange.first, curs, &rc);

            rc = (circular ? loadCircular : load)(result, curs, &rowRange, &cols[1], &cols[2], baseCount);
        }
    }
    VCursorRelease(curs);
    return rc;
}

void RefSeqFree(RefSeq *self)
{
    RangeListFree(&self->Ns);
    free(self->bases);
}
