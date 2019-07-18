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
 * =============================================================================
 *
 */

#include <vdb/extern.h>

#include "blast-mgr.h" /* BTableType */
#include "reader.h" /* Data2na */
#include "reference.h" /* _ReferencesWhack */
#include "run-set.h" /* VdbBlastRunSet */

#include <ncbi/vdb-blast-priv.h>  /* VDB_READ_DIRECT */

#include <insdc/sra.h> /* SRA_READ_TYPE_BIOLOGICAL */

#include <sra/srapath.h> /* SRAPath */

#include <vdb/manager.h> /* VDBManager */
#include <vdb/schema.h> /* VSchema */
#include <vdb/database.h> /* VDatabase */
#include <vdb/table.h> /* VTable */
#include <vdb/cursor.h> /* VCursor */
#include <vdb/blob.h> /* VBlob */
#include <vdb/vdb-priv.h> /* VCursorPermitPostOpenAdd */

#include <kdb/manager.h> /* kptDatabase */
#include <kdb/table.h> /* KTable */

#include <vfs/path.h> /* VPath */

#include <kfg/config.h> /* KConfig */

#include <kproc/lock.h> /* KLock */

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h> /* PLOGMSG */
#include <klib/rc.h> /* GetRCState */
#include <klib/refcount.h> /* KRefcount */
#include <klib/status.h> /* STSMSG */
#include <klib/text.h> /* String */

#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h> /* calloc */
#include <string.h> /* memset */

#include <stdio.h> /* fprintf */

/******************************************************************************/

static void *_NotImplementedP(const char *func) {
    PLOGERR(klogErr, (klogErr, -1,
        "$(func): is not implemented", "func=%s", func));
    return 0;
}

static size_t _NotImplemented(const char *func)
{   return (size_t)_NotImplementedP(func); }

/******************************************************************************/
/** Packed2naRead **/

static
void _Packed2naReadPrint(const Packed2naRead *self, const void *blob)
{
    static char *b = NULL;
    static size_t bsz = 0;
    int last = 0;
    unsigned i = 0;
    size_t num_writ = 0;
    size_t size = 0;
    assert(self);
    fflush(stderr);
    if (b == NULL) {
        bsz = self->length_in_bases + 64;
        b = malloc(bsz);
        if (b == NULL)
        {   return; }
    }
    else if (bsz < self->length_in_bases + 64) {
        char *tmp = NULL;
        bsz = self->length_in_bases + 64;
        tmp = realloc(b, bsz);
        if (tmp == NULL)
        {   return; }
        b = tmp;
    }
    if (string_printf(b, bsz, &num_writ, "%lu\t%p\t", self->read_id, blob))
    {
        return;
    }
    assert(num_writ < bsz);
    last = string_measure(b, &size);
    {
        unsigned ib = 0;
        uint32_t offset = self->offset_to_first_bit;
        for (ib = 0; ib < self->length_in_bases; ++ib) {
            int j = 0;
            uint8_t a[4];
            uint8_t u = 0;
            if (i >= self->length_in_bases)
            {   break; }
            u = ((uint8_t*)self->starting_byte)[ib];
            a[0] = (u >> 6) & 3;
            a[1] = (u >> 4) & 3;
            a[2] = (u >> 2) & 3;
            a[3] = u & 3;
            for (j = offset / 2; j < 4; ++j) {
                const char c[] = "ACGT";
                uint8_t h = a[j];
                if (i >= self->length_in_bases)
                {   break; }
                assert(h < 4);
                b[last++] = c[h];
                ++i;
            }
            offset = 0;
        }
        b[last++] = '\n';
        b[last++] = '\0';
        fprintf(stderr, "%s", b);
        fflush(stderr);
    }
}

/******************************************************************************/
/** ReaderCols **/

static void ReaderColsReset(ReaderCols *self) {
    assert(self);

    self   ->col_PRIMARY_ALIGNMENT_ID
     = self->col_READ_FILTER
     = self->col_READ_LEN
     = self->col_TRIM_LEN
     = self->col_TRIM_START
     = 0;
}

static
void ReaderColsFini(ReaderCols *self)
{
    assert(self);

    free(self->primary_alignment_id);
    free(self->read_filter);
    free(self->read_len);

    memset(self, 0, sizeof *self);
}

/* readId is an index in primary_alignment_id : 0-based */
static bool ReaderColsIsReadCompressed(const ReaderCols *self,
    uint32_t readId)
{
    assert(self);

    if (self->col_PRIMARY_ALIGNMENT_ID == 0) {
        return false; /* not cSRA */
    }

    assert(self->primary_alignment_id);

    if (self->primary_alignment_id[readId] != 0) {
        return true; /* has alignment == compressed */
    }
    else {
        /* does not have alignment == not compressed == is stored in CMP_READ */
        return false;
    }
}

/******************************************************************************/

static
uint32_t _VCursorReadArray(const VCursor *self,
    int64_t row_id,
    uint32_t col,
    void **buffer,
    uint8_t elem_size,
    uint8_t nReads,
    const char *name)
{
    rc_t rc = 0;
    uint32_t row_len = 0;

    assert(buffer && elem_size && nReads && name);

    if (*buffer == NULL) {
        *buffer = calloc(nReads, elem_size);
        if (*buffer == NULL) {
            return eVdbBlastMemErr;
        }
    }

    rc = VCursorReadDirect(self,
        row_id, col, 8, *buffer, nReads * elem_size * 8, &row_len);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc,
            "Error in VCursorReadDirect($(name))", "name=%s", name));
    }

/* TODO: needs to be verified what row_len is expected
    if (row_len != 1) return eVdbBlastErr; */

    S

    return rc == 0 ? eVdbBlastNoErr : eVdbBlastErr;
}

static
uint32_t _VCursorAddReaderCols(const VCursor *self,
    ReaderCols *cols, bool cSra, uint32_t tableId)
{
    rc_t rc = 0;

    assert(self && cols);

    if (rc == 0) {
        const char name[] = "READ_FILTER";
        rc = VCursorAddColumn(self, &cols->col_READ_FILTER, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpen($(name))", "name=%s", name));
        }
        else {
            assert(cols->col_READ_FILTER);
        }
    }

    if (rc == 0) {
        const char name[] = "READ_LEN";
        rc = VCursorAddColumn(self, &cols->col_READ_LEN, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpen($(name))", "name=%s", name));
        }
        else {
            assert(cols->col_READ_LEN);
        }
    }

    if (rc == 0) {
        const char name[] = "TRIM_LEN";
        rc = VCursorAddColumn(self, &cols->col_TRIM_LEN, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpen($(name))", "name=%s", name));
        }
        else {
            assert(cols->col_TRIM_LEN);
        }
    }

    if (rc == 0) {
        const char name[] = "TRIM_START";
        rc = VCursorAddColumn(self, &cols->col_TRIM_START, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpen($(name))", "name=%s", name));
        }
        else {
            assert(cols->col_TRIM_START);
        }
    }

    if (rc == 0 && cSra) {
        const char name[] = "PRIMARY_ALIGNMENT_ID";
        rc = VCursorAddColumn(self, &cols->col_PRIMARY_ALIGNMENT_ID, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpen($(name))", "name=%s", name));
        }
        else {
            assert(cols->col_PRIMARY_ALIGNMENT_ID);
        }
    }

    return rc ? eVdbBlastErr : eVdbBlastNoErr;
}

static uint32_t _VCursorReadReaderCols(const VCursor *self,
    const ReadDesc *desc, ReaderCols *cols, bool * empty)
{
    uint32_t status = eVdbBlastNoErr;
    rc_t rc = 0;
    uint32_t row_len = ~0;
    int64_t row_id = 0;
    uint8_t nReads = 0;

    assert(desc && cols && desc->run && empty);

    * empty = false;

    row_id = desc->spot;
    nReads = desc->run->rd.nReads;
    assert(desc->nReads);
    nReads = desc->nReads;

    if (cols->nReadsAllocated != 0 && cols->nReadsAllocated < nReads) {
        /* LOG */

        /* TODO: find a better way/place to realloc cols data buffers */
        free(cols->primary_alignment_id);
        cols->primary_alignment_id = NULL;
        free(cols->read_filter);
        cols->read_filter = NULL;
        free(cols->read_len);
        cols->read_len = NULL;
    }

    status = _VCursorReadArray(self, row_id, cols->col_READ_LEN,
        (void **)&cols->read_len, sizeof *cols->read_len, nReads,
        "READ_LEN");
    if (status != eVdbBlastNoErr)
        return status;
    if ( nReads == 1 && cols->read_len[0] == 0 ) {
        * empty = true;
        return status;
    }

    status = _VCursorReadArray(self, row_id, cols->col_READ_FILTER,
        (void **)&cols->read_filter, sizeof *cols->read_filter, nReads,
        "READ_FILTER");
    if (status != eVdbBlastNoErr)
    {   return status; }

    if (cols->col_PRIMARY_ALIGNMENT_ID != 0) {
        status = _VCursorReadArray(self, row_id, cols->col_PRIMARY_ALIGNMENT_ID,
            (void **)&cols->primary_alignment_id,
            sizeof *cols->primary_alignment_id, nReads, "PRIMARY_ALIGNMENT_ID");
        if (status != eVdbBlastNoErr) {
            return status;
        }
    }

    cols->nReadsAllocated = nReads;

    rc = VCursorReadDirect(self, row_id, cols->col_TRIM_LEN,
        8 * sizeof cols->TRIM_LEN, &cols->TRIM_LEN, sizeof cols->TRIM_LEN,
        &row_len);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc, "Error in VCursorReadDirect"
            " TRIM_LEN, spot=$(spot))", "spot=%ld", row_id));
        return eVdbBlastErr;
    }
    else if (row_len != 1) {
        STSMSG(1, ("Error: VCursorReadDirect(TRIM_LEN, spot=%lu) "
            "returned row_len=%u", row_id, row_len));
/* TODO */ return eVdbBlastErr;
    }

    rc = VCursorReadDirect(self, row_id, cols->col_TRIM_START,
        8 * sizeof cols->TRIM_LEN, &cols->TRIM_START, sizeof cols->TRIM_START,
        &row_len);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc, "Error in VCursorReadDirect"
            " TRIM_START, spot=$(spot))", "spot=%ld", row_id));
        return eVdbBlastErr;
    }
    else if (row_len != 1) {
        STSMSG(1, ("Error: VCursorReadDirect(TRIM_START, spot=%lu) "
            "returned row_len=%u", row_id, row_len));
/* TODO */ return eVdbBlastErr;
    }

    return status;
}

/******************************************************************************/

static
bool _ReadDescNextRead(ReadDesc *self, VdbBlastStatus *status)
{
    uint32_t read = 0;
    int i = 0;
    const RunDesc *rd = NULL;
    uint8_t nReads = 1;

    assert(self && self->run && status);

    rd = &self->run->rd;

    if (_VdbBlastRunVarReadNum(self->run)) {
        bool found = false;
        if (_ReadDescFindNextRead(self, &found)) {
            S
            return false;
        }
        else if (found) {
            *status = _ReadDescFixReadId(self);
            S
            return true;
        }
        else {
            S
            return false;
        }
    }

    if (rd->nBioReads == 0) {
        S
        return false;
    }

    if (self->tableId == VDB_READ_UNALIGNED) {
        nReads = rd->nReads;
    }

    for (i = self->read + 1; i <= nReads; ++i) {
        if (rd->readType[i - 1] & SRA_READ_TYPE_BIOLOGICAL) {
            S
            read = i;
            break;
        }
    }

    if (read == 0) {
        if (++self->spot > rd->spotCount) {
            S
            return false;
        }

        for (i = 1; i <= nReads; ++i) {
            if (rd->readType[i - 1] & SRA_READ_TYPE_BIOLOGICAL) {
                S
                read = i;
                break;
            }
        }
    }

    if (read > 0) {
        S
        self->read = read;
        ++self->read_id;
        *status = _ReadDescFixReadId(self);
    }
    else
    {   S }

    return read;
}

static
bool _ReadDescSameRun(const ReadDesc *self)
{
    assert(self);

    if (self->prev == NULL && self->run == NULL) {
        S
        return false;
    }

    S
    return self->prev == self->run;
}

/******************************************************************************/

static uint64_t _Reader2naReset(Reader2na *self,
    bool *alive, VdbBlastStatus *status)
{
    uint32_t mode = 0;
    VdbBlastRun *run = NULL;
    uint64_t read_id = 0;
    uint64_t table_id = 0;

    assert(self && alive);

    mode = self->mode;
    run = (VdbBlastRun*)self->desc.run;
    read_id = self->desc.read_id;
    table_id = self->desc.tableId;

    VCursorRelease(self->curs);
    ReaderColsFini(&self->cols);

    _ReferencesWhack(self->refs);

    memset(self, 0, sizeof *self);

    if (table_id == VDB_READ_UNALIGNED && self->mode & VDB_READ_ALIGNED) {
        read_id += _VdbBlastRunGetNumAlignments(run, status);
        self->desc.tableId = VDB_READ_ALIGNED;
        *alive = true;
    }
    else {
        *alive = false;
    }

    self->mode = mode;

    return read_id;
}

static
bool _Reader2naEor(const Reader2na *self)
{
    assert(self);
    S
    return self->eor;
}

static uint32_t _Reader2naReadReaderCols(Reader2na *self, bool * empty) {
    assert(self);
    return _VCursorReadReaderCols(self->curs, &self->desc, &self->cols, empty);
}

static
uint32_t _Reader2naGetBlob(Reader2na *self,
    const VBlob **blob,
    const ReadDesc *desc,
    int64_t *first,
    uint64_t *count)
{
    bool fresh = false;

    assert(self && blob && desc && first && count
        && desc->run && desc->run->path);

    for (fresh = false; ;) {
        rc_t rc = 0;

        if (*blob == NULL) {
            rc = VCursorGetBlobDirect
                (self->curs, blob, desc->spot, self->col_READ);
            if (rc) {
                PLOGERR(klogInt, (klogInt, rc, "Error in VCursorGetBlobDirect"
                    "($(path), READ, spot=$(spot))",
                    "path=%s,spot=%ld", desc->run->path, desc->spot));
                return eVdbBlastErr;
            }

            fresh = true;
        }

        rc = VBlobIdRange(*blob, first, count);
        if (rc) {
            PLOGERR(klogInt, (klogInt, rc, "Error in VBlobIdRange($(path))",
                "path=%s", desc->run->path));
            return eVdbBlastErr;
        }

        if ((int64_t)desc->spot >= *first &&
            desc->spot < *first + *count) {
            S
            return eVdbBlastNoErr;
        }

        if (fresh) {
            S
            return eVdbBlastErr;
        }

        rc = VBlobRelease(*blob);
        *blob = NULL;
        if (rc) {
            PLOGERR(klogInt, (klogInt, rc, "Error in VBlobRelease($(path))",
                "path=%s", desc->run->path));
            return eVdbBlastErr;
        }
    }

    S
    return eVdbBlastErr;
}

static
uint32_t _Reader2naCalcReadReaderColsParams(const ReadDesc *desc,
    const ReaderCols *cols,
    uint32_t *start,
    uint32_t min_read_length)
{
    uint32_t i = 1;
    uint32_t to_read = 0;
    assert(desc && cols && start);
    assert(desc->run && desc->run->path);

    *start = 0;

    assert(cols->read_len && cols->read_filter);
    for (i = 1; i < desc->read; ++i) {
        if (!_VdbBlastRunVarReadNum(desc->run)) {
            assert(i <= desc->run->rd.nReads);
        }

        /* do not count CMP_READ-s where primary_alignment_id != 0
           as are not stored in CMP_READ) */
        if (!ReaderColsIsReadCompressed(cols, i - 1)) {
            *start += cols->read_len[i - 1];
        }
    }
    S

    if (cols->read_len[desc->read - 1] == 0) {
        S
        DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
            ("%s: %s:%d:%d(%d): READ_LEN=0\n",
            __func__, desc->run->path, desc->spot, desc->read, desc->read_id));
        return 0;
    }
    else if (cols->read_filter[desc->read - 1] != READ_FILTER_PASS) {
        S
        DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
            ("%s: %s:%d:%d(%d): READ_FILTER != READ_FILTER_PASS\n",
            __func__, desc->run->path, desc->spot, desc->read, desc->read_id));
        return 0;
    }
    else if (cols->TRIM_LEN > 0
        && *start >= cols->TRIM_START + cols->TRIM_LEN)
    {
        return 0;
    }
    else {
        uint32_t end = 0;

        /* do not count CMP_READ-s where primary_alignment_id != 0
           as are not stored in CMP_READ) */
        if (ReaderColsIsReadCompressed(cols, desc->read - 1)) {
            to_read = 0;
            S
            DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
                ("%s: %s:%d:%d(%d): PRIMARY_ALIGNMENT_ID != 0\n", __func__,
                 desc->run->path, desc->spot, desc->read, desc->read_id));
        }
        else {
            to_read = cols->read_len[desc->read - 1];
            end = *start + to_read;
            if (cols->TRIM_LEN > 0 && cols->TRIM_START > (int32_t)*start) {
                uint32_t delta = cols->TRIM_START - *start;
                if (to_read > delta) {
                    *start = cols->TRIM_START;
                    to_read -= delta;
                    assert(*start + to_read == end);
                }
                else {
                    to_read = 0;
                }
            }
        }
        if (to_read > 0) {
            if (cols->TRIM_LEN > 0 &&
                end > (cols->TRIM_START + cols->TRIM_LEN))
            {
                uint32_t delta = end - (cols->TRIM_START + cols->TRIM_LEN);
                assert(delta < to_read);
                to_read -= delta;
            }
            if (to_read < min_read_length) {
                S
                DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
                    ("%s: %s:%d:%d(%d): READ_LEN=%d: TOO SHORT (<%d)\n",
                    __func__, desc->run->path, desc->spot, desc->read,
                    desc->read_id, cols->read_len[desc->read - 1],
                    min_read_length));
                return 0;
            }
            else {
                DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
                    ("%s: %s:%d:%d(%d): READ_LEN=%d\n", __func__,
                    desc->run->path, desc->spot, desc->read,
                    desc->read_id, cols->read_len[desc->read - 1]));
            }
        }
    }

    return to_read;
}

static
bool _Reader2naNextData(Reader2na *self,
    const VBlob *blob,
    uint32_t *status,
    Packed2naRead *out,
    uint32_t min_read_length)
{
    uint32_t start = 0;
    uint32_t to_read = 0;
    const ReadDesc *desc = NULL;
    bool empty = false;
    assert(self && status && out && self->curs);
    desc = &self->desc;
    memset(out, 0, sizeof *out);

    *status = _Reader2naReadReaderCols(self, &empty);
    if (*status != eVdbBlastNoErr) {
        S
        return false;
    }
    if (empty)
        return true;
    if (!self->curs || !desc->run) {
        S
        *status = eVdbBlastErr;
        return false;
    }

    assert(self->cols.read_len && self->cols.read_filter);
    if (!_VdbBlastRunVarReadNum(desc->run)) {
        assert(desc->read <= desc->run->rd.nReads);
    }

    to_read = _Reader2naCalcReadReaderColsParams(&self->desc, &self->cols,
        &start, min_read_length);
    if (to_read == 0) {
        return true;
    }
    else {
        uint32_t elem_bits = 0;
        rc_t rc = 0;

        S
        DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
            ("%s: %s:%d:%d(%d): READ_LEN=%d\n", __func__,
            self->desc.run->path, self->desc.spot, self->desc.read,
            self->desc.read_id, self->cols.read_len[desc->read - 1]));

        rc = VBlobCellData(blob, desc->spot, &elem_bits,
            (const void **)&out->starting_byte, &out->offset_to_first_bit,
            &out->length_in_bases);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                "for $(acc)/READ/$(spot)) /2na",
                "acc=%s,spot=%zu", self->desc.run->acc, self->desc.spot));
            DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
               ("%s: %s:%d:%d(%d): READ_LEN=%d: "
                "ERROR WHILE READING: SKIPPED FOR NOW\n", __func__,
                self->desc.run->path, self->desc.spot, self->desc.read,
                self->desc.read_id, self->cols.read_len[desc->read - 1]));
            /* special case */
            *status = eVdbBlastErr;
            return true;
        }

        if (elem_bits != 2) {
            S
            *status = eVdbBlastErr;
            return false;
        }

        if (out->length_in_bases < start) {
            S
            *status = eVdbBlastErr;
            return false;
        }

        out->offset_to_first_bit += start * 2;
        S

        out->length_in_bases = to_read;
        while (out->offset_to_first_bit >= 8) {
            out->starting_byte = ((uint8_t*)out->starting_byte) + 1;
            out->offset_to_first_bit -= 8;
        }
        out->read_id = desc->read_id;
        S
        return true;
    }
}

static
uint32_t _Reader2naData(Reader2na *self,
    Data2na *data,
    VdbBlastStatus *status,
    Packed2naRead *buffer,
    uint32_t buffer_length,
    uint32_t min_read_length)
{
    ReadDesc *desc = NULL;
    uint32_t n = 0;
    int64_t first = 0;
    uint64_t count = 0;
    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }
    *status = eVdbBlastErr;
    if (buffer_length && buffer == NULL) {
        S
        return 0;
    }

    assert(self && self->curs && data);
    desc = &self->desc;
    *status = _Reader2naGetBlob(self, &data->blob, desc, &first, &count);
    if (*status == eVdbBlastErr) {
        S
        return 0;
    }

    for (n = 0; n < buffer_length; ) {
        Packed2naRead *p = buffer + n;
        bool ignorable
            = _Reader2naNextData(self, data->blob, status, p, min_read_length);
        if (*status == eVdbBlastErr) {
            if (ignorable) {
                /* special case */
                if (n > 0) { /* let's retry during next call */
                    S
                    *status = eVdbBlastNoErr;
                }
                else
                {   S }
                return n;
            }
            S
            return 0;
        }
        if (p->length_in_bases > 0)
        {   ++n; }
        if (!_ReadDescNextRead(desc, status)) {
            if (*status != eVdbBlastNoErr) {
                return 0;
            }
            S
            self->eor = true;
            break;
        }
        if (desc->spot >= first + count) {
            S
            break;
        }
    }

    *status = eVdbBlastNoErr;
    return n;
}
/* struct Packed2naRead {
    uint64_t read_id;
    void *starting_byte;
    uint32_t offset_to_first_bit;
    uint32_t length_in_bases; }; */

static
uint64_t _Reader2naRead(Reader2na *self,
    uint32_t *status,
    uint64_t *read_id,
    size_t *starting_base,
    uint8_t *buffer,
    size_t buffer_size,
    uint32_t min_read_length)
{
    uint32_t num_read = 0;
    uint32_t to_read = 0;
    ReadDesc *desc = NULL;
    uint32_t start = 0;
    uint32_t remaining = 0;
    rc_t rc = 0;
    bool empty = false;
    assert(self && status && read_id && starting_base);
    desc = &self->desc;
    *read_id = desc->read_id;
    *starting_base = self->starting_base;

    if (_Reader2naEor(self)) {
        S
        *status = eVdbBlastNoErr;
        return 0;
    }

    *status = _Reader2naReadReaderCols(self, &empty);
    if (*status != eVdbBlastNoErr) {
        S
        return 0;
    }

    if (!empty) {
        *status = eVdbBlastErr;
        if (!self->curs || !desc->run) {
            S
            return 0;
        }

        assert(desc->run->path);

        *status = eVdbBlastNoErr;
        if (!_VdbBlastRunVarReadNum(desc->run) && desc->run->rd.nBioReads == 0) {
            S
            return 0;
        }

        to_read = _Reader2naCalcReadReaderColsParams(&self->desc, &self->cols,
            &start, min_read_length);
        if (to_read <= self->starting_base) {
            S
            DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST), (
                "%s: %s:%d:%d(%d): READ_LEN=%d: TOO SHORT (starting_base=%d)\n",
                __func__, desc->run->path, desc->spot, desc->read,
                desc->read_id, self->cols.read_len[desc->read - 1],
                self->starting_base));
            to_read = 0;
        }
        else {
            to_read -= (uint32_t)self->starting_base;
            start   += (uint32_t)self->starting_base;
        }
    }

    if (to_read > 0) {
        S
        rc = VCursorReadBitsDirect(self->curs, desc->spot, self->col_READ, 2,
            start, buffer, 0, (uint32_t)buffer_size * 4, &num_read, &remaining);
        if (rc) {
            if (rc == SILENT_RC
                (rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient))
            {
                S
                rc = 0;
                num_read = (uint32_t)buffer_size * 4;
            }
            else {
                PLOGERR(klogInt, (klogInt, rc,
                    "Error in VCursorReadBitsDirect"
                    "($(path), READ, spot=$(spot))", "path=%s,spot=%ld",
                    desc->run->path, desc->spot));
                *status = eVdbBlastErr;
                return 0;
            }
        }
        *status
            = (num_read == 0 && remaining == 0) ? eVdbBlastErr : eVdbBlastNoErr;
        S
    }

    if (num_read >= to_read) {
        self->starting_base = 0;
        num_read = to_read;
        if (!_ReadDescNextRead(desc, status))
        {   self->eor = true; }
        S
    }
    else {
        self->starting_base += num_read;
        S
    }

    return num_read;
}

static VdbBlastStatus _VdbBlastRunMakeReaderColsCursor(
    const VdbBlastRun *self, const VCursor **curs, uint32_t *read_col_idx,
    bool INSDC2na, ReaderCols *cols, const ReadDesc *desc)
{
    rc_t rc = 0;
    const VTable *tbl = NULL;
    bool cmpRead = false;
    const char *read_col_name = INSDC2na ? "(INSDC:2na:packed)READ"
                                         : "(INSDC:4na:bin)READ";
    assert(self && self->obj && desc);

    if (desc->tableId == VDB_READ_ALIGNED) {
        tbl = self->obj->prAlgnTbl;
    }
    else {
        tbl = self->obj->seqTbl;
        cmpRead = self->cSra;
    }

    if (cmpRead) {
        read_col_name = INSDC2na ? "(INSDC:2na:packed)CMP_READ"
                                 : "(INSDC:4na:bin)CMP_READ";
    }

    rc = _VTableMakeCursor(tbl, curs, read_col_idx, read_col_name, self->acc);

    if (rc == 0) {
        assert(*curs);
        return _VCursorAddReaderCols(*curs, cols, cmpRead, desc->tableId);
    }

    return eVdbBlastErr;
}

static VdbBlastStatus _Reader2naOpenCursor(Reader2na *self) {
    const ReadDesc *desc = NULL;
    const VdbBlastRun *run = NULL;

    assert(self);

    desc = &self->desc;
    run = desc->run;

    assert(run && self->curs == NULL && run->obj);

    return _VdbBlastRunMakeReaderColsCursor(run, &self->curs, &self->col_READ,
        true, &self->cols, desc);
    /*return _VTableMakeReaderColsCursor(run->obj->seqTbl, &self->curs,
        &self->col_READ, true, &self->cols, run->cSra);*/
}

/******************************************************************************/

void _Core2naFini(Core2na *self) {
    VdbBlastStatus status;
    bool keep = false;
    assert(self);
    _Reader2naReset(&self->reader, &keep, &status);
    KLockRelease(self->mutex);
    memset(self, 0, sizeof *self);
}

static
uint32_t _Core2naOpen1stRun(Core2na *self,
    RunSet *runs,
    uint64_t initial_read_id)
{
    Reader2na *reader = NULL;
    VdbBlastStatus status = eVdbBlastNoErr;

    assert(self && runs);

    reader = &self->reader;

    if (reader->curs) {
        return eVdbBlastNoErr;
    }

    if (runs->run && runs->krun) {
        self->initial_read_id = initial_read_id;
        if (reader->mode != VDB_READ_REFERENCE) {
            status = _RunSetFindReadDesc
                (runs, initial_read_id, &reader->desc);
            if (status == eVdbBlastNoErr) {
                status = _Reader2naOpenCursor(reader);
                if (status != eVdbBlastNoErr) {
                    bool keep = true;
                    S
                    _Reader2naReset(reader, &keep, &status);
                }
            }
            else
            {   S }
        }
        else if (reader->refs == NULL) {
            reader->refs = _RunSetMakeReferences(runs, &status);
            if (reader->refs == NULL) {
                status = eVdbBlastErr;
            }
        }
    }
    else {
        S
        self->eos = true;
    }

    return status;
}

static VdbBlastStatus _Core2naOpenNextRunOrTbl
    (Core2na *core, const RunSet *runs)
{
    VdbBlastStatus status = eVdbBlastNoErr;
    uint64_t read_id = 0;
    ReadDesc *desc = NULL;
    Reader2na *reader = NULL;
    bool keep = true;

    assert(core && runs);

    reader = &core->reader;
    desc = &reader->desc;

    assert(desc->run);

    read_id = _Reader2naReset(reader, &keep, &status) + 1;
    if (status != eVdbBlastNoErr) {
        return status;
    }

    if (!keep) {
        if (core->irun >= runs->krun - 1) { /* No more runs to read */
            S
            core->eos = true;
            return eVdbBlastNoErr;
        }
        ++core->irun;
    }

    while (core->irun < runs->krun) {
        uint32_t status = eVdbBlastNoErr;

        VdbBlastRun *run = &runs->run[core->irun];
        if (run == NULL) {
            S
            return eVdbBlastErr;
        }

        status = _VdbBlastRunFillReadDesc(run, 0, desc);
        if (status != eVdbBlastNoErr) {
            S
            return status;
        }

        desc->read_id = read_id;
        status = _ReadDescFixReadId(desc);
        if (status != eVdbBlastNoErr) {
            return status;
        }
        status = _Reader2naOpenCursor(reader);
        if (status == eVdbBlastNoErr) {
            S
        }
        else {
            S
        }

        return status;
    }

    S
    return eVdbBlastNoErr;
}

static
uint32_t _Core2naDataSeq(Core2na *self,
    Data2na *data,
    const RunSet *runs,
    VdbBlastStatus *status,
    Packed2naRead *buffer,
    uint32_t buffer_length)
{
    uint32_t num_read = 0;

    assert(self && data && status && runs);

    *status = eVdbBlastNoErr;

    while (*status == eVdbBlastNoErr && num_read == 0) {
        if (_Reader2naEor(&self->reader) || data->irun != self->irun) {
            S
            VBlobRelease(data->blob);
            data->blob = NULL;
        }
        if (_Reader2naEor(&self->reader)) {
            S
            *status = _Core2naOpenNextRunOrTbl(self, runs);
            if (*status != eVdbBlastNoErr) {
                STSMSG(1, ("Error: "
                    "failed to VdbBlast2naReaderData: cannot open next read"));
                return 0;
            }
        }
        if (data->irun != self->irun)
        {   data->irun = self->irun; }

        if (self->eos) {
            STSMSG(1, ("VdbBlast2naReaderData: End Of Set"));
            return 0;
        }

        num_read = _Reader2naData(&self->reader, data, status,
            buffer, buffer_length, self->min_read_length);
    }

    if (*status == eVdbBlastNoErr) {
        STSMSG(3, ("VdbBlast2naReaderData = %ld", num_read));
    }
    else {
        STSMSG(1, ("Error: failed to VdbBlast2naReaderData"));
    }

    return num_read;
}

static
uint32_t _Core2naData(Core2na *self,
    Data2na *data,
    const RunSet *runs,
    VdbBlastStatus *status,
    Packed2naRead *buffer,
    uint32_t buffer_length)
{
    assert(self);

    if (self->reader.mode != VDB_READ_REFERENCE) {
        return _Core2naDataSeq(self, data, runs, status, buffer, buffer_length);
    }
    else {
        return _Core2naDataRef(self, data, status, buffer, buffer_length);
    }
}

static
uint64_t _Core2naReadSeq(Core2na *self,
    const RunSet *runs,
    uint32_t *status,
    uint64_t *read_id,
    size_t *starting_base,
    uint8_t *buffer,
    size_t buffer_size)
{
    uint64_t num_read = 0;

    assert(self && status && runs);

    if (buffer_size == 0) {
        S
        *status = eVdbBlastErr;
        return 0;
    }

    *status = eVdbBlastNoErr;

    while (*status == eVdbBlastNoErr && num_read == 0) {
        if (_Reader2naEor(&self->reader)) {
            S
            *status = _Core2naOpenNextRunOrTbl(self, runs);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }
        }

        if (self->eos) {
            S
            return 0;
        }

        num_read = _Reader2naRead(&self->reader, status,
            read_id, starting_base, buffer, buffer_size, self->min_read_length);
        S
    }

    return num_read;
}

uint64_t _Core2naRead(Core2na *self, const RunSet *runs,
    uint32_t *status, uint64_t *read_id, size_t *starting_base,
    uint8_t *buffer, size_t buffer_size)
{
    assert(self);

    if (self->reader.mode != VDB_READ_REFERENCE) {
        return _Core2naReadSeq(self, runs, status,
            read_id, starting_base, buffer, buffer_size);
    }
    else {
        return _Core2naReadRef(self, status, read_id, buffer, buffer_size);
    }
}

void _Core4naFini(Core4na *self) {
    assert(self);

    VCursorRelease(self->curs);
    VBlobRelease(self->blob);
    KLockRelease(self->mutex);

    ReaderColsFini(&self->cols);

    memset(self, 0, sizeof *self);
}

static size_t _Core4naReadSeq(Core4na *self, const RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t starting_base,
    uint8_t *buffer, size_t buffer_length)
{
    uint32_t num_read = 0;
    ReadDesc *desc = NULL;
    assert(self && runs && status);
    desc = &((Core4na*)self)->desc;

    *status = _RunSetFindReadDesc(runs, read_id, desc);
    if (*status != eVdbBlastNoErr) {
        S
    }
    else {
        rc_t rc = 0;
        if (!_ReadDescSameRun(desc)) {
            S
            ReaderColsReset(&self->cols);
            VCursorRelease(self->curs);
            ((Core4na*)self)->curs = NULL;
            assert(desc->run && desc->run->obj);
            *status = _VdbBlastRunMakeReaderColsCursor(desc->run,
                &((Core4na*)self)->curs, &((Core4na*)self)->col_READ, false,
                &(((Core4na*)self)->cols), desc);
            S
        }

        if (*status == eVdbBlastNoErr && rc == 0) {
            uint32_t remaining = 0;
            uint32_t start = 0;
            uint32_t to_read = 0;
            bool empty = false;
            assert(desc->run && desc->read <= desc->run->rd.nReads
                && desc->run->path);
            *status = _VCursorReadReaderCols(self->curs,
                desc, &((Core4na*)self)->cols, &empty);
            if (*status == eVdbBlastNoErr) {
                if (!empty) {
                    assert(self->cols.read_len && self->cols.read_filter);
                    to_read = _Reader2naCalcReadReaderColsParams(&self->desc,
                        &self->cols, &start, self->min_read_length);
                }
                if (to_read == 0) {
                    /* When _Reader2naCalcReadReaderColsParams returns 0
                    then this read is skipped by 2na reader (usually filtered)
                    and should not be accessed by 4na reader */
                    *status = eVdbBlastInvalidId;
                    S
                }
                else {
                    if (to_read >= starting_base) {
                        to_read -= (uint32_t)starting_base;
                        start   += (uint32_t)starting_base;
                        if (buffer_length < to_read) {
                            to_read = (uint32_t)buffer_length;
                        }
                        S
                        rc = VCursorReadBitsDirect(self->curs,
                            desc->spot, self->col_READ, 8,
                            start, buffer, 0, to_read, &num_read, &remaining);
                        if (rc != 0) {
                            PLOGERR(klogInt, (klogInt, rc,
                                "Error in VCursorReadBitsDirect"
                                "($(path), READ, spot=$(spot))",
                                "path=%s,spot=%ld",
                                desc->run->path, desc->spot));
                        }
                    }
                    else {
                        S
                        *status = eVdbBlastErr;
                    }
                }
            }
        }

        if (*status == eVdbBlastNoErr)
        {   *status = rc ? eVdbBlastErr : eVdbBlastNoErr; }
    }

    S
    return num_read;
}

static size_t _Core4naRead(Core4na *self, const RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t starting_base,
    uint8_t *buffer, size_t buffer_length)
{
    assert(self);

    if (self->mode != VDB_READ_REFERENCE) {
        return _Core4naReadSeq(self, runs, status,
            read_id, starting_base, buffer, buffer_length);
    }
    else {
        return _Core4naReadRef(self, runs, status,
            read_id, starting_base, buffer, buffer_length);
    }
}

static const uint8_t* _Core4naDataSeq(Core4na *self, const RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t *length)
{
    ReadDesc *desc = NULL;
    assert(self && runs && status && length);
    desc = &self->desc;

    *length = 0;

    *status = _RunSetFindReadDesc(runs, read_id, desc);
    if (*status != eVdbBlastNoErr)
    {   S }
    else {
        rc_t rc = 0;
        bool empty = false;
        const uint8_t *base = NULL;
        if (!_ReadDescSameRun(desc)) {
            S
            ReaderColsReset(&self->cols);
            VCursorRelease(self->curs);
            self->curs = NULL;
            assert(desc->run && desc->run->obj);
            *status = _VdbBlastRunMakeReaderColsCursor(desc->run, &self->curs,
                &self->col_READ, false, &self->cols, desc);
            S
        }

        if (*status == eVdbBlastNoErr) {
            *status = _VCursorReadReaderCols(self->curs, desc, &self->cols, &empty);
            S
        }

        if (rc == 0 && *status == eVdbBlastNoErr) {
            if (empty) {
                /* empty (read_len=0) reads are skipped by 2na reader
                   and should not be accessed by 4na reader */
                *status = eVdbBlastInvalidId;
                return NULL;
            }

            assert(self->cols.read_len && self->cols.read_filter
                && desc->run->path);

            if (self->cols.read_filter[desc->read - 1]
                != READ_FILTER_PASS)
            {   /* FILTERed reads are not returned by 2na reader:
                   4na readed should not ask for them */
                *status = eVdbBlastInvalidId;
                S
            }
            else if (ReaderColsIsReadCompressed(&self->cols, desc->read - 1)) {
                /* Compressed CMP_READs are not returned by 2na reader:
                   4na readed should not ask for them */
                *status = eVdbBlastInvalidId; /*  */
                S
            }
            else {
                if (self->blob) {
                    VBlobRelease(self->blob);
                    self->blob = NULL;
                }

                if (rc == 0 && *status == eVdbBlastNoErr) {
                    rc = VCursorGetBlobDirect
                        (self->curs, &self->blob, desc->spot, self->col_READ);
                    if (rc != 0) {
                        PLOGERR(klogInt, (klogInt, rc,
                            "Error in VCursorGetBlobDirect"
                            "($(path), READ, spot=$(spot))",
                            "path=%s,spot=%ld", desc->run->path, desc->spot));
                    }
                }

                if (rc == 0 && *status == eVdbBlastNoErr) {
                    uint32_t boff = 0;
                    uint32_t elem_bits = 0;
                    uint32_t row_len = 0;

                    rc = VBlobCellData(self->blob, desc->spot,
                        &elem_bits, (const void**)&base, &boff, &row_len);
                    if (rc != 0) {
                        PLOGERR(klogInt, (klogInt, rc, "Error in VBlobCellData"
                            "$(path), READ, spot=$(spot))",
                            "path=%s,spot=%ld", desc->run->path, desc->spot));
                    }
                    else {
                        if (elem_bits != 8) {
                            S
                            *status = eVdbBlastErr;
                            base = NULL;
                        }
                        else {
                            size_t to_read
                                = self->cols.read_len[desc->read - 1];

                            if (to_read < self->min_read_length) {
                        /* Read with read_len < min is not returned by 2naReader
                           - it should not be accessed by 4na reader */
                                S
                                * status = eVdbBlastInvalidId;
                                base = NULL;
                            }
                            else {
                                uint32_t start = 0;
                                to_read = _Reader2naCalcReadReaderColsParams(
                                    &self->desc, &self->cols,
                                    &start, self->min_read_length);
                                base += boff + start;
                                if (row_len >= start) {
                                    row_len -= start;
                                    if (to_read > row_len) {
                                        S
                                        *status = eVdbBlastErr;
                                    }
                                    else {
                                        S
                                        *length = to_read;
                                        if (to_read > 0)
                                            *status = eVdbBlastNoErr;
                                        else {
                     /* Returned read with len == 0 is not returned by 2naReader
                        Here to_read can be 0 because read is trimmed
                        - this read should not be accessed by 4na reader */
                                            * status = eVdbBlastInvalidId;
                                            base = NULL;
                                        }
                                    }
                                }
                                else {
                                    S
                                    *status = eVdbBlastErr;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (*status == eVdbBlastNoErr) {
            if (rc != 0)
            {   *status = eVdbBlastErr; }
        }

        return base;
    }

    return NULL;
}

static const uint8_t* _Core4naData(Core4na *self, const RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t *length)
{
    assert(self);

    if (self->mode != VDB_READ_REFERENCE) {
        return _Core4naDataSeq(self, runs, status, read_id, length);
    }
    else {
        return _Core4naDataRef(self, runs, status, read_id, length);
    }
}

/******************************************************************************/

static const char VDB_BLAST_2NA_READER[] = "VdbBlast2naReader";

struct VdbBlast2naReader {
    KRefcount refcount;
    VdbBlastRunSet *set;
    Data2na data;
    KVdbBlastReadMode mode;
};

static
VdbBlast2naReader *_VdbBlastRunSetMake2naReader(VdbBlastRunSet *self,
    VdbBlastStatus *status,
    uint64_t initial_read_id,
    Core2na *core2na,
    KVdbBlastReadMode mode)
{
    VdbBlast2naReader *item = NULL;
    assert(self && status);
    if (core2na == NULL) {
        core2na
            = mode != VDB_READ_REFERENCE ? &self->core2na : &self->core2naRef;
    }
    else {
        core2na->initial_read_id = initial_read_id;
    }
    core2na->reader.mode = mode;
    if (!core2na->hasReader) {
        _VdbBlastRunSetBeingRead(self);
        *status = _Core2naOpen1stRun(core2na, &self->runs, initial_read_id);
        if (*status != eVdbBlastNoErr) {
            STSMSG(1, ("Error: failed to create VdbBlast2naReader: "
                "cannot open the first read"));
            return NULL;
        }
        core2na->initial_read_id = initial_read_id;
        core2na->reader.mode = mode;
        core2na->hasReader = true;
    }
    else if (core2na->initial_read_id != initial_read_id) {
        STSMSG(1, ("Error: failed to create VdbBlast2naReader"
            "(initial_read_id=%ld): allowed initial_read_id=%ld",
            initial_read_id, core2na->initial_read_id));
        *status = eVdbBlastErr;
        return NULL;
    }
    else if (core2na->reader.mode != mode) {
        STSMSG(1, ("Error: failed to create VdbBlast2naReader"
            "(mode=%d): allowed mode=%d", mode, core2na->reader.mode));
        *status = eVdbBlastErr;
        return NULL;
    }
    item = calloc(1, sizeof *item);
    if (item == NULL) {
        *status = eVdbBlastMemErr;
        return NULL;
    }

    item->set = VdbBlastRunSetAddRef((VdbBlastRunSet*)self);
    item->mode = mode;

    KRefcountInit(&item->refcount, 1,
        VDB_BLAST_2NA_READER, __func__, "2naReader");

    *status = eVdbBlastNoErr;

    return item;
}

LIB_EXPORT
VdbBlast2naReader* CC VdbBlastRunSetMake2naReaderExt(const VdbBlastRunSet *self,
    VdbBlastStatus *status,
    uint64_t initial_read_id,
    KVdbBlastReadMode mode)
{
    VdbBlast2naReader *item = NULL;
    KLock *mutex = NULL;

    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (self) {
        rc_t rc = 0;
        mutex = mode != VDB_READ_REFERENCE
            ? self->core2na.mutex : self->core2naRef.mutex;
        assert(mutex);
        rc = KLockAcquire(mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockAcquire");
        }
        else {
            item = _VdbBlastRunSetMake2naReader
                ((VdbBlastRunSet*)self, status, initial_read_id, NULL, mode);
            rc = KLockUnlock(mutex);
            if (rc != 0) {
                LOGERR(klogInt, rc, "Error in KLockUnlock");
                VdbBlast2naReaderRelease(item);
                item = NULL;
            }
        }
        if (rc != 0) {
            *status = eVdbBlastErr;
        }
    }
    else {
        *status = eVdbBlastErr;
    }

    if (item != NULL) {
        STSMSG(1, (
            "Created VdbBlast2naReader(initial_read_id=%ld)", initial_read_id));
    }
    else {
        STSMSG(1, ("Error: failed to create "
            "VdbBlast2naReader(initial_read_id=%ld)", initial_read_id));
    }

    return item;
}

LIB_EXPORT
VdbBlast2naReader* CC VdbBlastRunSetMake2naReader(const VdbBlastRunSet *self,
    uint32_t *status,
    uint64_t initial_read_id)
{
    return VdbBlastRunSetMake2naReaderExt(self, status, initial_read_id,
        VDB_READ_UNALIGNED);
}

LIB_EXPORT
VdbBlast2naReader* CC VdbBlast2naReaderAddRef(VdbBlast2naReader *self)
{
    if (self == NULL) {
        STSMSG(1, ("VdbBlast2naReaderAddRef(NULL)"));
        return self;
    }

    if (KRefcountAdd(&self->refcount, VDB_BLAST_2NA_READER)
        == krefOkay)
    {
        STSMSG(1, ("VdbBlast2naReaderAddRef"));
        return self;
    }

    STSMSG(1, ("Error: failed to VdbBlast2naReaderAddRef"));
    return NULL;
}

static
void _VdbBlast2naReaderWhack(VdbBlast2naReader *self)
{
    if (self == NULL) {
        return;
    }

    if (self->set != NULL) {
        STSMSG(1, ("Deleting VdbBlast2naReader(initial_read_id=%ld)",
            self->set->core2na.initial_read_id));
    }
    else {
        STSMSG(1, ("Deleting VdbBlast2naReader(self->set=NULL)",
            self->set->core2na.initial_read_id));
    }

    VBlobRelease(self->data.blob);
    VdbBlastRunSetRelease(self->set);

    memset(self, 0, sizeof *self);

    free(self);
}

LIB_EXPORT
void CC VdbBlast2naReaderRelease(VdbBlast2naReader *self)
{
    if (self == NULL) {
        return;
    }

    STSMSG(1, ("VdbBlast2naReaderRelease"));
    if (KRefcountDrop(&self->refcount, VDB_BLAST_2NA_READER) != krefWhack) {
        return;
    }

    _VdbBlast2naReaderWhack(self);
}

LIB_EXPORT
uint64_t CC _VdbBlast2naReaderRead(const VdbBlast2naReader *self,
    uint32_t *status,
    uint64_t *read_id,
    size_t *starting_base,
    uint8_t *buffer,
    size_t buffer_size)
{
    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (self == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    return _VdbBlastRunSet2naRead(self->set, status, read_id,
        starting_base, buffer, buffer_size, self->mode);
}

LIB_EXPORT
uint64_t CC VdbBlast2naReaderRead(const VdbBlast2naReader *self,
    uint32_t *status,
    uint64_t *read_id,
    uint8_t *buffer,
    size_t buffer_size)
{
    size_t starting_base = 0;

    return _VdbBlast2naReaderRead(self,
        status, read_id, &starting_base, buffer, buffer_size);
}

LIB_EXPORT
uint32_t CC VdbBlast2naReaderData(VdbBlast2naReader *self,
    uint32_t *status,
    Packed2naRead *buffer,
    uint32_t buffer_length)
{
    bool verbose = false;
    uint32_t n = 0;
    rc_t rc = 0;
    const Core2na *core2na = NULL;

    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    *status = eVdbBlastErr;

    if (self == NULL)
    {   return 0; }

    if ((buffer_length != 0 && buffer == NULL) || (buffer_length == 0))
    {   return 0; }

    *status = eVdbBlastNoErr;

    assert(self->set);

    core2na = self->mode != VDB_READ_REFERENCE
        ? &self->set->core2na : &self->set->core2naRef;

    rc = KLockAcquire(core2na->mutex);
    if (rc != 0) {
        LOGERR(klogInt, rc, "Error in KLockAcquire");
    }
    if (0 && verbose) {
        char b[256];
        string_printf(b, sizeof b, NULL, "KLockAcquire(%p)\n", core2na->mutex);
        assert(!rc);
        fprintf(stderr, "%s", b);
        fflush(stderr);
    }
    if (rc == 0) {
        n = _Core2naData((Core2na*)core2na, &self->data,
            &self->set->runs, status, buffer, buffer_length);
        S
        if (n > 0 && verbose)
        {   _Packed2naReadPrint(buffer, self->data.blob); }
        if (0 && verbose) {
            char b[256];
            string_printf(b, sizeof b, NULL, "KLockUnlock(%p)\n",
                core2na->mutex);
            assert(!rc);
            fprintf(stderr, "%s", b);
            fflush(stderr);
        }
        rc = KLockUnlock(core2na->mutex);
        if (rc != 0)
        {   LOGERR(klogInt, rc, "Error in KLockUnlock"); }
    }
    if (rc)
    {   *status = eVdbBlastErr; }

    S
    return n;
}

/******************************************************************************/
static const char VDB_BLAST_4NA_READER[] = "VdbBlast4naReader";

struct VdbBlast4naReader {
    KRefcount refcount;
    VdbBlastRunSet *set;
    KVdbBlastReadMode mode;
};

LIB_EXPORT
VdbBlast4naReader* CC VdbBlastRunSetMake4naReaderExt(const VdbBlastRunSet *self,
    uint32_t *status,
    KVdbBlastReadMode mode)
{
    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL) {
        status = &dummy;
    }

    if (self) {
        VdbBlast4naReader *item = calloc(1, sizeof *item);
        if (item == NULL) {
            *status = eVdbBlastMemErr;
            return NULL;
        }

        item->set = VdbBlastRunSetAddRef((VdbBlastRunSet*)self);
        item->mode = mode;

        KRefcountInit(&item->refcount, 1,
            VDB_BLAST_4NA_READER, __func__, "4naReader");

        _VdbBlastRunSetBeingRead(self);

        *status = eVdbBlastNoErr;

        STSMSG(1, ("Created VdbBlast4naReader"));

        return item;
    }
    else {
        *status = eVdbBlastErr;

        STSMSG(1, ("VdbBlastRunSetMake4naReader(self=NULL)"));

        return NULL;
    }
}

LIB_EXPORT
VdbBlast4naReader* CC VdbBlastRunSetMake4naReader(const VdbBlastRunSet *self,
    uint32_t *status)
{
    return VdbBlastRunSetMake4naReaderExt(self, status, VDB_READ_UNALIGNED);
}

LIB_EXPORT
VdbBlast4naReader* CC VdbBlast4naReaderAddRef(VdbBlast4naReader *self)
{
    if (self == NULL) {
        STSMSG(1, ("VdbBlast4naReaderAddRef(NULL)"));
        return self;
    }

    if (KRefcountAdd(&self->refcount, VDB_BLAST_4NA_READER)
        == krefOkay)
    {
        STSMSG(1, ("VdbBlast4naReaderAddRef"));
        return self;
    }

    STSMSG(1, ("Error: failed to VdbBlast4naReaderAddRef"));
    return NULL;
}

LIB_EXPORT
void CC VdbBlast4naReaderRelease(VdbBlast4naReader *self)
{
    if (self == NULL)
    {   return; }

    STSMSG(1, ("VdbBlast4naReaderRelease"));
    if (KRefcountDrop(&self->refcount, VDB_BLAST_4NA_READER) != krefWhack)
    {   return; }

    STSMSG(1, ("Deleting VdbBlast4naReader"));

    VdbBlastRunSetRelease(self->set);

    memset(self, 0, sizeof *self);
    free(self);
}

LIB_EXPORT
size_t CC VdbBlast4naReaderRead(const VdbBlast4naReader *self,
    uint32_t *status,
    uint64_t read_id,
    size_t starting_base,
    uint8_t *buffer,
    size_t buffer_length)
{
    size_t n = 0;
    rc_t rc = 0;
    Core4na *c = NULL;

    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (self == NULL) {
        S
        *status = eVdbBlastErr;
        return 0;
    }

    assert(self->set);

    c = self->mode != VDB_READ_REFERENCE
        ? &self->set->core4na : &self->set->core4naRef;

    rc = KLockAcquire(c->mutex);
    if (rc != 0) {
        LOGERR(klogInt, rc, "Error in KLockAcquire");
    }
    else {
        n = _Core4naRead(c, &self->set->runs, status,
            read_id, starting_base, buffer, buffer_length);
        rc = KLockUnlock(c->mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockUnlock");
        }
    }
    if (rc != 0) {
        *status = eVdbBlastErr;
    }

    if (*status == eVdbBlastNoErr) {
        STSMSG(3, (
            "VdbBlast4naReaderRead(read_id=%ld, starting_base=%ld) = %ld",
            read_id, starting_base, n));
    }
    else {
        STSMSG(2, ("Error: failed to "
            "VdbBlast4naReaderRead(read_id=%ld, starting_base=%ld)",
            read_id, starting_base));
    }

    return n;
}

LIB_EXPORT
const uint8_t* CC VdbBlast4naReaderData(const VdbBlast4naReader *self,
    uint32_t *status,
    uint64_t read_id,
    size_t *length)
{
    const uint8_t *d = NULL;
    rc_t rc = 0;
    Core4na *c = NULL;

    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (self == NULL || length == NULL) {
        if (self == NULL)
        {   STSMSG(1, ("VdbBlast4naReaderData(self=NULL)")); }
        if (length == NULL)
        {   STSMSG(1, ("VdbBlast4naReaderData(length=NULL)")); }
        *status = eVdbBlastErr;
        return 0;
    }

    assert(self->set);

    c = self->mode != VDB_READ_REFERENCE
        ? &self->set->core4na : &self->set->core4naRef;

    rc = KLockAcquire(c->mutex);
    if (rc != 0) {
        LOGERR(klogInt, rc, "Error in KLockAcquire");
    }
    else {
        d = _Core4naData(c, &self->set->runs, status, read_id, length);
        rc = KLockUnlock(c->mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockUnlock");
        }
    }
    if (rc) {
        *status = eVdbBlastErr;
    }

    if (*status == eVdbBlastNoErr) {
        STSMSG(3, ("VdbBlast4naReaderData(read_id=%ld, length=%ld)",
            read_id, *length));
    }
    else {
        STSMSG(1, ("Error: failed to VdbBlast4naReaderData(read_id=%ld)",
            read_id));
    }

    return d;
}

/******************************************************************************/
static const char VDB_BLAST_AA_READER[] = "VdbBlastStdaaReader";

struct VdbBlastStdaaReader {
    KRefcount refcount;
};

LIB_EXPORT
VdbBlastStdaaReader* CC VdbBlastRunSetMakeStdaaReader(
    const VdbBlastRunSet *self,
    uint32_t *status)
{
    VdbBlastStdaaReader *item = NULL;

    uint32_t dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    item = calloc(1, sizeof *item);
    if (item == NULL) {
        *status = eVdbBlastMemErr;
        STSMSG(1, ("Error: failed to create VdbBlastStdaaReader"));
        return NULL;
    }

    KRefcountInit(&item->refcount, 1,
        VDB_BLAST_AA_READER, __func__, "aaReader");

    _VdbBlastRunSetBeingRead(self);

    *status = eVdbBlastNoErr;

    STSMSG(1, ("Created VdbBlastStdaaReader"));

    return item;
}

LIB_EXPORT
VdbBlastStdaaReader* CC VdbBlastStdaaReaderAddRef(
    VdbBlastStdaaReader *self)
{
    if (self == NULL) {
        STSMSG(1, ("VdbBlastStdaaReaderAddRef(NULL)"));
        return self;
    }

    if (KRefcountAdd(&self->refcount, VDB_BLAST_AA_READER) == krefOkay) {
        STSMSG(1, ("VdbBlastStdaaReaderAddRef"));
        return self;
    }

    STSMSG(1, ("Error: failed to VdbBlastStdaaReaderAddRef"));
    return NULL;
}

LIB_EXPORT
void CC VdbBlastStdaaReaderRelease(VdbBlastStdaaReader *self)
{
    if (self == NULL)
    {   return; }

    STSMSG(1, ("VdbBlastStdaaReaderRelease"));
    if (KRefcountDrop(&self->refcount, VDB_BLAST_AA_READER) != krefWhack)
    {   return; }

    STSMSG(1, ("Deleting VdbBlastStdaaReader"));
    free(self);
    memset(self, 0, sizeof *self);
}

LIB_EXPORT
size_t CC VdbBlastStdaaReaderRead(const VdbBlastStdaaReader *self,
    uint32_t *status,
    uint64_t pig,
    uint8_t *buffer,
    size_t buffer_length)
{   return _NotImplemented(__func__); }

LIB_EXPORT
const uint8_t* CC VdbBlastStdaaReaderData(const VdbBlastStdaaReader *self,
    uint32_t *status,
    uint64_t pig,
    size_t *length)
{   return _NotImplementedP(__func__); }


/******************************************************************************/

static const char VDB_BLAST_REFERENCE_SET[] = "VdbBlastReferenceSet";

struct VdbBlastReferenceSet {
    KRefcount refcount;
    const VdbBlastRunSet *rs;
};


LIB_EXPORT VdbBlastReferenceSet* CC VdbBlastRunSetMakeReferenceSet(
    const VdbBlastRunSet *self,
    VdbBlastStatus *status)
{
    VdbBlastReferenceSet *p = calloc(1, sizeof *p);

    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (p == NULL) {
        *status = eVdbBlastMemErr;
    }
    else {
        KRefcountInit(&p->refcount, 1, VDB_BLAST_REFERENCE_SET,
            __func__, "referenceSet");
        *status = eVdbBlastNoErr;
    }

    p->rs = VdbBlastRunSetAddRef((VdbBlastRunSet*)self);

    return p;
}


static
void _VdbBlastReferenceSetWhack(VdbBlastReferenceSet *self)
{
    if (self == NULL) {
        return;
    }

    STSMSG(1, ("Deleting VdbBlastReferenceSet"));

    VdbBlastRunSetRelease((VdbBlastRunSet*)self->rs);

    memset(self, 0, sizeof *self);

    free(self);
}


LIB_EXPORT VdbBlastReferenceSet* CC VdbBlastReferenceSetAddRef
    (VdbBlastReferenceSet *self)
{
    if (self == NULL) {
        STSMSG(1, ("VdbBlastReferenceSetAddRef(NULL)"));
        return self;
    }

    if (KRefcountAdd(&self->refcount, VDB_BLAST_REFERENCE_SET)
        == krefOkay)
    {
        STSMSG(1, ("VdbBlastReferenceSetAddRef"));
        return self;
    }

    STSMSG(1, ("Error: failed to VdbBlastReferenceSetAddRef"));
    return NULL;
}


LIB_EXPORT
void CC VdbBlastReferenceSetRelease(VdbBlastReferenceSet *self)
{
    if (self == NULL) {
        return;
    }

    STSMSG(1, ("VdbBlastReferenceSetRelease"));
    if (KRefcountDrop(&self->refcount, VDB_BLAST_REFERENCE_SET) != krefWhack) {
        return;
    }

    _VdbBlastReferenceSetWhack(self);
}


LIB_EXPORT VdbBlast2naReader* CC VdbBlastReferenceSetMake2naReader(
    const VdbBlastReferenceSet *self,
    VdbBlastStatus *status,
    uint64_t initial_read_id)
{
    return VdbBlastRunSetMake2naReaderExt
        (self->rs, status, initial_read_id, VDB_READ_REFERENCE);
}


LIB_EXPORT VdbBlast4naReader* CC VdbBlastReferenceSetMake4naReader(
    const VdbBlastReferenceSet *self,
    VdbBlastStatus *status)
{
    return VdbBlastRunSetMake4naReaderExt(self->rs, status, VDB_READ_REFERENCE);
}


static const struct References* _VdbBlastReferenceSetCheckReferences(

    const VdbBlastReferenceSet *self, VdbBlastStatus *status)
{
    assert(status);

    if (self == NULL || self->rs == NULL ||
        self->rs->core2naRef.reader.refs == NULL)
    {
        *status = eVdbBlastErr;
        return NULL;
    }

    _VdbBlastRunSetBeingRead(self->rs);

    *status = eVdbBlastNoErr;
    return self->rs->core2naRef.reader.refs;
}


static const struct References* _VdbBlastReferenceSetInitReferences(
    const VdbBlastReferenceSet *self,
    VdbBlastStatus *status)
{
    VdbBlastRunSet *rs = NULL;
    Reader2na *reader = NULL;

    assert(status);

    if (self == NULL || self->rs == NULL) {
        *status = eVdbBlastErr;
        return NULL;
    }

    rs = (VdbBlastRunSet*)self->rs;

    _VdbBlastRunSetBeingRead(rs);

    reader = &rs->core2naRef.reader;

    *status = eVdbBlastNoErr;

    if (reader->refs == NULL) {
        rc_t rc = KLockAcquire(rs->core2naRef.mutex);
        if (rc != 0) {
            *status = eVdbBlastErr;
            return NULL;
        }

        if (reader->refs == NULL) {
            reader->refs = _RunSetMakeReferences(&rs->runs, status);
        }

        KLockUnlock(rs->core2naRef.mutex);
    }

    return reader->refs;
}


LIB_EXPORT uint64_t CC VdbBlastReferenceSetGetNumSequences(
    const VdbBlastReferenceSet *self,
    VdbBlastStatus *status)
{
    const struct References *refs = NULL;

    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL) {
        status = &dummy;
    }

    refs = _VdbBlastReferenceSetInitReferences(self, status);
    if (*status != eVdbBlastNoErr) {
        return 0;
    }

    assert                           (refs);
    return _ReferencesGetNumSequences(refs, status);
}


LIB_EXPORT uint64_t CC VdbBlastReferenceSetGetTotalLength(
    const VdbBlastReferenceSet *self,
    VdbBlastStatus *status)
{
    const struct References *refs = NULL;

    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL) {
        status = &dummy;
    }

    refs = _VdbBlastReferenceSetInitReferences(self, status);
    if (*status != eVdbBlastNoErr) {
        return 0;
    }

    assert                          (refs);
    return _ReferencesGetTotalLength(refs, status);
}


LIB_EXPORT size_t CC VdbBlastReferenceSetGetReadName(
    const VdbBlastReferenceSet *self,
    uint64_t read_id,
    char *name_buffer,
    size_t bsize)
{
    if (bsize > 0 && name_buffer != NULL) {
        name_buffer[0] = '\0';
    }

    {
        VdbBlastStatus status = eVdbBlastErr;

        const struct References *refs
            = _VdbBlastReferenceSetCheckReferences(self, &status);
        if (status != eVdbBlastNoErr) {
            return 0;
        }

        assert                       (refs);
        return _ReferencesGetReadName(refs, read_id, name_buffer, bsize);
    }
}


LIB_EXPORT VdbBlastStatus CC VdbBlastReferenceSetGetReadId(
    const VdbBlastReferenceSet *self,
    const char *name_buffer,
    size_t bsize,
    uint64_t *read_id)
{
    VdbBlastStatus status = eVdbBlastErr;

    const struct References *refs
        = _VdbBlastReferenceSetCheckReferences(self, &status);
    if (status != eVdbBlastNoErr) {
        return status;
    }

    assert                     (refs);
    return _ReferencesGetReadId(refs, name_buffer, bsize, read_id);
}


LIB_EXPORT uint64_t CC VdbBlastReferenceSetGetReadLength(
    const VdbBlastReferenceSet *self,
    uint64_t read_id,
    VdbBlastStatus *status)
{
    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    *status = eVdbBlastErr;

    {
        const struct References *refs
            = _VdbBlastReferenceSetCheckReferences(self, status);
        if (*status != eVdbBlastNoErr) {
            return 0;
        }

        assert                         (refs);
        return _ReferencesGetReadLength(refs, read_id, status);
    }
}


/* EOF */
