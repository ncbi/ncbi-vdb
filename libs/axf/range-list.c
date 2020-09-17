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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "range-list.h"
#include <atomic.h>
#include <kproc/lock.h>

/* Note: This synchronization is very specific to this use.
 * Give a good thought or two before using it somewhere else.
 * In particular, there is only one writer, and it needs to
 * synchronize very rarely, but the readers happen many
 * tens of thousands of times while the writer is running.
 * Furthermore, the readers continue to access the structure
 * long after the writer has finished and there's no longer
 * any possibility of a race condition.
 */
struct Sync {
    /* low order bit indicates that a writer is waiting (or active)
     * remainder of bits is the count of active readers
     */
    atomic64_t counter;
    KLock *mutex;
};

static void readerStart(Sync *const sync)
{
    if ((atomic64_read_and_add_even(&sync->counter, 2) & 1) != 0) {
        /* a writer is waiting or active */
        KLockAcquire(sync->mutex);
        atomic64_add(&sync->counter, 2);
        KLockUnlock(sync->mutex);
    }
}

static void readerDone(Sync *const sync)
{
    atomic64_add(&sync->counter, -2);
}

static void writerStart(Sync *const sync)
{
    assert((atomic64_read(&sync->counter) & 1) == 0); /* there is only one writer */

    atomic64_inc(&sync->counter); /* tell readers to wait */
    KLockAcquire(sync->mutex);
    while (atomic_read(&sync->counter) != 1) /* wait for readers to finish */
        ;
}

static void writerDone(Sync *const sync)
{
    atomic64_dec(&sync->counter);
    KLockUnlock(sync->mutex);
}

static void updateRanges(  RangeList *const self
                         , Range *const ranges
                         , unsigned const allocated)
{
    writerStart(self->sync);
    self->ranges = ranges;
    self->allocated = allocated;
    writerDone(self->sync);
}

void intersectRanges(Range *result, Range const *a, Range const *b)
{
    unsigned const s = a->start < b->start ? b->start : a->start;
    unsigned const e = a->end < b->end ? a->end : b->end;
    result->start = s;
    result->end = s < e ? e : s;
}

void intersectRangeList(RangeList const *list, Range const **begin, Range const **end, Range const *query)
{
    assert(begin != NULL);
    assert(end != NULL);
    assert(query != NULL);
    assert(list != NULL);

    *end = &list->ranges[list->count];
    *begin = *end;
    if (query->start < query->end && list->count > 0) {
        unsigned f = 0;
        unsigned e = list->count;

        while (f < e) {
            unsigned const m = f + (e - f) / 2;
            Range const *const M = &list->ranges[m];
            if (M->start < query->end)
                f = m + 1;
            else
                e = m;
        }
        *end = &list->ranges[f];

        f = 0;
        while (f < e) {
            unsigned const m = f + (e - f) / 2;
            Range const *const M = &list->ranges[m];
            if (M->end <= query->start)
                f = m + 1;
            else
                e = m;
        }
        *begin = &list->ranges[f];
    }
}

static void withIntersectRangeList_1(  RangeList const *const list
                                     , Range const *const query
                                     , IntersectRangeListCallback const callback
                                     , void *const data)
{
    if (list->sync) {
        readerStart(list->sync);
        {
            Range const *const ranges = list->ranges;
            unsigned const count = list->count;
            unsigned f = 0;
            unsigned e = count;

            while (f < e) {
                unsigned const m = f + (e - f) / 2;
                Range const *const M = &ranges[m];
                if (M->end <= query->start)
                    f = m + 1;
                else
                    e = m;
            }
            while (f < count) {
                Range intersected;

                intersectRanges(&intersected, &ranges[f++], query);
                if (intersected.end == intersected.start)
                    break;

                callback(data, &intersected);
            }
        }
        readerDone(list->sync);
    }
}


void withIntersectRangeList(RangeList const *list, Range const *query, IntersectRangeListCallback callback, void *data)
{
    withIntersectRangeList_1(list, query, callback, data);
}

static RangeList *grow(RangeList *const list)
{
    if (list->sync == NULL) {
        Sync *sync = calloc(1, sizeof(*sync));
        if (sync) {
            rc_t const rc = KLockMake(&sync->mutex);
            if (rc == 0)
                list->sync = sync;
            else {
                free(sync);
                return NULL;
            }
        }
        else
            return NULL;
    }

    if (list->count == list->allocated) {
        void *const old = list->ranges;
        unsigned const allocated = list->allocated;
        unsigned const new_allocated = allocated == 0 ? 16 : (allocated * 2);
        void *tmp = malloc(new_allocated * sizeof(list->ranges[0]));
        if (tmp == NULL)
            return NULL;
        memmove(tmp, old, allocated * sizeof(list->ranges[0]));

        updateRanges(list, tmp, new_allocated);
        free(old);
    }
    return list;
}

static void insert(RangeList *const list, unsigned const at)
{
    unsigned i = list->count;
    while (i > at) {
        list->ranges[i] = list->ranges[i - 1];
        --i;
    }
    ++list->count;
}

static void remove(RangeList *const list, unsigned at)
{
    while (at + 1 < list->count) {
        list->ranges[at] = list->ranges[at + 1];
        ++at;
    }
    --list->count;
}

/* merge [at] and [at + 1] */
static void collapse(RangeList *const list, unsigned const at)
{
    if (at + 1 < list->count && list->ranges[at].end == list->ranges[at + 1].start) {
        unsigned const start = list->ranges[at].start;
        remove(list, at);
        list->ranges[at].start = start;
    }
}

Range *appendRange(RangeList *list, Range const *newValue)
{
    if (grow(list) != NULL) {
        Range *nr = &list->ranges[list->count];

        if (newValue)
            *nr = *newValue;
        list->last = list->count;
        ++list->count;

        return nr;
    }
    return NULL;
}

static RangeList *extendRangeList_1(RangeList *const list, unsigned const position)
{
    if (list->count > 0) {
        Range const lastRange = list->ranges[list->last];

        if (lastRange.end == position) {
            list->ranges[list->last].end = position + 1;
            collapse(list, list->last);
            return list;
        }
        if (lastRange.start <= position && position < lastRange.end)
            return list;
        if (position < list->ranges[list->count - 1].end) {
            unsigned f = 0;
            unsigned e = list->count;

            while (f < e) {
                unsigned const m = f + (e - f) / 2;
                Range const *const M = &list->ranges[m];
                if (M->end < position)
                    f = m + 1;
                else
                    e = m;
            }
            if (f < list->count) {
                if (position < list->ranges[f].start) {
                    // insert new range before f
                    if (!grow(list))
                        return NULL;
                    insert(list, f);
                    list->ranges[f].start = position;
                    list->ranges[f].end = position + 1;
                    list->last = f;
                    return list;
                }
                if (position > list->ranges[f].end) {
                    // insert new range after f
                    if (!grow(list))
                        return NULL;
                    insert(list, f + 1);
                    list->ranges[f + 1].start = position;
                    list->ranges[f + 1].end = position + 1;
                    list->last = f + 1;
                    return list;
                }
                if (position == list->ranges[f].end) {
                    list->last = f;
                    return extendRangeList(list, position);
                }
            }
        }
    }
    {
        Range nr;
        nr.end = (nr.start = position) + 1;
        if (appendRange(list, &nr))
            return list;
    }
    return NULL;
}

RangeList *extendRangeList(RangeList *list, unsigned position)
{
    return extendRangeList_1(list, position);
}

void RangeListFree(RangeList *list)
{
    free(list->ranges);
}

RangeList const *copyRangeList(RangeList *list)
{
    struct {
        RangeList list;
        Range array[1];
    } *temp = NULL;
    size_t const alloc = (uint8_t const *)&temp->array[list->count] - (uint8_t const *)temp;

    temp = malloc(alloc);
    assert(temp != NULL);
    if (temp) {
        {
            unsigned i;
            for (i = 0; i < list->count; ++i)
                temp->array[i] = list->ranges[i];
        }
        temp->list = *list;
        temp->list.ranges = temp->array;
        temp->list.last = 0;

        return &temp->list;
    }
    return NULL;
}

int checkRangeList(RangeList const *list)
{
    unsigned i = 0;
    if (i < list->count) {
        Range r = list->ranges[i++];

        assert(r.start < r.end);
        if (!(r.start < r.end)) return !!0;
        while (i < list->count) {
            Range const p = r;

            r = list->ranges[i++];

            assert(r.start < r.end);
            if (!(r.start < r.end)) return !!0;
            assert(p.end < r.start);
            if (!(p.end < r.start)) return !!0;
        }
    }
    return !0;
}
