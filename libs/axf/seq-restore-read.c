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
#include <vdb/xform.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>

#include <kdb/meta.h>

#include <insdc/insdc.h>
#include <insdc/sra.h>

#include <align/refseq-mgr.h>

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <klib/time.h>
#include <klib/vector.h>
#include <klib/data-buffer.h>
#include <klib/sort.h>

#include <vdb/vdb-priv.h>

#include <bitstr.h>
#include <sysalloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif

/**********************************
VERSION     effect

1           in case of random-joins oscillation memory usage
            prefetch of align-id's in the cursor-cache
                                                                
2           in case of random-joins steady memroy usage
            prefetch of align-id's in dedicated cache

**********************************/
#define READ_RESTORER_VERSION 2
#define ROW_ID_INC_COUNT 100

#if READ_RESTORER_VERSION == 2

/* --------------------------- id_list --------------------------- */


typedef struct id_list
{
    int64_t * list;
    uint32_t count;
} id_list;


static bool sort_align_ids( const id_list * src, id_list * dst )
{
    bool res;
    dst -> count = 0;
    dst -> list = malloc( src -> count * sizeof( dst -> list[ 0 ] ) );
    res = ( dst -> list != NULL );
    if ( res )
    {
        uint32_t i;
        /* filter out the zero id's */
        for( i = 0; i < src -> count; i++ )
        {
            if ( src -> list[ i ] > 0 )
                dst -> list[ dst -> count ++ ] = src -> list[ i ];
        }
        /* now we can sort */
        if ( dst -> count > 0 )
            ksort_int64_t( dst -> list, dst -> count );
    }
    return res;
}


/* --------------------------- rr_entry --------------------------- */

typedef struct rr_entry
{
    uint32_t read_len;
    INSDC_4na_bin read[ 1 ];
} rr_entry;


static rc_t rr_entry_release ( uint64_t key, const void * item, void * data )
{
    if ( item != NULL )
        free( ( void * ) item );
    return 0;
}

static bool rr_entry_make ( rr_entry ** entry, const INSDC_4na_bin * read, uint32_t read_len )
{
    bool res = ( entry != NULL && read != NULL );
    if ( res )
    {
        rr_entry * obj = malloc ( ( sizeof * obj ) + read_len - 1 );
        res = ( obj != NULL );
        if ( res )
        {
            obj -> read_len = read_len;
            memmove( &( obj -> read[ 0 ] ), read, read_len );
            *entry = obj;
        }
    }
    return res;
}


/* --------------------------- rr_store --------------------------- */

typedef struct rr_store
{
    KVector * v;
    int64_t first_seq_row_id;
    int64_t last_seq_row_id;
} rr_store;


static void rr_store_release ( rr_store * rr )
{
    if ( rr != NULL )
    {
        KVectorVisitPtr( rr->v, false, rr_entry_release, NULL );        
        KVectorRelease ( rr->v );
        free( ( void * ) rr );
    }
}

static rc_t rr_store_make ( rr_store ** rr )
{
    rc_t rc;
    if ( rr == NULL )
        rc = RC ( rcXF, rcFunction, rcConstructing, rcSelf, rcNull );
    else
    {
        KVector * v;
        *rr = NULL;
        rc = KVectorMake ( &v );
        if ( rc == 0 )
        {
            rr_store * obj = malloc ( sizeof * obj );
            if ( obj == NULL )
            {
                KVectorRelease ( v );
                rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
            }
            else
            {
                obj -> v = v;
                *rr = obj;
            }
        }
    }
    return rc;
}


static bool rr_store_alignment( rr_store * rr, int64_t align_id, const VCursor * curs, uint32_t read_idx )
{
    bool res = false;
    const INSDC_4na_bin * read = NULL;
    uint32_t read_len;
    rc_t rc = VCursorCellDataDirect( curs, align_id, read_idx, NULL, ( const void** ) &read, NULL, &read_len );
    if ( rc == 0 )
    {
        rr_entry * entry;
        res = rr_entry_make ( &entry, read, read_len );
        if ( res )
        {
            uint64_t key = ( uint64_t ) align_id;
            res = ( KVectorSetPtr ( rr -> v, key, entry ) == 0 );
            if ( !res )
                rr_entry_release( key, entry, NULL );
        }
    }
    return res;
}


static bool rr_fill_cache( rr_store ** rr, const id_list * ids, const VCursor * curs, uint32_t read_idx,
    int64_t row_id, int64_t last_row_id )
{
    bool res = ( rr_store_make ( rr ) == 0 );
    if ( res )
    {
        id_list sorted;
        res = sort_align_ids( ids, &sorted );
        if ( res )
        {
            uint32_t i;
            
            rr_store * r = * rr;
            for( i = 0; i < sorted . count; i++ )
                rr_store_alignment( r, sorted . list[ i ], curs, read_idx );
            free( ( void * ) sorted . list );
            
            r -> first_seq_row_id = row_id;
            r -> last_seq_row_id = last_row_id;
        }
    }
    return res;
}


static bool rr_get_read ( rr_store * rr, int64_t align_id, rr_entry ** entry )
{
    uint64_t key = ( uint64_t ) align_id;
    bool res = ( KVectorGetPtr ( rr -> v, key, ( void ** )entry ) == 0 );
    if ( res && ( *entry == NULL ) ) res = false;
    return res;
}

#endif

/* ---------------------------------------------------------------- */

typedef struct Read_Restorer Read_Restorer;
struct Read_Restorer
{
    const VCursor *curs;
    uint32_t read_idx;
    
    int64_t  last_row_id;
    int64_t  first_sequential_row_id;
    int64_t  prefetch_start_id;
    int64_t  prefetch_stop_id;

#if READ_RESTORER_VERSION == 2
    uint32_t row_id_increments;     /* count how often we have an increment of the row_id */
    rr_store * read_store;          /* if NULL we are not caching... */
#endif
};

static
void CC Read_Restorer_Whack ( void *obj )
{
    Read_Restorer * self = obj;
    if ( self != NULL )
    {
        VCursorRelease ( self -> curs );
#if READ_RESTORER_VERSION == 2
        rr_store_release ( self -> read_store );
#endif
        free ( self );
    }
}


static
rc_t open_RR_cursor( Read_Restorer * obj, const VTable *tbl, const VCursor* native_curs, const char * tablename )
{
    rc_t rc = VCursorLinkedCursorGet( native_curs, tablename, &obj->curs );
    if ( rc != 0 )
    {
        /* get at the parent database */
        const VDatabase *db;
        rc = VTableOpenParentRead ( tbl, & db );
        if ( rc == 0 )
        {
            const VTable *patbl;
            /* open the primary alignment table */
            rc = VDatabaseOpenTableRead ( db, & patbl, tablename );
            VDatabaseRelease ( db );
            if ( rc == 0 )
            {
                /* create a cursor */
                rc = VTableCreateCachedCursorRead( patbl, &obj->curs, 32*1024*1024UL );
                /* rc = VTableCreateCursorRead( patbl, &obj->curs ); */
                VTableRelease ( patbl );
                if ( rc == 0 )
                    rc = VCursorLinkedCursorSet( native_curs, tablename, obj->curs );
            }
        }
    }
    if ( rc == 0 )
    {
        /* add columns to cursor */
        rc = VCursorAddColumn ( obj -> curs, & obj -> read_idx, "( INSDC:4na:bin ) READ" );
        if ( GetRCState(rc) == rcExists )
            rc = 0;
        if ( rc == 0 )
        {
            rc = VCursorOpen ( obj -> curs );
        }
    }
    return rc;
}


static
rc_t Read_Restorer_Make( Read_Restorer **objp, const VTable *tbl, const VCursor* native_curs )
{
    rc_t rc;

    /* create the object */
    Read_Restorer *obj = malloc ( sizeof * obj );
    if ( obj == NULL )
    {
        *objp = NULL;
        rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        memset( obj, 0, sizeof * obj );
        rc = open_RR_cursor( obj, tbl, native_curs, "PRIMARY_ALIGNMENT" );
        if ( rc == 0 )
        {
#if READ_RESTORER_VERSION == 2
            /* - we have no cache to begin with ( obj->read_store is NULL because of memset above )
               - we make one if sequential access is detected */
#endif
            if ( rc == 0 )
            {
                SUB_DEBUG( ( "SUB.Make in 'seq-restore-read.c'\n" ) );
                * objp = obj;
            }
        }
        if ( rc != 0 )
            free( obj );
    }
    return rc;
}


static INSDC_4na_bin  map[]={
/*0  0000 - 0000*/ 0,
/*1  0001 - 1000*/ 8,
/*2  0010 - 0100*/ 4,
/*3  0011 - 1100*/ 12,
/*4  0100 - 0010*/ 2,
/*5  0101 - 1010*/ 10,
/*  0110 - 0110*/ 6,
/*7  0111 - 1110*/ 14,
/*8  1000 - 0001*/ 1,
/*9  1001 - 1001*/ 9,
/*10  1010 - 0101*/ 5,
/*11  1011 - 1101*/ 13,
/*12  1100 - 0011*/ 3,
/*13  1101 - 1011*/ 11,
/*14  1110 - 0111*/ 7,
/*15  1111 - 1111*/ 15
};

#if READ_RESTORER_VERSION == 2


/* caching strategy for READ_RESTORER_VERSION_2 */
static void handle_caching( Read_Restorer * self, id_list * ids, int64_t row_id, int64_t last_row_id )
{
    bool is_sequential = ( self -> read_store != NULL );
    if ( is_sequential )
    {
        /* we are in sequential mode, because we have a cache,
           check if we still are in sequential mode, decrement the age of the cache */
        bool is_in_cache = ( ( row_id >= self -> read_store -> first_seq_row_id ) &&
                             ( row_id <= self -> read_store -> last_seq_row_id ) );
        if ( !is_in_cache )
        {
            is_sequential = ( row_id == self -> read_store -> last_seq_row_id + 1 );
            
            /* blow away the cache no matter if we are sequential or not */
            rr_store_release ( self -> read_store );
            self -> read_store = NULL;
            self -> row_id_increments = 0;
            
            if ( is_sequential )
            {
                /* fill it again */
                if ( !rr_fill_cache( & self -> read_store, ids, self -> curs, self -> read_idx, row_id, last_row_id ) )
                    self -> read_store = NULL;
            }
        }
    }
    else
    {
        /* we are not in sequential mode, because we do not have a cache
           count how often we incremented the row-id by 1 to enter sequential mode */
        if ( row_id == ( self -> last_row_id + 1 ) )
        {
            self -> row_id_increments ++;
            is_sequential = ( self -> row_id_increments > ROW_ID_INC_COUNT );
            if ( is_sequential )
            {
                if ( !rr_fill_cache( & self -> read_store, ids, self -> curs, self -> read_idx, row_id, last_row_id ) )
                    self -> row_id_increments = 0;
            }
        }
        else
            self -> row_id_increments = 0;
    }

    self -> last_row_id = row_id;
}

/* --------------------------------------------------------------------------------------
    Strategy for impl2:

    ( 1 ) - keep track of are we in sequential mode, is row_id continoulsy increasing?
          - if not trow away the cache
          
    ( 2 ) - when entering sequential mode, create the cache, fill it with k/v-pairs
            key ... alignment-id
            value.. READ
            
    ( 3 ) - if in sequential mode, keep track of the row_id beeing in the cache-window
            if end of window reached: throw away the cache

-------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------
    argv[ 0 ]   ... CMP_READ
    argv[ 1 ]   ... PRIM_ALIG_ID
    argv[ 2 ]   ... READ_LEN
    argv[ 3 ]   ... READ_TYPE
-------------------------------------------------------------------------------------- */
static rc_t CC seq_restore_read_impl2 ( void *data, const VXformInfo *info, int64_t row_id,
                                 VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    Read_Restorer   *self = data;
    INSDC_4na_bin   *dst;
    INSDC_coord_len len;
    id_list align_ids;
    uint32_t i; 
    uint32_t src_len                 = (uint32_t)argv[ 0 ] . u . data . elem_count;
    const INSDC_4na_bin * src        = argv[ 0 ] . u . data.base;
    const uint32_t num_reads         = (uint32_t)argv[ 1 ]. u . data . elem_count;
    const INSDC_coord_len * read_len = argv[ 2 ] . u . data.base;
    const uint8_t *read_type         = argv[ 3 ] . u . data.base;
    int64_t last_row_id              = argv[ 1 ] . blob_stop_id;
    
    align_ids.list  = ( int64_t * )argv[ 1 ].u.data.base;
    align_ids.count = ( uint32_t )( argv[ 1 ].u.data.base_elem_count - argv[ 1 ].u.data.first_elem );
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 64 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( INSDC_coord_len ) * 8 );
    assert( argv[ 2 ].u.data.elem_count == num_reads );
    assert( argv[ 3 ].u.data.elem_count == num_reads );
    
    src   += argv [ 0 ] . u . data . first_elem;
    align_ids.list += argv [ 1 ] . u . data . first_elem;
    read_len  += argv [ 2 ] . u . data . first_elem;
    read_type += argv [ 3 ] . u . data . first_elem;

    handle_caching( self, &align_ids, row_id, last_row_id );

    for ( i = 0, len = 0; i < num_reads; i++ )
        len += read_len[ i ];

    /* resize output row */    
    rslt->data->elem_bits = 8;
    rc = KDataBufferResize( rslt->data, len );
    rslt->elem_count = len;
    dst = rslt->data->base;
    
    if ( rc == 0 && len > 0 )
    {
        if ( len == src_len ) /*** shortcut - all data is local ***/
            memmove( dst, src, len );
        else
        {
            rr_entry * ep;
            const INSDC_4na_bin * rd;
            uint32_t rd_len;
            bool found_in_cache;
            
            for ( i = 0; i < num_reads && rc == 0; i++ ) /*** checking read by read ***/
            {
                int64_t align_id = align_ids.list[ i ];
                if ( align_id > 0 )
                {
                    found_in_cache = false;
                    if ( self -> read_store != NULL )
                        found_in_cache = rr_get_read ( self -> read_store, align_id, &ep );
                    if ( found_in_cache )
                    {
                        /* we found it in the cache... */
                        rd = &( ep->read[ 0 ] );
                        rd_len = ep->read_len;
                    }
                    else
                    {
                        /* we did not find it in the cache, get it from the alignment-table... */
                        rc = VCursorCellDataDirect( self -> curs, align_id, self -> read_idx,
                                                    NULL, ( const void** ) &rd, NULL, &rd_len );
                    }
                    
                    if ( rc == 0 )
                    {
                        if ( rd_len == read_len[ i ] )
                        {
                            if ( read_type[ i ] & SRA_READ_TYPE_FORWARD )
                            {
                                memmove( dst, rd, read_len[ i ] );
                            }
                            else if ( read_type[ i ] & SRA_READ_TYPE_REVERSE )
                            {
                                int j, k;
                                for( j = 0, k = read_len[ i ] - 1; j < (int)read_len[ i ]; j++, k-- )
                                {
                                    dst[ j ] = map [ rd[ k ] & 15 ];
                                }
                            }
                            else
                            {
                                rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                            }
                        }
                        else
                        {
                            rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                        }
                    }

                }
                else /*** data is in READ column **/
                {
                    if ( src_len >= read_len[ i ] )
                    {
                        memmove( dst, src, read_len[ i ] );
                        src_len -= read_len[ i ];
                        src     += read_len[ i ];
                    }
                    else
                    {
                        return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                    }
                }
                dst += read_len[ i ];
            }
        }
    }

    return rc;
}

#else

static
rc_t CC seq_restore_read_impl1 ( void *data, const VXformInfo *info, int64_t row_id,
                                 VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    int i;
    Read_Restorer   *self = data;
    INSDC_4na_bin   *dst;
    INSDC_coord_len     len;
    uint32_t        src_len     = (uint32_t)argv[ 0 ].u.data.elem_count;
    const INSDC_4na_bin *src    = argv[ 0 ].u.data.base;
    const uint32_t  num_reads   = (uint32_t)argv[ 1 ].u.data.elem_count;
    const int64_t   *align_id   = argv[ 1 ].u.data.base;
    const INSDC_coord_len *read_len = argv[ 2 ].u.data.base;
    const uint8_t   *read_type  = argv[ 3 ].u.data.base;
    bool is_sequential = false;
    
    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 64 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( INSDC_coord_len ) * 8 );
    assert( argv[ 2 ].u.data.elem_count == num_reads );
    assert( argv[ 3 ].u.data.elem_count == num_reads );
    
    
    src   += argv [ 0 ] . u . data . first_elem;
    align_id  += argv [ 1 ] . u . data . first_elem;
    read_len  += argv [ 2 ] . u . data . first_elem;
    read_type += argv [ 3 ] . u . data . first_elem;

    if ( row_id != self->last_row_id  && row_id != self->last_row_id + 1 )
    {
        self->first_sequential_row_id = row_id;
        is_sequential = false;
    }
    else if ( row_id > self->first_sequential_row_id + 100 )
    {
        is_sequential = true;
    }
    self->last_row_id = row_id;
    
    
    /* is_sequential = false; forcing it to false ... Sept. 16th 2015 to analyze prefetching */
    
    for ( i = 0, len = 0; i < (int)num_reads; i++ )
    {
        len += read_len[ i ];
    }

    /* resize output row */    
    rslt->data->elem_bits = 8;
    rc = KDataBufferResize( rslt->data, len );
    rslt->elem_count = len;
    dst = rslt->data->base;
    if ( rc == 0 && len > 0 )
    {
        if ( len == src_len ) /*** shortcut - all data is local ***/
        {
            memmove( dst, src, len );
        }
        else
        {
            if ( is_sequential &&
                 ( row_id < self->prefetch_start_id || row_id > self->prefetch_stop_id ) )
            { /* do prefetch */
                uint32_t num_rows = ( argv[ 1 ].u.data.base_elem_count - argv[ 1 ].u.data.first_elem );
                
                /*
                KTimeMs_t ts = KTimeMsStamp();
                fprintf( stderr, "\nprefetch row_id #%lu ( start_id #%lu, stop_id #%lu ) num_rows = %d\n", row_id, self->prefetch_start_id, self->prefetch_stop_id, num_rows );
                */
                
                VCursorDataPrefetch( self->curs,
                                     align_id,
                                     self->read_idx,
                                     num_rows,
                                     1,
                                     INT64_MAX,
                                     true );
                                     
                /*
                ts = KTimeMsStamp() - ts;
                fprintf( stderr, "prefetch done in %lu ms\n", ts );
                */
                
                self->prefetch_start_id=row_id;
                self->prefetch_stop_id =argv[1].blob_stop_id;
            }
            for( i = 0; i < (int)num_reads && rc == 0; i++ ) /*** checking read by read ***/
            {
                if ( align_id[ i ] > 0 )
                {
                    const INSDC_4na_bin *r_src;
                    uint32_t             r_src_len;

                    SUB_DEBUG( ( "SUB.Rd in 'seq-restore-read.c' at #%lu\n", align_id[ i ] ) );

                    rc = VCursorCellDataDirect( self -> curs, align_id[ i ], self -> read_idx,
                                                NULL, ( const void** ) &r_src, NULL, &r_src_len );
                    if ( rc == 0 )
                    {
                        if ( r_src_len == read_len[ i ] )
                        {
                            if ( read_type[ i ] & SRA_READ_TYPE_FORWARD )
                            {
                                memmove( dst, r_src, read_len[ i ] );
                            }
                            else if ( read_type[ i ] & SRA_READ_TYPE_REVERSE )
                            {
                                int j, k;
                                for( j = 0, k = read_len[ i ] - 1; j < (int)read_len[ i ]; j++, k-- )
                                {
                                    dst[ j ] = map [ r_src[ k ] & 15 ];
                                }
                            }
                            else
                            {
                                rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                            }
                        }
                        else
                        {
                            rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                        }
                    }
                }
                else /*** data is in READ column **/
                {
                    if ( src_len >= read_len[ i ] )
                    {
                        memmove( dst, src, read_len[ i ] );
                        src_len -= read_len[ i ];
                        src     += read_len[ i ];
                    }
                    else
                    {
                        return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                    }
                }
                dst += read_len[ i ];
            }
        }
    }

    return rc;
}

#endif

/* 
 * function
   INSDC:4na:bin NCBI:align:seq_restore_read #1 ( INSDC:4na:bin cmp_rd,
                                                  I64 align_id,
                                                  INSDC:coord:len read_len,
                                                  INSDC:SRA:xread_type rd_type )

 */
VTRANSFACT_IMPL ( ALIGN_seq_restore_read, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                     VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    Read_Restorer * fself;
    rc_t rc = Read_Restorer_Make ( & fself, info -> tbl,  (const VCursor*)info->parms );
    if ( rc == 0 )
    {
        rslt -> self = fself;
#if READ_RESTORER_VERSION == 2
        rslt -> u.ndf = seq_restore_read_impl2;
#else
        rslt -> u.ndf = seq_restore_read_impl1;
#endif
        rslt -> variant = vftRow;
        rslt -> whack = Read_Restorer_Whack;
    }
    return rc;
}
