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

static String const *getContainer(VTable const *const forTable)
{
    VDatabase const *db = NULL;
    rc_t rc = VTableOpenParentRead(forTable, &db);
    if (rc == 0) {
        String const *container = NULL;
        rc = VDatabaseGetAccession(db, &container);
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
        String const * const container = getContainer(forTable);
        rc_t const rc = VPathSetAccOfParentDb(result, container);
        StringWhack(container);
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

typedef struct Cache Cache;
typedef struct CacheEntry CacheEntry;

struct CacheEntry {
    char const *name;
    enum { CE_unset, CE_error, CE_refseq, CE_WGS } type;
    union {
        struct RefSeq *refseq;
        struct VPath const *WGS_url;
        rc_t error;
    } value;
};

struct Cache {
    CacheEntry *entry;
    unsigned entries;
    unsigned allocated;
};

static int name_cmp(char const *const name, char const *const qry, unsigned const qlen, unsigned const altlen)
{
    unsigned i;
    unsigned const alen = altlen == 0 ? qlen : altlen;
    for (i = 0; i < qlen; ++i) {
        int const a = name[i];
        int const b = qry[i];
        int const d = a - b;
        if (a == 0) return (i == alen) ? 0 : d;
        if (d == 0) continue;
        return d;
    }
    return name[qlen] - '\0';
}

static bool find(Cache const *self, unsigned *const at, char const *const qry, unsigned const qlen, unsigned const altlen)
{
    unsigned f = 0;
    unsigned e = self->entries;

    while (f < e) {
        unsigned const m = f + (e - f) / 2;
        CacheEntry const *const M = &self->entry[m];
        int const d = name_cmp(M->name, qry, qlen, altlen);

        assert(M->type != CE_unset);
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

static CacheEntry *insert(Cache *list, unsigned const at, unsigned const namelen, char const *name)
{
    char *const copy = malloc(namelen + 1);
    if (copy == NULL)
        return NULL;
    memmove(copy, name, namelen);
    copy[namelen] = '\0';

    if (list->entries >= list->allocated) {
        unsigned const new_alloc = list->allocated == 0 ? 16 : (list->allocated * 2);
        void *tmp = realloc(list->entry, new_alloc * sizeof(*list->entry));
        if (tmp == NULL)
            return NULL;
        list->entry = tmp;
        list->allocated = new_alloc;
    }
    memmove(&list->entry[at + 1], &list->entry[at], sizeof(*list->entry) * (list->entries - at));
    ++list->entries;

    memset(&list->entry[at], 0, sizeof(list->entry[at]));
    list->entry[at].name = copy;

    return &list->entry[at];
}

static
CacheEntry *CacheFind(Cache const *self, char const *const qry, unsigned const qlen, unsigned const altlen)
{
    unsigned at = self->entries;
    return find(self, &at, qry, qlen, altlen) ? &self->entry[at] : NULL;
}

static
CacheEntry *CacheInsert(Cache *self, char const *const qry, unsigned const qlen)
{
    unsigned at = self->entries;
    return find(self, &at, qry, qlen, qlen)
           ? &self->entry[at]
           : insert(self, at, qlen, qry);
}

static void CacheFree(Cache *list)
{
    unsigned i;
    for (i = 0; i != list->entries; ++i) {
        switch (list->entry[i].type) {
        case CE_refseq:
            RefSeqFree(list->entry[i].value.refseq);
            break;
        case CE_WGS:
            VPathRelease(list->entry[i].value.WGS_url);
            break;
        default:
            break;
        }
        free((void *)list->entry[i].name);
    }
    free(list->entry);
}

typedef struct RestoreReadShared RestoreReadShared;
struct RestoreReadShared {
    KRefcount refcount;
    KRWLock *rwl;
    Cache cache;
    WGS wgs;
};

static atomic_ptr_t g_shared;

static void RestoreReadSharedReader(RestoreReadShared *self)
{
    rc_t const rc = KRWLockAcquireShared(self->rwl);
    assert(rc == 0);
    ((void)rc);
}

static void RestoreReadSharedReaderDone(RestoreReadShared *self)
{
    rc_t const rc = KRWLockUnlock(self->rwl);
    assert(rc == 0);
    ((void)rc);
}

static void RestoreReadSharedWriter(RestoreReadShared *self)
{
    RestoreReadSharedReaderDone(self);
    {
        rc_t const rc = KRWLockAcquireExcl(self->rwl);
        assert(rc == 0);
        ((void)rc);
    }
}

static void RestoreReadSharedWriterDone(RestoreReadShared *self)
{
    {
        rc_t const rc = KRWLockUnlock(self->rwl);
        assert(rc == 0);
        ((void)rc);
    }
    RestoreReadSharedReader(self);
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
    CacheFree(&self->cache);
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
        unsigned n_refSeqs = 0;
        unsigned n_wgs = 0;
        unsigned n_errors = 0;
        rc_t rc = 0;
        RestoreReadShared *const ptr = getRestoreReadShared(&rc);

        RestoreReadSharedReader(ptr);

        *activeRefSeqs = 0;
        {
            unsigned i;
            unsigned const n = ptr->cache.entries;

            for (i = 0; i < n; ++i) {
                switch (ptr->cache.entry[i].type) {
                case CE_refseq:
                    ++n_refSeqs;
                    if (ptr->cache.entry[i].value.refseq->async != NULL)
                        ++*activeRefSeqs;
                    break;
                case CE_WGS:
                    ++n_wgs;
                    break;
                case CE_error:
                    ++n_errors;
                    break;
                default:
                    break;
                }
            }
        }
        *refSeqs = n_refSeqs;
        *wgs = n_wgs;
        *errors = n_errors;

        RestoreReadSharedReaderDone(ptr);
        RestoreReadSharedRelease(ptr);
        return 1;
    }
    else return 0;
}

struct RestoreRead {
    VDBManager const *mgr;
    RestoreReadShared *shared;
    CacheEntry *last;
    WGS wgs;
    unsigned count; /**< the last pointer is invalid if count != current count */
};

void RestoreReadFree(void *vp)
{
    RestoreRead *const self = (RestoreRead *)vp;

    WGS_close(&self->wgs);
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
    rc_t rc = 0;
    if (wgs_id_len == 0 || wgs_id_len == id_len) {
        VTable const *tbl = NULL;
        VPath const *const url = getURL(self->mgr, id_len, seq_id, forTable);
        if (url) {
            rc = VDBManagerOpenTableReadVPath(self->mgr, &tbl, NULL, url);
            VPathRelease(url);
        }
        else
            rc = VDBManagerOpenTableRead(self->mgr, &tbl, NULL, "ncbi-acc:%.*s?vdb-ctx=refseq", (int)id_len, seq_id);

        if (tbl != NULL && tableSchemaNameIsEqual(tbl, RefSeq_Scheme())) {
            RestoreReadSharedWriter(self->shared);
            self->last = CacheInsert(&self->shared->cache, seq_id, id_len);
            if (self->last->type == CE_unset) {
                self->last->type = CE_refseq;
                self->last->value.refseq = RefSeqNew(tbl, &rc);
                if (rc != 0) {
                    self->last->type = CE_error;
                    self->last->value.error = rc;
                    PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a RefSeq", "name=%.*s", (int)id_len, seq_id));
                }
                else {
                    PLOGMSG(klogDebug, (klogDebug, "opened $(name) as a RefSeq", "name=%.*s", (int)id_len, seq_id));
                }
            }
            RestoreReadSharedWriterDone(self->shared);
            assert(self->last->type == CE_refseq || self->last->type == CE_error);
            self->count = self->shared->cache.entries;
        }
        else {
            if (rc == 0)
                rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
            PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a RefSeq", "name=%.*s", (int)id_len, seq_id));
        }
        VTableRelease(tbl);
        return rc;
    }
    else {
        VDatabase const *db = NULL;
        VPath const *const url = getURL(self->mgr, wgs_id_len, seq_id, forTable);
        if (url)
            rc = VDBManagerOpenDBReadVPath(self->mgr, &db, NULL, url);
        else
            rc = VDBManagerOpenDBRead(self->mgr, &db, NULL, "%.*s", (int)wgs_id_len, seq_id);

        if (rc) {
            // not a database, so not WGS
            VPathRelease(url);
            return openSeqID(self, id_len, 0, seq_id, forTable);
        }
        if (dbSchemaNameIsEqual(db, WGS_Scheme())) {
            WGS_close(&self->wgs);
            RestoreReadSharedWriter(self->shared);
            self->last = CacheInsert(&self->shared->cache, seq_id, wgs_id_len);
            if (self->last->type == CE_unset) {
                rc = WGS_Init(&self->wgs, db);
                if (rc == 0) {
                    self->last->type = CE_WGS;
                    VPathAddRef(self->last->value.WGS_url = url);
                    PLOGMSG(klogDebug, (klogDebug, "opened $(name) as a WGS reference", "name=%.*s", (int)wgs_id_len, seq_id));
                }
                else {
                    self->last->type = CE_error;
                    self->last->value.error = rc;
                    PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a WGS", "name=%.*s", (int)wgs_id_len, seq_id));
                }
            }
            RestoreReadSharedWriterDone(self->shared);
            assert(self->last->type == CE_WGS || self->last->type == CE_error);
            self->count = self->shared->cache.entries;
        }
        else {
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
            PLOGERR(klogWarn, (klogWarn, rc, "can't open $(name) as a WGS", "name=%.*s", (int)id_len, seq_id));
        }
        VDatabaseRelease(db);
        VPathRelease(url);
        return rc;
    }
}

static rc_t getSequence(  RestoreRead *const self
                        , unsigned const start
                        , unsigned const length, uint8_t *const dst
                        , unsigned const id_len, char const *const seq_id
                        , unsigned *const actual
                        , VTable const *const forTable)
{
    int64_t wgs_row = 0;
    unsigned const wgs_namelen = WGS_splitName(&wgs_row, id_len, seq_id);
    rc_t rc = 0;

    if (self->count == self->shared->cache.entries
        && self->last != NULL
        && name_cmp(self->last->name, seq_id, id_len, wgs_namelen) == 0)
    {
    USE_LAST:
        assert(self->last->type != CE_unset);
        switch (self->last->type) {
        case CE_error:
            return self->last->value.error;
        case CE_refseq:
            *actual = RefSeq_getBases(self->last->value.refseq, dst, start, length);
            return 0;
        case CE_WGS:
            *actual = WGS_getBases(&self->wgs, dst, start, length, wgs_row);
            return 0;
        default:
            abort();
        }
    }
    self->last = CacheFind(&self->shared->cache, seq_id, id_len, wgs_namelen);
    if (self->last) {
        self->count = self->shared->cache.entries;
        if (self->last->type == CE_WGS && self->wgs.curs == NULL) {
            rc = WGS_reopen(&self->wgs, self->mgr, self->last->value.WGS_url, wgs_namelen, seq_id);
            if (rc) return rc;
            PLOGMSG(klogDebug, (klogDebug, "reopened $(name) as a WGS reference", "name=%.*s", (int)wgs_namelen, seq_id));
        }
        goto USE_LAST;
    }

    // it is a new seq_id
    rc = openSeqID(self, id_len, wgs_namelen, seq_id, forTable);
    if (rc == 0)
        goto USE_LAST;

    RestoreReadSharedWriter(self->shared);
    self->last = CacheInsert(&self->shared->cache, seq_id, id_len);
    assert(self->last->type == CE_unset);
    self->last->type = CE_error;
    self->last->value.error = rc;
    self->count = self->shared->cache.entries;
    RestoreReadSharedWriterDone(self->shared);
    return rc;
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
