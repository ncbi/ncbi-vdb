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

static inline VCursor const *createCursor(VTable const *const tbl, rc_t *prc)
{
    VCursor const *curs = NULL;
    rc_t const rc = VTableCreateCachedCursorRead(tbl, &curs, 0);
    if (prc) *prc = rc;
    return curs;
}
