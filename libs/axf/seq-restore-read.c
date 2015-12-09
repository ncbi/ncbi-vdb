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

#define READ_RESTORER_VERSION 2

#if READ_RESTORER_VERSION == 2

/* --------------------------- rr_store --------------------------- */

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
			memcpy( &( obj -> read[ 0 ] ), read, read_len );
			*entry = obj;
		}
	}
	return res;
}


typedef struct rr_store
{
	KVector * v;
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


static bool rr_store_read ( rr_store * rr, int64_t align_id, const INSDC_4na_bin * read, uint32_t read_len )
{
	bool res = ( rr != NULL && read != NULL );
	if ( res )
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


static bool rr_get_read ( rr_store * rr, int64_t align_id, rr_entry ** entry )
{
	bool res = ( rr != NULL && entry != NULL );
	if ( res )
	{
		uint64_t key = ( uint64_t ) align_id;
		res = ( KVectorGetPtr ( rr -> v, key, ( void ** )entry ) == 0 );
		if ( res )
			res = ( KVectorUnset ( rr -> v, key ) == 0 );
	}
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

	int64_t  last_max_align_id;	
	uint64_t hits, miss;

#if READ_RESTORER_VERSION == 2
	uint32_t last_align_id_count;
	rr_store * read_store;
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
				{
					/* add columns to cursor */
					rc = VCursorAddColumn ( obj -> curs, & obj -> read_idx, "( INSDC:4na:bin ) READ" );
					if ( rc == 0 )
					{
						rc = VCursorOpen ( obj -> curs );
						if ( rc == 0 )
							rc = VCursorLinkedCursorSet( native_curs, tablename, obj->curs );
					}

					VCursorRelease ( obj -> curs );
                    obj -> curs = NULL;
				}
			}
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
			rc = rr_store_make ( & obj -> read_store );
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

typedef struct id_list
{
	int64_t * list;
	uint32_t count;
} id_list;


/*
static id_list sort_align_ids( id_list *align_ids )
{
	id_list res;
	res.count = 0;
	res.list = malloc( align_ids->count * sizeof( * res.list ) );
	if ( res.list != NULL )
	{
		uint32_t i;
		for( i = 0, res.count = 0; i < align_ids->count; i++ )
		{
			if ( align_ids->list[ i ] > 0 )
				res.list[ res.count++ ] = align_ids->list[ i ];
		}
		if ( res.count > 0 )
			ksort_int64_t( res.list, res.count );
	}
	return res;
}
*/

static int64_t find_max_id( id_list *ids )
{
	int64_t res = 0;
	if ( ids != NULL && ids->list != NULL )
	{
		uint32_t i;
		for( i = 0; i < ids->count; i++ )
		{
			if ( ids->list[ i ] > res )
				res = ids->list[ i ];
		}
	}
	return res;
}

static rc_t prefetch_alignments( Read_Restorer * self, int64_t seq_row_id, id_list *align_ids )
{
	rc_t rc = 0;
	int64_t max_id = find_max_id( align_ids );
	if ( max_id > self->last_max_align_id )
	{
		int64_t min_id = self->last_max_align_id == 0 ? 1 : self->last_max_align_id + 1;
		int64_t spread = ( max_id - min_id + 1 );
		
		uint32_t stored = 0;
		int64_t align_id;

		if ( spread > ( 4 * align_ids->count ) )
			max_id = min_id + ( 4 * align_ids->count );
		
		for ( align_id = min_id; align_id <= max_id && rc == 0; ++align_id )
		{
			const INSDC_4na_bin * read = NULL;
			uint32_t read_len;
			rc = VCursorCellDataDirect( self -> curs, align_id, self -> read_idx,
										NULL, ( const void** ) &read, NULL, &read_len );
			if ( rc == 0 && rr_store_read ( self->read_store, align_id, read, read_len ) )
				stored++;
		}
		self->last_max_align_id = max_id;

		/*
		fprintf( stderr, "seq-row:%lu done: aligs:%d, min=%ld, max=%ld, stored=%d, hits=%lu, miss=%lu\n",
				seq_row_id, align_ids->count, min_id, max_id, stored, self->hits, self->miss );
		*/
	}
	return rc;
}


static rc_t CC seq_restore_read_impl2 ( void *data, const VXformInfo *info, int64_t row_id,
                                 VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    Read_Restorer	*self = data;
    INSDC_4na_bin	*dst;
    INSDC_coord_len	len;
	id_list align_ids;
    uint32_t i;	
    uint32_t src_len		    = (uint32_t)argv[ 0 ].u.data.elem_count;
    const INSDC_4na_bin	*src	= argv[ 0 ].u.data.base;
    const uint32_t 	num_reads	= (uint32_t)argv[ 1 ].u.data.elem_count;
    const INSDC_coord_len *read_len = argv[ 2 ].u.data.base;
    const uint8_t *read_type = argv[ 3 ].u.data.base;
	bool is_sequential = false;

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

    /* test that row_id is either:
       a. a repeat of the last row, or
       b. exactly 1 greater than last row.
    */
    if ( row_id != self -> last_row_id && row_id != self -> last_row_id + 1 )
    {
        /* not precisely sequential */
        is_sequential = false;

        /* but it may be the start of a new sequence */
        self -> first_sequential_row_id = row_id;
    }

    /* row_id is sequential.
       detect sequence of more than 100 */
    else if ( row_id > self -> first_sequential_row_id + 100 )
    {
        /* the pattern appears to be sequential reads */
        is_sequential = true;
    }

    /* record the last row_id seen */
    self -> last_row_id = row_id;

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
            memcpy( dst, src, len );
		else
		{
			rr_entry *ep;
			const INSDC_4na_bin * rd;
			uint32_t rd_len;

			if ( is_sequential )
            {
                if ( align_ids.count > self->last_align_id_count )
                    rc = prefetch_alignments( self, row_id, &align_ids );
                self->last_align_id_count = align_ids.count;
            }
			
			for ( i = 0; i < num_reads && rc == 0; i++ ) /*** checking read by read ***/
			{
				int64_t align_id = align_ids.list[ i ];
				if ( align_id > 0 )
				{
					bool found_in_cache = rr_get_read ( self->read_store, align_id, &ep );
					if ( found_in_cache && ep == NULL )
						found_in_cache = false;
					if ( found_in_cache )
					{
						rd = &( ep->read[ 0 ] );
						rd_len = ep->read_len;
						self->hits++;
					}
					else
					{
						rc = VCursorCellDataDirect( self -> curs, align_id, self -> read_idx,
												    NULL, ( const void** ) &rd, NULL, &rd_len );
						self->miss++;
					}
					
					if ( rc == 0 )
					{
						if ( rd_len == read_len[ i ] )
						{
							if ( read_type[ i ] & SRA_READ_TYPE_FORWARD )
							{
								memcpy( dst, rd, read_len[ i ] );
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

					if ( found_in_cache )
						free( ( void * ) ep );
					
				}
				else /*** data is in READ column **/
				{
					if ( src_len >= read_len[ i ] )
					{
						memcpy( dst, src, read_len[ i ] );
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
    Read_Restorer	*self = data;
    INSDC_4na_bin	*dst;
    INSDC_coord_len 	len;
    uint32_t		src_len		= (uint32_t)argv[ 0 ].u.data.elem_count;
    const INSDC_4na_bin	*src	= argv[ 0 ].u.data.base;
    const uint32_t 	num_reads	= (uint32_t)argv[ 1 ].u.data.elem_count;
    const int64_t	*align_id	= argv[ 1 ].u.data.base;
    const INSDC_coord_len *read_len = argv[ 2 ].u.data.base;
    const uint8_t	*read_type	= argv[ 3 ].u.data.base;
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
            memcpy( dst, src, len );
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
								memcpy( dst, r_src, read_len[ i ] );
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
						memcpy( dst, src, read_len[ i ] );
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
