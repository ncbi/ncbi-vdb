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

#include <search/extern.h>

#include <klib/rc.h>
#include <klib/refcount.h>
#include <insdc/insdc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if WINDOWS
#include <intrin.h>
#ifndef __builtin_popcount
#define __builtin_popcount __popcnt
#endif
#endif

#include <sysalloc.h>


#ifndef min
#define min(x,y) ((y) < (x) ? (y) : (x))
#endif

#ifndef max
#define max(x,y) ((y) >= (x) ? (y) : (x))
#endif

#define max4(x1, x2, x3, x4) (max( max((x1),(x2)), max((x3),(x4)) ))

#define COMPARE_4NA 0
#define CACHE_MAX_ROWS 1 /* and columns as well */

typedef struct VRefVariation
{
    KRefcount refcount;

    INSDC_dna_text const* ref_external; /* pointer to external buffer */
    size_t ref_size;

    INSDC_dna_text* variation;
    size_t var_start;
    size_t var_size;
    size_t var_len_on_ref;
} VRefVariation;


#if COMPARE_4NA == 1

unsigned char const map_char_to_4na [256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1,14, 2,13, 0, 0, 4,11, 0, 0,12, 0, 3,15, 0,
    0, 0, 5, 6, 8, 0, 7, 9, 0,10, 0, 0, 0, 0, 0, 0,
    0, 1,14, 2,13, 0, 0, 4,11, 0, 0,12, 0, 3,15, 0,
    0, 0, 5, 6, 8, 0, 7, 9, 0,10, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int compare_4na ( INSDC_dna_text ch2na, INSDC_dna_text ch4na )
{
    unsigned char bits4na = map_char_to_4na [(unsigned char)ch4na];
    unsigned char bits2na = map_char_to_4na [(unsigned char)ch2na];

    /*return (bits2na & bits4na) != 0 ? 2 : -1;*/

    unsigned char popcnt4na = (unsigned char) __builtin_popcount ( bits4na );

    return (bits2na & bits4na) != 0 ? 12 / popcnt4na : -6;
}
#endif

static int similarity_func (INSDC_dna_text ch2na, INSDC_dna_text ch4na)
{
#if COMPARE_4NA == 1
    return compare_4na ( ch2na, ch4na );
#else
    return tolower(ch2na) == tolower(ch4na) ? 2 : -1;
#endif
}

static int gap_score_func ( size_t idx )
{
#if COMPARE_4NA == 1
    return -6*(int)idx;
#else
    return -(int)idx;
#endif
}

typedef struct ValueIndexPair
{
    size_t index;
    int value;
} ValueIndexPair;


static char get_char (INSDC_dna_text const* str, size_t size, size_t pos, bool reverse)
{
    if ( !reverse )
        return str [pos];
    else
        return str [size - pos - 1];
}

static rc_t calculate_similarity_matrix (
    INSDC_dna_text const* text, size_t size_text,
    INSDC_dna_text const* query, size_t size_query,
    int* matrix, bool reverse)
{

    size_t ROWS = size_text + 1;
    size_t COLUMNS = size_query + 1;
    size_t i, j;

    /* arrays to store maximums for all previous rows and columns */
#if CACHE_MAX_ROWS != 0

    ValueIndexPair* vec_max_cols = NULL;
    ValueIndexPair* vec_max_rows = NULL;

    vec_max_cols = calloc ( COLUMNS, sizeof vec_max_cols [ 0 ] );
    if ( vec_max_cols == NULL)
        return RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);

    vec_max_rows = calloc ( ROWS, sizeof vec_max_rows [ 0 ] );
    if ( vec_max_rows == NULL)
    {
        free (vec_max_cols);
        return RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);
    }

#endif

    // init 1st row and column with zeros
    memset ( matrix, 0, COLUMNS * sizeof(matrix[0]) );
    for ( i = 1; i < ROWS; ++i )
        matrix [i * COLUMNS] = 0;

    for ( i = 1; i < ROWS; ++i )
    {
        for ( j = 1; j < COLUMNS; ++j )
        {
#if CACHE_MAX_ROWS == 0
            size_t k, l;
#endif
            int cur_score_del, cur_score_ins;
            int sim = similarity_func (
                            get_char (text, size_text, i-1, reverse),
                            get_char (query, size_query, j-1, reverse) );

#if CACHE_MAX_ROWS != 0
            cur_score_del = vec_max_cols[j].value + gap_score_func(j - vec_max_cols[j].index);
#else
            cur_score_del = -1;
            for ( k = 1; k < i; ++k )
            {
                int cur = matrix [ (i - k)*COLUMNS + j ] + gap_score_func(k);
                if ( cur > cur_score_del )
                    cur_score_del = cur;
            }
#endif

#if CACHE_MAX_ROWS != 0
            cur_score_ins = vec_max_rows[i].value + gap_score_func(i - vec_max_rows[i].index);;
#else
            
            cur_score_ins = -1;
            for ( l = 1; l < j; ++l )
            {
                int cur = matrix [ i*COLUMNS + (j - l) ] + gap_score_func(l);
                if ( cur > cur_score_ins )
                    cur_score_ins = cur;
            }
#endif

            matrix[i*COLUMNS + j] = max4 (
                        0,
                        matrix[(i-1)*COLUMNS + j - 1] + sim,
                        cur_score_del,
                        cur_score_ins);

#if CACHE_MAX_ROWS != 0
            if ( matrix[i*COLUMNS + j] > vec_max_cols[j].value )
            {
                vec_max_cols[j].value = matrix[i*COLUMNS + j];
                vec_max_cols[j].index = j;
            }

            vec_max_cols[j].value += gap_score_func(1);
#endif
#if CACHE_MAX_ROWS != 0
            if ( matrix[i*COLUMNS + j] > vec_max_rows[i].value )
            {
                vec_max_rows[i].value = matrix[i*COLUMNS + j];
                vec_max_rows[i].index = i;
            }

            vec_max_rows[i].value += gap_score_func(1);
#endif
        }
    }

#if CACHE_MAX_ROWS != 0
    free (vec_max_cols);
    free (vec_max_rows);
#endif

    return 0;
}

static void sw_find_indel_box ( int* matrix, size_t ROWS, size_t COLUMNS,
    int* ret_row_start, int* ret_row_end,
    int* ret_col_start, int* ret_col_end )
{
    /* find maximum score in the matrix */
    size_t max_row = 0, max_col = 0;
    size_t max_i = 0;

    size_t i = ROWS*COLUMNS - 1, j;
    int prev_indel = 0;
    /*do
    {
        if ( matrix[i] > matrix[max_i] )
            max_i = i;
        --i;
    }
    while (i > 0);*/

    /* TODO: prove the lemma: for all i: matrix[i] <= matrix[ROWS*COLUMNS - 1]
    (i.e. matrix[ROWS*COLUMNS - 1] is always the maximum element in the valid SW-matrix)*/

    max_i = ROWS*COLUMNS - 1;

    max_row = max_i / COLUMNS;
    max_col = max_i % COLUMNS;


    // traceback to (0,0)-th element of the matrix
    *ret_row_start = *ret_row_end = *ret_col_start = *ret_col_end = -1;

    i = max_row;
    j = max_col;
    while (1)
    {
        if (i > 0 && j > 0)
        {
            if ( matrix [(i - 1)*COLUMNS + (j - 1)] >= matrix [i*COLUMNS + (j - 1)] &&
                matrix [(i - 1)*COLUMNS + (j - 1)] >= matrix [(i - 1)*COLUMNS + j])
            {
                --i;
                --j;

                if (prev_indel)
                {
                    *ret_row_start = (int)i;
                    *ret_col_start = (int)j;
                }
                prev_indel = 0;
            }
            else if ( matrix [(i - 1)*COLUMNS + (j - 1)] < matrix [i*COLUMNS + (j - 1)] )
            {
                if ( *ret_row_end == -1 )
                {
                    *ret_row_end = (int)i;
                    *ret_col_end = (int)j;
                }
                --j;
                prev_indel = 1;
            }
            else
            {
                if ( *ret_row_end == -1 )
                {
                    *ret_row_end = (int)i;
                    *ret_col_end = (int)j;
                }
                --i;
                prev_indel = 1;
            }
        }
        else if ( i > 0 )
        {
            if ( *ret_row_end == -1 )
            {
                *ret_row_end = (int)i;
                *ret_col_end = 0;
            }
            *ret_row_start = 0;
            *ret_col_start = 0;
            break;
        }
        else if ( j > 0 )
        {
            if ( *ret_row_end == -1 )
            {
                *ret_row_end = 0;
                *ret_col_end = (int)j;
            }
            *ret_row_start = 0;
            *ret_col_start = 0;
            break;
        }
        else
        {
            break;
        }
    }
}

#if 0 /* leaving it here for debug*/
template <bool reverse> void print_matrix ( int const* matrix,
                                            char const* ref_slice, size_t ref_slice_size,
                                            char const* query, size_t query_size)
{
    size_t COLUMNS = ref_slice_size + 1;
    size_t ROWS = query_size + 1;

    int print_width = 2;

    CStringIterator<reverse> ref_slice_iterator(ref_slice, ref_slice_size);
    CStringIterator<reverse> query_iterator(query, query_size);

    printf ("  %*c ", print_width, '-');
    for (size_t j = 1; j < COLUMNS; ++j)
        printf ("%*c ", print_width, ref_slice_iterator[j-1]);
    printf ("\n");

    for (size_t i = 0; i < ROWS; ++i)
    {
        if ( i == 0 )
            printf ("%c ", '-');
        else
            printf ("%c ", query_iterator[i-1]);
    
        for (size_t j = 0; j < COLUMNS; ++j)
        {
            printf ("%*d ", print_width, matrix[i*COLUMNS + j]);
        }
        printf ("\n");
    }
}
#endif


/*
    FindRefVariationBounds uses Smith-Waterman algorithm
    to find theoretical bounds of the variation for
    the given reference slice and the query (properly preapared,
    i.e. containing sequences of bases at the beginning and
    the end matching the reference)

    ref_slice, ref_slice_size [IN] - the reference slice on which the
                                     variation will be looked for
    query, query_size [IN] - the query that represents the variation placed
                             inside the reference slice
    ref_start, ref_len [OUT, NULL OK] - the region of ambiguity on the reference
    have_indel [OUT] - pointer to flag indication if there is an insertion or deletion
                       (1 - there is an indel, 0 - there is match/mismatch only)
*/
static rc_t FindRefVariationBounds (
    INSDC_dna_text const* ref_slice, size_t ref_slice_size,
    INSDC_dna_text const* query, size_t query_size,
    size_t* ref_start, size_t* ref_len, bool * has_indel
    )
{
    /* building sw-matrix for chosen reference slice and sequence */

    size_t COLUMNS = ref_slice_size + 1;
    size_t ROWS = query_size + 1;
    rc_t rc = 0;

    int row_start, col_start, row_end, col_end;
    int row_start_rev, col_start_rev, row_end_rev, col_end_rev;
    int* matrix = malloc( ROWS * COLUMNS * sizeof (int) );
    if (matrix == NULL)
        return RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);
    * has_indel = true;

    /* forward scan */
    rc = calculate_similarity_matrix ( query, query_size, ref_slice, ref_slice_size, matrix, false );
    if ( rc != 0 )
        goto free_resources;
    sw_find_indel_box ( matrix, ROWS, COLUMNS, &row_start, &row_end, &col_start, &col_end );
    if ( row_start == -1 && row_end == -1 && col_start == -1 && col_end == -1 )
    {
        * has_indel = 0;
        goto free_resources;
    }

    /* reverse scan */
    rc = calculate_similarity_matrix ( query, query_size, ref_slice, ref_slice_size, matrix, true );
    if ( rc != 0 )
        goto free_resources;
    sw_find_indel_box ( matrix, ROWS, COLUMNS, &row_start_rev, &row_end_rev, &col_start_rev, &col_end_rev );

    row_start = min ( (int)query_size - row_end_rev - 1, row_start );
    row_end   = max ( (int)query_size - row_start_rev - 1, row_end );
    col_start = min ( (int)ref_slice_size - col_end_rev - 1, col_start );
    col_end   = max ( (int)ref_slice_size - col_start_rev - 1, col_end );

    if ( ref_start != NULL )
        *ref_start = col_start + 1;
    if ( ref_len != NULL )
        *ref_len = col_end - col_start - 1;

free_resources:
    free (matrix);

    return rc;
}

/****************************************************/
/* yet another string helper */
typedef struct c_string_const
{
    char const* str;
    size_t size;
} c_string_const;

static void c_string_const_assign ( c_string_const* self, char const* src, size_t size )
{
    self -> str = src;
    self -> size = size;
}

typedef struct c_string
{
    char* str;
    size_t size;
    size_t capacity;
} c_string;

static int c_string_make ( c_string* self, size_t capacity )
{
    assert ( capacity > 0 );
    self -> str = malloc (capacity + 1);
    if ( self -> str != NULL )
    {
        self -> str [0] = '\0';
        self -> size = 0;
        self -> capacity = capacity;
        return 1;
    }
    else
        return 0;
}

static void c_string_destruct ( c_string* self )
{
    if ( self->str != NULL )
    {
        free ( self -> str );
        self -> str = NULL;
        self -> size = 0;
        self -> capacity = 0;
    }
}

static int c_string_realloc_no_preserve ( c_string* self, size_t new_capacity )
{
    if ( self -> capacity < new_capacity )
    {
        c_string_destruct ( self );

        return c_string_make ( self, new_capacity );
    }
    else
    {
        self -> str [0] = '\0';
        self -> size = 0;
    }

    return 1;
}

static int c_string_assign ( c_string* self, char const* src, size_t src_size )
{
    assert ( self->capacity >= src_size );
    if ( self->capacity < src_size && !c_string_realloc_no_preserve (self, max(self->capacity * 2, src_size)) )
        return 0;

    memcpy ( self -> str, src, src_size );
    self -> str [src_size] = '\0';
    self -> size = src_size;

    return 1;
}

static int c_string_append ( c_string* self, char const* append, size_t append_size)
{
    if ( append_size != 0 )
    {
        size_t new_size = self->size + append_size;
        if ( self->capacity >= new_size )
        {
            memcpy ( self->str + self->size, append, append_size );
            self->size = new_size;
            self->str [new_size] = '\0';
        }
        else
        {
            size_t new_capacity = max (new_size + 1, self->capacity * 2);
            char* new_str = malloc ( new_capacity );
            if (new_str == NULL)
                return 0;

            memcpy (new_str, self->str, self->size);
            memcpy (new_str + self->size, append, append_size );
            new_str [ new_size ] = '\0';

            c_string_destruct ( self );
        
            self->str = new_str;
            self->size = new_size;
            self->capacity = new_capacity;
        }
    }

    return 1;
}

#if 0
static int c_string_wrap ( c_string* self,
    char const* prefix, size_t prefix_size,
    char const* postfix, size_t postfix_size)
{
    assert ( self -> str != NULL );
    size_t new_size = self->size + prefix_size + postfix_size;

    if ( new_size > self->capacity )
    {
        size_t new_capacity = max (new_size + 1, self->capacity * 2);
        char* new_str = malloc ( new_capacity );
        if (new_str == NULL)
            return 0;

        memcpy ( new_str, prefix, prefix_size );
        memcpy ( new_str + prefix_size, self -> str, self -> size );
        memcpy ( new_str + prefix_size + self->size, postfix, postfix_size );
        new_str [ new_size ] = '\0';

        c_string_destruct ( self );
        
        self->str = new_str;
        self->size = new_size;
        self->capacity = new_capacity;
    }
    else
    {
        memmove ( self->str + prefix_size, self->str, self->size );
        memcpy ( self->str, prefix, prefix_size );
        memcpy (self->str + prefix_size + self->size, postfix, postfix_size );
        self->str [new_size] = '\0';
    }
    return 1;
}
#endif
/************************************************/


/*
   returns true if a new ref_slice is selected
   returns false if the new ref_slice is the same as the previous one passed in ref_slice
*/
static bool get_ref_slice (
            INSDC_dna_text const* ref, size_t ref_size, size_t ref_pos_var,
            size_t var_len_on_ref,
            size_t slice_expand_left, size_t slice_expand_right,
            c_string_const* ref_slice)
{
    size_t ref_start, ref_xend;
    if ( ref_pos_var < slice_expand_left )
        ref_start = 0;
    else
        ref_start = ref_pos_var - slice_expand_left;

    if ( ref_pos_var + slice_expand_right + var_len_on_ref >= ref_size )
        ref_xend = ref_size;
    else
        ref_xend = ref_pos_var + slice_expand_right + var_len_on_ref;

    if ( ref_slice->str == ref + ref_start && ref_slice->size == ref_xend - ref_start)
        return false;

    c_string_const_assign ( ref_slice, ref + ref_start, ref_xend - ref_start );
    return true;
}

#if 1
static bool make_query ( c_string_const const* ref_slice,
        INSDC_dna_text const* variation, size_t variation_size, size_t var_len_on_ref,
        int64_t var_start_pos_adj, /* ref_pos adjusted to the beginning of ref_slice (in the simplest case - the middle of ref_slice) */
        c_string* query
    )
{
    if ( !c_string_realloc_no_preserve (query, variation_size + ref_slice->size - var_len_on_ref) )
        return false;

    if ( !c_string_append (query, ref_slice->str, var_start_pos_adj) ||
         !c_string_append (query, variation, variation_size) ||
         !c_string_append (query, ref_slice->str + var_start_pos_adj + var_len_on_ref, ref_slice->size - var_start_pos_adj - var_len_on_ref) )
    {
         return false;
    }

    return true;
}

static bool compose_variation ( c_string_const const* ref,
        size_t ref_start, size_t ref_len,
        INSDC_dna_text const* query, size_t query_len,
        int64_t ref_pos_var, size_t var_len_on_ref,
        c_string* variation )
{
    INSDC_dna_text const* query_adj = query;
    size_t query_len_adj = query_len;
    bool ret = true;

    /* TODO: not always correct */
    if ( !c_string_realloc_no_preserve( variation, ref_len + query_len - var_len_on_ref))
        return false;

    if ( (size_t)ref_pos_var > ref_start )
    {
        /* if extended window starts to the left from initial reported variation start
           then include preceding bases into adjusted variation */
        ret = ret && c_string_assign ( variation, ref->str + ref_start, (size_t)ref_pos_var - ref_start );
    }
    else if ( (size_t)ref_pos_var < ref_start )
    {
        /* the real window of ambiguity actually starts to the right from
           the reported variation start
           let's not to include the left unambigous part into
           adjusted variation (?) */

        query_adj += ref_start - ref_pos_var;
        query_len_adj -= ref_start - ref_pos_var;
    }

    if ( query_len_adj > 0 )
    {
        assert ( variation->capacity >= variation->size + query_len_adj );
        ret = ret && c_string_append ( variation, query_adj, query_len_adj );
    }

    if ( (int64_t)(ref_len - ((size_t)ref_pos_var - ref_start) - var_len_on_ref) > 0 )
    {
        assert ( variation->capacity >= variation->size + ref_len - ((size_t)ref_pos_var - ref_start) - var_len_on_ref );
        ret = ret && c_string_append ( variation, ref->str + (size_t)ref_pos_var + var_len_on_ref,
            ref_len - ((size_t)ref_pos_var - ref_start) - var_len_on_ref );
    }

    if ( ! ret )
        c_string_destruct ( variation );

    return ret;
}

#endif

#if 0
static bool make_query_ (
        INSDC_dna_text const* ref, size_t ref_size, size_t ref_pos_var,
        INSDC_dna_text const* variation, size_t variation_size, size_t var_len_on_ref,
        size_t slice_expand_left, size_t slice_expand_right,
        c_string* query
    )
{
    size_t ref_prefix_start, ref_prefix_len, ref_suffix_start, ref_suffix_len;
    if ( !c_string_realloc_no_preserve (query, variation_size + slice_expand_left + slice_expand_right + var_len_on_ref) )
        return false;

    if ( ref_pos_var < slice_expand_left )
    {
        ref_prefix_start = 0;
        ref_prefix_len = slice_expand_left - (ref_pos_var - 1);
    }
    else
    {
        ref_prefix_start = ref_pos_var - slice_expand_left;
        ref_prefix_len = slice_expand_left;
    }

    ref_suffix_start = ref_pos_var + var_len_on_ref;

    if ( ref_suffix_start + slice_expand_right >= ref_size )
        ref_suffix_len = ref_size - (slice_expand_right + 1);
    else
        ref_suffix_len = slice_expand_right;

    if ( !c_string_append (query, ref + ref_prefix_start, ref_prefix_len) ||
         !c_string_append (query, variation, variation_size) ||
         !c_string_append (query, ref + ref_suffix_start, ref_suffix_len) )
    {
         return false;
    }

    return true;
}
#endif

/*
    FindRefVariationRegionIUPAC uses Smith-Waterman algorithm
    to find theoretical bounds of the variation for
    the given reference, position on the reference
    and the raw query, or variation to look for at the given
    reference position

    ref, ref_size [IN]     - the reference on which the
                             variation will be looked for
    ref_pos_var [IN]       - the position on reference to look for the variation
    variation, variation_size [IN] - the variation to look for at the ref_pos_var
    var_len_on_ref [IN]    - the length of the variation on the reference, e.g.:
                           - mismatch, 2 bases: variation = "XY", var_len_on_ref = 2
                           - deletion, 3 bases: variation = "", var_len_on_ref = 3
                           - insertion, 2 bases:  variation = "XY", var_len_on_ref = 0

    p_ref_start, p_ref_len [OUT, NULL OK] - the region of ambiguity on the reference
                                            (return values)
*/

#if 0
#include <stdio.h>
#endif
LIB_EXPORT rc_t CC FindRefVariationRegionIUPAC (
        INSDC_dna_text const* ref, size_t ref_size, size_t ref_pos_var,
        INSDC_dna_text const* variation, size_t variation_size, size_t var_len_on_ref,
        size_t* p_ref_start, size_t* p_ref_len
    )
{
    rc_t rc = 0;

    size_t var_half_len = variation_size / 2 + 1;

    size_t exp_l = var_half_len;
    size_t exp_r = var_half_len;

    /* previous start and end for reference slice */
    int64_t slice_start = -1, slice_end = -1;

    c_string_const ref_slice;
    c_string query;

    size_t ref_start = 0, ref_len = 0;

    ref_slice.str = NULL;
    ref_slice.size = 0;

    if ( !c_string_make ( & query, ( variation_size + 1 ) * 2 ) )
        return RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);

    while ( 1 )
    {
        int64_t new_slice_start, new_slice_end;
        int64_t ref_pos_adj;
        int cont = 0;
        bool has_indel;

        /* get new expanded slice and check if it has not reached the bounds of ref */
        bool slice_expanded = get_ref_slice ( ref, ref_size, ref_pos_var, var_len_on_ref, exp_l, exp_r, & ref_slice );
        if ( !slice_expanded )
            break;

        /* get ref_pos relative to ref_slice start and new slice_start and end */
        ref_pos_adj = (int64_t)ref_pos_var - ( ref_slice.str - ref );
        new_slice_start = ref_slice.str - ref;
        new_slice_end = new_slice_start + ref_slice.size;

        /* compose a new query for newly extended ref slice */
        /*if ( !make_query_( ref, ref_size, ref_pos_var, variation, variation_size, var_len_on_ref, exp_l, exp_r, & query ) )*/
        if ( !make_query ( & ref_slice, variation, variation_size, var_len_on_ref, ref_pos_adj, & query ) )
        {
            rc = RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);
            goto free_resources;
        }

        rc = FindRefVariationBounds ( ref_slice.str, ref_slice.size,
                        query.str, query.size, & ref_start, & ref_len, & has_indel );

        /* if there are no indels report that there is no ambiguity
           for the given ref_pos_var: region starting at ref_pos_var has length = 0
           ambiguity
        */
        if ( !has_indel )
        {
            ref_start = ref_pos_adj;
            ref_len = 0;
        }

#if 0
        printf ("ref_slice: %.*s, query: %.*s, ref_start=%lu, ref_len=%lu%s\n",
            (int)ref_slice.size, ref_slice.str, (int)query.size, query.str,
            ref_start, ref_len, has_indel != 0 ? "" : " (no indels)");
#endif

        if ( rc != 0 )
            goto free_resources;

        /* if we've found the ambiguity window starting at the very
           beginning of the ref slice and if we're still able to extend
           ref slice to the left (haven't bumped into boundary) - extend to the left
           and repeat the search
        */
        if ( ref_start == 0 && (slice_start == -1 || new_slice_start != slice_start ) )
        {
            exp_l *= 2;
            cont = 1;
        }

        /* if we've found the ambiguity window ending at the very
           end of the ref slice and if we're still able to extend
           ref slice to the right (haven't bumped into boundary) - extend to the right
           and repeat the search
        */
        if ( ref_start + ref_len == ref_slice.size && (slice_end == -1 || new_slice_end != slice_end) )
        {
            exp_r *= 2;
            cont = 1;
        }

        if ( !cont )
            break;

        slice_start = new_slice_start;
        slice_end = new_slice_end;
    }
    if ( p_ref_start != NULL )
        *p_ref_start = ref_start + (ref_slice.str - ref);
    if ( p_ref_len != NULL )
        *p_ref_len = ref_len;

free_resources:
    c_string_destruct ( &query );

    return rc;
}

LIB_EXPORT rc_t CC VRefVariationIUPACMake (
        VRefVariation** self,
        INSDC_dna_text const* ref, size_t ref_size, size_t ref_pos_var,
        INSDC_dna_text const* variation, size_t variation_size, size_t var_len_on_ref
    )
{
    struct VRefVariation* obj;
    rc_t rc = 0;

    assert ( self != NULL );

    obj = calloc ( 1, sizeof * obj );

    if ( obj == NULL )
    {
        rc = RC ( rcVDB, rcExpression, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        size_t ref_start, ref_len;
        rc = FindRefVariationRegionIUPAC ( ref, ref_size,
                                           ref_pos_var,
                                           variation, variation_size, var_len_on_ref,
                                           & ref_start, & ref_len );
        if ( rc != 0 )
        {
            free ( obj );
            obj = NULL;
        }
        else
        {
            c_string_const ref_str;
            
            c_string var_str;
            var_str.capacity = var_str.size = 0;
            var_str.str = NULL;

            c_string_const_assign ( & ref_str, ref, ref_size );

            if ( ! compose_variation ( & ref_str,
                                       ref_start, ref_len,
                                       variation, variation_size,
                                       ref_pos_var, var_len_on_ref, & var_str ) )
            {
                rc = RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);
                free ( obj );
                obj = NULL;
            }
            else
            {
                KRefcountInit ( & obj->refcount, 1, "VRefVariation", "make", "ref-var" );
                /* moving var_str to the object (so no need to destruct var_str */
                obj->variation = var_str.str;
                obj->var_size = var_str.size;

                obj->ref_external = ref;
                obj->ref_size = ref_size;
                obj->var_start = ref_start;
                assert( ref_len == var_str.size + var_len_on_ref );
                obj->var_len_on_ref = var_len_on_ref;
            }
        }
    }

    * self = obj;

    /* TODO: if Kurt insists, return non-zero rc if var_start == 0 or var_start + var_len == ref_size */
    return rc;
}


LIB_EXPORT rc_t CC VRefVariationIUPACAddRef ( VRefVariation const* self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VRefVariation" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcExpression, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC VRefVariationIUPACWhack ( VRefVariation* self )
{
    KRefcountWhack ( & self -> refcount, "VRefVariation" );

    assert ( self->variation != NULL );
    free ( self->variation );

    memset ( self, 0, sizeof * self );

    free ( self );

    return 0;
}

LIB_EXPORT rc_t CC VRefVariationIUPACRelease ( VRefVariation const* self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VRefVariation" ) )
        {
        case krefWhack:
            return VRefVariationIUPACWhack ( ( VRefVariation* ) self );
        case krefNegative:
            return RC ( rcVDB, rcExpression, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT INSDC_dna_text const* CC VRefVariationIUPACGetVariation ( VRefVariation const* self )
{
    assert ( self != NULL );
    return self->variation;
}

LIB_EXPORT size_t CC VRefVariationIUPACGetVarStart ( VRefVariation const* self )
{
    assert ( self != NULL );
    return self->var_start;
}

LIB_EXPORT size_t CC VRefVariationIUPACGetVarSize ( VRefVariation const* self )
{
    assert ( self != NULL );
    return self->var_size;
}

LIB_EXPORT size_t CC VRefVariationIUPACGetVarLenOnRef ( VRefVariation const* self )
{
    assert ( self != NULL );
    return self->var_len_on_ref;
}
