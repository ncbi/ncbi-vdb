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
#include <klib/vector.h>
#include <klib/sort.h>
#include <klib/data-buffer.h>
#include <klib/printf.h>
#include <insdc/insdc.h>
#include <vdb/database.h>
#include <vdb/cursor.h>
#include <sra/sradb.h>
#include <align/writer-sequence.h>
#include "writer-priv.h"
#include "reader-cmn.h"
#include "debug.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

static const TableWriterColumn TableWriterSeq_cols[ewseq_cn_Last + 1] =
{
    /* order is important, see Write below!!! */
    {0, "(INSDC:dna:text)CMP_READ", sizeof(char) * 8, ewcol_IsArray},
    {0, "(INSDC:color:text)CMP_CSREAD", sizeof(char) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "CS_KEY", sizeof(char) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "QUALITY", sizeof(INSDC_quality_phred) * 8, ewcol_IsArray},
    {0, "PRIMARY_ALIGNMENT_ID", sizeof(int64_t) * 8, ewcol_IsArray},
    {0, "ALIGNMENT_COUNT", sizeof(uint8_t) * 8, ewcol_IsArray},
    {0, "PLATFORM", sizeof(INSDC_SRA_platform_id) * 8, 0},
    {0, "LABEL", sizeof(char) * 8, ewcol_IsArray},
    {0, "LABEL_START", sizeof(INSDC_coord_zero) * 8, ewcol_IsArray},
    {0, "LABEL_LEN", sizeof(INSDC_coord_len) * 8, ewcol_IsArray},
    {0, "READ_TYPE", sizeof(SRAReadTypes) * 8, ewcol_IsArray},
    {0, "READ_START", sizeof(INSDC_coord_zero) * 8, ewcol_IsArray},
    {0, "READ_LEN", sizeof(INSDC_coord_len) * 8, ewcol_IsArray},
    {0, "TMP_KEY_ID", sizeof(uint64_t) * 8, ewcol_Temporary},
    {0, "SPOT_GROUP", sizeof(char) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "READ_FILTER", sizeof(uint8_t) * 8, ewcol_IsArray},
    {0, "TI", sizeof(uint64_t) * 8, ewcol_IsArray | ewcol_Ignore}
};

static const TableReaderColumn TableSeqReadTmpKey_cols[] = {
    {0, "TMP_KEY_ID", {NULL}, 0, 0},
    {0, "READ_LEN", {NULL}, 0, 0},
    {0, NULL, {NULL}, 0, 0}
};

struct TableWriterSeq {
    uint32_t options;
    const TableWriter* base;
    uint8_t cursor_id;
    TableWriterColumn cols[sizeof(TableWriterSeq_cols)/sizeof(TableWriterSeq_cols[0])];
    bool init; /* default written indicator */
    int64_t qual_buf_sz;
    uint8_t* qual_buf;
    uint8_t discrete_qual[256];
    uint8_t alignd_cursor_id;
    TableWriterColumn cols_alignd[2];
    const TableReader* tmpkey_reader;
    TableReaderColumn cols_read_tmpkey[sizeof(TableSeqReadTmpKey_cols) / sizeof(TableSeqReadTmpKey_cols[0])];
    KVector *stats;
    unsigned statsCount;
    int64_t tmpKeyIdFirst;
    int64_t tmpKeyIdLast;
    bool flush;
};

static bool TableWriterSeq_InitQuantMatrix(uint8_t dst[256], char const quant[])
{
    unsigned i = 0;
    unsigned limit = 0;
    unsigned value = 0;
    unsigned cur;
    int ws = 1;
    int st = 0;
    
    memset(dst, 0, 256);
    for (cur = 0; quant[cur] != 0; ++cur) {
        int const ch = quant[cur];
        
        if (ws) {
            if (isspace(ch))
                continue;
            ws = false;
        }
        switch (st) {
        case 0:
            if (isdigit(ch)) {
                value = (value * 10) + ch - '0';
                break;
            }
            else if (isspace(ch)) {
                ++st;
                ws = true;
                break;
            }
            ++st;
            /* no break */
        case 1:
            if (ch != ':')
                return false;
            ws = true;
            ++st;
            break;
        case 2:
            if (isdigit(ch)) {
                limit  = (limit * 10) + ch - '0';
                break;
            }
            else if (isspace(ch)) {
                ++st;
                ws = true;
                break;
            }
            else if (ch == '-' && limit == 0) {
                memset(dst + i, value, 256 - i);
                return true;
            }
            ++st;
            /* no break */
        case 3:
            if (ch != ',')
                return false;
            ws = true;
            st = 0;
            if (i > limit)
                return false;
            memset(dst + i, value, limit - i);
            i = limit;
            limit = value = 0;
            break;
        }
    }
    if (st == 0) {
        switch (value) {
        case 1:
            break;
        case 2:
            break;
        }
    }
    return false;
}

typedef struct { uint64_t distance, count; } stats_pair_t;

typedef struct { 
    stats_pair_t *stats;
    unsigned i;
} pb_t;

static rc_t CC stats_cb(uint64_t key, uint64_t value, void *user_data)
{
    pb_t *pb = user_data;
    
    pb->stats[pb->i].distance = key;
    pb->stats[pb->i].count = value;
    ++pb->i;
    
    return 0;
}

static int CC stats_cmp_count(const void *A, const void *B, void *ignore)
{
    if (((stats_pair_t const *)A)->distance == 0)
        return -1;
    if (((stats_pair_t const *)B)->distance == 0)
        return 1;
    if (((stats_pair_t const *)B)->count < ((stats_pair_t const *)A)->count)
        return -1;
    if (((stats_pair_t const *)B)->count > ((stats_pair_t const *)A)->count)
        return 1;
    if (((stats_pair_t const *)B)->distance < ((stats_pair_t const *)A)->distance)
        return 1;
    if (((stats_pair_t const *)B)->distance > ((stats_pair_t const *)A)->distance)
        return -1;
    return 0;
}

static int CC stats_cmp_distance(const void *A, const void *B, void *ignore)
{
    if (((stats_pair_t const *)A)->distance < ((stats_pair_t const *)B)->distance)
        return -1;
    if (((stats_pair_t const *)A)->distance > ((stats_pair_t const *)B)->distance)
        return 1;
    return 0;
}

static rc_t TableWriterSeq_WriteStatistics(TableWriterSeq const *cself, KMDataNode *node)
{
    pb_t pb;
    rc_t rc;
    KDataBuffer buf;
    
    rc = KDataBufferMake(&buf, 8 * sizeof(pb.stats[0]), cself->statsCount);
    if (rc) return rc;
    
    pb.stats = buf.base;
    pb.i = 0;
    rc = KVectorVisitU64(cself->stats, 0, stats_cb, &pb);
    if (rc == 0) {
        unsigned i;
        unsigned const n = cself->statsCount < 126 ? cself->statsCount : 126;
        uint64_t *const distance = buf.base;
        
        ksort(pb.stats, cself->statsCount, sizeof(pb.stats[0]), stats_cmp_count, NULL);
        ksort(pb.stats, n, sizeof(pb.stats[0]), stats_cmp_distance, NULL);
        for (i = 0; i != n; ++i) {
            distance[i] = pb.stats[i].distance;
        }
        rc = KMDataNodeWrite(node, distance, n * sizeof(distance[0]));
    }
    KDataBufferWhack(&buf);
    return rc;
}

static rc_t TableWriterSeq_CollectStatistics(TableWriterSeq *self, const TableWriterData* primary_alignment_id)
{
    rc_t rc;
    int64_t mate[2] = {0, 0};
    uint64_t distance = 0;
    uint64_t count = 0;
    
    memcpy(mate, primary_alignment_id->buffer, sizeof(mate[0]) * primary_alignment_id->elements);
    if (mate[0] && mate[1]) {
        distance = (mate[0] < mate[1]) ? (mate[1] - mate[0]) : (mate[0] - mate[1]);
    }
    else
        return 0;
    if (self->stats == NULL) {
        rc = KVectorMake(&self->stats); if (rc) return rc;
    }
    rc = KVectorGetU64(self->stats, distance, &count);
    if (GetRCObject(rc) == rcItem && GetRCState(rc) == rcNotFound) {
        rc = KVectorSetU64(self->stats, distance, 1);
        if (rc == 0) ++self->statsCount;
    }
    else if (rc == 0) {
        ++count;
        rc = KVectorSetU64(self->stats, distance, count);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_Make(const TableWriterSeq** cself, VDatabase* db,
                                       uint32_t options, char const qual_quantization[])
{
    rc_t rc = 0;
    TableWriterSeq* self = NULL;
    char const *tblName = (options & ewseq_co_ColorSpace) ? "CS_SEQUENCE" : "SEQUENCE";

    options |= ewseq_co_SaveQual; /* TODO: remove when ready */
    if( cself == NULL || db == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            memcpy(self->cols, TableWriterSeq_cols, sizeof(TableWriterSeq_cols));
            if( options & ewseq_co_AlignData ) {
                self->cols[ewseq_cn_TMP_KEY_ID].flags |= ewcol_Ignore;
            } else {
                self->cols[ewseq_cn_PRIMARY_ALIGNMENT_ID].flags |= ewcol_Ignore;
                self->cols[ewseq_cn_ALIGNMENT_COUNT].flags |= ewcol_Ignore;
            }
            if(options & ewseq_co_NoLabelData) {
                self->cols[ewseq_cn_LABEL].flags |= ewcol_Ignore;
                self->cols[ewseq_cn_LABEL_LEN].flags |= ewcol_Ignore;
                self->cols[ewseq_cn_LABEL_START].flags |= ewcol_Ignore;
            }
            if(options & ewseq_co_ColorSpace) {
                self->cols[ewseq_cn_READ].flags |= ewcol_Ignore;
                self->cols[ewseq_cn_CSREAD].flags &= ~ewcol_Ignore;
                self->cols[ewseq_cn_CSKEY].flags &= ~ewcol_Ignore;
            }
            if( options & ewseq_co_SpotGroup) {
                self->cols[ewseq_cn_SPOT_GROUP].flags &= ~ewcol_Ignore;
            }
            if( options & ewseq_co_TI) {
                self->cols[ewseq_cn_TI].flags &= ~ewcol_Ignore;
            }
            if( (rc = TableWriter_Make(&self->base, db, tblName, "SEQUENCE")) == 0 ) {
                rc = TableWriter_AddCursor(self->base, self->cols, sizeof(self->cols)/sizeof(self->cols[0]), &self->cursor_id);
            }
        }
        if( rc == 0 ) {
            self->options = options;
            if (qual_quantization && strcmp(qual_quantization, "0") == 0) {
                self->options |= ewseq_co_FullQuality;
            }
            if( !(self->options & ewseq_co_FullQuality) ) {
                char const *quant_string = qual_quantization;
                
                if (quant_string == NULL || strcmp(quant_string, "1") == 0) {
                    quant_string = "1:10,10:20,20:30,30:-";
                } else if (strcmp(quant_string, "2") == 0) {
                    quant_string = "1:30,30:-";
                }
                if (!TableWriterSeq_InitQuantMatrix(self->discrete_qual, quant_string)) {
                    rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcInvalid);
                }
            }
        }
    }
    if( rc == 0 ) {
        *cself = self;
        ALIGN_DBG("table %s", "created");
    } else {
        TableWriterSeq_Whack(self, false, NULL);
        ALIGN_DBGERR(rc);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_Whack(const TableWriterSeq* cself, bool commit, uint64_t* rows)
{
    rc_t rc = 0;

    if( cself != NULL ) {
        TableWriterSeq* self = (TableWriterSeq*)cself;
        VTable *vtbl;
        
        TableReader_Whack(cself->tmpkey_reader);
#if 1
        if (cself->stats && commit && (rc = TableWriter_GetVTable(cself->base, &vtbl)) == 0 ) {
            KMetadata* meta;
            if( (rc = VTableOpenMetadataUpdate(vtbl, &meta)) == 0 ) {
                KMDataNode* node;
                if( (rc = KMetadataOpenNodeUpdate(meta, &node, "MATE_STATISTICS")) == 0 ) {
                    rc = TableWriterSeq_WriteStatistics(cself, node);
                    KMDataNodeRelease(node);
                }
                KMetadataRelease(meta);
            }
        }
#endif
        rc = TableWriter_Whack(cself->base, commit && (rc == 0), rows);
        KVectorRelease(cself->stats);
        free(self->qual_buf);
        free(self);
    }
    return rc;
}

static
rc_t TableWriteSeq_WriteDefaults(const TableWriterSeq* cself)
{
    rc_t rc = 0;
    if( cself != NULL ) {
        TableWriterSeq* self = (TableWriterSeq*)cself;
        self->init = true;
        if( (self->options & ewseq_co_AlignData) ) {
            static TableWriterData const d = { "", 0 };
            TW_COL_WRITE_DEF(self->base, cself->cursor_id, self->cols[ewseq_cn_PRIMARY_ALIGNMENT_ID], d);
            TW_COL_WRITE_DEF(self->base, cself->cursor_id, self->cols[ewseq_cn_ALIGNMENT_COUNT], d);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriteSeq_WriteDefault(const TableWriterSeq* cself,
                                              enum ETableWriterSeq_ColNames col, const TableWriterData* data)
{
    rc_t rc = 0;
    if( !cself->init ) {
        rc = TableWriteSeq_WriteDefaults(cself);
    }
    if( rc == 0 ) {
        rc = TableWriter_ColumnDefault(cself->base, cself->cursor_id, &cself->cols[col], data);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_GetNextRowId(const TableWriterSeq* cself, int64_t* rowid)
{
    return TableWriter_GetNextRowId(cself->base, rowid, cself->cursor_id);
}

LIB_EXPORT rc_t CC TableWriterSeq_Write(const TableWriterSeq* cself, const TableWriterSeqData* data, int64_t* rowid)
{
    rc_t rc = 0;
    int lbl;
    
    if( cself == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( !cself->init && (rc = TableWriteSeq_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else if( data->quality.buffer == NULL || data->sequence.elements != data->quality.elements ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, data->quality.buffer ? rcInconsistent : rcEmpty);
        ALIGN_DBGERRP("sequence and quality length %lu <> %lu", rc, data->sequence.elements, data->quality.elements);
    } else if( data->read_start.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and read_start length %u <> %lu", rc, data->nreads, data->read_start.elements);
    } else if( data->read_len.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent );
        ALIGN_DBGERRP("nreads and read_len length %u <> %lu", rc, data->nreads, data->read_len.elements);
    } else if( (cself->options & ewseq_co_AlignData) && data->primary_alignment_id.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and primary_alignment_id length %u <> %lu", rc, data->nreads, data->primary_alignment_id.elements);
    } else if( (cself->options & ewseq_co_AlignData) && data->alignment_count.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and alignment_count length %u <> %lu", rc, data->nreads, data->alignment_count.elements);
    } else if( data->no_quantize_mask.buffer && data->no_quantize_mask.elements != data->quality.elements ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("quality and no_quantize_mask length %u <> %lu", rc, data->quality.elements, data->no_quantize_mask.elements);
    } else if( !(cself->options & ewseq_co_NoLabelData) &&
               (lbl = ((data->label.buffer ? 1 : 0) + (data->label_start.buffer ? 1 : 0) +
                                                  (data->label_len.buffer ? 1 : 0))) != 0 && lbl != 3 ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("LABEL %s", rc, "incomplete");
    } else if( (rc = TableWriter_OpenRow(cself->base, rowid, cself->cursor_id)) == 0 ) {
        if( cself->options & ewseq_co_AlignData ) {
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_PRIMARY_ALIGNMENT_ID], data->primary_alignment_id);
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_ALIGNMENT_COUNT], data->alignment_count);
            if (rc == 0) {
                rc = TableWriterSeq_CollectStatistics((TableWriterSeq *)cself, &data->primary_alignment_id);
            }
        }
        if( cself->options & ewseq_co_SaveRead ) {
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_READ], data->sequence);
        }
        else {
            uint32_t i;
            const char* seq = data->sequence.buffer;
            const uint8_t* ac = data->alignment_count.buffer;
            const INSDC_coord_zero* rs = data->read_start.buffer;
            const INSDC_coord_len* rl = data->read_len.buffer;

            TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_READ], NULL, 0);
            TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_CSREAD], NULL, 0);
            for(i = 0; i < data->nreads; i++ ) {
                if( ac[i] == 0 ) {
                    TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_READ], &seq[rs[i]], rl[i]);
                    TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_CSREAD], &seq[rs[i]], rl[i]);
                }
            }
        }
        if( cself->options & ewseq_co_FullQuality ) {
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_QUALITY], data->quality);
        } else {
            uint64_t i;
            const uint8_t* b = data->quality.buffer;
            if( data->quality.elements > cself->qual_buf_sz ) {
                uint8_t* p = realloc(cself->qual_buf, data->quality.elements * cself->cols[ewseq_cn_QUALITY].element_bits);
                if( p == NULL ) {
                    rc = RC(rcAlign, rcType, rcWriting, rcMemory, rcExhausted);
                    ALIGN_DBGERRP("quality %s", rc, "discretization");
                } else {
                    ((TableWriterSeq*)cself)->qual_buf_sz = data->quality.elements;
                    ((TableWriterSeq*)cself)->qual_buf = p;
                }
            }
            if (data->no_quantize_mask.buffer) {
                bool const *mask = data->no_quantize_mask.buffer;
                
                for(i = 0; i < data->quality.elements; i++ ) {
                    uint8_t const q = b[i] & 0x3F;
                    cself->qual_buf[i] = mask[i] ? q : cself->discrete_qual[q];
                }
            }
            else {
                for(i = 0; i < data->quality.elements; i++ ) {
                    cself->qual_buf[i] = cself->discrete_qual[b[i]];
                }
            }
            if (cself->options & ewseq_co_SaveQual) {
                TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_QUALITY], cself->qual_buf, data->quality.elements);
            }
            else {
                uint32_t i;
                uint8_t const *const qual = data->sequence.buffer;
                uint8_t const *const aligned = data->alignment_count.buffer;
                INSDC_coord_zero const *const rs = data->read_start.buffer;
                INSDC_coord_len const *const rl = data->read_len.buffer;
                
                TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_QUALITY], NULL, 0);
                for (i = 0; i < data->nreads; ++i) {
                    if (!aligned[i]) {
                        INSDC_coord_zero const readStart = rs[i];
                        INSDC_coord_len const readLen = rl[i];
                        
                        TW_COL_WRITE_BUF(cself->base, cself->cols[ewseq_cn_QUALITY], &qual[readStart], readLen);
                    }
                }
            }
        }
        if( !(cself->options & ewseq_co_NoLabelData) ) {
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_LABEL], data->label);
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_LABEL_START], data->label_start);
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_LABEL_LEN], data->label_len);
        }
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_READ_TYPE], data->read_type);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_READ_START], data->read_start);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_READ_LEN], data->read_len);
        TW_COL_WRITE_VAR(cself->base, cself->cols[ewseq_cn_TMP_KEY_ID], data->tmp_key_id);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_SPOT_GROUP], data->spot_group);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_CSKEY], data->cskey);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_READ_FILTER], data->read_filter);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_PLATFORM], data->platform);
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_TI], data->ti);
        if( rc == 0 ) {
            rc = TableWriter_CloseRow(cself->base);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_TmpKeyStart(const TableWriterSeq* cself)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->options & ewseq_co_AlignData ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcViolated);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableWriter_CloseCursor(cself->base, 0, NULL)) == 0 ) {
        TableWriterSeq* self = (TableWriterSeq*)cself;
        VTable* vtbl = NULL;
        
        self->tmpKeyIdFirst = INT64_MAX;
        self->tmpKeyIdLast = INT64_MIN;
        memcpy(&self->cols_read_tmpkey, &TableSeqReadTmpKey_cols, sizeof(TableSeqReadTmpKey_cols));
        if( (rc = TableWriter_GetVTable(cself->base, &vtbl)) == 0 &&
            (rc = TableReader_Make(&self->tmpkey_reader, vtbl, self->cols_read_tmpkey, 50 * 1024 * 1024)) == 0 ) {
            memcpy(self->cols_alignd, &TableWriterSeq_cols[ewseq_cn_PRIMARY_ALIGNMENT_ID], sizeof(self->cols_alignd));
            rc = TableWriter_AddCursor(self->base, self->cols_alignd,
                                       sizeof(self->cols_alignd) / sizeof(self->cols_alignd[0]),
                                       &self->alignd_cursor_id);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_TmpKey(const TableWriterSeq* cself, int64_t rowid, uint64_t* key_id)
{
    rc_t rc = 0;

    if( cself == NULL || rowid == 0 || key_id == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->tmpkey_reader == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcMode, rcNotOpen);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableReader_ReadRow(cself->tmpkey_reader, rowid)) == 0 ) {
        memcpy(key_id, cself->cols_read_tmpkey[0].base.var, sizeof(*key_id));
        if (cself->tmpKeyIdLast < rowid || rowid < cself->tmpKeyIdFirst) {
            rc = TableReader_PageIdRange(cself->tmpkey_reader, rowid,
                                        &((TableWriterSeq*)cself)->tmpKeyIdFirst,
                                        &((TableWriterSeq*)cself)->tmpKeyIdLast);
            ((TableWriterSeq*)cself)->flush = rc == 0;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_WriteAlignmentData(const TableWriterSeq* cself, int64_t rowid,
                                                     const TableWriterData* primary_alignment_id,
                                                     const TableWriterData* alignment_count)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->options & ewseq_co_AlignData ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcViolated);
        ALIGN_DBGERR(rc);
    } else if( rowid == 0 ) {
        rc = TableWriter_Flush(cself->base, cself->alignd_cursor_id);
    }
    else if( primary_alignment_id == NULL || alignment_count == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableReader_ReadRow(cself->tmpkey_reader, rowid)) != 0 ||
        cself->cols_read_tmpkey[1].len != primary_alignment_id->elements ) {
        rc = rc ? rc : RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and primary_alignment_id length %u <> %lu",
            rc, cself->cols_read_tmpkey[1].len, primary_alignment_id->elements);
    } else if( primary_alignment_id->elements != alignment_count->elements ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("primary_alignment_id and alignment_count length %u <> %lu",
            rc, primary_alignment_id->elements, alignment_count->elements);
    }
    else {
        if (cself->flush) {
            rc = TableWriter_Flush(cself->base, cself->alignd_cursor_id);
            ((TableWriterSeq*)cself)->flush = false;
        }
        if( rc == 0 &&
           (rc = TableWriter_OpenRowId(cself->base, rowid, cself->alignd_cursor_id)) == 0 ) {
            TW_COL_WRITE(cself->base, cself->cols_alignd[0], *primary_alignment_id);
            TW_COL_WRITE(cself->base, cself->cols_alignd[1], *alignment_count);
            if( rc == 0 ) {
                rc = TableWriter_CloseRow(cself->base);
            }
            if (rc == 0/* && (cself->options & ewseq_co_WantMateStats)*/) {
                rc = TableWriterSeq_CollectStatistics((TableWriterSeq *)cself, primary_alignment_id);
            }
        }
    }
    return rc;
}
