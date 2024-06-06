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
#include <klib/refcount.h>
#include "refseq.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

typedef RefSeqList List;
typedef RefSeqListEntry Entry;
typedef RefSeq Object;

#include "list.c"
#include "util.h"

struct RefSeqAsyncLoadInfo {
    KRefcount refcount;
    KThread *th;
    KLock *mutex;               /**< mostly guards the cursor against concurrent use */
    VCursor const *curs;        /**< can be used by either thread after acquiring the mutex */
    RowRange rr;                /**< of the table */
    CursorAddResult car[2];     /**< column name and id */
    int64_t volatile loaded;    /**< rows less than this have been loaded already */
    unsigned volatile count;    /**< number of rows left to load, will cause bg thread to exit if set = 0 */
    unsigned max_seq_len;       /**< max length of any READ in the table */
    unsigned volatile hits;     /**< statistics to give some idea of ... */
    unsigned volatile miss;     /**< ... how effective the bg thread was */
};

static void RefSeqAsyncLoadInfo_Release(RefSeqAsyncLoadInfo *const self)
{
    switch (KRefcountDrop(&self->refcount, "RefSeqAsyncLoadInfo")) {
    case krefWhack:
        break;
    case krefOkay:
        return;
    default:
        assert(!"valid refcount");
        abort();
    }
    VCursorRelease(self->curs);
    KLockRelease(self->mutex);
    KThreadRelease(self->th);
    free(self);
}

static rc_t RefSeqAsyncLoadInfoFree(RefSeqAsyncLoadInfo *const self)
{
    rc_t rc = 0;
    if (self) {
        /* Synchronize with background thread in preparation for clean up */
        KRefcountAdd(&self->refcount, "RefSeqAsyncLoadInfo"); // keep alive; let die at line 89
        LOGMSG(klogDebug, "Foreground thread ending background thread");
        KLockAcquire(self->mutex);
        self->count = 0;
        KLockUnlock(self->mutex);
        KThreadWait(self->th, &rc);
        LOGERR(klogDebug, rc, "Background thread ended");
        RefSeqAsyncLoadInfo_Release(self);
        if (rc)
            LOGERR(klogErr, rc, "asynchronous loader thread failed");
    }
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
        uint8_t const b4na_2 = bases[j >> 1];
        uint8_t const b4na2 = b4na_2 & 0x0F;
        dst[i++] = b4na2;
        j = (j + 1) % length;
    }
    while ((i + 2) <= len) {
        uint8_t const b4na_2 = bases[j >> 1];
        uint8_t const b4na1 = b4na_2 >> 4;
        uint8_t const b4na2 = b4na_2 & 0x0F;
        dst[i++] = b4na1;
        dst[i++] = b4na2;
        j = (j + 2) % length;
    }
    if (i < len) {
        uint8_t const b4na_2 = bases[j >> 1];
        uint8_t const b4na1 = b4na_2 >> 4;
        uint8_t const b4na2 = b4na_2 & 0x0F;
        dst[i++] = (j % 2) == 0 ? b4na1 : b4na2;
    }
    assert(i == len);
    return i;
}

static unsigned readCircular(Object const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    return getBases_4na(self, dst, start, len);
}

static unsigned readNormal(Object const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned const length = self->length;
    unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;
    if (actlen > 0)
        getBases_2na(dst, start, actlen, self->bases, &self->Ns);
    return actlen;
}

static unsigned readZero(Object const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    /* this should not be reachable; an rc != 0 should have propagated up the
     * call stack and ended the program before we could get here */
    assert(!"reachable");
    abort();
}

static bool rowIsLoaded(RefSeqAsyncLoadInfo const *async, int64_t row)
{
    /* the lock is NOT held during this function */
    return row < async->loaded;
}

static int64_t positionToRow(RefSeqAsyncLoadInfo const *async, unsigned const position)
{
    assert(async != NULL);
    return async->rr.first + (position / async->max_seq_len);
}

static unsigned rowToPosition(RefSeqAsyncLoadInfo const *async, int64_t const row)
{
    assert(async != NULL);
    return (unsigned)((row - async->rr.first) * async->max_seq_len);
}

/* this is called on the main thread */
static unsigned readNormalIncomplete(Object const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned const length = self->length;
    unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;
    if (actlen > 0) {
        RefSeqAsyncLoadInfo *async = self->async;
        int64_t const first = positionToRow(async, start);
        int64_t const last = positionToRow(async, start + actlen - 1);
        size_t const max_bases = ((last + 1) - first) * async->max_seq_len;
        uint8_t *const buffer = (max_bases <= len && start == rowToPosition(async, first)) ? dst : malloc(max_bases);
        uint8_t *buf = buffer;
        int64_t row;
        rc_t rc = 0;

        if (buffer == NULL) {
            LOGERR(klogFatal, RC(rcXF, rcFunction, rcReading, rcMemory, rcExhausted), "Error reading reference");
            return 0;
        }
        for (row = first; row <= last && rc == 0; ++row) {
            ++async->hits;
            if (rowIsLoaded(async, row)) {
                getBases_2na(buf, rowToPosition(async, row), async->max_seq_len, self->bases, &self->Ns);
            }
            else {
                ReadStringResult read = { NULL, 0 };

                memset(buf, 15, async->max_seq_len);
                KLockAcquire(async->mutex);
                ++async->miss;
                if (readString(&read, &async->car[1], row, async->curs, &rc) != NULL) {
                    memmove(buf, read.value, read.length);
                }
                KLockUnlock(async->mutex);
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
            buf += async->max_seq_len;
        }
        if (buffer != dst) {
            unsigned const offset = start - rowToPosition(async, first);
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

/* this is called on the background thread */
static rc_t runLoadThread(Object *self)
{
    RefSeqAsyncLoadInfo *const async = self->async;
    uint8_t *const buffer = malloc(async->max_seq_len);
    uint64_t const count = async->rr.count;
    int64_t const first = async->rr.first;
    VCursor const *const curs = async->curs;
    CursorAddResult *const seqLenInfo = &async->car[0];
    CursorAddResult *const readInfo = &async->car[1];
    ReadStringResult read = { NULL, 0 };
    int accum = 0;
    int n = 0;
    uint64_t i;
    bool done = false;
    rc_t rc = 0;
    unsigned j = 0;
    unsigned position = 0;

    if (buffer == NULL)
        rc = RC(rcXF, rcFunction, rcReading, rcMemory, rcExhausted);

    LOGMSG(klogDebug, "Starting background loading of reference");
    for (i = 0; i < count && !done && rc == 0; ++i) {
        int64_t const row = i + first;
        uint32_t seqLen = 0;

        KLockAcquire(async->mutex);
        {
            done = async->count == 0;
            async->loaded = row - 1;
            seqLen = readU32(seqLenInfo, row, curs, &rc);
            if (seqLen == 0 || NULL == readString(&read, readInfo, row, curs, &rc) || read.length > async->max_seq_len)
                ;
            else
                memmove(buffer, read.value, read.length);
            --async->count;
        }
        KLockUnlock(async->mutex);
        if (!done && rc == 0 && read.length <= async->max_seq_len && position + seqLen <= self->length) {
            uint32_t ri; ///< index within current row

            for (ri = 0; ri < read.length; ++ri) {
                int base = 0;
                int isN = 1;

                switch (buffer[ri]) {
                case 1: base = 0; isN = 0; break;
                case 2: base = 1; isN = 0; break;
                case 4: base = 2; isN = 0; break;
                case 8: base = 3; isN = 0; break;
                }
                accum = (accum << 2) | base;
                ++n;
                if (n == 4) {
                    self->bases[j++] = (uint8_t)accum;
                    accum = 0;
                    n = 0;
                }
                if (isN) {
                    if (NULL == extendRangeList(&self->Ns, position)) {
                        rc = RC(rcXF, rcFunction, rcReading, rcMemory, rcExhausted);
                        break;
                    }
                }
                ++position;
            }
            for ( ; ri < seqLen; ++ri) {
                accum = accum << 2;
                ++n;
                if (n == 4) {
                    self->bases[j++] = (uint8_t)accum;
                    accum = 0;
                    n = 0;
                }
                if (NULL == extendRangeList(&self->Ns, position)) {
                    rc = RC(rcXF, rcFunction, rcReading, rcMemory, rcExhausted);
                    break;
                }
                ++position;
            }
        }
        else if (!done && rc == 0)
            rc = RC(rcXF, rcFunction, rcReading, rcData, rcInvalid);
    }
    if (n != 0) {
        while (n < 4) {
            accum <<= 2;
            ++n;
        }
        self->bases[j++] = (uint8_t)accum;
    }
    free(buffer);
    LOGMSG(klogDebug, "Done background loading of reference");
    if (rc == 0 && i == count) {
        KLockAcquire(async->mutex);
        async->loaded = i + first; /* last row was loaded */
        async->count = 0;
        KLockUnlock(async->mutex);
    }

    assert((atomic_read(&self->rwl) & 1) == 0); /* there is only one writer */
    atomic_inc(&self->rwl); /* tell readers we want to update the state */
    while (atomic_read(&self->rwl) != 1)
        ;
    /* readers are all waiting in the loop at line 445 */
    self->reader = rc == 0 ? readNormal : readZero;
    self->async = NULL;
    atomic_dec(&self->rwl); /* state is updated; readers continue to line 448 */
    if (rc == 0 && i == count) {
        double const pct = 100.0 * (async->hits - async->miss) / async->hits;

        PLOGMSG(klogDebug, (klogDebug, "Done with background loading of reference; preload was $(pct)%", "pct=%5.1f", (float)pct));
    }
    RefSeqAsyncLoadInfo_Release(async);
    LOGERR(klogDebug, rc, "Background thread exiting");

    return rc;
}

char const *RefSeq_Scheme(void) {
    return "NCBI:refseq:tbl:reference";
}

unsigned RefSeq_getBases(Object const * self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    atomic_t *const rwl = &((Object *)self)->rwl;

    if (self->async == NULL) {
        /* this is the fast path and the most common for normal use */
        /* there is no background thread running */
        return self->reader(self, dst, start, len);
    }
    /* there is a background thread running */
    if ((atomic_read_and_add_even(rwl, 2) & 1) == 0) {
        /* but it is not trying to update the state */
        unsigned const actlen = self->reader(self, dst, start, len);
        atomic_add(rwl, -2);
        return actlen;
    }
    /* very unlikely, but likelihood increases with the number of readers */
    /* there is a background thread trying to update the state */
    while ((atomic_read(rwl) & 1) != 0)
        ;
    /* the state has been updated; use the new state */
    return RefSeq_getBases(self, dst, start, len);
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
                result[j++] = (uint8_t)accum;
                accum = 0;
                n = 0;
            }
        }
    }
    if (n != 0) {
        accum = accum << 4;
        result[j++] = (uint8_t)accum;
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

static RefSeqAsyncLoadInfo *RefSeqAsyncLoadInfoMake(  VCursor const *curs
                                                    , RowRange const *rr
                                                    , CursorAddResult const *car
                                                    , rc_t *prc)
{
    RefSeqAsyncLoadInfo *result = calloc(1, sizeof(*result));
    if (result == NULL) {
        LOGERR(klogFatal, RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted), "OUT OF MEMORY!!!");
        abort();
    }
    *prc = KLockMake(&result->mutex);
    if (*prc == 0) {
        result->max_seq_len = readU32(&car[2], rr->first, curs, prc);
        assert(result->max_seq_len % 4 == 0);
        if (*prc == 0) {
            KRefcountInit(&result->refcount, 1, "RefSeqAsyncLoadInfo", "init", "");
            result->curs = curs;
            VCursorAddRef(curs);
            result->rr = *rr;
            result->count = (unsigned)rr->count;
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
        result->async = RefSeqAsyncLoadInfoMake(curs, rowRange, info + 1, &rc);
        if (rc == 0) {
            rc = KThreadMake(&result->async->th, run_load_thread, result);
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
    RefSeqAsyncLoadInfoFree(self->async);
    RangeListFree(&self->Ns);
    free(self->bases);
    free(self);
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
        *prc = 0;
        return &list->entry[at];
    }

    result = insert(list, at, qlen, qry);
    if (result == NULL) {
        LOGERR(klogFatal, (*prc = RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted)), "");
        return NULL;
    }

    result->object = calloc(1, sizeof(*result->object));
    if (result == NULL) {
        LOGERR(klogFatal, (*prc = RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted)), "");
        return NULL;
    }

    *prc = init(result->object, tbl);
    if (*prc == 0)
        return result;

    undo_insert(list, at);
    return NULL;
}

void RefSeqListFree(List *list)
{
    unsigned i;
    for (i = 0; i != list->entries; ++i) {
        RefSeqFree(list->entry[i].object);
        free(list->entry[i].name);
    }
    free(list->entry);
}

rc_t RefSeqListInit(List *list)
{
    rc_t rc = 0;
    return rc;
}
