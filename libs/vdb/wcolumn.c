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

#include "klib/symbol.h"
#include "column-priv.h"
#include "dbmgr-priv.h"
#include "schema-priv.h"
#include "schema-expr.h"
#include "schema-parse.h"
#include "cursor-priv.h"
#include "prod-priv.h"
#include "blob-priv.h"
#include "page-map.h"

#include <vdb/manager.h>
#include <vdb/cursor.h>
#include <kdb/column.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* these constants MUST each be a power of two */
#define INITIAL_DATA_PEAK ( 32 * 1024 )
#define SINGLE_PAGE_SIZE ( 4 * 1024 )

/* this is a fairly high value, useful for writing single tables
   of data that are generally accessed serially. it's bad for random. */
#define DFLT_TRIGGER ( 2 * 1024 * 1024 )

/* debugging aid - Row-wise RunLengthEncoding ( repeated column
   elimination ) can be disabled to check for errors it may introduce. */
#define USE_RLE 1

/* when defined, use as an additional condition for detecting
   auto-commit cutoff points, with a limit in kilo-rows given below */
#define ROW_COUNT_CUTOFFS 128

#if ROW_COUNT_CUTOFFS
#define MAX_ROW_COUNT ( ROW_COUNT_CUTOFFS * 1024 )
#else
#define MAX_ROW_COUNT 0x40000000
#endif


/*--------------------------------------------------------------------------
 * VColumn
 */

static
void WColumnDestroy ( WColumn * self )
{
#if PROD_REFCOUNT && ! PROD_ALL_IN_CURSOR
    PROD_TRACK_REFCOUNT ( VProductionRelease, self -> out );
    VProductionRelease ( self -> out, NULL );
    PROD_TRACK_REFCOUNT ( VProductionRelease, self -> val );
    VProductionRelease ( self -> val, NULL );
#endif
}


/* Whack
 *  perform read-only cleanup
 */
void CC VColumnWhack ( void *item, void *data )
{
    VColumn *self = item;
    VCursor *curs = data;

    /* remove from cursor */
    if ( curs != NULL )
    {
        VectorSwap ( VCursorGetRow ( curs ), self -> ord, NULL, & item );
        VCursorCacheSwap ( VCursorColumns ( curs ), & self -> scol -> cid, NULL, & item );
    }

    if ( ! self -> read_only )
    {
        WColumn *wself = ( WColumn* ) self;

        if ( wself -> page != NULL )
        {
            TRACK_BLOB (VBlobRelease, wself->page);
            VBlobRelease ( wself -> page );
        }

        KDataBufferWhack ( & wself -> dflt );
        KDataBufferWhack ( & wself -> data );
        KDataBufferWhack ( & wself -> rowmap );
        WColumnDestroy ( wself );

    }

    VColumnDestroy ( self );
    free ( self );
}


/* Make - PRIVATE
 *  make a write column
 */
rc_t WColumnMake ( VColumn **colp, const VSchema *schema, const SColumn *scol,
    const SExpression *blob_limit, VDBManager *mgr, Vector *cx_bind )
{
    rc_t rc;
    WColumn *col;

    assert ( colp != NULL );
    assert ( schema != NULL );
    assert ( scol != NULL );
    assert ( mgr != NULL );

    col = calloc ( 1, sizeof * col );
    if ( col == NULL )
        rc = RC ( rcVDB, rcColumn, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = VColumnInit ( & col -> dad, schema, scol );
        if ( rc == 0 )
        {
            uint32_t i;

            /* initialize peak histories */
            for ( i = 0; i < sizeof col -> data_peak_hist / sizeof col -> data_peak_hist [ 0 ]; ++ i )
                col -> data_peak_hist [ i ] = INITIAL_DATA_PEAK;

            /* initial current peak */
            col -> data_peak = INITIAL_DATA_PEAK;

            /* use column-specific expression if provided */
            if ( scol -> limit != NULL )
                blob_limit = scol -> limit;

            /* evaluate blob cutoff limit expression */
            if ( blob_limit == NULL )
            {
#ifdef DFLT_TRIGGER
                /* use define */
                col -> trigger = DFLT_TRIGGER;
#else
                /* produce max unsigned integer */
                -- col -> trigger;
#endif
            }
            else
            {
                /* evaluate column blob limit */
                uint64_t trigger;
                rc = eval_uint64_expr ( schema, blob_limit, & trigger, cx_bind );
                col -> trigger = ( size_t ) trigger;
            }

            if ( rc == 0 )
            {
                * colp = & col -> dad;
                return 0;
            }
        }

        free ( col );
    }

    * colp = NULL;
    return rc;
}


/* IdRange
 *  returns id range for column or page
 */
rc_t VColumnIdRange ( const VColumn *vcol, int64_t *first, int64_t *last )
{
    rc_t rc;
    const WColumn *self = ( const WColumn* ) vcol;

    assert ( self != NULL );
    assert ( first != NULL && last != NULL );

    if ( self -> dad . in != NULL )
        return VColumnIdRangeRead ( & self -> dad, first, last );

    if ( self -> val == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcRange, rcNotOpen );
    else
    {
        /* a little silly, but set max range in 64-bit
           without complaints from 32-bit compilers */
        * first = 1;
        * first <<= 63;
        * last = ~ * first;

        /* now intersect this range with all physical sources */
        rc = VProductionColumnIdRange ( self -> val, first, last );
        if ( rc == 0 )
            return 0;
    }

    * first = * last = 0;

    return rc;
}

/* SetDefault
 *  capture default row data
 */
rc_t WColumnSetDefault ( VColumn *vcol,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t len )
{
    rc_t rc;
    bitsz_t elem_size, to_copy;
    WColumn *self = ( WColumn* ) vcol;

    assert ( elem_bits != 0 );
    assert ( buffer != NULL || ( boff == 0 && len == 0 ) );

    /* test "compatibility" of elem_bits
       this is used to interpret "len" */
    elem_size = VTypedescSizeof ( & self -> dad . desc );
    if ( elem_bits < elem_size && elem_size % elem_bits != 0 )
        return RC ( rcVDB, rcColumn, rcUpdating, rcType, rcInconsistent );
    if ( elem_bits > elem_size && elem_bits % elem_size != 0 )
        return RC ( rcVDB, rcColumn, rcUpdating, rcType, rcInconsistent );

    /* forget about prior value */
    KDataBufferWhack ( & self -> dflt );
    memset ( & self -> dflt, 0, sizeof self -> dflt );
    self -> have_dflt = false;
    self -> dflt_last = false;

    /* set the element size */
    rc = KDataBufferCast ( & self -> dflt, & self -> dflt, elem_bits, false );
    if ( rc != 0 )
        return rc;

    /* allow NULL setting */
    if ( buffer == NULL )
    {
        self -> have_dflt = true;
        return 0;
    }

    /* set the length */
    rc = KDataBufferResize ( & self -> dflt, len );
    if ( rc != 0 )
    {
        assert ( KDataBufferWritable ( & self -> dflt ) );
        return rc;
    }

    /* copy in the row */
    to_copy = len * elem_bits;
    if ( ( ( boff | to_copy ) & 7 ) != 0 )
        bitcpy ( self -> dflt . base, 0, buffer, boff, to_copy );
    else
        memmove ( self -> dflt . base, & ( ( const uint8_t* ) buffer ) [ boff >> 3 ], to_copy >> 3 );

    self -> have_dflt = true;
    return 0;
}


/* OpenRow
 *  update state
 *
 *  "const_row_id" [ IN, CONST ] - id of row being opened. useful
 *  only on initial open when no other rows are buffered.
 */
void CC WColumnOpenRow ( void *item, void *const_row_id )
{
    WColumn *self = item;
    if ( self != NULL )
    {
        int64_t row_id = * ( const int64_t* ) const_row_id;

        assert ( ! self -> row_written );
        if ( self -> start_id != self -> end_id )
        {
            assert ( row_id == self -> end_id );
        }
        else
        {
            /* capture row id */
            self -> start_id = self -> end_id = self -> cutoff_id = row_id;
            assert ( self -> bits_in_buffer == 0 );
            assert ( self -> row_len == 0 );
            assert ( self -> num_rows == 0 );
            assert ( self -> dflt_last == false );
        }
    }
}


/* Write
 */
rc_t WColumnWrite ( VColumn *cself,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t len )
{
    WColumn *self = ( WColumn* ) cself;

    bitsz_t elem_size, num_bits, data_bits, doff;

    assert ( elem_bits != 0 );
    assert ( buffer != NULL || ( boff == 0 && len == 0 ) );

    /* the number of bits to write */
    num_bits = ( bitsz_t ) elem_bits * len;

    /* test "compatibility" of elem_bits
       this is used to interpret "len" */
    elem_size = VTypedescSizeof ( & self -> dad . desc );
    if ( elem_bits != elem_size )
    {
        if ( elem_bits < elem_size && elem_size % elem_bits != 0 )
            return RC ( rcVDB, rcColumn, rcUpdating, rcType, rcInconsistent );
        if ( elem_bits > elem_size && elem_bits % elem_size != 0 )
            return RC ( rcVDB, rcColumn, rcUpdating, rcType, rcInconsistent );
        if ( num_bits % elem_size != 0 )
            return RC ( rcVDB, rcColumn, rcUpdating, rcType, rcInconsistent );
    }

    /* allow empty row */
    if ( len == 0 )
    {
        self -> row_written = true;
        self -> dflt_last = false;
        return 0;
    }

    /* disallow any further modifications */
    if ( self -> row_committed )
        return RC ( rcVDB, rcColumn, rcUpdating, rcColumn, rcBusy );

    /* total number of bits to be put into buffer */
    doff = self -> bits_in_buffer + self -> row_len;
    data_bits = doff + num_bits;

    /* see if it fits into buffer */
    if ( data_bits > KDataBufferBits ( & self -> data ) )
    {
        rc_t rc;

        /* calculate needed bytes */
        size_t new_size = ( ( size_t ) ( ( data_bits + 7 ) >> 3 ) + INITIAL_DATA_PEAK - 1 )
            & ~ ( size_t ) ( INITIAL_DATA_PEAK - 1 );

        /* need buffer memory */
        if ( self -> data . elem_count == 0 )
        {
            /* always go with the peak size if larger */
            if ( new_size < self -> data_peak )
                new_size = self -> data_peak;

            /* make initial buffer at most recent peak */
            rc = KDataBufferMakeBytes ( & self -> data, new_size );
            if ( rc != 0 )
                return rc;
        }
        else
        {
            /* we have a new peak */
            rc = KDataBufferResize ( & self -> data, new_size );
            if ( rc != 0 )
                return rc;
        }
    }

    /* copy in data */
    if ( ( ( boff | doff | num_bits ) & 7 ) != 0 )
        bitcpy ( self -> data . base, doff, buffer, boff, num_bits );
    else
    {
        memmove ( & ( ( uint8_t* ) self -> data . base ) [ doff >> 3 ],
                 & ( ( const uint8_t* ) buffer ) [ boff >> 3 ], num_bits >> 3 );
    }

    /* accept changes */
    self -> row_len += num_bits;
    self -> row_written = true;
    self -> dflt_last = false;

    return 0;
}

/* RowDefaults
 *  if a row has not been written but has a default value,
 *  that value is written to the row. if no default exists,
 *  an error is generated.
 *
 *  "rc" [ OUT, DEFAULT ZERO ] - preset to 0
 *
 *  returns true if any error occurs ( i.e. "*rc != 0" )
 */
bool CC WColumnRowDefaults ( void *item, void *data )
{
    WColumn *self = item;
    rc_t *rc = data;

    /* nothing to do if row written */
    if ( self == NULL || self -> row_written )
        return false;

    /* error if no default value */
    if ( ! self -> have_dflt )
    {
        * rc = RC ( rcVDB, rcColumn, rcClosing, rcRow, rcIncomplete );
        PLOGERR ( klogErr,
                  ( klogErr, * rc, "Column: $(col)", "col=%.*s"
                    , self -> dad . scol -> name -> name . size
                    , self -> dad . scol -> name -> name . addr )
            );
        return true;
    }

    /* detect NULL row as default */
    if ( self -> dflt . elem_bits == 0 )
    {
        * rc = RC ( rcVDB, rcColumn, rcClosing, rcRow, rcNull );
        PLOGERR ( klogWarn,
                  ( klogWarn, * rc, "Column: $(col)", "col=%.*s"
                    , self -> dad . scol -> name -> name . size
                    , self -> dad . scol -> name -> name . addr )
            );
        return false;
    }

    /* if the last column written was default,
       there is an opportunity for simply incrementing the repeat count */
    if ( USE_RLE && self -> dflt_last )
    {
        WColumnRowMap *rm = self -> rowmap . base;
        assert ( self -> num_rows != 0 );
        ++ rm [ self -> num_rows - 1 ] . cnt;
        return false;
    }

    /* write default data */
    * rc = WColumnWrite ( & self -> dad, self -> dflt . elem_bits,
        self -> dflt . base, 0, self -> dflt . elem_count );
    if ( * rc != 0 )
        return true;

    /* record the fact that this was default */
    self -> dflt_last = true;
    return false;
}

/* CommitRow
 *  closes the row to further writes and accepts
 *  all data written so far as complete. if the accumulated
 *  page data trigger a flush, the flush parameter is set.
 *
 *  "end_id" [ IN/OUT ] - used to calculate the minimum
 *  end_id for pages. if the column decides that it has too
 *  much data in its buffer and wants a cutoff < current
 *  value, it can lower the id.
 *
 *  returns true if there was a memory error.
 */
static
bool WColumnCommitRepeatedRow ( WColumn *self, WColumnRowMap *rm, int64_t end_id )
{
    /* if we've previously declared a cutoff id,
       incorporate the repeated row into the range */
    if ( ( self -> cutoff_id != self -> start_id ) && ( self -> cutoff_id + 1 == end_id ) )
        self -> cutoff_id = end_id;

    /* just account for the repeated row */
    ++ rm -> cnt;

    /* drop data */
    self -> row_len = 0;

    /* the row has been successfully committed */
    self -> row_committed = true;

    /* no error */
    return false;
}

static
bool WColumnCommitRowData ( WColumn *self, int64_t *end_id )
{
    size_t cur_size;
    uint64_t row_len;
    bitsz_t elem_bits;
    WColumnRowMap *rm;

    /* if no data were written and that's okay, ignore */
    if ( ! self -> row_written )
    {
        assert ( self -> row_len == 0 );
        self -> row_committed = true;
        return false;
    }

    /* column's element size */
    elem_bits = VTypedescSizeof ( & self -> dad . desc );
    assert ( self -> row_len % elem_bits == 0 );
    row_len = self -> row_len / elem_bits;

    /* detect repeated row */
    if ( USE_RLE && self -> num_rows != 0 )
    {
        rm = self -> rowmap . base;
        rm += self -> num_rows - 1;

        if ( rm -> len == row_len && rm->cnt <= INT32_MAX)
        {
            bitsz_t poff, coff;
            const uint8_t *base;

            if ( row_len == 0 )
                return WColumnCommitRepeatedRow ( self, rm, * end_id );

            base = ( const void* ) self -> data . base;
            assert ( self -> bits_in_buffer >= self -> row_len );
            coff = self -> bits_in_buffer;
            poff = self -> bits_in_buffer - self -> row_len;

            if ( ( ( poff | coff | self -> row_len ) & 7 ) != 0 )
            {
                if ( bitcmp ( base, poff, base, coff, self -> row_len ) == 0 )
                    return WColumnCommitRepeatedRow ( self, rm, * end_id );
            }
            else
            {
                if ( memcmp ( & base [ poff >> 3 ], & base [ coff >> 3 ], self -> row_len >> 3 ) == 0 )
                    return WColumnCommitRepeatedRow ( self, rm, * end_id );
            }
        }
    }

    /* need to add a new row */
    if ( ( uint64_t ) self -> num_rows == self -> rowmap . elem_count )
    {
        /* need more memory */
        rc_t rc;
        self -> rowmap . elem_bits = sizeof * rm * 8;
        rc = KDataBufferResize ( & self -> rowmap, self -> rowmap . elem_count + 16 * 1024 );
        if ( rc != 0 )
        {
            PLOGERR ( klogErr,
                      ( klogErr, rc, "Column: $(col) failed to resize row map", "col=%.*s"
                        , self -> dad . scol -> name -> name . size
                        , self -> dad . scol -> name -> name . addr )
                );
            return true;
        }
    }

    rm = self -> rowmap . base;
    rm += self -> num_rows;

    rm -> start_id = self -> end_id;
    rm -> len = row_len;
    rm -> cnt = 1;

    self -> bits_in_buffer += self -> row_len;
    self -> row_len = 0;

    ++ self -> num_rows;
    self -> row_committed = true;

    /* detect 1x blob cutoff */
    cur_size = ( size_t ) ( self -> bits_in_buffer + 7 ) >> 3;
    if ( cur_size >= self -> trigger )
    {
        /* if size just crossed the trigger boundary and
         * cutoff_id has not been advanced yet */
        if ( self -> cutoff_id == self -> start_id )
        {
            self -> cutoff_id = * end_id;
        }

        /* or perhaps the buffer is too large */
        else if ( ( cur_size + cur_size ) >= self -> trigger * 3 )
        {
            /* set to min of current end or our cutoff */
            if ( self -> cutoff_id < * end_id )
            {
#if ROW_COUNT_CUTOFFS
                /* the number of rows until our cutoff */
                uint64_t row_count = self -> cutoff_id - self -> start_id;
                if ( row_count != 0 )
                {
/*                  int64_t end; */
                    uint64_t msb = row_count;

                    /* adjust id to nearest power of two size
                       that does not exceed current id */
                    while ( ( msb & ( msb - 1 ) ) != 0 )
                        msb &= msb - 1;

                    /* add in rounding factor to row_count */
                    row_count += ( msb >> 1 );

                    /* truncate */
                    row_count &= ~ ( msb - 1 );

                    if ( row_count >= 1024 )
                    {
                        /* limit to current row */
                        while ( row_count > 1024 && self -> start_id + row_count > * end_id )
                            row_count >>= 1;

                        if ( self -> start_id + row_count <= * end_id )
                            self -> cutoff_id = self -> start_id + row_count;
                    }
                }
#endif
                * end_id = self -> cutoff_id;
            }
        }
    }

    return false;
}

bool CC WColumnCommitRow ( void *item, void *data )
{
    WColumn *self = item;
    if ( self != NULL )
    {
        int64_t *end_id = data;

        if ( WColumnCommitRowData ( self, end_id ) )
            return true;

        /* if the row range is too great */
        if ( ( self -> end_id - self -> start_id ) >= MAX_ROW_COUNT )
        {
            /* if row range has just crossed the boundary and
             * cutoff_id has not been advanced yet */
            if ( self -> cutoff_id == self -> start_id )
            {
                self -> cutoff_id = * end_id;
            }

            /* set to min of current end or our cutoff */
            else if ( self -> cutoff_id < * end_id )
            {
                * end_id = self -> cutoff_id;
            }
        }
    }

    return false;
}

/* RepeatRow
 *  go into the last row entry
 *  extend the count by uint64_t
 */
void CC WColumnRepeatRow ( void *item, void *data )
{
    WColumn *self = item;
    const WColumnRepeatRowData *pb = data;

    if ( self != NULL && self -> num_rows != 0 )
    {
        WColumnRowMap *rm = self -> rowmap . base;
        rm += self -> num_rows - 1;

        /* if we've previously declared a cutoff id,
           incorporate the repeated row into the range */
        if ( ( self -> cutoff_id != self -> start_id ) && ( self -> cutoff_id + 1 == pb -> end_id ) )
            self -> cutoff_id = pb -> end_id + pb -> count;

        /* just account for the repeated row */
        rm -> cnt += pb -> count;
        self -> end_id += pb -> count;
    }
}

/* CloseRow
 *  discards uncommitted data
 *  update state
 */
void CC WColumnCloseRow ( void *item, void *ignore )
{
    WColumn *self = item;
    if ( self != NULL )
    {
        if ( self -> row_committed )
            ++ self -> end_id;

        self -> row_len = 0;
        self -> row_written = false;
        self -> row_committed = false;
    }
}

/* BufferPage
 *  captures page range
 *
 *  "end_id" [ IN, CONST ] - half-closed id of buffered range end
 *  column should capture this information for creating page
 *  id range either on demand, or pre-prepared.
 *
 *  returns true if there was a memory error.
 */
static
bool WColumnSplitBuffer ( WColumn *self, int64_t end_id, size_t rm_idx )
{
    rc_t rc;
    int64_t id;
    VBlob *vblob;
    bool splitting;
    size_t i, data_cur;
    uint64_t sum_elems;

    const WColumnRowMap *rm = self -> rowmap . base;

    uint64_t num_rows = rm [ rm_idx ] . start_id + rm [ rm_idx ] . cnt - self -> start_id;
    assert ( rm [ 0 ] . start_id == self -> start_id );

    /* adjust end_id [ TAKES CARE OF THEORETICAL ERROR CONDITION
       NOT KNOWN TO EXIST ] and num_rows ( when splitting repeat ) */
    id = end_id;
    splitting = false;
    if ( self -> start_id + num_rows <= id )
        id = self -> start_id + num_rows;
    else
    {
        num_rows = id - self -> start_id;
        splitting = true;
    }

    /* release previous ( but unexpected ) cache if there */
    if ( self -> page != NULL )
    {
        TRACK_BLOB ( VBlobRelease, self -> page );
        VBlobRelease ( self -> page );
        self -> page = NULL;
    }

    /* create new blob */
    rc = VBlobNew ( & vblob,
                    self -> start_id, id - 1,
                    self -> dad . scol -> name -> name . addr );
    TRACK_BLOB ( VBlobNew, vblob );
    if ( rc != 0 )
    {
        PLOGERR ( klogErr,
                  ( klogErr, rc, "Column: $(col) failed to allocate page", "col=%.*s"
                    , self -> dad . scol -> name -> name . size
                    , self -> dad . scol -> name -> name . addr )
            );
        return true;
    }

    /* create blob page map */
    rc = PageMapNew ( & vblob -> pm, rm_idx + 1 ); /*** rm_idx tells many WColumnRowMap need to be added to PageMap ***/
    if ( rc != 0 )
    {
        TRACK_BLOB ( VBlobRelease, vblob );
        VBlobRelease ( vblob );

        PLOGERR ( klogErr,
                  ( klogErr, rc, "Column: $(col) failed to create page map", "col=%.*s"
                    , self -> dad . scol -> name -> name . size
                    , self -> dad . scol -> name -> name . addr )
            );
        return true;
    }

    /* write page map up to ( but not including ) last entry */
    for ( sum_elems = 0, i = 0; i < rm_idx; ++ i )
    {
        sum_elems += rm [ i ] . len;
        rc = PageMapAppendSomeRows ( vblob -> pm, rm [ i ] . len, rm [ i ] . cnt );
        if ( rc != 0 )
        {
            TRACK_BLOB ( VBlobRelease, vblob );
            VBlobRelease ( vblob );

            PLOGERR ( klogErr,
                      ( klogErr, rc, "Column: $(col) failed to write page map", "col=%.*s"
                        , self -> dad . scol -> name -> name . size
                        , self -> dad . scol -> name -> name . addr )
                );
            return true;
        }
    }

    sum_elems += rm [ i ] . len;
    rc = PageMapAppendSomeRows ( vblob -> pm, rm [ i ] . len, id - rm [ i ] . start_id );
    if ( rc != 0 )
    {
        TRACK_BLOB ( VBlobRelease, vblob );
        VBlobRelease ( vblob );

        PLOGERR ( klogErr,
                  ( klogErr, rc, "Column: $(col) failed to write page map", "col=%.*s"
                    , self -> dad . scol -> name -> name . size
                    , self -> dad . scol -> name -> name . addr )
            );
        return true;
    }

    /* determine current data buffer usage */
    data_cur = ( size_t ) ( ( ( self -> bits_in_buffer + 7 ) >> 3 )
        + SINGLE_PAGE_SIZE - 1 ) & ~ ( size_t ) ( SINGLE_PAGE_SIZE - 1 );

    /* update peak history */
    if ( self -> data_peak_hist [ self -> peak_hist_idx ] == self -> data_peak )
    {
        int j;
        size_t data_peak = data_cur;

        for ( j = ( self -> peak_hist_idx + 1 ) & 0xF;
              j != self -> peak_hist_idx;
              j = ( j + 1 ) & 0xF )
        {
            if ( self -> data_peak_hist [ j ] > data_peak )
                data_peak = self -> data_peak_hist [ j ];
        }

        self -> data_peak = data_peak;
    }
    self -> data_peak_hist [ self -> peak_hist_idx ] = data_cur;
    self -> peak_hist_idx = ( self -> peak_hist_idx + 1 ) & 0xF;
    if ( data_cur > self -> data_peak )
        self -> data_peak = data_cur;

    /* hand data over to blob */
    vblob -> data = self -> data;
    vblob -> data . elem_bits = VTypedescSizeof ( & self -> dad . desc );
    vblob -> data . elem_count = sum_elems;

    /* handle residual data */
    if ( splitting )
        sum_elems -= rm [ rm_idx ] . len;
    else
        ++ rm_idx;

    if ( rm_idx == self -> num_rows )
    {
        /* no residual data/rows */
        memset ( & self -> data, 0, sizeof self -> data );
        self -> bits_in_buffer = 0;
        self -> num_rows = 0;
        self -> start_id = self -> cutoff_id = end_id;
    }
    else
    {
        KDataBuffer data;
        bitsz_t boff, to_copy;

        /* allocate a new data buffer */
        rc = KDataBufferMakeBytes ( & data, self -> data_peak );
        if ( rc != 0 )
        {
            /* forget transfer of blob */
            memset ( & vblob -> data, 0, sizeof vblob -> data );

            TRACK_BLOB ( VBlobRelease, vblob );
            VBlobRelease ( vblob );

            PLOGERR ( klogErr,
                      ( klogErr, rc, "Column: $(col) failed to split page", "col=%.*s"
                        , self -> dad . scol -> name -> name . size
                        , self -> dad . scol -> name -> name . addr )
                );
            return true;
        }

        /* copy data */
        boff = ( rm_idx > 0 ) ?
            sum_elems * vblob -> data . elem_bits : 0;
        to_copy = self -> bits_in_buffer - boff;
        if ( ( ( boff | to_copy ) & 7 ) != 0 )
            bitcpy ( data . base, 0, self -> data . base, boff, to_copy );
        else
            memmove ( data . base, & ( ( const uint8_t* ) self -> data . base ) [ boff >> 3 ], to_copy >> 3 );

        self -> data = data;
        self -> bits_in_buffer = to_copy;

        /* copy row map */
        memmove ( self -> rowmap . base, & rm [ rm_idx ], ( self -> num_rows -= rm_idx ) * sizeof * rm );

        /* adjust starting id and repeat count if splitting single row */
        if ( splitting )
        {
            WColumnRowMap *split = self -> rowmap . base;
            split -> cnt = split -> start_id + split -> cnt - id;
            split -> start_id = id;
        }

        self -> start_id = self -> cutoff_id = id;
    }

    self -> page = vblob;
    self -> dflt_last = false;

    return false;
}

static
int WColumnRowMapCompare ( const WColumnRowMap *self, int64_t id )
{
    /* id is right-edge exclusive */
    if ( self -> start_id >= id )
        return -1;
    return id > self -> start_id + self -> cnt;
}

bool CC WColumnBufferPage ( void *item, void *const_end_id )
{
    WColumn *self = item;
    if ( self != NULL )
    {
        int64_t end_id = * ( const int64_t* ) const_end_id;

        int diff;
        const WColumnRowMap *rm = self -> rowmap . base;
        size_t i, left, right = self -> num_rows;

        /* find the entry mentioned */
        if ( self -> num_rows == 0 )
        {
            PLOGMSG ( klogWarn,
                      ( klogWarn, "Column: $(col) has no rows to buffer", "col=%.*s"
                        , self -> dad . scol -> name -> name . size
                        , self -> dad . scol -> name -> name . addr )
                );
            return true;
        }

        /* check last entry */
        i = right - 1;
        diff = WColumnRowMapCompare ( & rm [ i ], end_id );
        if ( diff >= 0 )
            return WColumnSplitBuffer ( self, end_id, i );

        /* check that the id is within our range at all */
        assert ( self -> start_id == rm [ 0 ] . start_id );
        if ( end_id < self -> start_id )
        {
            PLOGMSG ( klogWarn,
                      ( klogWarn, "Column: $(col) cutoff id $(id) is not within range"
                        , "col=%.*s,id=%ld"
                        , self -> dad . scol -> name -> name . size
                        , self -> dad . scol -> name -> name . addr
                        , end_id )
                );
            return true;
        }

        /* search rails are left ( inclusive ) at 0, right ( exclusive ) at num_rows */
        left = 0;

        /* perform arithmetic search for initial portion */
        if ( right > 1000 )
        {
            double p = ( double ) ( end_id - rm [ left ] . start_id ) /
                ( rm [ right - 1 ] . start_id + rm [ right - 1 ] . cnt - rm [ left ] . start_id );
            assert ( p >= 0.0 && p <= 1.0 );

            i = left + p * ( right - left );

            diff = WColumnRowMapCompare ( & rm [ i ], end_id );
            if ( diff == 0 )
                return WColumnSplitBuffer ( self, end_id, i );
            if ( diff < 0 )
                right = i;
            else
                left = i + 1;

            if ( left < right )
            {
                p = ( double ) ( end_id - rm [ left ] . start_id ) /
                    ( rm [ right - 1 ] . start_id + rm [ right - 1 ] . cnt - rm [ left ] . start_id );
                assert ( p >= 0.0 && p <= 1.0 );

                i = left + p * ( right - left );

                diff = WColumnRowMapCompare ( & rm [ i ], end_id );
                if ( diff == 0 )
                    return WColumnSplitBuffer ( self, end_id, i );
                if ( diff < 0 )
                    right = i;
                else
                    left = i + 1;
            }
        }

        /* normal binary search */
        while ( left < right )
        {
            i = ( left + right ) >> 1;

            diff = WColumnRowMapCompare ( & rm [ i ], end_id );
            if ( diff == 0 )
                return WColumnSplitBuffer ( self, end_id, i );
            if ( diff < 0 )
                right = i;
            else
                left = i + 1;
        }

        PLOGMSG ( klogErr,
                  ( klogErr, "Column: $(col) cutoff id $(id) is not within range"
                    , "col=%.*s,id=%ld"
                    , self -> dad . scol -> name -> name . size
                    , self -> dad . scol -> name -> name . addr
                    , end_id )
            );
    }

    return true;
}

/* ReadBlob
 *  reads an input blob
 *  called as a result of commit page which reads the validation production
 */
rc_t WColumnReadBlob ( WColumn *self, VBlob **vblob, int64_t id )
{
    if ( self -> page == NULL )
        return RC ( rcVDB, rcColumn, rcReading, rcBuffer, rcNotFound );
    if ( id < self -> page -> start_id || id > self -> page -> stop_id )
        return RC ( rcVDB, rcColumn, rcReading, rcRow, rcNotFound );

    * vblob = self -> page;
    VBlobAddRef ( self -> page );
    TRACK_BLOB ( VBlobAddRef, self -> page );

    return 0;
}

/* DropPage
 *  drops any page buffers created
 */
void CC WColumnDropPage ( void *item, void *ignore )
{
    WColumn *self = item;
    if ( self != NULL && self -> page != NULL )
    {
        TRACK_BLOB ( VBlobRelease, self -> page );
        VBlobRelease ( self -> page );
        self -> page = NULL;
    }
}
