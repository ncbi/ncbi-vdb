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

#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/column.h>
#include <vfs/path.h>
#include "refseq.hpp"
#include <cctype>

static VCursor createCursor(VTable const *const tbl, rc_t *const rcp)
{
    VCursor const *curs = NULL;
    rc_t const rc = VTableCreateCachedCursorRead(tbl, &curs, 0);
    if (rcp) *rcp = rc;
    if (rc == 0)
        return curs;
    VCursorRelease(curs);
    return NULL;
}

struct CursorAddResult {
    char const *name;
    uint32_t colID;
};

CursorAddResult addColumn(VCursor const *curs, char const *name, rc_t *const rcp)
{
    uint32_t index = 0;
    rc_t rc = VCursorAddColumn(curs, &index, "%s", name);
    if (rcp) *rcp = rc;
    return { name, index };
}

static RefSeq RefSeq::load(VPath const *const url, VTable const *const tbl)
{
    CursorAddResult cols[3];
    void const *data = NULL;
    rc_t rc = 0;
    auto const curs = createCursor(tbl, &rc);
    if (rc) throw rc;

    VTableRelease(tbl);
    VPathRelease(url);

    cols[0] = addColumn(curs, "CIRCULAR", &rc);
    if (rc) throw rc;

    cols[1] = addColumn(curs, "(I32)SEQ_LEN", &rc);
    if (rc) throw rc;

    cols[2] = addColumn(curs, "(INSDC:dna:text)READ", &rc);
    if (rc) throw rc;

    rc = VCursorOpen(curs);
    if (rc) throw rc;

    int64_t first = 0;
    uint64_t count = 0;
    rc = VCursorIdRange(curs, 0, &first, &count);
    if (rc) throw rc;

    rc = VCursorCellDataDirect(curs, first, cols[0].colID, NULL, &data, NULL, NULL);
    if (rc) throw rc;

    auto const circular = *reinterpret_cast<bool const *>(data)
    RangeList Ns;
    std::vector<uint8_t> bases;
    unsigned position = 0;
    int n = 0;
    int accum = 0;

    auto const &do_2na = [&](int32_t const seqLen, uint32_t const readLen, char const *read)
    {
        for (auto j = decltype(seqLen)(0); j < seqLen; ++j) {
            int b2na = 0;
            int N = 1;

            if (j < readLen) {
                switch (toupper(read[j])) {
                case 'A': b2na = 0; N = 0; break;
                case 'C': b2na = 1; N = 0; break;
                case 'G': b2na = 2; N = 0; break;
                case 'T': b2na = 3; N = 0; break;
                }
            }
            accum = (accum << 2) | b2na;
            n += 1;
            if (n == 4) {
                bases.push_back(accum);
                accum = 0;
                n = 0;
            }
            if (N) Ns.add(position);
            position += 1;
        }
    };
    auto const &do_4na = [&](int32_t const seqLen, uint32_t const readLen, char const *read)
    {
        for (auto j = decltype(seqLen)(0); j < seqLen; ++j) {
            int b4na = 0;

            if (j < readLen) {
                switch (toupper(read[j])) {
                case 'A': b4na = 1; break;
                case 'C': b4na = 2; break;
                case 'G': b4na = 4; break;
                case 'T': b4na = 8; break;
                }
            }
            accum = (accum << 4) | b4na;
            n += 1;
            if (n == 2) {
                bases.push_back(accum);
                accum = 0;
                n = 0;
            }
            position += 1;
        }
    };
    auto const &do_row = circular ? do_4na : do_2na;

    for (auto i = decltype(count)(0); i < count; ++i) {
        auto const row = first + i;
        int32_t seqLen = 0;
        uint32_t readLen = 0;
        char const *read = NULL;

        rc = VCursorCellDataDirect(curs, row, cols[1].colID, NULL, &data, NULL, NULL);
        if (rc) throw rc;
        seqLen = *reinterpret_cast<int32_t const *>(data);

        rc = VCursorCellDataDirect(curs, row, cols[2].colID, NULL, &data, NULL, &readLen);
        if (rc) throw rc;

        do_row((seqLen, readLen, reinterpret_cast<char const *>(data)));
    }
    if (circular) {
        if (n != 0) {
            while (n < 2) {
                accum <<= 4;
                n += 1;
            }
            bases.push_back(accum);
        }
    }
    else {
        if (n != 0) {
            while (n < 4) {
                accum <<= 2;
                n += 1;
            }
            bases.push_back(accum);
        }
    }

    VCursorRelease(curs);

    return circular ? RefSeq(std::move(bases), position)
                    : RefSeq(std::move(Ns), std::move(bases), position);
}
