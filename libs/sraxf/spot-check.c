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
#include <assert.h>

#include <sra/sradb.h>

#define A_4na (1)
#define C_4na (2)
#define G_4na (4)
#define T_4na (8)
#define NO_4na (0)

#define USE_BASE_FILTER 0
#define PARAMS_DEFAULT_MIN_LENGTH (10u)
#define PARAMS_DEFAULT_MIN_QUALITY (4)
#define PARAMS_DEFAULT_NO_QUALITY (-1)

typedef struct params self_t;

typedef int (*spot_filter_func)(self_t const *self
                              , unsigned const nreads
                              , int32_t const *start
                              , uint32_t const *len
                              , uint8_t const *type
                              , uint8_t const *read);

struct params {
    spot_filter_func spot_filter;
    unsigned min_length;
    int min_quality;
    int no_quality;
};

/** \brief: Initialize parameter block with default values */
static struct params *initialize_params(struct params *self)
{
    if (self) {
        memset(self, 0, sizeof(*self));
        self->min_length = PARAMS_DEFAULT_MIN_LENGTH;
        self->min_quality = PARAMS_DEFAULT_MIN_QUALITY;
        self->no_quality = PARAMS_DEFAULT_NO_QUALITY;
    }
    return self;
}

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
    tooManyAmbiguous,
    tooManyBadQualValues
};

#define DEBUG_PRINT_CAUSE 0
#if DEBUG_PRINT_CAUSE
#include <stdio.h>
static void printRejectCause(enum RejectCause const cause, void const *const self)
{
    switch (cause) {
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
        fprintf(stderr, "no quality value >= %i for first or last %i bases\n", Q, M);
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
        fprintf(stderr, "fewer than 50%% bases are unambiguous\n");
        break;
    case tooManyBadQualValues:
        fprintf(stderr, "fewer than 50%% bases have quality values >= %i\n", Q);
        break;
    }
}
#else
static void printRejectCause(enum RejectCause const cause, void const *const self) {}
#endif

static int check_quality(self_t const *self
                         , unsigned const nreads
                         , int32_t const *start
                         , uint32_t const *len
                         , uint8_t const *type
                         , uint8_t const *qual)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        unsigned good = 0;
        unsigned first = readLen;
        unsigned last = readLen;
        unsigned j = 0;
        
        if ((type[i] & SRA_READ_TYPE_BIOLOGICAL) != SRA_READ_TYPE_BIOLOGICAL)
            continue;

        if (readLen < M)
            return tooShort;

        for (j = 0; j < readLen; ++j) {
            if (BAD_QUAL(qual[j]))
                continue;

            if (first == readLen)
                first = j;
            last = j;
            ++good;
        }
        if (good + good < readLen)
            return tooManyBadQualValues;
        last = readLen - last - 1;
        if (first >= M || last >= M)
            return badQualValueFirstM;
    }
    return notRejected;
}

static bool check_ambiguousFirstM(self_t const *self, unsigned const A, unsigned const C, unsigned const G, unsigned const T)
{
    return (A+C+G+T) == M;
}

static bool check_tooManyAmbiguous(self_t const *self, unsigned readLen, unsigned const A, unsigned const C, unsigned const G, unsigned const T)
{
    return (A+C+G+T) * 2 >= readLen;
}

static bool check_lowComplexityFirstM(self_t const *self, unsigned const A, unsigned const C, unsigned const G, unsigned const T)
{
    bool const all_A = A == M;
    bool const all_C = C == M;
    bool const all_G = G == M;
    bool const all_T = T == M;
    return !(all_A || all_C || all_G || all_T);
}

static int check_dna(self_t const *self
                     , unsigned const nreads
                     , int32_t const *start
                     , uint32_t const *len
                     , uint8_t const *type
                     , uint8_t const *read)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & SRA_READ_TYPE_REVERSE) == SRA_READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[256];
        
        if ((type[i] & SRA_READ_TYPE_BIOLOGICAL) != SRA_READ_TYPE_BIOLOGICAL)
            continue;
        
        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];
            
            ++count[base];
        }
        
        if (!check_ambiguousFirstM(self, count['A'] + count['a'], count['C'] + count['c'], count['G'] + count['g'], count['T'] + count['t'])) return ambiguousFirstM; ///< first M bases must be unambiguous

        if (!check_lowComplexityFirstM(self, count['A'] + count['a'], count['C'] + count['c'], count['G'] + count['g'], count['T'] + count['t'])) return lowComplexityFirstM; ///< first M bases must not be all the same

        for ( ; j < readLen; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];
            
            ++count[base];
        }
        if (!check_tooManyAmbiguous(self, readLen, count['A'] + count['a'], count['C'] + count['c'], count['G'] + count['g'], count['T'] + count['t'])) return tooManyAmbiguous; ///< at least 1/2 the bases must be unambiguous

        {
            unsigned iupac = 0;
            for (j = 0; j < 16; ++j) {
                char const K[16] = {'.','A','C','M','G','R','S','V','T','W','Y','H','K','D','B','N'};
                char const k[16] = {'.','a','c','m','g','r','s','v','t','w','y','h','k','d','b','n'};
                iupac += count[(int)(K[j])] + count[(int)(k[j])];
            }
            if (iupac != readLen) return badBaseValue; ///< all bases must be one of ACGT or UIPAC ambiguity codes
        }
    }
    return notRejected;
}

static int check_4na(self_t const *self
                     , unsigned const nreads
                     , int32_t const *start
                     , uint32_t const *len
                     , uint8_t const *type
                     , uint8_t const *read)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & SRA_READ_TYPE_REVERSE) == SRA_READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[(A_4na|C_4na|G_4na|T_4na)+1];

        if ((type[i] & SRA_READ_TYPE_BIOLOGICAL) != SRA_READ_TYPE_BIOLOGICAL)
            continue;
        
        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            ++count[base];
        }
        
        if (!check_ambiguousFirstM(self, count[A_4na], count[C_4na], count[G_4na], count[T_4na])) return ambiguousFirstM; ///< first M bases must be unambiguous
        
        if (!check_lowComplexityFirstM(self, count[A_4na], count[C_4na], count[G_4na], count[T_4na])) return lowComplexityFirstM; ///< first M bases must not be all the same
        
        for ( ; j < readLen; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];
            
            ++count[base];
        }
        if (!check_tooManyAmbiguous(self, readLen, count[A_4na], count[C_4na], count[G_4na], count[T_4na])) return tooManyAmbiguous; ///< at least 1/2 the bases must be unambiguous

        if (count[NO_4na] != 0) return badBaseValue; ///< all bases must be one of ACGT or UIPAC ambiguity codes
    }
    return notRejected;
}

/* read contains only ACGTN */
static int check_x2na(self_t const *self
                      , unsigned const nreads
                      , int32_t const *start
                      , uint32_t const *len
                      , uint8_t const *type
                      , uint8_t const *read)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & SRA_READ_TYPE_REVERSE) == SRA_READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[5];

        if ((type[i] & SRA_READ_TYPE_BIOLOGICAL) != SRA_READ_TYPE_BIOLOGICAL)
            continue;
        
        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            ++count[base];
        }
        
        if (!check_ambiguousFirstM(self, count[0], count[1], count[2], count[3])) return ambiguousFirstM; ///< first M bases must be unambiguous
        
        if (!check_lowComplexityFirstM(self, count[0], count[1], count[2], count[3])) return lowComplexityFirstM; ///< first M bases must not be all the same
        
        for ( ; j < readLen; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];
            
            ++count[base];
        }
        if (!check_tooManyAmbiguous(self, readLen, count[0], count[1], count[2], count[3])) return tooManyAmbiguous; ///< at least 1/2 the bases must be unambiguous
    }
    return notRejected;
}

static int check_2na(self_t const *self
                     , unsigned const nreads
                     , int32_t const *start
                     , uint32_t const *len
                     , uint8_t const *type
                     , uint8_t const *read)
{
    unsigned i;
    for (i = 0; i < nreads; ++i) {
        unsigned const readLen = len[i];
        bool const rev = (type[i] & SRA_READ_TYPE_REVERSE) == SRA_READ_TYPE_REVERSE;
        unsigned j = 0;
        unsigned count[4];

        if ((type[i] & SRA_READ_TYPE_BIOLOGICAL) != SRA_READ_TYPE_BIOLOGICAL)
            continue;
        
        memset(count, 0, sizeof(count));
        for ( ; j < M; ++j) {
            unsigned const k = start[i] + (rev ? (readLen - j - 1) : j);
            int const base = read[k];

            ++count[base];
        }
        /* no checks for ambiguity since 2na can't represent ambiguous bases */

        if (!check_lowComplexityFirstM(self, count[0], count[1], count[2], count[3])) return lowComplexityFirstM; ///< first M bases must not be all the same
    }
    return notRejected;
}
