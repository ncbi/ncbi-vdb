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

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <klib/text.h>
#include <klib/refcount.h>
#include <atomic.h>
#include <kproc/lock.h>
#include <kdb/kdb-priv.h> /* KDBManagerGetVFSManager */
#include <kdb/manager.h>
#include <kdb/meta.h>
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path-priv.h> /* VPathSetAccOfParentDb */

#include <ctype.h>
#include <assert.h>
#include <limits.h>

#define DEFAULT_WGS_OPEN_LIMIT 8

#include "restore-read.h"
#include "refseq.h"
#include "wgs.h"

static VFSManager *getVFSManager(VDBManager const *mgr)
{
    VFSManager *result = NULL;
    KDBManager const *kmgr = NULL;
    rc_t rc = 0;

    rc = VDBManagerGetKDBManagerRead(mgr, &kmgr);
    assert(rc == 0);

    rc = KDBManagerGetVFSManager(kmgr, &result);
    KDBManagerRelease(kmgr);
    assert(rc == 0);
    UNUSED(rc);

    return result;
}

static VPath *makePath(VDBManager const *mgr, unsigned length, char const *accession)
{
    VPath *result = NULL;
    VFSManager *const vfs = getVFSManager(mgr);
    VFSManagerMakePath(vfs, &result, "%.*s", (int)length, accession);
    VFSManagerRelease(vfs);
    return result;
}

static String const *getContainer(VTable const *const forTable,
    const String ** path)
{
    VDatabase const *db = NULL;
    rc_t rc = VTableOpenParentRead(forTable, &db);
    if (rc == 0) {
        String const *container = NULL;
        rc = VDatabaseGetAccession(db, &container, path);
        VDatabaseRelease(db);
        if (rc == 0)
            return container;
    }
    return NULL;
}

static VPath const *getURL(VDBManager const *mgr, unsigned length, char const *accession, VTable const *const forTable)
{
    VPath * const result = makePath(mgr, length, accession);
    if (result) {
        String const * path = NULL;
        String const * const container = getContainer(forTable, &path);
        rc_t const rc = VPathSetAccOfParentDb(result, container, path);
        StringWhack(container);
        StringWhack(path);
        if (rc == 0)
            return result;
        VPathRelease(result);
    }
    return NULL;
}

static char *getSchemaName_Node(char *result, size_t *actsize, KMDataNode const *node)
{
    rc_t const rc = KMDataNodeReadAttr(node, "name", result, *actsize, actsize);
    KMDataNodeRelease(node);
    return rc == 0 ? result : NULL;
}

static char const *getSchemaName_DB(char *result, size_t *actsize, VDatabase const *db)
{
    KMetadata const *meta = NULL;
    rc_t rc = VDatabaseOpenMetadataRead(db, &meta);
    assert(rc == 0);

    KMDataNode const *node = NULL;
    rc = KMetadataOpenNodeRead(meta, &node, "schema");
    KMetadataRelease(meta);
    if (rc) return NULL;

    return getSchemaName_Node(result, actsize, node);
}

static char *getSchemaName_Table(char *result, size_t *actsize, VTable const *tbl)
{
    KMetadata const *meta = NULL;
    rc_t rc = VTableOpenMetadataRead(tbl, &meta);
    assert(rc == 0);

    KMDataNode const *node = NULL;
    rc = KMetadataOpenNodeRead(meta, &node, "schema");
    KMetadataRelease(meta);
    if (rc) return NULL;

    return getSchemaName_Node(result, actsize, node);
}

static bool schemaNameIsEqual(char const *name, unsigned namelen, char const *query)
{
    unsigned i;
    for (i = 0; i < namelen; ++i) {
        if (query[i] == '\0') break;
        if (query[i] != name[i]) return false;
    }
    return i == namelen || (i + 1 < namelen && name[i] == '#' && isdigit(name[i + 1]));
}

static bool tableSchemaNameIsEqual(VTable const *tbl, char const *name)
{
    char buffer[1024];
    size_t size = sizeof(buffer);
    return getSchemaName_Table(buffer, &size, tbl) == NULL ? false : schemaNameIsEqual(buffer, (unsigned)size, name);
}

static bool dbSchemaNameIsEqual(VDatabase const *db, char const *name)
{
    char buffer[1024];
    size_t size = sizeof(buffer);
    return getSchemaName_DB(buffer, &size, db) == NULL ? false : schemaNameIsEqual(buffer, (unsigned)size, name);
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

typedef struct ErrorListEntry ErrorListEntry;
struct ErrorListEntry {
    char *name;
    rc_t error;
};

typedef struct ErrorList ErrorList;
struct ErrorList {
    ErrorListEntry *entry;
    unsigned entries;
    unsigned allocated;
};

static bool Error_Find(ErrorList *list, unsigned *at, unsigned const qlen, char const *qry)
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
    *at = f; // not found but it could be inserted here
    return false;
}

static rc_t Error_Insert(ErrorList *list, unsigned at, unsigned const qlen, char const *qry, rc_t error)
{
    ErrorListEntry temp;

    temp.name = malloc(qlen + 1);
    if (temp.name == NULL) {
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
    }
    memmove(temp.name, qry, qlen);
    temp.name[qlen] = '\0';
    temp.error = error;

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

static void ErrorListFree(ErrorList *list)
{
    unsigned i;
    for (i = 0; i != list->entries; ++i) {
        free(list->entry[i].name);
    }
    free(list->entry);
}

typedef struct RestoreReadShared RestoreReadShared;
struct RestoreReadShared {
    KRefcount refcount;
    KRWLock *rwl;
    RefSeqList refSeqs;
    WGS_List wgs;
    ErrorList errors;
};

static atomic_ptr_t g_shared;

static void RestoreReadSharedReader(RestoreReadShared *self)
{
    rc_t const rc = KRWLockAcquireShared(self->rwl);
    assert(rc == 0);
    UNUSED(rc);
}

static void RestoreReadSharedReaderDone(RestoreReadShared *self)
{
    rc_t const rc = KRWLockUnlock(self->rwl);
    assert(rc == 0);
    UNUSED(rc);
}

static void RestoreReadSharedWriter(RestoreReadShared *self)
{
    RestoreReadSharedReaderDone(self);
    {
        rc_t const rc = KRWLockAcquireExcl(self->rwl);
        assert(rc == 0);
        UNUSED(rc);
    }
}

static void RestoreReadSharedWriterDone(RestoreReadShared *self)
{
    {
        rc_t const rc = KRWLockUnlock(self->rwl);
        assert(rc == 0);
        UNUSED(rc);
    }
    RestoreReadSharedReader(self);
}

static bool isSameCountRefSeqs(RestoreReadShared *const self, unsigned const count)
{
    return count == self->refSeqs.entries;
}

static bool isSameCountWGS(RestoreReadShared *const self, unsigned const count)
{
    return count == self->wgs.entries;
}

static rc_t RestoreReadSharedMake(RestoreReadShared **const pnew)
{
    rc_t rc = 0;
    RestoreReadShared *const rslt = calloc(1, sizeof(*rslt));

    *pnew = NULL;
    if (rslt) {
        KRefcountInit(&rslt->refcount, 0, "RestoreReadShared", "init", "global");
        rc = KRWLockMake(&rslt->rwl);
        if (rc == 0)
            *pnew = rslt;
        else
            free(rslt);
    }
    return rc;
}

static void RestoreReadSharedFree(RestoreReadShared *const self)
{
    RefSeqListFree(&self->refSeqs);
    WGS_ListFree(&self->wgs);
    ErrorListFree(&self->errors);
    KRWLockRelease(self->rwl);
    KRefcountWhack(&self->refcount, "RestoreReadShared");
    free(self);
    LOGMSG(klogDebug, "Released shared global RestoreRead data object");
}

static void RestoreReadSharedRelease(RestoreReadShared *const self)
{
    switch (KRefcountDrop(&self->refcount, "RestoreReadShared")) {
    case krefWhack:
        atomic_test_and_set_ptr(&g_shared, NULL, self);
        RestoreReadSharedFree(self);
        /* no break */
    case krefOkay:
        return;
    default:
        assert(!"valid refcount");
        abort();
    }
}

static RestoreReadShared *getRestoreReadShared(rc_t *const prc)
{
    if (g_shared.ptr == NULL) {
        RestoreReadShared *temp = NULL;
        *prc = RestoreReadSharedMake(&temp);
        if (temp) {
            WGS_ListInit(&temp->wgs, DEFAULT_WGS_OPEN_LIMIT);
            *prc = RefSeqListInit(&temp->refSeqs);

            if (atomic_test_and_set_ptr(&g_shared, temp, NULL) != NULL)
                RestoreReadSharedFree(temp);
        }
    }
    if (g_shared.ptr)
        KRefcountAdd(&((RestoreReadShared *)g_shared.ptr)->refcount, "RestoreReadShared");

    return g_shared.ptr;
}

unsigned RestoreReadShared_getState(unsigned *refSeqs, unsigned *wgs, unsigned *errors, unsigned *activeRefSeqs)
{
    if (g_shared.ptr) {
        rc_t rc = 0;
        RestoreReadShared *const ptr = getRestoreReadShared(&rc);

        RestoreReadSharedReader(ptr);
        *refSeqs = ptr->refSeqs.entries;
        *wgs = ptr->wgs.entries;
        *errors = ptr->errors.entries;
        *activeRefSeqs = 0;
        {
            unsigned i;
            for (i = 0; i < *refSeqs; ++i) {
                if (ptr->refSeqs.entry[i].object->async != NULL)
                    ++*activeRefSeqs;
            }
        }
        RestoreReadSharedReaderDone(ptr);
        RestoreReadSharedRelease(ptr);
        return 1;
    }
    else return 0;
}

struct RestoreRead {
    VDBManager const *mgr;
    RestoreReadShared *shared;
    struct Last {
        union U {
            RefSeqListEntry *r;
            WGS_ListEntry *w;
        } u;
        unsigned count; /**< the pointer is invalid if count != current count */
        enum { L_none, refSeq_type, wgs_type } type;
    } last;
};

void RestoreReadFree(void *vp)
{
    RestoreRead *const self = (RestoreRead *)vp;

    VDBManagerRelease(self->mgr);
    RestoreReadSharedRelease(self->shared);

    free(self);
}

RestoreRead *RestoreReadMake(VDBManager const *vmgr, rc_t *rcp)
{
    RestoreRead *self = calloc(1, sizeof(*self));
    if (self) {
        self->mgr = vmgr;
        *rcp = VDBManagerAddRef(self->mgr);
        self->shared = getRestoreReadShared(rcp);
    }
    return self;
}

static rc_t openSeqID(  RestoreRead *const self
                      , unsigned const id_len
                      , unsigned const wgs_id_len
                      , char const *const seq_id
                      , VTable const *const forTable)
{
    VDatabase const *db = NULL;
    VTable const *tbl = NULL;
    VPath const *url = getURL(self->mgr, id_len, seq_id, forTable);
    rc_t rc = 0;

    if (url) {
        // open the new way with the URL
        VDBManagerOpenTableReadVPath(self->mgr, &tbl, NULL, url);
        if (tbl == NULL && wgs_id_len > 0)
            rc = VDBManagerOpenDBReadVPath(self->mgr, &db, NULL, url);
    }
    else {
        // open the old way
        VDBManagerOpenTableRead(self->mgr, &tbl, NULL, "ncbi-acc:%.*s?vdb-ctx=refseq", (int)id_len, seq_id);
        if (tbl == NULL && wgs_id_len > 0)
            rc = VDBManagerOpenDBRead(self->mgr, &db, NULL, "%.*s", (int)id_len, seq_id);
    }
    if (tbl != NULL) {
        if (tableSchemaNameIsEqual(tbl, RefSeq_Scheme())) {
            RestoreReadSharedWriter(self->shared);
            self->last.u.r = RefSeqInsert(&self->shared->refSeqs, id_len, seq_id, tbl, &rc);
            self->last.count = self->shared->refSeqs.entries;
            RestoreReadSharedWriterDone(self->shared);
            if (self->last.u.r)
                self->last.type = refSeq_type;
        }
        else {
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
            PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a RefSeq", "name=%.*s", (int)id_len, seq_id));
        }
    }
    else if (db != NULL) {
        if (dbSchemaNameIsEqual(db, WGS_Scheme())) {
            RestoreReadSharedWriter(self->shared);
            self->last.u.w = WGS_Insert(&self->shared->wgs, wgs_id_len, seq_id, url, db, &rc);
            self->last.count = self->shared->wgs.entries;
            RestoreReadSharedWriterDone(self->shared);
            if (self->last.u.w)
                self->last.type = wgs_type;
        }
        else {
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
            PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a WGS", "name=%.*s", (int)id_len, seq_id));
        }
    }
    else {
        if (rc == 0)
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
        PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a RefSeq or as a WGS", "name=%.*s", (int)id_len, seq_id));
    }
    VPathRelease(url);
    VTableRelease(tbl);
    VDatabaseRelease(db);
    return rc;
}

static rc_t getSequence(  RestoreRead *const self
                        , unsigned const start
                        , unsigned const length, uint8_t *const dst
                        , unsigned const id_len, char const *const seq_id
                        , unsigned *const actual
                        , VTable const *const forTable)
{
    unsigned wgs_namelen = 0;
    int64_t wgs_row = 0;
    unsigned error_at = 0;
    rc_t rc = 0;

    for ( ; ; ) {
        switch (self->last.type) {
        case refSeq_type:
            if (isSameCountRefSeqs(self->shared, self->last.count) && name_cmp(self->last.u.r->name, (unsigned)id_len, seq_id) == 0) {
REFSEQ_FROM_LAST:
                *actual = RefSeq_getBases(self->last.u.r->object, dst, start, length);
                return 0;
            }
            break;
        case wgs_type:
            wgs_namelen = WGS_splitName(&wgs_row, id_len, seq_id);
            if (wgs_namelen > 0 && isSameCountWGS(self->shared, self->last.count) && name_cmp(self->last.u.w->name, wgs_namelen, seq_id) == 0) {
                assert(self->last.u.w->object->curs != NULL);
WGS_FROM_LAST:
                *actual = WGS_getBases(self->last.u.w->object, dst, start, length, wgs_row);
                return 0;
            }
            break;
        default:
            break;
        }
        self->last.type = L_none;

        // check error list
        if (Error_Find(&self->shared->errors, &error_at, id_len, seq_id))
            return self->shared->errors.entry[error_at].error;

        // check refSeq list
        if ((self->last.u.r = RefSeqFind(&self->shared->refSeqs, id_len, seq_id)) != NULL) {
            self->last.count = self->shared->refSeqs.entries;
            self->last.type = refSeq_type;
            goto REFSEQ_FROM_LAST;
        }

        // check WGS list
        wgs_namelen = WGS_splitName(&wgs_row, id_len, seq_id);
        if (wgs_namelen > 0 && (self->last.u.w = WGS_Find(&self->shared->wgs, wgs_namelen, seq_id)) != NULL) {
            if (self->last.u.w->object->curs == NULL) {
                rc_t rc2 = 0;

                WGS_limitOpen(&self->shared->wgs);
                rc2 = WGS_reopen(self->last.u.w->object, self->mgr, wgs_namelen, seq_id);
                if (rc2) return rc2;
                ++self->shared->wgs.openCount;
            }
            self->last.count = self->shared->wgs.entries;
            self->last.type = wgs_type;
            goto WGS_FROM_LAST;
        }

        // it is a new seq_id
        rc = openSeqID(self, id_len, wgs_namelen, seq_id, forTable);
        if (rc) {
            Error_Insert(&self->shared->errors, error_at, id_len, seq_id, rc);
            return rc;
        }
    }
    assert(!"reachable");
    abort();
}

rc_t RestoreReadGetSequence(  RestoreRead *self
                            , unsigned start
                            , size_t length, uint8_t *dst
                            , size_t id_len, char const *seq_id
                            , unsigned *actual
                            , VTable const *forTable)
{
    rc_t rc;

    assert(length < UINT_MAX);
    assert(id_len < UINT_MAX);

    RestoreReadSharedReader(self->shared);
    rc = getSequence(self, start, (unsigned)length, dst, (unsigned)id_len, seq_id, actual, forTable);
    RestoreReadSharedReaderDone(self->shared);
    return rc;
}
