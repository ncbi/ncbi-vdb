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

#include "reference.h" /* _ReferencesWhack */

#include "blast-mgr.h" /* BTableType */
#include "reader.h" /* Data2na */
#include "run-set.h" /* Core4na */

#include <klib/log.h> /* PLOGMSG */
#include <klib/rc.h> /* SILENT_RC */
#include <klib/status.h> /* STSMSG */

#include <vdb/blob.h> /* VBlob */
#include <vdb/cursor.h> /* VCursor */
#include <vdb/table.h> /* VTable */

#include <string.h> /* strcmp */

/******************************************************************************/

#define MAX_SEQ_LEN 5000

/******************************************************************************/

#define MAX_BIT64 (~((uint64_t)-1 >> 1))

static bool _is_set_read_id_reference_bit(uint64_t read_id) {
    return ( read_id & MAX_BIT64 ) == 0 ? false : true;
}

static
uint64_t _clear_read_id_reference_bit(uint64_t read_id, bool *bad)
{
    assert(bad);

    *bad = false;

    if (! _is_set_read_id_reference_bit(read_id)) {
        *bad = true;
        S
        return read_id;
    }

    return read_id & ~MAX_BIT64;
}

static uint64_t _set_read_id_reference_bit
    (uint64_t read_id, VdbBlastStatus *status)
{
    assert(status);

    if (_is_set_read_id_reference_bit(read_id)) {
        *status = eVdbBlastErr;
        S
        return read_id;
    }

    return read_id | MAX_BIT64;
}

/******************************************************************************/
typedef struct {
    BSTNode n;

    const char *str;
    uint32_t rfdi;
} BstNode;
static void BstNodeInit(BstNode *self, const char *str, uint32_t rfdi) {
    assert(self && str);

    self->str  = str;
    self->rfdi = rfdi;
}

static void CC BstNodeWhack(BSTNode *n, void *ignore) {
    memset(n, 0, sizeof *n);

    free(n);
}

static int64_t CC BstNodeCmpStr(const void *item, const BSTNode *n) {
    const char *c = item;
    const BstNode *rn = (const BstNode*)n;

    if (c == NULL || rn == NULL || rn->str == NULL) {
        return 1;
    }

    return strcmp(c, rn->str);
}

static int64_t CC BstNodeCmpString(const void *item, const BSTNode *n) {
    const String *c = item;
    const BstNode *rn = (const BstNode*)n;

    if (c == NULL || rn == NULL || rn->str == NULL) {
        return 1;
    }

    return string_cmp(c->addr, c->size, rn->str, string_size(rn->str),
                                                 string_size(rn->str));
}

static int64_t CC RunBstSort(const BSTNode *item, const BSTNode *n) {
    const BstNode *rn = (const BstNode*)item;

    assert(rn);

    return BstNodeCmpStr(rn->str, n);
}

/******************************************************************************/
struct VdbBlastRef {
    uint32_t iRun;  /* in run table */
    char *SEQ_ID;
    uint64_t first; /* spot  in REFERENCE table */
    uint64_t count; /* spots in REFERENCE table */
    bool external;  /* reference */
    bool circular;  /* reference */
    size_t base_count; /* number of bases in reference */
};
static void _VdbBlastRefWhack(VdbBlastRef *self) {
    assert(self);

    free(self->SEQ_ID);

    memset(self, 0, sizeof *self);
}

static VdbBlastStatus _VdbBlastRefSetCounts(VdbBlastRef *self, uint64_t cur_row,
    int64_t first_row, const VCursor *cursor, uint32_t idxREAD_LEN, int64_t idx,
    uint64_t *base_count)
{
    assert(self && base_count);

    *base_count = 0;

    if (self->count != 0) {
        return eVdbBlastNoErr;
    }

    assert(cur_row != first_row);

    self->count = cur_row - self->first;

    {
        uint32_t read_len = 0;
        uint32_t row_len = 0;
        rc_t rc = VCursorReadDirect(cursor, self->first + self->count - 1,
            idxREAD_LEN, 8, &read_len, 4, &row_len);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorReadDirect(READ_LEN, spot=$(spot))",
                "spot=%ld", self->first + self->count - 1));
            return eVdbBlastErr;
        }
        else if (row_len != 4) {
            PLOGERR(klogInt, (klogInt, rc,
                "Bad row_len in VCursorReadDirect(READ_LEN, spot=$(spot))",
                "spot=%ld", self->first + self->count - 1));
            return eVdbBlastErr;
        }
        else {
            self->base_count
                = ( size_t ) ( (self->count - 1) * MAX_SEQ_LEN + read_len );
        }

    }

    STSMSG(1, ("%i) '%s'[%i-%i(%i)][%lu]", idx, self->SEQ_ID, self->first,
        self->first + self->count - 1, self->count, self->base_count));

    *base_count = self->base_count;

    return eVdbBlastNoErr;
}

/******************************************************************************/
void _RefSetFini(RefSet *self) {
    size_t i = 0;

    if (self == NULL)
        return;

    BSTreeWhack(&self->tRuns   , BstNodeWhack, NULL);
    BSTreeWhack(&self->tExtRefs, BstNodeWhack, NULL);
    BSTreeWhack(&self->tIntRefs, BstNodeWhack, NULL);
    
    for (i = 0; i < self->rfdk; ++i)
        _VdbBlastRefWhack(&self->rfd[i]);

    free(self->rfd);

    memset(self, 0, sizeof *self);
}

/******************************************************************************/
typedef struct References {
    const RunSet *rs;     /* table of runs */

    RefSet       *refs;

    size_t        rfdi; /* refs member being read */
    size_t        spot; /* next spot to be read in refs member being read */
    bool      circular; /* for circular references
                         - if true than the spot is provided the second time */

    const VCursor *curs; /* to REFERENCE table of current refs member ([rfdi])*/
    uint32_t   idxREAD; /* index of READ column in VCursor */
    uint64_t   read_id;
    bool           eos; /* end if set: no more sequences to read */
} References;
void _ReferencesWhack(const References *cself) {
    References *self = (References *)cself;

    if (self == NULL) {
        return;
    }

    VCursorRelease(self->curs);

    memset(self, 0, sizeof *self);

    free(self);
}

const References* _RunSetMakeReferences
    (RunSet *self, VdbBlastStatus *status)
{
    rc_t rc = 0;
    uint64_t totalLen = 0;
    uint32_t irun = 0;
    References *r = NULL;
    RefSet *refs = NULL;
    const VCursor *c = NULL;
    uint32_t iREAD_LEN = 0;
    assert(self && status);
    refs = &self->refs;
    r = calloc(1, sizeof *r);
    if (r == NULL) {
        *status = eVdbBlastMemErr;
        return NULL;
    }
    assert(!refs->rfd);
    refs->rfdn = 512; /* initially allocated number of references */
    refs->rfd = calloc(1, refs->rfdn * sizeof *refs->rfd);
    if (refs->rfd == NULL) {
        *status = eVdbBlastMemErr;
        return NULL;
    }
    BSTreeInit(&refs->tRuns);
    BSTreeInit(&refs->tExtRefs);
    BSTreeInit(&refs->tIntRefs);
    r->rs = self;
    r->refs = refs;
    for (irun = 0; irun < self->krun; ++irun) {
        const void *crntSeqId = NULL;
        uint32_t iCIRCULAR = 0; 
        uint32_t iCMP_READ = 0;
        uint32_t iSEQ_ID   = 0;
        int64_t first = 0;
        uint64_t count = 0;
        uint64_t cur_row = 0;
        const VdbBlastRun *run = &self->run[irun];
        if (run->obj == NULL || run->obj->db == NULL)
            continue;

        {
            BstNode *n
                = (BstNode*)BSTreeFind(&refs->tRuns, run->acc, BstNodeCmpStr);
            if (n != NULL) {
                continue; /* ignore repeated runs */
            }
            else {
                n = calloc(1, sizeof *n);
                if (n == NULL) {
                    *status = eVdbBlastMemErr;
                    return NULL;
                }
            }
            BstNodeInit(n, run->acc, refs->rfdk);
            BSTreeInsert(&refs->tRuns, (BSTNode*)n, RunBstSort);
        }
        if (run->obj->refTbl == NULL) {
            rc = VDatabaseOpenTableRead(run->obj->db,
                &run->obj->refTbl, "REFERENCE");
            if (rc != 0) {
          /* Do not log error : this DB does not have any reference - it happens
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot open REFERENCE table for $(acc))",
                    "acc=%s", run->acc));*/
                continue;
            }
        }
        rc = VTableCreateCursorRead(run->obj->refTbl, &c);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot create REFERENCE Cursor for $(acc))",
                "acc=%s", run->acc));
            continue;
        }
        if (rc == 0) {
            rc = VCursorAddColumn(c, &iCIRCULAR, "CIRCULAR");
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot add REFERENCE/CIRCULAR Column for $(acc))",
                    "acc=%s", run->acc));
            }
        }
        if (rc == 0) {
            rc = VCursorAddColumn(c, &iCMP_READ, "CMP_READ");
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot add REFERENCE/CMP_READ Column for $(acc))",
                    "acc=%s", run->acc));
            }
        }
        if (rc == 0) {
            rc = VCursorAddColumn(c, &iREAD_LEN, "READ_LEN");
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot add REFERENCE/READ_LEN Column for $(acc))",
                    "acc=%s", run->acc));
            }
        }
        if (rc == 0) {
            rc = VCursorAddColumn(c, &iSEQ_ID, "SEQ_ID");
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot add REFERENCE/SEQ_ID Column for $(acc))",
                    "acc=%s", run->acc));
            }
        }
        if (rc == 0) {
            rc = VCursorOpen(c);
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot open REFERENCE Cursor for $(acc))",
                    "acc=%s", run->acc));
            }
        }
        if (rc == 0) {
            rc = VCursorIdRange(c, iSEQ_ID, &first, &count);
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Cannot get REFERENCE/CursorIdRange for $(acc))",
                    "acc=%s", run->acc));
            }
        }

        for (cur_row = first;
            cur_row < first + count && rc == 0; ++cur_row)
        {
            bool newRef = false;

            const void *base = NULL;
            uint32_t elem_bits = 0, boff = 0, row_len = 0;
            char *SEQ_ID = NULL;

            rc = VCursorCellDataDirect(c, cur_row, iSEQ_ID,
                &elem_bits, &base, &boff, &row_len);
            if (rc == 0 && (elem_bits != 8 || boff != 0)) {
                rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
            }
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc, "Cannot get "
                    "CellDataDirect for $(acc)/REFERENCE/SEQ_ID/$(row)",
                    "acc=%s,row=%lu", run->acc, cur_row));
                break;
            }
            if (crntSeqId == NULL) { /* the first reference in a run */
                newRef = true;
            }
            else if (crntSeqId != base) { /* pointer to SeqId has changed -
                                             might be a new reference*/
                newRef = true;
            }

            if (newRef) {
                crntSeqId = base;
                SEQ_ID = string_dup(base, row_len);
                if (SEQ_ID == NULL) {
                    *status = eVdbBlastMemErr;
                    return NULL;
                }
                if (refs->rfdk > 0 && /* there are previous references */
                    cur_row != first) /* not the first reference row in a run */
                {
                    const VdbBlastRef *rfd1 = &refs->rfd[refs->rfdk - 1];
                    if (string_cmp(rfd1->SEQ_ID, string_size(rfd1->SEQ_ID),
                            SEQ_ID, string_size(SEQ_ID), string_size(SEQ_ID))
                        == 0)
                    {
                     /* a SEQ_ID with a different pointer but the same value:
                       (e.g. SRR520124/REFERENCE) */
                        free((void*)SEQ_ID);
                        SEQ_ID = NULL;
                        newRef = false;
                    }
                }
            }

            if (newRef) {
                bool CIRCULAR = false;
                bool external = false;
                VdbBlastRef *rfd  = NULL;
                rc = VCursorCellDataDirect(c, cur_row, iCIRCULAR,
                    &elem_bits, &base, &boff, &row_len);
                if (rc == 0 &&
                    (base == NULL || elem_bits != 8 || boff != 0))
                {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during "
                        "CellDataDirect for $(acc)/REFERENCE/CIRCULAR/$(row)",
                        "acc=%s,row=%lu", run->acc, cur_row));
                    break;
                }
                CIRCULAR = *(bool*)base;

                rc = VCursorCellDataDirect(c, cur_row, iCMP_READ,
                    &elem_bits, &base, &boff, &row_len);
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Cannot get "
                        "CellDataDirect for $(acc)/REFERENCE/CMP_READ/$(row)",
                        "acc=%s,row=%lu", run->acc, cur_row));
                    break;
                }
                else if (base == NULL
                    || elem_bits == 0 || row_len == 0)
                {
                    external = true;
                }

                if (refs->rfdk >= refs->rfdn) {
                    void *tmp = NULL;
                    refs->rfdn *= 2;
                    tmp = realloc(refs->rfd, refs->rfdn * sizeof *refs->rfd);
                    if (tmp == NULL) {
                        *status = eVdbBlastMemErr;
                        return NULL;
                    }
                    refs->rfd = tmp;
                }

                rfd = &refs->rfd[refs->rfdk];

                if (refs->rfdk != 0) {
                    uint64_t bc;
                    *status = _VdbBlastRefSetCounts(&refs->rfd[refs->rfdk - 1],
                        cur_row, first, c, iREAD_LEN, refs->rfdk - 1, &bc);
                    if (*status != eVdbBlastNoErr) {
                        return NULL;
                    }
                    totalLen += bc;
                }

#ifndef UNIQ_INTERNAL_SEQ_ID_REFS
                if (external)
#endif
                {
                    BSTree * tRefs
                        = external ? & refs->tExtRefs : & refs->tIntRefs;
                    BstNode *n = (BstNode*)
                        BSTreeFind(tRefs, SEQ_ID, BstNodeCmpStr);
                    if (n != NULL) {  /* we already have this reference */
                        free(SEQ_ID); /* in one of the previous runs */
                        SEQ_ID = NULL;
                    }
                    else {
                        n = calloc(1, sizeof *n);
                        if (n == NULL) {
                            *status = eVdbBlastMemErr;
                            return NULL;
                        }
                        BstNodeInit(n, SEQ_ID, refs->rfdk);
                        BSTreeInsert(tRefs, (BSTNode*)n, RunBstSort);
                    }
                }

                if (SEQ_ID != NULL) { /* we already have it in refs->tExtRefs */
                    rfd->iRun     = irun;                /* or refs->tIntRefs */
                    rfd->SEQ_ID   = SEQ_ID;
                    rfd->first    = cur_row;
                    rfd->circular = CIRCULAR;
                    rfd->external = external;

                    rfd->count      = 0; /* will initialize later */
                    rfd->base_count = 0; /* uninitialized */

                    ++refs->rfdk;
                }
            }
        }
        if (refs->rfdk > 0 && refs->rfd[refs->rfdk - 1].count == 0) {
            uint64_t bc;
            *status = _VdbBlastRefSetCounts(&refs->rfd[refs->rfdk - 1],
                cur_row, first, c, iREAD_LEN, refs->rfdk - 1, &bc);
            if (*status != eVdbBlastNoErr) {
                return NULL;
            }
            totalLen += bc;
        }
        RELEASE(VCursor, c);
    }
    refs->totalLen = totalLen;
    *status = eVdbBlastNoErr;
    return r;
}

/******************************************************************************/
uint64_t _ReferencesGetNumSequences
    (const References *self, VdbBlastStatus *status)
{
    assert(status);

    if (self == NULL || self->refs == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    *status = eVdbBlastNoErr;
    return self->refs->rfdk;
}

uint64_t _ReferencesGetTotalLength
    (const References *self, VdbBlastStatus *status)
{
    assert(status);

    if (self == NULL || self->refs == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    *status = eVdbBlastNoErr;
    return self->refs->totalLen;
}


uint64_t _ReferencesGetReadLength(const struct References *self,
    uint64_t read_id,
    VdbBlastStatus *status)
{
    bool bad = false;
    read_id = _clear_read_id_reference_bit(read_id, &bad);

    assert(status);

    if (bad) {
        *status = eVdbBlastInvalidId;
        return 0;
    }

    if (self == NULL || self->refs == NULL ||
        self->refs->rfdk <= read_id)
    {
        return 0;
    }

    {
        const VdbBlastRef *r = NULL;
        assert(self->refs->rfd);
        r = &self->refs->rfd[read_id];

        if (r->circular) {
            *status = eVdbBlastCircularSequence;
        }
        else {
            *status = eVdbBlastNoErr;
        }

        return r->base_count;
    }
}


#define REF_SEPARATOR '/'

size_t CC _ReferencesGetReadName(const struct References *self,
    uint64_t read_id, char *name_buffer, size_t bsize)
{
    const VdbBlastRef *r = NULL;
    const char *acc = NULL;

    bool bad = false;
    read_id = _clear_read_id_reference_bit(read_id, &bad);
    if (bad) {
        return 0;
    }

    if (self == NULL || self->refs == NULL ||
        self->refs->rfdk <= read_id)
    {
        return 0;
    }

    assert(self->refs->rfd);
    r = &self->refs->rfd[read_id];

    if (!r->external) {
        if (self->rs == NULL || self->rs->krun <= r->iRun) {
            return 0;
        }

        acc = self->rs->run[r->iRun].acc;
    }

    {
        const char *SEQ_ID = self->refs->rfd[read_id].SEQ_ID;

        size_t num_writ = 0;
        if (acc == NULL) {
            string_printf(name_buffer, bsize, &num_writ, "%s", SEQ_ID);
        }
        else {
            string_printf(name_buffer, bsize, &num_writ,
                "%s%c%s", acc, REF_SEPARATOR, SEQ_ID);
        }
        return num_writ;
    }
}

/******************************************************************************/
VdbBlastStatus _ReferencesGetReadId(const References *self,
    const char *name_buffer, size_t bsize, uint64_t *read_id)
{
    int32_t rfdi = ~0;
    const char *sp = name_buffer;
    const RefSet *refs = NULL;
    String acc, seq;
    BstNode *n = NULL;
    if (self == NULL || self->refs == NULL || self->rs == NULL ||
        name_buffer == NULL || bsize == 0 || read_id == NULL)
    {
        return eVdbBlastErr;
    }
    refs = self->refs;
    memset(&acc, 0 , sizeof acc);
    memset(&seq, 0 , sizeof seq);

    sp = string_chr(name_buffer, bsize, REF_SEPARATOR);
    if (sp != NULL) {
        StringInit(&acc, name_buffer, sp - name_buffer, sp - name_buffer);
        if ((sp - name_buffer) >= bsize) {
            return eVdbBlastErr;
        }
    }

    if (sp != NULL) {
        StringInit(&seq, sp + 1, bsize - (sp - name_buffer) - 1,
                                 bsize - (sp - name_buffer) - 1);
    }
    else {
        StringInit(&seq, name_buffer, bsize, bsize);
    }
    if (acc.size != 0) {
        bool found = false;
        int32_t iRun = ~0;
        n = (BstNode*)BSTreeFind(&refs->tRuns, &acc, BstNodeCmpString);
        if (n == NULL) {
            return eVdbBlastErr;
        }
        iRun = refs->rfd[n->rfdi].iRun;
        assert(iRun < self->rs->krun);
        assert(!string_cmp(self->rs->run[iRun].acc,
               string_size(self->rs->run[iRun].acc),
               acc.addr, acc.size, acc.size));
        for (rfdi = n->rfdi; rfdi < refs->rfdk; ++rfdi) {
            const VdbBlastRef *r = &refs->rfd[rfdi];
            if (r->iRun != iRun)
                return eVdbBlastErr;
            if (string_cmp(seq.addr, seq.size,
                r->SEQ_ID, string_size(r->SEQ_ID), string_size(r->SEQ_ID)) == 0)
            {
                assert(!r->external);
                found = true;
                break;
            }
        }
        if (!found)
            return eVdbBlastErr;
    }
    else {
        n     = (BstNode*)BSTreeFind(&refs->tExtRefs, &seq, BstNodeCmpString);
        if (n == NULL)
            n = (BstNode*)BSTreeFind(&refs->tIntRefs, &seq, BstNodeCmpString);
        if (n == NULL)
            return eVdbBlastErr;
        rfdi = n->rfdi;
        assert(refs->rfd[rfdi].external);
    }
    {
        VdbBlastStatus status = eVdbBlastNoErr;
        assert(rfdi < refs->rfdk);
        *read_id = _set_read_id_reference_bit(rfdi, &status);
        return status;
    }
}

/******************************************************************************/
static uint64_t _ReferencesRead2na(References *self,
    VdbBlastStatus *status, uint64_t *read_id,
    size_t *starting_base, uint8_t *buffer, size_t buffer_size)
{
    rc_t rc = 0;
    uint64_t total = 0;
    const VdbBlastRef *rfd = NULL;
    uint8_t *begin = buffer;
    assert(status && self && self->rs && read_id && starting_base);
    *status = eVdbBlastNoErr;
    assert(self->refs);
    rfd = &self->refs->rfd[self->rfdi];
    while (total < buffer_size * 4) {
        uint32_t start = 0;
        uint32_t to_read = 0;
        uint32_t num_read = 0;
        uint32_t remaining = 0;
        if (self->spot == 0 ||
           /* the very first call: open the first spot of the first reference */

            self->rfdi != self->read_id) /* should switch to a next reference */
        {
            const VdbBlastRef *rfd1 = NULL;
            const VTable *t = NULL;
            assert(!total);
            if (self->rfdi != self->read_id) {/* switching to a next reference*/
                if (self->rfdi + 1 != self->read_id) { /* should never happen */
                    *status = eVdbBlastErr;
                    S
                    return 0;
                }
                *starting_base = 0;
                ++self->rfdi;
                if (self->rfdi >= self->refs->rfdk) {
                    self->eos = true;
                    *status = eVdbBlastNoErr; /* end of set */
                    S
                    return 0;
                }
                rfd1 = rfd;
                rfd = &self->refs->rfd[self->rfdi];
            }
            if (rfd->iRun >= self->rs->krun) {
                S
                return 0;
            }
            if (self->rs->run == NULL || self->rs->run[rfd->iRun].obj == NULL ||
                self->rs->run[rfd->iRun].obj->refTbl == NULL)
            {
                S
                return 0;
            }
            if (self->rfdi == 0 || rfd1->iRun != rfd->iRun) {
                const char *acc = self->rs->run[rfd->iRun].acc;
                t = self->rs->run[rfd->iRun].obj->refTbl;
                RELEASE(VCursor, self->curs);
                rc = VTableCreateCursorRead(t, &self->curs);
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc,
                        "Cannot create REFERENCE Cursor for $(acc)) /Read2na",
                        "acc=%s", acc));
                    return 0;
                }
                rc = VCursorAddColumn(self->curs,
                    &self->idxREAD, "(INSDC:2na:packed)READ");
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Cannot add "
                        "REFERENCE/READ Column for $(acc)) /Read2na",
                        "acc=%s", acc));
                    return 0;
                }
                rc = VCursorOpen(self->curs);
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc,
                        "Cannot open REFERENCE Cursor for $(acc)) /Read2na",
                        "acc=%s", acc));
                    return 0;
                }
            }
            else {
                if (self->curs == NULL || self->idxREAD == 0) {
                    *status = eVdbBlastErr;
                    S /* should never happen */
                    return 0;
                }
            }
            if (self->spot == 0) {
                self->read_id = 0;
            }
            self->spot = rfd->first;
        }
        start = (uint32_t)*starting_base;
        to_read = (uint32_t)(buffer_size * 4 - total);
        rc = VCursorReadBitsDirect(self->curs, self->spot, self->idxREAD,
            2, start, begin, 0, to_read, &num_read, &remaining);
        total += num_read;
        *status = eVdbBlastNoErr;
        *read_id = _set_read_id_reference_bit(self->read_id, status);
        if (*status != eVdbBlastNoErr) {
            break;
        }
        if (rc != 0) {
            if (rc == SILENT_RC
                (rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient))
            {
                S
                if (num_read == 0) {
                    *status = eVdbBlastErr;
                    S /* should never happen */
                }
                else {
                    rc = 0;
                }
                *starting_base += num_read;
                break;
            }
            else {
                PLOGERR(klogInt, (klogInt, rc, "Error in VCursorReadBitsDirect"
                    "($(path), READ, spot=$(spot)) /Read2na",
                    "path=%s,spot=%ld",
                    self->rs->run[rfd->iRun].path, self->spot));
                *status = eVdbBlastErr;
                return 0;
            }
        }
        else {
            if (remaining != 0) { /* The buffer is filled. */
                S     /* There remains more data to read in the current spot. */
                *starting_base += num_read;
                break;
            }
            ++self->spot;
            *starting_base = 0;
            if (self->spot >= rfd->first + rfd->count) {
                if (rfd->circular && ! self->circular) {
                       /* end of the first repeat of a circular sequence */
                    *status = eVdbBlastCircularSequence;
                    self->circular = true;
                    self->spot = rfd->first;
                }
                else { /* end of sequence */
                    self->circular = false;
                    ++self->read_id;
                }
                break;
            }
            begin += num_read / 4;
            if ((num_read % 4) != 0) {
                S
                *status = eVdbBlastErr;
                break; /* should never happen */
            }
        }
    }
    return total;
}

#if _DEBUGGING
#define COMPARE
#endif
static uint32_t _ReferencesData2na(References *self,
    Data2na *data, VdbBlastStatus *status,
    Packed2naRead *buffer, uint32_t buffer_length)
{
    uint32_t num_read = 0;
    assert(data && status && self && self->rs);
    *status = eVdbBlastNoErr;
    assert(self->refs);
    for (num_read = 0; num_read < buffer_length; ) {
        Packed2naRead *out = NULL;
        rc_t rc = 0;
        const VdbBlastRef *rfd = &self->refs->rfd[self->rfdi];
        const char *acc = self->rs->run[rfd->iRun].acc;
        int64_t first = 0;
        uint64_t count = 0;
        uint64_t last_spot = 0;
        uint32_t elem_bits = 0;
#ifdef COMPARE
        uint32_t row_len = 0;
#endif
        size_t first_spot = 0;
        bool full_scan = false;
        *status = eVdbBlastErr;
        assert(buffer);
        RELEASE(VBlob, data->blob);
        out = &buffer[num_read];
        assert(self->refs);
        if (self->spot == 0 ||
         /* the very first call: open the first spot of the first reference */

            self->rfdi != self->read_id) /* should switch to a next reference */
        {
            const VdbBlastRef *rfd1 = NULL;
            const VTable *t = NULL;
            if (self->rfdi != self->read_id) {/* switching to a next reference*/
                if (self->rfdi + 1 != self->read_id) { /* should never happen */
                    S
                    return 0;
                }
                ++self->rfdi;
                if (self->rfdi >= self->refs->rfdk) {
                    self->eos = true;
                    *status = eVdbBlastNoErr; /* end of set */
                    S
                    return 0;
                }
                rfd1 = rfd;
                rfd = &self->refs->rfd[self->rfdi];
            }
            if (rfd->iRun >= self->rs->krun) {
                S
                return 0;
            }
            if (self->rs->run == NULL || self->rs->run[rfd->iRun].obj == NULL ||
                self->rs->run[rfd->iRun].obj->refTbl == NULL)
            {
                S
                return 0;
            }
            if (self->rfdi == 0 || rfd1->iRun != rfd->iRun) {
                const char *acc = self->rs->run[rfd->iRun].acc;
                t = self->rs->run[rfd->iRun].obj->refTbl;
                RELEASE(VCursor, self->curs);
                rc = VTableCreateCursorRead(t, &self->curs);
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc,
                        "Cannot create REFERENCE Cursor for $(acc)) /Data2na",
                        "acc=%s", acc));
                    return 0;
                }
                rc = VCursorAddColumn(self->curs,
                    &self->idxREAD, "(INSDC:2na:packed)READ");
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc,
                        "Cannot add REFERENCE/READ Column for $(acc)) /Data2na",
                        "acc=%s", acc));
                    return 0;
                }
                rc = VCursorOpen(self->curs);
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc,
                        "Cannot open REFERENCE Cursor for $(acc) /Data2na)",
                        "acc=%s", acc));
                    return 0;
                }
            }
            else {
                if (self->curs == NULL || self->idxREAD == 0) {
                    S /* should never happen */
                    return 0;
                }
            }
            if (self->spot == 0) {
                self->read_id = 0;
            }
            self->spot = rfd->first;
            data->irun = self->rfdi;
        }
        rc = VCursorGetBlobDirect(self->curs,
            &data->blob, self->spot, self->idxREAD);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot GetBlob REFERENCE/READ for $(acc)/$(spot) /2na)",
                "acc=%s,spot=%zu", acc, self->spot));
            return 0;
        }
        if (data->blob == NULL) {
            S
            return 0;
        }
        rc = VBlobIdRange(data->blob, &first, &count);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot BlobIdRange REFERENCE/READ for $(acc)/$(spot) /2na",
                "acc=%s,spot=%zu", acc, self->spot));
            return 0;
        }
        if (self->spot < first || self->spot >= first + count) {
            /* requested blob b(spot) but spot < b.first || spot > b.last:
               should never happen */
            S /* PLOGERR */
            return 0;
        }
        if (first > rfd->first + rfd->count) { /* should never happen */
            S /* PLOGERR */
            return 0;
        }
        last_spot = first + count;
        if (rfd->first + rfd->count < last_spot) {
            last_spot = rfd->first + rfd->count;
        }
        first_spot = self->spot;
#ifdef COMPARE
        for (; self->spot < last_spot; ++self->spot) {
            if (self->spot == first_spot) {
                rc = VBlobCellData(data->blob, self->spot, &elem_bits,
                    (const void **)&out->starting_byte,
                    &out->offset_to_first_bit, &row_len);
                if (rc == 0 && elem_bits != 2) {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /2na",
                        "acc=%s,spot=%zu", acc, self->spot));
                    return 0;
                }
                else {
                    out->length_in_bases = row_len;
                }
            }
            else if (self->spot != last_spot - 1) {
                out->length_in_bases += row_len;
            }
            else {
                const void *base = NULL;
                rc = VBlobCellData(data->blob, self->spot,
                    &elem_bits, &base, NULL, &row_len);
                if (rc == 0 && elem_bits != 2) {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /2na",
                        "acc=%s,spot=%zu", acc, self->spot));
                    return 0;
                }
                else {
                    out->length_in_bases += row_len;
                }
            }
        }
#endif
        {
            const void *base = NULL;
            uint32_t boff = 0;
            uint32_t len = 0;
            rc = VBlobCellData
                (data->blob, first_spot, &elem_bits, &base, &boff, &len);
            if (rc == 0 && elem_bits != 2) {
                rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
            }
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                    "for $(acc)/REFERENCE/READ/$(spot)) /2na",
                    "acc=%s,spot=%zu", acc, first_spot));
                return 0;
            }
            if (first_spot + 1 == last_spot) {
                /* the only chunk */
            }
            else if (len != MAX_SEQ_LEN) {
                full_scan = true;
                assert(0);
            }
            else {
                const void *last_base = NULL;
                uint32_t boff = 0;
                uint32_t last_len = 0;
                rc = VBlobCellData(data->blob, last_spot - 1,
                    &elem_bits, &last_base, &boff, &last_len);
                if (rc == 0 && elem_bits != 2) {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /2na",
                        "acc=%s,spot=%zu", acc, last_spot - 1));
                    return 0;
                }
                else {
                    size_t chunk_len = ((const uint8_t*)last_base
                                         - (const uint8_t*)base) * 4 + last_len;
                    size_t num_ref_chunks
                        = (chunk_len + MAX_SEQ_LEN - 1) / MAX_SEQ_LEN;
                    if (num_ref_chunks != last_spot - first_spot) {
                        full_scan = true;
                    }
                    else {
                        len = chunk_len;
                    }
                }
            }
            if (!full_scan) {
#ifdef COMPARE
                assert(out->starting_byte == base);
                assert(out->offset_to_first_bit == boff);
                assert(out->length_in_bases == len);
                assert(self->spot == last_spot);
#else
                out->starting_byte = (void*)base;
                out->offset_to_first_bit = boff;
                out->length_in_bases = len;
                self->spot = last_spot;
#endif
            }
        }
#ifndef COMPARE
        if (full_scan)
#endif
        {
            const void *starting_byte = NULL;
            const void *prev_base = NULL;
            size_t length_in_bases = 0;
            uint32_t offset_to_first_bit = 0;
            for (self->spot = first_spot;
                self->spot < last_spot; ++self->spot)
            {
                const void *base = NULL;
                uint32_t boff = 0;
                uint32_t len = 0;
                rc = VBlobCellData(data->blob, self->spot,
                    &elem_bits, &base, &boff, &len);
                if (rc == 0 && elem_bits != 2) {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /2na",
                        "acc=%s,spot=%zu", acc, self->spot));
                    return 0;
                }
                if (self->spot == first_spot) {
                    starting_byte = base;
                    offset_to_first_bit = boff;
                }
                else {
                    if ((const uint8_t*)prev_base + MAX_SEQ_LEN / 4 != base) {
                        assert(full_scan);
                        break;
                    }
                }
                prev_base = base;
                length_in_bases += len;
            }
#ifdef COMPARE
            if (!full_scan) {
                assert(out->starting_byte == starting_byte);
                assert(out->offset_to_first_bit == offset_to_first_bit);
                assert(out->length_in_bases == length_in_bases);
                assert(self->spot == last_spot);
            }
#endif
            if (full_scan) {
                out->starting_byte = (void*)starting_byte;
                out->offset_to_first_bit = offset_to_first_bit;
                out->length_in_bases = length_in_bases;
            }
        }
        *status = eVdbBlastNoErr;
        out->read_id = _set_read_id_reference_bit(self->read_id, status);
        if (*status != eVdbBlastNoErr) {
            break;
        }
        if (self->spot < rfd->first + rfd->count) {
            *status = eVdbBlastChunkedSequence;
        }
        else if (rfd->circular && ! self->circular) {
               /* end of the first repeat of a circular sequence */
            *status = eVdbBlastCircularSequence;
            self->circular = true;
            self->spot = rfd->first;
        }
        else { /* end of sequence */
            *status = eVdbBlastNoErr;
            self->circular = false;
            ++self->read_id;
        }
        ++num_read;
        break;
    }
    return num_read;
}

/******************************************************************************/
uint64_t _Core2naReadRef(Core2na *self, VdbBlastStatus *status,
    uint64_t *read_id, uint8_t *buffer, size_t buffer_size)
{
    uint64_t num_read = 0;

    References *r = NULL;

    assert(status);

    *status = eVdbBlastNoErr;

    if (self == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    if (self->reader.refs == NULL) { /* do not have any reference */
        self->eos = true;
        return 0;
    }

    r = (References*)(self->reader.refs);

    assert(r->refs);

    if (r->rfdi > r->refs->rfdk) {
        self->eos = true;
    }

    if (self->eos) {
        return 0;
    }

    num_read = _ReferencesRead2na(r, status, read_id,
        &self->reader.starting_base, buffer, buffer_size);

    if (num_read == 0 && *status == eVdbBlastNoErr && r->eos) {
        self->eos = true;
    }

    return num_read;
}

uint32_t _Core2naDataRef(struct Core2na *self,
    Data2na *data, VdbBlastStatus *status,
    Packed2naRead *buffer, uint32_t buffer_length)
{
    uint32_t num_read = 0;

    References *r = NULL;

    assert(status);

    *status = eVdbBlastNoErr;

    if (self == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    if (self->reader.refs == NULL) { /* do not have any reference */
        self->eos = true;
        return 0;
    }

    r = (References*)(self->reader.refs);

    assert(r->refs);

    if (r->rfdi > r->refs->rfdk) {
        self->eos = true;
    }

    if (self->eos) {
        return 0;
    }

    num_read = _ReferencesData2na(r, data, status, buffer, buffer_length);
    if (num_read == 0 && *status == eVdbBlastNoErr && r->eos) {
        self->eos = true;
    }

    return num_read;
}

/******************************************************************************/
size_t _Core4naReadRef(Core4na *self, const RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t starting_base,
    uint8_t *buffer, size_t buffer_length)
{
    size_t total = 0;
    uint8_t *begin = buffer;
    const VdbBlastRef *rfd = NULL;
    const VdbBlastRun *run = NULL;
    uint64_t spot = 0;

    bool circular = false;
                 /* true when returning a circular reference the second time */

    uint32_t start = 0;
    assert(status);
    if (self == NULL || runs == NULL ||
        runs->refs.rfd == NULL || runs->refs.rfdk == 0)
    {
        *status = eVdbBlastErr;
        return 0;
    }
    {
        bool bad = false;
        read_id = _clear_read_id_reference_bit(read_id, &bad);
        if (bad) {
            *status  = eVdbBlastInvalidId;
            return 0;
        }
    }
    if (read_id >= runs->refs.rfdk) {
        *status  = eVdbBlastInvalidId;
        return 0;
    }
    rfd = &runs->refs.rfd[read_id];
    *status = eVdbBlastErr;
    if (runs->run == NULL) {
        return 0;
    }
    run = &runs->run[rfd->iRun];
    if (self->curs != NULL) {
        if (self->desc.tableId != read_id) {
            VCursorRelease(self->curs);
            self->curs = NULL;
        }
    }
    if (self->curs == NULL) {
        rc_t rc = 0;
        if (rfd->iRun >= runs->krun) {
            return 0;
        }
        if (run->obj == NULL || run->obj->refTbl == NULL) {
            return 0;
        }
        rc = VTableCreateCursorRead(run->obj->refTbl, &self->curs);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot create REFERENCE Cursor for $(acc))",
                "acc=%s", run->acc));
            return 0;
        }
        rc = VCursorAddColumn(self->curs,
            &self->col_READ, "(INSDC:4na:bin)READ");
        if (rc == 0) {
            rc = VCursorOpen(self->curs);
        }
        if (rc != 0) {
            RELEASE(VCursor, self->curs);
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot make REFERENCE/READ Cursor for $(acc)) /Read4na",
                "acc=%s", run->acc));
            return 0;
        }
        self->desc.tableId = read_id;
        self->desc.spot = 0;
    }
    *status = eVdbBlastNoErr;

    if (rfd->circular) {
        assert(rfd->base_count);
        if (starting_base >= rfd->base_count) {
            starting_base -= rfd->base_count;
            circular = true;
        }
    }

    spot = rfd->first + starting_base / MAX_SEQ_LEN;
    if (spot >= rfd->first + rfd->count) {
        return 0;
    }
    start = starting_base % MAX_SEQ_LEN;
    while (total < buffer_length) {
        rc_t rc = 0;
        uint32_t num_read = 0;
        uint32_t remaining = 0;
        uint32_t to_read = (uint32_t)(buffer_length - total);
        if (to_read == 0) {
            S
            break;
        }
        rc = VCursorReadBitsDirect(self->curs, spot, self->col_READ,
            8, start, begin, 0, to_read, &num_read, &remaining);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorReadBitsDirect($(path), READ, spot=$(spot))",
                "path=%s,spot=%ld", run->path, spot));
            *status = eVdbBlastErr;
            break;
        }
        else {
            total += num_read;
            if (total > buffer_length) {
                total = buffer_length;
            }
            if (total == buffer_length) {
                break;
            }
            begin += num_read;
            if (remaining > 0) {
            }
            else if (++spot >= rfd->first + rfd->count) {
                if (rfd->circular && ! circular) {
                    *status = eVdbBlastCircularSequence;
                }
                break; /* end of reference */
            }
            else {     /* next spot */
                start = 0;
            }
        }
    }
    return total;
}

const uint8_t* _Core4naDataRef(Core4na *self, const RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t *length)
{
    const void *out = NULL;
    rc_t rc = 0;
    const VdbBlastRef *rfd = NULL;
    const VdbBlastRun *run = NULL;
    int64_t first = 0;
    uint64_t count = 0;
    uint64_t last_spot = 0;
    size_t first_spot = 0;
    bool full_scan = false;
    assert(status);
    *status = eVdbBlastErr;
    if (length == NULL || self == NULL || runs == NULL ||
        runs->refs.rfd == NULL || runs->refs.rfdk == 0)
    {
        return NULL;
    }
    *length = 0;
    {
        bool bad = false;
        read_id = _clear_read_id_reference_bit(read_id, &bad);
        if (bad) {
            *status  = eVdbBlastInvalidId;
            return NULL;
        }
    }
    if (read_id >= runs->refs.rfdk) {
        *status  = eVdbBlastInvalidId;
        return NULL;
    }
    rfd = &runs->refs.rfd[read_id];
    if (runs->run == NULL) {
        return NULL;
    }
    run = &runs->run[rfd->iRun];
    if (self->curs != NULL) {
        if (self->desc.tableId != read_id) {
            RELEASE(VCursor, self->curs);
        }
    }
    if (self->curs == NULL) {
        rc_t rc = 0;
        if (rfd->iRun >= runs->krun) {
            return NULL;
        }
        if (run->obj == NULL || run->obj->refTbl == NULL) {
            return NULL;
        }
        rc = VTableCreateCursorRead(run->obj->refTbl, &self->curs);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot create REFERENCE Cursor for $(acc))",
                "acc=%s", run->acc));
            return NULL;
        }
        rc = VCursorAddColumn(self->curs,
            &self->col_READ, "(INSDC:4na:bin)READ");
        if (rc == 0) {
            rc = VCursorOpen(self->curs);
        }
        if (rc != 0) {
            RELEASE(VCursor, self->curs);
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot make REFERENCE/READ Cursor for $(acc)) /Data4na",
                "acc=%s", run->acc));
            return NULL;
        }
        self->desc.tableId = read_id;
        self->desc.spot = 0;
    }
    if (self->blob) {
        RELEASE(VBlob, self->blob);
    }
    if (self->desc.spot == 0) {
        self->desc.spot = rfd->first;
    }
    else {
        if (self->desc.spot >= rfd->first + rfd->count) {
            *status = eVdbBlastNoErr;  /* end of reference */
            return NULL;
        }
    }
    rc = VCursorGetBlobDirect(self->curs,
        &self->blob, self->desc.spot, self->col_READ);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc,
            "Cannot GetBlob REFERENCE/READ for $(acc)/$(spot) /4na)",
            "acc=%s,spot=%zu", run->acc, self->desc.spot));
        return 0;
    }
    if (self->blob == NULL) {
        S
        return 0;
    }
    rc = VBlobIdRange(self->blob, &first, &count);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc,
            "Cannot BlobIdRange REFERENCE/READ for $(acc)/$(spot) /4na",
            "acc=%s,spot=%zu", run->acc, self->desc.spot));
        return 0;
    }
    if (self->desc.spot < first || self->desc.spot >= first + count) {
        /* requested blob b(spot) but spot < b.first || spot > b.last:
           should never happen */
        S /* PLOGERR */
        return 0;
    }
    if (first > rfd->first + rfd->count) { /* should never happen */
        S /* PLOGERR */
        return 0;
    }
    last_spot = first + count;
    if (rfd->first + rfd->count < last_spot) {
        last_spot = rfd->first + rfd->count;
    }
    {
#ifdef COMPARE
        uint32_t row_len = 0;
#endif
        first_spot = self->desc.spot;
#ifdef COMPARE
        for (; self->desc.spot < last_spot; ++self->desc.spot) {
            uint32_t elem_bits = 0;
            uint32_t offset_to_first_bit = 0;
            if (self->desc.spot == first_spot) {
                rc = VBlobCellData(self->blob, self->desc.spot, &elem_bits,
                    &out, &offset_to_first_bit, &row_len);
                if (rc == 0 &&
                    (elem_bits != 8 || offset_to_first_bit != 0))
                {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /4na",
                        "acc=%s,spot=%zu", run->acc, self->desc.spot));
                    return NULL;
                }
                else {
                    *length = row_len;
                }
            }
            else if (self->desc.spot != last_spot - 1) {
                *length += row_len;
            }
            else {
                const void *base = NULL;
                rc = VBlobCellData(self->blob, self->desc.spot,
                    &elem_bits, &base, NULL, &row_len);
                if (rc == 0 && elem_bits != 8) {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /4na",
                        "acc=%s,spot=%zu", run->acc, self->desc.spot));
                    return NULL;
                }
                else {
                    *length += row_len;
                }
            }
        }
#endif
    }
    {
        const void *base = NULL;
        uint32_t boff = 0;
        uint32_t len = 0;
        uint32_t elem_bits = 0;
        rc = VBlobCellData
            (self->blob, first_spot, &elem_bits, &base, &boff, &len);
        if (rc == 0 && (elem_bits != 8 || boff != 0)) {
            rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
        }
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                "for $(acc)/REFERENCE/READ/$(spot)) /4na",
                "acc=%s,spot=%zu", run->acc, first_spot));
            return NULL;
        }
        if (first_spot + 1 == last_spot) {
            /* the only chunk */
        }
        else if (len != MAX_SEQ_LEN) {
            full_scan = true;
            assert(0);
        }
        else {
            const void *last_base = NULL;
            uint32_t boff = 0;
            uint32_t last_len = 0;
            rc = VBlobCellData(self->blob, last_spot - 1,
                &elem_bits, &last_base, &boff, &last_len);
            if (rc == 0 && (elem_bits != 8 || boff != 0)) {
                rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
            }
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                    "for $(acc)/REFERENCE/READ/$(spot)) /4na",
                    "acc=%s,spot=%zu", run->acc, last_spot - 1));
                return 0;
            }
            else {
                size_t chunk_len = ((const uint8_t*)last_base
                                     - (const uint8_t*)base) + last_len;
                size_t num_ref_chunks
                    = (chunk_len + MAX_SEQ_LEN - 1) / MAX_SEQ_LEN;
                if (num_ref_chunks != last_spot - first_spot) {
                    full_scan = true;
                }
                else {
                    len = chunk_len;
                }
            }
        }
        if (!full_scan) {
#ifdef COMPARE
            assert(out == base);
            assert(*length == len);
            assert(self->desc.spot == last_spot);
#else
            out = base;
            *length = len;
            self->desc.spot = last_spot;
#endif
        }
    }
#ifndef COMPARE
    if (full_scan)
#endif
    {
            const void *starting_byte = NULL;
            const void *prev_base = NULL;
            size_t length_in_bases = 0;
            uint32_t elem_bits = 0;
            for (self->desc.spot = first_spot;
                self->desc.spot < last_spot; ++self->desc.spot)
            {
                const void *base = NULL;
                uint32_t boff = 0;
                uint32_t len = 0;
                rc = VBlobCellData(self->blob, self->desc.spot,
                    &elem_bits, &base, &boff, &len);
                if (rc == 0 && (elem_bits != 8 || boff != 0)) {
                    rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
                }
                if (rc != 0) {
                    PLOGERR(klogInt, (klogInt, rc, "Error during CellData "
                        "for $(acc)/REFERENCE/READ/$(spot)) /4na",
                        "acc=%s,spot=%zu", run->acc, self->desc.spot));
                    return 0;
                }
                if (self->desc.spot == first_spot) {
                    starting_byte = base;
                }
                else {
                    if ((const uint8_t*)prev_base + MAX_SEQ_LEN != base) {
                        assert(full_scan);
                        break;
                    }
                }
                prev_base = base;
                length_in_bases += len;
            }
#ifdef COMPARE
            if (!full_scan) {
                assert(out == starting_byte);
                assert(*length == length_in_bases);
                assert(self->desc.spot == last_spot);
            }
#endif
            if (full_scan) {
                out = starting_byte;
                *length = length_in_bases;
            }
    }
    if (self->desc.spot < rfd->first + rfd->count) {
        *status = eVdbBlastChunkedSequence;
    }
    else if (rfd->circular && ! self->desc.circular) {
        *status = eVdbBlastCircularSequence;
        self->desc.circular = true;
        self->desc.spot = rfd->first;
    }
    else {
        *status = eVdbBlastNoErr;
        self->desc.circular = false;
    }
    return out;
}

/******************************************************************************/
