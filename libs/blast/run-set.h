#ifndef _h_libs_blast_run_set_
#define _h_libs_blast_run_set_

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

#include "blast-mgr.h" /* BTableType */

#ifndef _h_insdc_insdc_
#include <insdc/insdc.h> /* INSDC_coord_len */
#endif

#ifndef _h_insdc_sra_
#include <insdc/sra.h> /* INSDC_SRA_platform_id */
#endif

#ifndef _h_klib_container_
#include <klib/container.h> /* BSTree */
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h> /* KRefcount */
#endif

#ifndef _h_ncbi_vdb_blast_
#include <ncbi/vdb-blast.h> /* VdbBlastStatus */
#endif

#include <stdbool.h> /* bool */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    eColTypeError,
    eColTypeAbsent,
    eColTypeStatic,
    eColTypeNonStatic
} EColType;

typedef enum {
    eFixedReadN,
    eFactor10,
} EReadIdType;

typedef struct {
    EReadIdType idType;
    uint32_t runBits;
    bool varReadN;
} ReadIdDesc;

typedef struct {
    uint32_t index;

    uint64_t spotCount;
    uint32_t spotBits;

    uint8_t nReads;
    uint8_t nBioReads; /* knowing filtering (if static) and min_read_len info */
    uint64_t bioLen; /* per read. is assigned just when allStatic */
    INSDC_SRA_platform_id platform;

    uint64_t bioBaseCount; /* BIO_BASE_COUNT, ~0 if not found */
    uint64_t cmpBaseCount; /* CMP_BASE_COUNT, ~0 if not found */

    INSDC_read_type *readType;
    EColType readTypeStatic;

    uint32_t *readLen;
    EColType readLenStatic;

    uint8_t *rdFilter;
    EColType rdFilterStatic;

    bool varReadLen;
    ReadIdDesc readIdDesc;
} RunDesc;

typedef struct {
    const struct VDatabase *db;

    const struct VTable *seqTbl;
    const struct VTable *prAlgnTbl;
    const struct VTable *refTbl;

    /* WGS */
    const struct VCursor *cursACCESSION;
    uint32_t col_ACCESSION;

    /* SRA_PLATFORM_PACBIO_SMRT : variable read number */
    const struct VCursor *cursSeq;
    uint32_t col_READ_FILTER;
    uint32_t col_READ_LEN;
    uint32_t col_READ_TYPE;
    uint32_t col_TRIM_LEN;
    uint32_t col_TRIM_START;
} VdbBlastDb;

typedef struct {
    /* rundesc; */
    char *acc;
    char *path;

    VdbBlastDb *obj;
    BTableType type;
    bool cSra;

   /* bioReads = numSequences = number-of-spots * number-of-bio-reads-in-spot */
    uint64_t bioReads;

    uint64_t alignments; /* rows number in PRIMARY_ALIGNMENT table */

    bool bioReadsTooExpensive; /* numSequences is TooExpensive */
    uint64_t bioReadsApprox;   /* numSequencesApprox; */

    uint64_t bioBases;         /* length; */
    bool bioBasesTooExpensive; /* totalLength is TooExpensive */
    uint64_t bioBasesApprox;   /* lengthApprox; */

    RunDesc rd;

    uint32_t min_read_length;
} VdbBlastRun;

typedef struct VdbBlastRef VdbBlastRef;
typedef struct {
    VdbBlastRef  *rfd;
    size_t        rfdk; /* Number of rfd members */
    size_t        rfdn; /* Allocated rfd members */

    uint64_t  totalLen; /* Total number of bases in reference set.
                           Base count for circular references is doubled. */

    BSTree tRuns;       /* rundesc-s */
    BSTree tExtRefs;    /* SEQ_ID-s of external references */
    BSTree tIntRefs;    /* SEQ_ID-s of external references */
} RefSet;

void _RefSetFini(RefSet *self);

typedef struct RunSet {
    VdbBlastRun *run;
    uint32_t krun; /* number of run-s */
    uint32_t nrun; /* sizeof of run-s */

    RefSet refs;
} RunSet;

typedef struct {
    const VdbBlastRun *prev;

    VdbBlastRun *run;

    uint32_t tableId;
 /* VDB_READ_UNALIGNED, VDB_READ_ALIGNED or VDB_READ_DIRECT */

    uint64_t spot; /* 1-based */
    uint32_t read; /* 1-based */
    uint32_t nReads; /* is variable in SRA_PLATFORM_PACBIO_SMRT */

    uint64_t read_id; /* BioReadId in RunSet */

    bool circular;
               /* we are going to return a circular reference the second time */
} ReadDesc;

typedef struct {
    uint32_t col_PRIMARY_ALIGNMENT_ID;
    uint32_t col_READ_FILTER;
    uint32_t col_READ_LEN;
    uint32_t col_TRIM_LEN;
    uint32_t col_TRIM_START;

    int64_t *primary_alignment_id;
    uint8_t *read_filter;
    uint32_t *read_len;
    INSDC_coord_len TRIM_LEN;
    INSDC_coord_val TRIM_START;

    uint8_t nReadsAllocated;
} ReaderCols;

/* cSRA READ mode : is ignored for non-cSRA runs */
typedef uint32_t KVdbBlastReadMode;
enum {
    VDB_READ_UNALIGNED =       1, /* return unaligned reads */
    VDB_READ_ALIGNED   =       2, /* return aligned reads */
    VDB_READ_REFERENCE =       3, /* return reference sequence */
};

struct References;

typedef struct {
    bool eor;
    ReadDesc desc;
    uint32_t col_READ;
    const struct VCursor *curs;
    size_t starting_base; /* 0-based, in current read */
    ReaderCols cols;
    KVdbBlastReadMode mode;
    const struct References *refs;
} Reader2na;

typedef struct Core2na {
    uint32_t min_read_length;
    bool hasReader;
    struct KLock *mutex;
    uint64_t initial_read_id;
    uint32_t irun; /* index in RunSet */
    bool eos;
    Reader2na reader;
} Core2na;

typedef struct Core4na {
    uint32_t min_read_length;
    struct KLock *mutex;
    ReadDesc desc;
    const struct VCursor *curs;
    const struct VBlob *blob; /* TODO */
    ReaderCols cols;
    uint32_t col_READ;
    KVdbBlastReadMode mode;
} Core4na;

struct VdbBlastRunSet {
    KRefcount refcount;
    bool protein;
    VdbBlastMgr *mgr;

    RunSet runs;

    bool beingRead;
    ReadIdDesc readIdDesc;

    Core2na core2na;
    Core4na core4na;

    Core2na core2naRef;
    Core4na core4naRef;

    uint64_t minSeqLen;
    uint64_t avgSeqLen;
    uint64_t maxSeqLen;
};

rc_t _VTableMakeCursor(const struct VTable *self, const struct VCursor **curs,
    uint32_t *col_idx, const char *col_name, const char *acc);

rc_t _ReadDescFindNextRead(ReadDesc *self, bool *found);
VdbBlastStatus _ReadDescFixReadId(ReadDesc *self);

uint64_t _VdbBlastRunAdjustSequencesAmountForAlignments(VdbBlastRun *self,
    VdbBlastStatus *status);

#ifdef TEST_VdbBlastRunFillReadDesc
VDB_EXTERN
#endif
uint32_t _VdbBlastRunFillReadDesc(VdbBlastRun *self,
    uint64_t read_id, ReadDesc *desc);

uint64_t _VdbBlastRunGetNumAlignments(VdbBlastRun *self,
    VdbBlastStatus *status);

bool _VdbBlastRunVarReadNum(const VdbBlastRun *self);

uint32_t _RunSetFindReadDesc(const struct RunSet *self,
    uint64_t read_id, ReadDesc *desc);

uint64_t _VdbBlastRunSet2naRead(const VdbBlastRunSet *self,
    VdbBlastStatus *status, uint64_t *read_id, size_t *starting_base,
    uint8_t *buffer, size_t buffer_size, KVdbBlastReadMode mode);

void _VdbBlastRunSetBeingRead(const VdbBlastRunSet *self);

VdbBlastStatus _VdbBlastRunSetFindFirstRead
    (const VdbBlastRunSet *self, uint64_t *read_id, bool useGetFirstRead);

uint64_t _VdbBlastRunSetGetAllReads(const VdbBlastRunSet *self, uint32_t run);

EReadIdType _VdbBlastRunSetGetReadIdType(const VdbBlastRunSet *self);


#ifdef __cplusplus
}
#endif

#endif /* _h_libs_blast_run_set_ */
