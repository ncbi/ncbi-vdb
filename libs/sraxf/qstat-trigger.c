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

#include <sra/sradb.h>
#include <vdb/xform.h>
#include <vdb/table.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <kdb/meta.h>
#include <sysalloc.h>

#include "stat_mod.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef struct self_s self_t;
struct self_s {
    VDatabase *db;
    statistic stats;
    bool alignMode;
};

static
rc_t CC qstats_trigger_impl(void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    self_t *self = data;
    unsigned const nreads = argv[2].u.data.elem_count;
    unsigned nrofs = 0;
    unsigned i;
    unsigned start;
    rc_t rc = 0;
    
    uint8_t              const *read = argv[0].u.data.base;
    uint8_t              const *qual = argv[1].u.data.base;
    uint32_t             const *rlen = argv[2].u.data.base;
    INSDC_SRA_xread_type const *rtyp = argv[3].u.data.base;
    char                 const *sgrp = argv[4].u.data.base;
    bool                 const *hmis = NULL;
    bool                 const *hrof = NULL;
    int32_t              const *rofs = NULL;
    
    read += argv[0].u.data.first_elem;
    qual += argv[1].u.data.first_elem;
    rlen += argv[2].u.data.first_elem;
    rtyp += argv[3].u.data.first_elem;
    sgrp += argv[4].u.data.first_elem;
    
    if (argc > 5) {
        hmis = argv[5].u.data.base;
        hmis += argv[5].u.data.first_elem;

        hrof = argv[6].u.data.base;
        hrof += argv[6].u.data.first_elem;
        
        rofs = argv[7].u.data.base;
        rofs += argv[7].u.data.first_elem;
        
        nrofs = argv[7].u.data.elem_count;
    }
    for (start = i = 0; i < nreads && rc == 0; ++i) {
        row_input row;
        unsigned const len = rlen[i];
        
        memset(&row, 0, sizeof(row));
        
        row.spotgroup = sgrp;
        row.spotgroup_len = argv[4].u.data.elem_count;
        
        row.read = &read[start];
        row.quality = &qual[start];
        row.read_len = row.quality_len = len;
        
        row.reversed = (rtyp[i] & SRA_READ_TYPE_REVERSE) != 0;
        row.base_pos_offset = start;
        
        start += len;
        
        if (hmis) {
            row.has_mismatch = hmis;
            row.has_roffs = hrof;
            row.roffs = rofs;
            
            row.has_mismatch_len = row.has_roffs_len = len;
            row.roffs_len = nrofs;
        }
        rc = extract_statistic_from_row(&self->stats, &row);
    }
    return rc;
}

static char const *column_name[] = {
    "SPOT_GROUP",
    "(U32)DIMER_OFFSET",
    "DIMER",
    "(U8)HPRUN",
    "(U8)GC_CONTENT",
    "QUALITY",
    "(U32)TOTAL_COUNT",
    "(U32)MISMATCH_COUNT",
    "(U32)INSERT_COUNT",
    "(U32)DELETE_COUNT"
};

typedef struct writer_ctx_s writer_ctx_t;
struct writer_ctx_s {
    VCursor *curs;
    rc_t rc;
    bool alignMode;
    uint32_t cid[10];
};

static bool CC qstats_write(stat_row const *row, void *ctx)
{
    writer_ctx_t *self = ctx;
    rc_t rc = VCursorOpenRow(self->curs);
    
    while (rc == 0) {
        rc = VCursorWrite(self->curs, self->cid[0],  8,  row->spotgroup,  0, strlen(row->spotgroup)); if (rc) break;
        rc = VCursorWrite(self->curs, self->cid[1], 32, &row->base_pos,   0, 1);                      if (rc) break;
        rc = VCursorWrite(self->curs, self->cid[2],  8,  row->dimer,      0, 2);                      if (rc) break;
        rc = VCursorWrite(self->curs, self->cid[3],  8, &row->hp_run,     0, 1);                      if (rc) break;
        rc = VCursorWrite(self->curs, self->cid[4],  8, &row->gc_content, 0, 1);                      if (rc) break;
        rc = VCursorWrite(self->curs, self->cid[5],  8, &row->quality,    0, 1);                      if (rc) break;
        rc = VCursorWrite(self->curs, self->cid[6], 32, &row->count,      0, 1);                      if (rc) break;
        
        if (self->alignMode) {
            rc = VCursorWrite(self->curs, self->cid[7], 32, &row->mismatch_count, 0, 1); if (rc) break;
            rc = VCursorWrite(self->curs, self->cid[8], 32, &row->insert_count,   0, 1); if (rc) break;
            rc = VCursorWrite(self->curs, self->cid[9], 32, &row->delete_count,   0, 1); if (rc) break;
        }
        rc = VCursorCommitRow(self->curs); if (rc) break;
        rc = VCursorCloseRow(self->curs);
    }
    return rc == 0;
}

static rc_t OpenTableAndCursor(VDatabase *db, VTable **ptbl, VCursor **pcurs, uint32_t cid[], unsigned ncols)
{
    rc_t rc = VDatabaseCreateTable(db, ptbl, "READ_STATS",
                                   kcmCreate | kcmMD5, "READ_STATS");
    
    if (rc == 0) {
        rc = VTableColumnCreateParams(*ptbl, kcmCreate, kcsCRC32, 0);
        if (rc == 0) {
            rc = VTableCreateCursorWrite(*ptbl, &pcurs, kcmInsert);
            if (rc == 0) {
                unsigned i;
                
                for (i = 0; i < ncols && rc == 0; ++i) {
                    rc = VCursorAddColumn(*pcurs, &cid[i], "%s", column_name[i]);
                }
                if (rc == 0) {
                    rc = VCursorOpen(*pcurs);
                    if (rc == 0)
                        return 0;
                }
                VCursorRelease(*pcurs);
            }
        }
        VTableRelease(*ptbl);
    }
    *pcurs = NULL;
    *ptbl = NULL;
    return rc;
}

static void CC qstats_whack(void *vp)
{
    self_t *self = vp;
    VTable *tbl;
    writer_ctx_t ctx;
    
    memset(&ctx, 0, sizeof(ctx));
    ctx.alignMode = self->alignMode;
    
    ctx.rc = OpenTableAndCursor(self->db, &tbl, &ctx.curs, self->alignMode ? 10 : 7);
    VDatabaseRelease(self->db);
    if (ctx.rc == 0) {
        foreach_statistic(&self->stat, qstats_write, self);
        VCursorRelease(ctx.curs);
        if (ctx.rc == 0)
            VTableReindex(tbl);
        VTableRelease(tbl);
    }
    whack_statistic(&self->stat);
    free(self);
}

static rc_t qstats_trigger_make(const void *Self, const VXfactInfo *info, VFuncDesc *rslt,
                 const VFactoryParams *cp, const VFunctionParams *dp, bool alignMode)
{
    rc_t rc = 0;
    self_t *self = malloc(sizeof(*self));
    
    if (self == NULL)
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
    self->alignMode = no_algn_info;
    rc = VTableOpenParentUpdate(info->tbl, &self->db);
    if (rc == 0) {
        rc = make_statistic(&self->stat, 7, no_algn_info);
        if (rc == 0) {
            rslt->self = self;
            rslt->variant = vftIdDepRow;
            rslt->whack = qstats_whack;
            rslt->u.rf = qstats_trigger_impl;
            
            return 0;
        }
        VDatabaseRelease(self->db);
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_seq_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    return qstats_trigger_make(self, info, rslt, cp, dp, false);
}

VTRANSFACT_IMPL ( NCBI_align_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    return qstats_trigger_make(self, info, rslt, cp, dp, true);
}
