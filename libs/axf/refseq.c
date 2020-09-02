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
        fillNs(self->Ns, dst, &full);
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

static unsigned readCircular(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    return getBases_4na(self, dst, start, len);
}

static unsigned readNormal(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    unsigned const length = self->length;
    unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;
    if (actlen > 0)
        getBases_2na(self, dst, start, actlen);
    return actlen;
}

char const *RefSeq_Scheme(void) {
    return "NCBI:refseq:tbl:reference";
}

unsigned RefSeq_getBases(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    return self->reader(self, dst, start, len);
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

#define USE_ASYNC_LOADING 1
#if USE_ASYNC_LOADING
#include <kproc/thread.h>
#include <kproc/queue.h>
#include <kproc/lock.h>
#include <kproc/sem.h>

struct RefSeqAsyncLoadInfo {
    KThread *th;
    KQueue *queue;
    VCursor const *curs;
    semaphore const *sema;
    RowRange rowRange;
    CursorAddResult info[2];
    unsigned currentBaseCount;
    unsigned hits;
    unsigned miss;
};
#else
typedef struct KQueue KQueue;
#endif

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

static rc_t load_1(  uint8_t *bases
                   , RangeList *Ns
                   , VCursor const *const curs
                   , RowRange const *const rowRange
                   , CursorAddResult const *const seqLenInfo
                   , CursorAddResult const *const readInfo
                   , KQueue *queue
                   )
{
    int accum = 0;
    int n = 0;
    unsigned j = 0; ///< current index in bases
    uint64_t i;
    unsigned position = 0;
    rc_t rc = 0;
    uint64_t const reportFreq = rowRange->count < 1024 ? 0 : (rowRange->count / 1024);
    uint64_t nextReport = (reportFreq == 0 || queue == NULL) ? rowRange->count : reportFreq;

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
                bases[j++] = accum;
                accum = 0;
                n = 0;
            }
            if (isN)
                extendRangeList(Ns, position);
            ++position;
        }
        if (i >= nextReport) {
#if USE_ASYNC_LOADING
            rc = KQueuePush(queue, (void const *)(uintptr_t)position, NULL);
            if (rc)
                return rc;
#endif
            nextReport += reportFreq;
        }
    }
    if (n != 0) {
        while (n < 4) {
            accum = accum << 2;
            ++n;
        }
        bases[j++] = accum;
    }
    return 0;
}

#if USE_ASYNC_LOADING

static rc_t asyncLoad(RefSeq *temp)
{
    uint8_t *bases = temp->bases;
    RangeList *Ns = temp->Ns;
    RefSeqAsyncLoadInfo const *const info = temp->asyncLoader;
    CursorAddResult const *const seqLenInfo = &info->info[0];
    CursorAddResult const *const readInfo = &info->info[1];
    rc_t rc = 0;

    free(temp);
    rc = load_1(bases, Ns, info->curs, &info->rowRange, seqLenInfo, readInfo, info->queue);
    KQueueSeal(info->queue);

    KLockAcquire(info->sema->lock);
    KSemaphoreSignal(info->sema->sema);
    KLockUnlock(info->sema->lock);

    return rc;
}

static rc_t runAsyncLoad(KThread const *th, void *data)
{
    rc_t rc;
    LOGMSG(klogDebug, "Starting async load of reference");
    rc = asyncLoad(data);
    LOGMSG(klogDebug, "Starting async load of reference");
    if (rc)
        LOGERR(klogDebug, rc, "async load of reference failed");
    return rc;
}

static unsigned asyncReadNormal(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len)
{
    RefSeqAsyncLoadInfo *const info = self->asyncLoader;
    unsigned const length = self->length;
    unsigned const actlen = (start + len) < length ? len : start < length ? length - start : 0;

    assert(info != NULL);
    if (KQueueSealed(info->queue)) {
        rc_t rc;

    QUEUE_IS_SEALED:
        ((RefSeq *)self)->reader = readNormal;
        ((RefSeq *)self)->asyncLoader = NULL;

        rc = 0;
        KThreadWait(info->th, &rc);
        KThreadRelease(info->th);

        KQueueRelease(info->queue);
        VCursorRelease(info->curs); /* undo addref in makeAsyncLoadInfo */

        free(info);

        if (rc) {
            LOGERR(klogErr, rc, "async load of reference failed");
            return 0;
        }
        PLOGMSG(klogDebug, (klogDebug
                            , "Async load of reference is complete (stall rate: $(p)%)"
                            , "p=% 5.1f", info->miss * 100.0 / info->hits)
                );
        return readNormal(self, dst, start, len);
    }

    if (actlen == 0)
        return 0;

    ++info->hits;
    ++info->miss;
    for ( ; ; ) {
        if (start + actlen <= info->currentBaseCount) {
            --info->miss;
            getBases_2na(self, dst, start, actlen);
            return actlen;
        }
        else {
            void *popped = NULL;
            rc_t const rc = KQueuePop(info->queue, &popped, NULL);

            ++info->miss;
            if (rc == 0)
                info->currentBaseCount = (uintptr_t)popped;
            else if ((int)GetRCObject(rc) == rcData && (int)GetRCState(rc) == rcDone)
                goto QUEUE_IS_SEALED;
            else {
                LOGERR(klogErr, rc, "async load of reference failed, the queue is dead");
                return 0;
            }
        }
    }
}

static rc_t makeAsyncLoadInfo(  RefSeqAsyncLoadInfo **result
                              , VCursor const *const curs
                              , RowRange const *const rowRange
                              , CursorAddResult const *const seqLenInfo
                              , CursorAddResult const *const readInfo
                              , semaphore *sema)
{
    *result = calloc(1, sizeof(**result));
    if (*result) {
        VCursorAddRef(curs);
        (**result).curs = curs;
        (**result).rowRange = *rowRange;
        (**result).info[0] = *seqLenInfo;
        (**result).info[1] = *readInfo;
        (**result).sema = sema;
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
#endif

static rc_t loadCircular(  RefSeq *result
                         , VCursor const *const curs
                         , RowRange const *const rowRange
                         , CursorAddResult const *const seqLenInfo
                         , CursorAddResult const *const readInfo
                         , unsigned const baseCount
                         , semaphore *sema)
{
    unsigned const allocated = (baseCount + 1) / 2;
    uint8_t *bases = malloc(allocated);
    rc_t rc = 0;

    if (bases == NULL)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    rc = loadCircular_1(bases, curs, rowRange, seqLenInfo, readInfo);
    if (rc == 0) {
        result->bases = bases;
        result->length = baseCount;
        result->reader = readCircular;
    }
    else {
        free(bases);
    }
    return rc;
}

static rc_t load(  RefSeq *result
                 , VCursor const *const curs
                 , RowRange const *const rowRange
                 , CursorAddResult const *const seqLenInfo
                 , CursorAddResult const *const readInfo
                 , unsigned const baseCount
                 , semaphore *sema)
{
    unsigned const allocated = (baseCount + 3) / 4;
    uint8_t *bases = malloc(allocated);
    rc_t rc = 0;

    if (bases == NULL)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    result->bases = bases;
    result->length = baseCount;
    result->Ns = calloc(1, sizeof(*result->Ns));
    if (result->Ns == NULL)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

#if USE_ASYNC_LOADING
    if (rowRange->count >= 20000) {
        /* this includes only about 7% of RefSeq objects but over 55% of the data */
        RefSeqAsyncLoadInfo *info = 0;
        rc = makeAsyncLoadInfo(&info, curs, rowRange, seqLenInfo, readInfo, sema);
        if (rc == 0) {
            rc = KQueueMake(&info->queue, 1024);
            if (rc == 0) {
                RefSeq *temp = NULL;

                result->asyncLoader = info;
                result->reader = asyncReadNormal;

                temp = malloc(sizeof(*temp));
                if (temp) {
                    memcpy(temp, result, sizeof(*result));

                    KLockAcquire(sema->lock);
                    KSemaphoreWait(sema->sema, sema->lock);
                    KLockUnlock(sema->lock);

                    rc = KThreadMake(&info->th, runAsyncLoad, temp);
                    if (rc == 0) {
                        LOGMSG(klogDebug, "Requesting async load of reference");
                        return 0;
                    }
                    free(temp);

                    KLockAcquire(sema->lock);
                    KSemaphoreSignal(sema->sema);
                    KLockUnlock(sema->lock);
                }
                else
                    rc = RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
                KQueueRelease(info->queue);
            }
            VCursorRelease(curs); /* undo addref in makeAsyncLoadInfo */
            free(info);
        }
        LOGERR(klogErr, rc, "Failed to start async load of reference, trying synchronous load");
    }
    /* fall through to synchronous loading */
#endif

    LOGMSG(klogDebug, "Synchronous load of reference");
    rc = load_1(bases, result->Ns, curs, rowRange, seqLenInfo, readInfo, NULL);
    if (rc == 0)
        result->reader = readNormal;
    else
        RefSeqFree(result);
    return rc;
}

rc_t RefSeq_load(RefSeq *result, VTable const *const tbl, semaphore *sema)
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

            rc = (circular ? loadCircular : load)(result, curs, &rowRange, &cols[1], &cols[2], baseCount, sema);
        }
    }
    VCursorRelease(curs);
    return rc;
}

void RefSeqFree(RefSeq *self)
{
    RangeListFree(self->Ns);
    free(self->Ns);
    free(self->bases);
#if USE_ASYNC_LOADING
    if (self->asyncLoader) {
        rc_t rc2 = 0;
        KQueueSeal(self->asyncLoader->queue);
        KThreadWait(self->asyncLoader->th, &rc2);
        KQueueRelease(self->asyncLoader->queue);
        KThreadRelease(self->asyncLoader->th);
        free(self->asyncLoader);
    }
#endif
    memset(self, 0, sizeof(*self));
}

static int name_cmp(char const *name, unsigned const qlen, char const *qry)
{
    unsigned i;
    for (i = 0; i < qlen; ++i) {
        int const a = name[i];
        int const b = qry[i];
        int const d = a - b;
        if (a == 0) return d;
        if (d == 0) continue;
        return d;
    }
    return name[qlen] - '\0';
}

bool RefSeq_Find(RefSeqList *list, unsigned *at, unsigned const qlen, char const *qry)
{
    unsigned f = 0;
    unsigned e = list->entries;

    while (f < e) {
        unsigned const m = f + (e - f) / 2;
        int const d = name_cmp(list->entry[m].name, qlen, qry);
        if (d == 0) {
            *at = m;
            return true;
        }
        if (d < 0)
            f = m + 1;
        else
            e = m;
    }
    *at = f; // it could be inserted here
    return false;
}

rc_t RefSeq_Insert(RefSeqList *list, unsigned at, unsigned const qlen, char const *qry, RefSeq *value)
{
    RefSeqListEntry temp;

    temp.name = malloc(qlen + 1);
    if (temp.name == NULL) {
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
    }
    memmove(temp.name, qry, qlen);
    temp.name[qlen] = '\0';
    temp.object = *value;

    if (list->entries >= list->allocated) {
        unsigned const new_alloc = list->allocated == 0 ? 16 : (list->allocated * 2);
        void *tmp = realloc(list->entry, new_alloc * sizeof(*list->entry));
        if (tmp == NULL) {
            free(temp.name);
            return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
        }
        list->entry = tmp;
        list->allocated = new_alloc;
    }
    memmove(&list->entry[at + 1], &list->entry[at], sizeof(*list->entry) * (list->entries - at));
    ++list->entries;
    list->entry[at] = temp;

    return 0;
}

void RefSeqListFree(RefSeqList *list)
{
    unsigned i;
    for (i = 0; i != list->entries; ++i) {
        RefSeqFree(&list->entry[i].object);
        free(list->entry[i].name);
    }
    free(list->entry);
#if USE_ASYNC_LOADING
    if (list->sema.lock) {
        semaphore *const sema = &list->sema;
        LOGMSG(klogDebug, "Destructing synchronization for async load of reference");
        KLockAcquire(sema->lock);
        KSemaphoreCancel(sema->sema);
        KLockUnlock(sema->lock);
        KLockRelease(sema->lock);
        KSemaphoreRelease(sema->sema);
    }
#endif
}

rc_t RefSeqListInit(RefSeqList *list)
{
    rc_t rc = 0;
#if USE_ASYNC_LOADING
    semaphore *const sema = &list->sema;

    LOGMSG(klogDebug, "Creating synchronization for async load of reference");
    rc = KLockMake(&sema->lock);
    if (rc == 0)
        rc = KSemaphoreMake(&sema->sema, 2);
#endif
    return rc;
}
