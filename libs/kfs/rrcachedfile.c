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
#include <kfs/rrcachedfile.h>

#include "poolpages.h"
#include "lru_cache.h"

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>

#define RR_CACHE_DEFAULT_BLOCKSIZE ( 128 * 1024 * 1024 )
#define RR_CACHE_MIN_BLOCKSIZE ( 16 * 1024 )

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
    struct lru_cache * cache;               /* the lru-cache */
    uint64_t cached_size;
    bool size_cached;
} RRCachedFile;


static rc_t CC RRCachedDestroy ( RRCachedFile * self )
{
    release_lru_cache ( self -> cache );
    KFileRelease ( self -> wrapped );
    free ( ( void * )self );
    return 0;
}

static struct KSysFile* RRCachedGetSysFile ( const RRCachedFile * self, uint64_t * offset )
{
    * offset = 0;
    return NULL;
}


static rc_t RRCachedRandomAccess ( const RRCachedFile * self )
{
    return 0;
}


static rc_t RRCachedSize ( const RRCachedFile * self, uint64_t * size )
{
    rc_t rc = 0;
    if ( self -> size_cached )
        *size = self -> cached_size;
    else
    {
        rc = KFileSize ( self -> wrapped, size );
        if ( rc == 0 )
        {
            RRCachedFile * cf = ( RRCachedFile * )self;
            cf -> cached_size = *size;
            cf -> size_cached = true;
        }
    }
    return rc;
}


static rc_t RRCachedSetSize ( RRCachedFile * self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcUpdating, rcFile, rcReadonly );
}

static rc_t RRCachedRead ( const RRCachedFile * cself, uint64_t pos,
                           void *buffer, size_t bsize, size_t *num_read )
{
    return read_lru_cache ( cself -> cache, pos, buffer, bsize, num_read, NULL );
}

static rc_t RRCachedWrite ( RRCachedFile *self, uint64_t pos,
                           const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static uint32_t RRCachedGetType ( const RRCachedFile *self )
{
    return KFileType ( self -> wrapped );
}

static rc_t RRCachedTimedRead ( const RRCachedFile * cself, uint64_t pos,
                                void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    return read_lru_cache ( cself -> cache, pos, buffer, bsize, num_read, tm );
}

static rc_t RRCachedTimedWrite ( RRCachedFile *self, uint64_t pos,
                                 const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    return RC ( rcFS, rcFile, rcUpdating, rcInterface, rcUnsupported );
}

static KFile_vt_v1 vtRRCached_v1_0 =
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
    RRCachedWrite,
    /* end minor version 0 methods */
};

static KFile_vt_v1 vtRRCached_v1_2 =
{
    /* version 1.2 */
    1, 2,

    /* start minor version 0 methods */
    RRCachedDestroy,
    RRCachedGetSysFile,
    RRCachedRandomAccess,
    RRCachedSize,
    RRCachedSetSize,
    RRCachedRead,
    RRCachedWrite,
    /* end minor version 0 methods */

    /* start minor version 1 method */
    RRCachedGetType,
    /* end minor version 1 method */

    /* start minor version 2 methods */
    RRCachedTimedRead,
    RRCachedTimedWrite
    /* end minor version 2 methods */
};

static rc_t make_rr_cached( struct KFile const **cached,
                            struct KFile const *to_wrap,
                            uint32_t page_size,
                            uint32_t page_count )
{
    rc_t rc = KFileAddRef ( to_wrap );
    if ( rc == 0 )
    {
        struct lru_cache * cache;
        rc = make_lru_cache ( &cache, to_wrap, page_size, page_count );
        if ( rc == 0 )
        {
            RRCachedFile * rrf = malloc ( sizeof * rrf );
            if ( rrf == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                const KFile_vt_v1 * vt;
                
                rrf -> wrapped = to_wrap;
                rrf -> cache = cache;
                rrf -> cached_size = 0;
                rrf -> size_cached = false;

                /* don't appear to be more modern than wrapped file */
                vt = ( to_wrap -> vt -> v1 . min >= 2 ) ? & vtRRCached_v1_2 : & vtRRCached_v1_0;

                rc = KFileInit ( &rrf -> dad,
                                 ( const union KFile_vt * ) vt,
                                 "RRCachedFile",
                                 "rrcached",
                                 true,
                                 false );
                if ( rc == 0 )
                {
                    /* the wrapper is ready to use now! */
                    *cached = ( const KFile * ) &rrf -> dad;
                }
            }
            if ( rc != 0 )
                RRCachedDestroy ( rrf );
        }
    }
    if ( rc != 0 )
        rc = hand_out_to_wrap_file_as_rr_cached ( cached, to_wrap );
    return rc;
}

LIB_EXPORT rc_t CC MakeRRCached ( struct KFile const **cached,
                                  struct KFile const *to_wrap,
                                  uint32_t page_size,
                                  uint32_t page_count )
{
    rc_t rc = 0;
    
    if ( cached == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *cached = NULL;
        if ( to_wrap == NULL || page_size == 0 || page_count == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    }

    if ( rc == 0 )
        rc = make_rr_cached( cached, to_wrap, page_size, page_count );

    return rc;
}

LIB_EXPORT rc_t CC SetRRCachedEventHandler( struct KFile const * self,
                        void * data, on_cache_event handler )
{
    rc_t rc = 0;
    if ( self == NULL || handler == NULL || data == NULL )
        rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcNull );
    else
    {
        if ( &self -> vt -> v1 != &vtRRCached_v1_0 && &self -> vt -> v1 != &vtRRCached_v1_2 )
            rc = RC ( rcFS, rcFile, rcValidating, rcParam, rcInvalid );
        else
        {
            struct RRCachedFile * rrf = ( struct RRCachedFile * )self;
            rc = set_lru_cache_event_handler( rrf -> cache, data, handler );
        }
    }
    return rc;
}
