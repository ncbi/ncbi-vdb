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

/* ********* NOTE!!! ********* */
/*                             */
/* CONCURRENT USE IS ONLY SAFE */
/* IN EXACTLY THE WAY THAT     */
/* refseq.c USES THIS.         */
/*                             */
/* That is, inserts only occur */
/* at the end and are outside  */
/* the region that another     */
/* thread would be using.      */
/*                             */
/* There is no provision for   */
/* synchronizing multiple      */
/* writers.                    */
/*                             */
/* ********* NOTE!!! ********* */

typedef struct Range Range;
typedef struct RangeList RangeList;
typedef struct Sync Sync;

/**
 * @note start must be less than or equal to end
 */
struct Range {
    unsigned start, end;
};

/**
 * @brief An ordered list of Range, memset(0) to initialize.
 * @note ranges do not overlap
 */
struct RangeList {
    Range *ranges;
    Sync *sync;
    unsigned count;
    unsigned allocated; ///< used only by the writer
    unsigned last;      ///< used only by the writer
};

typedef void (*IntersectRangeListCallback)(void *data, Range const *intersectingRange);

/**
 * @brief intersect the query range with the ranges in the list and callback with the intersecting ranges
 * @note thread safe
 *
 * This is the main function used by a reader.
 *
 * There is an unchecked assertion is that readers have checked that
 * their query can not possibly overlap the end of the list before they got on this code path.
 */
void withIntersectRangeList(RangeList const *list, Range const *query, IntersectRangeListCallback callback, void *data);

/**
 * @brief Extend the last range in the list or insert a new range if position is not contiguous
 * @note thread safe
 *
 * This is the main function used by a writer.
 */
RangeList *extendRangeList(RangeList *list, unsigned position);

/**
 * @brief performs a consistency check
 */
int checkRangeList(RangeList const *);

/**
 * @brief free the memory for the ranges and any synchronization
 * @param list this pointer is not freed
 * @note *NOT* thread safe, only call after any writer thread has exited
 */
void RangeListFree(RangeList *list);

/**
 * @brief intersect two ranges
 * @note not used directly by refseq.c
 */
void intersectRanges(Range *result, Range const *a, Range const *b);

/**
 * @brief add a range to the end of the list
 * @note not used directly by refseq.c and not thread safe in general
 */
Range *appendRange(RangeList *list, Range const *newValue);

/**
 * @brief intersect the query range with the ranges in the list
 * @note not used by refseq.c and not thread safe or easy to use, exists for testing
 */
void intersectRangeList(RangeList const *list, Range const **begin, Range const **end, Range const *query);

/**
 * @note not used by refseq.c and not thread safe or easy to use, exists for testing, don't use otherwise
 */
RangeList const *copyRangeList(RangeList *list);
