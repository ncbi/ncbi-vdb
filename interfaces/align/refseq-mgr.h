/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was readten as part of
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
#ifndef _h_align_refseq_mgr_
#define _h_align_refseq_mgr_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#include <vdb/manager.h>
#include <vdb/table.h>
#include <align/reader-refseq.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RefSeqMgr RefSeqMgr;

/* Manages refseq tables
    reader_options [IN] - passed to TableReaderRefSeq
    cache [IN] - db cache size per single!!! reference (vdb cursor cache)
    keep_open_num [IN] - number of refseq kept open, 0 - no limit
 */
ALIGN_EXTERN rc_t CC RefSeqMgr_Make(const RefSeqMgr** cself, const VDBManager* vmgr,
                                    uint32_t reader_options, size_t cache, uint32_t keep_open_num);

ALIGN_EXTERN rc_t CC RefSeqMgr_Release(const RefSeqMgr* cself);

ALIGN_EXTERN rc_t CC RefSeqMgr_SetCache(RefSeqMgr const *const cself, size_t cache, uint32_t keep_open_num);

/* return value if 0 means object was found, path is optional */
ALIGN_EXTERN rc_t RefSeqMgr_Exists(const RefSeqMgr* cself, const char* accession, uint32_t accession_sz, char** path);

/* Read from refseq ided by seq_id chunk of 'len' bases into provided 'buffer' (must be enough big for len);
   on return written has number of bases written
 */
ALIGN_EXTERN rc_t CC RefSeqMgr_Read(const RefSeqMgr* cself, const char* seq_id, uint32_t seq_id_sz,
                                    INSDC_coord_zero offset, INSDC_coord_len len,
                                    uint8_t* buffer, INSDC_coord_len* written);

/* db: path to parents's database: is used to resolve refseq in Accession Directory */
ALIGN_EXTERN rc_t CC RefSeqMgr_ReadForDb(const RefSeqMgr* cself, const char* seq_id,
    uint32_t seq_id_sz, INSDC_coord_zero offset, INSDC_coord_len len,
    uint8_t* buffer, INSDC_coord_len* written, const String * accOfParentDb);

typedef struct RefSeq RefSeq;

ALIGN_EXTERN rc_t CC RefSeqMgr_GetSeq(const RefSeqMgr* cmgr, const RefSeq** cself, const char* seq_id, uint32_t seq_id_sz);

/* Same as RefSeqMgr_Read
 */
ALIGN_EXTERN rc_t CC RefSeq_Read(const RefSeq* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                                 uint8_t* buffer, INSDC_coord_len* written);

ALIGN_EXTERN rc_t CC RefSeq_Circular(const RefSeq* cself, bool* circular);

ALIGN_EXTERN rc_t CC RefSeq_SeqLength(const RefSeq* cself, INSDC_coord_len* len);

ALIGN_EXTERN rc_t CC RefSeq_MD5(const RefSeq* cself, const uint8_t** md5);

ALIGN_EXTERN rc_t CC RefSeq_Name(const RefSeq* cself, const char** name);
    
ALIGN_EXTERN rc_t CC RefSeq_Release(const RefSeq* cself);

#ifdef __cplusplus
}
#endif

#endif /* _h_align_refseq_mgr_ */
