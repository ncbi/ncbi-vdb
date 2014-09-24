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
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <klib/log.h>
#include <arch-impl.h>
#include <kns/url-fetcher.h>
#include <kns/entrez-fetcher.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct entrez_context {
    KUrlFetcher *url_fetcher;
    KEntrezFetcher *entrez_fetcher;
} entrez_context_t;


static rc_t init_entrez_context( entrez_context_t * ctx )
{
    /* first make the url-fetcher, because 
       it is neccessary to create the sra-fetcher... */
    rc_t rc = KUrlFetcherCurlMake( &(ctx->url_fetcher), false );
    if ( rc == 0 )
    {
        /* then make the sra-fetcher, with the url-fetcher as parameter... */
        rc = KEntrezFetcherMake( &(ctx->entrez_fetcher), ctx->url_fetcher );
    }
    return rc;
}


static void CC entrez_context_whack( void *ptr )
{
    entrez_context_t * ctx = ( entrez_context_t * )ptr;
    if ( ctx )
    {
        KEntrezFetcherRelease( ctx->entrez_fetcher );
        KUrlFetcherRelease( ctx->url_fetcher );
        free( ctx );
    }
}

static rc_t vrowdata_get64( const VRowData * src, uint64_t * dst )
{
    if ( src->u.data.elem_bits == 64 )
    {
        uint64_t * src_ptr = (uint64_t *)src->u.data.base;
        src_ptr += src->u.data.first_elem;
        *dst = *src_ptr;
        return 0;
    }
    return RC ( rcXF, rcFunction, rcReading, rcType, rcIncorrect );
}

static uint32_t vrowdata_get32( const VRowData * src, uint32_t * dst )
{
    if ( src->u.data.elem_bits == 32 )
    {
        uint32_t * src_ptr = (uint32_t *)src->u.data.base;
        src_ptr += src->u.data.first_elem;
        *dst = *src_ptr;
        return 0;
    }
    return RC ( rcXF, rcFunction, rcReading, rcType, rcIncorrect );
}

const char * entrez_server = "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi";

static rc_t CC perform_entrez_fetch ( void *self,
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    entrez_context_t * ctx = ( entrez_context_t * )self;
    if ( ctx == NULL )
        rc = RC ( rcXF, rcFunction, rcReading, rcSelf, rcNull );
    else
    {
        char * entrez_seq_id = "NC_000001";
        uint32_t entrez_seq_len = 1024;
        uint64_t entrez_row_id = 1;
        uint32_t entrez_row_count = 1;
        size_t buffsize;

        if ( argc > 0 )
            rc = vrowdata_get64( &( argv[0] ), &entrez_row_id );
        if ( rc == 0 )
        {
            if ( argc > 1 )
                rc = vrowdata_get32( &( argv[1] ), &entrez_seq_len );
            if ( rc == 0 )
            {
                /* prepare the Fetcher with all the information it needs,
                   returns in buffsize the size of the buffer we have to provide */
                rc = KEntrezFetcherSetup ( ctx->entrez_fetcher,
                            entrez_server, entrez_seq_id, entrez_seq_len,
                            entrez_row_id, entrez_row_count, &buffsize );
                if ( rc == 0 )
                {
                    rc = KDataBufferResize( rslt->data, buffsize );
                    if ( rc == 0 )
                    {
                        size_t num_read;
                        /* perform the fetch-operation directly into the KDatabuffer */
                        rc = KEntrezFetcherRead ( ctx->entrez_fetcher,
                                          rslt->data->base,
                                          buffsize, &num_read );
                        if ( rc == 0 )
                        {
                            rslt->elem_count = num_read;
                            rslt->elem_bits = 8;
                        }
                    }
                }
            }
        }
    }
    return rc;
}

VTRANSFACT_IMPL ( vdb_entrez, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    entrez_context_t *ctx = malloc ( sizeof( *ctx ) );
    if ( ctx == NULL )
        rc = RC ( rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = init_entrez_context( ctx );
        if ( rc == 0 )
        {
            rslt -> self = ctx;
            rslt -> whack = entrez_context_whack;
            rslt -> u . rf = perform_entrez_fetch;
            rslt -> variant = vftRow;
        }
    }
    return rc;
}
