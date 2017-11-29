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
 */

#include <kfs/extern.h>

struct RRCachedFile;
#define KFILE_IMPL struct RRCachedFile
#include <kfs/impl.h>

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/printf.h>

#include <kfs/defs.h>

#include <kfs/recorder.h>
#include "poolpages.h"
#include <kproc/queue.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>

#define RR_CACHE_DEFAULT_BLOCKSIZE ( 128 * 1024 )
#define RR_CACHE_MIN_BLOCKSIZE ( 16 * 1024 )
#define RECORDING 1

static rc_t hand_out_to_wrap_file_as_rr_cached( struct KFile const ** rr_cached, struct KFile const * to_wrap )
{
    rc_t rc = KFileAddRef( to_wrap );
    if ( rc != 0 )
    {
        LOGERR( klogErr, rc, "KFileAddRef( file to be wrapped ) failed" );
    }
    else
    {
        *rr_cached = to_wrap;
    }
    return rc;
}


typedef struct RRCachedFile
{
    KFile dad;
    const KFile * wrapped;                  /* the file we are wrapping */
    uint64_t wrapped_size;                  /* the size of the wrapped file */
    uint64_t block_count;                   /* how many blocks do we need to cache the remote file ( last block may be shorter ) */
    struct ThePool * pool;                  /* have a cache to answer from RAM! */
    
#if RECORDING
    struct Recorder * recorder;             /* optional recorder ( see above ) */
#endif

    uint32_t block_size;                    /* how big is a block ( aka 1 bit in the bitmap )*/
} RRCachedFile;


static rc_t CC RRCachedDestroy ( RRCachedFile * self )
{
    if ( self -> pool != NULL )
        pool_release ( self -> pool );
    
    KFileRelease ( self -> wrapped );

#if RECORDING
    if ( self -> recorder != NULL )
    {
        WriteToRecorder ( self -> recorder, "recorder released\n" );
        ReleaseRecorder ( self -> recorder );
    }
#endif

    free ( self );
    return 0;
}

static struct KSysFile* RRCachedGetSysFile ( const RRCachedFile *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}


static rc_t RRCachedRandomAccess ( const RRCachedFile *self )
{
    return 0;
}


static rc_t RRCachedSize ( const RRCachedFile *self, uint64_t *size )
{
    *size = self->wrapped_size;
    return 0;
}


static rc_t RRCachedSetSize ( RRCachedFile *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFile, rcReadonly );
}

static rc_t RRCachedRead ( const RRCachedFile *cself, uint64_t pos,
                           void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;
    struct PoolPage * pp = NULL;

#if RECORDING
    bool in_ram = false;
    uint64_t rec_block_pos = 0;
    size_t rec_block_size = 0;
    uint32_t rec_block_idx = 0;
    uint32_t rec_block_usage = 0;
#endif

    if ( pool_page_find ( cself -> pool, &pp, pos ) == 0 && pp != NULL )
    {
        /* we found a pool-page that has at least the first portion of the request in it */
        rc = pool_page_get ( pp, pos, buffer, bsize, num_read );
        /* we are done - let the caller come back for more if that was not all he asked for */
        if ( rc == 0 )
        {
#if RECORDING
            in_ram = true;
            rec_block_idx = pool_page_idx ( pp ) + 1;
            rec_block_usage = pool_page_usage( pp );
#endif
        }
        else
        {
            /* we have not other choice than reading directly from the wrapped file */
            rc = KFileReadAll( cself -> wrapped, pos, buffer, bsize, num_read );
        }
    }
    else
    {
        rc = pool_page_find_new ( cself -> pool, &pp );
        if ( rc != 0 || pp == NULL )
        {
            /* we have not other choice than reading directly from the wrapped file */
            rc = KFileReadAll( cself -> wrapped, pos, buffer, bsize, num_read );
        }
        else
        {
            /* we are able to find a block-page to use for the request... */
            uint64_t block_idx = ( pos / cself -> block_size );
            uint64_t block_pos = ( block_idx * cself -> block_size );
            rc = pool_page_prepare( pp, 1, block_pos );
            if ( rc == 0 )
            {
                size_t read;
                rc = pool_page_read_from_file( pp, cself -> wrapped, &read );
                if ( rc == 0 )
                {
                    rc = pool_page_get ( pp, pos, buffer, bsize, num_read );
                    if ( rc == 0 )
                    {
#if RECORDING
                        rec_block_pos = block_pos;
                        rec_block_size = cself -> block_size;
                        rec_block_idx = pool_page_idx ( pp ) + 1;
                        rec_block_usage = pool_page_usage( pp );
#endif
                    }
                    else
                    {
                        /* we have not other choice than reading directly from the wrapped file */
                        rc = KFileReadAll( cself -> wrapped, pos, buffer, bsize, num_read );
                    }
                }
                else
                {
                    /* we have not other choice than reading directly from the wrapped file */
                    rc = KFileReadAll( cself -> wrapped, pos, buffer, bsize, num_read );
                }
            }
            else
            {
                /* we have not other choice than reading directly from the wrapped file */
                rc = KFileReadAll( cself -> wrapped, pos, buffer, bsize, num_read );
            }
        }
    }

#if RECORDING
    if ( cself -> recorder != NULL )
        WriteToRecorder ( cself -> recorder,
                          "%lu\t%lu\t%lu\t%s\t%lu\t%lu\t%d.%d\n",
                          pos, bsize, *num_read, in_ram ? "Y" : "N",
                          rec_block_pos, rec_block_size, rec_block_idx, rec_block_usage );
#endif                
    
    if ( pp != NULL )
        pool_page_release ( pp );
    return rc;
}

static rc_t RRCachedWrite ( RRCachedFile *self, uint64_t pos,
                           const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static KFile_vt_v1 vtRRCached =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    RRCachedDestroy,
    RRCachedGetSysFile,
    RRCachedRandomAccess,
    RRCachedSize,
    RRCachedSetSize,
    RRCachedRead,
    RRCachedWrite
    /* end minor version 0 methods */
};

LIB_EXPORT rc_t CC MakeRRCached ( struct KFile const **rr_cached,
                                  struct KFile const *to_wrap,
                                  uint32_t block_size,
                                  uint32_t page_count,
                                  struct Recorder * recorder )
{
    rc_t rc = 0;
    size_t to_wrap_size = 0;
    uint32_t b_size = RR_CACHE_DEFAULT_BLOCKSIZE;
        
    if ( rr_cached == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *rr_cached = NULL;
        if ( to_wrap == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    }

    if ( rc == 0 )
    {
        rc = KFileSize ( to_wrap, &to_wrap_size );
        if ( rc != 0 )
        {
            LOGERR( klogErr, rc, "cannot detect size of file to be wrapped" );
        }
        else if ( to_wrap_size == 0 )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );
            LOGERR( klogErr, rc, "file to be wrapped is empty" );
        }
    }
    
    if ( rc == 0 )
    {
        if ( block_size > 0 ) b_size = block_size;
        /* let the blocksize be a multiple of 16 */
        b_size &= 0xFFFFF0;
        if ( b_size < RR_CACHE_MIN_BLOCKSIZE )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcInsufficient );
            LOGERR( klogErr, rc, "blocksize too small" );
        }
    }

    if ( rc == 0 )
    {
        rc = KFileAddRef ( to_wrap );
        if ( rc == 0 )
        {
            struct ThePool * pool;
            rc = make_pool ( &pool, b_size, page_count );
            if ( rc == 0 )
            {
                RRCachedFile * rrf = malloc ( sizeof * rrf );
                if ( rrf == NULL )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    /* now we can enter everything into the rr - struct */
                    rrf -> wrapped = to_wrap;
                    rrf -> wrapped_size = to_wrap_size;
                    rrf -> block_count = to_wrap_size / b_size;
                    rrf -> pool = pool;
                    rrf -> block_size = b_size;
#if RECORDING
                    rrf -> recorder = recorder;
                    if ( recorder != NULL )
                        WriteToRecorder ( recorder, "\nrecorder set\n" );
#endif
                    rc = KFileInit ( &rrf -> dad,
                                     ( const union KFile_vt * ) &vtRRCached,
                                     "RRCachedFile",
                                     "rrcached",
                                     true,
                                     false );
                    if ( rc != 0 )
                        free( ( void * ) rrf );
                    else
                    {
                        /* the wrapper is ready to use now! */
                        *rr_cached = ( const KFile * ) &rrf -> dad;
                    }

                    if ( rc != 0 )
                        pool_release ( pool );
                }
            }
            if ( rc != 0 )
                KFileRelease ( to_wrap );
        }
    }
    
    if ( rc != 0 )
        rc = hand_out_to_wrap_file_as_rr_cached ( rr_cached, to_wrap );
    return rc;
}
