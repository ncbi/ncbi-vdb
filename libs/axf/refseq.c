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
#include <klib/log.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <kproc/lock.h>
#include <kproc/thread.h>
#include <atomic.h>
#include "refseq.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

typedef RefSeqList List;
typedef RefSeqListEntry Entry;
typedef RefSeq Object;

#include "list.c"
#include "util.h"

struct RefSeqSyncLoadInfo {
    KThread *th;
    KLock *mutex;               /**< mostly guards the cursor against concurrent use */
    VCursor const *curs;        /**< can be used by either thread after acquiring the mutex */
    RowRange rr;                /**< of the table */
    CursorAddResult car[2];     /**< column name and id */
    atomic64_t loaded;          /**< rows less than this have been loaded already */
    atomic64_t count;           /**< number of rows left to load, will cause bg thread to exit if set = 0 */
    atomic64_t sync;
    unsigned max_seq_len;       /**< max length of any READ in the table */
    unsigned hits;              /**< statistics to give some idea of ... */
    unsigned miss;              /**< ... how effective the bg thread was */
};

static rc_t RefSeqSyncLoadInfoFree(RefSeqSyncLoadInfo *const self)
{
    rc_t rc = 0;
    if (self) {
        KLockAcquire(self->mutex);
        atomic64_set(&self->count, 0);
        KLockUnlock(self->mutex);
        KThreadWait(self->th, &rc);
    }
    if (rc)
        LOGERR(klogErr, rc, "async loader thread failed");
    return rc;
}

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

struct FillNsData {
    uint8_t *dst;
    Range full;
};
static void fillNs(void *vp, Range const *intersectingRange)
{
    struct FillNsData const *data = vp;
    unsigned const offset = intersectingRange->start - data->full.start;
    memset(data->dst + offset, 15, intersectingRange->end - intersectingRange->start);
}

static void getBases_2na(uint8_t *const dst, unsigned const start, unsigned const len, uint8_t const *bases, RangeList const *Ns)
{
    unsigned pos = start;
    unsigned i = 0;

    if (pos % 4 != 0) {
        unsigned const n = partial_unpack_2na(bases, dst, i, len, pos);
        i += n; pos += n;
    }
    while ((i + 4) <= len) {
        unpack_2na(bases, dst + i, pos);
        i += 4;
        pos += 4;
    }
    if (i < len) {
        unsigned const n = partial_unpack_2na(bases, dst, i, len, pos);
        i += n; pos += n;
    }
    assert(i == len);
    assert(start + len == pos);

    // 2na will have 'A's in place of 'N's, put the 'N's back
    {
        struct FillNsData data;
        data.full.start = start;
        data.full.end = start + len;
        data.dst = dst;
        withIntersectRangeList(Ns, &data.full, fillNs, &data);
    }
}

static unsigned getBases_4na(Object const *self, uint8_t *const dst, unsigned const start, unsigned const len)
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

static unsigned readCircular(Object *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    return getBases_4na(self, dst, start, len);
}

static unsigned readNormal(Object *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned const length = self->length;
    unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;
    if (actlen > 0)
        getBases_2na(dst, start, actlen, self->bases, &self->Ns);
    return actlen;
}

/* read one row from the cursor and populate the bases array and Ns structure */
static rc_t read1Row(Object *self, int64_t row)
{
    RefSeqSyncLoadInfo *const info = self->info;
    VCursor const *const curs = info->curs;
    CursorAddResult *const seqLenInfo = &info->car[0];
    CursorAddResult *const readInfo = &info->car[1];
    unsigned position = (unsigned)((row - info->rr.first) * info->max_seq_len);
    int accum = 0;
    int n = 0;
    rc_t rc = 0;
    uint32_t const seqLen = readU32(seqLenInfo, row, curs, &rc);
    uint32_t ri; ///< index within current row
    ReadStringResult read;
    unsigned j = position / 4;

    assert(position % 4 == 0);
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
        accum = (accum << 2) | base;
        ++n;
        if (n == 4) {
            self->bases[j++] = accum;
            accum = 0;
            n = 0;
        }
        if (isN) {
            if (NULL == extendRangeList(&self->Ns, position))
                return RC(rcXF, rcFunction, rcReading, rcMemory, rcExhausted);
        }
        ++position;
    }
    if (n) {
        while (n < 4) {
            accum <<= 2;
            ++n;
        }
        self->bases[j++] = accum;
    }
    assert(0 != checkRangeList(&self->Ns));
    return 0;
}

static void rowWasLoaded(RefSeqSyncLoadInfo *info, int64_t row)
{
    assert(atomic64_read(&info->count) > 0);

    atomic64_set(&info->loaded, row);
    atomic64_dec(&info->count);
}

static bool rowIsLoaded(RefSeqSyncLoadInfo const *info, int64_t row)
{
    /* the lock is NOT held during this function */
    return row < atomic64_read(&info->loaded);
}

static int64_t positionToRow(RefSeqSyncLoadInfo const *info, unsigned const position)
{
    assert(info != NULL);
    return info->rr.first + (position / info->max_seq_len);
}

static unsigned rowToPosition(RefSeqSyncLoadInfo const *info, int64_t const row)
{
    assert(info != NULL);
    return (unsigned)((row - info->rr.first) * info->max_seq_len);
}

static unsigned readNormalIncomplete_1(Object *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned const length = self->length;
    unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;
    if (actlen > 0) {
        RefSeqSyncLoadInfo *info = self->info;
        int64_t const first = positionToRow(info, start);
        int64_t const last = positionToRow(info, start + actlen - 1);
        size_t const max_bases = ((last + 1) - first) * info->max_seq_len;
        uint8_t *const buffer = (max_bases <= len && start == rowToPosition(info, first)) ? dst : malloc(max_bases);
        uint8_t *buf = buffer;
        int64_t row;
        rc_t rc = 0;

        if (buffer == NULL) {
            LOGERR(klogFatal, RC(rcXF, rcFunction, rcReading, rcMemory, rcExhausted), "Error reading reference");
            return 0;
        }
        for (row = first; row <= last && rc == 0; ++row) {
            ++info->hits;
            ++info->miss;
            if (rowIsLoaded(info, row)) {
                --info->miss;
                getBases_2na(buf, rowToPosition(info, row), info->max_seq_len, self->bases, &self->Ns);
            }
            else {
                ReadStringResult read;

                memset(buf, 15, info->max_seq_len);
                KLockAcquire(info->mutex);
                if (readString(&read, &info->car[1], row, info->curs, &rc) != NULL) {
                    memmove(buf, read.value, read.length);
                }
                KLockUnlock(info->mutex);
                {
                    unsigned i;
                    for (i = 0; i < read.length; ++i) {
                        switch (buf[i]) {
                        case 1:
                        case 2:
                        case 4:
                        case 8:
                            break;
                        default:
                            buf[i] = 15;
                        }
                    }
                }
            }
            buf += info->max_seq_len;
        }
        if (buffer != dst) {
            unsigned const offset = start - rowToPosition(info, first);
            memmove(dst, buffer + offset, actlen);
            free(buffer);
        }
        if (rc) {
            LOGERR(klogErr, rc, "Error reading reference");
            return 0;
        }
    }
    return actlen;
}

/* this is called on the main thread */
static unsigned readNormalIncomplete(Object *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    if (self->info == NULL || (atomic64_read_and_add_even(&self->info->sync, 2) & 1) != 0)
        return readNormal(self, dst, start, len);
    {
        unsigned const actlen = readNormalIncomplete_1(self, dst, start, len);
        atomic64_add(&self->info->sync, -2);
        return actlen;
    }
}

/* this is called on the background thread */
static rc_t runLoadThread(Object *self)
{
    RefSeqSyncLoadInfo *const info = self->info;
    uint64_t const count = info->rr.count;
    int64_t const first = info->rr.first;
    uint64_t i;

    LOGMSG(klogDebug, "Starting background loading of reference");
    for (i = 0; i < count; ++i) {
        bool done = false;
        int64_t const row = i + first;
        rc_t rc = 0;

        KLockAcquire(info->mutex);
        if (atomic64_read(&info->count) != 0) {
            rc = read1Row(self, row);
            if (rc == 0)
                rowWasLoaded(info, row);
        }
        else
            done = true;
        KLockUnlock(info->mutex);
        if (rc) return rc;
        if (done) break;
    }
    LOGMSG(klogDebug, "Done background loading of reference");

    self->reader = readNormal;
    self->info = NULL;

    atomic64_inc(&info->sync); /* tell readers to bail out */
    while (atomic_read(&info->sync) != 1) /* wait for readers to finish */
        ;
    {
        double const pct = (100.0 * (info->hits - info->miss)) / info->hits;

        KLockRelease(info->mutex);
        VCursorRelease(info->curs);
        free(info);

        PLOGMSG(klogDebug, (klogDebug, "Done with background loading of reference; preload was $(pct)%", "pct=%5.1f", (float)pct));
    }
    return 0;
}

char const *RefSeq_Scheme(void) {
    return "NCBI:refseq:tbl:reference";
}

unsigned RefSeq_getBases(Object const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    return self->reader((Object *)self, dst, start, len);
}

static rc_t loadCircular_1(  uint8_t *result
                           , VCursor const *const curs
                           , RowRange const *const rowRange
                           , CursorAddResult const *const seqLenInfo
                           , CursorAddResult const *const readInfo)
{
    int accum = 0;
    int n = 0;
    unsigned j = 0; ///< current index in bases
    uint64_t i;
    rc_t rc = 0;

    for (i = 0; i < rowRange->count; ++i) {
        int64_t const row = rowRange->first + i;
        uint32_t const seqLen = readU32(seqLenInfo, row, curs, &rc);
        uint32_t ri; ///< index within current row
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
                result[j++] = accum;
                accum = 0;
                n = 0;
            }
        }
    }
    if (n != 0) {
        accum = accum << 4;
        result[j++] = accum;
    }
    return 0;
}

static rc_t loadCircular(  Object *result
                         , VCursor const *const curs
                         , RowRange const *const rowRange
                         , CursorAddResult const *const info
                         )
{
    rc_t rc = 0;
    uint64_t const baseCount = readU64(&info[0], rowRange->first, curs, &rc);
    assert(baseCount < UINT_MAX);
    if (rc == 0) {
        unsigned const allocated = (unsigned)((baseCount + 1) / 2);
        uint8_t *bases = malloc(allocated);

        if (bases == NULL)
            return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

        rc = loadCircular_1(bases, curs, rowRange, &info[1], &info[2]);
        if (rc == 0) {
            result->bases = bases;
            result->length = (unsigned)baseCount;
            result->reader = readCircular;
        }
        else {
            free(bases);
        }
    }
    return rc;
}

static RefSeqSyncLoadInfo *RefSeqSyncLoadInfoMake(  VCursor const *curs
                                                  , RowRange const *rr
                                                  , CursorAddResult const *car
                                                  , rc_t *prc)
{
    RefSeqSyncLoadInfo *result = calloc(1, sizeof(*result));
    if (result == NULL) {
        LOGERR(klogFatal, RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted), "OUT OF MEMORY!!!");
        abort();
    }
    *prc = KLockMake(&result->mutex);
    if (*prc == 0) {
        result->max_seq_len = readU32(&car[2], rr->first, curs, prc);
        assert(result->max_seq_len % 4 == 0);
        if (*prc == 0) {
            result->curs = curs;
            VCursorAddRef(curs);
            result->rr = *rr;
            atomic64_set(&result->count, rr->count);
            result->car[0] = car[0];
            result->car[1] = car[1];
            return result;
        }
        KLockRelease(result->mutex);
    }
    free(result);
    return NULL;
}

static rc_t run_load_thread(const KThread *self, void *data)
{
    return runLoadThread(data);
}

static rc_t load(  Object *result
                 , VCursor const *const curs
                 , RowRange const *const rowRange
                 , CursorAddResult const *const info
                 )
{
    rc_t rc = 0;
    uint64_t const baseCount = readU64(&info[0], rowRange->first, curs, &rc);
    assert(baseCount < UINT_MAX);
    if (rc == 0) {
        unsigned const allocated = (unsigned)((baseCount + 3) / 4);
        uint8_t *bases = malloc(allocated);

        if (bases == NULL)
            return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

        result->bases = bases;
        result->length = (unsigned)baseCount;
        result->info = RefSeqSyncLoadInfoMake(curs, rowRange, info + 1, &rc);
        if (rc == 0) {
            rc = KThreadMake(&result->info->th, run_load_thread, result);
            if (rc == 0) {
                result->reader = readNormalIncomplete;
                return 0;
            }
        }
        RefSeqFree(result);
    }
    return rc;
}

static rc_t init(Object *result, VTable const *const tbl)
{
    CursorAddResult cols[5];
    RowRange rowRange;
    rc_t rc = 0;
    VCursor const *const curs = createCursor(tbl, &rc);

    memset(result, 0, sizeof(*result));
    if (curs == NULL) return rc;

    if (!addColumn(&cols[0], "CIRCULAR", curs, &rc)) return rc;
    if (!addColumn(&cols[1], "TOTAL_SEQ_LEN", curs, &rc)) return rc;
    if (!addColumn(&cols[2], "SEQ_LEN", curs, &rc)) return rc;
    if (!addColumn(&cols[3], "(INSDC:4na:bin)READ", curs, &rc)) return rc;
    if (!addColumn(&cols[4], "(U32)MAX_SEQ_LEN", curs, &rc)) return rc;

    rc = VCursorOpen(curs);
    assert(rc == 0);
    if (rc == 0) {
        if (getRowRange(&rowRange, curs, &rc) != NULL) {
            bool const circular = readBool(&cols[0], rowRange.first, curs, &rc);

            assert(rowRange.count < UINT_MAX);
            rc = (circular ? loadCircular : load)(result, curs, &rowRange, &cols[1]);
        }
    }
    VCursorRelease(curs);
    return rc;
}

void RefSeqFree(Object *self)
{
    RefSeqSyncLoadInfoFree(self->info);
    RangeListFree(&self->Ns);
    free(self->bases);
    memset(self, 0, sizeof(*self));
}

Entry *RefSeqFind(List *list, unsigned const qlen, char const *qry)
{
    unsigned at = 0;
    return find(list, &at, qlen, qry) ? &list->entry[at] : NULL;
}

Entry *RefSeqInsert(List *list, unsigned const qlen, char const *qry, VTable const *tbl, rc_t *prc)
{
    Entry *result = NULL;
    unsigned at = 0;
    if (find(list, &at, qlen, qry)) {
        assert(!"entry exists!!!");
        abort();
    }

    result = insert(list, at, qlen, qry);
    if (result == NULL) {
        LOGERR(klogFatal, (*prc = RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted)), "");
        return NULL;
    }

    *prc = init(&result->object, tbl);
    if (*prc == 0)
        return result;

    undo_insert(list, at);
    return NULL;
}

void RefSeqListFree(List *list)
{
    unsigned i;
    for (i = 0; i != list->entries; ++i) {
        RefSeqFree(&list->entry[i].object);
        free(list->entry[i].name);
    }
    free(list->entry);
}

rc_t RefSeqListInit(List *list)
{
    rc_t rc = 0;
    return rc;
}
