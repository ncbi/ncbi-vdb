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

#include <vdb/extern.h>

#include "reader.h" /* _RunSetFindReadDesc */
#include "blast-mgr.h" /* BTableType */
#include "run-set.h" /* VdbBlastRunSet */

#include <kdb/kdb-priv.h> /* KTableGetPath */
#include <kdb/table.h> /* KTable */

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h> /* LOGERR */
#include <klib/rc.h> /* RC */
#include <klib/refcount.h> /* KRefcount */
#include <klib/status.h> /* STSMSG */

#include <kproc/lock.h> /* KLockMake */

#include <ncbi/vdb-blast.h> /* VdbBlastRunSet */

#include <vdb/cursor.h> /* VCursor */
#include <vdb/database.h> /* VDatabase */
#include <vdb/table.h> /* VTable */
#include <vdb/vdb-priv.h> /* VTableOpenKTableRead */

#include <sysalloc.h>

#include <string.h> /* memset */
#include <time.h> /* time_t */

#include <limits.h> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

void _Core2naFini(Core2na *self);
void _Core4naFini(Core4na *self);

/******************************************************************************/

static void *_NotImplementedP(const char *func) {
    PLOGERR(klogErr, (klogErr, -1,
        "$(func): is not implemented", "func=%s", func));
    return 0;
}

static size_t _NotImplemented(const char *func)
{   return (size_t)_NotImplementedP(func); }

static uint64_t CC Min(uint64_t cand,
    uint64_t champ, int64_t minRead, bool *done)
{
    assert(done);
    if (minRead >= 0) {
        if (cand < minRead) {
            return champ;
        }
        else if (cand == minRead) {
            *done = true;
            return minRead;
        }
    } 
    return cand < champ ? cand : champ;
}

static uint64_t CC Max(uint64_t cand,
    uint64_t champ, int64_t minRead, bool *done)
{
    return cand > champ ? cand : champ;
}

static char* _CanonocalName(const char *name) {
    size_t noExtSize = 0;
    size_t namelen = 0;
    const char ext[] = ".sra";

    if (name == NULL) {
        return NULL;
    }

    noExtSize = namelen = string_size(name);

    if (namelen >= sizeof(ext)) {
        const char *tail = NULL;
        noExtSize -= sizeof ext - 1;
        tail = name + noExtSize;
        if (string_cmp(ext, sizeof ext - 1, tail, sizeof ext - 1, 99) != 0) {
            noExtSize = namelen;
        }
    }

    return string_dup(name, noExtSize);
}

/******************************************************************************/

static rc_t _VCursorCellDataDirect(const VCursor *self,
    int64_t row_id, uint32_t col_idx, uint32_t elemBits,
    const void **base, uint32_t nreads, const char *name)
{
    uint32_t elem_bits = 0;
    uint32_t row_len = 0;
    uint32_t boff = 0;

    rc_t rc = VCursorCellDataDirect(self, row_id, col_idx,
        &elem_bits, base, &boff, &row_len);

    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc,
            "Error during VCursorCellDataDirect($(name), $(spot))",
            "name=%s,spot=%lu", name, col_idx));
    }
    else if (boff != 0 || elem_bits != elemBits) {
        rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
        PLOGERR(klogInt, (klogInt, rc,
            "Bad VCursorCellDataDirect($(name), $(spot)) result: "
            "boff=$(boff), elem_bits=$(elem_bits)",
            "name=%s,spot=%lu,boff=%u,elem_bits=%u",
            name, col_idx, boff, elem_bits));
    }
    else if (row_len != nreads) {
        rc = RC(rcSRA, rcCursor, rcReading, rcData, rcUnexpected);
        PLOGERR(klogInt, (klogInt, rc,
            "Bad VCursorCellDataDirect($(name), $(spot)) result: "
            "row_len=$(row_len)",
            "name=%s,spot=%lu,row_len=%u", name, col_idx, row_len));
    }

    return rc;
}

/******************************************************************************/

static VdbBlastStatus _VDatabaseOpenAlignmentTable(const VDatabase *self,
    const char *path,
    const VTable **tbl)
{
    const char *table = "PRIMARY_ALIGNMENT";

    rc_t rc = VDatabaseOpenTableRead(self, tbl, table);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc,
            "Error in VDatabaseOpenTableRead($(name), $(tbl))",
            "name=%s,tbl=%s", path, table));
        STSMSG(1, ("Error: failed to open DB table '%s/%s'", path, table));
    }
    else {
        STSMSG(1, ("Opened DB table '%s/%s'", path, table));
    }

    return rc != 0 ? eVdbBlastErr : eVdbBlastNoErr;;
}

/******************************************************************************/

typedef struct {
    const VCursor *curs;
    uint32_t colREAD_LEN;
    uint32_t colREAD_TYPE;
    uint64_t techBasesPerSpot;
    bool techBasesPerSpotEquals;
    uint64_t bioBasesCnt;
} ApprCnt;
static rc_t _ApprCntInit(ApprCnt *self, const VTable *tbl, const char *acc) {
    rc_t rc = 0;

    assert(self);

    memset(self, 0, sizeof *self);

    self->techBasesPerSpotEquals = true;
    self->techBasesPerSpot = ~0;

    if (rc == 0) {
        rc = VTableCreateCursorRead(tbl, &self->curs);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VTableCreateCursorRead for $(acc)",
                "acc=%s", acc));
        }
    }

    if (rc == 0) {
        const char name[] = "READ_LEN";
        rc = VCursorAddColumn(self->curs, &self->colREAD_LEN, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }

    if (rc == 0) {
        const char name[] = "READ_TYPE";
        rc = VCursorAddColumn(self->curs, &self->colREAD_TYPE, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }

    if (rc == 0) {
        rc = VCursorOpen(self->curs);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error during VCursorOpen");
        }
    }

    return rc;
}

static rc_t _ApprCntFini(ApprCnt *self) {
    rc_t rc = 0;
    assert(self);
    RELEASE(VCursor, self->curs);
    memset(self, 0, sizeof *self);
    return rc;
}

static rc_t _ApprCntChunk(ApprCnt *self,
    uint64_t chunk, uint64_t l, uint64_t nspots, uint32_t nreads)
{
    rc_t rc = 0;
    uint64_t start = nspots / 10 * chunk + 1;
    uint64_t spot = 0;
    uint64_t end = start + l;
    assert(self);
    if (end - 1 > nspots) {
        end = nspots + 1;
    }
    for (spot = start; spot < end; ++spot) {
        uint64_t techBases = 0;
        uint64_t bioBases = 0;
        uint32_t read = 0;
        const uint32_t *readLen = NULL;
        const INSDC_read_type *readType = NULL;
        const void *base = NULL;
        rc = _VCursorCellDataDirect(self->curs, spot, self->colREAD_LEN,
            32, &base, nreads, "READ_LEN");
        if (rc != 0) {
            return rc;
        }
        readLen = base;
        rc = _VCursorCellDataDirect(self->curs, spot, self->colREAD_TYPE,
            8, &base, nreads, "READ_TYPE");
        if (rc != 0) {
            return rc;
        }
        readType = base;
        for (read = 0; read < nreads; ++read) {
            INSDC_read_type type = readType[read] & 1;
            if (type == READ_TYPE_BIOLOGICAL) {
                bioBases += readLen[read];
            }
            else {
                techBases += readLen[read];
            }
        }
        if (self->techBasesPerSpotEquals) {
            if (self->techBasesPerSpot != techBases) {
                if (self->techBasesPerSpot == ~0) {
                    self->techBasesPerSpot = techBases;
                }
                else {
                    self->techBasesPerSpotEquals = false;
                }
            }
        }
        self->bioBasesCnt += bioBases;
    }
    return rc;
}

/******************************************************************************/

static rc_t _VTableLogRowData(const VTable *self,
    const char *column, void *buffer, uint32_t blen)
{
    rc_t rc = 0;

#if _DEBUGGING
    if (buffer && blen == 64) {
        uint64_t data = *((uint64_t*)buffer);
        const KTable *ktbl = NULL;
        rc_t rc = VTableOpenKTableRead(self, &ktbl);
        if (rc == 0) {
            const char *path = NULL;
            rc = KTableGetPath(ktbl, &path);
            if (rc == 0) {
                DBGMSG(DBG_BLAST, DBG_FLAG(DBG_BLAST_BLAST),
                    ("%s: %s: %lu\n", path, column, data));
            }
        }

        KTableRelease(ktbl);
    }
#endif

    if (rc != 0)
    {   PLOGERR(klogInt, (klogInt, rc, "Error in $(f)", "f=%s", __func__)); }

    return rc;
}

static rc_t _VTableMakeCursorImpl(const VTable *self, const VCursor **curs,
    uint32_t *col_idx, const char *col_name, bool canBeMissed, const char *acc)
{
    rc_t rc = 0;

    assert(curs && col_name);

    if (rc == 0) {
        rc = VTableCreateCursorRead(self, curs);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error during VTableCreateCursorRead");
        }
    }

    if (rc == 0) {
        VCursorPermitPostOpenAdd(*curs);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error during VCursorPermitPostOpenAdd");
        }
    }

    if (rc == 0) {
        rc = VCursorOpen(*curs);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpen($(name))", "name=%s", col_name));
        }
    }

    if (rc == 0) {
        assert(*curs);
        rc = VCursorAddColumn(*curs, col_idx, "%s", col_name);
        if (rc != 0 && !canBeMissed) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorAddColumn($(name))", "name=%s", col_name));
        }
    }

    STSMSG(2, ("Prepared a VCursor to read '%s'", col_name));

    return rc;
}

rc_t _VTableMakeCursor(const VTable *self, const VCursor **curs,
    uint32_t *col_idx, const char *col_name, const char *acc)
{
    return _VTableMakeCursorImpl(self, curs, col_idx, col_name, false, acc);
}

/*static*/
uint32_t _VTableReadFirstRowImpl(const VTable *self, const char *name,
    void *buffer, uint32_t blen, EColType *is_static, bool canBeMissed,
    uint32_t *row_len, const char *acc)
{
    VdbBlastStatus status = eVdbBlastNoErr;

    rc_t rc = 0;

    const VCursor *curs = NULL;
    uint32_t idx = 0;

    uint32_t dummy = 0;
    if (row_len == NULL) {
        row_len = &dummy;
    }

    assert(self && name);

    blen *= 8;

    rc = _VTableMakeCursorImpl(self, &curs, &idx, name, canBeMissed, acc);
    if (rc != 0) {
        if (rc ==
            SILENT_RC(rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined)
         || rc ==
            SILENT_RC(rcVDB, rcCursor, rcUpdating, rcColumn, rcNotFound))
        {
            if (!canBeMissed) {
                PLOGMSG(klogInfo, (klogInfo, "$(f): Column '$(name)' not found",
                    "f=%s,name=%s", __func__, name));
            }
            if (is_static != NULL) {
                *is_static = eColTypeAbsent;
            }
            status = eVdbBlastTooExpensive;
        }
        else {
            status = eVdbBlastErr;
            if (is_static != NULL) {
                *is_static = eColTypeError;
            }
        }
    }

    if (status == eVdbBlastNoErr && rc == 0) {
        rc = VCursorOpenRow(curs);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorOpenRow($(name))", "name=%s", name));
        }
    }

    if (status == eVdbBlastNoErr && rc == 0 && buffer != NULL && blen > 0) {
        rc = VCursorRead(curs, idx, 8, buffer, blen, row_len);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error in VCursorRead($(name))", "name=%s", name));
        }
    }

/* TODO: needs to be verified what row_len is expected
    if (*row_len != 1) return eVdbBlastErr; */

    STSMSG(2, ("Read the first row of '%s'", name));

    if (status == eVdbBlastNoErr && rc == 0) {
        if (blen == 64) {
            _VTableLogRowData(self, name, buffer, blen);
        }
        if (is_static != NULL) {
            bool isStatic = false;
            rc = VCursorIsStaticColumn(curs, idx, &isStatic);
            if (rc != 0) {
                PLOGERR(klogInt, (klogInt, rc,
                    "Error in VCursorIsStaticColumn($(name))",
                    "name=%s", name));
            }
            else {
                *is_static = isStatic ? eColTypeStatic : eColTypeNonStatic;
            }
        }
    }

    VCursorRelease(curs);

    if (status == eVdbBlastNoErr && rc != 0) {
        status = eVdbBlastErr;
    }
    return status;
}

static VdbBlastStatus _VTableReadFirstRow(const VTable *self, const char *name,
    void *buffer, uint32_t blen, EColType *is_static, const char *acc)
{
    return _VTableReadFirstRowImpl
        (self, name, buffer, blen, is_static, false, NULL, acc);
}

static INSDC_SRA_platform_id _VTableReadPLATFORMoptional
    (const VTable *self, const char *acc)
{
    INSDC_SRA_platform_id platform = SRA_PLATFORM_UNDEFINED;

    VdbBlastStatus status = _VTableReadFirstRowImpl(self,
        "PLATFORM", &platform, sizeof platform, NULL, true, NULL, acc);

    if (status == eVdbBlastNoErr) {
        return platform;
    }
    else {
        return SRA_PLATFORM_UNDEFINED;
    }
}

static bool _VTableVarReadNum(const VTable *self, const char *acc) {
    return _VTableReadPLATFORMoptional(self, acc) == SRA_PLATFORM_PACBIO_SMRT;
}

static uint64_t BIG = 10000 /*11*/; 
static rc_t _VTableBioBaseCntApprox(const VTable *self,
    uint64_t nspots, uint32_t nreads, uint64_t *bio_base_count, const char *acc)
{
    rc_t rc = 0;
    uint64_t c = nspots;
    uint64_t d = 1;
    ApprCnt ac;
    rc = _ApprCntInit(&ac, self, acc);
    assert(bio_base_count);
    if (rc == 0) {
        if (nspots < BIG) {
            STSMSG(2,
                ("VdbBlastRunSetGetTotalLengthApprox: counting all reads\n"));
            rc = _ApprCntChunk(&ac, 0, nspots, nspots, nreads);
        }
        else {
            uint64_t i = 0;
            uint64_t l = 0;
            uint64_t n = 10 /* 3 */;
            while (c > BIG) {
                c /= 2;
                d *= 2;
            }
            l = c / n;
            if (l == 0) {
                l = 1;
            }
            for (i = 1; i < n - 1 && rc == 0; ++i) {
                rc = _ApprCntChunk(&ac, i, l, nspots, nreads);
            }
        }
    }
    if (rc == 0) {
        VdbBlastStatus status = eVdbBlastNoErr;
        if (nspots < BIG) {
            *bio_base_count = ac.bioBasesCnt;
        }
        else {
            if (ac.techBasesPerSpotEquals && ac.techBasesPerSpot == ~0) {
                ac.techBasesPerSpotEquals = false;
            }
            if (ac.techBasesPerSpotEquals ) {
                uint64_t baseCount = 0;
                status = _VTableReadFirstRow(self,
                    "BASE_COUNT", &baseCount, sizeof baseCount, NULL, acc);
                if (status == eVdbBlastNoErr) {
                    STSMSG(2, ("VdbBlastRunSetGetTotalLengthApprox: "
                        "fixed technical reads length\n"));
                    *bio_base_count = baseCount - nspots * ac.techBasesPerSpot;
                }
                else {
                    STSMSG(1, ("VdbBlastRunSetGetTotalLengthApprox: "
                        "cannot read BASE_COUNT\n"));
                }
            }
            if (!ac.techBasesPerSpotEquals || status != eVdbBlastNoErr) {
/*              double dd = nspots / c; */
                STSMSG(2, ("VdbBlastRunSetGetTotalLengthApprox: "
                    "extrapolating variable read length\n"));
                *bio_base_count = ac.bioBasesCnt * d;
            }
        }
    }
    _ApprCntFini(&ac);
    return rc;
}

static
uint32_t _VTableGetNReads(const VTable *self, uint32_t *nreads, const char *acc)
{
    rc_t rc = 0;
    VdbBlastStatus status = eVdbBlastNoErr;
    const VCursor *curs = NULL;
    uint32_t idx = 0;

    const char name[] = "READ_LEN";

    assert(nreads);

    rc = _VTableMakeCursor(self, &curs, &idx, name, acc);
    if (rc != 0) {
        status = eVdbBlastErr;
        if (rc ==
            SILENT_RC(rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined))
        {
            PLOGMSG(klogInfo, (klogInfo, "$(f): Column '$(name)' not found",
                "f=%s,name=%s", __func__, name));
        }
        else {
            PLOGMSG(klogInfo, (klogInfo, "$(f): Cannot open column '$(name)'",
                "f=%s,name=%s", __func__, name));
        }
    }

    if (status == eVdbBlastNoErr) {
        uint32_t elem_bits, elem_off, elem_cnt;
        const void *base = NULL;
        rc = VCursorCellDataDirect(curs, 1, idx,
                &elem_bits, &base, &elem_off, &elem_cnt);
        if (rc != 0) {
            status = eVdbBlastErr;
            PLOGMSG(klogInfo, (klogInfo,
                "$(f): Cannot '$(name)' CellDataDirect",
                "f=%s,name=%s", __func__, name));
        }
        else if (elem_off != 0 || elem_bits != 32) {
            status = eVdbBlastErr;
            PLOGERR(klogInt, (klogInt, rc,
                "Bad VCursorCellDataDirect(READ_LEN) result: "
                "boff=$(elem_off), elem_bits=$(elem_bits)",
                "elem_off=%u, elem_bits=%u", elem_off, elem_bits));
        }
        else {
            *nreads = elem_cnt;
        }
    }
    
    RELEASE(VCursor, curs);

    return status;
}

static bool _VTableCSra(const VTable *self) {
    bool cSra = false;

    KNamelist *names = NULL;

    uint32_t i = 0, count = 0;

    rc_t rc = VTableListPhysColumns(self, &names);

    if (rc == 0) {
        rc = KNamelistCount(names, &count);
    }

    for (i = 0 ; i < count && rc == 0; ++i) {
        const char *name = NULL;
        rc = KNamelistGet(names, i, &name);
        if (rc == 0) {
            const char b[] = "CMP_READ";
            if (string_cmp(name, string_measure(name, NULL),
                b, sizeof b - 1, sizeof b - 1) == 0)
            {
                cSra = true;
                break;
            }
        }
    }

    RELEASE(KNamelist, names);

    return cSra;
}

/******************************************************************************/

static void _RunDescFini(RunDesc *self) {
    assert(self);
    free(self->readLen);
    free(self->readType);
    free(self->rdFilter);
    memset(self, 0, sizeof *self);
}

static
uint64_t _RunDescMakeReadId(const RunDesc *self, uint64_t spot, uint8_t read,
    uint64_t read_id, VdbBlastStatus *status)
{
    bool overflow = false;

    assert(self && status);

    *status = eVdbBlastNoErr;

    if (self->readIdDesc.idType == eFixedReadN) {
        return read_id;
    }

    assert(self->readIdDesc.idType == eFactor10);

    read_id = read * self->spotBits + spot;
    if (read_id < self->spotBits) {
        overflow = true;
    }

    if (self->readIdDesc.runBits > 0) {
        uint64_t prev = read_id;
        read_id *= self->readIdDesc.runBits;
        read_id += self->index;
        if (read_id < prev) {
            overflow = true;
        }
    }

    if (overflow) {
        *status = eVdbBlastErr;
    }

    return read_id;
}

/******************************************************************************/
/* VdbBlastDb */
static void _VdbBlastDbWhack(VdbBlastDb *self) {
    if (self == NULL) {
        return;
    }

    VCursorRelease(self->cursACCESSION);
    VCursorRelease(self->cursSeq);

    VTableRelease(self->prAlgnTbl);
    VTableRelease(self->refTbl);
    VTableRelease(self->seqTbl);

    VDatabaseRelease(self->db);

    memset(self, 0, sizeof *self);

    free(self);
}

static rc_t _VdbBlastDbOpenSeqCurs(VdbBlastDb *self, const char *acc) {
    rc_t rc = 0;
    const VCursor *curs = NULL;
    assert(self);
    if (self->cursSeq != NULL) {
        return 0;
    }
    rc = VTableCreateCursorRead(self->seqTbl, &self->cursSeq);
    if (rc != 0) {
        PLOGERR(klogInt, (klogInt, rc,
            "Error during VTableCreateCursorRead for $(acc)", "acc=%s", acc));
    }
    else {
        curs = self->cursSeq;
    }
    if (rc == 0) {
        const char name[] = "READ_FILTER";
        rc = VCursorAddColumn(curs, &self->col_READ_FILTER, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }
    if (rc == 0) {
        const char name[] = "READ_LEN";
        rc = VCursorAddColumn(curs, &self->col_READ_LEN, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }
    if (rc == 0) {
        const char name[] = "READ_TYPE";
        rc = VCursorAddColumn(curs, &self->col_READ_TYPE, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }
    if (rc == 0) {
        const char name[] = "TRIM_LEN";
        rc = VCursorAddColumn(curs, &self->col_TRIM_LEN, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }
    if (rc == 0) {
        const char name[] = "TRIM_START";
        rc = VCursorAddColumn(curs, &self->col_TRIM_START, name);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during VCursorAddColumn($(name))", "name=%s", name));
        }
    }
    if (rc == 0) {
        rc = VCursorOpen(curs);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error during VCursorOpen");
        }
    }
    return rc;
}

typedef enum {
    eFirstRead,
    eNextRead,
    eAllReads,
} TReadType;
static rc_t _VdbBlastDbFindRead
    (const VdbBlastDb *self, ReadDesc *rd, TReadType type, bool *found)
{
    rc_t rc = 0;

    int64_t first = 0;
    uint64_t count = 0;

    const VCursor *curs = NULL;

    assert(self && rd && found);

    *found = false;

    if (type == eAllReads) {
        memset(rd, 0, sizeof *rd);
    }

    curs = self->cursSeq;

    rc = VCursorIdRange(curs, self->col_READ_LEN, &first, &count);

    if (rc == 0) {
        int64_t i = 0;
        uint32_t c = 0;
        if (type == eNextRead) {
            if (rd->spot < first) {
                return -1;
            }
            i = rd->spot - first;
            c = rd->read;
            if (c + 1 >= rd->nReads) {
                c = 0;
                ++i;
            }
        }
        for (; i < count; ++i) {
            int64_t spot = first + i;

            const void *base = NULL;
            uint32_t elem_bits = 0;
            uint32_t elem_cnt = 0;
            uint32_t elem_off = 0;
            rc = VCursorCellDataDirect(curs, spot, self->col_READ_TYPE,
                &elem_bits, &base, &elem_off, &elem_cnt);
            if (rc != 0) {
                break;
            }
            else {
                INSDC_read_type *aRt = (INSDC_read_type*)base;
                for (; c < elem_cnt; ++c) {
                    INSDC_read_type *rt = aRt + c;
                    assert(rt);

                    if (*rt & SRA_READ_TYPE_BIOLOGICAL) {
                        *found = true;
                        if (type == eAllReads) {
                            ++rd->spot;
                        }
                        else {
                            rd->spot = spot;
                            rd->read = c + 1;
                            rd->nReads = elem_cnt;
                            return rc;
                        }
                    }
                }
                c = 0;
            }
        }
    }

    return rc;
}

static rc_t _VdbBlastDbFindFirstRead
    (const VdbBlastDb *self, ReadDesc *rd, bool *found)
{   return _VdbBlastDbFindRead(self, rd, eFirstRead, found); }
static rc_t _VdbBlastDbFindNextRead
    (const VdbBlastDb *self, ReadDesc *rd, bool *found)
{   return _VdbBlastDbFindRead(self, rd, eNextRead , found); }

rc_t _ReadDescFindNextRead(ReadDesc *self, bool *found) {
    assert(self && self->run && self->run);
    return _VdbBlastDbFindNextRead(self->run->obj, self, found);
}

static rc_t _VdbBlastDbOpenCursAndGetFirstRead
    (VdbBlastDb *self, ReadDesc *desc, bool *found, const char *acc)
{
    rc_t rc = _VdbBlastDbOpenSeqCurs(self, acc);
    if (rc == 0) {
        rc = _VdbBlastDbFindFirstRead(self, desc, found);
    }
    return rc;
}

static rc_t _VdbBlastDbOpenCursAndGetAllReads
    (VdbBlastDb *self, ReadDesc *desc, const char *acc)
{
    rc_t rc = _VdbBlastDbOpenSeqCurs(self, acc);
    if (rc == 0) {
        bool found = false;
        rc = _VdbBlastDbFindRead(self, desc, eAllReads, &found);
    }
    return rc;
}

uint64_t _VdbBlastRunSetGetAllReads
    (const VdbBlastRunSet *self, uint32_t run)
{
    ReadDesc desc;

    assert(self);

    if (run >= self->runs.krun) {
        return 0;
    }

    if (_VdbBlastDbOpenCursAndGetAllReads
        (self->runs.run[run].obj, &desc, self->runs.run[run].acc))
    {
        return 0;
    }

    return desc.spot;
}

static rc_t _VdbBlastDbGetNReads
    (VdbBlastDb *self, uint64_t spot, uint32_t *nReads, const char *acc)
{
    rc_t rc = _VdbBlastDbOpenSeqCurs(self, acc);
    assert(self && nReads);
    *nReads = 0;
    if (rc == 0) {
        const void *base = NULL;
        uint32_t elem_bits = 0;
        uint32_t elem_cnt = 0;
        uint32_t elem_off = 0;
        rc = VCursorCellDataDirect(self->cursSeq, spot, self->col_READ_TYPE,
            &elem_bits, &base, &elem_off, &elem_cnt);
        if (rc == 0) {
            *nReads = elem_cnt;
        }
        else {
            PLOGERR(klogInt, (klogInt, rc, "Cannot get "
                "CellDataDirect for $(acc)/READ_TYPE/$(spot)",
                "acc=%s,spot=%lu", acc, spot));
        }
    }
    return rc;
}

/******************************************************************************/
/*VdbBlastRun*/

static void _VdbBlastRunFini(VdbBlastRun *self) {
    if (self == NULL) {
        return;
    }

    _VdbBlastDbWhack(self->obj);

    free(self->acc);
    free(self->path);

    _RunDescFini(&self->rd);

    memset(self, 0, sizeof *self);
}

static VdbBlastStatus _VdbBlastRunInit(VdbBlastRun *self, VdbBlastDb *obj,
    const char *rundesc, BTableType type, const KDirectory *dir,
    char *fullpath, uint32_t min_read_length, uint32_t index)
{
    rc_t rc = 0;
    const char *acc = rundesc;
    char rbuff[4096] = "";
    size_t size = 0;

    char slash = '/';

    assert(!dir && self && obj && type != btpUndefined && rundesc);

    {
        KDirectory *dir = NULL;
        if (KDirectoryNativeDir(&dir) != 0) {
            LOGERR(klogInt, rc, "Error during KDirectoryNativeDir");
            return eVdbBlastErr;
        }
/* TODO This is obsolete and incorrect */
        rc = KDirectoryResolvePath(dir, true,
            rbuff, sizeof rbuff, "%s", rundesc);
        KDirectoryRelease(dir);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc,
                "Error during KDirectoryResolvePath($(path))",
                "path=%s", rundesc));
            return eVdbBlastErr;
        }
    }

    memset(self, 0, sizeof *self);

    self->rd.index = index;
    self->obj = obj;
    self->type = type;

    self->alignments
        = self->bioBases = self->bioBasesApprox
        = self->bioReads = self->bioReadsApprox = ~0;

    acc = strrchr(rbuff, slash);
    if (acc == NULL) {
        acc = rbuff;
    }
    else if (string_measure(acc, &size) > 1) {
        ++acc;
    }
    else {
        acc = rbuff;
    }

    if (fullpath == NULL) {
        self->path = string_dup(rbuff, sizeof(rbuff));
        if (self->path == NULL) {
            return eVdbBlastMemErr;
        }
    }
    else {
        self->path = fullpath;
    }
    self->acc = _CanonocalName(acc);
    if (self->acc == NULL) {
        return eVdbBlastMemErr;
    }

    self->min_read_length = min_read_length;

    self->cSra = _VTableCSra(self->obj->seqTbl);

    return eVdbBlastNoErr;
}

static uint32_t Bits(uint64_t n, EReadIdType idType) {
    uint32_t bits = 1;

    if (idType == eFixedReadN) {
        return 0;
    }

    assert(idType == eFactor10);

    if (n == 0) {
        return 0;
    }

    while (n > 0) {
        bits *= 10;
        n /= 10;
    }

    return bits;
}

/*static*/ VdbBlastStatus _VdbBlastRunFillRunDesc(VdbBlastRun *self) {
    VdbBlastStatus status = eVdbBlastNoErr;
    RunDesc *rd = NULL;

    int i = 0;
    const char *col = NULL;

    assert(self);

    rd = &self->rd;

    if (rd->spotCount || rd->readType || rd->nReads || rd->nBioReads) {
        if (self->cSra && rd->cmpBaseCount == ~0) {
            rc_t rc = RC(rcSRA, rcTable, rcReading, rcColumn, rcNotFound);
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot read CMP_BASE_COUNT column for $(p)",
                "p=%s", self->path));
            STSMSG(1, ("Error: failed to read %s/%s",
                self->path, "CMP_BASE_COUNT"));
            return eVdbBlastErr;
        }
        else {
            S
            return eVdbBlastNoErr;
        }
    }

    assert(self->path && self->obj);

    col = "SPOT_COUNT";
    status = _VTableReadFirstRow(self->obj->seqTbl,
        col, &rd->spotCount, sizeof rd->spotCount, NULL, self->acc);
    if (status != eVdbBlastNoErr) {
        STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
        return status;
    }

    rd->spotBits = Bits(rd->spotCount, rd->readIdDesc.idType);

    if (self->type == btpWGS) {
        S
        status = eVdbBlastNoErr;
        rd->nReads = rd->spotCount > 0 ? 1 : 0;
    }
    else if (self->type == btpREFSEQ) {
        S
        status = eVdbBlastNoErr;
        rd->nReads = 1;
    }
    else {
        uint32_t nreads = 0;
        status = _VTableGetNReads(self->obj->seqTbl, &nreads, self->acc);
        if (status == eVdbBlastNoErr) {
            rd->nReads = nreads;
        }
    }

    switch (self->type) {
        case btpSRA:
            col = "PLATFORM";
            status = _VTableReadFirstRow(self->obj->seqTbl,
                col, &rd->platform, sizeof rd->platform, NULL, self->acc);
            if (status != eVdbBlastNoErr) {
                STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
                return status;
            }
            switch (rd->platform) { /* TODO */
                case SRA_PLATFORM_ILLUMINA:
                case SRA_PLATFORM_ABSOLID:
                case SRA_PLATFORM_COMPLETE_GENOMICS:
                    rd->varReadLen = false;
                    break;
                case SRA_PLATFORM_UNDEFINED:
                case SRA_PLATFORM_454:
                case SRA_PLATFORM_HELICOS:
                case SRA_PLATFORM_PACBIO_SMRT:
                case SRA_PLATFORM_ION_TORRENT:
                case SRA_PLATFORM_CAPILLARY:
                case SRA_PLATFORM_OXFORD_NANOPORE:
                default:
                    rd->varReadLen = true;
                    break;
            }
            break;
        case btpWGS:
            rd->varReadLen = true;
            break;
        case btpREFSEQ:
            break;
        default:
            assert(0);
            break;
    }

    col = "READ_TYPE";
    if (rd->readType == NULL) {
        rd->readType = calloc(rd->nReads, sizeof *rd->readType);
        if (rd->readType == NULL)
        {   return eVdbBlastMemErr; }
    }
    {
        bool optional = self->type == btpREFSEQ;
        status = _VTableReadFirstRowImpl(self->obj->seqTbl, col, rd->readType,
            rd->nReads * sizeof *rd->readType, &rd->readTypeStatic,
            optional, NULL, self->acc);
    }
    /* TODO: check case when ($#READ_TYPE == 0 && nreads > 0) */
    if (status != eVdbBlastNoErr) {
        if (status == eVdbBlastTooExpensive) {
            status = eVdbBlastNoErr;
        }
        else {
            STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
            return status;
        }
    }

    col = "READ_LEN";
    if (rd->readLen == NULL) {
        rd->readLen = calloc(rd->nReads, sizeof *rd->readLen);
        if (rd->readLen == NULL)
        {   return eVdbBlastMemErr; }
    }
    status = _VTableReadFirstRow(self->obj->seqTbl, col, rd->readLen,
        rd->nReads * sizeof *rd->readLen, &rd->readLenStatic, self->acc);
    /* TODO: check case when ($#READ_TYPE == 0 && nreads > 0) */
    if (status != eVdbBlastNoErr) {
        STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
        return status;
    }

    col = "READ_FILTER"; /* col = "RD_FILTER"; */
    if (rd->rdFilter == NULL) {
        rd->rdFilter = calloc(rd->nReads, sizeof *rd->rdFilter);
        if (rd->rdFilter == NULL)
        {   return eVdbBlastMemErr; }
    }
    status = _VTableReadFirstRow(self->obj->seqTbl, col, rd->rdFilter,
        rd->nReads * sizeof *rd->rdFilter, &rd->rdFilterStatic, self->acc);
    if (status != eVdbBlastNoErr) {
        if (status == eVdbBlastTooExpensive) {
            status = eVdbBlastNoErr;
        }
        else {
            STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
            return status;
        }
    }

    col = "BIO_BASE_COUNT";
    status = _VTableReadFirstRowImpl(self->obj->seqTbl, col, &rd->bioBaseCount,
        sizeof rd->bioBaseCount, NULL, true, NULL, self->acc);
     /* Do not generate error message when BIO_BASE_COUNT is not found */

    if (status != eVdbBlastNoErr) {
        if (status == eVdbBlastTooExpensive) {
            status = eVdbBlastNoErr;
            rd->bioBaseCount = ~0;
        }
        else {
            STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
            return status;
        }
    }

    col = "CMP_BASE_COUNT";
    status = _VTableReadFirstRow(self->obj->seqTbl, col,
        &rd->cmpBaseCount, sizeof rd->cmpBaseCount, NULL, self->acc);
    if (status != eVdbBlastNoErr) {
        if (status == eVdbBlastTooExpensive) {
            /* CMP_BASE_COUNT should be always found */
            rc_t rc = RC(rcSRA, rcTable, rcReading, rcColumn, rcNotFound);
            PLOGERR(klogInt, (klogInt, rc,
                "Cannot read CMP_BASE_COUNT column for $(p)",
                "p=%s", self->path));
            STSMSG(1, ("Error: failed to read %s/%s",
                self->path, "CMP_BASE_COUNT"));
            rd->cmpBaseCount = ~0;
            return eVdbBlastErr;
        }
        else {
            STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
            return status;
        }
    }

    if (self->type == btpREFSEQ) {
        rd->nBioReads = 1;
        /* rd->bioLen is set incorrectly for refseq-s */
    }
    else {
        for (rd->nBioReads = 0, i = 0; i < rd->nReads; ++i) {
            S
            if (rd->readType[i] & SRA_READ_TYPE_BIOLOGICAL) {
                if ((rd->rdFilterStatic == eColTypeStatic &&
                     rd->rdFilter[i] == READ_FILTER_PASS) 
                    || (rd->rdFilterStatic == eColTypeAbsent))
                {
                    ++rd->nBioReads;
                    rd->bioLen += rd->readLen[i];
                }
                else {
                    ++rd->nBioReads;
                }
            }
        }
    }
    S /* LOG nBioReads */

    return status;
}

bool _VdbBlastRunVarReadNum(const VdbBlastRun *self) {
    VdbBlastStatus status = _VdbBlastRunFillRunDesc((VdbBlastRun*)self);
    if (status != eVdbBlastNoErr) {
        LOGERR(klogInt, 1, "Cannot fill run descriptor");
    }

    assert(self);

    return self->rd.platform == SRA_PLATFORM_PACBIO_SMRT;
}

/* _VdbBlastRunGetNumSequences
    returns (number of spots) * (number of biological reads in spot).
    If read_filter is not static: some reads could be filtered,
    so status is set to eVdbBlastTooExpensive */
/*static*/
uint64_t _VdbBlastRunGetNumSequences(VdbBlastRun *self,
    VdbBlastStatus *status)
{
    assert(self && status);

    *status = eVdbBlastNoErr;

    if (self->bioReads == ~0) {
        RunDesc *rd = NULL;

        if (self->type == btpREFSEQ) {
            S
            self->bioReads = 1;
        }
        else if (_VdbBlastRunVarReadNum(self)) {
            rc_t rc = 0;
            ReadDesc desc;
            memset(&desc, 0, sizeof desc);
            rc = _VdbBlastDbOpenCursAndGetAllReads(self->obj, &desc, self->acc);
            if (rc != 0) {
                *status = eVdbBlastErr;
            }
            else {
                self->bioReads = desc.spot;
            }
        }
        else {
            *status = _VdbBlastRunFillRunDesc(self);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }

            rd = &self->rd;

            if (rd->rdFilterStatic != eColTypeStatic) {
                self->bioReadsTooExpensive = true;
            }

            if (self->cSra) {
                self->bioReadsTooExpensive = true;
            }

            if (_VdbBlastRunVarReadNum(self)) {
                self->bioReads = 0;
                S
            }
            else {
                self->bioReads = rd->spotCount * rd->nBioReads;
                S
            }
        }
    }
    else {
        S
    }

    if (*status == eVdbBlastNoErr && self->bioReadsTooExpensive) {
        *status = eVdbBlastTooExpensive;
    }
    return self->bioReads;
}

static uint64_t _VdbBlastRunCountBioBaseCount(VdbBlastRun *self,
    VdbBlastStatus *status)
{
    uint64_t bio_base_count = 0;

    rc_t rc = _VTableBioBaseCntApprox(self->obj->seqTbl,
        self->rd.spotCount, self->rd.nReads, &bio_base_count, self->acc);

    if (rc != 0) {
        *status = eVdbBlastErr;
    }

    return bio_base_count;
}

static uint64_t _VdbBlastSraRunGetLengthApprox(VdbBlastRun *self,
    VdbBlastStatus *status)
{
    assert(self && status);

    *status = eVdbBlastNoErr;

    if (self->bioBasesApprox == ~0) {
        if (! self->bioBasesTooExpensive && self->bioBases != ~0) {
            self->bioBasesApprox = self->bioBases;
        }
        else {
            RunDesc *rd = NULL;
            *status = _VdbBlastRunFillRunDesc(self);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }

            rd = &self->rd;
            if (rd->nReads == 0) {
                S
                self->bioBasesApprox = 0;
            }
            else if (rd->varReadLen) {
                S
                self->bioBasesApprox
                    = _VdbBlastRunCountBioBaseCount(self, status);
            }
            else {
                if (self->type == btpREFSEQ) {
                    if (rd->bioBaseCount == ~0) {
                        S
                        *status = eVdbBlastErr;
                    }
                    else {
                        self->bioBasesApprox = rd->bioBaseCount;
                    }
                }
                else {
                    uint8_t read = 0;
                    S
                    for (read = 0, self->bioBasesApprox = 0;
                        read < rd->nReads; ++read)
                    {
                        if (rd->readType[read] & SRA_READ_TYPE_BIOLOGICAL) {
                            self->bioBasesApprox += rd->readLen[read];
                        }
                    }
                    self->bioBasesApprox *= rd->spotCount;
                }
            }
        }
    }

    return self->bioBasesApprox;
}

static uint64_t _VdbBlastRunGetNumSequencesApprox(VdbBlastRun *self,
    VdbBlastStatus *status)
{

    assert(self && status);

    *status = eVdbBlastNoErr;

    if (self->bioReadsApprox == ~0) {
        RunDesc *rd = NULL;

        if (! self->bioReadsTooExpensive && self->bioReads != ~0) {
            self->bioReadsApprox = self->bioReads;
        }
        else if (self->type == btpREFSEQ) {
            S
            self->bioReadsApprox = 1;
        }
        else if (self->cSra) {
/* Number of Bio Reads for cSra == number of CMP reads
    = Number of all bio reads * CMP_BASE_COUNT / BIO_BASE_COUNT
   Number of all bio reads = nSpots * n of bio reads per spot */
            double r = 0;
            uint64_t n = 0;

            *status = _VdbBlastRunFillRunDesc(self);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }

            rd = &self->rd;

            n = _VdbBlastSraRunGetLengthApprox(self, status);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }
            r = rd->cmpBaseCount * rd->spotCount * rd->nBioReads;
            r /= n;
            self->bioReadsApprox = r;
        }
        else if (_VdbBlastRunVarReadNum(self)) {
            self->bioReads = _VdbBlastRunGetNumSequences(self, status);
        }
        else {
            *status = _VdbBlastRunFillRunDesc(self);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }

            rd = &self->rd;

            self->bioReadsApprox = rd->spotCount * rd->nBioReads;
            S
        }
    }

    return self->bioReadsApprox;
}

static uint64_t _VdbBlastRunGetLength
    (VdbBlastRun *self, VdbBlastStatus *status)
{
    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    *status = eVdbBlastNoErr;

    if (self->bioBasesTooExpensive) {
        *status = eVdbBlastTooExpensive;
        return 0;
    }
    else if (self->bioBases == ~0) {
        if (self->cSra) {
            *status = _VdbBlastRunFillRunDesc(self);
            if (*status != eVdbBlastNoErr) {
                S
                return 0;
            }
            self->bioBases = self->rd.cmpBaseCount;
        }
        else {
/* if BIO_BASE_COUNT is not found then status is set to eVdbBlastTooExpensive */
            *status = _VTableReadFirstRowImpl(self->obj->seqTbl,
                "BIO_BASE_COUNT", &self->bioBases,
                sizeof self->bioBases, NULL, true, NULL, self->acc);
            if (*status == eVdbBlastTooExpensive) {
                self->bioBasesTooExpensive = true;
            }
        }
    }

    if (*status == eVdbBlastNoErr) {
        S
        return self->bioBases;
    }
    else {
        S
        return 0;
    }
}

static uint64_t _VdbBlastRunGetLengthApprox(VdbBlastRun *self,
    VdbBlastStatus *status)
{
    if (self->cSra) {
        return _VdbBlastRunGetLength(self, status);
    }
    else {
        return _VdbBlastSraRunGetLengthApprox(self, status);
    }
}

static uint64_t _VdbBlastRunScan(const VdbBlastRun *self,
    uint64_t (*cmp)
        (uint64_t cand, uint64_t champ, int64_t minRead, bool *done),
    uint64_t minRead, uint64_t start, VdbBlastStatus *status)
{
    uint64_t res = start;
    uint64_t bad = start;
    uint64_t spot = 0;
    bool done = false;
    rc_t rc = 0;
    const VCursor *curs = NULL;
    uint32_t idxREAD_LEN = 0;
    uint32_t idxREAD_TYPE = 0;
    assert(self && status &&
        (self->rd.spotCount || self->rd.readType ||
         self->rd.nReads    || self->rd.nBioReads));
    rc = _VTableMakeCursor
        (self->obj->seqTbl, &curs, &idxREAD_LEN, "READ_LEN", self->acc);
    if (rc != 0) {
        return bad;
    }
    if (_VdbBlastRunVarReadNum(self)) {
        rc = VCursorAddColumn(curs, &idxREAD_TYPE, "READ_TYPE");
        if (rc != 0) {
            return bad;
        }
    }
    for (spot = 1;
        spot <= self->rd.spotCount && *status == eVdbBlastNoErr && !done;
        ++spot)
    {
        uint32_t elem_bits, elem_off, elem_cnt;
        const void *base = NULL;
        if (_VdbBlastRunVarReadNum(self)) {
            rc = VCursorCellDataDirect(curs, spot, idxREAD_TYPE,
                &elem_bits, &base, &elem_off, &elem_cnt);
            if (rc != 0) {
                *status = eVdbBlastErr;
                PLOGMSG(klogInfo, (klogInfo,
                    "$(f): Cannot '$(name)' CellDataDirect",
                    "f=%s,name=%s", __func__, "READ_TYPE"));
                res = bad;
                break;
            }
            else {
                INSDC_read_type *aRt = (INSDC_read_type*)base;
                uint32_t c = 0;
                for (c = 0; c < elem_cnt; ++c) {
                    INSDC_read_type *rt = aRt + c;
                    assert(rt);

                    if (*rt & SRA_READ_TYPE_BIOLOGICAL) {
                        uint32_t elem_bits, elem_off, elem_cnt;
                        const void *base = NULL;
                        rc = VCursorCellDataDirect(curs, spot, idxREAD_LEN,
                            &elem_bits, &base, &elem_off, &elem_cnt);
                        if (rc != 0) {
                            *status = eVdbBlastErr;
                            PLOGMSG(klogInfo, (klogInfo,
                                "$(f): Cannot '$(name)' CellDataDirect",
                                "f=%s,name=%s", __func__, "READ_LEN"));
                            res = bad;
                            break;
                        }
                        else if (elem_off != 0 || elem_bits != 32) {
                            *status = eVdbBlastErr;
                            PLOGERR(klogInt, (klogInt, rc,
                                "Bad VCursorCellDataDirect(READ_LEN) result: "
                                "boff=$(elem_off), elem_bits=$(elem_bits)",
                                "elem_off=%u,elem_bits=%u",
                                elem_off, elem_bits));
                            res = bad;
                            break;
                        }
                        else {
                            uint8_t read = c;
                            const uint32_t *readLen = base;
                            res = cmp(readLen[read], res, minRead, &done);
                        }
                    }
                }
            }
        }
        else {
            rc = VCursorCellDataDirect(curs, spot, idxREAD_LEN,
                &elem_bits, &base, &elem_off, &elem_cnt);
            if (rc != 0) {
                *status = eVdbBlastErr;
                PLOGMSG(klogInfo, (klogInfo,
                    "$(f): Cannot '$(name)' CellDataDirect",
                    "f=%s,name=%s", __func__, "READ_LEN"));
                res = bad;
                break;
            }
            else if (elem_off != 0 || elem_bits != 32) {
                *status = eVdbBlastErr;
                PLOGERR(klogInt, (klogInt, rc,
                    "Bad VCursorCellDataDirect(READ_LEN) result: "
                    "boff=$(elem_off), elem_bits=$(elem_bits)",
                    "elem_off=%u,elem_bits=%u", elem_off, elem_bits));
                res = bad;
                break;
            }
            else {
                uint8_t read = 0;
                const uint32_t *readLen = base;
                assert(self->rd.readType && self->rd.readLen);
                assert(self->rd.nReads == elem_cnt);
                for (read = 0; read < self->rd.nReads; ++read) {
                    if (self->rd.readType[read] & SRA_READ_TYPE_BIOLOGICAL) {
                       res = cmp(readLen[read], res, minRead, &done);
                    }
                }
            }
        }
    }
    RELEASE(VCursor, curs);
    return res;
}

static
VdbBlastStatus _VdbBlastRunGetWgsAccession(VdbBlastRun *self, int64_t spot,
    char *name_buffer, size_t bsize, size_t *num_required)
{
    rc_t rc = 0;
    uint32_t row_len = 0;

    assert(num_required);

    if (self == NULL || spot <= 0 || name_buffer == NULL || bsize == 0) {
        STSMSG(0, ("Error: some of %s parameters is NULL or 0", __func__));
        return eVdbBlastErr;
    }
    assert(self->obj);
    if (self->obj->seqTbl == NULL) {
        STSMSG(0, ("Error: %s: VTable is NULL in VdbBlastRun", __func__));
        return eVdbBlastErr;
    }

    if (self->obj->cursACCESSION == NULL) {
        rc = _VTableMakeCursor(self->obj->seqTbl, &self->obj->cursACCESSION,
            &self->obj->col_ACCESSION, "ACCESSION", self->acc);
        if (rc != 0) {
            VCursorRelease(self->obj->cursACCESSION);
            self->obj->cursACCESSION = NULL;
            return eVdbBlastErr;
        }
    }

    assert(self->obj->cursACCESSION && rc == 0);

    rc = VCursorReadDirect(self->obj->cursACCESSION, spot,
        self->obj->col_ACCESSION, 8, name_buffer, bsize, &row_len);
    *num_required = row_len;
    if (row_len > 0) /* include ending '\0' */
    {   ++(*num_required); }
    if (rc == 0) {
        if (bsize > row_len)
        { name_buffer[row_len] = '\0'; }
        return eVdbBlastNoErr;
    }
    else if (rc == SILENT_RC
        (rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient))
    {   return eVdbBlastNoErr; }
    else {
        assert(self->path);
        PLOGERR(klogInt, (klogInt, rc, "Error in VCursorReadDirect"
            "$(path), ACCESSION, spot=$(spot))",
            "path=%s,spot=%ld", self->path, spot));
        return eVdbBlastErr;
    }
}

uint64_t _VdbBlastRunGetNumAlignments(VdbBlastRun *self,
    VdbBlastStatus *status)
{
    assert(status);

    *status = eVdbBlastNoErr;

    if (self->alignments == ~0) {
        assert(self->obj);

        if (self->obj->prAlgnTbl == NULL) {
            self->alignments = 0;
        }
        else {
            const char col[] = "SPOT_COUNT";
            *status = _VTableReadFirstRow(self->obj->prAlgnTbl, col,
                &self->alignments, sizeof self->alignments, NULL, self->acc);
            if (*status != eVdbBlastNoErr) {
                STSMSG(1, ("Error: failed to read %s/%s", self->path, col));
                return 0;
            }
        }
    }

    assert(self->alignments != ~0);

    return self->alignments;
}

#ifdef TEST_VdbBlastRunFillReadDesc
LIB_EXPORT
#endif
VdbBlastStatus _VdbBlastRunFillReadDesc(VdbBlastRun *self,
    uint64_t read_id, ReadDesc *desc)
{
    VdbBlastStatus status = eVdbBlastNoErr;

    const VdbBlastRun *prev = NULL;
    const RunDesc *rd = NULL;
    
    int bioIdx = 0;

    if (self == NULL || desc == NULL) {
        S
        return eVdbBlastErr;
    }

    prev = desc->run;
    memset(desc, 0, sizeof *desc);
    desc->prev = prev;
    desc->run = self;

    rd = &self->rd;

    if (rd->nReads == 0 || rd->readType == NULL) {
        status = _VdbBlastRunFillRunDesc(self);
        if (status != eVdbBlastNoErr)
        {   return status; }
        assert(rd->nReads && rd->readType);
    }

    if (rd->readIdDesc.idType == eFixedReadN || read_id == 0) {
        if (!_VdbBlastRunVarReadNum(self)) {
            desc->nReads = rd->nReads;
            desc->spot = read_id / rd->nBioReads + 1;
            if (desc->spot <= rd->spotCount) {
                int idInSpot
                    = read_id - (desc->spot - 1) * rd->nBioReads; /* 0-based */

                int i = 0;
                for (i = 0; i < rd->nReads; ++i) {
                    if (rd->readType[i] & SRA_READ_TYPE_BIOLOGICAL) {
                        if (bioIdx++ == idInSpot) {
                            S
                            desc->tableId = VDB_READ_UNALIGNED;
                            desc->read = i + 1;
                            desc->read_id = _RunDescMakeReadId(rd,
                                desc->spot, desc->read, read_id, &status);
                            return status;
                        }
                    }
                }
                S
            }
            else {
                uint64_t alignments = 0;
                S
                desc->spot -= rd->spotCount;
                alignments = _VdbBlastRunGetNumAlignments(self, &status);
                if (status != eVdbBlastNoErr) {
                    return status;
                }
                if (desc->spot <= alignments) {
                    desc->tableId = VDB_READ_ALIGNED;
                    desc->read = 1;
                    return eVdbBlastNoErr;
                }
                S
            }
        }
        else {
            bool found = false;
            if (_VdbBlastDbOpenCursAndGetFirstRead
                (self->obj, desc, &found, self->acc) == 0)
            {
                return eVdbBlastNoErr;
            }
        }
    }
    else if (rd->readIdDesc.idType == eFactor10) {
        if (rd->readIdDesc.runBits > 0) {
            read_id /= rd->readIdDesc.runBits;
        }
        if (rd->spotBits > 0) {
            rc_t rc = 0;
            desc->read = read_id / rd->spotBits;
            desc->spot = read_id % rd->spotBits;
            if (desc->read > 0 && desc->spot > 0) {
                rc = _VdbBlastDbGetNReads
                    (self->obj, desc->spot, &desc->nReads, self->acc);
                if (rc == 0) {
                    desc->tableId = VDB_READ_UNALIGNED;
                    S
                    return eVdbBlastNoErr;
                }
                else {
                    S
                }
            }
            else {
                S
            }
        }
    }
    else {
        assert(0);
    }

    memset(desc, 0, sizeof *desc);
    return eVdbBlastErr;
}

VdbBlastStatus _VdbBlastRunSetFindFirstRead(const VdbBlastRunSet *self,
    uint64_t *read_id, bool useGetFirstRead)
{
    VdbBlastStatus status = eVdbBlastErr;
    const RunSet *rs = NULL;
    ReadDesc desc;
    assert(self);
    memset(&desc, 0, sizeof desc);
    rs = &self->runs;
    _VdbBlastRunSetBeingRead(self);
    if (useGetFirstRead) {
        bool found = false;
        uint32_t i = 0;
        for (i = 0; i < rs->krun; ++i) {
            VdbBlastRun *run = &rs->run[i];
            rc_t rc = _VdbBlastDbOpenCursAndGetFirstRead
                (rs->run[i].obj, &desc, &found, rs->run[i].acc);
            if (rc != 0) {
                break;
            }
            else if (!found) {
                continue;
            }

            if (desc.spot == 0) {
                if (rs->krun > 1) {
                    desc.prev = run;
                }
            }
            else {
                desc.run = run;
                status = _ReadDescFixReadId(&desc);
                break;
            }
        }
    }
    else {
        status = _RunSetFindReadDesc(rs, 0, &desc);
        if (status == eVdbBlastNoErr) {
            status = _ReadDescFixReadId(&desc);
        }
    }
    if (status == eVdbBlastNoErr) {
        assert(read_id);
        *read_id = desc.read_id;
    }
    return status;
}

EReadIdType _VdbBlastRunSetGetReadIdType(const VdbBlastRunSet *self) {
    assert(self);

    return self->readIdDesc.idType;
}

static VdbBlastStatus _VdbBlastRunGetReadId(VdbBlastRun *self, const char *acc,
    uint64_t spot, /* 1-based */
    uint32_t read, /* 1-based */
    uint64_t *read_id)
{
    uint64_t id = ~0;
    VdbBlastStatus status = eVdbBlastErr;
    size_t size;

    assert(self && acc && read_id && self->acc);
    assert(memcmp(self->acc, acc, string_measure(self->acc, &size)) == 0);

    if ((spot <= 0 && read > 0) || (spot > 0 && read <= 0)) {
        S
        return eVdbBlastErr;
    }

    if (spot > 0) {
        if (self->type != btpSRA) {
            return eVdbBlastErr;
        }

        if (self->rd.readIdDesc.idType == eFixedReadN) {
            for (id = (spot - 1) * self->rd.nBioReads; ; ++id) {
                ReadDesc desc;
                status = _VdbBlastRunFillReadDesc(self, id, &desc);
                if (status != eVdbBlastNoErr)
                {   return status; }
                if (desc.spot < spot) {
                    S
                    return eVdbBlastErr;
                }
                if (desc.spot > spot) {
                    S
                    return eVdbBlastErr;
                }
                if (desc.read == read) {
                    *read_id = id;
                    return eVdbBlastNoErr;
                }
            }
            S
            return eVdbBlastErr;
        }
        else if (self->rd.readIdDesc.idType == eFactor10) {
            if (spot <= self->rd.spotCount) {
                if (self->rd.readIdDesc.varReadN) {
                    /* TODO How to make sure this read exists in this spot?*/
                }
                else {
                    if (read > self->rd.nReads) {
                        S
                        return eVdbBlastErr;
                    }
                    else {
                        /* TODO how to check it when READ_TYPE is variable
                                withing a run (is it possible?) */
                        if (read == 0) {
                            S
                            return eVdbBlastErr;
                        }
                        if (! (self->rd.readType[read - 1]
                                & SRA_READ_TYPE_BIOLOGICAL))
                        {
                            S
                            return eVdbBlastErr;
                        }
                    }
                }
                id = _RunDescMakeReadId(&self->rd, spot, read, ~0, &status);
                *read_id = id;
                return status;
            }
            else {
                S
                return eVdbBlastErr;
            }
        }
        else {
            assert(0);
            return eVdbBlastErr;
        }
    }
    else {
        uint64_t n = ~0;
        uint64_t i = ~0;
        if (self->type == btpSRA)
        {   return eVdbBlastErr; }
        else if (self->type == btpREFSEQ) {
            *read_id = 0;
            return eVdbBlastNoErr;
        }
        else if (self->type == btpWGS) {
            n = _VdbBlastRunGetNumSequences(self, &status);
            if (status != eVdbBlastNoErr
                && status != eVdbBlastTooExpensive)
            {
                return status;
            }
            /* TODO optimize: avoid full run scan */
            for (i = 0; i < n ; ++i) {
                size_t need = ~0;
#define SZ 4096
                char name_buffer[SZ + 1];
                if (string_measure(acc, &size) > SZ) {
                    S
                    return eVdbBlastErr;
                }
#undef SZ
                status = _VdbBlastRunGetWgsAccession(
                    self, i + 1, name_buffer, sizeof name_buffer, &need);
                if (need > sizeof name_buffer) {
                    S
                    return eVdbBlastErr;
                }
                if (strcmp(name_buffer, acc) == 0) {
                    i = _RunDescMakeReadId(&self->rd, i + 1, 1, i, &status);
                    *read_id = i;
                    return status;
                }
            }
        }
        else { assert(0); }
        return eVdbBlastErr;
    }
}

static uint64_t _VdbBlastRunGetSequencesAmount(
    VdbBlastRun *self, VdbBlastStatus *status)
{
    uint64_t n = _VdbBlastRunGetNumSequences(self, status);

    assert(status);

    if (*status == eVdbBlastNoErr) {
        n += _VdbBlastRunGetNumAlignments(self, status);
    }

    return n;
}

/******************************************************************************/

VdbBlastStatus _ReadDescFixReadId(ReadDesc *self) {
    VdbBlastStatus status = eVdbBlastNoErr;

    assert(self && self->run);

    self->read_id = _RunDescMakeReadId
        (&self->run->rd, self->spot, self->read, self->read_id, &status);

    return status;
}

/******************************************************************************/

static  void _RunSetFini(RunSet *self) {
    assert(self);

    if (self->run) {
        uint32_t i = 0;

        for (i = 0; i < self->krun; ++i) {
            _VdbBlastRunFini(&self->run[i]);
        }

        free(self->run);
    }

    _RefSetFini(&self->refs);
    
    memset(self, 0, sizeof *self);
}

static VdbBlastStatus _RunSetAllocTbl(RunSet *self) {
    size_t nmemb = 16;

    if (self == NULL)
    {   return eVdbBlastErr; }

    if (self->run && self->krun < self->nrun) {
        return eVdbBlastNoErr;
    }

    if (self->run == NULL) {
        self->run = calloc(nmemb, sizeof *self->run);
        if (self->run == NULL)
        {   return eVdbBlastMemErr; }
        S
    }
    else {
        void *p = NULL;
        nmemb += self->nrun;
        p = realloc(self->run, nmemb * sizeof *self->run);
        if (p == NULL)
        {   return eVdbBlastMemErr; }
        self->run = p;
        S
    }

    self->nrun = nmemb;
    return eVdbBlastNoErr;
}

static VdbBlastStatus _RunSetAddObj(RunSet *self, VdbBlastDb *obj,
    const char *rundesc, BTableType type, const KDirectory *dir,
    char *fullpath, uint32_t min_read_length)
{

    VdbBlastRun* run = NULL;

    VdbBlastStatus status = _RunSetAllocTbl(self);
    if (status != eVdbBlastNoErr) {
        return status;
    }

    assert(self && self->run);

    run = &self->run[self->krun];

    status = _VdbBlastRunInit(run,
        obj, rundesc, type, dir, fullpath, min_read_length, self->krun++);

    return status;
}

static uint64_t _RunSetGetNumSequences
    (const RunSet *self, VdbBlastStatus *aStatus)
{
    uint64_t num = 0;
    uint32_t i = 0;

    assert(self && aStatus);

    *aStatus = eVdbBlastNoErr;

    for (i = 0; i < self->krun; ++i) {
        VdbBlastStatus status = eVdbBlastNoErr;
        VdbBlastRun *run = NULL;

        assert(self->run);

        run = &self->run[i];

        num += _VdbBlastRunGetNumSequences(run, &status);
        if (status != eVdbBlastNoErr) {
            assert(run->path);

            if (*aStatus == eVdbBlastNoErr) {
                *aStatus = status; 
            }

            if (status != eVdbBlastTooExpensive) {
                STSMSG(1, (
                    "Error: failed to GetNumSequences(on run %s)", run->path));
                return 0;
            }

            assert(*aStatus == eVdbBlastTooExpensive);
        }
    }

    STSMSG(1, ("_RunSetGetNumSequences = %ld", num));

    return num;
}

static
uint64_t _RunSetGetNumSequencesApprox(const RunSet *self,
    VdbBlastStatus *status)
{
    uint64_t num = 0;
    uint32_t i = 0;

    assert(self && status);

    *status = eVdbBlastNoErr;

    for (i = 0; i < self->krun; ++i) {
        VdbBlastRun *run = NULL;

        assert(self->run);

        run = &self->run[i];

        num += _VdbBlastRunGetNumSequencesApprox(run, status);
        if (*status != eVdbBlastNoErr) {
            assert(run->path);

            STSMSG(1, (
                "Error: failed to GetNumSequencesApprox(on run %s)",
                run->path));

            return 0;
        }
    }

    STSMSG(1, ("_RunSetGetNumSequencesApprox = %ld", num));

    return num;
}

static
uint64_t _RunSetGetTotalLength(const RunSet *self,
    VdbBlastStatus *status)
{
    uint64_t num = 0;
    uint32_t i = 0;
    assert(self && status);

    if (self->krun)
    {   assert(self->run); }

    for (i = 0; i < self->krun; ++i) {
        VdbBlastRun *run = &self->run[i];
        assert(run && run->path);
        num += _VdbBlastRunGetLength(run, status);
        if (*status != eVdbBlastNoErr) {
            STSMSG(1, (
                "Error: failed to _RunSetGetTotalLength(on run %s)",
                run->path));
            return 0;
        }
    }

    STSMSG(1, ("_RunSetGetTotalLength = %ld", num));

    return num;
}

static uint64_t _RunSetGetTotalLengthApprox(const RunSet *self,
    VdbBlastStatus *status)
{
    uint64_t num = 0;

    uint32_t i = 0;

    assert(self && status);

    for (num = 0, i = 0; i < self->krun; ++i) {
        VdbBlastRun *run = NULL;

        assert(self->run);

        run = &self->run[i];

        num += _VdbBlastRunGetLengthApprox(run, status);
        if (*status != eVdbBlastNoErr) {
            STSMSG(1, ("Error: failed "
                "to _VdbBlastRunGetLengthApprox(on run %s)", run->path));
            return 0;
        }
    }

    STSMSG(1, ("VdbBlastRunSetGetTotalLengthApprox = %ld", num));

    return num;
}

static size_t _RunSetGetName(const RunSet *self,
    VdbBlastStatus *status, char *name_buffer, size_t bsize)
{
    size_t need = 0, idx = 0;
    int i = 0;
    size_t size;
    
    assert(self && status);
    for (i = 0; i < self->krun; ++i) {
        VdbBlastRun *run = &self->run[i];
        if (run && run->acc) {
            if (i)
            {   ++need; }
            need += string_measure(run->acc, &size);
        }
        else {
            S
            return 0;
        }
    }

    if (name_buffer == NULL || bsize == 0) {
        S
        return need;
    }

    for (i = 0; i < self->krun; ++i) {
        VdbBlastRun *run = &self->run[i];
        if (run && run->acc) {
            if (i)
            {   name_buffer[idx++] = '|'; }
            if (idx >= bsize) {
                S
                return need;
            }
            string_copy(name_buffer + idx, bsize - idx,
                run->acc, string_size(run->acc));
            idx += string_measure(run->acc, &size);
            if (idx >= bsize) {
                S
                return need;
            }
        }
    }
    name_buffer[idx++] = '\0';
    *status = eVdbBlastNoErr;

    S
    return need;
}

typedef struct {
    uint64_t read_id;
} ReadId;
static VdbBlastStatus _RunSetFindReadDescContinuous(
    const RunSet *self, uint64_t read_id, ReadDesc *desc)
{
    uint64_t i = 0;
    uint64_t prev = 0;
    uint64_t crnt = 0;

    assert(self && desc);

    for (i = 0, prev = 0; i < self->krun; ++i) {
        VdbBlastStatus status = eVdbBlastNoErr;
        VdbBlastRun *run = NULL;
        uint64_t l = 0;

        if (prev > 0 && i < prev) {
            S
            return eVdbBlastErr;
        }

        run = &self->run[i];
        if (run == NULL) {
            S
            return eVdbBlastErr;
        }

        l = _VdbBlastRunGetSequencesAmount(run, &status);
        if (status != eVdbBlastNoErr &&
            status != eVdbBlastTooExpensive)
        {
            S
            return status;
        }

        if (crnt + l <= read_id) {
            crnt += l;
        }
        else {
            uint64_t a_read_id = read_id - crnt;
            status = _VdbBlastRunFillReadDesc(run, a_read_id, desc);
            if (status == eVdbBlastNoErr) {
                S
                desc->read_id = read_id;
            }
            else
            {   S }

            return status;
        }

        prev = i;
    }

    S
    return eVdbBlastErr;
}

static VdbBlastStatus _RunSetFindReadDescFactor10(
    const RunSet *self, uint64_t read_id, ReadDesc *desc)
{
    VdbBlastStatus status = eVdbBlastErr;

    VdbBlastRun *run = NULL;
    uint64_t i = 0;

    assert(self && desc && self->run);

    if (self->run[0].rd.readIdDesc.runBits > 0) {
        i = read_id % self->run[0].rd.readIdDesc.runBits;
    }
    else {
        i = 0;
    }

    if (i >= self->krun) {
        S
        return eVdbBlastErr;
    }

    run = &self->run[i];

    status = _VdbBlastRunFillReadDesc(run, read_id, desc);
    if (status == eVdbBlastNoErr) {
        S
        desc->read_id = read_id;
    }
    else
    {   S }

    return status;
}

VdbBlastStatus _RunSetFindReadDesc(const RunSet *self,
    uint64_t read_id, ReadDesc *desc)
{
    if (self == NULL || desc == NULL || self->krun == 0) {
        S
        return eVdbBlastErr;
    }

    switch (self->run[0].rd.readIdDesc.idType) {
        case eFixedReadN:
            return _RunSetFindReadDescContinuous(self, read_id, desc);
        case eFactor10:
            return _RunSetFindReadDescFactor10(self, read_id, desc);
        default:
            assert(0);
            return eVdbBlastErr;
    }
}

/******************************************************************************/

static const char VDB_BLAST_RUN_SET[] = "VdbBlastRunSet";

static void _VdbBlastRunSetWhack(VdbBlastRunSet *self) {
    assert(self);

    STSMSG(1, ("Deleting VdbBlastRunSet(min_read_length=%d, protein=%s)",
        self->core2na.min_read_length, self->protein ? "true" : "false"));

    VdbBlastMgrRelease(self->mgr);

    _RunSetFini(&self->runs);
    _Core2naFini(&self->core2na);
    _Core4naFini(&self->core4na);
    _Core2naFini(&self->core2naRef);
    _Core4naFini(&self->core4naRef);

    memset(self, 0, sizeof *self);
    free(self);
}

LIB_EXPORT
VdbBlastRunSet *VdbBlastMgrMakeRunSet(const VdbBlastMgr *cself,
    VdbBlastStatus *status,
    uint32_t min_read_length,
    bool protein)
{
    rc_t rc = 0;
    VdbBlastRunSet *item = NULL;
    VdbBlastMgr *self = (VdbBlastMgr*)cself;

    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    *status = eVdbBlastNoErr;

    item = calloc(1, sizeof *item);
    if (item == NULL) {
        *status = eVdbBlastMemErr;
        return item;
    }

    item->protein = protein;

    item->core2na.min_read_length = min_read_length;

    item->core2naRef.min_read_length = min_read_length;

    item->core4na.min_read_length = min_read_length;
    item->core4na.mode = VDB_READ_UNALIGNED;

    item->core4naRef.min_read_length = min_read_length;
    item->core4naRef.mode = VDB_READ_REFERENCE;

    item->minSeqLen = ~0;
    item->avgSeqLen = ~0;
    item->maxSeqLen = ~0;

    if (rc == 0) {
        rc = KLockMake(&item->core2na.mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockMake");
        }
    }
    if (rc == 0) {
        rc = KLockMake(&item->core2naRef.mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockMake");
        }
    }
    if (rc == 0) {
        rc = KLockMake(&item->core4na.mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockMake");
        }
    }
    if (rc == 0) {
        rc = KLockMake(&item->core4naRef.mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockMake");
        }
    }
    if (rc == 0) {
        item->mgr = VdbBlastMgrAddRef(self);
        if (item->mgr) {
            KRefcountInit(&item->refcount,
                1, VDB_BLAST_RUN_SET, __func__, "set");
            STSMSG(1, ("Created VdbBlastRunSet(min_read_length=%d, protein=%s)",
                min_read_length, protein ? "true" : "false"));
            return item;
        }
    }

    STSMSG(1, ("Error: failed to create VdbBlastRunSet"));
    _VdbBlastRunSetWhack(item);

    *status = eVdbBlastErr;

    return NULL;
}

LIB_EXPORT void CC VdbBlastRunSetRelease(VdbBlastRunSet *self) {
    if (self == NULL) {
        return;
    }

    STSMSG(1, ("VdbBlastRunSetRelease"));
    if (KRefcountDrop(&self->refcount, VDB_BLAST_RUN_SET) != krefWhack)
    {   return; }

    _VdbBlastRunSetWhack(self);
}

LIB_EXPORT
VdbBlastRunSet* CC VdbBlastRunSetAddRef(VdbBlastRunSet *self)
{
    if (self == NULL) {
        STSMSG(1, ("VdbBlastRunSetAddRef(NULL)"));
        return self;
    }

    if (KRefcountAdd(&self->refcount, VDB_BLAST_RUN_SET) == krefOkay) {
        STSMSG(1, ("VdbBlastRunSetAddRef"));
        return self;
    }

    STSMSG(1, ("Error: failed to VdbBlastRunSetAddRef"));
    return NULL;
}

LIB_EXPORT
VdbBlastStatus CC VdbBlastRunSetAddRun(VdbBlastRunSet *self,
    const char *native)
{
    rc_t rc = 0;
    char rundesc[PATH_MAX] = "";
    VdbBlastStatus status = eVdbBlastNoErr;
    BTableType type = btpUndefined;

    /* allocated in _VdbBlastMgrFindNOpenSeqTable()
       in _RunSetAddObj() is assigned to VdbBlastRun::path
       freed during VdbBlastRun release */
    char *fullpath = NULL;

    VdbBlastDb *obj = NULL;
    if (self == NULL || self->mgr == NULL || self->beingRead) {
        S
        return eVdbBlastErr;
    }

    obj = calloc(1, sizeof *obj);
    if (obj == NULL) {
        return eVdbBlastMemErr;
    }

    rc = _VdbBlastMgrNativeToPosix(self->mgr, native, rundesc, sizeof rundesc);
    if (rc != 0) {
        S
        status = eVdbBlastErr;
    }

    status = _VdbBlastMgrFindNOpenSeqTable(self->mgr,
        rundesc, &obj->seqTbl, &type, &fullpath, &obj->db);
    if (status != eVdbBlastNoErr) {
        S
        PLOGMSG(klogInfo,
            (klogInfo, "failed to open $(rundesc)", "rundesc=%s", rundesc));
    }
    else {
        S
        PLOGMSG(klogInfo,
            (klogInfo, "opened $(rundesc)", "rundesc=%s", rundesc));
    }

    if (status == eVdbBlastNoErr && _VTableCSra(obj->seqTbl)) {
        if (obj->db == NULL) {
            S
            status = eVdbBlastErr;
        }
        else {
            status = _VDatabaseOpenAlignmentTable(
                obj->db, rundesc, &obj->prAlgnTbl);
        }
    }

    if (status == eVdbBlastNoErr) {
        if (_VTableVarReadNum(obj->seqTbl, rundesc)) {
            self->readIdDesc.varReadN = true;
            self->readIdDesc.idType   = eFactor10;
        }

        status = _RunSetAddObj(&self->runs, obj, rundesc, type,
            NULL, fullpath, self->core2na.min_read_length);
        S
    }

    return status;
}

void _VdbBlastRunSetBeingRead(const VdbBlastRunSet *cself) {
    uint32_t i = 0;
    uint32_t runBits = 0;
    VdbBlastRunSet *self = (VdbBlastRunSet*)cself;
    EReadIdType idType = eFixedReadN;

    if (self == NULL || self->beingRead) {
        return;
    }

    idType = self->readIdDesc.idType;
    runBits = Bits(self->runs.krun - 1, idType);

    for (i = 0; i < self->runs.krun; ++i) {
        VdbBlastRun *r = &self->runs.run[i];
        assert(r);
        r->rd.readIdDesc.idType = idType;
        r->rd.readIdDesc.runBits = runBits;
        r->rd.readIdDesc.varReadN = _VdbBlastRunVarReadNum(r);
    }

    self->beingRead = true;
}

LIB_EXPORT
bool CC VdbBlastRunSetIsProtein(const VdbBlastRunSet *self)
{
    if (self == NULL) {
        STSMSG(1, ("VdbBlastRunSetIsProtein(self=NULL)"));
        return false;
    }

    STSMSG(1, (
        "VdbBlastRunSetIsProtein = %s", self->protein ? "true" : "false"));

    return self->protein;
}

LIB_EXPORT
uint64_t CC VdbBlastRunSetGetNumSequences(const VdbBlastRunSet *self,
    VdbBlastStatus *status)
{
    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (self == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    _VdbBlastRunSetBeingRead(self);

    return _RunSetGetNumSequences(&self->runs, status);
}

LIB_EXPORT uint64_t CC VdbBlastRunSetGetNumSequencesApprox(
    const VdbBlastRunSet *self)
{
    uint64_t num = 0;
    VdbBlastStatus status = eVdbBlastNoErr;

    _VdbBlastRunSetBeingRead(self);

    num = _RunSetGetNumSequencesApprox(&self->runs, &status);

    STSMSG(1, ("VdbBlastRunSetGetNumSequencesApprox=%lu", num));

    return num;
}

LIB_EXPORT
uint64_t CC VdbBlastRunSetGetTotalLength(const VdbBlastRunSet *self,
    VdbBlastStatus *status)
{
    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    if (self == NULL) {
        *status = eVdbBlastErr;
        return 0;
    }

    _VdbBlastRunSetBeingRead(self);

    return _RunSetGetTotalLength(&self->runs, status);
}

LIB_EXPORT
uint64_t CC VdbBlastRunSetGetTotalLengthApprox(
    const VdbBlastRunSet *self)
{
    VdbBlastStatus status = eVdbBlastNoErr;

    if (self == NULL) {
        STSMSG(1, ("VdbBlastRunSetGetTotalLengthApprox(self=NULL)"));
        return 0;
    }

    _VdbBlastRunSetBeingRead(self);

    return _RunSetGetTotalLengthApprox(&self->runs, &status);
}

LIB_EXPORT
uint64_t CC VdbBlastRunSetGetMinSeqLen(const VdbBlastRunSet *self)
{
    if (self->minSeqLen == ~0) {
        bool empty = true;
        VdbBlastStatus status = eVdbBlastNoErr;
        uint64_t res = ~0;
        uint32_t i = 0;
        _VdbBlastRunSetBeingRead(self);
        for (i = 0; i < self->runs.krun; ++i) {
            VdbBlastRun *run = &self->runs.run[i];
            assert(run);
            if (run->type == btpREFSEQ) {
                uint64_t cand = _VdbBlastRunGetLengthApprox(run, &status);
                if (status != eVdbBlastNoErr) {
                    S
                    return ~0;
                }
                if (cand < res && cand >= self->core2na.min_read_length) {
                    res = cand;
                }
            }
            else {
                status = _VdbBlastRunFillRunDesc(run);
                if (status != eVdbBlastNoErr) {
                    S
                    return ~0;
                }
                if (!run->rd.varReadLen) {
                    uint8_t read = 0;
                    assert(run->rd.readType && run->rd.readLen);
                    for (read = 0; read < run->rd.nReads; ++read) {
                        if (run->rd.readType[i] & SRA_READ_TYPE_BIOLOGICAL) {
                            if (run->rd.readLen[i]
                                == self->core2na.min_read_length)
                            {
                                ((VdbBlastRunSet*)self)->minSeqLen
                                    = self->core2na.min_read_length;
                                return self->minSeqLen;
                            }
                            else if (run->rd.readLen[i]
                                    > self->core2na.min_read_length
                                && run->rd.readLen[i] < res)
                            {
                                res = run->rd.readLen[i];
                                empty = false;
                            }
                        }
                    }
                }
                else {
                    res = _VdbBlastRunScan(
                        run, Min, self->core2na.min_read_length, res, &status);
                    if (status != eVdbBlastNoErr) {
                        S
                        return ~0;
                    }
                }
            }
        }
        if (empty && res == ~0) {
            res = 0;
        }
        ((VdbBlastRunSet*)self)->minSeqLen = res;
    }
    return self->minSeqLen;
}

LIB_EXPORT
uint64_t CC VdbBlastRunSetGetMaxSeqLen(const VdbBlastRunSet *self)
{
    if (self->maxSeqLen == ~0) {
        VdbBlastStatus status = eVdbBlastNoErr;
        uint64_t res = 0;
        uint32_t i = 0;
        _VdbBlastRunSetBeingRead(self);
        for (i = 0; i < self->runs.krun; ++i) {
            VdbBlastRun *run = &self->runs.run[i];
            assert(run);
            if (run->type == btpREFSEQ) {
                uint64_t cand = _VdbBlastRunGetLengthApprox(run, &status);
                if (status != eVdbBlastNoErr) {
                    S
                    return ~0;
                }
                if (cand > res) {
                    res = cand;
                }
            }
            else {
                status = _VdbBlastRunFillRunDesc(run);
                if (status != eVdbBlastNoErr) {
                    S
                    return ~0;
                }
                if (!run->rd.varReadLen) {
                    uint8_t read = 0;
                    assert(run->rd.readType && run->rd.readLen);
                    for (read = 0; read < run->rd.nReads; ++read) {
                        if (run->rd.readType[i] & SRA_READ_TYPE_BIOLOGICAL) {
                            if (run->rd.readLen[i] > res) {
                                res = run->rd.readLen[i];
                            }
                        }
                    }
                }
                else {
                    res = _VdbBlastRunScan(
                        run, Max, -1, res, &status);
                    if (status != eVdbBlastNoErr) {
                        S
                        return ~0;
                    }
                }
            }
        }
        ((VdbBlastRunSet*)self)->maxSeqLen = res;
    }
    return self->maxSeqLen;
}

LIB_EXPORT
uint64_t CC VdbBlastRunSetGetAvgSeqLen(const VdbBlastRunSet *self)
{
    uint64_t num = 0;

    if (self == NULL) {
        STSMSG(1, ("VdbBlastRunSetGetAvgSeqLen(self=NULL)"));
        return 0;
    }

    if (self->avgSeqLen == ~0) {
        uint64_t n = 0;
        _VdbBlastRunSetBeingRead(self);

        n = VdbBlastRunSetGetNumSequencesApprox(self);
        if (n != 0) {
            num = VdbBlastRunSetGetTotalLengthApprox(self) / n;
        }
        else {
            num = n;
        }
        ((VdbBlastRunSet*)self)->avgSeqLen = num;
    }

    STSMSG(1, ("VdbBlastRunSetGetAvgSeqLen = %ld", num));
    return self->avgSeqLen;
}

LIB_EXPORT size_t CC VdbBlastRunSetGetName(const VdbBlastRunSet *self,
    VdbBlastStatus *status, char *name_buffer, size_t bsize)
{
    size_t sz = 0;

    VdbBlastStatus dummy = eVdbBlastNoErr;
    if (status == NULL)
    {   status = &dummy; }

    *status = eVdbBlastErr;

    if (self == NULL)
    {   return 0; }

    _VdbBlastRunSetBeingRead(self);

    sz = _RunSetGetName(&self->runs, status, name_buffer, bsize);

    STSMSG(1, ("VdbBlastRunSetGetName = '%.*s'", bsize, name_buffer));

    return sz;
}

LIB_EXPORT
time_t CC VdbBlastRunSetLastUpdatedDate(const VdbBlastRunSet *self)
{
    _VdbBlastRunSetBeingRead(self);
    return _NotImplemented(__func__);
}

LIB_EXPORT
size_t CC VdbBlastRunSetGetReadName(const VdbBlastRunSet *self,
    uint64_t read_id, /* 0-based in RunSet */
    char *name_buffer,
    size_t bsize)
{
    rc_t rc = 0;
    VdbBlastStatus status = eVdbBlastNoErr;
    size_t need = 0;
    size_t num_writ = 0;

    ReadDesc desc;
    memset(&desc, 0, sizeof desc);

    if (name_buffer && bsize)
    {   name_buffer[0] = '\0'; }

    if (self == NULL) {
        STSMSG(1, ("VdbBlastRunSetGetReadName(self=NULL)"));
        return 0;
    }

    _VdbBlastRunSetBeingRead(self);

    status = _RunSetFindReadDesc(&self->runs, read_id, &desc);
    if (status != eVdbBlastNoErr) {
        STSMSG(1, ("Error: failed to VdbBlastRunSetGetReadName: "
            "cannot find RunSet ReadDesc"));
        return 0;
    }

    assert(desc.run && desc.run->path && desc.run->acc && desc.spot
        && desc.read);

    if (desc.run->type == btpUndefined) {
        desc.run->type
            = _VdbBlastMgrBTableType(self->mgr, desc.run->path);
        assert(desc.run->type != btpUndefined);
    }
    if (desc.run->type == btpWGS) {
        if (desc.read != 1) {
            STSMSG(1, ("Error: failed to VdbBlastRunSetGetReadName: "
                "Unexpected read='%u' for run '%s', spot='%lu'",
                desc.read, desc.run->path, desc.spot));
            return 0;
        }
        status = _VdbBlastRunGetWgsAccession(
            desc.run, desc.spot, name_buffer, bsize, &need);
        if (status != eVdbBlastNoErr)
        {   need = 0; }
        return need;
    }
    else if (desc.run->type == btpREFSEQ) {
        rc = string_printf(name_buffer, bsize, &num_writ, "%s", desc.run->acc);
        if (rc == 0) {
            S
            need = num_writ;
        }
        else if (GetRCObject(rc) == (enum RCObject)rcBuffer
            && GetRCState(rc) == rcInsufficient)
        {
            size_t size;
            S
            need = string_measure(desc.run->acc, &size) + 1;
        }
    }
    else {
        rc = string_printf(name_buffer, bsize, &num_writ,
            "%s.%lu.%u", desc.run->acc, desc.spot, desc.read);
        if (rc == 0) {
            S
            need = num_writ;
        }
        else if (GetRCObject(rc) == (enum RCObject)rcBuffer
            && GetRCState(rc) == rcInsufficient)
        {
            int i = 0;
            size_t size;
            S
            need = string_measure(desc.run->acc, &size) + 2;
            i = desc.spot;
            while (i > 0) {
                ++need;
                i /= 10;
            }
            i = desc.read;
            while (i > 0) {
                ++need;
                i /= 10;
            }
        }
        else
        {   LOGERR(klogInt, rc, "Unexpecter error in string_printf"); }
    }

    STSMSG(1, ("VdbBlastRunSetGetName = '%.*s'", bsize, name_buffer));
    return need;
}

LIB_EXPORT uint32_t CC VdbBlastRunSetGetReadId(const VdbBlastRunSet *self,
    const char *name_buffer, size_t bsize, uint64_t *read_id)
{
    VdbBlastStatus status = eVdbBlastNoErr;
    bool found = false;

    uint64_t result = 0;
    char *acc = NULL;
    uint64_t spot = 0;
    uint32_t read = 0;
    uint32_t i = ~0;
    if (self == NULL || name_buffer == NULL || name_buffer[0] == '\0' ||
        bsize == 0 || read_id == 0)
    {   return eVdbBlastErr; }

    {
        size_t n = bsize;
        const char *end = name_buffer + bsize;
        char *dot2 = NULL;
        char *dot1 = memchr(name_buffer, '.', bsize);
        if (dot1 != NULL) {
            if (dot1 == name_buffer)
            {   return eVdbBlastErr; }
            if (dot1 - name_buffer + 1 >= bsize)
            {   return eVdbBlastErr; }
            n -= (dot1 - name_buffer + 1);
            dot2 = memchr(dot1 + 1, '.', n);
            if (dot2 != NULL) {
                if (dot2 - name_buffer + 1 >= bsize)
                {   return eVdbBlastErr; }
                acc = string_dup(name_buffer, dot1 - name_buffer + 1);
                if (acc == NULL)
                {   return eVdbBlastMemErr; }
                acc[dot1 - name_buffer] = '\0';
                while (++dot1 < dot2) {
                    char c = *dot1;
                    if (c < '0' || c > '9') {
                        S
                        status = eVdbBlastErr;
                        break;
                    }
                    spot = spot * 10 + c - '0';
                }
                while (status == eVdbBlastNoErr && ++dot2 < end) {
                    char c = *dot2;
                    if (c < '0' || c > '9') {
                        S
                        status = eVdbBlastErr;
                        break;
                    }
                    read = read * 10 + c - '0';
                }
            }
            else {
                acc = malloc(bsize + 1);
                if (acc == NULL)
                {   return eVdbBlastMemErr; }
                string_copy(acc, bsize + 1, name_buffer, bsize);
                acc[bsize] = '\0';
            }
        }
        else {
            acc = malloc(bsize + 1);
            if (acc == NULL)
            {   return eVdbBlastMemErr; }
            string_copy(acc, bsize + 1, name_buffer, bsize);
            acc[bsize] = '\0';
        }
    }

    _VdbBlastRunSetBeingRead(self);

    for (i = 0; i < self->runs.krun && status == eVdbBlastNoErr; ++i) {
        uint64_t id = ~0;
        VdbBlastRun *run = self->runs.run + i;
        size_t size;
        assert(run && run->acc);
        if (string_measure(run->acc, &size)
            == string_measure(acc, &size))
        {
            if (strcmp(run->acc, acc) == 0) {
                status = _VdbBlastRunGetReadId(run, acc, spot, read, &id);
                if (status == eVdbBlastNoErr) {
                    if (run->rd.readIdDesc.idType == eFixedReadN) {
                        id += result;
                    }
                    *read_id = id;
                    found = true;
                }
                break;
            }
        }
        else if ((string_measure(run->acc, &size) < string_measure(acc, &size))
            && (run->type == btpWGS)
            && (memcmp(run->acc, acc, string_measure(run->acc, &size))
                == 0))
        {
            status = _VdbBlastRunGetReadId(run, acc, spot, read, &id);
            if (status == eVdbBlastNoErr) {
                if (run->rd.readIdDesc.idType == eFixedReadN) {
                    result += id;
                }
                else {
                    result = id;
                }
                *read_id = result;
                found = true;
            }
            break;
        }
        result += _VdbBlastRunGetSequencesAmount(run, &status);
        if (status != eVdbBlastNoErr) {
            if (status == eVdbBlastTooExpensive) {
                status = eVdbBlastNoErr;
            }
            else {
                break;
            }
        }
    }

    if (status == eVdbBlastNoErr && !found) {
        S
        status = eVdbBlastErr;
    }

    free (acc);
    acc = NULL;

    return status;
}

/* TODO: make sure
         ReadLength is correct when there are multiple reads in the same spot */
LIB_EXPORT
uint64_t CC VdbBlastRunSetGetReadLength(const VdbBlastRunSet *self,
    uint64_t read_id)
{
    rc_t rc = 0;
    const VCursor *curs = NULL;
    uint32_t col_idx = 0;
    char buffer[84] = "";
    uint32_t row_len = 0;
    ReadDesc desc;
    VdbBlastStatus status = eVdbBlastErr;

    if (self == NULL) {
        STSMSG(1, ("VdbBlastRunSetGetReadLength(self=NULL)"));
        return 0;
    }

    status = _RunSetFindReadDesc(&self->runs, read_id, &desc);
    if (status != eVdbBlastNoErr) {
        STSMSG(1, ("Error: failed to VdbBlastRunSetGetReadLength: "
            "cannot find RunSet ReadDesc"));
        return 0;
    }
    assert(desc.run && desc.spot && desc.run->path);

    _VdbBlastRunSetBeingRead(self);

    if (rc == 0) {
        rc = _VTableMakeCursor
            (desc.run->obj->seqTbl, &curs, &col_idx, "READ", desc.run->acc);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc, "Error in _VTableMakeCursor"
                "$(path), READ)", "path=%s", desc.run->path));
        }
    }
    if (rc == 0) {
        rc = VCursorReadDirect
            (curs, desc.spot, col_idx, 8, buffer, sizeof buffer, &row_len);
        if (rc != 0) {
            PLOGERR(klogInt, (klogInt, rc, "Error in VCursorReadDirect"
                "$(path), READ, spot=$(spot))",
                "path=%s,spot=%ld", desc.run->path, desc.spot));
        }
    }
    VCursorRelease(curs);
    curs = NULL;
    if (rc == 0) {
        STSMSG(1, ("VdbBlastRunSetGetReadLength = %ld", row_len));
        return row_len;
    }
    else {
        STSMSG(1, ("Error: failed to VdbBlastRunSetGetReadLength"));
        return 0;
    }
}

uint64_t _VdbBlastRunSet2naRead(const VdbBlastRunSet *self,
    VdbBlastStatus *status, uint64_t *read_id, size_t *starting_base,
    uint8_t *buffer, size_t buffer_size, KVdbBlastReadMode mode)
{
    uint64_t n = 0; 
    rc_t rc = 0;
    assert(self && status);

    rc = KLockAcquire(self->core2na.mutex);
    if (rc != 0) {
        LOGERR(klogInt, rc, "Error in KLockAcquire");
    }
    else {
        const Core2na *c
            = mode != VDB_READ_REFERENCE ? &self->core2na : &self->core2naRef;
        n = _Core2naRead((Core2na*)c, &self->runs, status,
            read_id, starting_base, buffer, buffer_size);
        if (n == 0 && self->core2na.eos) {
            *read_id = ~0;
        }
        rc = KLockUnlock(self->core2na.mutex);
        if (rc != 0) {
            LOGERR(klogInt, rc, "Error in KLockUnlock");
        }
    }
    if (rc != 0) {
        *status = eVdbBlastErr;
    }

    if (*status == eVdbBlastNoErr || *status == eVdbBlastCircularSequence) {
        if (read_id != NULL && starting_base != NULL) {
            STSMSG(3, (
                "VdbBlast2naReaderRead(read_id=%ld, starting_base=%ld) = %ld",
                *read_id, *starting_base, n));
        }
        else {
            STSMSG(2, ("VdbBlast2naReaderRead = %ld", n));
        }
    }
    else {
        if (read_id != NULL && starting_base != NULL) {
            STSMSG(1, ("Error: failed to VdbBlast2naReaderRead(%ld)", n));
        }
        else {
            STSMSG(1, ("Error: failed to VdbBlast2naReaderRead"));
        }
    }
    return n;
}

/******************************************************************************/
