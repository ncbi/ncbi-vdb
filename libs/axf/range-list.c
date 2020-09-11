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

typedef struct Sync Sync;
struct Sync {
    atomic64_t readers;
    atomic64_t writers;
};

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

void withIntersectRangeList(RangeList const *list, Range const *query, IntersectRangeListCallback callback, void *data)
{
    if (list->sync) {
        Sync *const sync = list->sync;

        atomic_inc(&sync->readers);
        while (atomic_read(&sync->writers) != 0)
            ;
        {
            unsigned f = 0;
            unsigned e = list->count;

            while (f < e) {
                unsigned const m = f + (e - f) / 2;
                Range const *const M = &list->ranges[m];
                if (M->end <= query->start)
                    f = m + 1;
                else
                    e = m;
            }
            while (f < list->count) {
                Range intersected;

                intersectRanges(&intersected, &list->ranges[f++], query);
                if (intersected.end == intersected.start)
                    break;

                callback(data, &intersected);
            }
        }
        atomic_dec(&sync->readers);
    }
}

static RangeList *grow(RangeList *list)
{
    if (list->count == list->allocated) {
        if (list->sync == NULL)
            list->sync = calloc(1, sizeof(Sync));
        if (list->sync) {
            Sync *const sync = list->sync;
            unsigned const new_allocated = (list->allocated == 0 ? 16 : list->allocated * 2);
            void *tmp = malloc(new_allocated * sizeof(list->ranges[0]));
            if (tmp == NULL)
                return NULL;
            memmove(tmp, list->ranges, list->allocated * sizeof(list->ranges[0]));
            list->allocated = new_allocated;

            atomic_inc(&sync->writers);
            while (atomic_read(&sync->readers) != 0)
                ;

            list->ranges = tmp;

            atomic_dec(&sync->writers);
        }
        else
            return NULL;
    }
    return list;
}

static void insert(RangeList *list, unsigned at)
{
    unsigned i = list->count;
    while (i > at) {
        list->ranges[i] = list->ranges[i - 1];
        --i;
    }
    ++list->count;
}

static void remove(RangeList *list, unsigned at)
{
    while (at + 1 < list->count) {
        list->ranges[at] = list->ranges[at + 1];
        ++at;
    }
    --list->count;
}

/* merge [at] and [at + 1] */
static void collapse(RangeList *list, unsigned at)
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

        list->last = list->count;
        ++list->count;
        if (newValue)
            *nr = *newValue;

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
        Range *new_range = appendRange(list, NULL);
        if (new_range) {
            new_range->start = position;
            new_range->end = position + 1;
            return list;
        }
    }
    return NULL;
}

RangeList *extendRangeList(RangeList *const list, unsigned const position)
{
    return extendRangeList_1(list, position);
}

void RangeListFree(RangeList *list)
{
    assert(list->sync == NULL || atomic_read(&((Sync *)(list->sync))->readers) == 0);
    assert(list->sync == NULL || atomic_read(&((Sync *)(list->sync))->writers) == 0);
    free(list->ranges);
    free(list->sync);
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
