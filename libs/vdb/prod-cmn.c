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
#define BYTECODE 0

#define USE_EUGENE 1


#define TRACK_REFERENCES 0

#include <vdb/extern.h>

#define KONST const
#include "prod-priv.h"
#include "prod-expr.h"
#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-expr.h"
#include "table-priv.h"
#include "cursor-priv.h"
#include "linker-priv.h"
#include "column-priv.h"
#include "phys-priv.h"
#include "blob-priv.h"
#include "blob.h"
#include "page-map.h"
#include "blob-headers.h"
#undef KONST

#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <vdb/xform.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <ctype.h>
#include <os-native.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <bitstr.h>
#include <stdio.h>
#include <limits.h>

#include "bytecode.h"

#if !defined(WINDOWS)  &&  !defined(_WIN32)  &&  !defined(NCBI_WITHOUT_MT)
#define LAUNCH_PAGEMAP_THREAD 1
#endif


/*--------------------------------------------------------------------------
 * VBlob
 */


static
void CC vblob_release ( void *item, void *ignore )
{
    TRACK_BLOB ( VBlobRelease, ( VBlob* ) item );
    VBlobRelease ( ( VBlob* ) item );
}

/*--------------------------------------------------------------------------
 * VProduction
 */


/* Make
 *  allocation and parent initialization
 *  called from the "Make" functions below
 *
 *  "prod" [ OUT ] - returned production
 *
 *  "size" [ IN ] - sizeof sub-type
 *
 *  "owned" [ IN ] - vector to contain productions
 *
 *  "var" [ IN ] - variant code, e.g. prodSimple, prodFunc
 *
 *  "sub" [ IN ] - sub-variant code, e.g. prodSimplePage2Blob
 *
 *  "name" [ IN, NULL OKAY ] - optional object name, derived
 *  from schema if possible
 *
 *  "fd" [ IN ] and "desc" [ IN ] - production type description
 *
 *  "cid" [ IN ] - contextual ( two-level ) id
 *
 *  "chain" [ IN ] - which chain(s) are supported
 *    chainEncoding    - when going from input to physical
 *    chainDecoding    - when pulling from physical to output
 *    chainUncommitted - when resolving trigger and validation expressions
 */
rc_t VProductionMake ( VProduction **prodp, Vector *owned, size_t size,
    int var, int sub, const char *name, const VFormatdecl *fd,
    const VTypedesc *desc, const VCtxId *cid, uint8_t chain )
{
    rc_t rc;
    VProduction *prod;

#if PROD_NAME
    size_t psize = size;
#endif

    assert ( size >= sizeof * prod );

#if PROD_NAME
    if ( name != NULL )
        size += strlen ( name );
    size += 1;
#endif

    prod = calloc ( 1, size );
    if ( prod == NULL )
        rc = RC ( rcVDB, rcProduction, rcResolving, rcMemory, rcExhausted );
    else
    {
        rc = VectorAppend ( owned, & prod -> oid, prod );
        if ( rc != 0 )
        {
            free ( prod );
            prod = NULL;
        }
        else
        {
#if PROD_NAME
            prod -> name = ( ( const char* ) prod ) + psize;
            strcpy ( ( char* ) prod -> name, name ? name : "" );
#endif

            if ( fd != NULL )
                prod -> fd = * fd;
            if ( desc != NULL )
                prod -> desc = * desc;
            if ( cid != NULL )
                prod -> cid = * cid;

            prod -> var = ( uint8_t ) var;
            prod -> sub = ( uint8_t ) sub;
            prod -> chain = chain;
        }
    }

    * prodp = prod;
    return rc;
}

#if PROD_CACHE
static
void VProductionFlushCacheDeep ( VProduction *self, const char *context )
{
    int i;
    for ( i = 0; i < self -> cache_cnt; ++ i )
    {
#if TRACKING_BLOBS
        if ( self -> cache [ i ] != NULL )
        {
            fprintf( stderr, "%p->%p(%d) dropped from cache on %s *** %s\n"
                     , self
                     , self -> cache [ i ]
                     , atomic32_read ( & self -> cache -> refcount )
                     , context
                     , self -> name
                );
        }
#endif
        vblob_release ( self -> cache [ i ], NULL );
        self -> cache [ i ] = NULL;
    }
}
#endif


/*--------------------------------------------------------------------------
 * VSimpleProd
 *  single input param
 */

/* Make
 */
rc_t VSimpleProdMake ( VProduction **prodp, Vector *owned, struct VCursor const *curs,
   int sub, const char *name, const VFormatdecl *fd, const VTypedesc *desc,
    const VCtxId *cid, VProduction *in, uint8_t chain )
{
    VSimpleProd *prod;
    rc_t rc = VProductionMake ( prodp, owned, sizeof * prod,
        prodSimple, sub, name, fd, desc, cid, chain );
    if ( rc == 0 )
    {
        prod = ( VSimpleProd* ) * prodp;
        prod -> in = in;
        prod -> curs = curs;
    }
    return rc;
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
    rc_t rc = VProductionReadBlob ( self -> in, &sblob, & id, cnt, NULL );
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
            VCursorLaunchPagemapThread ( (VCursor*) self -> curs );
#endif

            rc = VBlobCreateFromData ( & y, sblob -> start_id, sblob -> stop_id,
                & buffer, VTypedescSizeof ( & self -> dad . desc ), VCursorPageMapProcessRequest( self -> curs ) );
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

    rc = VProductionReadBlob(self->in, &sblob, & id, cnt, NULL);
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

/* Read
 *  return a blob containing row id
 */
rc_t VSimpleProdRead ( VSimpleProd *self, VBlob **vblob, int64_t * id, uint32_t cnt, VBlobMRUCacheCursorContext *cctx)
{
    rc_t rc;

    switch ( self -> dad . sub )
    {
    case prodSimpleCast:
        rc = VProductionReadBlob ( self -> in, vblob, id, cnt, cctx ); /* *id may change here */
        break;
    case prodSimplePage2Blob:
        return VSimpleProdPage2Blob(self, vblob, * id, cnt);
    case prodSimpleSerial2Blob:
        return VSimpleProdSerial2Blob(self, vblob, * id, cnt);
    case prodSimpleBlob2Serial:
        return VSimpleProdBlob2Serial(self, vblob, * id, cnt);
    default:
        * vblob = NULL;
        return RC ( rcVDB, rcProduction, rcReading, rcProduction, rcCorrupt );
    }

    if ( rc == 0 )
    {
        VBlob *blob = * vblob;

        /* force data buffer to reflect element size */
        if ( self -> dad . fd . fmt == 0 &&
             self -> dad . fd . td . type_id > 2 )
        {
            uint32_t elem_bits = VTypedescSizeof ( & self -> dad . desc );
            if ( elem_bits != 0 && blob -> data . elem_bits != elem_bits )
            {
                rc = KDataBufferCast ( & blob -> data, & blob -> data, elem_bits, true );
                if ( rc != 0 )
                {
                    vblob_release ( blob, NULL );
                    * vblob = NULL;
                }
            }
        }
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * VFunctionProd
 *  function input params are VProduction*
 *  extern C function pointer and self object
 */

rc_t VFunctionProdMake ( VFunctionProd **prodp, Vector *owned,
    const VCursor *curs, int sub, const char *name,
    const VFormatdecl *fd, const VTypedesc *desc, uint8_t chain )
{
    VFunctionProd *prod;
    rc_t rc = VProductionMake ( ( VProduction** ) prodp, owned,
        sizeof * prod, prodFunc, sub, name, fd, desc, NULL, chain );
    if ( rc == 0 )
    {
        prod = * prodp;
        prod -> curs = curs;

        if ( sub != prodFuncByteswap )
            VectorInit ( & prod -> parms, 0, 4 );
        else
        {
            SDatatype *dt = VSchemaFindTypeid ( VCursorGetSchema ( curs ), fd -> td . type_id );
            assert ( dt != NULL );
            prod -> u . bswap = dt -> byte_swap;
            VectorInit ( & prod -> parms, 0, 1 );
        }
    }
    return rc;
}

void VFunctionProdDestroy ( VFunctionProd *self )
{
    /* release input parameters */
    VectorWhack ( & self -> parms, NULL, NULL );
    if ( self -> whack != NULL )
        ( * self -> whack ) ( self -> fself );
}



/* Read
 */

#define VECTOR_ALLOC_ARRAY( ARGC, ARRAY, S_ARRAY, H_ARRAY )	\
    do { \
        size_t const needed = (ARGC) * sizeof((ARRAY)[0]); \
        (H_ARRAY) = NULL; \
        (ARRAY) = &((S_ARRAY)[0]); \
        if (needed > sizeof((S_ARRAY))) { \
            (H_ARRAY) = malloc(needed); \
            if ((H_ARRAY) == NULL) \
                return RC(rcVDB, rcProduction, rcReading, rcMemory, rcExhausted); \
            (ARRAY) = &((H_ARRAY)[0]); \
        } \
        memset((ARRAY), 0, needed); \
    } while (0)

#define VECTOR_COPY_TO_ARRAY( VECTOR, ARRAY )	\
    do { \
        int i, n; \
        for (n = (i = VectorStart((VECTOR))) + VectorLength((VECTOR)); i != n; ++i) \
            (ARRAY)[i] = VectorGet((VECTOR), i); \
    } while (0)

#define VECTOR_TO_ARRAY( ARGC, ARRAY, S_ARRAY, H_ARRAY, VECTOR )	\
    do { \
        VECTOR_ALLOC_ARRAY((ARGC), (ARRAY), (S_ARRAY), (H_ARRAY)); \
        VECTOR_COPY_TO_ARRAY((VECTOR), (ARRAY)); \
    } while (0)

static
rc_t VFunctionProdCallNDRowFunc(
                                VFunctionProd *self,
                                VBlob **prslt,
                                int64_t row_id,
                                const VXformInfo *info,
                                Vector *args
                                )
{
    rc_t rc;

    /* create output blob
       TBD - try to used cached blob if available */
#if PROD_NAME
    rc = VBlobNew ( prslt, row_id, row_id, self->dad.name );
#else
    rc = VBlobNew ( prslt, row_id, row_id, "VFunctionProdCallNDRowFunc" );
#endif
    TRACK_BLOB ( VBlobNew, *prslt );
    if ( rc == 0 )
    {
        VRowResult rslt;
        VRowData on_stack [ 16 ], *on_heap, *argv;

        VBlob *blob = * prslt;
        uint32_t i, argc = VectorLength ( args );

        /* create and populate array of input parameters */
        VECTOR_ALLOC_ARRAY(argc, argv, on_stack, on_heap);
        for ( i = 0; i < argc; ++ i )
        {
            const VBlob *in = VectorGet(args, i);
            uint32_t first_elem;

            /* always point to page base address */
            argv [ i ] . u . data . base = in -> data . base;

            /* get row length and starting element in one pass */
            argv [ i ] . u . data . elem_count = PageMapGetIdxRowInfo ( in -> pm, (uint32_t)( row_id - in -> start_id ), & first_elem, NULL );
            argv [ i ] . u . data . first_elem = first_elem;

            /* finally set the element size */
            argv [ i ] . u . data . elem_bits = in -> data . elem_bits;
        }

        /* fill out return param block
           NB - the initially passed-in buffer
           may be reallocated by external function */
        rslt . data =  & blob -> data;
        rslt . elem_count = 0;
        rslt . elem_bits = blob -> data . elem_bits =
            VTypedescSizeof ( & self -> dad . desc );
        rslt.no_cache = 0;

        blob -> byte_order = vboNative;

        /* invoke the row function */
        rc = self -> u . ndf ( self -> fself, info, row_id, & rslt, argc, argv );
        blob->no_cache = (rslt.no_cache ? true : false);

        /* clean up input arguments */
        if ( on_heap != NULL )
            free ( on_heap );

        /* take reallocated buffer */
        if ( rslt . data != & blob -> data )
        {
            KDataBufferWhack ( & blob -> data );
            KDataBufferSub ( rslt . data, & blob -> data, 0, UINT64_MAX );
            KDataBufferWhack ( rslt . data );
        }
        blob->data.elem_count = rslt.elem_count;

        /* if the function was successful incorporate row length */
        if (rc == 0)
        {
            assert(rslt . elem_count >> 32 == 0);
            rc = PageMapNewFixedRowLength ( & blob -> pm, 1, (uint32_t)rslt . elem_count );
            if ( rc == 0 )
                return 0;
        }

        vblob_release ( blob, NULL );

        *prslt = NULL;
    }

    return rc;
}

/* TODO: enable in next release */
#define PAGEMAP_PRE_EXPANDING_SINGLE_ROW_FIX 0
static
rc_t VFunctionProdCallNullaryRowFunc(VFunctionProd *self,
                                     VBlob **prslt,
                                     int64_t row_id,
                                     uint32_t row_count,
                                     const VXformInfo *info)
{
    rc_t rc = 0;
    KDataBuffer scratch;
    VRowData args[1];
    VRowResult rslt;

    memset(&scratch, 0, sizeof(scratch));
    memset(&args[0], 0, sizeof(args[0]));

    rslt.data = &scratch;
    rslt.elem_count = 0;
    rslt.elem_bits = scratch.elem_bits = VTypedescSizeof(&self->dad.desc);

    rc = self->u.rf(self->fself, info, row_id, &rslt, 0, args);
    if (rc == 0) {
        VBlob *blob = NULL;

#if PROD_NAME
        rc = VBlobNew ( &blob, -INT64_MAX - 1, INT64_MAX, self->dad.name );
#else
        rc = VBlobNew ( &blob, -INT64_MAX - 1, INT64_MAX, "VFunctionProdCallDetRowFunc" );
#endif
        if (rc == 0) {
            blob->byte_order = vboNative;
            assert(rslt.elem_count <= UINT32_MAX);
            KDataBufferSub(rslt.data, &blob->data, 0, rslt.elem_count);
            if ( rslt.data != & scratch )
                KDataBufferWhack(rslt.data);
            rc = PageMapNewSingle(&blob->pm, UINT32_MAX, (uint32_t)rslt.elem_count);
            if (rc == 0)
                *prslt = blob;
            else
                vblob_release(blob, NULL);
        }
    }
    KDataBufferWhack(&scratch);
    return rc;
}

static bool computeWindow(uint32_t *const pwindow, int64_t const start_id, int64_t const stop_id, int64_t const row_id, uint32_t const max_blob_regroup)
{
    int64_t window = stop_id - start_id + 1;
    bool window_resized = false;

    /*** from previous fetch **/

    /** detect sequentual io ***/
    if (row_id == stop_id + 1)
    {
        if (window > max_blob_regroup)
        {
            window = max_blob_regroup;
            window_resized = true;
        }
        else if (row_id % (4 * window) == 1)
        {
            if (window < max_blob_regroup)
            {
                if (4 * window <= max_blob_regroup)
                    window *= 4;
                else
                    window = max_blob_regroup;
                window_resized = true;
            }
            /* we know that row_id lands on the first row of the new window */
        }
    }
    else
    {
        /* random access - use tiny blob window */
        window = 1;
        window_resized = true;
    }
    assert(window <= UINT32_MAX);
    *pwindow = window;
    return window_resized;
}

#if UNIT_TEST_VDB_3058
static void UnitTest_VDB_3058(void)
{
/* test for case of integer overflow to zero causing divide-by-zero error in computeWindow */
    uint32_t max_blob_regroup = 1024;
    uint32_t window = 0;
    int64_t start_id = 1;
    int64_t stop_id = UINT32_MAX / 4 + 1;
    int64_t row_id = stop_id + 1;
    bool window_resized = computeWindow(&window, start_id, stop_id, row_id, max_blob_regroup);

    /* this output isn't important, it's just to make sure that
     * the compiler doesn't optimize out the relevant computations */
    printf("window: %u\nwindow_resized: %s\n", window, window_resized ? "true" : "false");
}
#endif

static
rc_t VFunctionProdCallRowFunc1( VFunctionProd *self, VBlob **prslt, int64_t row_id,
    uint32_t row_count, const VXformInfo *info, Vector *args,int64_t param_start_id,int64_t param_stop_id)
{
    rc_t rc;
    uint32_t i, argc = VectorLength ( args );
    VRowResult rslt;
    VRowData args_os[16], *args_oh, *argv;
    KDataBuffer scratch;
    VBlob *blob;
    const VBlob *in;
    PageMapIterator iter_os[16], *iter_oh, *iter;
    uint64_t last = 0;
    uint32_t last_len = 0;
    uint32_t window;
	uint32_t min_row_count=UINT32_MAX; /* will increase row_count due to larger common repeat count of parameters */
    int64_t  row_id_max=0;
    uint32_t max_blob_regroup; /** max rows in blob for regrouping ***/
    bool function_failed = false;
    bool window_resized = false;

    if ( VCursorCacheActive ( self -> curs, & row_id_max ) )
    {   /*** since cache_cursor exist, trying to avoid prefetching data which is in cache cursor ***/
		max_blob_regroup=256;
    } else
    {
		max_blob_regroup=1024;
    }

	if(self->dad.sub == vftRowFast)
    {
		window = max_blob_regroup;
	}
    else
    {
        window_resized = computeWindow(&window, self->start_id, self->stop_id, row_id, max_blob_regroup);
	}

    assert ( 0 < window && window <= max_blob_regroup );

    if(window == 1)
    {
        /* random access or initial blob - create blob with initial row-count */
		self->start_id=self->stop_id=row_id;
		if(row_count > 0)
            self->stop_id += row_count-1;
    }
    else
    {
        /* start out with supplied row range */
        self->start_id=param_start_id;
        self->stop_id =param_stop_id;
        assert(row_id >= self->start_id && row_id  + row_count -1 <= self->stop_id);

        /* special code to detect an old-style static column with infinite range */
        if(self->start_id==-INT64_MAX - 1 || self->stop_id==INT64_MAX)
        {
            /* same logic as above */
            self->start_id=self->stop_id=row_id;
            if(row_count > 0)
                self->stop_id += row_count-1;
        }

        /* this code only executes if requested row-count is 1 */
        else if ( row_count == 1 )
        {
            /* the blob itself has to be at least twice the window size.
              this may be a problem because once "window" becomes large enough,
              it could cause us to switch from "reblobbing" back to whole blob. */
            if ( self->stop_id - self->start_id > 2*window)
            {
                /* determine which "window" ( relative to start of TABLE ) contains row_id */
                int64_t	n=(row_id-1)/window;

                /* look at blob left edge, move up to window left edge if possible */
                if(self->start_id <= n*window)
                    self->start_id=n*window+1;

                /* look at blob right edge, move down to window right edge if possible */
                if(self->stop_id > (n+1) * window)
                    self->stop_id = (n+1)*window;

                /* eventual window from self->start_id..self->stop_id must be <= "window" size */
                assert ( self -> start_id <= self -> start_id );
                assert ( self -> stop_id - self -> start_id + 1 <= window );
            }
            else if ( window_resized )
            {
                self -> start_id = row_id;
            }
            else
            {
                /* handle case when the window has grown large enough
                   to disable reblobbing after blobbing was once in place */
            }
        }
    }

    /* create and populate array of input parameters */
    VECTOR_ALLOC_ARRAY(argc, argv, args_os, args_oh);
    VECTOR_ALLOC_ARRAY(argc, iter, iter_os, iter_oh);
    for (i = 0; i != argc; ++ i){

        in = VectorGet(args, i);

		if(in->start_id == -INT64_MAX - 1 ) {
			rc = PageMapNewIterator(in->pm, &iter[i],0,-1);
		} else if(param_stop_id>self->stop_id && param_stop_id < INT64_MAX){
			rc = PageMapNewIterator(in->pm, &iter[i], self->start_id-in->start_id, param_stop_id - self->start_id + 1);
			if(rc == 0){
				uint32_t n = PageMapIteratorRepeatCount(&iter[i]);
				if(n < min_row_count) min_row_count=n;
			}
		} else {
			rc = PageMapNewIterator(in->pm, &iter[i], self->start_id-in->start_id, self->stop_id - self->start_id + 1);
		}
        if ( rc ) break;
        argv[i].variant = vrdData;
        argv[i].blob_stop_id = in->stop_id;
        argv[i].u.data.elem_bits = in->data.elem_bits;
        argv[i].u.data.base = in->data.base;
        argv[i].u.data.base_elem_count = in->data.elem_count;
    }
	if(min_row_count < UINT32_MAX && self->start_id + min_row_count -1 > self->stop_id ){
		self->stop_id = self->start_id + min_row_count - 1;
	}
	if(row_id_max >= row_id && self->stop_id > row_id_max)     self->stop_id = row_id_max;




#if PROD_NAME
    rc = VBlobNew ( &blob, self->start_id, self->stop_id, self->dad.name );
#else
    rc = VBlobNew ( &blob, self->start_id, self->stop_id, "VFunctionProdCallDetRowFunc" );
#endif
    TRACK_BLOB ( VBlobNew, blob );
    if (rc)
        return rc;

#if PAGEMAP_PRE_EXPANDING_SINGLE_ROW_FIX
#else
    rc = PageMapNew(&blob->pm, row_count /**BlobRowCount(blob)**/);
#if 0
    /* disabled for causing problems with accumulating static columns */
    if (rc == 0)
		rc = PageMapPreExpandFull(blob->pm, BlobRowCount(blob));
#endif
    if (rc) {
      vblob_release(blob, NULL);
        return rc;
    }
#endif

    memset(&scratch, 0, sizeof(scratch));
    rslt.data = &scratch;
    rslt.elem_bits = scratch.elem_bits = blob->data.elem_bits = VTypedescSizeof(&self->dad.desc);
    blob->byte_order = vboNative;



    for (row_id = self->start_id; row_id <= self->stop_id && rc == 0; ) {
        uint32_t row_count = 1;
        if(self->dad.sub == vftRow || self->dad.sub ==vftRowFast ){
            row_count = PageMapIteratorRepeatCount(&iter[0]);

            for (i = 1; i != argc; ++i) {
                uint32_t j = PageMapIteratorRepeatCount(&iter[i]);
                if (row_count > j)
                row_count = j;
            }
            if (row_id + row_count > self->stop_id + 1)
            row_count = (uint32_t)( self->stop_id + 1 - row_id );
        }

        for (i = 0; i != argc; ++i) {
            argv[i].u.data.elem_count = PageMapIteratorDataLength(&iter[i]);
            argv[i].u.data.first_elem = PageMapIteratorDataOffset(&iter[i]);
        }

        rslt.elem_count = 0;
        rc = self->u.rf(self->fself, info, row_id, &rslt, argc, argv);
        if (rc) {
            function_failed = true;
            break;
        }

        assert(rslt.elem_count >> 32 == 0);

        if (row_id == self->start_id) {
#if PAGEMAP_PRE_EXPANDING_SINGLE_ROW_FIX
            if (blob->start_id + row_count > blob->stop_id) {
                last = blob->data.elem_count;
                rc = KDataBufferResize(&blob->data, blob->data.elem_count + rslt.elem_count);
                if (rc == 0) {
                    bitcpy(blob->data.base, last * rslt.elem_bits,
                           rslt.data->base, 0, rslt.elem_count * rslt.elem_bits);
                    rc = PageMapNewSingle(&blob->pm, row_count, rslt.elem_count);
                }
            }
            else
            goto ADD_ROW_TO_BLOB;
#else
            goto ADD_ROW_TO_BLOB;
#endif
        }
        else if (last_len != rslt.elem_count ||
                 bitcmp(blob->data.base, last * rslt.elem_bits,
                        rslt.data->base, 0, rslt.elem_count * rslt.elem_bits) != 0)
        {
        ADD_ROW_TO_BLOB:
            last = blob->data.elem_count;
            rc = KDataBufferResize(&blob->data, blob->data.elem_count + rslt.elem_count);
            if (rc == 0) {
                bitcpy(blob->data.base, last * rslt.elem_bits,
                       rslt.data->base, 0, rslt.elem_count * rslt.elem_bits);
                rc = PageMapAppendRows(blob->pm, rslt.elem_count, row_count, false);
            }
        }
        else
            rc = PageMapAppendRows(blob->pm, rslt.elem_count, row_count, true);

        /* drop any new buffer that was returned to us */
        if (rslt.data != &scratch) {
            KDataBufferWhack(rslt.data);
        }

        if (rc) break;

        last_len = (uint32_t)rslt.elem_count;

        for (i = 0; i != argc; ++i)
            PageMapIteratorAdvance(&iter[i], row_count);
        row_id += row_count;
    }
    KDataBufferWhack(&scratch);
    if (args_oh) free(args_oh);
    if (iter_oh) free(iter_oh);

    if (rc == 0 || (function_failed && row_id > self->start_id)) {
        *prslt = blob;
        return 0;
    }
    vblob_release(blob, NULL);

    return rc;
}

static
rc_t VFunctionProdCallRowFunc( VFunctionProd *self, VBlob **prslt, int64_t row_id,
    uint32_t row_count, const VXformInfo *info, Vector *args,int64_t param_start_id,int64_t param_stop_id)
{
    uint32_t const argc = VectorLength(args);

    if (argc == 0)
        return VFunctionProdCallNullaryRowFunc(self, prslt, row_id, row_count, info);
    else
        return VFunctionProdCallRowFunc1(self, prslt, row_id, row_count, info, args, param_start_id, param_stop_id);
}

static
rc_t VFunctionProdCallArrayFunc( VFunctionProd *self, VBlob **prslt,
    int64_t id, const VXformInfo *info, Vector *args ) {
    VBlob *rslt = 0;
    VBlob *sblob;
    rc_t rc;

    sblob = VectorGet(args, 0);
    assert(sblob);

#if PROD_NAME
    rc = VBlobNew(&rslt, sblob->start_id, sblob->stop_id, self->dad.name);
#else
    rc = VBlobNew(&rslt, sblob->start_id, sblob->stop_id, "VFunctionProdCallArrayFunc");
#endif
    TRACK_BLOB( VBlobNew, rslt );
    if (rc == 0) {
        rslt->pm = sblob->pm;
        PageMapAddRef(rslt->pm);

        if (sblob->headers) {
            if ( self -> dad . chain == chainEncoding )
                rc = BlobHeadersCreateChild(sblob->headers, &rslt->headers);
            else {
                rslt->headers = (BlobHeaders *)BlobHeadersGetNextFrame(sblob->headers);
                BlobHeadersAddRef(rslt->headers);
            }
        }
        if (rc == 0) {
	    rc = KDataBufferMake(&rslt->data, VTypedescSizeof(&self->dad.desc), sblob->data.elem_count);
            if (rc == 0) {
                rc = self->u.af(
                                self->fself,
                                info,
                                rslt->data.base,
                                sblob->data.base,
                                sblob->data.elem_count
                                );
                if (rc == 0) {
                    *prslt = rslt;
                    return 0;
                }
            }
        }

        vblob_release( rslt, NULL );
    }

    return rc;
}

static
rc_t VFunctionProdCallPageFunc( VFunctionProd *self, VBlob **rslt, int64_t id,
    const VXformInfo *info, Vector *args )
{
    struct input_t {
        const VBlob *blob;
        bool	sb_input;
        PageMapIterator cur_row;
        bool at_end;
    };
    struct input_t on_stack[8];
    struct input_t *on_heap;
    struct input_t *argv;

    VRowData pb_stack[8];
    VRowData *pb_heap;
    VRowData *param;

    rc_t rc=0;
    uint32_t i, argc = VectorLength(args);
    int64_t start_id;
    int64_t stop_id;
    uint32_t elem_count;
    uint32_t row_count;
    int first_non_control_input;
    int allInputsAreSingleRow;
    VBlob *blob = NULL;

    VECTOR_ALLOC_ARRAY(argc, argv, on_stack, on_heap);
    VECTOR_ALLOC_ARRAY(argc, param, pb_stack, pb_heap);

    for (start_id = stop_id = 0,
         first_non_control_input=-1,
         allInputsAreSingleRow = true,
         i = 0; i != argc ; ++i) {
        const VBlob *b = (const VBlob *)VectorGet(args, i);
        const VProduction *prod = (const VProduction *)VectorGet(&self->parms, i);


        if(b->pm == NULL){
            rc=PageMapProcessGetPagemap( VCursorPageMapProcessRequest ( self -> curs ) ,(PageMap**)(&b->pm));
            if(rc != 0) return rc;
        }

        if (prod->control) {
            param[i].variant = vrdControl;
            assert(0); /*** TBD: Not implemented ???? ***/
        } else {
            param[i].variant = vrdData;
            argv[i].blob = b;
            argv[i].sb_input = VBlobIsSingleRow(argv[i].blob);
            allInputsAreSingleRow &= argv[i].sb_input;

            if(first_non_control_input < 0){
                first_non_control_input = i;
                start_id = argv[i].blob->start_id;
                stop_id = argv[i].blob->stop_id;
            } else {
                if(start_id < argv[i].blob->start_id)
                    start_id = argv[i].blob->start_id;
                if(stop_id > argv[i].blob->stop_id)
                    stop_id  = argv[i].blob->stop_id;
            }
        }
    }
    if ( allInputsAreSingleRow ) {
	row_count = stop_id - start_id + 1;
	if(row_count == 0 ) /*** case of static column **/
		row_count=1;
    } else {
	row_count = stop_id - start_id + 1;
    }
    if (first_non_control_input < 0) /* no non-control inputs */
        rc = RC(rcVDB, rcFunction, rcExecuting, rcParam, rcInvalid);
    else if (start_id > stop_id )
        rc = RC(rcVDB, rcFunction, rcExecuting, rcRange, rcEmpty);

    for ( elem_count = 0, i = 0; i != argc && rc ==0; ++i) {
        rc = PageMapNewIterator(argv[i].blob->pm, &argv[i].cur_row, start_id - argv[i].blob->start_id ,row_count);
        if(rc == 0){
            PageMapIterator temp = argv[i].cur_row;
            uint32_t ec = 0;

            if ( argv[i].sb_input ){
                ec = PageMapIteratorDataLength(&temp) * row_count;
            } else do {
                ec+=PageMapIteratorDataLength(&temp);
            } while (PageMapIteratorNext(&temp));
            if(ec == 0){
                rc = RC(rcVDB, rcFunction, rcExecuting, rcPagemap, rcInvalid); /* bad page map */
            } else if (elem_count == 0){
                elem_count=ec;
            } else if (ec != elem_count){
                rc = RC(rcVDB, rcFunction, rcExecuting, rcParam, rcInvalid); /* Pages have to have the same number of elements*/
            }
    	}
    }

    while (rc == 0) /* not really while */ {
#if PROD_NAME
        rc = VBlobNew(&blob, start_id, stop_id, self->dad.name);
#else
        rc = VBlobNew(&blob, start_id, stop_id, "VFunctionProdCallPageFunc");
#endif
        if (rc) break;

        TRACK_BLOB(VBlobNew,blob);

        if (allInputsAreSingleRow) {
            VFixedRowResult rslt;
            uint32_t	row_element_count = PageMapIteratorDataLength(&argv[first_non_control_input].cur_row);

            rc = PageMapNewSingle(&blob->pm, row_count, row_element_count);
            if (rc) break;

            rc = KDataBufferMake(&blob->data, VTypedescSizeof(&self->dad.desc), row_element_count);
            if (rc) break;

            for (i = 0; i != argc; ++i) {
                if (param[i].variant == vrdControl)
                    continue;
                if (argv[i].at_end) {
                    rc = RC(rcVDB, rcFunction, rcExecuting, rcRow, rcNotFound);
                    break;
                }

                param[i].u.data.base = argv[i].blob->data.base;
                param[i].u.data.elem_count = row_element_count;
                param[i].u.data.first_elem = PageMapIteratorDataOffset(&argv[i].cur_row);
                param[i].u.data.elem_bits = argv[i].blob->data.elem_bits;

                argv[i].at_end = PageMapIteratorNext(&argv[i].cur_row) ? false : true;
            }
            if (rc) break;

            rslt.base = blob->data.base;
            rslt.first_elem = 0;
            rslt.elem_count = row_element_count;
            rslt.elem_bits = blob->data.elem_bits;

            rc = self->u.pf(self->fself, info, start_id, &rslt, argc, param);
        } else {
            uint32_t first_write;
            int64_t row_id;
            uint32_t last = 0;
            uint32_t last_rowlen = 0;

            rc = PageMapNew(&blob->pm, row_count); /*** max number of rows - it may collapse some **/
            if (rc) break;
            rc = KDataBufferMake(&blob->data, VTypedescSizeof(&self->dad.desc), elem_count);
            if (rc) break;

            for (first_write = 0, row_id = start_id; row_id <= stop_id; ++row_id) {
                VFixedRowResult rslt;

                for (i = 0; i != argc; ++i) {
                    if (param[i].variant == vrdControl)
                        continue;
                    if (argv[i].at_end) {
                        rc = RC(rcVDB, rcFunction, rcExecuting, rcRow, rcNotFound);
                        break;
                    }

                    param[i].u.data.base = argv[i].blob->data.base;
                    param[i].u.data.elem_count = PageMapIteratorDataLength(&argv[i].cur_row);
                    param[i].u.data.first_elem = PageMapIteratorDataOffset(&argv[i].cur_row);
                    param[i].u.data.elem_bits = argv[i].blob->data.elem_bits;

                    argv[i].at_end = PageMapIteratorNext(&argv[i].cur_row) ? false : true;
                }
                if (rc)
                    break;

                rslt.base = blob->data.base;
                rslt.first_elem = first_write;
                rslt.elem_count = param[first_non_control_input].u.data.elem_count;
                rslt.elem_bits = blob->data.elem_bits;

                rc = self->u.pf(self->fself, info, row_id, &rslt, argc, param);
                if (rc)
                    break;

                assert(rslt.elem_count >> 32 == 0);
                if ( row_id != start_id && last_rowlen == rslt.elem_count &&
                    memcmp(((char*)blob->data.base) + (last*rslt.elem_bits)/8,
                           ((char*)blob->data.base) + (first_write*rslt.elem_bits)/8,
                           (rslt.elem_count*rslt.elem_bits)/8) == 0)
                {
                    rc = PageMapAppendRow(blob->pm, (uint32_t)rslt.elem_count, true);
                }
                else {
                    last = first_write;
                    first_write += rslt.elem_count;
                    rc = PageMapAppendRow(blob->pm, (uint32_t)rslt.elem_count, false);
                }
                if (rc)
                    break;
                last_rowlen = (uint32_t)rslt.elem_count;
            }
            if (rc)
                break;
            KDataBufferSub(&blob->data, &blob->data, 0, first_write);
        }
        *rslt = blob;
        break;
    }
    if (rc != 0 && blob != NULL) vblob_release(blob, NULL);
    if (on_heap) free(on_heap);
    if (pb_heap) free(pb_heap);
    return rc;
}

static
rc_t VFunctionProdCallBlobFuncEncoding( VFunctionProd *self, VBlob *rslt, int64_t id,
    const VXformInfo *info, const VBlob *sblob ) {
    VBlobData src;
    VBlobResult dst;
    VBlobHeader *hdr;
    rc_t rc;
    uint32_t elem_size = VTypedescSizeof(&self->dad.desc);

    rc = BlobHeadersCreateChild(sblob->headers, &rslt->headers);
    if (rc == 0) {
        hdr = BlobHeadersGetHdrWrite(rslt->headers);
        if (hdr) {
            bitsz_t sz = KDataBufferBits(&sblob->data);

            VBlobHeaderSetSourceSize(hdr, KDataBufferBytes(&sblob->data));
            sz = (sz + elem_size - 1) / elem_size;
            rc = KDataBufferMake( &rslt->data, elem_size, sz );
        }
        else
            rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    }
    if (rc)
        return rc;

    dst.header = NULL;

    if ( sblob -> data.elem_count == 0)
        goto SKIP_COMPRESSION;

    src.data = sblob -> data.base;
    src.elem_count = sblob -> data.elem_count;
    src.elem_bits = sblob -> data.elem_bits;
    src.byte_order = sblob -> byte_order;

    dst.data = rslt -> data.base;
    dst.elem_count = rslt -> data.elem_count;
    dst.elem_bits = rslt -> data.elem_bits;
    dst.byte_order = sblob -> byte_order;

    rc = self->u.bf(self->fself, info, &dst, &src, hdr);

    if (rc == 0) {
        if ( dst.header != NULL && dst.header != hdr ) {
            VBlobHeaderReplace ( hdr, dst.header );
            VBlobHeaderRelease ( dst.header );
        }
        rslt->data.elem_bits = dst.elem_bits;
        rslt->data.elem_count = dst.elem_count;
        rslt->byte_order = dst.byte_order;
    }
    else if (GetRCObject(rc) == (enum RCObject)rcBuffer && GetRCState(rc) == rcInsufficient) {
    SKIP_COMPRESSION:
        VBlobHeaderSetFlags(hdr, 1);

        KDataBufferWhack(&rslt->data);
        if ( dst.header != NULL && dst.header != hdr )
            VBlobHeaderRelease ( dst.header );

        /* compressors usually produce bits (elem_size == 1) or bytes (elem_size == 8)
         * the cast to bits can never fail, so we will force the cast to bytes to also be
         * infallible; casts to other sizes are allowed to fail to prevent data loss */
        if (elem_size == 8) {
            KDataBufferSub(&sblob->data, &rslt->data, 0, UINT64_MAX);
            /* We can't shrink the data and KDataBufferCast won't increase the number of bits
             * but we know that KDataBuffer can't allocate anything other than whole bytes
             * so we're forcing the conversion to bytes manually. */
            rslt->data.elem_count = KDataBufferBytes(&rslt->data);
            rslt->data.elem_bits = 8;
            rc = 0;
        }
        else /* if elem_size == 1 this will always work */
            rc = KDataBufferCast(&sblob->data, &rslt->data, elem_size, false);
    }
    VBlobHeaderRelease(hdr);

    return rc;
}

static
rc_t VFunctionProdCallBlobFuncDecoding( VFunctionProd *self, VBlob *rslt,
    int64_t id, const VXformInfo *info, const VBlob *sblob ) {
    VBlobHeader *hdr;
    rc_t rc;
    uint32_t elem_size = VTypedescSizeof(&self->dad.desc);

    if (sblob->headers == NULL) {
        /* v1 blobs don't have headers, but v1 blobs
         * are fixed row-length so we know the data size
         * we are relying on the blob deserialization code
         * to set the page map up correctly */
        if (sblob->pm == NULL) {
            hdr = BlobHeadersCreateDummyHeader(0, 0, 0, (sblob->data.elem_bits * sblob->data.elem_count + 7) >> 3);
        }
        else
            hdr = BlobHeadersCreateDummyHeader(0, 0, 0, BlobRowCount(sblob) * PageMapGetIdxRowInfo(sblob->pm, 0, 0, NULL));
        /* leave rslt->headers null so that the next
         * stage will also create a dummy header */
    }
    else {
        /* rslt gets the headers for the next stage in decoding */
        rslt->headers = (BlobHeaders *)BlobHeadersGetNextFrame(sblob->headers);
        BlobHeadersAddRef(rslt->headers);

        /* get the headers for this stage in decoding */
        hdr = BlobHeadersGetHeader(sblob->headers);
    }
    if ( hdr == NULL )
        rc = RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    else if ((VBlobHeaderFlags(hdr) & 1) != 0)
    {
        /* compression was skipped */
        VBlobHeaderRelease(hdr);
        return KDataBufferCast(&sblob->data, &rslt->data, elem_size, true);
    }
    else
    {
        rc = KDataBufferMakeBytes(&rslt->data, VBlobHeaderSourceSize(hdr));
        if (rc == 0) {
            VBlobData src;
            VBlobResult dst;

            dst.header = NULL;

            src.data = sblob -> data.base;
            src.elem_count = sblob -> data.elem_count;
            src.elem_bits = sblob -> data.elem_bits;
            src.byte_order = sblob -> byte_order;

            dst.data = rslt -> data.base;
            dst.elem_count = (rslt -> data.elem_count << 3) / elem_size;
            dst.elem_bits = elem_size;
            dst.byte_order = sblob -> byte_order;

            rc = self->u.bf(self->fself, info, &dst, &src, hdr);

            if (rc == 0) {
                if ( dst.header != NULL && dst.header != hdr ) {
                    /* only allow replacement of headers when encoding */
                    VBlobHeaderRelease ( dst.header );
                }

                rslt->data.elem_bits = dst.elem_bits;
                rslt->data.elem_count = dst.elem_count;
                rslt->byte_order = dst.byte_order;

                rc = KDataBufferCast(&rslt->data, &rslt->data, elem_size, true);
            }
        }
        VBlobHeaderRelease(hdr);
    }

    return rc;
}

static
rc_t VFunctionProdCallBlobFunc( VFunctionProd *self, VBlob **prslt,
    int64_t id, const VXformInfo *info, Vector *args ) {
    VBlob *rslt = 0;
    VBlob *sblob;
    rc_t rc;

    sblob = VectorGet(args, 0);
    assert(sblob);
    if(self->dad.chain == chainEncoding){
	VBlobAddRef(sblob);
	if(sblob->headers==NULL)/**first in encryption chain***/
		VBlobPageMapOptimize(&sblob); /** try to optimize the blob **/
    }

#if PROD_NAME
    rc = VBlobNew(&rslt, sblob->start_id, sblob->stop_id, self->dad.name);
#else
    rc = VBlobNew(&rslt, sblob->start_id, sblob->stop_id, "VFunctionProdCallBlobFunc");
#endif
    if (rc)
        return rc;

    TRACK_BLOB(VBlobNew,rslt);

    /* blob funcs are not allowed to change page maps */
    rslt->pm = sblob->pm;
    PageMapAddRef(rslt->pm);

    rslt->byte_order = sblob->byte_order;

    if (self->dad.chain == chainEncoding){
        rc = VFunctionProdCallBlobFuncEncoding(self, rslt, id, info, sblob);
	vblob_release( sblob, NULL );
    } else {
        rc = VFunctionProdCallBlobFuncDecoding(self, rslt, id, info, sblob);
    }

    if (rc == 0) {
        *prslt = rslt;
        return 0;
    }
    vblob_release( rslt, NULL );
    return rc;
}

static
rc_t VFunctionProdCallBlobNFunc( VFunctionProd *self, VBlob **rslt,
    int64_t id, const VXformInfo *info, Vector *args ) {
    const VBlob *on_stack[16];
    const VBlob **on_heap;
    const VBlob **argv;
    int argc = VectorLength(args);
    rc_t rc;

    VECTOR_TO_ARRAY(argc, argv, on_stack, on_heap, args);
    {
	int i;
	for(i=0;i<argc;i++){
		VBlob const *vb=argv[i];
		if(vb->pm == NULL){
			rc=PageMapProcessGetPagemap( VCursorPageMapProcessRequest ( self -> curs ),(PageMap**)(&vb->pm));
			if(rc != 0) return rc;
		}
	}
    }
    rc = self->u.bfN(self->fself, info, id, rslt, argc, argv);
    if ( on_heap )
        free( (void*) on_heap );
    return rc;
}

static
rc_t VFunctionProdCallLegacyBlobFunc( VFunctionProd *self, VBlob **prslt,
    int64_t id, const VXformInfo *info, Vector *args ) {
    VBlob *rslt = 0;
    VBlob *sblob;
    VNoHdrBlobFunc func = (VNoHdrBlobFunc)self->u.bf;
    rc_t rc;
    uint32_t elem_size = VTypedescSizeof(&self->dad.desc);

    sblob = VectorGet(args, 0);
    assert(sblob);

#if PROD_NAME
    rc = VBlobNew(&rslt, sblob->start_id, sblob->stop_id, self->dad.name);
#else
    rc = VBlobNew(&rslt, sblob->start_id, sblob->stop_id, "VFunctionProdCallLegacyBlobFunc");
#endif
    TRACK_BLOB(VBlobNew,rslt);
    if (rc == 0) {
        rc = KDataBufferMakeBytes(&rslt->data, 0);
        if (rc == 0) {
            VLegacyBlobResult dst;
            dst.dst = & rslt -> data;
            dst.byte_order = vboLittleEndian;
            rc = func(self->fself,
                      info,
                      &dst,
                      &sblob->data
                      );

            if (rc == 0)
            {
                rslt->byte_order = dst.byte_order;

                rc = KDataBufferCast(&rslt->data, &rslt->data, elem_size, true);
                if (rc == 0) {
                    rslt->pm = sblob->pm;
                    PageMapAddRef(rslt->pm);

                    *prslt = rslt;
                    return 0;
                }
            }
        }

        vblob_release( rslt, NULL );
    }
    return rc;
}

static
rc_t VFunctionProdCallByteswap ( VFunctionProd *self, VBlob **vblob,
    int64_t id, const VXformInfo *info, Vector *args )
{
    /* get single input blob */
    VBlob *blob = VectorFirst ( args );
    rc_t rc;

    assert ( blob != NULL );

#if PROD_CACHE
    VProductionFlushCacheDeep ( & self -> dad, "byteswap" );
#endif

    /* CAST */
    rc = KDataBufferCast ( & blob->data, & blob->data,
                           self->dad.desc.intrinsic_bits * self->dad.desc.intrinsic_dim,
                           false );
    if ( rc == 0 )
    {
        /* legacy blob check
         * repair missing pagemap
         */
        if (blob->pm == NULL) {
            uint64_t row_count = BlobRowCount ( blob );
            if ( row_count == 0 || blob->data.elem_count % row_count != 0)
                rc = RC(rcVDB, rcBlob, rcReading, rcBlob, rcCorrupt);
            else {
                uint64_t row_len = blob->data.elem_count / row_count;
                rc = PageMapNewFixedRowLength(&blob->pm, row_count, row_len);
            }
        }
    }

    if ( rc != 0)
        return rc;

    /* check for byteswapping function */
    if ( self -> u.bswap != NULL )
    {

        if ( blob -> byte_order ==
#if __BYTE_ORDER == __LITTLE_ENDIAN
             vboBigEndian
#else
             vboLittleEndian
#endif
            )
        {
            uint32_t int_size;
            uint64_t blob_bits;

            /* make writable */
            KDataBuffer buffer;

            rc = KDataBufferMakeWritable ( & blob -> data, & buffer );
            if ( rc != 0 )
                return rc;

            /* invoke byte-swap function on input */
            blob_bits = KDataBufferBits ( & buffer );
            int_size = self -> dad.desc .intrinsic_bits;
            ( * self -> u.bswap ) ( buffer.base, buffer.base,
                ( uint32_t ) ( blob_bits / int_size ) );

            /* poke bytes back into blob */
            KDataBufferWhack ( & blob -> data );
            blob -> data = buffer;
        }
    }

    blob -> byte_order = vboNative;
    *vblob = blob;

    (void)VBlobAddRef ( blob );
    TRACK_BLOB( VBlobAddRef, blob );

    return 0;
}

#if _DEBUGGING

static
rc_t VFunctionProdCallCompare1(VFunctionProd *self, VBlob **vblob, int64_t id, uint32_t cnt) {
    VBlob *orig;
    rc_t rc;

    *vblob = NULL;
    assert(VectorLength(&self->parms) == 2);
    rc = VProductionReadBlob((const VProduction *)VectorGet(&self->parms, 0), &orig, &id, cnt, NULL);
    if (rc == 0) {
        int64_t i;
        PageMapIterator oi;
        VRowData orig_data;
        const VProduction *test_prod = VectorGet(&self->parms, 1);

        memset(&orig_data, 0, sizeof(orig_data));
        orig_data.u.data.base = orig->data.base;
        orig_data.u.data.elem_bits = orig->data.elem_bits;

        PageMapNewIterator(orig->pm, &oi, 0, -1);

        for (i = orig->start_id; i <= orig->stop_id; ++i) {
            VBlob *test;
            uint32_t j;

            j = PageMapIteratorDataLength(&oi);

            rc = VProductionReadBlob(test_prod, &test, &i, 1, NULL);
            if (rc == 0) {
                if (orig->data.elem_bits != test->data.elem_bits || orig->byte_order != test->byte_order)
                    rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                else {
                    PageMapIterator ti;
                    VRowData test_data;

                    memset(&test_data, 0, sizeof(test_data));
                    test_data.u.data.base = test->data.base;
                    test_data.u.data.elem_bits = test->data.elem_bits;

                    PageMapNewIterator(test->pm, &ti, 0, -1);

                    if (!PageMapIteratorAdvance(&ti, (uint32_t)(i - test->start_id))) {
                        rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                    }
                    else {
                        uint32_t k = PageMapIteratorDataLength(&ti);
                        orig_data.u.data.elem_count = test_data.u.data.elem_count = j;

                        orig_data.u.data.first_elem = (orig->data.bit_offset / orig->data.elem_bits) + PageMapIteratorDataOffset(&oi);
                        test_data.u.data.first_elem = (test->data.bit_offset / test->data.elem_bits) + PageMapIteratorDataOffset(&ti);

                        if (j != k) {
                            rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                        } else {
                            rc = self->u.cf(self->fself, &orig_data, &test_data);
                            if (rc) {
                                rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                            }
                        }
                        if (rc) {
                            const uint8_t *a = orig_data.u.data.base;
                            const uint8_t *b = test_data.u.data.base;
                            unsigned count;
                            unsigned k;
                            unsigned m;
                            char f, ax[9 + 16 * 4 + 1], bx[9 + 16 * 4 + 1];
                            char av[16], bv[16];

                            a += (orig_data.u.data.first_elem * orig_data.u.data.elem_bits) >> 3;
                            b += (test_data.u.data.first_elem * test_data.u.data.elem_bits) >> 3;
                            /* show up to a row of data before */
                            count = a - (const uint8_t*)orig_data.u.data.base;
                            count = count < b - (const uint8_t*)orig_data.u.data.base ? count : b - (const uint8_t*)orig_data.u.data.base;
                            count = count > 16 ? 16 : count;
                            a -= count;
                            b -= count;

                            count += (j * orig->data.elem_bits + 7) >> 3;

                            for (k = 0, m = 0; k != count; ++k) {
                                if (m == 0) {
                                    sprintf(ax, "%08X>", k);
                                    sprintf(bx, "%08X<", k);
                                }
                                f = a[k] == b[k] ? ' ': '*';
                                sprintf(ax + m * 4 + 9, " %02x%c", a[k], f);
                                av[m] = isprint(a[k]) ? a[k] : '.';
                                sprintf(bx + m * 4 + 9, " %02x%c", b[k], f);
                                bv[m] = isprint(b[k]) ? b[k] : '.';
                                m++;
                                if(m == 16 || k == count - 1) {
                                    DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%-73s '%.*s'\n%-73s '%.*s'\n\n", ax, m, av, bx, m, bv));
                                    m = 0;
                                }
                            }
                        }
                    }
                }
                vblob_release(test, NULL);
                if (rc)
                    break; }
            else
                break;
            PageMapIteratorAdvance(&oi, 1);
        }
        vblob_release(orig, NULL);
    }
    return rc;
}
#endif

static
rc_t VFunctionProdCallCompare( VFunctionProd *self, VBlob **vblob, int64_t id, uint32_t cnt ) {
    VBlob *orig;
    rc_t rc;
    VProduction const *orig_prod;

    *vblob = NULL;
    assert(VectorLength(&self->parms) == 2);
    orig_prod = (const VProduction *)VectorGet(&self->parms, 0);
    rc = VProductionReadBlob(orig_prod, &orig, &id, cnt, NULL);
    if (rc == 0) {
        VBlob *test;
        const VProduction *test_prod = VectorGet(&self->parms, 1);

        rc = VProductionReadBlob(test_prod, &test, &id, cnt, NULL);
        if (rc == 0) {
            if (orig->data.elem_bits != test->data.elem_bits || orig->byte_order != test->byte_order){
                rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
            } else if(   orig->pm->data_recs != 1				/*** catching static **/
                      || test->pm->data_recs != 1				/*** trying quick comparison **/
		      || orig->data.elem_count != test->data.elem_count
                      || memcmp( orig->data.base, test->data.base, (orig->data.elem_bits*orig->data.elem_count+7)/8)){
                uint64_t i;
                PageMapIterator oi;
                PageMapIterator ti;
                VRowData orig_data;
                VRowData test_data;

                memset(&orig_data, 0, sizeof(orig_data));
                orig_data.u.data.base = orig->data.base;
                orig_data.u.data.elem_bits = orig->data.elem_bits;

                memset(&test_data, 0, sizeof(test_data));
                test_data.u.data.base = test->data.base;
                test_data.u.data.elem_bits = test->data.elem_bits;

                PageMapNewIterator(orig->pm, &oi, 0, -1);
                PageMapNewIterator(test->pm, &ti, 0, -1);
                if (test->start_id < orig->start_id) {
                    if ( !PageMapIteratorAdvance( &ti, (uint32_t)( orig->start_id - test->start_id ) ) ) {
                        rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                        DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%s: page map mismatch at row %li\n", self->dad.name, id));
                    }
                }

                for (i = orig->start_id; rc == 0; ) {
                    uint32_t elem_count;
                    uint64_t prev_i = i;

                    orig_data.u.data.first_elem = (orig->data.bit_offset / orig->data.elem_bits) + PageMapIteratorDataOffset(&oi);
                    test_data.u.data.first_elem = (test->data.bit_offset / test->data.elem_bits) + PageMapIteratorDataOffset(&ti);

                    for (elem_count = 0; ; ) {
                        bool done = false;
                        uint32_t j;
                        uint32_t k;

                        j = PageMapIteratorDataLength(&oi);
                        k = PageMapIteratorDataLength(&ti);
                        if (j != k) {
                            rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                            DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%s: length mismatch at row %li ( original=%u, test=%u )\n", self->dad.name, i, j, k));
                            break;
                        }
                        elem_count += j;

                        j = PageMapIteratorRepeatCount(&oi);
                        k = PageMapIteratorRepeatCount(&ti);
                        if (j != k) {
                            done = true;
                            if (j > k)
                                j = k;
                        }
                        if (PageMapIteratorAdvance(&ti, j) != PageMapIteratorAdvance(&oi, j)) {
                            rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                            DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%s: page map corrupt at row %li\n", self->dad.name, i));
                            break;
                        }
                        i += j;
			if ( done || (int64_t)i > orig->stop_id || test->pm->random_access || orig->pm->random_access || !PageMapIteratorAdvance( &ti, 0 ) )
                            break;
                    }
                    if (rc)
                        break;
                    if ( (int64_t)i > ( orig->stop_id + 1 ) ) {
                        rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                        (void)prev_i; /* shut up warning when not printing debug msg */
                        DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%s: page map has too many rows at row %li\n", self->dad.name, prev_i));
                        break;
                    }

                    orig_data.u.data.elem_count = test_data.u.data.elem_count = elem_count;

                    rc = self->u.cf(self->fself, &orig_data, &test_data);
                    if (rc) {
                        rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                        DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%s: data mismatch at row %li\n", self->dad.name, prev_i));
                        break;
                    }
                    if ((int64_t)i > orig->stop_id )
                        break;

                    /* check to see if the test iterator is at end
                     * and if so, fetch next blob */
                    if (!PageMapIteratorAdvance(&ti, 0)) {
                        VBlob *temp;
                        int64_t row = i;
                        rc = VProductionReadBlob(test_prod, &temp, &row, orig->stop_id - row, NULL);
                        if (rc == 0) {
                            vblob_release(test, NULL);
                            test = temp;
                            test_data.u.data.base = test->data.base;
                            PageMapNewIterator(test->pm, &ti, 0, -1);
                            if ( test->start_id < row ) {
                                if ( !PageMapIteratorAdvance( &ti, (uint32_t)( row - test->start_id ) ) ) {
                                    rc = RC(rcVDB, rcBlob, rcValidating, rcBlob, rcCorrupt);
                                    DBGMSG(DBG_VDB, DBG_VDB_COMPARE, ("%s: page map mismatch at row %li\n", self->dad.name, row));
                                }
                            }
                        }
                    }
                }
            }
            vblob_release(test, NULL);
        }
        vblob_release(orig, NULL);
    }
    return rc;
}

typedef struct fetch_param_blob_data fetch_param_blob_data;
struct fetch_param_blob_data
{
    int64_t id;
    uint32_t cnt;
    int64_t range_start_id;
    int64_t range_stop_id;
    Vector *inputs;
    VBlob *vblob;
    rc_t rc;
    bool no_cache;
};

static void fetch_param_blob_data_init(fetch_param_blob_data *pb,int64_t id,uint32_t cnt,Vector *inputs)
{
    pb->id = id;
    pb->cnt = cnt;
    pb->inputs = inputs;
    pb->rc = 0;
    pb->vblob = NULL;
    pb->range_start_id=INT64_MIN;
    pb->range_stop_id =INT64_MAX;
    pb->no_cache = false;
}
static
bool CC fetch_param_blob ( void *item, void *data )
{
    fetch_param_blob_data *pb = data;
    VBlob *blob;

    pb -> rc = VProductionReadBlob ( item, & blob, & pb -> id , pb -> cnt, NULL);
    if ( pb -> rc == 0 )
    {
        pb -> rc = VectorAppend ( pb -> inputs, NULL, blob );
        if ( pb -> rc == 0 ) {
            pb->no_cache |= blob->no_cache;
	    if(blob->start_id > pb->range_start_id) pb->range_start_id=blob->start_id;
	    if(blob->stop_id  < pb->range_stop_id)  pb->range_stop_id =blob->stop_id;
            return false;
        }
        vblob_release ( blob, NULL );
    }

    return true;
}

static
bool CC fetch_first_param_blob ( void *item, void *data )
{
    fetch_param_blob_data *pb = data;

    pb -> rc = VProductionReadBlob ( item, &pb->vblob, &pb -> id , pb -> cnt, NULL);
    if (GetRCState(pb->rc) == rcNotFound)
        return false;
    if ( pb -> vblob -> data.elem_count == 0 )
        return false;
    pb->range_start_id=pb->vblob->start_id;
    pb->range_stop_id =pb->vblob->stop_id;
    return true;
}

static
rc_t VFunctionProdSelect ( VFunctionProd *self, VBlob **vblob, int64_t id, uint32_t cnt ) {
    fetch_param_blob_data pb;
    fetch_param_blob_data_init(&pb,id,cnt,NULL);
    VectorDoUntil ( & self -> parms, false, fetch_first_param_blob, & pb );
    * vblob = pb.vblob;
    return pb.rc;
}

static
rc_t VFunctionProdPassThrough ( VFunctionProd *self, VBlob **vblob, int64_t id, uint32_t cnt ) {
    assert(VectorLength(&self->parms) == 1);
    return VProductionReadBlob(VectorGet(&self->parms, 0), vblob, &id, cnt, NULL);
}

static rc_t VFunctionProdReadNormal ( VFunctionProd *self, VBlob **vblob, int64_t id ,uint32_t cnt)
{
    rc_t rc;
    Vector inputs;
    fetch_param_blob_data pb;
    VBlob *vb=NULL;
    int64_t	id_run;
    int64_t     cnt_run;

    /* fill out information for function to use */
    const VCursor *curs = self -> curs;
    VXformInfo info;

    if(cnt == 0) cnt = 1;

#if VMGR_PASSED_TO_XFORM
    info . mgr = curs -> tbl -> mgr;
#endif
#if VSCHEMA_PASSED_TO_XFORM
    info . schema = curs -> schema;
#endif
#if VTABLE_PASSED_TO_XFORM
    info . tbl = VCursorGetTable ( curs );
#endif
#if VPRODUCTION_PASSED_TO_XFORM
    info . prod = & self -> dad;
#endif
    info . fdesc . fd = self -> dad . fd;
    info . fdesc . desc = self -> dad . desc;
    *vblob = NULL;

    if (self->dad.sub == prodFuncBuiltInCompare) {
        rc = VFunctionProdCallCompare(self, vblob, id, cnt);
#if _DEBUGGING
        if (rc != 0)
            rc = VFunctionProdCallCompare1(self, vblob, id, cnt);
#endif
        return rc;
    }

    /* all other functions take some form of blob input */
    VectorInit ( & inputs, 0, VectorLength ( & self -> parms ) );
    fetch_param_blob_data_init(&pb,id,cnt,&inputs);
    if ( VectorDoUntil ( & self -> parms, false, fetch_param_blob, & pb ) )
        rc = pb . rc;
    else for( id_run=id, cnt_run=cnt, rc=0; cnt_run > 0 && rc==0;)
    {
        switch ( self -> dad . sub )
        {
        case vftLegacyBlob:
            rc = VFunctionProdCallLegacyBlobFunc ( self, &vb, id_run, & info, & inputs );
            break;
        case vftNonDetRow:
            rc = VFunctionProdCallNDRowFunc ( self, &vb, id_run, & info, & inputs );
            break;
        case vftRow:
	    case vftRowFast:
        case vftIdDepRow:
            rc = VFunctionProdCallRowFunc ( self, &vb, id_run, cnt_run, & info, & inputs, pb.range_start_id,pb.range_stop_id );
            break;
        case vftArray:
            rc = VFunctionProdCallArrayFunc ( self, &vb, id_run, & info, & inputs );
            break;
        case vftFixedRow:
            rc = VFunctionProdCallPageFunc ( self, &vb, id_run, & info, & inputs );
            break;
        case vftBlob:
            rc = VFunctionProdCallBlobFunc ( self, &vb, id_run, & info, & inputs );
            break;
        case vftBlobN:
            rc = VFunctionProdCallBlobNFunc ( self, &vb, id_run, & info, & inputs );
            break;
        case prodFuncByteswap:
            rc = VFunctionProdCallByteswap ( self, &vb, id_run, & info, & inputs );
            break;
        default:
            rc = RC ( rcVDB, rcFunction, rcReading, rcProduction, rcCorrupt );
        }
        if (rc == 0) {
            if (vb == NULL) {
                rc = RC ( rcVDB, rcFunction, rcReading, rcProduction, rcNull );
            }
            else {
                if (vb -> start_id > id_run || vb -> stop_id < id_run) { /*** shoudn't happen ***/
                    rc = RC ( rcVDB, rcBlob, rcReading, rcRange, rcInsufficient );
                }
                if (*vblob == NULL) {
                    *vblob=vb;
                }
                else {
                    if (vb -> start_id <= id) {/** new blob is not appendable, but can replace the current one **/
                        vblob_release(*vblob, NULL);
                        *vblob = vb;
                    }
                    else {
                        /*** append here **/
                        rc = VBlobAppend(*vblob, vb);
                        vblob_release(vb, NULL);
                    }
                }
                /* propagate dirty flag */
                (*vblob)->no_cache |= pb.no_cache;
                if( (*vblob) -> stop_id >= id + cnt - 1)
                    break;

                id_run  = (*vblob) -> stop_id + 1;
                cnt_run = id + cnt - id_run;

            }
        }
        else if (id < id_run) {
            /* if there is reblobbing and our result blob already contains some
             * data, then return that data and not error out */
            rc = 0;
            break;
        }
    }
    /* drop input blobs */
    VectorWhack ( & inputs, vblob_release, NULL );
    return rc;
}

rc_t VFunctionProdRead ( VFunctionProd *self, VBlob **vblob, int64_t id , uint32_t cnt)
{
    if ( self -> dad . sub == vftSelect )
        return VFunctionProdSelect ( self, vblob, id , cnt);
    if ( self -> dad . sub == vftPassThrough )
        return VFunctionProdPassThrough ( self, vblob, id , cnt);
    return VFunctionProdReadNormal(self, vblob, id, cnt);
}

typedef struct fetch_param_IdRange_data fetch_param_IdRange_data;
struct fetch_param_IdRange_data
{
    int64_t first;
    int64_t last;
    rc_t rc;
    bool first_time;
};

static
bool CC fetch_param_IdRange ( void *item, void *data )
{
    fetch_param_IdRange_data *pb = data;
    int64_t first;
    int64_t last;
    rc_t rc;

    rc = VProductionColumnIdRange(item, &first, &last);
    if (GetRCState(rc) == rcEmpty && GetRCObject(rc) == rcRange)
        return false;

    pb->rc = rc;
    if (rc == 0 )
    {
        if (pb->first_time || first < pb->first)
            pb->first = first;
        if (pb->first_time || last > pb->last)
            pb->last = last;
        pb->first_time = false;
        return false;
    }

    return true;
}

LIB_EXPORT rc_t CC VFunctionProdColumnIdRange ( const VFunctionProd *self, int64_t *first, int64_t *last )
{
    fetch_param_IdRange_data pb;

    pb.first_time = true;
    pb . rc = 0;
    pb.first = 1;
    pb.last = 0;

    VectorDoUntil ( & self -> parms, false, fetch_param_IdRange, & pb );

    if (pb.rc == 0) {
#if 0
/* this causes problems in the loaders */
        if(pb.first_time){ /** no parameters - some function which generated data; f.e.  meta_value() ***/
              pb.last = INT64_MAX;
        }
#endif
        *first = pb.first;
        *last = pb.last;
    }
    return pb . rc;
}

typedef struct fetch_param_FixedRowLength_data fetch_param_FixedRowLength_data;
struct fetch_param_FixedRowLength_data
{
    uint32_t length;
    int64_t row_id;
    bool first_time;
};

static
bool CC fetch_param_FixedRowLength ( void *item, void *data )
{
    fetch_param_FixedRowLength_data *pb = data;
    uint32_t length;

    if (((const VProduction *)item)->control == false) {
        length = VProductionFixedRowLength(item, pb->row_id, false);

        if (pb->first_time)
            pb->length = length;

        pb->first_time = false;

        if (length == 0 || length != pb->length)
            return true;
    }
    return false;
}

static
uint32_t VFunctionProdFixedRowLength ( const VFunctionProd *self, int64_t row_id, bool ignore_self )
{
    fetch_param_FixedRowLength_data pb;

    if ( ! ignore_self )
    {
        switch ( self -> dad . sub )
        {
        case vftRow:
		case vftRowFast:
        case vftNonDetRow:
        case vftIdDepRow:
            return 0;
        }
    }

    pb.first_time = true;
    pb.length = 0;

    VectorDoUntil ( & self -> parms, false, fetch_param_FixedRowLength, & pb );

    return pb.length;
}


/*--------------------------------------------------------------------------
 * VScriptProd
 */

rc_t VScriptProdMake ( VScriptProd **prodp, Vector *owned, struct VCursor const *curs,
    int sub, const char *name, const VFormatdecl *fd,
    const VTypedesc *desc, uint8_t chain )
{
    VScriptProd *prod;
    rc_t rc = VProductionMake ( ( VProduction** ) prodp, owned, sizeof * prod,
        prodScript, sub, name, fd, desc, NULL, chain );
    if ( rc == 0 )
    {
        prod = * prodp;
        prod -> curs = curs;
        VectorInit ( & prod -> owned, 0, 4 );
    }
    return rc;
}

void VScriptProdDestroy ( VScriptProd *self )
{
    VectorWhack ( & self -> owned, VProductionWhack, NULL );
}


/* Read
 */
rc_t VScriptProdRead ( VScriptProd *self, VBlob **vblob, int64_t id,uint32_t cnt )
{
    return VProductionReadBlob ( self -> rtn, vblob, &id , cnt, NULL);
}

static rc_t VScriptProdColumnIdRange ( const VScriptProd *self, int64_t *first, int64_t *last )
{
    return VProductionColumnIdRange(self->rtn, first, last);
}

static uint32_t VScriptProdFixedRowLength ( const VScriptProd *self, int64_t row_id )
{
    return VProductionFixedRowLength(self->rtn, row_id, false);
}

/*--------------------------------------------------------------------------
 * VPivotProd
 *  potentially pivots to a new row-id space
 */

rc_t VPivotProdMake ( VPivotProd ** p_prodp,
                      Vector *      p_owned,
                      VProduction * p_member,
                      VProduction * p_row_id,
                      const char *  p_name,
                      int           p_chain )
{
    VPivotProd * prod;
    VFormatdecl fd = { { 0, 0 }, 0 };
    VTypedesc desc = { 64, 1, vtdInt };
    rc_t rc = VProductionMake ( ( VProduction** ) p_prodp, p_owned, sizeof * prod,
        prodPivot, 0, p_name, & fd, & desc, NULL, p_chain );
    if ( rc == 0 )
    {
        prod = * p_prodp;
        prod -> member = p_member;
        prod -> row_id = p_row_id;
    }
    return rc;
}

void VPivotProdDestroy ( VPivotProd * p_self )
{
}

/* Read
 */
rc_t VPivotProdRead ( VPivotProd * p_self, struct VBlob ** p_vblob, int64_t * p_id, uint32_t p_cnt )
{
    struct VBlob * rowIdBlob;
    rc_t rc;
    assert ( p_id != NULL );
    rc = VProductionReadBlob ( p_self -> row_id, & rowIdBlob, p_id , p_cnt, NULL);
    if ( rc == 0 )
    {
        uint32_t elemNum;
        uint32_t repeat_count;
        uint32_t rowLen = PageMapGetIdxRowInfo ( rowIdBlob -> pm, ( uint32_t ) ( * p_id - rowIdBlob -> start_id ), & elemNum, & repeat_count );
        /* assume elem size is 64 */
        int64_t newRowId = * ( ( int64_t* ) rowIdBlob -> data . base + elemNum );

		assert ( rowLen == 1);
        assert ( repeat_count == 1);

		vblob_release ( rowIdBlob, NULL );

		rc = VProductionReadBlob ( p_self -> member, p_vblob, & newRowId, p_cnt, NULL);
        if ( rc == 0 )
        {
            /* the cache mechanism will get confused by our overwriting p_id, so turn it off */
            ( * p_vblob ) -> no_cache = true;
            * p_id = newRowId;
        }
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * VProduction
 */

/* Init
 *  parent initialization function
 *  called from the "Make" functions below
 */
#if 0
static
void VProductionInit ( VProduction *self, int var, int sub, const char *name,
                       const VFormatdecl *fd, const VTypedesc *desc,
                       const VCtxId *cid, uint8_t chain )
{
    memset ( self, 0, sizeof * self );

    if ( fd != NULL )
        self -> fd = * fd;
    if ( desc != NULL )
        self -> desc = * desc;
    if ( cid != NULL )
        self -> cid = * cid;

    self -> var = ( uint8_t ) var;
    self -> sub = ( uint8_t ) sub;
    self -> chain = chain;
}
#endif

void CC VProductionWhack ( void *item, void *owned )
{
    VProduction * self = item;

    if ( self != NULL )
    {
        if ( owned != NULL)
        {
            void *ignore;
            VectorSwap ( owned, self -> oid, NULL, & ignore );
            assert ( ( void* ) self == ignore );
        }


#if PROD_CACHE
        VProductionFlushCacheDeep ( self, "whack" );
#endif
        switch ( self -> var )
        {
        case prodSimple:
#if TRACKING_BLOBS
            fprintf( stderr, "VSimpleProd %p being whacked *** %s\n", self, self->name );
#endif
            VSimpleProdDestroy ( ( VSimpleProd* ) self );
            break;

        case prodFunc:
#if TRACKING_BLOBS
            fprintf( stderr, "VFunctionProd %p being whacked *** %s\n", self, self->name );
#endif
            VFunctionProdDestroy ( ( VFunctionProd* ) self );
            break;

        case prodScript:
#if TRACKING_BLOBS
            fprintf( stderr, "VScriptProd %p being whacked *** %s\n", self, self->name );
#endif
            VScriptProdDestroy ( ( VScriptProd* ) self );
            break;

        case prodPhysical:
#if TRACKING_BLOBS
            fprintf( stderr, "VPhysicalProd %p being whacked *** %s\n", self, self->name );
#endif
            VPhysicalProdDestroy ( ( VPhysicalProd* ) self );
            break;

        case prodColumn:
#if TRACKING_BLOBS
            fprintf( stderr, "VColumnProd %p being whacked *** %s\n", self, self->name );
#endif
            VColumnProdDestroy ( ( VColumnProd* ) self );
            break;

        case prodPivot:
#if TRACKING_BLOBS
            fprintf( stderr, "VPivotProd %p being whacked *** %s\n", self, self->name );
#endif
            VPivotProdDestroy ( ( VPivotProd* ) self );
            break;
        }

        free ( self );
    }
}

/* Cmp
 * Sort
 *  compare item is a VCtxId
 *  sort item is a VProduction
 *  n is always a VProduction
 */
LIB_EXPORT int64_t CC VProductionCmp ( const void *item, const void *n )
{
    const VCtxId *a = item;
    const VProduction *b = n;
    return VCtxIdCmp ( a, & b -> cid );
}

LIB_EXPORT int64_t CC VProductionSort ( const void *item, const void *n )
{
    const VProduction *a = item;
    const VProduction *b = n;
    return VCtxIdCmp ( & a -> cid, & b -> cid );
}


/* IdRange
 *  obtains intersection of all physical sources
 *
 *  "first" [ IN/OUT ] and "last" [ IN/OUT ] - range to intersect
 */
rc_t VProductionColumnIdRange ( const VProduction *self,
    int64_t *first, int64_t *last )
{
    if ( self <= FAILED_PRODUCTION )
        return 0;

    switch ( self -> var )
    {
    case prodSimple:
        return VProductionColumnIdRange ( ( ( const VSimpleProd* ) self ) -> in, first, last );
    case prodFunc:
        return VFunctionProdColumnIdRange((const VFunctionProd *)self, first, last);
    case prodScript:
        return VScriptProdColumnIdRange((const VScriptProd *)self, first, last);
    case prodPhysical:
        return VPhysicalProdColumnIdRange((const VPhysicalProd *)self, first, last);
    case prodPivot:
        return VProductionColumnIdRange( ( (const VPivotProd *)self ) -> member, first, last);
    case prodColumn:
        return RC ( rcVDB, rcColumn, rcAccessing, rcRange, rcEmpty );
    }

    return RC ( rcVDB, rcColumn, rcAccessing, rcType, rcUnknown );
}

rc_t VProductionPageIdRange ( VProduction *self,
    int64_t id, int64_t *first, int64_t *last )
{
    VBlob *blob;
    rc_t rc = VProductionReadBlob ( self, & blob, & id , 1, NULL);
    if ( rc == 0 )
    {
        * first = blob -> start_id;
        * last = blob -> stop_id;

        vblob_release ( blob, NULL );
    }
    return rc;
}

/* RowLength
 *  get row length for a particular row
 */
uint32_t VProductionRowLength ( const VProduction *self, int64_t row_id )
{
    uint32_t row_len;

    VBlob *blob;
    rc_t rc = VProductionReadBlob ( self, & blob, & row_id, 1, NULL );
    if ( rc != 0 )
        return 0;

    row_len = PageMapGetIdxRowInfo ( blob -> pm, (uint32_t)( row_id - blob -> start_id) , NULL, NULL );

    vblob_release ( blob, NULL );

    return row_len;
}

/* FixedRowLength
 *  get fixed row length for entire column
 *  returns 0 if not fixed
 */
uint32_t VProductionFixedRowLength ( const VProduction *self, int64_t row_id,bool ignore_self )
{
    switch ( self -> var )
    {
    case prodSimple:
        return VProductionFixedRowLength ( ( ( const VSimpleProd* ) self ) -> in, row_id, ignore_self );
    case prodFunc:
        return VFunctionProdFixedRowLength((const VFunctionProd *)self, row_id, ignore_self);
    case prodScript:
        return VScriptProdFixedRowLength((const VScriptProd *)self, row_id);
    case prodPhysical:
        return VPhysicalProdFixedRowLength((const VPhysicalProd *)self, row_id);
    case prodPivot:
        assert(false); /*TODO*/
    }

    return RC ( rcVDB, rcColumn, rcAccessing, rcType, rcUnknown );
}

/* ReadBlob
 */
rc_t VProductionReadBlob ( const VProduction *cself, VBlob **vblob, int64_t * p_id, uint32_t cnt, VBlobMRUCacheCursorContext *cctx )
{
#if BYTECODE

    rc_t rc;
    VProduction *self = ( VProduction* ) cself;
    if ( self == NULL )
        return RC ( rcVDB, rcProduction, rcReading, rcSelf, rcNull );

    struct ByteCodeContext ctx;
    ctx . id = * p_id;
    ctx . cnt = cnt;
    ctx . cctx = cctx;
    ctx . result = NULL;
    rc = ExecuteByteCode ( bcProductionReadBlob, self, & ctx );
    * vblob = ctx . result;
    if ( rc == 0 )
    {
        * p_id = ctx . id;
    }
    return rc;

#else

    rc_t rc;
    VProduction *self = ( VProduction* ) cself;

#if PROD_CACHE
    int i;
    VBlob *blob;
#endif

    * vblob = NULL;

    /* should not be possible, but safety is cheap */
    if ( self == NULL )
        return RC ( rcVDB, rcProduction, rcReading, rcSelf, rcNull );

    /*** Cursor-level column blobs may be 1-to-1 with production blobs ***/
    if(cctx != NULL && self->cctx.cache == NULL ){ /*** we are connected to read cursor **/
	self->cctx = *cctx; /*** remember it ***/
	/** No need to do anything else here - we are on "direct line" to the column ***/
    } else if(self->cctx.cache != NULL){
	/** somewhere else this production is connected to a cursor **/
	/** lets try to get answers from the cursor **/
	blob=(VBlob*) VBlobMRUCacheFind(self->cctx.cache,self->cctx.col_idx,*p_id);
	if(blob){
		rc = VBlobAddRef ( blob );
                if ( rc != 0 ) return rc;
		*vblob=blob;
		return 0;
	}
    }

#if PROD_CACHE
    /* check cache */
    for ( i = 0; i < self -> cache_cnt; ++ i )
    {
        blob = self -> cache [ i ];
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
                * p_id >= blob -> start_id &&
                * p_id <= blob -> stop_id )
            {
                rc = VBlobAddRef ( blob );
                if ( rc != 0 )
                    return rc;
#if TRACKING_BLOBS
                fprintf( stderr, "%p->%p(%d) new reference to cached blob *** %s\n"
                         , self
                         , blob
                         , atomic32_read ( & blob -> refcount )
                         , self->name
                    );
#endif
                /* return new reference */
                * vblob = blob;
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

    /* dispatch */
    switch ( self -> var )
    {
    case prodSimple:
        rc = VSimpleProdRead ( ( VSimpleProd* ) self, vblob, p_id, cnt,cctx );
        break;
    case prodFunc:
        rc = VFunctionProdRead ( ( VFunctionProd* ) self, vblob, * p_id , cnt);
#if _DEBUGGING && PROD_NAME
        if ( rc != 0 )
            DBGMSG ( DBG_VDB, DBG_VDB_FUNCTION, ( "%s: %R\n", self -> name, rc ) );
#endif
        break;
    case prodScript:
        rc = VScriptProdRead ( ( VScriptProd* ) self, vblob, * p_id , cnt);
        break;
    case prodPhysical:
        rc = VPhysicalProdRead ( ( VPhysicalProd* ) self, vblob, * p_id, cnt );
        break;
    case prodColumn:
        rc = VColumnProdRead ( ( VColumnProd* ) self, vblob, * p_id );
        break;
    case prodPivot:
        rc = VPivotProdRead ( ( VPivotProd* ) self, vblob, p_id, cnt );
        break;
    default:
        return RC ( rcVDB, rcProduction, rcReading, rcType, rcUnknown );
    }

#if ! PROD_CACHE
    return rc;
#else
    blob = * vblob;

    if ( rc != 0 || * vblob == NULL )
        return rc;

#if ! USE_EUGENE
         /* NB - there is another caching mechanism on VColumn
            if a blob does not want to be cached, it is rejected here */
    if ( ! blob -> no_cache )
        return 0;
#endif
    if(cctx == NULL && self->cctx.cache != NULL && blob->stop_id > blob->start_id + 4){/** we will benefit from caching here **/
		VBlobMRUCacheSave(self->cctx.cache,self->cctx.col_idx,blob);
		return 0;
    }

    if(blob->pm == NULL) return 0;


    /* cache output */
    rc = VBlobAddRef ( blob );
    if ( rc == 0 )
    {
        VBlobCheckIntegrity ( blob );
#if PROD_CACHE
	if(self -> cache_cnt < PROD_CACHE){
#if PROD_CACHE > 1
		if(self -> cache_cnt > 0 ){
#if PROD_CACHE > 2
			memmove(self -> cache + 1, self -> cache , self -> cache_cnt * sizeof(*self -> cache));
#else
			self -> cache[1]=self -> cache[0];
#endif
		}
#endif
		self -> cache_cnt ++;
	} else {
		/* release whatever was there previously */
        	/* drop LRU */
		vblob_release ( self -> cache [ self -> cache_cnt - 1 ], NULL );
#if PROD_CACHE > 1
#if PROD_CACHE > 2
		memmove(self -> cache + 1, self -> cache , (self -> cache_cnt -1) * sizeof(*self -> cache));
#else
		self -> cache[1]=self -> cache[0];
#endif
#endif
        }
        /* insert a head of list */
        self -> cache [ 0 ] = blob;
#endif

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

#endif /* PROD_CACHE */

#endif
}

/* IsStatic
 *  trace all the way to a physical production
 */
rc_t VProductionIsStatic ( const VProduction *self, bool *is_static )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcSelf, rcNull );
    else
    {
        for ( rc = 0; self != NULL; )
        {
            switch ( self -> var )
            {
            case prodSimple:
                self = ( ( const VSimpleProd*) self ) -> in;
                break;
            case prodFunc:
            case prodScript:
            {
                const VFunctionProd *fp = ( const VFunctionProd* ) self;
                uint32_t start = VectorStart ( & fp -> parms );
                uint32_t end = VectorLength ( & fp -> parms );
                for ( end += start; start < end; ++ start )
                {
                    self = ( const VProduction* ) VectorGet ( & fp -> parms, start );
                    if ( self != NULL )
                    {
                        rc = VProductionIsStatic ( self, is_static );
                        if ( rc != 0 || * is_static )
                            break;
                    }
                }
                return rc;
            }
            case prodPhysical:
                return VPhysicalIsStatic ( ( ( const VPhysicalProd* ) self ) -> phys, is_static );
            case prodColumn:
                self = NULL;
                break;
            case prodPivot:
                assert(false); /* TODO */
                break;
            default:
                return RC ( rcVDB, rcProduction, rcReading, rcType, rcUnknown );
            }
        }
    }

    return rc;
}

/* GetKColumn
 *  drills down to physical production to get a KColumn,
 *  and if that fails, indicate whether the column is static
 */
rc_t VProductionGetKColumn ( const VProduction * self, struct KColumn ** kcol, bool * is_static )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcSelf, rcNull );
    else
    {
        for ( rc = 0; self != NULL; )
        {
            switch ( self -> var )
            {
            case prodSimple:
                self = ( ( const VSimpleProd*) self ) -> in;
                break;
            case prodFunc:
            case prodScript:
            {
                const VFunctionProd *fp = ( const VFunctionProd* ) self;
                uint32_t start = VectorStart ( & fp -> parms );
                uint32_t end = VectorLength ( & fp -> parms );
                for ( end += start; start < end; ++ start )
                {
                    self = ( const VProduction* ) VectorGet ( & fp -> parms, start );
                    if ( self != NULL )
                    {
                        rc = VProductionGetKColumn ( self, kcol, is_static );
                        if ( rc != 0 || * kcol != NULL || * is_static )
                            break;
                    }
                }
                return rc;
            }
            case prodPhysical:
                return VPhysicalGetKColumn ( ( ( const VPhysicalProd* ) self ) -> phys, kcol, is_static );
            case prodColumn:
                self = NULL;
                break;
            case prodPivot:
                assert(false); /* TODO */
                break;
            default:
                return RC ( rcVDB, rcProduction, rcReading, rcType, rcUnknown );
            }
        }
    }

    return rc;
}
