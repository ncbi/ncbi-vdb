/* ===========================================================================
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

#ifndef _h_samextract_lib_
#define _h_samextract_lib_
#include <align/extern.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/vector.h>
#include <kproc/queue.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

typedef enum efile_type {
    unknown = 999,
    SAM,
    BAM,
    SAMGZUNSUPPORTED
} efile_type;

typedef struct Alignment
{
    const char* qname;
    uint16_t flags;
    const char* rname;
    int32_t pos;
    uint8_t mapq;
    const char* cigar;
    const char* rnext;
    int32_t pnext;
    int32_t tlen;
    const char* read;
    const char* qual;
} Alignment;

typedef struct SAMExtractor
{
    const KFile* infile;

    Vector headers;
    Vector alignments;
    Vector bam_references;

    Vector tagvalues;
    Vector* prev_headers;
    Vector* prev_aligns;

    Vector threads;
    KQueue* inflatequeue;
    KQueue* parsequeue;

    uint64_t file_pos;
    char* readbuf;
    uint32_t readbuf_sz;
    uint32_t readbuf_pos;

    efile_type file_type;

    rc_t rc;

    String* filter_rname;
    ssize_t filter_pos;
    ssize_t filter_length;

    int32_t num_threads;
    int32_t pos;

    int32_t n_ref;

    bool filter_ordered;

    bool hashdvn;
    bool hashdso;
    bool hashdgo;
    bool hassqsn;
    bool hassqln;
    bool hasrgid;
    bool haspgid;
} SAMExtractor;

typedef struct tagvalue
{
    const char* tag; /* VN, SN, LN, ID, ... */
    const char* value;
} TagValue;

typedef struct Header
{
    const char* headercode; /* HD, SQ, RG, PG, CO */
    Vector tagvalues;
} Header;

ALIGN_EXTERN rc_t CC SAMExtractorMake(SAMExtractor** state, const KFile* fin,
                                      String* fname_desc,
                                      int32_t num_threads);

ALIGN_EXTERN rc_t CC SAMExtractorAddFilterName(SAMExtractor* state,
                                               String* rname, bool ordered);
ALIGN_EXTERN rc_t CC SAMExtractorAddFilterNamePos(SAMExtractor* state,
                                                  String* rname, ssize_t pos,
                                                  bool ordered);
ALIGN_EXTERN rc_t CC SAMExtractorAddFilterNamePosLength(SAMExtractor* state,
                                                        String* rname,
                                                        ssize_t pos,
                                                        ssize_t length,
                                                        bool ordered);
ALIGN_EXTERN rc_t CC SAMExtractorAddFilterPos(SAMExtractor* state,
                                              ssize_t pos, bool ordered);
ALIGN_EXTERN rc_t CC SAMExtractorAddFilterPosLength(SAMExtractor* state,
                                                    ssize_t pos,
                                                    ssize_t length,
                                                    bool ordered);

ALIGN_EXTERN rc_t CC SAMExtractorRelease(SAMExtractor* state); /* dtor */

ALIGN_EXTERN rc_t CC SAMExtractorGetHeaders(SAMExtractor* state,
                                            Vector* headers);
ALIGN_EXTERN rc_t CC SAMExtractorInvalidateHeaders(SAMExtractor* state);

ALIGN_EXTERN rc_t CC SAMExtractorGetAlignments(SAMExtractor* state,
                                               Vector* alignments);
ALIGN_EXTERN rc_t CC SAMExtractorInvalidateAlignments(SAMExtractor* state);

#ifdef __cplusplus
}
#endif
#endif /* __h_sam_extract_lib_ */
