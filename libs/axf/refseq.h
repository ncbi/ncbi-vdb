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

#include <atomic.h>
#include "range-list.h"

typedef struct RefSeq RefSeq;
typedef struct RefSeqListEntry RefSeqListEntry;
typedef struct RefSeqList RefSeqList;
typedef struct RefSeqAsyncLoadInfo RefSeqAsyncLoadInfo;
typedef unsigned (*RefSeqReaderFunc)(RefSeq const *, uint8_t *, unsigned, unsigned);

struct RefSeq {
    RangeList Ns; ///< exclusion list
    uint8_t *bases;
    RefSeqReaderFunc volatile reader;
    RefSeqAsyncLoadInfo *volatile async;
    atomic_t rwl;
    unsigned length; ///< logical length, is base count of the reference
};

#define LIST_OBJECT RefSeq
#define LIST_ENTRY RefSeqListEntry
#include "list.h"

struct RefSeqList {
    LIST;
};
#undef LIST
#undef LIST_ENTRY
#undef LIST_OBJECT

char const *RefSeq_Scheme(void);

unsigned RefSeq_getBases(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len);

void RefSeqFree(RefSeq *self);

RefSeqListEntry *RefSeqFind(RefSeqList *list, unsigned const qlen, char const *qry);

RefSeqListEntry *RefSeqInsert(RefSeqList *list, unsigned const qlen, char const *qry, VTable const *tbl, rc_t *prc);

void RefSeqListFree(RefSeqList *list);

rc_t RefSeqListInit(RefSeqList *list);
