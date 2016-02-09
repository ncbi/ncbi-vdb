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

#ifndef _h_column_priv_
#define _h_column_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_vdb_xform_
#include <vdb/xform.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_data_buffer_
#include <klib/data-buffer.h>
#endif

#include <os-native.h>

#ifndef KONST
#define KONST
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct Vector;
struct KColumn;
struct VDBManager;
struct VSchema;
struct SColumn;
struct SExpression;
struct VProduction;
struct VBlob;
struct VBlobMRUCacheCursorContext;


/*--------------------------------------------------------------------------
 * VColumn
 *  externally visible column object
 */
typedef struct VColumn VColumn;
struct VColumn
{
    /* for type queries */
    struct VSchema const *schema;

    /* typed column */
    struct SColumn const *scol;

    /* read production */
    struct VProduction *in;

    /* cached output */
    struct VBlob *cache;

    /* type information */
    VTypedecl td;
    VTypedesc desc;

    /* vector ids */
    uint32_t ord;

    bool read_only;
    uint8_t align [ 3 ];
};

void CC VColumnWhack ( void *item, void *curs );
void VColumnDestroy ( VColumn *self );
rc_t VColumnInit ( VColumn *self,
    struct VSchema const *schema, struct SColumn const *scol );

int CC VColumnCmp ( const void *item, const void *n );
int CC VColumnSort ( const void *item, const void *n );

rc_t VColumnMake ( VColumn **col,
    struct VSchema const *schema, struct SColumn const *scol );

rc_t VColumnIdRange ( const VColumn *self,
    int64_t *first, int64_t *last );
rc_t VColumnIdRangeRead ( const VColumn *self,
    int64_t *first, int64_t *last );
rc_t VColumnPageIdRange ( const VColumn *self,
    int64_t id, int64_t *first, int64_t *last );

rc_t VColumnDatatype ( const VColumn *self,
    struct VTypedecl *type, struct VTypedesc *desc );

rc_t VColumnRead ( const VColumn *self, int64_t row_id,
   uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len,
   struct VBlob **vblob );

rc_t VColumnReadBlob ( const VColumn *self, struct VBlob const **blob, int64_t row_id,
   uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len, uint32_t *repeat_count,
   struct VBlobMRUCacheCursorContext *cctx);

rc_t VColumnReadCachedBlob ( const VColumn *self, struct VBlob const *blob, int64_t row_id,
   uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len, uint32_t *repeat_count );

rc_t VColumnIsStatic ( const VColumn *self, bool *is_static );

rc_t VColumnGetKColumn ( const VColumn * self, struct KColumn ** kcol, bool * is_static );

/*--------------------------------------------------------------------------
 * WColumn
 *  column with input buffer
 */
typedef struct WColumn WColumn;
struct WColumn
{
    VColumn dad;

    /* half-closed range of buffered rows */
    int64_t start_id, end_id;

    /* half-closed id for page cutoff */
    int64_t cutoff_id;

    /* alternate read production */
    struct VProduction *alt;

    /* validate production */
    struct VProduction *val;

    /* write production */
    struct VProduction *out;

    /* output page */
    struct VBlob *page;

    /* default row data */
    KDataBuffer dflt;

    /* accumulators */
    KDataBuffer data, rowmap;

    /* peak byte size history of data accumulator */
    size_t data_peak_hist [ 16 ];
    size_t data_peak;

    /* total committed bits in buffer */
    bitsz_t bits_in_buffer;

    /* number of uncommitted bits in buffer */
    bitsz_t row_len;

    /* size ( in bytes ) to trigger page commit */
    size_t trigger;

    /* number of committed rows in buffer */
    size_t num_rows;

    /* peak history index */
    uint8_t peak_hist_idx;

    /* true if there is a default value */
    bool have_dflt;

    /* set upon any successful write */
    bool row_written;

    /* set if the last row written was default */
    bool dflt_last;

    /* set upon row commit */
    bool row_committed;
};

/* WColumnRowMap
 */
typedef struct WColumnRowMap WColumnRowMap;
struct WColumnRowMap
{
    int64_t start_id;
    uint64_t len, cnt;
};

rc_t WColumnMake ( VColumn **col, struct VSchema const *schema,
    struct SColumn const *scol, struct SExpression const *dflt_limit,
    struct VDBManager *mgr, struct Vector *cx_bind );

rc_t WColumnSetDefault ( VColumn *self,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t len );
rc_t WColumnWrite ( VColumn *self,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t len );

/* OpenRow
 *  update state
 *
 *  "const_row_id" [ IN, CONST ] - id of row being opened. useful
 *  only on initial open when no other rows are buffered.
 */
void CC WColumnOpenRow ( void *self, void *const_row_id );

/* RowDefaults
 *  if a row has not been written but has a default value,
 *  that value is written to the row. if no default exists,
 *  an error is generated.
 *
 *  "rc" [ OUT, DEFAULT ZERO ] - preset to 0
 *
 *  returns true if any error occurs ( i.e. "*rc != 0" )
 */
bool CC WColumnRowDefaults ( void *self, void *rc );

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
bool CC WColumnCommitRow ( void *self, void *end_id );

/* RepeatRow
 *  go into the last row entry
 *  extend the count by count
 *  data points to this structure
 */
typedef struct WColumnRepeatRowData WColumnRepeatRowData;
struct WColumnRepeatRowData
{
    uint64_t count;
    int64_t row_id;
    int64_t end_id;
};
void CC WColumnRepeatRow ( void *self, void *data );

/* CloseRow
 *  discards uncommitted data
 *  update state
 */
void CC WColumnCloseRow ( void *self, void *ignore );

/* BufferPage
 *  captures page range
 *
 *  "end_id" [ IN, CONST ] - half-closed id of buffered range end
 *  column should capture this information for creating page
 *  id range either on demand, or pre-prepared.
 *
 *  returns true if there was a memory error.
 */
bool CC WColumnBufferPage ( void *self, void *const_end_id );

/* DropPage
 *  drops all rows associated with committed page
 */
void CC WColumnDropPage ( void *self, void *ignore );

/* ReadBlob
 *  reads an input blob
 *  returns a blob with all rows in commit range
 *
 *  "vblob" [ OUT ] - page to return
 *
 *  "id" [ IN ] - an id that must be within page range
 *  returns rcNotFound and NULL blob if id is not within range
 */
rc_t WColumnReadBlob ( WColumn *self, struct VBlob **vblob, int64_t id );

#ifdef __cplusplus
}
#endif

#endif /* _h_column_priv_ */
