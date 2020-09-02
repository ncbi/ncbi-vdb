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

#include "range-list.h"

typedef struct RefSeq RefSeq;
typedef unsigned (*RefSeqReaderFunc)(RefSeq const *self, uint8_t *dst, unsigned start, unsigned len);
typedef struct RefSeqAsyncLoadInfo RefSeqAsyncLoadInfo;

struct RefSeq {
    RangeList *Ns; ///< exclusion list
    uint8_t *bases;
    RefSeqAsyncLoadInfo *asyncLoader;
    RefSeqReaderFunc reader;
    unsigned length; ///< logical length, is base count of the reference
};

char const *RefSeq_Scheme(void);

unsigned RefSeq_getBases(RefSeq const *self, uint8_t *const dst, unsigned const start, unsigned const len);

typedef struct semaphore semaphore;
struct semaphore {
    struct KSemaphore *sema;
    struct KLock *lock;
};

rc_t RefSeq_load(RefSeq *result, VTable const *tbl, semaphore *sema);

void RefSeqFree(RefSeq *self);

typedef struct RefSeqListEntry RefSeqListEntry;
struct RefSeqListEntry {
    char *name;
    RefSeq object;
};

typedef struct RefSeqList RefSeqList;
struct RefSeqList {
    semaphore sema;
    RefSeqListEntry *entry;
    unsigned entries;
    unsigned allocated;
};

bool RefSeq_Find(RefSeqList *list, unsigned *at, unsigned const qlen, char const *qry);

rc_t RefSeq_Insert(RefSeqList *list, unsigned at, unsigned const qlen, char const *qry, RefSeq *value);

void RefSeqListFree(RefSeqList *list);

rc_t RefSeqListInit(RefSeqList *list);
