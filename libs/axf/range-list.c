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
#include <assert.h>
#include "range-list.h"

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

Range *appendRange(RangeList *list, Range const *newValue)
{
    if (list->count == list->allocated) {
        unsigned const new_allocated = (list->allocated == 0 ? 16 : list->allocated * 2);
        void *tmp = realloc(list->ranges, new_allocated * sizeof(list->ranges[0]));
        if (tmp == NULL)
            return NULL;
        list->ranges = tmp;
        list->allocated = new_allocated;
    }
    {
        Range *nr = &list->ranges[list->count];

        ++list->count;
        if (newValue)
            *nr = *newValue;

        return nr;
    }
}

void extendRangeList(RangeList *list, unsigned position)
{
    if (list->count > 0 && list->ranges[list->count - 1].end == position) {
        list->ranges[list->count - 1].end += 1;
    }
    else {
        Range *new_range = appendRange(list, NULL);
        new_range->start = position;
        new_range->end = position + 1;
    }
}

void RangeListFree(RangeList *list)
{
    free(list->ranges);
}
