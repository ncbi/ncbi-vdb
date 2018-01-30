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
#include <insdc/insdc.h>
#include <vdb/manager.h>
#include <align/writer-refseq.h>
#include "writer-priv.h"
#include "debug.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

static const TableWriterColumn TableWriterRefSeq_cols[ewrefseq_cn_Last + 1] =
{
    /* order is important, see enum in .h !!! */
    {0, "MAX_SEQ_LEN", sizeof(uint32_t) * 8, 0},
    {0, "DEF_LINE", sizeof(char) * 8, ewcol_IsArray},
    {0, "(INSDC:dna:text)CS_KEY", sizeof(char) * 8, 0},
    {0, "(INSDC:dna:text)READ", sizeof(char) * 8, ewcol_IsArray},
    {0, "QUALITY", sizeof(INSDC_quality_phred) * 8, ewcol_IsArray},
    {0, "SEQ_ID", sizeof(char) * 8, ewcol_IsArray},
    {0, "SEQ_START", sizeof(INSDC_coord_one) * 8, 0},
    {0, "SEQ_LEN", sizeof(INSDC_coord_len) * 8, 0},
    {0, "CIRCULAR", sizeof(bool) * 8, 0}
};

struct TableWriterRefSeq {
    const TableWriter* base;
    uint8_t cursorid;
    TableWriterColumn cols[sizeof(TableWriterRefSeq_cols)/sizeof(TableWriterRefSeq_cols[0])];
    INSDC_coord_one seq_start_last;
    uint32_t max_seq_len;
    INSDC_dna_text last_cs_key;
};

LIB_EXPORT rc_t CC TableWriterRefSeq_Make(const TableWriterRefSeq** cself, VDBManager* mgr, const char* schema_path,
                                          const char* table_path, const uint32_t options)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        TableWriterRefSeq* self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            memmove(self->cols, TableWriterRefSeq_cols, sizeof(TableWriterRefSeq_cols));
            if( !(options & ewrefseq_co_QUALITY) ) {
                self->cols[ewrefseq_cn_QUALITY].flags |= ewcol_Ignore;
            }
            if( (rc = TableWriter_MakeMgr(&self->base, mgr, schema_path, "NCBI:refseq:tbl:reference", table_path)) == 0 ) {
                rc = TableWriter_AddCursor(self->base, self->cols, sizeof(self->cols)/sizeof(self->cols[0]), &self->cursorid);
            }
        }
        if( rc == 0 ) {
            *cself = self;
            ALIGN_DBG("table %s created", table_path);
        } else {
            TableWriterRefSeq_Whack(self, false, NULL, 0, 0, NULL, NULL, 0);
            ALIGN_DBGERRP("table %s", rc, table_path);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterRefSeq_Whack(const TableWriterRefSeq* cself,
                                           bool commit, uint64_t* rows,
                                           const char loader_name[],
                                           const ver_t loader_version,
                                           const char loader_date[],
                                           const char app_name[],
                                           const ver_t app_version)
{
    rc_t rc = 0;

    if( cself != NULL ) {
        rc_t rc1;
        if( commit ) {
            rc = TableWriter_Sign(cself->base, loader_name, loader_version, loader_date, app_name, app_version);
        }
        rc1 = TableWriter_Whack(cself->base, rc ? false : commit, rows);
        rc = rc ? rc : rc1;
        free((TableWriterRefSeq*)cself);
    }
    return rc;
}

static
rc_t TableWriterRefSeq_WriteDefaults(const TableWriterRefSeq* cself)
{
    rc_t rc = 0;
    if( cself != NULL ) {
        TableWriterRefSeq* self = (TableWriterRefSeq*)cself;
        self->max_seq_len = TableWriterRefSeq_MAX_SEQ_LEN;
        self->seq_start_last = 1;
        self->last_cs_key = 'T';
        TW_COL_WRITE_DEF_VAR(cself->base, cself->cursorid, cself->cols[ewrefseq_cn_MAX_SEQ_LEN], cself->max_seq_len);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterRefSeq_WriteDefault(const TableWriterRefSeq* cself,
                                                  enum ETableWriterRefSeq_ColNames col, const TableWriterData* data)
{
    rc_t rc = 0;
    if( cself == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->last_cs_key == '\0' && (rc = TableWriterRefSeq_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else {
        TW_COL_WRITE_DEF(cself->base, cself->cursorid, cself->cols[col], (*data));
        if( col == ewrefseq_cn_MAX_SEQ_LEN ) {
            ((TableWriterRefSeq*)cself)->max_seq_len = *((uint32_t*)data->buffer);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterRefSeq_Write(const TableWriterRefSeq* cself, const TableWriterRefSeqData* data, int64_t* rowid)
{
    rc_t rc = 0;

    if( cself == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->last_cs_key == '\0' && (rc = TableWriterRefSeq_WriteDefaults(cself)) != 0 ) {
        ALIGN_DBGERR(rc);
    } else if( data->read.elements > cself->max_seq_len ) {
        rc = RC( rcAlign, rcType, rcWriting, rcData, rcInvalid);
        ALIGN_DBGERRP("%s is longer than %s", rc, cself->cols[ewrefseq_cn_READ].name, cself->cols[ewrefseq_cn_MAX_SEQ_LEN].name);
    } else if( (rc = TableWriter_OpenRow(cself->base, rowid, cself->cursorid)) == 0 ) {
        INSDC_coord_len len = data->read.elements;
        TW_COL_WRITE(cself->base, cself->cols[ewrefseq_cn_READ], data->read);
        TW_COL_WRITE(cself->base, cself->cols[ewrefseq_cn_QUALITY], data->quality);
        TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefseq_cn_SEQ_START], cself->seq_start_last);
        TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefseq_cn_SEQ_LEN], len);
        TW_COL_WRITE_VAR(cself->base, cself->cols[ewrefseq_cn_CS_KEY], cself->last_cs_key);
        if( rc == 0 && (rc = TableWriter_CloseRow(cself->base)) == 0 ) {
            TableWriterRefSeq* self = (TableWriterRefSeq*)cself;
            
            self->last_cs_key = ((const INSDC_dna_text*)(data->read.buffer))[data->read.elements - 1];
            self->seq_start_last += len;
        }
    }
    return rc;
}
