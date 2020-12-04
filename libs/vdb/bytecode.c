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

/* does not work without this!! */
#define USE_EUGENE 1

#include "bytecode.h"

#include "prod-priv.h"

/********************************************************************************/
/* temporary copies of some static functions from prod-cmn.c */
#include "cursor-priv.h"
#include "page-map.h"

static
void CC vblob_release ( void *item, void *ignore )
{
    TRACK_BLOB ( VBlobRelease, ( VBlob* ) item );
    VBlobRelease ( ( VBlob* ) item );
}

static
rc_t VSimpleProdPage2Blob ( VSimpleProd *self, VBlob **rslt, int64_t id ,uint32_t cnt)
{
    return VProductionReadBlob(self->in, rslt, & id, cnt, NULL);
}

static
rc_t VSimpleProdSerial2Blob ( VSimpleProd *self, VBlob **rslt, int64_t id, uint32_t cnt )
{
    /* read serialized blob */
    VBlob *sblob;
    rc_t rc = VProductionReadBlob ( self -> in, &sblob, &id, cnt, NULL );
    if ( rc == 0 )
    {
        /* recast data to 8 bit */
        KDataBuffer buffer;
        rc = KDataBufferCast ( & sblob -> data, & buffer, 8, false );
        if (rc == 0)
        {
            /* create a new, fluffy blob having rowmap and headers */
            VBlob *y;
#if LAUNCH_PAGEMAP_THREAD
            if(self->curs->pagemap_thread == NULL){
                VCursor *curs = (VCursor*) self->curs;
                if(--curs->launch_cnt<=0){
                    /* ignoring errors because we operate with or without thread */
                    VCursorLaunchPagemapThread(curs);
                }
            }
#endif

            rc = VBlobCreateFromData ( & y, sblob -> start_id, sblob -> stop_id,
                & buffer, VTypedescSizeof ( & self -> dad . desc ),
                VCursorPageMapProcessRequest ( self->curs ) );
            KDataBufferWhack ( & buffer );

            /* return on success */
            if ( rc == 0 )
                * rslt = y;
        }

	vblob_release(sblob, NULL);
    }

    return rc;
}


static
rc_t VSimpleProdBlob2Serial( VSimpleProd *self, VBlob **rslt, int64_t id, uint32_t cnt )
{
    rc_t rc;
    VBlob *sblob;

    rc = VProductionReadBlob(self->in, &sblob, &id, cnt, NULL);
    if (rc == 0) {
        VBlob *y;

        rc = VBlobNew(&y, sblob->start_id, sblob->stop_id, "blob2serial");
        TRACK_BLOB (VBlobNew, y);
        if (rc == 0) {
            rc = KDataBufferMakeBytes(&y->data, 0);
            if (rc == 0) {
                /* save a reference to the page map so that fixed row-length can be determined */
                y->pm = sblob->pm;
                PageMapAddRef(y->pm);

                rc = VBlobSerialize(sblob, &y->data);
                if (rc == 0)
                    * rslt = y;
            }
            if (rc)
	      vblob_release(y, NULL);
        }

	vblob_release(sblob, NULL);
    }
    return rc;
}

/********************************************************************************/

rc_t ByteCode_ProductionReadBlob ( struct VProduction * self, struct ByteCodeContext * ctx )
{
    ctx -> rc = ExecuteByteCode ( bcPre_ProductionReadBlob, self, ctx );
    if ( ctx -> rc == 0 )
    {
        if ( ctx -> result != NULL )
        {
            return 0;
        }

        switch ( self -> var )
        {
        case prodSimple:
            ctx -> rc = ExecuteByteCode ( bcSimpleProdRead, self, ctx );
            break;
        case prodFunc:
            ctx ->rc = VFunctionProdRead ( ( VFunctionProd* ) self, & ctx -> result, ctx -> id , ctx -> cnt);
            break;
            /*ctx -> rc = ExecuteByteCode ( bcFunctionProdRead,  self, ctx ); break;*/
        case prodScript:
            ctx ->rc = VScriptProdRead ( ( VScriptProd* ) self, & ctx -> result, ctx -> id , ctx -> cnt);
            break;
            /*ctx -> rc = ExecuteByteCode ( bcScriptProdRead,    self, ctx ); break;*/
        case prodPhysical:
            ctx ->rc = VPhysicalProdRead ( ( VPhysicalProd* ) self, & ctx -> result, ctx -> id, ctx -> cnt );
            break;
            /*ctx -> rc = ExecuteByteCode ( bcPhysicalProdRead,  self, ctx ); break;*/
        case prodColumn:
            ctx ->rc = VColumnProdRead ( ( VColumnProd* ) self, & ctx -> result, ctx -> id );
            break;
            /*ctx -> rc = ExecuteByteCode ( bcColumnProdRead,    self, ctx ); break;*/
        case prodPivot:
            ctx ->rc = VPivotProdRead ( ( VPivotProd* ) self, & ctx -> result, & ctx -> id, ctx -> cnt );
            /*ctx -> rc = ExecuteByteCode ( bcPivotProdRead, (VPivotProd*)self, & ctx );*/
            break;
        default:
            ctx -> rc = RC ( rcVDB, rcProduction, rcReading, rcType, rcUnknown );
        }
        if ( ctx -> rc == 0 )
        {
            ctx -> rc = ExecuteByteCode ( bcPost_ProductionReadBlob, self, ctx );
        }
    }
    return ctx -> rc;
}

rc_t ByteCode_Pre_ProductionReadBlob ( struct VProduction * self, struct ByteCodeContext * ctx )
{
    int i;

    ctx -> result = NULL;

    /* should not be possible, but safety is cheap */
    if ( self == NULL )
        return RC ( rcVDB, rcProduction, rcReading, rcSelf, rcNull );

    /*** Cursor-level column blobs may be 1-to-1 with production blobs ***/
    if ( ctx -> cctx != NULL && self -> cctx . cache == NULL )
    { /*** we are connected to read cursor **/
        self -> cctx = * ctx -> cctx; /*** remember it ***/
        /** No need to do anything else here - we are on "direct line" to the column ***/
    }
    else if ( self -> cctx . cache != NULL )
    {
        /** somewhere else this production is connected to a cursor **/
        /** lets try to get answers from the cursor **/
        VBlob * blob = ( VBlob * ) VBlobMRUCacheFind ( self -> cctx . cache, self -> cctx . col_idx, ctx -> id );
        if ( blob )
        {
            ctx -> rc = VBlobAddRef ( blob );
            if ( ctx -> rc != 0 )
                return ctx -> rc;
            ctx -> result = blob;
            return 0;
        }
    }

#if PROD_CACHE
    /* check cache */
    for ( i = 0; i < self -> cache_cnt; ++ i )
    {
        VBlob * blob = self -> cache [ i ];
        if ( self -> cache [ i ] != NULL )
        {
            /* check id range */
            if (
#if USE_EUGENE
                /* NB - this is an approach where we always cache
                a blob after a read in order to keep it alive,
                but never allow a cache hit on retrieval */
                ! blob -> no_cache &&
#endif
                ctx -> id >= blob -> start_id &&
                ctx -> id <= blob -> stop_id )
            {
                ctx -> rc = VBlobAddRef ( blob );
                if ( ctx -> rc != 0 )
                    return ctx -> rc;
#if TRACKING_BLOBS
                fprintf( stderr, "%p->%p(%d) new reference to cached blob *** %s\n"
                        , self
                        , blob
                        , atomic32_read ( & blob -> refcount )
                        , self->name
                );
#endif
                /* return new reference */
                ctx -> result = blob;
#if PROD_CACHE > 1
#if PROD_CACHE > 2
                /* MRU cache */
                if ( i > 0 )
                {
                    memmove(self -> cache +1,self -> cache,i*sizeof(*self->cache));
                    self -> cache [ 0 ] = blob;
                }
#else
                if(i > 0 ){  /** trivial case ***/
                    self -> cache [ 1 ] =  self -> cache [ 0 ];
                    self -> cache [ 0 ] = blob;
                }
#endif
#endif
                return 0;
            }
        }
    }
#endif /* PROD_CACHE */
    return 0; /* not in cache */
}

rc_t ByteCode_Post_ProductionReadBlob ( struct VProduction * self, struct ByteCodeContext * ctx )
{
    rc_t rc;
    VBlob * blob = ctx -> result;

    if ( ctx -> rc != 0 || blob == NULL )
        return ctx -> rc;

    #if ! USE_EUGENE
            /* NB - there is another caching mechanism on VColumn
                if a blob does not want to be cached, it is rejected here */
        if ( ! blob -> no_cache )
            return 0;
    #endif
        if ( ctx -> cctx == NULL &&
                self -> cctx.cache != NULL &&
                blob -> stop_id > blob -> start_id + 4 )
        {   /** we will benefit from caching here **/
            VBlobMRUCacheSave ( self -> cctx . cache, self -> cctx . col_idx, blob );
            return 0;
        }

        if ( blob -> pm == NULL )
            return 0;

        /* cache output */
        rc = VBlobAddRef ( blob );
        if ( rc == 0 )
        {
            VBlobCheckIntegrity ( blob );
            if ( self -> cache_cnt < PROD_CACHE )
            {
    #if PROD_CACHE > 1
                if ( self -> cache_cnt > 0 )
                {
    #if PROD_CACHE > 2
                    memmove ( self -> cache + 1, self -> cache , self -> cache_cnt * sizeof(*self -> cache) );
    #else
                    self -> cache[1]=self -> cache[0];
    #endif
                }
    #endif
                self -> cache_cnt ++;
            }
            else
            {
                /* release whatever was there previously */
                /* drop LRU */
                VBlobRelease ( self -> cache [ self -> cache_cnt - 1 ] );
    #if PROD_CACHE > 1
    #if PROD_CACHE > 2
                memmove ( self -> cache + 1, self -> cache , (self -> cache_cnt -1) * sizeof(*self -> cache) );
    #else
                self -> cache [ 1 ] = self -> cache [ 0 ];
    #endif
    #endif
            }
            /* insert a head of list */
            self -> cache [ 0 ] = blob;

    #if TRACKING_BLOBS
            fprintf( stderr, "%p->%p(%d) cached *** %s\n"
                    , self
                    , blob
                    , atomic32_read ( & blob -> refcount )
                    , self -> name
                );
    #endif
        }

    #if USE_EUGENE
        /* this code requires the blob to be cached on the production */
        return rc;
    #else
        /* we don't care if the blob was not cached */
        return 0;
    #endif
}

rc_t ByteCode_SimpleProdRead ( struct VProduction * p_self, struct ByteCodeContext * ctx )
{
    VSimpleProd *self = (VSimpleProd *)p_self;
    switch ( p_self -> sub )
    {
    case prodSimpleCast:
        ctx -> rc = ExecuteByteCode ( bcProductionReadBlob, self -> in, ctx );
        if ( ctx -> rc == 0 )
        {
            ctx -> rc = ExecuteByteCode ( bcPost_SimpleProdRead, p_self, ctx );
        }
        break;
    case prodSimplePage2Blob:
        return VSimpleProdPage2Blob ( self, & ctx -> result, ctx -> id, ctx -> cnt );
            /* return ExecuteByteCode ( bcSimpleProdPage2Blob, p_self, ctx ); */
    case prodSimpleSerial2Blob:
        return VSimpleProdSerial2Blob ( self, & ctx -> result, ctx -> id, ctx -> cnt );
            /* return ExecuteByteCode ( bcSimpleProdSerial2Blob, p_self, ctx ); */
    case prodSimpleBlob2Serial:
        return VSimpleProdBlob2Serial ( self, & ctx -> result, ctx -> id, ctx -> cnt );
            /* return ExecuteByteCode ( bcSimpleProdBlob2Serial, p_self, ctx ); */
    default:
        ctx -> rc = RC ( rcVDB, rcProduction, rcReading, rcProduction, rcCorrupt );
        break;
    }
    return ctx -> rc;
}

rc_t ByteCode_Post_SimpleProdRead ( struct VProduction * self, struct ByteCodeContext * ctx )
{
    /* force data buffer to reflect element size */
    if ( self -> fd . fmt == 0 &&
            self -> fd . td . type_id > 2 )
    {
        uint32_t elem_bits = VTypedescSizeof ( & self ->  desc );
        if ( elem_bits != 0 && ctx -> result -> data . elem_bits != elem_bits )
        {
            ctx -> rc = KDataBufferCast ( & ctx -> result -> data, & ctx -> result -> data, elem_bits, true );
            if ( ctx -> rc != 0 )
            {
                VBlobRelease ( ctx -> result );
                ctx -> result = NULL;
            }
        }
    }
    return ctx -> rc;
}

ByteCodeTable_t ByteCode_Default =
{
    ByteCode_ProductionReadBlob,
    ByteCode_Pre_ProductionReadBlob,
    ByteCode_Post_ProductionReadBlob,
    ByteCode_SimpleProdRead,
    ByteCode_Post_SimpleProdRead,
    NULL/*bcFunctionProdRead*/,
    NULL/*bcFunctionProdSelect*/,
    NULL/*bcFunctionProdReadNormal*/,
    NULL/*bcPre_FunctionProdReadNormal*/,
    NULL/*bcPost_FunctionProdReadNormal*/,
    NULL/*bcScriptProdRead*/,
    NULL/*bcPhysicalProdRead*/,
    NULL/*bcColumnProdRead*/,
    NULL/*bcSimpleProdPage2Blob*/,
    NULL/*bcSimpleProdSerial2Blob*/,
    NULL/*bcSimpleProdBlob2Serial*/,
    NULL/*bcFunctionProdCallCompare*/,
    NULL/*bcFunctionProdCallCompare1*/,
    NULL/*bcFunctionProdCallLegacyBlobFunc*/,
    NULL/*bcFunctionProdCallNDRowFunc*/,
    NULL/*bcFunctionProdCallRowFunc*/,
    NULL/*bcFunctionProdCallArrayFunc*/,
    NULL/*bcFunctionProdCallPageFunc*/,
    NULL/*bcFunctionProdCallBlobFunc*/,
    NULL/*bcPre_FunctionProdCallBlobFunc*/,
    NULL/*bcPost_FunctionProdCallBlobFunc*/,
    NULL/*bcFunctionProdCallBlobNFunc*/,
    NULL/*bcFunctionProdCallByteswap*/,
    NULL/*bcPhysicalRead*/,
    NULL/*bcWPhysicalRead*/,
    NULL/*bcPhysicalReadKColumn*/,
    NULL/*bcWColumnReadBlob*/,
    NULL/*bcFunctionProdCallBlobFuncEncoding*/,
    NULL/*bcFunctionProdCallBlobFuncDecoding*/,
    NULL/*bcPhysicalReadBlob*/,
    NULL/*bcPhysicalWrite*/,
    NULL/*bcPre_PhysicalWrite*/,
    NULL/*bcPost_PhysicalWrite*/,
    NULL/*bcPhysicalConvertStatic*/,
    NULL/*bcPre_PhysicalConvertStatic*/,
    NULL/*bcPost_PhysicalConvertStatic*/,
    NULL/*bcPhysicalWriteKColumn*/,
    NULL/*bcPhysicalReadStatic*/,
    NULL/*bcReturn*/,
};

rc_t ExecuteByteCode ( enum ByteCodeOperation p_op, VProduction * p_prod, ByteCodeContext * p_ctx )
{
    if ( p_op >= bcEnd )
    {
        return RC ( rcVDB, rcProduction, rcReading, rcMessage, rcInvalid );
    }
    else
    {
        ByteCodeOperation_fn fn = ByteCode_Default [ p_op ];
        if ( fn == NULL )
        {
            return RC ( rcVDB, rcProduction, rcReading, rcMessage, rcUndefined );
        }
        return fn ( p_prod, p_ctx );
    }
}

