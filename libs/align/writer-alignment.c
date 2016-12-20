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
#include <vdb/database.h>
#include <vdb/cursor.h>
#include <align/writer-alignment.h>
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

static const TableWriterColumn TableWriterAlgn_cols[ewalgn_cn_Last] =
{
    /* order is important, see enum in .h !!! */
    {0, "TMP_KEY_ID", sizeof(uint64_t) * 8, ewcol_Temporary | ewcol_Ignore},
    {0, "PLOIDY", sizeof(uint32_t) * 8, ewcol_Ignore},
    {0, "SEQ_SPOT_ID", sizeof(int64_t) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "SEQ_READ_ID", sizeof(INSDC_coord_one) * 8, ewcol_IsArray },
    {0, "READ_START", sizeof(INSDC_coord_zero) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "READ_LEN", sizeof(INSDC_coord_len) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "REF_ID", sizeof(int64_t) * 8, ewcol_Ignore},
    {0, "REF_START", sizeof(INSDC_coord_zero) * 8, ewcol_Ignore},
    {0, "GLOBAL_REF_START", sizeof(uint64_t) * 8, 0},
    {0, "REF_LEN", sizeof(INSDC_coord_len) * 8, 0},
    {0, "REF_ORIENTATION", sizeof(bool) * 8, 0},
    {0, "REF_PLOIDY", sizeof(uint32_t) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "MATE_REF_ORIENTATION", sizeof(bool) * 8, ewcol_Ignore},
    {0, "MATE_REF_ID", sizeof(int64_t) * 8, ewcol_Ignore},
    {0, "MATE_REF_POS", sizeof(INSDC_coord_zero) * 8, ewcol_Ignore},
    {0, "MATE_ALIGN_ID", sizeof(int64_t) * 8, ewcol_Ignore},
    {0, "TEMPLATE_LEN", sizeof(INSDC_coord_zero) * 8, ewcol_Ignore},
    {0, "MAPQ", sizeof(int32_t) * 8, 0},
    {0, "HAS_MISMATCH", sizeof(bool) * 8, ewcol_IsArray},
    {0, "(bool)HAS_REF_OFFSET", sizeof(bool) * 8, ewcol_IsArray},
    {0, "MISMATCH", sizeof(INSDC_dna_text) * 8, ewcol_IsArray},
    {0, "REF_OFFSET", sizeof(int32_t) * 8, ewcol_IsArray},
    {0, "REF_OFFSET_TYPE", sizeof(uint8_t) * 8, ewcol_IsArray},
    {0, "EVIDENCE_ALIGNMENT_IDS", sizeof(int64_t) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "ALIGN_GROUP", sizeof(char) * 8, ewcol_IsArray | ewcol_Ignore },
    {0, "LINKAGE_GROUP", sizeof(char) * 8, ewcol_IsArray | ewcol_Ignore },
    {0, "MISMATCH_QUALITY", sizeof(uint8_t) * 8, ewcol_IsArray | ewcol_Ignore},
    {0, "MATE_GLOBAL_REF_START", sizeof(uint64_t) * 8, ewcol_Ignore},
    {0, "MATE_REF_START", sizeof(INSDC_coord_zero) * 8, ewcol_Ignore}
};

static const TableReaderColumn TableAlgnReadTmpKey_cols[] = {
    {0, "TMP_KEY_ID", {NULL}, 0, 0},
    {0, "GLOBAL_REF_START", {NULL}, 0, ercol_Skip},
    {0, "REF_ID", {NULL}, 0, ercol_Skip},
    {0, "REF_START", {NULL}, 0, ercol_Skip},
    {0, NULL, {NULL}, 0, 0}
};

struct TableWriterAlgn {
    uint32_t options;
    const TableWriter* base;
    const char* ref_table_name;
    uint8_t cursor_id;
    TableWriterColumn cols[ewalgn_cn_Last];
    uint8_t spotid_cursor_id;
    TableWriterColumn cols_spotid[5];
    const TableReader* tmpkey_reader;
    TableReaderColumn cols_read_tmpkey[sizeof(TableAlgnReadTmpKey_cols) / sizeof(TableAlgnReadTmpKey_cols[0])];
};

LIB_EXPORT rc_t CC TableWriterAlgn_Make(const TableWriterAlgn** cself, VDatabase* db,
                                        ETableWriterAlgn_TableType type, uint32_t options)
{
    rc_t rc = 0;
    TableWriterAlgn* self = NULL;
    const char* tbl_nm = __func__;

    if( cself == NULL || db == NULL ) {
        rc = RC(rcAlign, rcFormatter, rcConstructing, rcParam, rcNull);
    } else {
        self = calloc(1, sizeof(*self));
        if( self == NULL ) {
            rc = RC(rcAlign, rcFormatter, rcConstructing, rcMemory, rcExhausted);
        } else {
            self->ref_table_name = "REFERENCE";
            memmove(self->cols, TableWriterAlgn_cols, sizeof(TableWriterAlgn_cols));
            switch(type) {
            case ewalgn_tabletype_PrimaryAlignment:
                tbl_nm = "PRIMARY_ALIGNMENT";
                self->cols[ewalgn_cn_ALIGN_GROUP].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_LINKAGE_GROUP].flags &= ~ewcol_Ignore;
                if (options & ewalgn_co_MISMATCH_QUALITY)
                    self->cols[ewalgn_cn_MISMATCH_QUALITY].flags &= ~ewcol_Ignore;
                break;
            case ewalgn_tabletype_SecondaryAlignment:
                tbl_nm = "SECONDARY_ALIGNMENT";
#if 0
                self->cols[ewalgn_cn_HAS_MISMATCH].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_MISMATCH].flags |= ewcol_Ignore;
#else
                self->cols[ewalgn_cn_MISMATCH].name = "TMP_MISMATCH";
                self->cols[ewalgn_cn_HAS_MISMATCH].name = "TMP_HAS_MISMATCH";
#endif
                self->cols[ewalgn_cn_MATE_REF_ORIENTATION].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_MATE_REF_ID].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_MATE_REF_POS].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_MATE_ALIGN_ID].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_TEMPLATE_LEN].flags &= ~ewcol_Ignore;
                /* self->cols[ewalgn_cn_ALIGN_GROUP].flags &= ~ewcol_Ignore; why? */
                self->cols[ewalgn_cn_LINKAGE_GROUP].flags &= ~ewcol_Ignore;
                break;
            case ewalgn_tabletype_EvidenceInterval:
                tbl_nm = "EVIDENCE_INTERVAL";
                options |= ewalgn_co_unsorted;
                options |= ewalgn_co_PLOIDY;
                self->cols[ewalgn_cn_SEQ_SPOT_ID].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_SEQ_READ_ID].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_EVIDENCE_ALIGNMENT_IDS].flags &= ~ewcol_Ignore;
                break;
            case ewalgn_tabletype_EvidenceAlignment:
                tbl_nm = "EVIDENCE_ALIGNMENT";
                self->ref_table_name = "EVIDENCE_INTERVAL";
                self->cols[ewalgn_cn_REF_PLOIDY].flags &= ~ewcol_Ignore;
#if 0
                self->cols[ewalgn_cn_HAS_MISMATCH].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_MISMATCH].flags |= ewcol_Ignore;
#else
                self->cols[ewalgn_cn_MISMATCH].name = "TMP_MISMATCH";
                self->cols[ewalgn_cn_HAS_MISMATCH].name = "TMP_HAS_MISMATCH";
#endif
                options |= ewalgn_co_unsorted;
                break;
            default:
                rc = RC(rcAlign, rcFormatter, rcConstructing, rcType, rcUnrecognized);
            }
            if( options & ewalgn_co_SEQ_SPOT_ID ) {
                self->cols[ewalgn_cn_SEQ_SPOT_ID].flags &= ~ewcol_Ignore;
            }
            if( options & ewalgn_co_TMP_KEY_ID ) {
                self->cols[ewalgn_cn_TMP_KEY_ID].flags &= ~ewcol_Ignore;
            }
            if( options & ewalgn_co_PLOIDY ) {
                self->cols[ewalgn_cn_PLOIDY].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_READ_START].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_READ_LEN].flags &= ~ewcol_Ignore;
            }
            if( options & ewalgn_co_unsorted ) {
                self->cols[ewalgn_cn_REF_ID].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_REF_START].flags &= ~ewcol_Ignore;
                self->cols[ewalgn_cn_GLOBAL_REF_START].flags |= ewcol_Ignore;
                if( type == ewalgn_tabletype_SecondaryAlignment ) {
                    self->cols[ewalgn_cn_MISMATCH].name = "TMP_MISMATCH";
                    self->cols[ewalgn_cn_MISMATCH].flags &= ~ewcol_Ignore;
                    self->cols[ewalgn_cn_HAS_MISMATCH].name = "TMP_HAS_MISMATCH";
                    self->cols[ewalgn_cn_HAS_MISMATCH].flags &= ~ewcol_Ignore;
                }
            }
            if( options & ewalgn_co_MATE_ALIGN_ID_only ) {
                self->cols[ewalgn_cn_MATE_REF_ORIENTATION].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_MATE_REF_ID].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_MATE_REF_POS].flags |= ewcol_Ignore;
                self->cols[ewalgn_cn_TEMPLATE_LEN].flags |= ewcol_Ignore;
            }
            if( (rc = TableWriter_Make(&self->base, db, tbl_nm, NULL)) == 0 ) {
                static TableWriterData const d = { "", 0 };

                rc = TableWriter_AddCursor(self->base, self->cols, sizeof(self->cols)/sizeof(self->cols[0]), &self->cursor_id);
                TW_COL_WRITE_DEF(self->base, self->cursor_id, self->cols[ewalgn_cn_LINKAGE_GROUP], d);
                self->options = options;
            }
        }
    }
    if( rc == 0 ) {
        *cself = self;
        ALIGN_DBG("table %s created", tbl_nm);
    } else {
        TableWriterAlgn_Whack(self, false, NULL);
        ALIGN_DBGERR(rc);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_Whack(const TableWriterAlgn* cself, bool commit, uint64_t* rows)
{
    rc_t rc = 0;
    if( cself != NULL ) {
        VTable* vtbl;

        TableReader_Whack(cself->tmpkey_reader);
        if( (rc = TableWriter_GetVTable(cself->base, &vtbl)) == 0 ) {
            KMetadata* meta;
            if( (rc = VTableOpenMetadataUpdate(vtbl, &meta)) == 0 ) {
                KMDataNode* node;
                if( (rc = KMetadataOpenNodeUpdate(meta, &node, "CONFIG/REF_TABLE")) == 0 ) {
                    rc = KMDataNodeWriteCString(node, cself->ref_table_name);
                    KMDataNodeRelease(node);
                }
                KMetadataRelease(meta);
            }
        }
        rc = TableWriter_Whack(cself->base, commit, rows);
        free((TableWriterAlgn*)cself);
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_WriteDefault(const TableWriterAlgn* cself, enum ETableWriterAlgn_ColNames col, const TableWriterData* data)
{
    return TableWriter_ColumnDefault(cself->base, 0, &cself->cols[col], data);
}

LIB_EXPORT rc_t CC TableWriterAlgn_GetNextRowId(const TableWriterAlgn* cself, int64_t* rowid)
{
    return TableWriter_GetNextRowId(cself->base, rowid, cself->cursor_id);
}

LIB_EXPORT rc_t CC TableWriterAlgn_Write(const TableWriterAlgn* cself, const TableWriterAlgnData* data, int64_t* rowid)
{
    rc_t rc = 0;

    if( cself == NULL || data == NULL ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableWriter_OpenRow(cself->base, rowid, cself->cursor_id)) == 0 ) {
        TW_COL_WRITE_VAR(cself->base, cself->cols[ewalgn_cn_PLOIDY], data->ploidy);

        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_SEQ_SPOT_ID], data->seq_spot_id);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_SEQ_READ_ID], data->seq_read_id);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_READ_START], data->read_start);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_READ_LEN], data->read_len);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_TMP_KEY_ID], data->tmp_key_id);

        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_HAS_REF_OFFSET], data->has_ref_offset);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_REF_OFFSET], data->ref_offset);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_REF_OFFSET_TYPE], data->ref_offset_type);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_REF_ID], data->ref_id);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_REF_START], data->ref_start);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_GLOBAL_REF_START], data->global_ref_start);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_HAS_MISMATCH], data->has_mismatch);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_MISMATCH], data->mismatch);
        TW_COL_WRITE_VAR(cself->base, cself->cols[ewalgn_cn_REF_LEN], data->ref_len);

        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_REF_ORIENTATION], data->ref_orientation);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_REF_PLOIDY], data->ref_ploidy);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_MAPQ], data->mapq);

        if( data->mate_align_id.elements != 1 ) {
            TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_MATE_ALIGN_ID], data->mate_align_id);
        } else {
            TW_COL_WRITE_BUF(cself->base, cself->cols[ewalgn_cn_MATE_ALIGN_ID], data->mate_align_id.buffer,
                             ((const int64_t*)(data->mate_align_id.buffer))[0] ? 1 : 0);
        }
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_MATE_REF_ORIENTATION], data->mate_ref_orientation);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_MATE_REF_ID], data->mate_ref_id);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_MATE_REF_POS], data->mate_ref_pos);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_TEMPLATE_LEN], data->template_len);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_EVIDENCE_ALIGNMENT_IDS], data->alingment_ids);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_ALIGN_GROUP], data->align_group);
        TW_COL_WRITE(cself->base, cself->cols[ewalgn_cn_LINKAGE_GROUP], data->linkageGroup);

        if( rc == 0 ) {
            rc = TableWriter_CloseRow(cself->base);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_TmpKeyStart(const TableWriterAlgn* cself)
{
    rc_t rc = 0;

    if( cself == NULL ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( (cself->options & ewalgn_co_SEQ_SPOT_ID) || !(cself->options & ewalgn_co_TMP_KEY_ID) ) {
        rc = RC(rcAlign, rcType, rcWriting, rcParam, rcViolated);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableWriter_CloseCursor(cself->base, cself->cursor_id, NULL)) == 0 ) {
        TableWriterAlgn* self = (TableWriterAlgn*)cself;
        VTable* vtbl = NULL;
        
        memmove(&self->cols_read_tmpkey, &TableAlgnReadTmpKey_cols, sizeof(TableAlgnReadTmpKey_cols));
        if (self->cols[ewalgn_cn_GLOBAL_REF_START].flags & ewcol_Ignore) {
            self->cols_read_tmpkey[2].flags = 0;
            self->cols_read_tmpkey[3].flags = 0;
        }
        else
            self->cols_read_tmpkey[1].flags = 0;

        if( (rc = TableWriter_GetVTable(self->base, &vtbl)) == 0 &&
            (rc = TableReader_Make(&self->tmpkey_reader, vtbl, self->cols_read_tmpkey, 50 * 1024 * 1024)) == 0 ) {
            int64_t v = 0;
            
            memmove(self->cols_spotid + 0, &TableWriterAlgn_cols[ewalgn_cn_SEQ_SPOT_ID], sizeof(self->cols_spotid[0]));
            memmove(self->cols_spotid + 1, &TableWriterAlgn_cols[ewalgn_cn_MATE_GLOBAL_REF_START], sizeof(self->cols_spotid[0]));
            memmove(self->cols_spotid + 2, &TableWriterAlgn_cols[ewalgn_cn_MATE_REF_ID], sizeof(self->cols_spotid[0]));
            memmove(self->cols_spotid + 3, &TableWriterAlgn_cols[ewalgn_cn_MATE_REF_START], sizeof(self->cols_spotid[0]));
            memmove(self->cols_spotid + 4, &TableWriterAlgn_cols[ewalgn_cn_MATE_ALIGN_ID], sizeof(self->cols_spotid[0]));
            
            self->cols_spotid[0].flags &= ~ewcol_Ignore;
            if (self->options & ewalgn_co_MATE_POSITION) {
                if (self->cols[ewalgn_cn_GLOBAL_REF_START].flags & ewcol_Ignore) {
                    self->cols_spotid[2].flags &= ~ewcol_Ignore;
                    self->cols_spotid[3].flags &= ~ewcol_Ignore;
                }
                else
                    self->cols_spotid[1].flags &= ~ewcol_Ignore;
                self->cols_spotid[4].flags &= ~ewcol_Ignore;
            }
            else {
                self->cols_spotid[1].flags |= ewcol_Ignore;
                self->cols_spotid[2].flags |= ewcol_Ignore;
                self->cols_spotid[3].flags |= ewcol_Ignore;
                self->cols_spotid[4].flags |= ewcol_Ignore;
            }
            rc = TableWriter_AddCursor(self->base, self->cols_spotid,
                                       sizeof(self->cols_spotid) / sizeof(self->cols_spotid[0]),
                                       &self->spotid_cursor_id);

            TW_COL_WRITE_DEF_VAR(self->base, self->spotid_cursor_id, self->cols_spotid[0], v);
            TW_COL_WRITE_DEF_VAR(self->base, self->spotid_cursor_id, self->cols_spotid[1], v);
            TW_COL_WRITE_DEF_VAR(self->base, self->spotid_cursor_id, self->cols_spotid[2], v);
            TW_COL_WRITE_DEF_VAR(self->base, self->spotid_cursor_id, self->cols_spotid[3], v);
            TW_COL_WRITE_DEF_VAR(self->base, self->spotid_cursor_id, self->cols_spotid[4], v);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_TmpKey(const TableWriterAlgn* cself, int64_t rowid, uint64_t* key_id)
{
    rc_t rc = 0;

    if( cself == NULL || rowid == 0 || key_id == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->tmpkey_reader == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcMode, rcNotOpen);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableReader_ReadRow(cself->tmpkey_reader, rowid)) == 0 ) {
        memmove(key_id, cself->cols_read_tmpkey[0].base.var, sizeof(*key_id));
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_RefStart(const TableWriterAlgn* cself, int64_t rowid, ReferenceStart *const rslt)
{
    rc_t rc = 0;
    
    if( cself == NULL || rowid == 0 || rslt == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->tmpkey_reader == NULL ) {
        rc = RC( rcAlign, rcType, rcReading, rcMode, rcNotOpen);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableReader_ReadRow(cself->tmpkey_reader, rowid)) == 0 ) {
        if (cself->cols_read_tmpkey[1].flags & ewcol_Ignore) {
            memmove(&rslt->local.ref_id, cself->cols_read_tmpkey[2].base.var, sizeof(rslt->local.ref_id));
            memmove(&rslt->local.ref_start, cself->cols_read_tmpkey[3].base.var, sizeof(rslt->local.ref_start));
        }
        else
            memmove(&rslt->global_ref_start, cself->cols_read_tmpkey[1].base.var, sizeof(rslt->global_ref_start));
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_Write_SpotId(const TableWriterAlgn* cself, int64_t rowid, int64_t spot_id)
{
    rc_t rc = 0;

    if( cself == NULL || rowid == 0 ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->options & ewalgn_co_SEQ_SPOT_ID ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcViolated);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableWriter_OpenRowId(cself->base, rowid, cself->spotid_cursor_id)) == 0 ) {
        TW_COL_WRITE_VAR(cself->base, cself->cols_spotid[0], spot_id);
        if( rc == 0 ) {
            rc = TableWriter_CloseRow(cself->base);
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC TableWriterAlgn_Write_SpotInfo(const TableWriterAlgn* cself,
                                                  int64_t rowid,
                                                  int64_t spot_id,
                                                  int64_t mate_id,
                                                  ReferenceStart const *ref_start)
{
    rc_t rc = 0;
    
    if( cself == NULL || rowid == 0 ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcNull);
        ALIGN_DBGERR(rc);
    } else if( cself->options & ewalgn_co_SEQ_SPOT_ID ) {
        rc = RC( rcAlign, rcType, rcWriting, rcParam, rcViolated);
        ALIGN_DBGERR(rc);
    } else if( (rc = TableWriter_OpenRowId(cself->base, rowid, cself->spotid_cursor_id)) == 0 ) {
        TW_COL_WRITE_VAR(cself->base, cself->cols_spotid[0], spot_id);
        TW_COL_WRITE_VAR(cself->base, cself->cols_spotid[1], ref_start->global_ref_start);
        TW_COL_WRITE_VAR(cself->base, cself->cols_spotid[2], ref_start->local.ref_id);
        TW_COL_WRITE_VAR(cself->base, cself->cols_spotid[3], ref_start->local.ref_start);
        TW_COL_WRITE_VAR(cself->base, cself->cols_spotid[4], mate_id);
        
        if( rc == 0 ) {
            rc = TableWriter_CloseRow(cself->base);
        }
    }
    return rc;
}
