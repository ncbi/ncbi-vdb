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
#include <kdb/kdb-priv.h> /* KDBManagerGetVFSManager */
#include <kdb/manager.h>
#include <kdb/meta.h>
#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path-priv.h> /* VPathSetAccOfParentDb */

#include <ctype.h>

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
    return getSchemaName_Table(buffer, &size, tbl) == NULL ? false : schemaNameIsEqual(buffer, size, name);
}

static bool dbSchemaNameIsEqual(VDatabase const *db, char const *name)
{
    char buffer[1024];
    size_t size = sizeof(buffer);
    return getSchemaName_DB(buffer, &size, db) == NULL ? false : schemaNameIsEqual(buffer, size, name);
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

struct RestoreRead {
    VDBManager const *mgr;
    RefSeqList refSeqs;
    WGS_List wgs;
    ErrorList errors;
    struct Last {
        enum { L_none, refSeq_type, wgs_type } type;
        union U {
            RefSeqListEntry *r;
            WGS_ListEntry *w;
        } u;
    } last;
};

void RestoreReadFree(void *const vp)
{
    RestoreRead *const self = (RestoreRead *)vp;

    RefSeqListFree(&self->refSeqs);
    WGS_ListFree(&self->wgs);
    ErrorListFree(&self->errors);
    VDBManagerRelease(self->mgr);

    free(self);
}

RestoreRead *RestoreReadMake(VDBManager const *vmgr, rc_t *rcp)
{
    RestoreRead *self = calloc(1, sizeof(*self));
    self->mgr = vmgr;
    *rcp = VDBManagerAddRef(self->mgr);
    WGS_ListInit(&self->wgs, DEFAULT_WGS_OPEN_LIMIT);
    *rcp = RefSeqListInit(&self->refSeqs);
    return self;
}

static rc_t openSeqID(  RestoreRead *const self
                      , size_t const id_len
                      , size_t const wgs_id_len
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
            self->last.u.r = RefSeqInsert(&self->refSeqs, id_len, seq_id, tbl, &rc);
            if (self->last.u.r)
                self->last.type = refSeq_type;
        }
        else {
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
        }
    }
    else if (db != NULL) {
        if (dbSchemaNameIsEqual(db, WGS_Scheme())) {
            self->last.u.w = WGS_Insert(&self->wgs, wgs_id_len, seq_id, url, db, &rc);
            if (self->last.u.w)
                self->last.type = wgs_type;
        }
        else {
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
        }
    }
    else {
        rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
    }
    VPathRelease(url);
    VTableRelease(tbl);
    VDatabaseRelease(db);
    return rc;
}

rc_t RestoreReadGetSequence(  RestoreRead *const self
                            , unsigned const start
                            , size_t const length, uint8_t *const dst
                            , size_t const id_len, char const *const seq_id
                            , unsigned *const actual
                            , VTable const *const forTable)
{
    unsigned wgs_namelen = 0;
    int64_t wgs_row = 0;
    unsigned error_at = 0;
    rc_t rc = 0;
    unsigned loops;

    for (loops = 0; loops != 2; ++loops) {
        switch (self->last.type) {
        case refSeq_type:
            if (name_cmp(self->last.u.r->name, id_len, seq_id) == 0) {
REFSEQ_FROM_LAST:
                *actual = RefSeq_getBases(&self->last.u.r->object, dst, start, length);
                return 0;
            }
            break;
        case wgs_type:
            wgs_namelen = WGS_splitName(&wgs_row, id_len, seq_id);
            if (wgs_namelen > 0 && name_cmp(self->last.u.w->name, wgs_namelen, seq_id) == 0) {
                assert(self->last.u.w->object.curs != NULL);
WGS_FROM_LAST:
                *actual = WGS_getBases(&self->last.u.w->object, dst, start, length, wgs_row);
                return 0;
            }
            break;
        default:
            break;
        }
        self->last.type = L_none;
        assert(loops == 0);
        if (loops > 0)
            return RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);

        // check error list
        if (Error_Find(&self->errors, &error_at, id_len, seq_id))
            return self->errors.entry[error_at].error;

        // check refSeq list
        if ((self->last.u.r = RefSeqFind(&self->refSeqs, id_len, seq_id)) != NULL) {
            self->last.type = refSeq_type;
            goto REFSEQ_FROM_LAST;
        }

        // check WGS list
        wgs_namelen = WGS_splitName(&wgs_row, id_len, seq_id);
        if (wgs_namelen > 0 && (self->last.u.w = WGS_Find(&self->wgs, wgs_namelen, seq_id)) != NULL) {
            if (self->last.u.w->object.curs == NULL) {
                rc_t rc = 0;

                WGS_limitOpen(&self->wgs);
                rc = WGS_reopen(&self->last.u.w->object, self->mgr, wgs_namelen, seq_id);
                if (rc) return rc;
                ++self->wgs.openCount;
            }
            self->last.type = wgs_type;
            goto WGS_FROM_LAST;
        }

        // it is a new seq_id
        rc = openSeqID(self, id_len, wgs_namelen, seq_id, forTable);
        if (rc) {
            Error_Insert(&self->errors, error_at, id_len, seq_id, rc);
            return rc;
        }
    }
    assert(!"reachable");
    abort();
}
