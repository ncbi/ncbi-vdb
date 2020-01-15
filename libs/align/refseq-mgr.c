/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was readten as part of
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

#include <align/extern.h>

#include <klib/rc.h>
#include <klib/container.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/log.h>

#include <kdb/kdb-priv.h> /* KDBManagerGetVFSManager */
#include <kdb/manager.h>
#include <kdb/meta.h>

#include <kfg/config.h>
#include <insdc/insdc.h>
#include <vdb/cursor.h>
#include <vdb/database.h>
#include <vdb/table.h> /* VDBManagerOpenTableReadVPath */
#include <vdb/vdb-priv.h>

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/path-priv.h> /* VPathSetAccOfParentDb */

#include <align/refseq-mgr.h>
#include <sysalloc.h>

#include "refseq-mgr-priv.h"
#include "reader-wgs.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>

struct RefSeqMgr {
    KConfig *kfg;
    VDBManager const *vmgr;
    RefSeq *mru;
    RefSeq *lru;
    RefSeq **refSeq;
    size_t cache;
    uint32_t reader_options;
    unsigned num_open_max;
    unsigned num_open;
    unsigned nRefSeqs;
    unsigned maxRefSeqs;
};

typedef struct RefSeq_VT RefSeq_VT;

enum {
    refSeqType_RefSeq = 1,
    refSeqType_RefSeq_odd, /* for some weirdos, like hs37d5, that aren't accessioned */
    refSeqType_WGS,
    refSeqType_MAX
};

struct RefSeq_RefSeq {
    TableReaderRefSeq const *reader;
    char name[1];
};

struct RefSeq_WGS {
    TableReaderWGS const *reader;
    int64_t row;
    unsigned prefixLen; /* e.g. AFNW01000001; common prefix is 'AFNW01' */
    char name[1];
};

struct RefSeq {
    RefSeq_VT const *vt;
    RefSeqMgr const *mgr;
    struct RefSeq *newer;
    struct RefSeq *older;
    const String * accOfParentDb;
    union {
        struct RefSeq_RefSeq refSeq;
        struct RefSeq_WGS wgs;
    } u;
};

struct RefSeq_VT {
    RefSeq *(*init)(RefSeq *self, RefSeqMgr const *mgr, unsigned namelen, char const name[]);
    char const *(*name)(RefSeq const *self);
    bool (*isopen)(RefSeq const *self);
    rc_t (*open)(RefSeq *self, RefSeqMgr const *mgr);
    void (*close)(RefSeq *self);
    rc_t (*setRow)(RefSeq *self, unsigned N, char const full[]);
    rc_t (*read)(RefSeq const *self, INSDC_coord_zero offset,
                 INSDC_coord_len len, uint8_t *buffer,
                 INSDC_coord_len *written);
    rc_t (*circular)(RefSeq const *self, bool *result);
    rc_t (*length)(RefSeq const *self, INSDC_coord_len *result);
    rc_t (*checksum)(RefSeq const *self, uint8_t const **result);
    int (*compare)(RefSeq const *self, unsigned qlen, char const qry[]);
};

static RefSeq *RefSeq_RefSeq_init(RefSeq *self,
                                   RefSeqMgr const *mgr,
                                   unsigned namelen,
                                   char const name[]);
static char const *RefSeq_RefSeq_name(RefSeq const *self);
static bool RefSeq_RefSeq_isopen(RefSeq const *self);
static rc_t RefSeq_RefSeq_open(RefSeq *self, RefSeqMgr const *mgr);
static rc_t RefSeq_RefSeq_odd_open(RefSeq *self, RefSeqMgr const *mgr);
static void RefSeq_RefSeq_close(RefSeq *self);
static rc_t RefSeq_RefSeq_setRow(RefSeq *self, unsigned N, char const name[]);
static rc_t RefSeq_RefSeq_read(RefSeq const *self,
                               INSDC_coord_zero offset,
                               INSDC_coord_len len,
                               uint8_t *buffer,
                               INSDC_coord_len *written);
static rc_t RefSeq_RefSeq_circular(RefSeq const *self,
                                   bool *result);
static rc_t RefSeq_RefSeq_length(RefSeq const *self,
                                INSDC_coord_len *result);
static rc_t RefSeq_RefSeq_checksum(RefSeq const *self,
                                   uint8_t const **result);
static int RefSeq_RefSeq_compare(RefSeq const *self, unsigned const qlen,
                                 char const qry[]);

static RefSeq_VT const RefSeq_RefSeq_VT = {
    RefSeq_RefSeq_init,
    RefSeq_RefSeq_name,
    RefSeq_RefSeq_isopen,
    RefSeq_RefSeq_open,
    RefSeq_RefSeq_close,
    RefSeq_RefSeq_setRow,
    RefSeq_RefSeq_read,
    RefSeq_RefSeq_circular,
    RefSeq_RefSeq_length,
    RefSeq_RefSeq_checksum,
    RefSeq_RefSeq_compare
};

static RefSeq_VT const RefSeq_RefSeq_odd_VT = {
    RefSeq_RefSeq_init,
    RefSeq_RefSeq_name,
    RefSeq_RefSeq_isopen,
    RefSeq_RefSeq_odd_open,
    RefSeq_RefSeq_close,
    RefSeq_RefSeq_setRow,
    RefSeq_RefSeq_read,
    RefSeq_RefSeq_circular,
    RefSeq_RefSeq_length,
    RefSeq_RefSeq_checksum,
    RefSeq_RefSeq_compare
};

static RefSeq *RefSeq_WGS_init(RefSeq *self,
                                RefSeqMgr const *mgr,
                                unsigned const namelen,
                                char const name[]);
static char const *RefSeq_WGS_name(RefSeq const *self);
static bool RefSeq_WGS_isopen(RefSeq const *self);
static rc_t RefSeq_WGS_open(RefSeq *self, RefSeqMgr const *mgr);
static void RefSeq_WGS_close(RefSeq *self);
static rc_t RefSeq_WGS_setRow(RefSeq *self, unsigned N, char const name[]);
static rc_t RefSeq_WGS_read(RefSeq const *const self,
                            INSDC_coord_zero const offset,
                            INSDC_coord_len const len,
                            uint8_t *const buffer,
                            INSDC_coord_len *const written);
static rc_t RefSeq_WGS_circular(RefSeq const *const self,
                                bool *const result);
static rc_t RefSeq_WGS_length(RefSeq const *const self,
                              INSDC_coord_len *const result);
static rc_t RefSeq_WGS_checksum(RefSeq const *const self,
                                uint8_t const **const result);
static int RefSeq_WGS_compare(RefSeq const *self, unsigned const qlen,
                              char const qry[]);


static RefSeq_VT const RefSeq_WGS_VT = {
    RefSeq_WGS_init,
    RefSeq_WGS_name,
    RefSeq_WGS_isopen,
    RefSeq_WGS_open,
    RefSeq_WGS_close,
    RefSeq_WGS_setRow,
    RefSeq_WGS_read,
    RefSeq_WGS_circular,
    RefSeq_WGS_length,
    RefSeq_WGS_checksum,
    RefSeq_WGS_compare
};

static RefSeq *RefSeq_RefSeq_alloc(unsigned const namelen)
{
    RefSeq *const self = calloc(1, sizeof(RefSeq) + namelen);
    if (self)
        self->vt = &RefSeq_RefSeq_VT;
    return self;
}

static RefSeq *RefSeq_RefSeq_odd_alloc(unsigned const namelen)
{
    RefSeq *const self = RefSeq_RefSeq_alloc(namelen);
    if (self)
        self->vt = &RefSeq_RefSeq_odd_VT;
    return self;
}

static RefSeq *RefSeq_WGS_alloc(unsigned const namelen)
{
    RefSeq *const self = calloc(1, sizeof(RefSeq) + namelen);
    if (self)
        self->vt = &RefSeq_WGS_VT;
    return self;
}

static RefSeq *RefSeq_RefSeq_init(RefSeq *const super,
                                   RefSeqMgr const *const mgr,
                                   unsigned const namelen,
                                   char const name[])
{
    struct RefSeq_RefSeq *const self = &super->u.refSeq;

    memmove(self->name, name, namelen);
    self->name[namelen] = '\0';
    super->mgr = mgr;

    return super;
}

static RefSeq *RefSeq_WGS_init(RefSeq *const super,
                                RefSeqMgr const *const mgr,
                                unsigned const namelen,
                                char const name[])
{
    struct RefSeq_WGS *const self = &super->u.wgs;
    unsigned prefixLen = 0;
    unsigned digits = 0;

    while (prefixLen < namelen && digits < 2) {
        int const ch = name[prefixLen];
        if (isdigit(ch))
            ++digits;
        else
            digits = 0;
        ++prefixLen;
    }
    assert(digits == 2);
    assert(prefixLen > 0);
    assert(prefixLen < namelen);

    memmove(self->name, name, namelen);
    self->name[prefixLen] = '\0';
    self->prefixLen = prefixLen;
    super->mgr = mgr;

    return super;
}

static char const *RefSeq_RefSeq_name(RefSeq const *super)
{
    return super->u.refSeq.name;
}

static char const *RefSeq_WGS_name(RefSeq const *super)
{
    return super->u.wgs.name;
}

static bool RefSeq_RefSeq_isopen(RefSeq const *const super)
{
    return (super->u.refSeq.reader == NULL) ? false : true;
}

static bool RefSeq_WGS_isopen(RefSeq const *const super)
{
    return (super->u.wgs.reader == NULL) ? false : true;
}

static void RefSeq_RefSeq_close(RefSeq *const super)
{
    TableReaderRefSeq const *const reader = super->u.refSeq.reader;

    super->u.refSeq.reader = NULL;
    TableReaderRefSeq_Whack(reader);
}

static void RefSeq_WGS_close(RefSeq *const super)
{
    TableReaderWGS const *const reader = super->u.wgs.reader;

    super->u.wgs.reader = NULL;
    TableReaderWGS_Whack(reader);
}

static rc_t RefSeq_RefSeq_setRow(RefSeq *const super,
                                 unsigned const N, char const name[])
{
    return 0;
}

static rc_t RefSeq_WGS_setRow(RefSeq *const super,
                              unsigned const N, char const name[])
{
    struct RefSeq_WGS *const self = &super->u.wgs;
    unsigned i;
    int64_t row = 0;

    assert(strncmp(name, self->name, self->prefixLen) == 0);
    for (i = self->prefixLen; i < N; ++i) {
        int const ch = name[i];

        if (isdigit(ch))
            row = (row * 10) + (ch - '0');
        else if (ch == '.')
            break;
        else {
            row = 0;
            break;
        }
    }
    if (row) {
        self->row = row;
        return 0;
    }
    return RC(rcAlign, rcTable, rcAccessing, rcRow, rcInvalid);
}

static rc_t RefSeq_RefSeq_read(RefSeq const *const super,
                               INSDC_coord_zero const offset,
                               INSDC_coord_len const length,
                               uint8_t *const buffer,
                               INSDC_coord_len *const written)
{
    return TableReaderRefSeq_Read(super->u.refSeq.reader, offset, length, buffer, written);
}

static rc_t RefSeq_WGS_read(RefSeq const *const super,
                            INSDC_coord_zero const offset,
                            INSDC_coord_len const length,
                            uint8_t *const buffer,
                            INSDC_coord_len *const written)
{
    return TableReaderWGS_Read(super->u.wgs.reader, super->u.wgs.row, offset, length, buffer, written);
}

static rc_t RefSeq_RefSeq_circular(RefSeq const *const super, bool *const result)
{
    return TableReaderRefSeq_Circular(super->u.refSeq.reader, result);
}

static rc_t RefSeq_WGS_circular(RefSeq const *const super, bool *const result)
{
    return TableReaderWGS_Circular(super->u.wgs.reader, super->u.wgs.row, result);
}

static rc_t RefSeq_RefSeq_length(RefSeq const *const super, INSDC_coord_len *const result)
{
    return TableReaderRefSeq_SeqLength(super->u.refSeq.reader, result);
}

static rc_t RefSeq_WGS_length(RefSeq const *const super, INSDC_coord_len *const result)
{
    return TableReaderWGS_SeqLength(super->u.wgs.reader, super->u.wgs.row, result);
}

static rc_t RefSeq_RefSeq_checksum(RefSeq const *const super, uint8_t const **const result)
{
    return TableReaderRefSeq_MD5(super->u.refSeq.reader, result);
}

static rc_t RefSeq_WGS_checksum(RefSeq const *const super, uint8_t const **const result)
{
    return TableReaderWGS_MD5(super->u.wgs.reader, super->u.wgs.row, result);
}

static int RefSeq_RefSeq_compare(RefSeq const *super,
                                 unsigned const qlen, char const qry[])
{
    char const *const fnd = super->u.refSeq.name;
    unsigned i;

    for (i = 0; ; ++i) {
        int const a = i == qlen ? '\0' : qry[i];
        int const b = fnd[i];

        if (a < b)
            return -1;

        if (b < a)
            return 1;

        if (a == 0)
            break;
    }
    return 0;
}

static int RefSeq_WGS_compare(RefSeq const *super, unsigned const qlen, char const qry[])
{
    struct RefSeq_WGS const *const self = &super->u.wgs;
    char const *const fnd = self->name;
    unsigned const prefixLen = self->prefixLen;
    unsigned i;

    for (i = 0; i < prefixLen && i < qlen; ++i) {
        int const a = qry[i];
        int const b = fnd[i];
        int const diff = a - b;

        if (diff != 0)
            return diff;
    }
    return i == prefixLen ? 0 : qry[i];
}

static unsigned FindAccession(unsigned const N,
                              RefSeq const * const *const refSeq,
                              unsigned const qlen, char const qry[],
                              bool *matched)
{
    unsigned f = 0;
    unsigned e = N;

    while (f < e) {
        unsigned const m = f + ((e - f) >> 1);
        int const diff = refSeq[m]->vt->compare(refSeq[m], qlen, qry);

        if (diff == 0) {
            *matched = true;
            return m;
        }
        if (diff < 0)
            e = m;
        else
            f = m + 1;
    }
    *matched = false;
    return f;
}

static rc_t get_schema_info(KMetadata const *const meta,
                            unsigned const bsz, char buffer[])
{
    KMDataNode const *node;
    rc_t rc = KMetadataOpenNodeRead(meta, &node, "schema");

    if (rc == 0) {
        size_t sz;

        rc = KMDataNodeReadAttr(node, "name", buffer, bsz - 1, &sz);
        KMDataNodeRelease(node);
        if (rc == 0) {
            buffer[sz] = '\0';
            while (sz) {
                --sz;
                if (buffer[sz] == '#') {
                    buffer[sz] = '\0';
                    break;
                }
            }
        }
    }
    return rc;
}

static rc_t get_tbl_schema_info(VTable const *const tbl,
                                unsigned const bsz, char buffer[])
{
    KMetadata const *meta;
    rc_t rc = VTableOpenMetadataRead(tbl, &meta);

    buffer[0] = '\0';
    if (rc == 0) rc = get_schema_info(meta, bsz, buffer);
    KMetadataRelease(meta);
    return rc;
}

static rc_t get_db_schema_info(VDatabase const *db,
                               unsigned const bsz, char buffer[])
{
    KMetadata const *meta;
    rc_t rc = VDatabaseOpenMetadataRead(db, &meta);

    buffer[0] = '\0';
    if (rc == 0) rc = get_schema_info(meta, bsz, buffer);
    KMetadataRelease(meta);
    return rc;
}

static rc_t RefSeq_RefSeq_open(RefSeq *const super, RefSeqMgr const *const mgr)
{
    rc_t rc = 0;

    struct RefSeq_RefSeq *const self = &super->u.refSeq;

    VTable const *tbl = NULL;
    const KDBManager * kmgr = NULL;
    VFSManager * vfs;
    VPath * aOrig = NULL;

    rc = VDBManagerGetKDBManagerRead(mgr->vmgr, &kmgr);
    if (rc == 0)
        rc = KDBManagerGetVFSManager(kmgr, &vfs);
    if (rc == 0)
        rc = VFSManagerMakePath(vfs, &aOrig, "%s", self->name);
    if (rc == 0)
        VPathSetAccOfParentDb(aOrig, super->accOfParentDb);
    if (rc == 0)
        rc = VDBManagerOpenTableReadVPath(mgr->vmgr, &tbl, NULL, aOrig);

    if (tbl) {
        char scheme[1024];

        get_tbl_schema_info(tbl, sizeof(scheme), scheme);
        if (strcmp(scheme, "NCBI:refseq:tbl:reference") == 0) {
            rc = TableReaderRefSeq_MakeTable(&self->reader, mgr->vmgr, tbl,
                                             mgr->reader_options, mgr->cache);
        }
        else {
            rc = RC(rcAlign, rcTable, rcOpening, rcType, rcInvalid);
        }
        VTableRelease(tbl);
    }

    VPathRelease(aOrig);
    VFSManagerRelease(vfs);
    KDBManagerRelease(kmgr);

    return rc;
}

static rc_t RefSeq_RefSeq_odd_open(RefSeq *const super, RefSeqMgr const *const mgr)
{
    struct RefSeq_RefSeq *const self = &super->u.refSeq;
    VTable const *tbl;
    rc_t rc = VDBManagerOpenTableRead(mgr->vmgr, &tbl, NULL, "ncbi-acc:%s?vdb-ctx=refseq", self->name);

    if (tbl) {
        char scheme[1024];

        get_tbl_schema_info(tbl, sizeof(scheme), scheme);
        if (strcmp(scheme, "NCBI:refseq:tbl:reference") == 0) {
            rc = TableReaderRefSeq_MakeTable(&self->reader, mgr->vmgr, tbl,
                                             mgr->reader_options, mgr->cache);
        }
        else {
            rc = RC(rcAlign, rcTable, rcOpening, rcType, rcInvalid);
        }
        VTableRelease(tbl);
    }
    return rc;
}

static rc_t RefSeq_WGS_open(RefSeq *const super, RefSeqMgr const *const mgr)
{
    struct RefSeq_WGS *const self = &super->u.wgs;
    VDatabase const *db;
    rc_t rc = VDBManagerOpenDBRead(mgr->vmgr, &db, NULL, self->name);

    if (db) {
        char scheme[1024];

        get_db_schema_info(db, sizeof(scheme), scheme);
        if (strcmp(scheme, "NCBI:WGS:db:contig") == 0) {
            VTable const *tbl;

            rc = VDatabaseOpenTableRead(db, &tbl, "SEQUENCE");
            if (tbl) {
                rc = TableReaderWGS_MakeTable(&self->reader, mgr->vmgr, tbl,
                                                 mgr->reader_options, mgr->cache);
                VTableRelease(tbl);
            }
        }
        else {
            rc = RC(rcAlign, rcTable, rcOpening, rcType, rcInvalid);
        }
        VDatabaseRelease(db);
    }
    return rc;
}

static int AccessionType(VDBManager const *const mgr,
                         unsigned const N, char const accession[],
                         rc_t *const rc,
                         const String * accOfParentDb)
{
    char scheme[1024];
    bool isOdd = false;

    assert(rc);

    scheme[0] = '\0';
    {
        KMetadata const *meta = NULL;
        {
            const KDBManager * kmgr = NULL;
            /* need VFS manager to make a path */
            VFSManager * vfs;
            VDatabase const *db = NULL;
            VPath * aOrig = NULL;
            *rc = VDBManagerGetKDBManagerRead(mgr, &kmgr);
            if (*rc == 0)
                *rc = KDBManagerGetVFSManager(kmgr, &vfs);
            if (*rc == 0)
                *rc = VFSManagerMakePath(vfs, &aOrig, "%.*s", (int)N, accession);
            if (*rc == 0)
                VPathSetAccOfParentDb(aOrig, accOfParentDb);

            if (*rc == 0)
                *rc = VDBManagerOpenDBReadVPath(mgr, &db, NULL, aOrig);
            else
                *rc = VDBManagerOpenDBRead(mgr, &db, NULL, "%.*s", (int)N, accession);

            if (db) {
                *rc = VDatabaseOpenMetadataRead(db, &meta);
                VDatabaseRelease(db);
            }
            else {
                VTable const *tbl = NULL;

                *rc = VDBManagerOpenTableReadVPath(mgr, &tbl, NULL, aOrig);
                if (tbl) {
                    *rc = VTableOpenMetadataRead(tbl, &meta);
                    VTableRelease(tbl);
                }
                else {
                    isOdd = true;
                    if (aOrig == NULL)
                        *rc = VDBManagerOpenTableRead(mgr, &tbl, NULL,
                            "ncbi-acc:%.*s?vdb-ctx=refseq", (int)N, accession);
                    else
                        *rc = VDBManagerOpenTableReadVPath(mgr, &tbl, NULL, aOrig);
                    if (tbl) {
                        *rc = VTableOpenMetadataRead(tbl, &meta);
                        VTableRelease(tbl);
                    }
                }
            }
            VPathRelease(aOrig);
            VFSManagerRelease(vfs);
            KDBManagerRelease(kmgr);
        }
        if (meta) {
            KMDataNode const *node = NULL;

            *rc = KMetadataOpenNodeRead(meta, &node, "schema");
            KMetadataRelease(meta);
            if (node) {
                size_t sz = 0;

                *rc = KMDataNodeReadAttr(node, "name", scheme, sizeof(scheme) - 1, &sz);
                KMDataNodeRelease(node);
                scheme[sz] = '\0';
                while (sz) {
                    --sz;
                    if (scheme[sz] == '#') {
                        scheme[sz] = '\0';
                        break;
                    }
                }
            }
        }
    }
    if (strcmp(scheme, "NCBI:WGS:db:contig") == 0)
        return refSeqType_WGS;
    if (strcmp(scheme, "NCBI:refseq:tbl:reference") == 0)
        return isOdd ? refSeqType_RefSeq_odd : refSeqType_RefSeq;
    return 0;
}

static void WhackAllReaders(RefSeqMgr *const mgr);

LIB_EXPORT rc_t CC RefSeqMgr_SetCache(RefSeqMgr const *const cself, size_t cache, uint32_t keep_open_num)
{
    if (cself) {
        RefSeqMgr *const self = (RefSeqMgr *)cself;

        WhackAllReaders(self);
        assert(self->num_open == 0);
        self->cache = cache;
        self->num_open_max = keep_open_num;
    }
    return 0;
}

LIB_EXPORT rc_t CC RefSeqMgr_Make( const RefSeqMgr** cself, const VDBManager* vmgr,
                                   uint32_t reader_options, size_t cache, uint32_t keep_open_num )
{
    rc_t rc = 0;
    RefSeqMgr* obj = NULL;

    if ( cself == NULL || vmgr == NULL )
    {
        rc = RC( rcAlign, rcIndex, rcConstructing, rcParam, rcNull );
    }
    else
    {
        obj = calloc( 1, sizeof( *obj ) );
        if ( obj == NULL )
        {
            rc = RC( rcAlign, rcIndex, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            rc = KConfigMake( &obj->kfg, NULL );
            if ( rc == 0 )
            {
                rc = VDBManagerAddRef( vmgr );
                if ( rc == 0 )
                {
                    obj->vmgr = vmgr;
                    obj->cache = cache;
                    obj->num_open_max = keep_open_num;
                    obj->reader_options = reader_options;
                }
            }
        }
    }

    if ( rc == 0 )
    {
        *cself = obj;
/*        ALIGN_DBG( "max_open: %u", obj->num_open_max ); */
    }
    else
    {
        RefSeqMgr_Release( obj );
        ALIGN_DBGERR( rc );
    }
    return rc;
}

void RefseqWhack(RefSeq * self) {
    assert(self);

    StringWhack(self->accOfParentDb);
    memset(self, 0, sizeof *self);

    free(self);
}

LIB_EXPORT rc_t CC RefSeqMgr_Release(const RefSeqMgr* cself)
{
    if( cself != NULL ) {
        RefSeqMgr* self = (RefSeqMgr*)cself;
        unsigned i;

        WhackAllReaders(self);
        for (i = 0; i < self->nRefSeqs; ++i)
            RefseqWhack(self->refSeq[i]);
        free(self->refSeq);
        VDBManagerRelease(self->vmgr);
        KConfigRelease(self->kfg);
        free(self);
    }
    return 0;
}

static rc_t NewRefSeq(RefSeqMgr *const self,
                                int const type,
                                unsigned const at,
                                unsigned const N,
                                char const accession[],
                                const String * accOfParentDb)
{
    rc_t rc = 0;

    if (self->nRefSeqs + 1 > self->maxRefSeqs) {
        unsigned const maxRefSeqs = (self->maxRefSeqs == 0) ? 32 : (self->maxRefSeqs << 1);
        void *tmp = realloc(self->refSeq, maxRefSeqs * sizeof(self->refSeq[0]));

        if (tmp == NULL)
            return RC(rcAlign, rcTable, rcAccessing, rcMemory, rcExhausted);

        self->maxRefSeqs = maxRefSeqs;
        self->refSeq = tmp;
    }
    ALIGN_CF_DBG("Inserting '%.*s' at %u", N, accession, at);
    memmove(&self->refSeq[at + 1], &self->refSeq[at], sizeof(self->refSeq[0]) * (self->nRefSeqs - at));
    ++self->nRefSeqs;
    {
        RefSeq *rs = NULL;

        switch (type) {
        case refSeqType_RefSeq:
            rs = RefSeq_RefSeq_alloc(N);
            break;
        case refSeqType_RefSeq_odd:
            rs = RefSeq_RefSeq_odd_alloc(N);
            break;
        case refSeqType_WGS:
            rs = RefSeq_WGS_alloc(N);
            break;
        default:
            assert("unknown type of RefSeq object");
            break;
        }
        self->refSeq[at] = rs;

        if (rs == NULL)
            return RC(rcAlign, rcTable, rcAccessing, rcMemory, rcExhausted);

        rs->vt->init(rs, self, N, accession);
        if (accOfParentDb != NULL)
            rc = StringCopy(&rs->accOfParentDb, accOfParentDb);
    }

    return rc;
}

static rc_t exists(RefSeqMgr *const self, unsigned const N, char const accession[])
{
    rc_t rc = 0;
    bool matched = false;
    unsigned const at = FindAccession(self->nRefSeqs,
                                      (RefSeq const **)self->refSeq,
                                      N, accession, &matched);
    if (matched)
        return 0;
    {
        int const type = AccessionType(self->vmgr, N, accession, &rc, NULL);
        if (type)
            rc = 0;
        else if (rc == 0)
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
        if (rc == 0)
            rc = NewRefSeq(self, type, at, N, accession, NULL);
        else {
            ALIGN_CF_DBG("failed to open %.*s", N, accession);
            ALIGN_DBGERR(rc);
        }
    }
    return rc;
}

static void MakeNewest(RefSeqMgr *const self, RefSeq *const rs)
{
    if (self->mru == rs) /* is already newest */
        return;

    {   /* unlink rs from list */
        RefSeq *const older = rs->older;
        RefSeq *const newer = rs->newer;

        if (older) older->newer = newer;
        if (newer) newer->older = older;
    }
    /* make rs be head of list */
    {
        RefSeq *const prev = self->mru;

        if (prev) prev->newer = rs;
        rs->older = prev;
        rs->newer = NULL;
    }
    self->mru = rs;
    if (self->lru == NULL)
        self->lru = rs;
}

static rc_t GetSeq(RefSeqMgr *const self, RefSeq **result,
            unsigned const N, char const accession[], const String * accOfParentDb)
{
    rc_t rc = 0;
    bool matched = false;
    unsigned const at = FindAccession(self->nRefSeqs,
                                      (RefSeq const **)self->refSeq,
                                      N, accession, &matched);

    if (!matched) {
        int const type = AccessionType(self->vmgr, N, accession, &rc, accOfParentDb);
        if (type)
            rc = 0;
        else if (rc == 0)
            rc = RC(rcAlign, rcTable, rcAccessing, rcType, rcUnexpected);
        if (rc == 0)
            rc = NewRefSeq(self, type, at, N, accession, accOfParentDb);
        else {
            ALIGN_CF_DBG("failed to open %.*s", N, accession);
            ALIGN_DBGERR(rc);
        }
    }
    if (rc)
        return rc;
    {
        RefSeq *const fnd = self->refSeq[at];

        *result = fnd;
    }
    return 0;
}

LIB_EXPORT rc_t RefSeqMgr_Exists(const RefSeqMgr* cself, const char* accession, uint32_t accession_sz, char** path)
{
    if (cself == NULL || accession == NULL || accession_sz == 0) {
        return RC(rcAlign, rcIndex, rcAccessing, rcParam, rcNull);
    }
    else {
        return exists((RefSeqMgr *)cself, accession_sz, accession);
    }
}

static rc_t RefSeqMgr_GetSeqForDb(RefSeqMgr const *const cmgr,
    RefSeq const **result, char const *seq_id, uint32_t seq_id_sz,
    const String * accOfParentDb);

LIB_EXPORT rc_t CC RefSeqMgr_ReadForDb(const RefSeqMgr* cself, const char* seq_id,
    uint32_t seq_id_sz, INSDC_coord_zero offset, INSDC_coord_len len,
    uint8_t* buffer, INSDC_coord_len* written, const String * accOfParentDb)
{
    RefSeq const *obj;
    rc_t rc = 0;

    if( (rc = RefSeqMgr_GetSeqForDb(cself, &obj, seq_id, seq_id_sz, accOfParentDb))
        == 0 )
    {
        rc = RefSeq_Read(obj, offset, len, buffer, written);
        RefSeq_Release(obj);
    }
    return rc;
}

LIB_EXPORT rc_t CC RefSeqMgr_Read(const RefSeqMgr* cself, const char* seq_id, uint32_t seq_id_sz,
                                  INSDC_coord_zero offset, INSDC_coord_len len,
                                  uint8_t* buffer, INSDC_coord_len* written)
{
    return RefSeqMgr_ReadForDb(cself, seq_id,
        seq_id_sz, offset, len, buffer, written, NULL);
}

static void WhackAllReaders(RefSeqMgr *const self)
{
    unsigned i;

    for (i = 0; i < self->nRefSeqs; ++i) {
        RefSeq *const rs = self->refSeq[i];

        if (rs->vt->isopen(rs)) {
            ALIGN_CF_DBG("closing %s", rs->vt->name(rs));
            rs->vt->close(rs);
            --self->num_open;
        }
        rs->newer = rs->older = NULL;
    }
    self->mru = self->lru = NULL;
}

static rc_t GetReader(RefSeqMgr *const self, RefSeq *const obj)
{
    if (obj->vt->isopen(obj))
        return 0;
    {
        unsigned const max_open = self->num_open_max;

        while (max_open > 0 && self->num_open >= max_open) {
            RefSeq *const lru = self->lru;

            assert(lru);
            ALIGN_CF_DBG("closing %s", lru->vt->name(lru));
            self->lru = lru->newer;
            --self->num_open;
            lru->vt->close(lru);
            lru->newer = lru->older = NULL;
        }
    }
    {
        rc_t const rc = obj->vt->open(obj, self);

        if (rc) return rc;
    }
    MakeNewest(self, obj);
    ++self->num_open;
    ALIGN_CF_DBG("opened %s", obj->vt->name(obj));

    return 0;
}

static rc_t GetSeqInternal(RefSeqMgr *const self,
                              RefSeq const **const result,
                              unsigned const seq_id_sz,
                              char const seq_id[],
                              const String * accOfParentDb)
{
    RefSeq *obj = NULL;

    if (self->mru == NULL || self->mru->vt->compare(self->mru, seq_id_sz, seq_id) != 0) {
        rc_t const rc = GetSeq(self, &obj, seq_id_sz, seq_id, accOfParentDb);
        if (rc)
            return rc;
    }
    else
        obj = self->mru;
    {
        rc_t const rc = GetReader(self, obj);
        if (rc)
            return rc;
    }
    obj->vt->setRow(obj, seq_id_sz, seq_id);
    *result = obj;
    return 0;
}

static rc_t RefSeqMgr_GetSeqForDb  (RefSeqMgr const *const cmgr,
                                    RefSeq const **result,
                                    char const *seq_id,
                                    uint32_t seq_id_sz,
                                    const String * accOfParentDb)
{
    rc_t rc;

    if (cmgr == NULL || result == NULL || seq_id == NULL)
        rc = RC(rcAlign, rcIndex, rcConstructing, rcParam, rcNull);
    else
        rc = GetSeqInternal((RefSeqMgr *)cmgr, result, seq_id_sz, seq_id,
            accOfParentDb);

    if (rc)
        ALIGN_DBGERRP("SEQ_ID: '%.*s'", rc, seq_id_sz, seq_id);

    return rc;
}

LIB_EXPORT rc_t CC RefSeqMgr_GetSeq(RefSeqMgr const *const cmgr,
                                    RefSeq const **result,
                                    char const *seq_id,
                                    uint32_t seq_id_sz)
{
    return RefSeqMgr_GetSeqForDb(cmgr, result, seq_id, seq_id_sz, NULL);
}

LIB_EXPORT rc_t CC RefSeq_Read(const RefSeq* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                               uint8_t* buffer, INSDC_coord_len* written)
{
    rc_t rc = 0;

    if (cself == NULL || buffer == NULL || written == NULL)
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    else {
        RefSeq *const self = (RefSeq *)cself;
        RefSeqMgr *const mgr = (RefSeqMgr *)self->mgr;

        rc = GetReader(mgr, self);
        if (rc == 0)
            rc = self->vt->read(self, offset, len, buffer, written);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_Circular(const RefSeq* cself, bool* circular)
{
    rc_t rc = 0;

    if (cself == NULL)
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    else {
        RefSeq const *const self = (RefSeq *)cself;

        rc = self->vt->circular(self, circular);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_SeqLength(const RefSeq* cself, INSDC_coord_len* len)
{
    rc_t rc = 0;

    if (cself == NULL)
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    else {
        RefSeq const *const self = (RefSeq *)cself;

        rc = self->vt->length(self, len);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_MD5(const RefSeq* cself, const uint8_t** md5)
{
    rc_t rc = 0;

    if (cself == NULL)
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    else {
        RefSeq const *const self = (RefSeq *)cself;

        rc = self->vt->checksum(self, md5);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_Name(const RefSeq* cself, const char** name)
{
    rc_t rc = 0;

    if (cself == NULL)
        rc = RC(rcAlign, rcFile, rcReading, rcParam, rcNull);
    else {
        RefSeq const *const self = (RefSeq *)cself;

        *name = self->vt->name(self);
    }
    ALIGN_DBGERR(rc);
    return rc;
}

LIB_EXPORT rc_t CC RefSeq_Release(const RefSeq* cself)
{
    return 0;
}

static
rc_t RefSeqMgr_ConfigValue ( const KConfig *kfg, const char *node_path, char *value, size_t value_size )
{
    const KConfigNode *node;
    rc_t rc = KConfigOpenNodeRead ( kfg, & node, "%s", node_path );
    if ( rc == 0 )
    {
        size_t num_read, remaining;
        rc = KConfigNodeRead ( node, 0, value, value_size - 1, & num_read,  & remaining );
        if ( rc == 0 )
        {
            if ( remaining != 0 )
                rc = RC ( rcSRA, rcMgr, rcConstructing, rcString, rcExcessive );
            else
                value [ num_read ] = 0;
        }

        KConfigNodeRelease ( node );
    }
    return rc;
}

static
rc_t RefSeqMgr_KfgReadRepositories(const KConfig* kfg, char* paths, size_t paths_sz)
{
    /* servers are children of refseq/repository, e.g.:             /refseq/repository/main="..." */
    /* volumes are in refseq/repository/<serverName>/volumes, e.g.: /refseq/repository/main/volumes="..." */
    /* all server/volume combinations are returned in paths separated by ':' */

    rc_t rc = 0;
    const KConfigNode *node;
#define KFG_PATH "/refseq/repository/"
    paths[0] = 0;

    rc = KConfigOpenNodeRead ( kfg, & node, KFG_PATH );
    if ( rc == 0 )
    {
        KNamelist* children;
        rc = KConfigNodeListChild ( node, &children );
        if ( rc == 0 )
        {
            uint32_t count;
            rc = KNamelistCount ( children, &count );
            if ( rc == 0 )
            {
                uint32_t i;
                for (i = 0; i < count; ++i) /* for all servers */
                {
                    const char* name;
                    rc = KNamelistGet ( children, i, &name );
                    if ( rc == 0 )
                    {
#define BufSize 4096
                        char server[ BufSize ];
                        char buf[ BufSize ];
                        size_t num_writ;

                        rc = string_printf(buf, BufSize, &num_writ, KFG_PATH "%s", name);
                        if (rc == 0)
                        {
                            rc = RefSeqMgr_ConfigValue ( kfg, buf, server, sizeof(server) );
                            if (rc == 0)
                            {
                                rc = string_printf(buf, BufSize, &num_writ, KFG_PATH "%s/volumes", name);
                                if (rc == 0)
                                {
                                    char volumes[ BufSize ];
                                    rc = RefSeqMgr_ConfigValue ( kfg, buf, volumes, sizeof(volumes) );
                                    if (rc == 0)
                                    {   /* create a server/volume pair for every combination, append to paths, ':' - separate */
                                        char *vol_rem = volumes;
                                        char *vol_sep;

                                        do {
                                            char const *volume = vol_rem;
                                            vol_sep = string_chr(volume, string_size(volume), ':');
                                            if(vol_sep) {
                                                vol_rem = vol_sep + 1;
                                                *vol_sep = 0;
                                            }
                                            string_copy(paths + string_size(paths), paths_sz - string_size(paths), server, string_size(server));
                                            if (paths[string_size(paths)-1] != '/')
                                            {
                                                string_copy(paths + string_size(paths), paths_sz - string_size(paths), "/", 1);
                                            }
                                            string_copy(paths + string_size(paths), paths_sz - string_size(paths), volume, string_size(volume));
                                            string_copy(paths + string_size(paths), paths_sz - string_size(paths), ":", 1);
                                        } while(vol_sep);
                                    }
                                }
                            }
                        }
#undef BufSize
                    }
                    if ( rc != 0 )
                    {
                        break;
                    }
                }
            }
            KNamelistRelease ( children );
        }

        KConfigNodeRelease ( node );
    }
    if (GetRCState(rc) == rcNotFound)
    {
        paths[0] = '\0';
        return 0;
    }
    return 0;
}

static
rc_t RefSeqMgr_KfgReadStr(const KConfig* kfg, const char* path, char* value, size_t value_sz)
{
    rc_t rc = 0;
    const KConfigNode *node;

    if ( (rc = KConfigOpenNodeRead(kfg, &node, "%s", path)) == 0 ) {
        size_t num_read, remaining;
        if( (rc = KConfigNodeRead(node, 0, value, value_sz - 1, &num_read, &remaining)) == 0 ) {
            if( remaining != 0 ) {
                rc = RC(rcAlign, rcIndex, rcConstructing, rcString, rcTooLong);
            } else {
                value[num_read] = '\0';
            }
        }
        KConfigNodeRelease(node);
    } else if( GetRCState(rc) == rcNotFound ) {
        rc = 0;
        value[0] = '\0';
    }
    return rc;
}

rc_t RefSeqMgr_ForEachVolume(const RefSeqMgr* cself, RefSeqMgr_ForEachVolume_callback cb, void *data)
{
    rc_t rc = 0;
    char servers[4096];
    char volumes[4096];

    if( cself == NULL || cb == NULL ) {
        rc = RC(rcAlign, rcType, rcConstructing, rcParam, rcNull);
    } else if( cb(".", NULL, data) ) {
        /* found in local dir */
    } else if( (rc = RefSeqMgr_KfgReadStr(cself->kfg, "refseq/paths", servers, sizeof(servers))) != 0 ) {
        ALIGN_DBGERRP("%s", rc, "RefSeqMgr_KfgReadStr(paths)");
    } else {
        bool found = false;
        if( servers[0] != '\0' ) {
            char *srv_sep;
            char *srv_rem = servers;
            do {
                char const* server = srv_rem;

                srv_sep = strchr(server, ':');
                if(srv_sep) {
                    srv_rem = srv_sep + 1;
                    *srv_sep = 0;
                }
                if( cb(server, NULL, data) ) {
                    found = true;
                    break;
                }
            } while(srv_sep);
        }
        if( !found ) {
            /* locate refseq servers/volumes in possibly multiple repositories */
            if( (rc = RefSeqMgr_KfgReadRepositories(cself->kfg, servers, sizeof(servers))) != 0 ) {
                ALIGN_DBGERRP("%s", rc, "RefSeqMgr_KfgReadStr(refseq/repository/*)");
            };
            if( servers[0] != '\0' ) {
                char *srv_sep;
                char *srv_rem = servers;
                do {
                    char const* server = srv_rem;

                    srv_sep = strchr(server, ':');
                    if(srv_sep) {
                        srv_rem = srv_sep + 1;
                        *srv_sep = 0;
                    }
                    if( cb(server, NULL, data) ) {
                        found = true;
                        break;
                    }
                } while(srv_sep);
            }
        }
        if( !found ) {
            if ( (rc = RefSeqMgr_KfgReadStr(cself->kfg, "refseq/servers", servers, sizeof(servers))) != 0 ||
                (rc = RefSeqMgr_KfgReadStr(cself->kfg, "refseq/volumes", volumes, sizeof(volumes))) != 0 ) {
                ALIGN_DBGERRP("%s", rc, "RefSeqMgr_KfgReadStr(servers/volumes)");
            }
            /* servers and volumes are deprecated and optional */
            if( rc == 0 && (servers[0] != '\0' || volumes[0] != '\0') ) {
                char *srv_sep;
                char *srv_rem = servers;
                do {
                    char vol[ 4096 ];
                    char const *server = srv_rem;
                    char *vol_rem = vol;
                    char *vol_sep;

                    string_copy ( vol, sizeof vol, volumes, string_size( volumes ) );
                    srv_sep = strchr(server, ':');
                    if(srv_sep) {
                        srv_rem = srv_sep + 1;
                        *srv_sep = 0;
                    }
                    do {
                        char const *volume = vol_rem;

                        vol_sep = strchr(volume, ':');
                        if(vol_sep) {
                            vol_rem = vol_sep + 1;
                            *vol_sep = 0;
                        }
                        found = cb(server, volume, data);
                    } while(!found && vol_sep);
                } while(!found && srv_sep);
            }
        }
    }
    return rc;
}
