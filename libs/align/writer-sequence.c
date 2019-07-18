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
#include <klib/log.h>
#include <insdc/insdc.h>
#include <vdb/database.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
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
#include <os-native.h>
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
    {0, "TI", sizeof(uint64_t) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "RAW_NAME", sizeof(char) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "CMP_LINKAGE_GROUP", sizeof(char) * 8, ewcol_IsArray},
};

static const TableReaderColumn TableSeqReadTmpKey_cols[] = {
    {0, "TMP_KEY_ID", {NULL}, 0, 0},
    {0, "READ_LEN", {NULL}, 0, 0},
    {0, NULL, {NULL}, 0, 0}
};

static const char READ_DNA_TEXT[] = "(INSDC:dna:text)READ";

struct TableWriterSeq {
    TableWriter const *base;
    TableReader const *reader;
    uint8_t* qual_buf;
    KVector *stats;
    int64_t tmpKeyIdFirst;
    int64_t tmpKeyIdLast;
    TableWriterColumn cols[sizeof(TableWriterSeq_cols)/sizeof(TableWriterSeq_cols[0])];
    TableWriterColumn cols_alignd[2];
    TableReaderColumn cols_read[5];
    int64_t numAlignedSpots;
    int64_t numSpots;
    unsigned options;
    unsigned init; /* default written indicator */
    unsigned qual_buf_sz;
    unsigned statsCount;
    bool flush;
    uint8_t cursor_id;
    uint8_t alignd_cursor_id;
    uint8_t discrete_qual[256];
};

static void CopyTableWriterColumns(unsigned const N, TableWriterColumn *const dst, TableWriterColumn const *const src)
{
    unsigned i;
    for (i = 0; i < N; ++i)
        dst[i] = src[i];
}

static void CopyTableReaderColumns(unsigned const N, TableReaderColumn *const dst, TableReaderColumn const *const src)
{
    unsigned i;
    for (i = 0; i < N; ++i)
        dst[i] = src[i];
}

#define COPY_TableWriterColumns(DST, SRC) do { CopyTableWriterColumns(sizeof(SRC)/sizeof(*SRC), DST, SRC); } while (0)

#define COPY_TableReaderColumns(DST, SRC) do { CopyTableReaderColumns(sizeof(SRC)/sizeof(*SRC), DST, SRC); } while (0)

#define TableReaderColumn_GET(TYPE, DST, COL) do { union uv { uint8_t u[sizeof(TYPE)]; TYPE v; } uv; memmove(uv.u, (COL).base.var, sizeof(TYPE)); DST = uv.v; } while(0)

#if 0 /* always write full quality */
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
#endif

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

static int64_t CC stats_cmp_count(const void *A, const void *B, void *ignore)
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

static int64_t CC stats_cmp_distance(const void *A, const void *B, void *ignore)
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

    if (cself->stats == NULL) return 0; /* if there were no alignments */
    
    rc = KDataBufferMake(&buf, 8 * sizeof(pb.stats[0]), cself->statsCount);
    assert(rc == 0);
    if (rc) return rc;
    
    pb.stats = buf.base;
    pb.i = 0;
    rc = KVectorVisitU64(cself->stats, 0, stats_cb, &pb);
    assert(rc == 0);
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
        assert(rc == 0);
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
    
    memmove(mate, primary_alignment_id->buffer, sizeof(mate[0]) * primary_alignment_id->elements);
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

static rc_t MakeSequenceTable(TableWriterSeq *self, VDatabase* db,
                              char const qual_quantization[])
{
    char const *tblName = (self->options & ewseq_co_ColorSpace) ? "CS_SEQUENCE" : "SEQUENCE";

#if 0  /* always write full quality */
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
            return RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcInvalid);
        }
    }
#else
    self->options |= ewseq_co_FullQuality;
#endif
    
    COPY_TableWriterColumns(self->cols, TableWriterSeq_cols);

    /* always write full sequence */
    self->cols[ewseq_cn_READ].name = READ_DNA_TEXT;
    self->cols[ewseq_cn_READ].flags |= ewcol_Temporary;
    if ((self->options & ewseq_co_SaveRead) != 0) {
        self->cols[ewseq_cn_READ].flags &= ~ewcol_Temporary;
    }
    if (self->options & ewseq_co_KeepKey) {
        self->cols[ewseq_cn_TMP_KEY_ID].flags &= ~ewcol_Temporary;
    }
    if( self->options & ewseq_co_AlignData ) {
        self->cols[ewseq_cn_TMP_KEY_ID].flags |= ewcol_Ignore;
    } else {
        self->cols[ewseq_cn_PRIMARY_ALIGNMENT_ID].flags |= ewcol_Ignore;
        self->cols[ewseq_cn_ALIGNMENT_COUNT].flags |= ewcol_Ignore;
    }
    if(self->options & ewseq_co_NoLabelData) {
        self->cols[ewseq_cn_LABEL].flags |= ewcol_Ignore;
        self->cols[ewseq_cn_LABEL_LEN].flags |= ewcol_Ignore;
        self->cols[ewseq_cn_LABEL_START].flags |= ewcol_Ignore;
    }
    if(self->options & ewseq_co_ColorSpace) {
        self->cols[ewseq_cn_READ].flags |= ewcol_Ignore;
        self->cols[ewseq_cn_CSREAD].flags &= ~ewcol_Ignore;
        self->cols[ewseq_cn_CSKEY].flags &= ~ewcol_Ignore;
    }
    if( self->options & ewseq_co_SpotGroup) {
        self->cols[ewseq_cn_SPOT_GROUP].flags &= ~ewcol_Ignore;
    }
    if( self->options & ewseq_co_TI) {
        self->cols[ewseq_cn_TI].flags &= ~ewcol_Ignore;
    }
    if( self->options & ewseq_co_SpotName) {
        self->cols[ewseq_cn_NAME].flags &= ~ewcol_Ignore;
    }
    return TableWriter_Make(&self->base, db, tblName, "SEQUENCE");
}

LIB_EXPORT rc_t CC TableWriterSeq_Make(const TableWriterSeq** cself, VDatabase* db,
                                       uint32_t options, char const qual_quantization[])
{
    rc_t rc = 0;
    TableWriterSeq* self = NULL;

    if( cself == NULL || db == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            self->options = options;
            rc = MakeSequenceTable(self, db, qual_quantization);
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

struct CompressRowData {
    char const *sequence;
    INSDC_coord_zero const *read_start;
    INSDC_coord_len const *read_length;
    int64_t const *alignment_id;
    unsigned num_reads;
    unsigned sequence_length;
};
typedef struct CompressRowData CompressRowData;

static rc_t compressOneRead(VCursor *const out, CompressRowData const *data, int64_t const row, int64_t *const buffer)
{
    int64_t *const firstUnaligned = &buffer[0];
    int64_t *const firstHalfAligned = &buffer[1];
    rc_t rc;
    unsigned i;
    unsigned aligned = 0;
    
    for (i = 0; i < data->num_reads; ++i) {
        if (data->alignment_id[i] != 0)
            ++aligned;
    }
    if (aligned == 0 && *firstUnaligned == 0) *firstUnaligned = row;
    if (aligned > 0 && aligned < data->num_reads && *firstHalfAligned == 0) *firstHalfAligned = row;
    
    if (out == NULL) return 0;
    
    rc = VCursorOpenRow(out);
    assert(rc == 0); if (rc) return rc;
    
    if (aligned == 0) {
        rc = VCursorWrite(out, 1, sizeof(data->sequence[0]) * 8, data->sequence, 0, data->sequence_length);
        assert(rc == 0); if (rc) return rc;
    }
    else if (aligned == data->num_reads) {
        rc = VCursorWrite(out, 1, sizeof(data->sequence[0]) * 8, data->sequence, 0, 0);
        assert(rc == 0); if (rc) return rc;
    }
    else {
        for (i = 0; i < data->num_reads; ++i) {
            if (data->alignment_id[i] != 0) continue;

            rc = VCursorWrite(out, 1, sizeof(data->sequence[0]) * 8, data->sequence + data->read_start[i], 0, data->read_length[i]);
            assert(rc == 0); if (rc) return rc;
        }
    }
    rc = VCursorCommitRow(out); assert(rc == 0); if (rc) return rc;
    rc = VCursorCloseRow(out); assert(rc == 0); if (rc) return rc;
    return 0;
}

static VTable *getVTable(TableWriterSeq *const self) {
    VTable *vtbl = NULL;
    rc_t rc = TableWriter_GetVTable(self->base, &vtbl);
    assert(rc == 0);
    return vtbl;
}

static rc_t readOneCompressRow(VCursor const *const curs, CompressRowData *result, int64_t const row) {
    rc_t rc;
    void const *base;
    uint32_t count;
    uint32_t boff;
    uint32_t elem_bits;
    
    rc = VCursorCellDataDirect(curs, row, 1, &elem_bits, &base, &boff, &count);
    if (rc) return rc;
    assert(rc == 0 && boff == 0 && elem_bits == sizeof(result->sequence[0]) * 8);
    result->sequence = base;
    result->sequence_length = count;
    
    rc = VCursorCellDataDirect(curs, row, 2, &elem_bits, &base, &boff, &count);
    assert(rc == 0 && boff == 0 && elem_bits == sizeof(result->read_start[0]) * 8);
    result->read_start = base;
    result->num_reads = count;
    
    rc = VCursorCellDataDirect(curs, row, 3, &elem_bits, &base, &boff, &count);
    assert(rc == 0 && boff == 0 && elem_bits == sizeof(result->read_length[0]) * 8 && count == result->num_reads);
    result->read_length = base;
    
    rc = VCursorCellDataDirect(curs, row, 4, &elem_bits, &base, &boff, &count);
    assert(rc == 0 && boff == 0 && elem_bits == sizeof(result->alignment_id[0]) * 8 && count == result->num_reads);
    result->alignment_id = base;
    
    return rc;
}

/* CompressREAD has the IMPORTANT side-effect of calculating the initial row IDs
 * for half aligned and unaligned spots, needed by sra-sort
 */
static rc_t CompressREAD(TableWriterSeq *const self, int64_t *const buffer)
{
    bool const notSavingRead = ((self->options & ewseq_co_SaveRead) == 0);
    rc_t rc = 0;
    VTable *const vtbl = getVTable(self);
    VCursor const *in = NULL;
    VCursor *out = NULL;
    TableReaderColumn const cols_read[] = {
        {0, READ_DNA_TEXT, {NULL}, 0, 0},
        {0, "READ_START", {NULL}, 0, 0},
        {0, "READ_LEN", {NULL}, 0, 0},
        {0, "PRIMARY_ALIGNMENT_ID", {NULL}, 0, 0},
        {0, NULL, {NULL}, 0, 0}
    };
    CompressRowData data;
    int64_t i;
    
    rc = VTableCreateCursorRead(vtbl, &in); assert(rc == 0); if (rc) return rc;
    for (i = 0; ; ++i) {
        if (cols_read[i].name) {
            uint32_t cid = 0;
            rc = VCursorAddColumn(in, &cid, cols_read[i].name);
            assert(rc == 0 && cid == i + 1); if (rc) return rc;
            continue;
        }
        break;
    }
    rc = VCursorOpen(in); assert(rc == 0); if (rc) return rc;

    if (notSavingRead) {
        uint32_t cid = 0;
        
        rc = VTableColumnCreateParams(vtbl, kcmCreate, kcsCRC32, 0); assert(rc == 0); if (rc) return rc;
        rc = VTableCreateCursorWrite(vtbl, &out, kcmInsert); assert(rc == 0); if (rc) return rc;
        rc = VCursorAddColumn(out, &cid, TableWriterSeq_cols[0].name); assert(rc == 0 && cid == 1); if (rc) return rc;
        rc = VCursorOpen(out); assert(rc == 0); if (rc) return rc;
    }
    i = 0;
    while ((rc = readOneCompressRow(in, &data, i + 1)) == 0) {
        ++i;
        compressOneRead(out, &data, i, buffer);
    }
    if (GetRCState(rc) == rcNotFound) rc = 0;
    assert(rc == 0);
    
    rc = VCursorRelease(in); assert(rc == 0); if (rc) return rc;
    if (out) {
        rc = VCursorCommit(out); assert(rc == 0); if (rc) return rc;
        rc = VCursorRelease(out); assert(rc == 0); if (rc) return rc;
        VTableDropColumn(vtbl, "READ");
        VTableDropColumn(vtbl, "ALTREAD");
    }

    return 0;
}

static KMetadata *getMetadata(TableWriterSeq *const self) {
    KMetadata* md = NULL;
    VTable *vtbl = getVTable(self);
    rc_t const rc = VTableOpenMetadataUpdate(vtbl, &md);
    assert(rc == 0);
    return md;
}

static KMDataNode *openNode(KMetadata *const md, char const *const nodeName) {
    KMDataNode *node = NULL;
    rc_t const rc = KMetadataOpenNodeUpdate(md, &node, "%s", nodeName);
    assert(rc == 0);
    return node;
}

static void writeStatsMetadata(TableWriterSeq *const self, KMetadata *const md) {
    KMDataNode *const node = openNode(md, "MATE_STATISTICS");
    rc_t const rc = TableWriterSeq_WriteStatistics(self, node);
    KMDataNodeRelease(node);
    assert(rc == 0);
}

static void writeInt64Metadata(KMDataNode *const pnode, char const *const nodeName, int64_t const *const pValue)
{
    KMDataNode *sub = NULL;
    rc_t const rc1 = KMDataNodeOpenNodeUpdate(pnode, &sub, "%s", nodeName);
    rc_t const rc2 = KMDataNodeWriteB64(sub, pValue);
    KMDataNodeRelease(sub);
    assert(rc1 == 0);
    assert(rc2 == 0);
}

static void writeUnalignedMetadata(KMetadata *const md, int64_t const *const rows)
{
    KMDataNode *const node = openNode(md, "unaligned");
    writeInt64Metadata(node, "first-unaligned", &rows[0]);
    writeInt64Metadata(node, "first-half-aligned", &rows[1]);
    KMDataNodeRelease(node);
}

static void SaveMetadata(TableWriterSeq *const self, int64_t const *const rows)
{
    KMetadata *const md = getMetadata(self);
    writeStatsMetadata(self, md);
    writeUnalignedMetadata(md, rows);
    KMetadataRelease(md);
}

/* compress READ and save metadata */
static void Commit(TableWriterSeq *const self)
{
    int64_t rows[2] = {0, 0}; /* to hold first-unaligned and first-half-aligned row id */

    if (self->numSpots > 0) {
        if (self->numAlignedSpots > 0)
            CompressREAD(self, rows);
        else
            rows[0] = 1;
    }
    SaveMetadata(self, rows);
}

LIB_EXPORT rc_t CC TableWriterSeq_Whack(TableWriterSeq const *const cself, bool const commit, uint64_t *const rows)
{
    TableWriterSeq *const self = (TableWriterSeq *)cself;

    if (self != NULL) {
        TableReader_Whack(self->reader);
        if (commit) {
            TableWriter_CloseCursor(self->base, 1, NULL);
            Commit(self);
        }
        TableWriter_Whack(self->base, commit, rows);
        KVectorRelease(self->stats);
        free(self->qual_buf);
        free(self);
    }
    return 0;
}

static
rc_t TableWriteSeq_WriteDefaults(TableWriterSeq *const self)
{
    static TableWriterData const d = { "", 0 };
    rc_t rc = 0;

    self->init = 1;
    rc = TableWriter_AddCursor(self->base, self->cols, sizeof(self->cols)/sizeof(self->cols[0]), &self->cursor_id);
    if( (self->options & ewseq_co_AlignData) ) {
        TW_COL_WRITE_DEF(self->base, self->cursor_id, self->cols[ewseq_cn_PRIMARY_ALIGNMENT_ID], d);
        TW_COL_WRITE_DEF(self->base, self->cursor_id, self->cols[ewseq_cn_ALIGNMENT_COUNT], d);
    }
    TW_COL_WRITE_DEF(self->base, self->cursor_id, self->cols[ewseq_cn_LINKAGE_GROUP], d);
    return rc;
}

LIB_EXPORT rc_t CC TableWriteSeq_WriteDefault(const TableWriterSeq* cself,
                                              enum ETableWriterSeq_ColNames col, const TableWriterData* data)
{
    rc_t rc = 0;
    if( !cself->init ) {
        rc = TableWriteSeq_WriteDefaults((TableWriterSeq *)cself);
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

static rc_t TableWriterSeq_Write_validate_params(const TableWriterSeq* cself, const TableWriterSeqData* data)
{
    rc_t rc = 0;
    int lbl;
    
    /* make debug builds produce FATAL errors */
    assert(cself != NULL);
    assert(data != NULL);
    assert(data->sequence.elements == data->quality.elements);
    assert(data->read_start.elements == data->nreads);
    assert(data->read_len.elements == data->nreads);
    assert(data->read_filter.elements == data->nreads);
    assert(data->read_type.elements == data->nreads);

    assert(data->primary_alignment_id.elements == 0 || data->primary_alignment_id.elements == data->nreads);
    assert(data->alignment_count.elements == 0 || data->alignment_count.elements == data->nreads);
    assert(data->no_quantize_mask.elements == 0 || data->no_quantize_mask.elements == data->quality.elements);
    
    if( cself == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
        return rc;
    }
    if( !cself->init && (rc = TableWriteSeq_WriteDefaults((TableWriterSeq *)cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    }
    else if( data->quality.buffer == NULL || data->sequence.elements != data->quality.elements ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, data->quality.buffer ? rcInconsistent : rcEmpty);
        ALIGN_DBGERRP("sequence and quality length %lu <> %lu", rc, data->sequence.elements, data->quality.elements);
    }
    else if( data->read_start.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and read_start length %u <> %lu", rc, data->nreads, data->read_start.elements);
    }
    else if( data->read_len.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent );
        ALIGN_DBGERRP("nreads and read_len length %u <> %lu", rc, data->nreads, data->read_len.elements);
    }
    else if( (cself->options & ewseq_co_AlignData) && data->primary_alignment_id.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and primary_alignment_id length %u <> %lu", rc, data->nreads, data->primary_alignment_id.elements);
    }
    else if( (cself->options & ewseq_co_AlignData) && data->alignment_count.elements != data->nreads ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("nreads and alignment_count length %u <> %lu", rc, data->nreads, data->alignment_count.elements);
    }
    else if( data->no_quantize_mask.buffer && data->no_quantize_mask.elements != data->quality.elements ) {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("quality and no_quantize_mask length %u <> %lu", rc, data->quality.elements, data->no_quantize_mask.elements);
    }
    else if( !(cself->options & ewseq_co_NoLabelData) &&
            (lbl = ((data->label.buffer ? 1 : 0) + (data->label_start.buffer ? 1 : 0) +
                    (data->label_len.buffer ? 1 : 0))) != 0 && lbl != 3 )
    {
        rc = RC(rcAlign, rcType, rcWriting, rcData, rcInconsistent);
        ALIGN_DBGERRP("LABEL %s", rc, "incomplete");
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterSeq_Write(const TableWriterSeq* cself, const TableWriterSeqData* data, int64_t* rowid)
{
    rc_t rc = TableWriterSeq_Write_validate_params(cself, data);
    if (rc) return rc;

    assert(rowid != NULL);

    if( (rc = TableWriter_OpenRow(cself->base, rowid, cself->cursor_id)) == 0 ) {
        if( cself->options & ewseq_co_AlignData ) {
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_PRIMARY_ALIGNMENT_ID], data->primary_alignment_id);
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_ALIGNMENT_COUNT], data->alignment_count);
            if (rc == 0) {
                rc = TableWriterSeq_CollectStatistics((TableWriterSeq *)cself, &data->primary_alignment_id);
            }
        }
#if 0 /* always write full sequence */
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
#else
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_READ], data->sequence);
#endif
#if 0 /* always write full quality */
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
#else
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_QUALITY], data->quality);
#endif
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
        TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_NAME], data->spot_name);
        if (data->linkageGroup.elements > 0)
            TW_COL_WRITE(cself->base, cself->cols[ewseq_cn_LINKAGE_GROUP], data->linkageGroup);
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
        COPY_TableReaderColumns(self->cols_read, TableSeqReadTmpKey_cols);
        if( (rc = TableWriter_GetVTable(cself->base, &vtbl)) == 0 &&
            (rc = TableReader_Make(&self->reader, vtbl, self->cols_read, 50 * 1024 * 1024)) == 0 ) {
            CopyTableWriterColumns(2, self->cols_alignd, &TableWriterSeq_cols[ewseq_cn_PRIMARY_ALIGNMENT_ID]);
            rc = TableWriter_AddCursor(self->base, self->cols_alignd, 2, &self->alignd_cursor_id);
            self->init = 2;
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
    } else if( cself->reader == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcMode, rcNotOpen);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableReader_ReadRow(cself->reader, rowid)) == 0 ) {
        TableReaderColumn_GET(uint64_t, *key_id, cself->cols_read[0]);
        if (cself->tmpKeyIdLast < rowid || rowid < cself->tmpKeyIdFirst) {
            rc = TableReader_PageIdRange(cself->reader, rowid,
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
           (rc = TableWriter_OpenRowId(cself->base, rowid, cself->alignd_cursor_id)) == 0 )
        {
            {
                unsigned i;
                int64_t *numAlignedSpots = &((TableWriterSeq *)cself)->numAlignedSpots;
                int64_t *numSpots = &((TableWriterSeq *)cself)->numSpots;
                int64_t const *const pid = primary_alignment_id->buffer;
                for (i = 0; i < (unsigned)primary_alignment_id->elements; ++i) {
                    if (pid[i] != 0) {
                        *numAlignedSpots += 1;
                        break;
                    }
                }
                *numSpots += 1;
            }
            TW_COL_WRITE(cself->base, cself->cols_alignd[0], *primary_alignment_id);
            TW_COL_WRITE(cself->base, cself->cols_alignd[1], *alignment_count);
            if( rc == 0 ) {
                rc = TableWriter_CloseRow(cself->base);
            }
            if (rc == 0) {
                rc = TableWriterSeq_CollectStatistics((TableWriterSeq *)cself, primary_alignment_id);
            }
        }
    }
    return rc;
}
