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
#include <vdb/extern.h>

#include <sra/sradb.h>
#include <vdb/xform.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <klib/log.h>

#include <string.h>
#include <assert.h>

#define DEBUG_PRINT_RESULT 0
#if DEBUG_PRINT_RESULT
#include <stdio.h>
#endif

#define A_4na (1)
#define C_4na (2)
#define G_4na (4)
#define T_4na (8)
#define NO_4na (0)

typedef struct params self_t;
typedef int (*spot_filter_func)(self_t const *self
                              , unsigned const nreads
                              , int32_t const *start
                              , uint32_t const *len
                              , uint8_t const *type
                              , uint8_t const *read
                              , uint8_t const *qual);
struct params {
    spot_filter_func spot_filter;
    unsigned min_length;
    int min_quality;
    int no_quality;
};

#define M (((self_t const *)self)->min_length)
#define Q (((self_t const *)self)->min_quality)
#define BAD_QUAL(QV) (QV < ((self_t const *)self)->min_quality && QV != ((self_t const *)self)->no_quality)

enum RejectCause {
    notRejected,
    spotFilter,
    tooShort,
    badQualValueFirstM,
    ambiguousFirstM,
    lowComplexityFirstM,
    badBaseValue,
    tooManyAmbiguous
};

static int spot_filter_4na(self_t const *self
                            , unsigned const nreads
                            , int32_t const *start
                            , uint32_t const *len
                            , uint8_t const *type
                            , uint8_t const *read
                            , uint8_t const *qual)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & READ_TYPE_REVERSE) == READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[(A_4na|C_4na|G_4na|T_4na)+1];

        if ((type[i] & READ_TYPE_BIOLOGICAL) != READ_TYPE_BIOLOGICAL)
            continue;
        
        if (readLen < M)
            return tooShort;

        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];
            int const qval = qual[k];

            ++count[base];
            if (BAD_QUAL(qval)) return badQualValueFirstM; ///< first M quality values must all be >= minimum quality value
        }
        {
            unsigned const unambiguous = count[A_4na]+count[C_4na]+count[G_4na]+count[T_4na];
            if (unambiguous != M) return ambiguousFirstM; ///< first M bases must be unambiguous
        }
        {
            bool const all_A = count[A_4na] == M;
            bool const all_C = count[C_4na] == M;
            bool const all_G = count[G_4na] == M;
            bool const all_T = count[T_4na] == M;
            if (all_A || all_C || all_G || all_T) return lowComplexityFirstM; ///< first M bases must not be all the same
        }
        for ( ; j < readLen; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            ++count[base];
        }
        if (count[NO_4na] != 0) return badBaseValue; ///< all bases must be one of ACGT or UIPAC ambiguity codes
        {
            unsigned const unambiguous = count[A_4na]+count[C_4na]+count[G_4na]+count[T_4na];
            if (unambiguous * 2 < readLen) return tooManyAmbiguous; ///< at least 1/2 the bases must be unambiguous
        }
    }
    return notRejected;
}

/* read contains only ACGTN */
static int spot_filter_x2na(self_t const *self
                          , unsigned const nreads
                          , int32_t const *start
                          , uint32_t const *len
                          , uint8_t const *type
                          , uint8_t const *read
                          , uint8_t const *qual)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & READ_TYPE_REVERSE) == READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[5];

        if ((type[i] & READ_TYPE_BIOLOGICAL) != READ_TYPE_BIOLOGICAL)
            continue;
        
        if (readLen < M)
            return tooShort;

        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];
            int const qval = qual[k];

            if (BAD_QUAL(qval)) return badQualValueFirstM; ///< first M quality values must all be >= minimum quality value
            ++count[base];
        }
        {
            unsigned const unambiguous = count[0]+count[1]+count[2]+count[3];
            if (unambiguous != M) return ambiguousFirstM; ///< first M bases must be unambiguous
        }
        {
            bool const all_A = count[0] == M;
            bool const all_C = count[1] == M;
            bool const all_G = count[2] == M;
            bool const all_T = count[3] == M;
            if (all_A || all_C || all_G || all_T) return lowComplexityFirstM; ///< first M bases must not be all the same
        }
        for ( ; j < readLen; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            ++count[base];
        }
        {
            unsigned const unambiguous = count[0]+count[1]+count[2]+count[3];
            if (unambiguous * 2 < readLen) return tooManyAmbiguous; ///< at least 1/2 the bases must be unambiguous
        }
    }
    return notRejected;
}

static int spot_filter_2na(self_t const *self
                         , unsigned const nreads
                         , int32_t const *start
                         , uint32_t const *len
                         , uint8_t const *type
                         , uint8_t const *read
                         , uint8_t const *qual)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & READ_TYPE_REVERSE) == READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[4];

        if ((type[i] & READ_TYPE_BIOLOGICAL) != READ_TYPE_BIOLOGICAL)
            continue;
        
        if (readLen < M)
            return tooShort;

        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const qval = qual[k];
            int const base = read[k];

            if (BAD_QUAL(qval)) return badQualValueFirstM; ///< first M quality values must all be >= minimum quality value
            ++count[base];
        }
        {
            bool const all_A = count[0] == M;
            bool const all_C = count[1] == M;
            bool const all_G = count[2] == M;
            bool const all_T = count[3] == M;
            if (all_A || all_C || all_G || all_T) return lowComplexityFirstM; ///< first M bases must not be all the same
        }
        /* no checks for ambiguity since 2na can't represent ambiguous bases */
    }
    return notRejected;
}

#define SPOT_FILTER ((self_t const *)self)->spot_filter
#define BIND_COLUMN(ELEM, DTYPE, POINTER) DTYPE const *const POINTER = (ELEM < argc && sizeof(DTYPE) * 8 == (size_t)argv[ELEM].u.data.elem_bits) ? (((DTYPE const *)argv[ELEM].u.data.base) + argv[ELEM].u.data.first_elem) : ((DTYPE const *)NULL)
#define SAFE_COUNT(ELEM) (ELEM < argc ? argv[ELEM].u.data.elem_count : 0)
#define SAME_COUNT(ELEM1, ELEM2) (SAFE_COUNT(ELEM1) == SAFE_COUNT(ELEM2))

static
rc_t CC make_spot_filter(void *const self
                         , const VXformInfo *const info
                         , int64_t const row_id
                         , VRowResult *const rslt
                         , uint32_t const argc
                         , const VRowData *const argv)
{
    enum COLUMNS {
        COL_READ,
        COL_QUALITY,
        COL_READ_START,
        COL_READ_LEN,
        COL_READ_TYPE,
        COL_SPOT_FILTER
    };
    unsigned const nreads = (unsigned)SAFE_COUNT(COL_READ_LEN);
    unsigned const nfilt = (unsigned)SAFE_COUNT(COL_SPOT_FILTER);
    BIND_COLUMN(COL_READ       ,  uint8_t, read  ); ///< 4NA or x2na or 2na
    BIND_COLUMN(COL_QUALITY    ,  uint8_t, qual  ); ///< phred+0
    BIND_COLUMN(COL_READ_START ,  int32_t, start );
    BIND_COLUMN(COL_READ_LEN   , uint32_t, len   );
    BIND_COLUMN(COL_READ_TYPE  ,  uint8_t, type  );
    BIND_COLUMN(COL_SPOT_FILTER,  uint8_t, filter);
    enum RejectCause result = notRejected;
    rc_t rc = 0;

    assert(read != NULL);
    assert(qual != NULL);
    assert(start != NULL);
    assert(len != NULL);
    assert(type != NULL);
    assert(filter != NULL);
    assert(SAME_COUNT(COL_READ, COL_QUALITY));
    assert(SAME_COUNT(COL_READ_START, COL_READ_LEN));
    assert(SAME_COUNT(COL_READ_START, COL_READ_TYPE));

    {
        unsigned i;
        for (i = 0; i < nfilt; ++i) {
            if (filter[i] == SRA_SPOT_FILTER_REJECT) {
                result = spotFilter;
                break;
            }
        }
    }
    if (result == notRejected)
        result = SPOT_FILTER(self, nreads, start, len, type, read, qual);

#if DEBUG_PRINT_RESULT
    switch (result) {
    case notRejected:
        fprintf(stderr, "passed\n");
        break;
    case spotFilter:
        fprintf(stderr, "spot filter was set\n");
        break;
    case tooShort:
        fprintf(stderr, "sequence length < %i\n", M);
        break;
    case badQualValueFirstM:
        fprintf(stderr, "quality value < %i in first %i bases\n", Q, M);
        break;
    case ambiguousFirstM:
        fprintf(stderr, "ambiguous base in first %i bases\n", M);
        break;
    case lowComplexityFirstM:
        fprintf(stderr, "first %i bases are all the same\n", M);
        break;
    case badBaseValue:
        fprintf(stderr, "bad base value\n");
        break;
    case tooManyAmbiguous:
        fprintf(stderr, "at lease 1/2 bases are ambiguous\n");
        break;
    default:
        assert(!"reachable");
    }
#endif
    
    rslt->data->elem_bits = 8;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
    {
        uint8_t *const dst = (uint8_t *)rslt->data->base;

        rslt->elem_bits = rslt->data->elem_bits;
        rslt->elem_count = 1;
        dst[0] = result == notRejected ? SRA_SPOT_FILTER_PASS : SRA_SPOT_FILTER_REJECT;
    }
    return rc;
}

static spot_filter_func read_data_type_to_function(VSchema const *const schema, VFormatdecl const *const read_data_type)
{
    VFormatdecl decl;
    rc_t rc;

    rc = VSchemaResolveFmtdecl(schema, &decl, "INSDC:4na:bin");
    assert(rc == 0);
    if (read_data_type->td.type_id == decl.td.type_id) {
        LOGMSG(klogDebug, "matched INSDC:4na:bin");
        return spot_filter_4na;
    }
    rc = VSchemaResolveFmtdecl(schema, &decl, "INSDC:x2na:bin");
    assert(rc == 0);
    if (read_data_type->td.type_id == decl.td.type_id) {
        LOGMSG(klogDebug, "matched INSDC:x2na:bin");
        return spot_filter_x2na;
    }
    rc = VSchemaResolveFmtdecl(schema, &decl, "INSDC:2na:bin");
    assert(rc == 0);
    if (read_data_type->td.type_id == decl.td.type_id) {
        LOGMSG(klogDebug, "matched INSDC:2na:bin");
        return spot_filter_2na;
    }
    LOGMSG(klogDebug, "matched no type");
    return NULL;
}

/*
  function NCBI:SRA:spot_filter < * U32 min_length, * U8 min_quality, * U8 no_quality >
  NCBI:SRA:make_spot_filter #1
       ( INSDC:dna:bin read, INSDC:quality:phred quality,
         INSDC:coord:zero read_start, U32 read_len,
         INSDC:SRA:read_type read_type, INSDC:SRA:spot_filter spot_filter )

 */
VTRANSFACT_IMPL( NCBI_SRA_make_spot_filter, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    spot_filter_func func = read_data_type_to_function(info->schema, &dp->argv[0].fd);
    if (func) {
        self_t *self = malloc(sizeof(*self));
        self->min_length = 10;
        self->min_quality = 4;
        self->no_quality = -1;
        assert(cp->argc <= 3);
        if (cp->argc >= 1) {
            assert(cp->argv[0].count == 1);
            self->min_length = cp->argv[0].data.u32[0];
            if (cp->argc >= 2) {
                assert(cp->argv[1].count == 1);
                self->min_quality = cp->argv[1].data.u8[0];
                if (cp->argc >= 3) {
                    assert(cp->argv[2].count == 1);
                    self->no_quality = cp->argv[2].data.u8[0];
                }
            }
        }
        self->spot_filter = func;
        rslt->self = self;
        rslt->whack = free;
        rslt -> u . rf = make_spot_filter;
        rslt -> variant = vftRow;
        return 0;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcType, rcInvalid);
}
