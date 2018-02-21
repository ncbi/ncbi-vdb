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
#ifndef _h_align_writer_reference_
#define _h_align_writer_reference_

#ifndef _h_align_extern_
#include <align/extern.h>
#endif

#include <insdc/insdc.h>
#include <align/writer-cmn.h>
#include <align/writer-alignment.h>
#include <align/reader-refseq.h>

#ifdef __cplusplus
extern "C" {
#endif

struct KFile;

enum EReference_Options {
    ewrefmgr_co_allREADs = 0x01, /* always write READ */
    ewrefmgr_co_Coverage = 0x02,  /* use coverage data, by default not used */
    ewrefmgr_co_AcceptHardClip = 0x04 /* accept hard clipping in CIGAR */
};

typedef struct ReferenceMgr ReferenceMgr;

/* Writes REFERENCE table with in given db
    
    vmgr [IN] - needed only if conf is not NULL
    conf [IN] - optional path to tab separated file to resolve reference names
                from source files to standard refseq accessions;
                standard refseq dictionary location is obtained from kfg files.
    path {IN] - optional path points to a directory with 'local' references
                (not found in conf file), default is '.'
    max_seq_len [IN] - 0 - use default
    cache [IN] - db cache size per single!!! reference (vdb cursor cache)
    num_open [IN] - number of refseq kept open, 0 - no limit
 */
ALIGN_EXTERN rc_t CC ReferenceMgr_Make(const ReferenceMgr** cself, VDatabase* db, const VDBManager* vmgr,
                                       const uint32_t options, const char* conf, const char* path, uint32_t max_seq_len,
                                       size_t cache, uint32_t num_open);

ALIGN_EXTERN rc_t CC ReferenceMgr_Release(const ReferenceMgr *cself,
                                          const bool commit,
                                          uint64_t *const Rows,
                                          const bool build_coverage,
                                          rc_t (*const quitting)(void)
                                          );

ALIGN_EXTERN rc_t CC ReferenceMgr_SetCache(ReferenceMgr const *const self, size_t cache, uint32_t num_open);

typedef struct ReferenceSeq ReferenceSeq;

/* id: chr12 or NC_000001.3 */
ALIGN_EXTERN rc_t CC ReferenceMgr_GetSeq(ReferenceMgr const *self,
                                         ReferenceSeq const **seq,
                                         char const id[],
                                         bool *shouldUnmap,
                                         bool allowMultiMapping,
                                         bool wasRenamed[]);

ALIGN_EXTERN rc_t CC ReferenceMgr_Verify(ReferenceMgr const *self,
                                         char const id[],
                                         INSDC_coord_len length,
                                         uint8_t const md5[16],
                                         bool allowMultiMapping,
                                         bool wasRenamed[]);

ALIGN_EXTERN rc_t CC ReferenceMgr_Get1stRow(const ReferenceMgr* cself, int64_t* row_id, char const id[]);
    
ALIGN_EXTERN rc_t CC ReferenceMgr_FastaPath(const ReferenceMgr* cself, const char* fasta_path);

ALIGN_EXTERN rc_t CC ReferenceMgr_FastaFile(const ReferenceMgr* cself, struct KFile const* file);

enum EReference_CompressionOptions {
    ewrefmgr_cmp_Binary = 0x01, /* binary cigar on input (BAM format: 28+4 bits) */
    ewrefmgr_cmp_Exact = 0x02 /* exact matching no cliping */
};

ALIGN_EXTERN rc_t CC ReferenceMgr_Compress(const ReferenceMgr* cself,
                                           uint32_t options,
                                           const char* id,
                                           INSDC_coord_zero offset,
                                           const char* seq,
                                           INSDC_coord_len seq_len,
                                           const void* cigar,
                                           uint32_t cigar_len,
                                           INSDC_coord_zero allele_offset,
                                           const char* allele,
                                           INSDC_coord_len allele_len,
                                           INSDC_coord_zero offset_in_allele,
                                           const void* allele_cigar,
                                           uint32_t allele_cigar_len,
                                           uint8_t rna_orient,
                                           TableWriterAlgnData* data);


/* Read refseq chunk of 'len' bases into provided 'buffer' (must be enough big for len);
   ref_len - on return has number of bases written to the buffer
 */
ALIGN_EXTERN rc_t CC ReferenceSeq_Read(const ReferenceSeq* cself, INSDC_coord_zero offset, INSDC_coord_len len,
                                       uint8_t* buffer, INSDC_coord_len* ref_len);

ALIGN_EXTERN rc_t CC ReferenceSeq_Get1stRow(const ReferenceSeq* cself, int64_t* row_id);
    
ALIGN_EXTERN rc_t CC ReferenceSeq_GetID(ReferenceSeq const *self, char const **rslt);

ALIGN_EXTERN rc_t CC ReferenceSeq_Compress(const ReferenceSeq* cself,
                                           uint32_t options,
                                           INSDC_coord_zero offset,
                                           const char* seq,
                                           INSDC_coord_len seq_len,
                                           const void* cigar,
                                           uint32_t cigar_len,
                                           INSDC_coord_zero allele_offset,
                                           const char* allele,
                                           INSDC_coord_len allele_len,
                                           INSDC_coord_zero offset_in_allele,
                                           const void* allele_cigar,
                                           uint32_t allele_cigar_len,
                                           uint8_t rna_orient,
                                           TableWriterAlgnData* data);
                                           
ALIGN_EXTERN rc_t CC ReferenceSeq_TranslateOffset_int(ReferenceSeq const *const cself,
                                                      INSDC_coord_zero const offset,
                                                      int64_t *const ref_id,
                                                      INSDC_coord_zero *const ref_start,
                                                      uint64_t *const global_ref_start);
                                           

enum ReferenceSeqCoverageTableType {
    ewrefcov_primary_table = 0,
    ewrefcov_secondary_table,
    ewrefcov_evidence_table
};

typedef struct ReferenceSeqCoverage_struct {
    /* based only on primary and secondary */
    uint8_t high;
    uint8_t low;
    uint32_t mismatches;
    uint32_t indels;
    /* in that order (use enum above): prim, 2nd, evidence */
    INSDC_coord_zero overlap_ref_pos[3];
    INSDC_coord_len overlap_ref_len[3];
    TableWriterData ids[3];
} ReferenceSeqCoverage;

ALIGN_EXTERN rc_t CC ReferenceSeq_AddCoverage(const ReferenceSeq* cself, INSDC_coord_zero offset, const ReferenceSeqCoverage* data);

ALIGN_EXTERN rc_t CC ReferenceSeq_Release(const ReferenceSeq* cself);

#ifdef __cplusplus
}
#endif

#endif /* _h_align_writer_reference_ */
