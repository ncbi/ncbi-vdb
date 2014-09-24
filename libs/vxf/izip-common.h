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

typedef struct {
    unsigned size;
    unsigned used;
    int64_t min;
    int64_t max;
    unsigned var;
    union {
        int64_t  raw[1];
        uint64_t u64[1];
        uint32_t u32[2];
        uint16_t u16[4];
        uint8_t  u8[8];
    } data;
} nbuf;

#define nbuf_get(X, I) (((const nbuf *)X)->var == 4 ? ((const nbuf *)X)->data.u8[(I)] : (((const nbuf *)X)->var == 3 ? ((const nbuf *)X)->data.u16[(I)] : (((const nbuf *)X)->var == 2 ? ((const nbuf *)X)->data.u32[(I)] : ((const nbuf *)X)->data.u64[(I)])))

static unsigned nbuf_size(const nbuf *x) {
    size_t y;
    
    switch (x->var) {
    default:
        y = sizeof(x->data.raw[0]) * x->used;
        break;
    case 1:
        y = sizeof(x->data.u64[0]) * x->used;
        break;
    case 2:
        y = sizeof(x->data.u32[0]) * x->used;
        break;
    case 3:
        y = sizeof(x->data.u16[0]) * x->used;
        break;
    case 4:
        y = sizeof(x->data.u8[0]) * x->used;
        break;
    }
    assert(y <= UINT32_MAX);
    return (unsigned)y;
}

static nbuf *alloc_nbuf(unsigned cnt, unsigned var) {
    nbuf *rslt;
    
    rslt = malloc(sizeof(*rslt) - sizeof(rslt->data) + cnt * sizeof(rslt->data.raw[0]));
    if ( rslt != NULL ) {
        memset(rslt, 0, sizeof(*rslt) - sizeof(rslt->data));
        rslt->size = cnt;
        rslt->used = 0;
        rslt->var = var;
    }
    return rslt;
}

static nbuf *alloc_raw_nbuf(unsigned cnt) {
    return alloc_nbuf(cnt, 0);
}

struct decoded {
    unsigned data_count;
    unsigned lines;
    unsigned outliers;
    unsigned size_type;
    
    nbuf *diff;      /* diff[data_count] */
    uint8_t *type;   /* type[lines + outliers] */
    nbuf *length;    /* length[lines + outliers]  */
    nbuf *dy;        /* dy[lines] */
    nbuf *dx;        /* dx[lines] */
    nbuf *a;         /* a[lines] */
    nbuf *outlier;   /* outlier[outliers] */
};

#define DATA_CONSTANT (1)
#define DATA_ZIPPED (2)
#define DATA_ABSENT (3)

#define FLAG_BITS (4)
#define FLAG_MASK ((1 << FLAG_BITS) - 1)

#define FLAG_BITS_TYPE      (0)
#define FLAG_BITS_DIFF      (FLAG_BITS + FLAG_BITS_TYPE)
#define FLAG_BITS_LENGTH    (FLAG_BITS + FLAG_BITS_DIFF)
#define FLAG_BITS_DY        (FLAG_BITS + FLAG_BITS_LENGTH)
#define FLAG_BITS_DX        (FLAG_BITS + FLAG_BITS_DY)
#define FLAG_BITS_A         (FLAG_BITS + FLAG_BITS_DX)
#define FLAG_BITS_OUTLIER   (FLAG_BITS + FLAG_BITS_A)

#define SET_TYPE_ABSENT(FLAGS)      ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_TYPE)
#define SET_DIFF_ABSENT(FLAGS)      ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_DIFF)
#define SET_LENGTH_ABSENT(FLAGS)    ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_LENGTH)
#define SET_DY_ABSENT(FLAGS)        ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_DY)
#define SET_DX_ABSENT(FLAGS)        ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_DX)
#define SET_A_ABSENT(FLAGS)         ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_A)
#define SET_OUTLIER_ABSENT(FLAGS)   ((FLAGS).data_flags) |= (DATA_ABSENT << FLAG_BITS_OUTLIER)

#define SET_TYPE_CONSTANT(FLAGS)    ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_TYPE)
#define SET_DIFF_CONSTANT(FLAGS)    ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_DIFF)
#define SET_LENGTH_CONSTANT(FLAGS)  ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_LENGTH)
#define SET_DY_CONSTANT(FLAGS)      ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_DY)
#define SET_DX_CONSTANT(FLAGS)      ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_DX)
#define SET_A_CONSTANT(FLAGS)       ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_A)
#define SET_OUTLIER_CONSTANT(FLAGS) ((FLAGS).data_flags) |= (DATA_CONSTANT << FLAG_BITS_OUTLIER)

#define SET_TYPE_ZIPPED(FLAGS)      ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_TYPE)
#define SET_DIFF_ZIPPED(FLAGS)      ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_DIFF)
#define SET_LENGTH_ZIPPED(FLAGS)    ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_LENGTH)
#define SET_DY_ZIPPED(FLAGS)        ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_DY)
#define SET_DX_ZIPPED(FLAGS)        ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_DX)
#define SET_A_ZIPPED(FLAGS)         ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_A)
#define SET_OUTLIER_ZIPPED(FLAGS)   ((FLAGS).data_flags) |= (DATA_ZIPPED << FLAG_BITS_OUTLIER)

#define FLAG_TYPE(O)                ((((O).data_flags) >> FLAG_BITS_TYPE   ) & FLAG_MASK)
#define FLAG_DIFF(O)                ((((O).data_flags) >> FLAG_BITS_DIFF   ) & FLAG_MASK)
#define FLAG_LENGTH(O)              ((((O).data_flags) >> FLAG_BITS_LENGTH ) & FLAG_MASK)
#define FLAG_DY(O)                  ((((O).data_flags) >> FLAG_BITS_DY     ) & FLAG_MASK)
#define FLAG_DX(O)                  ((((O).data_flags) >> FLAG_BITS_DX     ) & FLAG_MASK)
#define FLAG_A(O)                   ((((O).data_flags) >> FLAG_BITS_A      ) & FLAG_MASK)
#define FLAG_OUTLIER(O)             ((((O).data_flags) >> FLAG_BITS_OUTLIER) & FLAG_MASK)

struct encoded {
    uint8_t flags;   /* 0x1: zipped, 0x2: packed, 0x3: packed+zipped, 0x0: izipped */
    uint32_t data_count;
    union {
        struct {
            uint32_t data_size;
            const uint8_t *data;
        } zipped;
        struct {
            uint32_t data_size;
            const uint8_t *data;
            int64_t min;
        } packed;
        struct {
            uint32_t data_flags;
            uint32_t segments;
            uint32_t outliers;
            uint32_t type_size;
            uint32_t diff_size;
            uint32_t length_size;
            uint32_t dy_size;
            uint32_t dx_size;
            uint32_t a_size;
            uint32_t outlier_size;
            
            int64_t min_diff;
            int64_t min_length;
            int64_t min_dy;
            int64_t min_dx;
            int64_t min_a;
            int64_t min_outlier;
            
            const uint8_t *type;
            const uint8_t *diff;
            const uint8_t *length;
            const uint8_t *dy;
            const uint8_t *dx;
            const uint8_t *a;
            const uint8_t *outlier;
        } izipped;
    } u;
};

#if 1
#define DEBUG_PRINT(fmt, ...) { do { } while(0); }
#else
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "%s - " fmt "\n", __func__, __VA_ARGS__)
#endif

