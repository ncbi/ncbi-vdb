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
#include <align/extern.h>

#include <klib/rc.h>
#include <kfs/file.h>
#include <insdc/insdc.h>
#include <vdb/table.h>
#include <align/writer-refseq.h>
#include "writer-ref.h"
#include "writer-priv.h"
#include "debug.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

static const TableWriterColumn TableWriterRefData_cols[ewrefd_cn_Last + 1] =
{
    /* order is important, see enum in .h !!! */
    {0, "MAX_SEQ_LEN", sizeof(uint32_t) * 8, 0},
    {0, "NAME", sizeof(char) * 8, ewcol_IsArray},
    {0, "(INSDC:dna:text)CS_KEY", sizeof(char) * 8, 0},
    {0, "(INSDC:dna:text)CMP_READ", sizeof(char) * 8, ewcol_IsArray},
    {0, "QUALITY", sizeof(INSDC_quality_phred) * 8, ewcol_IsArray},
    {0, "SEQ_ID", sizeof(char) * 8, ewcol_IsArray},
    {0, "SEQ_START", sizeof(INSDC_coord_one) * 8, 0},
    {0, "SEQ_LEN", sizeof(INSDC_coord_len) * 8, 0},
    {0, "CIRCULAR", sizeof(bool) * 8, 0}
};

static const TableWriterColumn TableWriterRefCoverage_cols[ewrefcv_cn_Last + 1] =
{
    /* order is important, see enum in .h !!! */
    {0, "CGRAPH_HIGH", sizeof(uint8_t) * 8, 0},
    {0, "CGRAPH_LOW", sizeof(uint8_t) * 8, 0},
    {0, "CGRAPH_MISMATCHES", sizeof(uint32_t) * 8, 0},
    {0, "CGRAPH_INDELS", sizeof(uint32_t) * 8, 0},
    {0, "OVERLAP_REF_POS", sizeof(INSDC_coord_zero) * 8, ewcol_IsArray},
    {0, "OVERLAP_REF_LEN", sizeof(INSDC_coord_len) * 8, ewcol_IsArray},
    {0, "PRIMARY_ALIGNMENT_IDS", sizeof(int64_t) * 8, ewcol_IsArray},
    {0, "SECONDARY_ALIGNMENT_IDS", sizeof(int64_t) * 8, ewcol_IsArray},
    {0, "EVIDENCE_INTERVAL_IDS", sizeof(int64_t) * 8, ewcol_IsArray}
};

struct TableWriterRef {
    uint32_t options;
    const TableWriter* base;
    uint8_t cursor_id;
    TableWriterColumn cols_data[sizeof(TableWriterRefData_cols)/sizeof(TableWriterRefData_cols[0])];
    uint8_t coverage_cursor_id;
    TableWriterColumn cols_coverage[sizeof(TableWriterRefCoverage_cols)/sizeof(TableWriterRefCoverage_cols[0])];
    INSDC_coord_one seq_start_last;
    uint32_t max_seq_len;
    char last_cs_key;
    uint64_t last_seq_id_len;
    char last_seq_id[2048];
    int64_t last_coverage_row;
};

rc_t CC TableWriterRef_Make(const TableWriterRef** cself, VDatabase* db, const uint32_t options)
{
    rc_t rc = 0;
    TableWriterRef* self = NULL;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            memmove(self->cols_data, TableWriterRefData_cols, sizeof(TableWriterRefData_cols));
            memmove(self->cols_coverage, TableWriterRefCoverage_cols, sizeof(TableWriterRefCoverage_cols));
            if( !(options & ewref_co_QUALITY) ) {
                self->cols_data[ewrefd_cn_QUALITY].flags |= ewcol_Ignore;
            }
            if( (rc = TableWriter_Make(&self->base, db, "REFERENCE", NULL)) == 0 &&
                (rc = TableWriter_AddCursor(self->base, self->cols_data,
                            sizeof(self->cols_data) / sizeof(self->cols_data[0]), &self->cursor_id)) == 0 ) {
                self->options = options;
            }
            if( rc == 0 && (options & ewref_co_Coverage) ) {
                rc = TableWriter_AddCursor(self->base, self->cols_coverage,
                    sizeof(self->cols_coverage) / sizeof(self->cols_coverage[0]), &self->coverage_cursor_id);
            }
        }
    }
    if( rc == 0 ) {
        *cself = self;
        ALIGN_DBG("table %s", "created");
    } else {
        TableWriterRef_Whack(self, false, NULL);
        ALIGN_DBGERR(rc);
    }
    return rc;
}

static
rc_t TableWriterRef_DefaultCoverage(const TableWriterRef* cself, int64_t rowid)
{
    rc_t rc = 0;
    int64_t i;
    ReferenceSeqCoverage c;

    memset(&c, 0, sizeof(c));
    for(i = cself->last_coverage_row + 1; rc == 0 && i < rowid; i++) {
        rc = TableWriterRef_WriteCoverage(cself, i, 0, &c);
    }
    return rc;
}

rc_t CC TableWriterRef_Whack(const TableWriterRef* cself, bool commit, uint64_t* rows)
{
    rc_t rc = 0;
    if( cself != NULL ) {
        TableWriterRef* self = (TableWriterRef*)cself;

        if( commit && (cself->options & ewref_co_Coverage) ) {
            int64_t next_row;
            if( (rc = TableWriter_GetNextRowId(cself->base, &next_row, 0)) == 0 ) {
                if( (next_row - 1) > cself->last_coverage_row ) {
                    ALIGN_DBG("adding default coverage to tail of table (%li,%li]", cself->last_coverage_row, next_row);
                    rc = TableWriterRef_DefaultCoverage(cself, next_row);
                }
            }
        }
        if( rc == 0 ) {
            rc = TableWriter_Whack(cself->base, commit, rows);
        }
        free(self);
    }
    return rc;
}

static
rc_t TableWriterRef_WriteDefaults(const TableWriterRef* cself)
{
    rc_t rc = 0;
    if( cself != NULL ) {
        TableWriterRef* self = (TableWriterRef*)cself;
        self->max_seq_len = TableWriterRefSeq_MAX_SEQ_LEN;
        self->seq_start_last = 1;
        self->last_cs_key = 'T';
        self->last_seq_id_len = 0;
        TW_COL_WRITE_DEF_VAR(self->base, cself->cursor_id, self->cols_data[ewrefd_cn_MAX_SEQ_LEN], self->max_seq_len);
        if( rc == 0 && (cself->options & ewref_co_Coverage) ) {
            ReferenceSeqCoverage c;

            memset(&c, 0, sizeof(c));
            TW_COL_WRITE_DEF_VAR(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_CGRAPH_HIGH], c.low);
            TW_COL_WRITE_DEF_VAR(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_CGRAPH_LOW], c.high);
            TW_COL_WRITE_DEF_VAR(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_CGRAPH_MISMATCHES], c.mismatches);
            TW_COL_WRITE_DEF_VAR(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_CGRAPH_INDELS], c.indels);
            TW_COL_WRITE_DEF_BUF(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_OVERLAP_REF_POS],
                    c.overlap_ref_pos, sizeof(c.overlap_ref_pos) / sizeof(c.overlap_ref_pos[0]));
            TW_COL_WRITE_DEF_BUF(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_OVERLAP_REF_LEN],
                    c.overlap_ref_len, sizeof(c.overlap_ref_len) / sizeof(c.overlap_ref_len[0]));
            TW_COL_WRITE_DEF(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_PRIMARY_ALIGNMENT_IDS], c.ids[ewrefcov_primary_table]);
            TW_COL_WRITE_DEF(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_SECONDARY_ALIGNMENT_IDS], c.ids[ewrefcov_secondary_table]);
            TW_COL_WRITE_DEF(self->base, self->coverage_cursor_id, self->cols_coverage[ewrefcv_cn_EVIDENCE_INTERVAL_IDS], c.ids[ewrefcov_evidence_table]);
        }
    }
    return rc;
}

rc_t CC TableWriterRef_WriteDefaultData(const TableWriterRef* cself, enum ETableWriterRefData_ColNames col,
                                        const TableWriterData* data)
{
    rc_t rc = 0;
    if( cself == NULL || data == NULL ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->last_cs_key == '\0' && (rc = TableWriterRef_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else {
        TW_COL_WRITE_DEF(cself->base, cself->cursor_id, cself->cols_data[col], (*data));
        if( col == ewrefd_cn_MAX_SEQ_LEN ) {
            ((TableWriterRef*)cself)->max_seq_len = *((uint32_t*)data->buffer);
        }
    }
    return rc;
}

rc_t CC TableWriterRef_WriteDefaultCovarage(const TableWriterRef* cself, enum ETableWriterRefCoverage_ColNames col,
                                            const TableWriterData* data)
{
    rc_t rc = 0;
    if( cself == NULL || data == NULL ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->last_cs_key == '\0' && (rc = TableWriterRef_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else {
        TW_COL_WRITE_DEF(cself->base, cself->coverage_cursor_id, cself->cols_coverage[col], (*data));
    }
    return rc;
}

rc_t CC TableWriterRef_Write(const TableWriterRef* cself, const TableWriterRefData* data, int64_t* rowid)
{
    rc_t rc = 0;

    if( cself == NULL || data == NULL ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->last_cs_key == '\0' && (rc = TableWriterRef_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else if( data->read.elements > cself->max_seq_len ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInvalid);
        ALIGN_DBGERRP("%s is longer than %s", rc, cself->cols_data[ewrefd_cn_READ].name, cself->cols_data[ewrefd_cn_MAX_SEQ_LEN].name);
    } else if( data->seq_id.elements > sizeof(cself->last_seq_id) ) {
        rc = RC(rcAlign, rcType, rcWriting, rcBuffer, rcInsufficient);
        ALIGN_DBGERRP("%s is longer than %s", rc, cself->cols_data[ewrefd_cn_READ].name, cself->cols_data[ewrefd_cn_MAX_SEQ_LEN].name);
    } else if( (rc = TableWriter_OpenRow(cself->base, rowid, cself->cursor_id)) == 0 ) {
        const INSDC_dna_text* b = data->read.buffer;
        INSDC_coord_len const len = data->read.elements;
        bool write_read = data->seq_id.elements < 1 || data->force_READ_write;
        INSDC_coord_len trim_len = len;
        
        if( cself->last_seq_id_len != data->seq_id.elements ||
            strncmp(cself->last_seq_id, data->seq_id.buffer, cself->last_seq_id_len) != 0 ) {
            /* new seq_id: reset counters and mem it */
            TableWriterRef* self = (TableWriterRef*)cself;
            self->last_seq_id_len = data->seq_id.elements;
            memmove(self->last_seq_id, data->seq_id.buffer, cself->last_seq_id_len);
            self->seq_start_last = 1;
            self->last_cs_key = 'T';
        }
        /* trunc trailing N */
        b += len - 1;
        while( trim_len > 0 && (*b == 'N' || *b == 'n' || *b == '.') ) {
            trim_len--; b--;
        }
        TW_COL_WRITE(cself->base, cself->cols_data[ewrefd_cn_NAME], data->name);
        TW_COL_WRITE_BUF(cself->base, cself->cols_data[ewrefd_cn_READ], data->read.buffer, (write_read ? trim_len : 0));
        TW_COL_WRITE(cself->base, cself->cols_data[ewrefd_cn_QUALITY], data->quality);
        TW_COL_WRITE(cself->base, cself->cols_data[ewrefd_cn_SEQ_ID], data->seq_id);
        if( trim_len == 0 ) {
            TW_COL_WRITE_VAR(cself->base, cself->cols_data[ewrefd_cn_SEQ_START], trim_len);
        } else {
            TW_COL_WRITE_VAR(cself->base, cself->cols_data[ewrefd_cn_SEQ_START], cself->seq_start_last);
        }
        TW_COL_WRITE_VAR(cself->base, cself->cols_data[ewrefd_cn_SEQ_LEN], len);
        TW_COL_WRITE_VAR(cself->base, cself->cols_data[ewrefd_cn_CS_KEY], cself->last_cs_key);
        TW_COL_WRITE_VAR(cself->base, cself->cols_data[ewrefd_cn_CIRCULAR], data->circular);
        if( rc == 0 && (rc = TableWriter_CloseRow(cself->base)) == 0 ) {
            TableWriterRef* self = (TableWriterRef*)cself;
            
            self->last_cs_key = ((const char*)(data->read.buffer))[data->read.elements - 1];
            switch (self->last_cs_key) {
            case 'A':
            case 'C':
            case 'G':
            case 'T':
                break;
            default:
                self->last_cs_key = 'T';
                break;
            }
            self->seq_start_last += data->read.elements;
        }
    }
    return rc;
}

rc_t CC TableWriterRef_WriteCoverage(const TableWriterRef* cself, int64_t rowid, INSDC_coord_zero offset, const ReferenceSeqCoverage* coverage)
{
    rc_t rc = 0;

    if( cself == NULL || coverage == NULL ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->last_cs_key == '\0' && (rc = TableWriterRef_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else if( !(cself->options & ewref_co_Coverage) ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcUnexpected);
        ALIGN_DBGERRP("coverage %s", rc, "data");
    } else if( offset > cself->seq_start_last ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInvalid);
        ALIGN_DBGERRP("%i is beyond last written chunk", rc, offset);
    } else {
        if( (rowid + offset / cself->max_seq_len) - cself->last_coverage_row > 1 ) {
            /* fill in gap in coverage rows with default values */
            ALIGN_DBG("gap in coverage rows (%li,%li) -> use defaults", cself->last_coverage_row, rowid + offset / cself->max_seq_len);
            rc = TableWriterRef_DefaultCoverage(cself, rowid + offset / cself->max_seq_len);
        }
        if( rc == 0 && (rc = TableWriter_OpenRowId(cself->base, rowid + offset / cself->max_seq_len, cself->coverage_cursor_id)) == 0 ) {
            TW_COL_WRITE_VAR(cself->base, cself->cols_coverage[ewrefcv_cn_CGRAPH_HIGH], coverage->high);
            TW_COL_WRITE_VAR(cself->base, cself->cols_coverage[ewrefcv_cn_CGRAPH_LOW], coverage->low);
            TW_COL_WRITE_VAR(cself->base, cself->cols_coverage[ewrefcv_cn_CGRAPH_MISMATCHES], coverage->mismatches);
            TW_COL_WRITE_VAR(cself->base, cself->cols_coverage[ewrefcv_cn_CGRAPH_INDELS], coverage->indels);
            TW_COL_WRITE_BUF(cself->base, cself->cols_coverage[ewrefcv_cn_OVERLAP_REF_POS],
                    coverage->overlap_ref_pos, sizeof(coverage->overlap_ref_pos) / sizeof(coverage->overlap_ref_pos[0]));
            TW_COL_WRITE_BUF(cself->base, cself->cols_coverage[ewrefcv_cn_OVERLAP_REF_LEN],
                    coverage->overlap_ref_len, sizeof(coverage->overlap_ref_len) / sizeof(coverage->overlap_ref_len[0]));
            TW_COL_WRITE(cself->base, cself->cols_coverage[ewrefcv_cn_PRIMARY_ALIGNMENT_IDS], coverage->ids[ewrefcov_primary_table]);
            TW_COL_WRITE(cself->base, cself->cols_coverage[ewrefcv_cn_SECONDARY_ALIGNMENT_IDS], coverage->ids[ewrefcov_secondary_table]);
            TW_COL_WRITE(cself->base, cself->cols_coverage[ewrefcv_cn_EVIDENCE_INTERVAL_IDS], coverage->ids[ewrefcov_evidence_table]);
            if( rc == 0 ) {
                TableWriterRef* self = (TableWriterRef*)cself;
                self->last_coverage_row = rowid + offset / cself->max_seq_len;
                rc = TableWriter_CloseRow(cself->base);
            }
        }
    }
    return rc;
}

struct TableWriterRefCoverage {
    const TableWriter* base;
    bool init; /* default written indicator */
    uint8_t cursor_id;
    TableWriterColumn cols[ewrefcv_cn_ReCover + 1];
};

rc_t CC TableWriterRefCoverage_MakeCoverage(const TableWriterRefCoverage** cself, VDatabase* db, const uint32_t options)
{
    rc_t rc = 0;
    TableWriterRefCoverage* self = NULL;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            memmove(self->cols, TableWriterRefCoverage_cols, sizeof(self->cols));
            if( (rc = TableWriter_MakeUpdate(&self->base, db, "REFERENCE")) == 0 ) {
                rc = TableWriter_AddCursor(self->base, self->cols,
                        sizeof(self->cols) / sizeof(self->cols[0]), &self->cursor_id);
            }
        }
    }
    if( rc == 0 ) {
        *cself = self;
        ALIGN_R_DBG("table %s", "opened");
    } else {
        TableWriterRefCoverage_Whack(self, false, NULL);
        ALIGN_DBGERR(rc);
    }
    return rc;
}
rc_t CC TableWriterRefCoverage_MakeIds(const TableWriterRefCoverage** cself, VDatabase* db, const char  * col_name)
{
    rc_t rc = 0;
    TableWriterRefCoverage* self = NULL;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            if( (rc = TableWriter_MakeUpdate(&self->base, db, "REFERENCE")) == 0 ) {
                self->cols[0].idx = 0;
                self->cols[0].name = col_name;
                self->cols[0].element_bits = 8 * sizeof(int64_t);
                self->cols[0].flags=ewcol_IsArray;
                rc = TableWriter_AddCursor(self->base, self->cols,1, &self->cursor_id);
            }
        }
    }
    if( rc == 0 ) {
        *cself = self;
        ALIGN_R_DBG("table %s", "opened");
    } else {
        TableWriterRefCoverage_Whack(self, false, NULL);
        ALIGN_DBGERR(rc);
    }
    return rc;
}


rc_t CC TableWriterRefCoverage_Whack(const TableWriterRefCoverage* cself, bool commit, uint64_t* rows)
{
    rc_t rc = 0;
    if( cself != NULL ) {
        TableWriterRefCoverage* self = (TableWriterRefCoverage*)cself;
        rc = TableWriter_Whack(cself->base, commit, rows);
        free(self);
    }
    return rc;
}


rc_t CC TableWriterRefCoverage_WriteCoverage(const TableWriterRefCoverage* cself, int64_t rowid, const ReferenceSeqCoverage* coverage)
{
    rc_t rc = 0;

    if( cself == NULL || coverage == NULL ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    }
    else {
        if( !cself->init ) {
            /* set the defaults */
            ReferenceSeqCoverage c;
            
            memset(&c, 0, sizeof(c));
            TW_COL_WRITE_DEF_VAR(cself->base, cself->cursor_id, cself->cols[ewrefcv_cn_CGRAPH_HIGH], c.low);
            TW_COL_WRITE_DEF_VAR(cself->base, cself->cursor_id, cself->cols[ewrefcv_cn_CGRAPH_LOW], c.high);
            TW_COL_WRITE_DEF_VAR(cself->base, cself->cursor_id, cself->cols[ewrefcv_cn_CGRAPH_MISMATCHES], c.mismatches);
            TW_COL_WRITE_DEF_VAR(cself->base, cself->cursor_id, cself->cols[ewrefcv_cn_CGRAPH_INDELS], c.indels);
            TW_COL_WRITE_DEF_BUF(cself->base, cself->cursor_id, cself->cols[ewrefcv_cn_OVERLAP_REF_POS],
                                 c.overlap_ref_pos, sizeof(c.overlap_ref_pos) / sizeof(c.overlap_ref_pos[0]));
            TW_COL_WRITE_DEF_BUF(cself->base, cself->cursor_id, cself->cols[ewrefcv_cn_OVERLAP_REF_LEN],
                                 c.overlap_ref_len, sizeof(c.overlap_ref_len) / sizeof(c.overlap_ref_len[0]));
            
            ((TableWriterRefCoverage*)cself)->init = true;
        }
        if( rc == 0 && (rc = TableWriter_OpenRowId(cself->base, rowid, cself->cursor_id)) == 0 ) {
            TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefcv_cn_CGRAPH_HIGH], coverage->high);
            TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefcv_cn_CGRAPH_LOW], coverage->low);
            TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefcv_cn_CGRAPH_MISMATCHES], coverage->mismatches);
            TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefcv_cn_CGRAPH_INDELS], coverage->indels);
            TW_COL_WRITE_BUF(cself->base, cself->cols[ewrefcv_cn_OVERLAP_REF_POS],
                             coverage->overlap_ref_pos, sizeof(coverage->overlap_ref_pos) / sizeof(coverage->overlap_ref_pos[0]));
            TW_COL_WRITE_BUF(cself->base, cself->cols[ewrefcv_cn_OVERLAP_REF_LEN],
                             coverage->overlap_ref_len, sizeof(coverage->overlap_ref_len) / sizeof(coverage->overlap_ref_len[0]));
            
            if( rc == 0 ) {
                rc = TableWriter_CloseRow(cself->base);
            }
        }
    }
    return rc;
}
rc_t CC TableWriterRefCoverage_WriteIds(const TableWriterRefCoverage* cself, int64_t rowid, const int64_t* buf,uint32_t num)
{
    rc_t rc = 0;

    if( cself == NULL || (buf  == NULL && num > 0)) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    }
    else {
        if( !cself->init ) {
            /* set the defaults */
            TW_COL_WRITE_DEF_BUF(cself->base, cself->cursor_id, cself->cols[0],NULL,0);
            ((TableWriterRefCoverage*)cself)->init = true;
        }
        if( rc == 0 && (rc = TableWriter_OpenRowId(cself->base, rowid, cself->cursor_id)) == 0 ) {
            TW_COL_WRITE_BUF(cself->base, cself->cols[0],buf,num);
            if( rc == 0 ) {
                rc = TableWriter_CloseRow(cself->base);
            }
        }
    }
    return rc;
}

