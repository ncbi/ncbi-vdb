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
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>

#include <klib/rc.h>

#ifdef _WIN32
#include <intrin.h>
#ifndef __builtin_popcount
#define __builtin_popcount __popcnt
#endif
#endif


#ifndef min
#define min(x,y) ((y) < (x) ? (y) : (x))
#endif

#ifndef max
#define max(x,y) ((y) >= (x) ? (y) : (x))
#endif

#define max4(x1, x2, x3, x4) (max( max((x1),(x2)), max((x3),(x4)) ))

#define COMPARE_4NA 0
#define CACHE_MAX_ROWS 1 /* and columns as well */

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

static int compare_4na ( char ch2na, char ch4na )
{
    unsigned char bits4na = map_char_to_4na [(int)ch4na];
    unsigned char bits2na = map_char_to_4na [(int)ch2na];

    /*return (bits2na & bits4na) != 0 ? 2 : -1;*/

    unsigned char popcnt4na;
    popcnt4na = (unsigned char) __builtin_popcount ( bits4na );

    return (bits2na & bits4na) != 0 ? 12 / popcnt4na : -6;
}

static int similarity_func (char ch2na, char ch4na)
{
#if COMPARE_4NA == 1
    return compare_4na ( ch2na, ch4na );
#else
    return ch2na == ch4na ? 2 : -1;
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


static char get_char (char const* str, size_t size, size_t pos, int reverse)
{
    if ( !reverse )
        return str [pos];
    else
        return str [size - pos - 1];
}

static rc_t calculate_similarity_matrix (
    char const* text, size_t size_text,
    char const* query, size_t size_query,
    int* matrix, int reverse)
{

    size_t ROWS = size_text + 1;
    size_t COLUMNS = size_query + 1;
    size_t i, j;

    /* arrays to store maximums for all previous rows and columns */
#ifdef CACHE_MAX_ROWS

    ValueIndexPair* vec_max_cols = NULL;
    ValueIndexPair* vec_max_rows = NULL;

    vec_max_cols = malloc ( COLUMNS * sizeof (ValueIndexPair) );
    if ( vec_max_cols == NULL)
        return RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);

    vec_max_rows = malloc ( ROWS * sizeof (ValueIndexPair) );
    if ( vec_max_rows == NULL)
    {
        free (vec_max_cols);
        return RC(rcText, rcString, rcSearching, rcMemory, rcExhausted);
    }
    memset ( vec_max_cols, 0, COLUMNS * sizeof (ValueIndexPair) );
    memset ( vec_max_rows, 0, ROWS * sizeof (ValueIndexPair) );

#endif

    // init 1st row and column with zeros
    memset ( matrix, 0, COLUMNS * sizeof(matrix[0]) );
    for ( i = 1; i < ROWS; ++i )
        matrix [i * COLUMNS] = 0;

    for ( i = 1; i < ROWS; ++i )
    {
        for ( j = 1; j < COLUMNS; ++j )
        {
            int sim = similarity_func (
                            get_char (text, size_text, i-1, reverse),
                            get_char (query, size_query, j-1, reverse) );

#ifdef CACHE_MAX_ROWS
            int cur_score_del = vec_max_cols[j].value + gap_score_func(j - vec_max_cols[j].index);
#else
            int cur_score_del = -1;
            for ( size_t k = 1; k < i; ++k )
            {
                int cur = matrix [ (i - k)*COLUMNS + j ] + gap_score_func(k);
                if ( cur > cur_score_del )
                    cur_score_del = cur;
            }
#endif

#ifdef CACHE_MAX_ROWS
            int cur_score_ins = vec_max_rows[i].value + gap_score_func(i - vec_max_rows[i].index);;
#else
            int cur_score_ins = -1;
            for ( size_t l = 1; l < j; ++l )
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

#ifdef CACHE_MAX_ROWS
            if ( matrix[i*COLUMNS + j] > vec_max_cols[j].value )
            {
                vec_max_cols[j].value = matrix[i*COLUMNS + j];
                vec_max_cols[j].index = j;
            }

            vec_max_cols[j].value += gap_score_func(1);
#endif
#ifdef CACHE_MAX_ROWS
            if ( matrix[i*COLUMNS + j] > vec_max_rows[i].value )
            {
                vec_max_rows[i].value = matrix[i*COLUMNS + j];
                vec_max_rows[i].index = i;
            }

            vec_max_rows[i].value += gap_score_func(1);
#endif
        }
    }

#ifdef CACHE_MAX_ROWS
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
    query_start, query_len [OUT, NULL OK] - the region of ambiguity (the
                                            possible variation) on the query

*/
LIB_EXPORT rc_t CC FindRefVariationBounds (
    char const* ref_slice, size_t ref_slice_size,
    char const* query, size_t query_size,
    size_t* ref_start, size_t* ref_len,
    size_t* query_start, size_t* query_len
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

    /* forward scan */
    rc = calculate_similarity_matrix ( query, query_size, ref_slice, ref_slice_size, matrix, 0 );
    if ( rc != 0 )
        goto exit_search;
    sw_find_indel_box ( matrix, ROWS, COLUMNS, &row_start, &row_end, &col_start, &col_end );

    /* reverse scan */
    rc = calculate_similarity_matrix ( query, query_size, ref_slice, ref_slice_size, matrix, 1 );
    if ( rc != 0 )
        goto exit_search;
    sw_find_indel_box ( matrix, ROWS, COLUMNS, &row_start_rev, &row_end_rev, &col_start_rev, &col_end_rev );

    row_start = min ( (int)query_size - row_end_rev - 1, row_start );
    row_end   = max ( (int)query_size - row_start_rev - 1, row_end );
    col_start = min ( (int)ref_slice_size - col_end_rev - 1, col_start );
    col_end   = max ( (int)ref_slice_size - col_start_rev - 1, col_end );

    if ( ref_start != NULL )
        *ref_start = col_start + 1;
    if ( ref_len != NULL )
        *ref_len = col_end - col_start - 1;
    if ( query_start != NULL )
        *query_start = row_start + 1;
    if ( query_len != NULL )
        *query_len = row_end - row_start - 1;

exit_search:
    free (matrix);

    return rc;
}
