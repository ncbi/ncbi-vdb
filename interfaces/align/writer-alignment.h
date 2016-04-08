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
#ifndef _h_align_writer_alignment_
#define _h_align_writer_alignment_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#include <align/writer-cmn.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ETableWriterAlgn_ColNames {
    ewalgn_cn_TMP_KEY_ID,
    ewalgn_cn_PLOIDY,
    ewalgn_cn_SEQ_SPOT_ID,
    ewalgn_cn_SEQ_READ_ID,
    ewalgn_cn_READ_START,
    ewalgn_cn_READ_LEN,
    ewalgn_cn_REF_ID,
    ewalgn_cn_REF_START,
    ewalgn_cn_GLOBAL_REF_START,
    ewalgn_cn_REF_LEN,
    ewalgn_cn_REF_ORIENTATION,
    ewalgn_cn_REF_PLOIDY,
    ewalgn_cn_MATE_REF_ORIENTATION,
    ewalgn_cn_MATE_REF_ID,
    ewalgn_cn_MATE_REF_POS,
    ewalgn_cn_MATE_ALIGN_ID,
    ewalgn_cn_TEMPLATE_LEN,
    ewalgn_cn_MAPQ,
    ewalgn_cn_HAS_MISMATCH,
    ewalgn_cn_HAS_REF_OFFSET,
    ewalgn_cn_MISMATCH,
    ewalgn_cn_REF_OFFSET,
    ewalgn_cn_REF_OFFSET_TYPE,
    ewalgn_cn_EVIDENCE_ALIGNMENT_IDS,
    ewalgn_cn_ALIGN_GROUP,
    ewalgn_cn_LINKAGE_GROUP,
    ewalgn_cn_MISMATCH_QUALITY,
    ewalgn_cn_MATE_GLOBAL_REF_START,
    ewalgn_cn_MATE_REF_START,
    ewalgn_cn_Last
};

typedef uint8_t ETableWriterAlgn_TableType;
enum {
    ewalgn_tabletype_PrimaryAlignment,
    ewalgn_tabletype_SecondaryAlignment,
    ewalgn_tabletype_EvidenceInterval,
    ewalgn_tabletype_EvidenceAlignment
};

enum ETableWriterAlgn_ColOptions {
    ewalgn_co_SEQ_SPOT_ID = 0x01,           /* SEQ_SPOT_ID will be written with the whole record */
    ewalgn_co_TMP_KEY_ID = 0x02,            /* use TMP_KEY_ID column, by default not opened */
    ewalgn_co_PLOIDY = 0x04,                /* ploidy is more than 1, columns PLOIDY, READ_START, READ_LEN needs data */
    ewalgn_co_unsorted = 0x08,              /* use the unsorted table scheme */
    ewalgn_co_MATE_ALIGN_ID_only = 0x10,    /* disable all other MATE_* columns */
    ewalgn_co_MISMATCH_QUALITY = 0x20,      /* enable MISMATCH_QUALITY column */
    ewalgn_co_MATE_POSITION = 0x40          /* enable mate position columns */
};

typedef struct TableWriterAlgnData_struct {

    TableWriterData seq_spot_id;
    TableWriterData seq_read_id;
    TableWriterData tmp_key_id;

    /* BEGIN filled out by ReferenceMgr_Compress */
    uint32_t ploidy; /* incremented sequentially for each call, must be set to 0 for each new spot */
    TableWriterData read_start; /* mandatory */
    TableWriterData read_len; /* mandatory */
    TableWriterData has_ref_offset; /* mandatory */
    TableWriterData ref_offset; /* mandatory */
    TableWriterData ref_offset_type; /* mandatory */
    TableWriterData ref_id; /* used for unsorted*/
    TableWriterData ref_start; /* used only for unsorted */
    TableWriterData global_ref_start; /* used only for sorted */
    TableWriterData has_mismatch;  /* mandatory only for primary */
    TableWriterData mismatch; /* mandatory only for primary */
    TableWriterData mismatch_qual;

    INSDC_coord_len ref_len; /* projection on refseq is same for all alleles! */
    /* tmp data, never saved to db */
    /* recalculated offset in reference based on CIGAR and circularity in Compression */
    INSDC_coord_zero effective_offset; 
    int64_t ref_1st_row_id;
    /* END filled out by ReferenceMgr_Compress */

    TableWriterData ref_orientation;
    TableWriterData ref_ploidy;
    TableWriterData mapq;
    TableWriterData align_group; /* set length to 0 if not used */
    TableWriterData linkageGroup; /* set length to 0 if not used */

    /* used only only in secondary */
    TableWriterData mate_ref_orientation;
    TableWriterData mate_ref_id;
    TableWriterData mate_ref_pos;
    TableWriterData mate_align_id;
    TableWriterData template_len;

    /* used only in for ewalgn_tabletype_EvidenceInterval table type */
    TableWriterData alingment_ids;
} TableWriterAlgnData;

typedef struct TableWriterAlgn TableWriterAlgn;
    
typedef union ReferenceStart ReferenceStart;
union ReferenceStart {
    uint64_t global_ref_start;
    struct {
        int64_t ref_id;
        INSDC_coord_one ref_start;
    } local;
};

ALIGN_EXTERN rc_t CC TableWriterAlgn_Make(const TableWriterAlgn** cself, VDatabase* db,
                                          ETableWriterAlgn_TableType type, uint32_t options);

/* rows optional here */
ALIGN_EXTERN rc_t CC TableWriterAlgn_Whack(const TableWriterAlgn* cself, bool commit, uint64_t* rows);

ALIGN_EXTERN rc_t CC TableWriterAlgn_WriteDefault(const TableWriterAlgn* cself,
                                                  enum ETableWriterAlgn_ColNames col, const TableWriterData* data);

ALIGN_EXTERN rc_t CC TableWriterAlgn_GetNextRowId(const TableWriterAlgn* cself, int64_t* rowid);

/* rowid optional here */
ALIGN_EXTERN rc_t CC TableWriterAlgn_Write(const TableWriterAlgn* cself, const TableWriterAlgnData* data, int64_t* rowid);

/* closes main cursor and creates lookup object on the successfully committed main cursor */
ALIGN_EXTERN rc_t CC TableWriterAlgn_TmpKeyStart(const TableWriterAlgn* cself);

/* retrieve TMP_KEY value by rowid */
ALIGN_EXTERN rc_t CC TableWriterAlgn_TmpKey(const TableWriterAlgn* cself, int64_t rowid, uint64_t* key_id);

/* retrieve reference start value by rowid */
ALIGN_EXTERN rc_t CC TableWriterAlgn_RefStart(const TableWriterAlgn* cself, int64_t rowid, ReferenceStart *const rslt);

/* assign a SPOT_ID value to row */
ALIGN_EXTERN rc_t CC TableWriterAlgn_Write_SpotId(const TableWriterAlgn* cself, int64_t rowid, int64_t spot_id);

ALIGN_EXTERN rc_t CC TableWriterAlgn_Write_SpotInfo(const TableWriterAlgn* cself,
                                                    int64_t rowid,
                                                    int64_t spot_id,
                                                    int64_t mate_id,
                                                    ReferenceStart const *ref_start);

#ifdef __cplusplus
}
#endif

#endif /* _h_align_writer_alignment_ */
