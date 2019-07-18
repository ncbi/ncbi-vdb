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
#include <vdb/vdb-priv.h>

#include <kdb/meta.h>

#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <klib/sort.h>
#include <klib/vector.h>
#include <klib/debug.h>

#include <insdc/sra.h>
#include <insdc/insdc.h>

#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef struct ID_cache_t
{
    uint32_t idx;
    VCursor const *curs;
    KVector *j_cache;
    KVector *p_cache;
}ID_cache_t;

#define MIN_DIFF_TO_CACHE 10000
#define TMP_CACHE_PAGESIZE (16*1024)

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif


#if 0
static rc_t lookup_mate( ID_cache_t *const self, int64_t const id, int64_t *mateid )
{
    return KVectorGetI64( self->j_cache, ( uint64_t )id,mateid );
}

static rc_t cache_mate( ID_cache_t *self, int64_t id, int64_t mateid )
{
    return KVectorSetU64( self->j_cache, ( uint64_t )id, mateid );
}
#endif

static rc_t fetch_mates( ID_cache_t *self, int64_t spotid, const int64_t **ids, uint32_t * rlen )
{
    rc_t rc = 0;
    uint32_t elem_bits;
    uint32_t boff;

    SUB_DEBUG( ( "SUB.Rd in 'get_mate_align_ids.c' at #%lu\n", spotid ) );

    rc = VCursorCellDataDirect( self->curs, spotid, self->idx,&elem_bits, (const void**) ids, &boff, rlen );
    assert( *rlen == 0 || elem_bits == 64 );
    assert( boff == 0 );
    return rc;
}

static rc_t fetch_mate_id( ID_cache_t *self, int64_t spotid, int64_t const id, int64_t *mateid )
{
    rc_t rc = 0;
    uint32_t rlen;
    int i;
    const int64_t *base;

    rc = fetch_mates( self, spotid,&base,&rlen );
    if ( rc != 0 )
        return rc;
    if ( rlen > 2 )
        return RC( rcXF, rcFunction, rcExecuting, rcConstraint, rcViolated );
    mateid[ 0 ] = 0;
    for( i = 0; i < ( int )rlen; i++ )
    {
        if ( base[ i ] != id )
        {
            mateid[ 0 ] = base[ i ];
        }
    }
    return rc;
}


#if 0
static rc_t get_mate_algn_id_impl( ID_cache_t *const self,
                                   int64_t const row,
                                   int64_t rslt[],
                                   int64_t const spotid )
{
    rc_t rc = 0;
    bool page_processed = false;
    int64_t first, first_blob, last, last_blob;

    /** check cache **/
    rc = lookup_mate( self, row, rslt );
    if ( rc == 0 )
        return 0;

    /** Page boundary is defined by the variable "first" **/
    rc = VCursorPageIdRange( self->curs, self->idx, spotid, &first_blob, &last_blob );
    if ( rc != 0 )
        return rc;
    first = spotid & ~( TMP_CACHE_PAGESIZE - 1 );
    if ( first < first_blob )
        first = first_blob;
    last = first + TMP_CACHE_PAGESIZE -1;
    if ( last > last_blob )
        last = last_blob;

    rc = KVectorGetBool( self->p_cache, ( uint64_t )first, &page_processed );
    if ( rc == 0 && page_processed )
    {
        /*** This page was processed before. Row did not need caching. Just fetch Row being asked **/
        return fetch_mate_id( self, spotid, row, rslt );
    }
    else
    {
        /** unprocessed page **/
        int64_t i;
		rc = KVectorSetBool( self->p_cache, ( uint64_t )first, true ); /** mark as processed **/
        if( rc != 0 )
            return rc;
	
        for ( i = first; i <= last; i++ )
        {
            int64_t	ids[ 2 ] = { 0, 0 };
            const int64_t *ptr;
            uint32_t rlen;
            rc = fetch_mates( self, i, &ptr, &rlen );
            if ( rc != 0 )
            {
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;
                else
                    return rc;
            }
            memmove( ids, ptr, rlen * sizeof( *ptr ) );
            if ( ids[ 0 ] != 0 && ids[ 1 ] != 0 && labs( ids[ 0 ] - ids[ 1 ] ) > MIN_DIFF_TO_CACHE )
            {
                rc = cache_mate( ( ID_cache_t* )self, ids[ 0 ], ids[ 1 ] );
                if( rc != 0 )
                    return rc;
                rc = cache_mate( ( ID_cache_t* )self, ids[ 1 ], ids[ 0 ] );
                if( rc != 0 )
                    return rc;
            }
            if ( i == spotid )
            {
                if( ids[ 0 ] == row )
                {
                    rslt[ 0 ] = ids[ 1 ];
                }
                else if( ids[ 1 ] == row )
                {
                    rslt[ 0 ] = ids[ 0 ];
                }
                else
                {
                    return RC( rcXF, rcFunction, rcExecuting, rcData, rcInconsistent );
                } 
            }
        }
    }
    return rc;
}
#endif


static rc_t CC get_mate_algn_id_drvr( void * Self,
                                      const VXformInfo * info,
                                      int64_t row_id,
                                      VRowResult *rslt,
                                      uint32_t argc,
                                      const VRowData argv [] )
{
    rc_t rc;
    ID_cache_t *self = Self;

    rslt->data->elem_bits = rslt->elem_bits;
    rc = KDataBufferResize( rslt->data, 1 );
    if ( rc == 0 )
    {
#if 0
        rc = get_mate_algn_id_impl( self,
                                    row_id,
                                    rslt->data->base,
                                    ( ( int64_t const * )argv[ 0 ].u.data.base )[ argv[ 0 ].u.data.first_elem ] );
#else
        rc = fetch_mate_id( self,
                            ( ( int64_t const * )argv[ 0 ].u.data.base )[ argv[ 0 ].u.data.first_elem ],
                            row_id,
                            rslt->data->base );
#endif
        if ( rc == 0 )
        {
            if ( *( int64_t* )rslt->data->base == 0 )
                rslt->elem_count = 0;
            else
                rslt->elem_count = 1;
        }
    }
    return rc;
}


/* open_sub_cursor */
static rc_t open_sub_cursor( ID_cache_t *self, const VXfactInfo *info, const VCursor *native_curs )
{
    rc_t rc;

    rc = VCursorLinkedCursorGet( native_curs, "SEQUENCE", &self->curs );
    if ( rc != 0 )
    {
        const VDatabase *db;
        const VTable *tbl;
        /* get at the parent database */
        rc = VTableOpenParentRead ( info -> tbl, & db );
        if ( rc != 0 )
            return rc;
        /* open the table */
        rc = VDatabaseOpenTableRead ( db, &tbl, "SEQUENCE" );
        VDatabaseRelease ( db );
        if ( rc != 0 )
            return rc;
        /* create a cursor */
        rc = VTableCreateCachedCursorRead( tbl, &self->curs, 32*1024*1024 );
        VTableRelease( tbl );
        if ( rc != 0 )
            return rc;
        rc = VCursorPermitPostOpenAdd( self->curs );
        if ( rc != 0 )
            return rc;
        rc = VCursorOpen( self->curs );
        if ( rc != 0 )
            return rc;
        rc = VCursorLinkedCursorSet( native_curs, "SEQUENCE", self->curs );
        if ( rc != 0 )
            return rc;
    }

    rc = VCursorAddColumn( self->curs, &self->idx, "(I64)PRIMARY_ALIGNMENT_ID" );
    if ( rc == 0 )
    {
        rc = KVectorMake( &self->j_cache );
        if ( rc == 0 )
            rc = KVectorMake( &self->p_cache );
        if ( rc == 0 )
            return 0;
    }
    VCursorRelease( self->curs );
    return rc;
}


/* close_sub_cursor
 */
static void CC close_sub_cursor( void * data )
{
    ID_cache_t *self = data;

    VCursorRelease( self->curs );
    KVectorRelease( self->j_cache );
    KVectorRelease( self->p_cache );
    free( self );
}


/*
 * extern function I64[2] NCBI:align:get_mate_id #1 (I64 spotId);
 */
VTRANSFACT_IMPL( NCBI_align_get_mate_align_id, 1, 0, 0 ) ( const void * Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    ID_cache_t *self = calloc( 1, sizeof( *self ) );

    if ( self == NULL )
        return RC( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );

    SUB_DEBUG( ( "SUB.Make in 'get_mate_align_ids.c'\n" ) );

    rc = open_sub_cursor( self, info, ( const VCursor* )info->parms );
    if ( rc == 0 ) {
        rslt->self = self;
        rslt->u.rf = get_mate_algn_id_drvr;
        rslt->variant = vftIdDepRow;
        rslt->whack = close_sub_cursor;
    }
    else {
        free(self);
    }

    return rc;
}
