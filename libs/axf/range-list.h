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

typedef struct Range Range;
typedef struct RangeList RangeList;

struct Range {
    unsigned start, end;
};

struct RangeList {
    Range *ranges;
    unsigned count;
    unsigned allocated;
};

void intersectRanges(Range *result, Range const *a, Range const *b);

void intersectRangeList(RangeList const *list, Range const **begin, Range const **end, Range const *query);

Range *appendRange(RangeList *list, Range const *newValue);

void extendRangeList(RangeList *list, unsigned position);

void RangeListFree(RangeList *list);
