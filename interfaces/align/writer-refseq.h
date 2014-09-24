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
#ifndef _h_align_writer_refseq_
#define _h_align_writer_refseq_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#include <vdb/manager.h>
#include <align/writer-cmn.h>

#ifdef __cplusplus
extern "C" {
#endif

/* default sequence chunk length */
#define TableWriterRefSeq_MAX_SEQ_LEN 5000

/* use in TableWriterRefSeq_Write* methods */
enum ETableWriterRefSeq_ColNames {
    ewrefseq_cn_MAX_SEQ_LEN, /* static/default */
    ewrefseq_cn_DEF_LINE, /* static/default */
    ewrefseq_cn_CS_KEY, /* handled by writer */
    ewrefseq_cn_READ,
    ewrefseq_cn_QUALITY, /* optional */
    ewrefseq_cn_SEQ_ID, /* static/default */
    ewrefseq_cn_SEQ_START, /* calculated on fly by writer */
    ewrefseq_cn_SEQ_LEN, /* calculated on fly by writer */
    ewrefseq_cn_CIRCULAR, /* static/default */
    ewrefseq_cn_Last = ewrefseq_cn_CIRCULAR
};

enum ETableWriterRefSeq_ColOptions {
    ewrefseq_co_QUALITY = 0x01 /* use QUALITY column, by default not opened */
};

typedef struct TableWriterRefSeqData_struct {
    TableWriterData read;
    TableWriterData quality;
} TableWriterRefSeqData;

typedef struct TableWriterRefSeq TableWriterRefSeq;

ALIGN_EXTERN rc_t CC TableWriterRefSeq_Make(const TableWriterRefSeq** cself, VDBManager* mgr, const char* schema_path,
                                            const char* table_path, const uint32_t options);

/* rows optional here, app signature is mandatory */
ALIGN_EXTERN rc_t CC TableWriterRefSeq_Whack(const TableWriterRefSeq* cself, bool commit, uint64_t* rows,
                                             const char loader_name[],
                                             const ver_t loader_version,
                                             const char loader_date[],
                                             const char app_name[],
                                             const ver_t app_version);

ALIGN_EXTERN rc_t CC TableWriterRefSeq_WriteDefault(const TableWriterRefSeq* cself,
                                                    enum ETableWriterRefSeq_ColNames col, const TableWriterData* data);

/* rowid optional here */
ALIGN_EXTERN rc_t CC TableWriterRefSeq_Write(const TableWriterRefSeq* cself, const TableWriterRefSeqData* data, int64_t* rowid);

#ifdef __cplusplus
}
#endif

#endif /* _h_align_writer_refseq_ */
