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
#ifndef _h_align_writer_sequence_
#define _h_align_writer_sequence_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#include <align/writer-cmn.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ETableWriterSeq_ColNames {
    ewseq_cn_READ,
    ewseq_cn_CSREAD,
    ewseq_cn_CSKEY,
    ewseq_cn_QUALITY,
    ewseq_cn_PRIMARY_ALIGNMENT_ID,
    ewseq_cn_ALIGNMENT_COUNT,
    ewseq_cn_PLATFORM,
    ewseq_cn_LABEL,
    ewseq_cn_LABEL_START,
    ewseq_cn_LABEL_LEN,
    ewseq_cn_READ_TYPE,
    ewseq_cn_READ_START,
    ewseq_cn_READ_LEN,
    ewseq_cn_TMP_KEY_ID,
    ewseq_cn_SPOT_GROUP,
    ewseq_cn_READ_FILTER,
    ewseq_cn_TI,
    ewseq_cn_NAME,
    ewseq_cn_LINKAGE_GROUP,
    ewseq_cn_Last = ewseq_cn_LINKAGE_GROUP
};

enum ETableWriterSeq_ColOptions {
    ewseq_co_SaveRead = 0x01,       /* READ is always written; this options causes CMP_READ to not be written */
    ewseq_co_FullQuality = 0x02,    /* this option is ignored; full quality values will always be written */
    ewseq_co_AlignData = 0x04,      /* PRIMARY_ALIGNMENT_ID, ALIGNMENT_COUNT will be written with the whole record */
    ewseq_co_NoLabelData = 0x08,    /* LABEL and related columns will not be written */
    ewseq_co_ColorSpace = 0x10,     /* CMP_CSREAD will be written instead of CMP_READ */
    ewseq_co_SpotGroup = 0x20,      /* SPOT_GROUP will be written */
    ewseq_co_TI = 0x40,             /* TI will be written */
    ewseq_co_SaveQual_DEAD = 0x80,  /* DEAD OPTION; quality will always be written */
    ewseq_co_SpotName = 0x100,      /* NAME will be written */
    ewseq_co_KeepKey = 0x200,       /* TMP_KEY_ID won't be removed */
};

typedef struct TableWriterSeqData_struct {
    uint64_t        spot_len;
    TableWriterData sequence; /* always writes sequence; compression is later */
    TableWriterData quality;

    TableWriterData primary_alignment_id;
    TableWriterData alignment_count;
    TableWriterData label;
    TableWriterData label_start;
    TableWriterData label_len;
    uint8_t nreads;
    TableWriterData read_type;
    TableWriterData read_start;
    TableWriterData read_len;
    uint64_t tmp_key_id;
    TableWriterData spot_group;
    TableWriterData cskey;
    TableWriterData read_filter;
    TableWriterData no_quantize_mask;
    TableWriterData platform;
    TableWriterData ti;
    TableWriterData spot_name;
    TableWriterData linkageGroup;
} TableWriterSeqData;

typedef struct TableWriterSeq TableWriterSeq;

/*
 * THIS FEATURE IS DISABLED
 * quality_quantization:
 *  is a ',' seperated list of value pairs, with the pairs seperated by ':'.
 *  The first of the pair is the value to substitute.
 *  The second is the limit at which the substitution is no longer valid.
 *  The second value can be '-' to indication the remainder; this also terminates
 *  processing of the string.  If the second value is not '-', then the trailing
 *  ',' is required.
 *
 *  Example: 1:10,10:20,20:30,30:40,
 *  This will substitute 1 for quality values [0, 10), 10 for [10, 20),
 *  20 for [20, 30), 30 for [30, 40), and 0 for the rest
 *
 *  Example: 1:10,10:20,20:30,30:-
 *  This will substitute 1 for quality values [0, 10), 10 for [10, 20),
 *  20 for [20, 30), and 30 for the rest
 *
 *  Example: 1:30,30:-
 *  This will substitute 1 for quality values [0, 30), and 30 for the rest
 *
 *  Example: 10:30,20:-
 *  This will substitute 10 for quality values [0, 30), 20 for the rest
 */
ALIGN_EXTERN rc_t CC TableWriterSeq_Make(const TableWriterSeq** cself, VDatabase* db,
                                         const uint32_t options, char const quality_quantization[]);

/* rows optional here */
ALIGN_EXTERN rc_t CC TableWriterSeq_Whack(const TableWriterSeq* cself, bool commit, uint64_t* rows);

ALIGN_EXTERN rc_t CC TableWriteSeq_WriteDefault(const TableWriterSeq* cself,
                                                enum ETableWriterSeq_ColNames col, const TableWriterData* data);

ALIGN_EXTERN rc_t CC TableWriterSeq_GetNextRowId(const TableWriterSeq* cself, int64_t* rowid);

/* rowid optional here */
ALIGN_EXTERN rc_t CC TableWriterSeq_Write(const TableWriterSeq* cself, const TableWriterSeqData* data, int64_t* rowid);


/* if option ewseq_co_AlignData is not set (default) below methods are available */

/* closes main cursor and creates lookup object on the successfully committed main cursor */
ALIGN_EXTERN rc_t CC TableWriterSeq_TmpKeyStart(const TableWriterSeq* cself);

/* retrieve TMP_KEY value by rowid */
ALIGN_EXTERN rc_t CC TableWriterSeq_TmpKey(const TableWriterSeq* cself, int64_t rowid, uint64_t *key_id);

/* passing rowid == 0 will cause the cursor to be flushed */
ALIGN_EXTERN rc_t CC TableWriterSeq_WriteAlignmentData(const TableWriterSeq* cself, int64_t rowid,
                                                       const TableWriterData* primary_alignment_id,
                                                       const TableWriterData* alignment_count);

#ifdef __cplusplus
}
#endif

#endif /* _h_align_writer_sequence_ */
