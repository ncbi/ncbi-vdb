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
#ifndef _h_align_writer_ref_
#define _h_align_writer_ref_

#include <vdb/database.h>
#include <align/writer-alignment.h>
#include <align/writer-reference.h>

enum ETableWriterRefData_ColNames {
    ewrefd_cn_MAX_SEQ_LEN, /* static/default value from refseq writer */
    ewrefd_cn_NAME,
    ewrefd_cn_CS_KEY, /* handled by writer */
    ewrefd_cn_READ,
    ewrefd_cn_QUALITY,
    ewrefd_cn_SEQ_ID,
    ewrefd_cn_SEQ_START, /* calculated on fly by writer */
    ewrefd_cn_SEQ_LEN, /* calculated on fly by writer */
    ewrefd_cn_CIRCULAR, 
    ewrefd_cn_Last = ewrefd_cn_CIRCULAR
};

enum ETableWriterRefCoverage_ColNames {
    /* coverage data starts here */
    ewrefcv_cn_CGRAPH_HIGH=0,
    ewrefcv_cn_CGRAPH_LOW,
    ewrefcv_cn_CGRAPH_MISMATCHES,
    ewrefcv_cn_CGRAPH_INDELS,
    ewrefcv_cn_OVERLAP_REF_POS,
    ewrefcv_cn_OVERLAP_REF_LEN,
    ewrefcv_cn_ReCover = ewrefcv_cn_OVERLAP_REF_LEN,
    ewrefcv_cn_PRIMARY_ALIGNMENT_IDS,
    ewrefcv_cn_SECONDARY_ALIGNMENT_IDS,
    ewrefcv_cn_EVIDENCE_INTERVAL_IDS,
    ewrefcv_cn_Last = ewrefcv_cn_EVIDENCE_INTERVAL_IDS
};

enum ETableWriterRef_ColOptions {
    ewref_co_SaveRead = 0x01, /* always write READ */
    ewref_co_QUALITY = 0x02,  /* use QUALITY column, by default not opened */
    ewref_co_Coverage = 0x04  /* use coverage columns, by default not opened */
};

typedef struct TableWriterRefData_struct {
    TableWriterData name;
    bool force_READ_write;
    TableWriterData read;
    TableWriterData quality;
    TableWriterData seq_id;
    bool circular;
} TableWriterRefData;

typedef struct TableWriterRef TableWriterRef;

rc_t CC TableWriterRef_Make(const TableWriterRef** cself, VDatabase* db, const uint32_t options);

/* rows optional here */
rc_t CC TableWriterRef_Whack(const TableWriterRef* cself, bool commit, uint64_t* rows);

rc_t CC TableWriterRef_WriteDefaultData(const TableWriterRef* cself, enum ETableWriterRefData_ColNames col,
                                        const TableWriterData* data);

rc_t CC TableWriterRef_WriteDefaultCovarage(const TableWriterRef* cself, enum ETableWriterRefCoverage_ColNames col,
                                            const TableWriterData* data);

/* rowid optional here */
rc_t CC TableWriterRef_Write(const TableWriterRef* cself, const TableWriterRefData* data, int64_t* rowid);

rc_t CC TableWriterRef_WriteCoverage(const TableWriterRef* cself, int64_t rowid, INSDC_coord_zero offset, const ReferenceSeqCoverage* coverage);




typedef struct TableWriterRefCoverage TableWriterRefCoverage;

rc_t CC TableWriterRefCoverage_MakeCoverage(const TableWriterRefCoverage** cself, VDatabase* db, const uint32_t options);
rc_t CC TableWriterRefCoverage_MakeIds(const TableWriterRefCoverage** cself, VDatabase* db, const char *col_name);
/* rows optional here */
rc_t CC TableWriterRefCoverage_Whack(const TableWriterRefCoverage* cself, bool commit, uint64_t* rows);
rc_t CC TableWriterRefCoverage_WriteCoverage(const TableWriterRefCoverage* cself, int64_t rowid, const ReferenceSeqCoverage* coverage);
rc_t CC TableWriterRefCoverage_WriteIds(const TableWriterRefCoverage* cself, int64_t rowid, const int64_t* buf,uint32_t num);

#endif /* _h_align_writer_ref_ */
