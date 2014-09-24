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

#include <kdb/extern.h>
#include "colidx2-priv.h"
#include "idxblk-priv.h"
#include <kfs/file.h>
#include <kfs/buffile.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>


#define IDX2_READ_FILE_BUFFER 64 * 1024


/*--------------------------------------------------------------------------
 * KColumnIdx2
 *  level 2 index
 */

/* Init
 */
static
rc_t KColumnIdx2Init ( KColumnIdx2 *self, uint64_t idx2_eof )
{
    rc_t rc;

#if 0
    memset(&self->cstorage,0,sizeof(self->cstorage));
    self->cstorage.elem_bits = sizeof(KColumnIdx2BlockCache)*8;
    self->last = 0;
#endif
    rc = KFileSize ( self -> f, & self -> eof );
    if ( rc == 0 )
    {
        if ( self -> eof < idx2_eof )
            rc = RC ( rcDB, rcColumn, rcConstructing, rcIndex, rcCorrupt );
        else
        {
            self -> eof = idx2_eof;
            return 0;
        }
    }
    
    KFileRelease ( self -> f );

    self -> f = NULL;
    return rc;
}

/* Open
 */
rc_t KColumnIdx2OpenRead ( KColumnIdx2 *self,
    const KDirectory *dir, uint64_t eof )
{
    rc_t rc;

    rc = KDataBufferMake ( & self -> cstorage, sizeof ( KColumnIdx2BlockCache ) * 8, 0 );
    if ( rc != 0 )
    {
        memset ( self, 0, sizeof * self );
        return rc;
    }

    self -> last = 0;

    if ( eof == 0 )
    {
#if 0
        KDataBufferResize(&self->cstorage,0);
        self -> last = 0;
#endif
        self -> eof = 0;
        self -> f = NULL;
        return 0;
    }

    rc = KDirectoryOpenFileRead ( dir,
                                  & self -> f, "idx2" );
#if IDX2_READ_FILE_BUFFER
    if ( rc == 0 )
    {
        const KFile * orig = self -> f;
        rc = KBufFileMakeRead ( & self -> f, self -> f, IDX2_READ_FILE_BUFFER );
	if ( rc == 0 )
        {
            KFileRelease ( orig );
        }
        else
        {
            self -> f = orig;
            rc = 0;
        }
    }
#endif
    if ( rc == 0 )
        rc = KColumnIdx2Init ( self, eof );
    return rc;
}

/* Whack
 */
rc_t KColumnIdx2Whack ( KColumnIdx2 *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
    {
        int i;
        KColumnIdx2BlockCache * cache=(KColumnIdx2BlockCache *)self -> cstorage.base;
        self -> f = NULL;
        for(i=0;i<self->cstorage.elem_count;i++){
            free(cache[i].block);
        }
        KDataBufferWhack(&self->cstorage);
    }
    return rc;
}

/* LocateBlob
 *  locate an existing blob
 */
static
rc_t KColIdxBlockLocateBlob ( const KColIdxBlock *iblk,
    KColBlobLoc *loc, const KColBlockLoc *bloc, uint32_t count,
    int64_t first, int64_t upper )
{
    uint32_t span;
    int64_t start_id;
    int slot = KColIdxBlockFind ( iblk,
        bloc, count, first, & start_id, & span );
    if ( slot < 0 )
        return RC ( rcDB, rcColumn, rcSelecting, rcBlob, rcNotFound );
    if ( upper > ( start_id + span ) )
        return RC ( rcDB, rcColumn, rcSelecting, rcRange, rcInvalid );

    loc -> start_id = start_id;
    loc -> id_range = span;

    KColIdxBlockGet ( iblk, bloc, count, slot, & loc -> pg, & span );
    loc -> u . blob . size = span;

    return 0;
}

rc_t KColumnIdx2LocateBlob ( const KColumnIdx2 *self,
    KColBlobLoc *loc, const KColBlockLoc *bloc,
    int64_t first, int64_t upper, bool bswap )
{
    rc_t rc;
    uint32_t count;
    size_t block_size, orig;
    uint32_t slot=0;

    void *block;

    /* check within page cache */
    if(self -> cstorage.elem_count > 0){
        KColumnIdx2BlockCache * cache=(KColumnIdx2BlockCache *)self -> cstorage.base;
        assert(self->last < self->cstorage.elem_count);
        if(bloc -> start_id == cache[self->last].start_id){
            rc = KColIdxBlockLocateBlob ( & cache[self->last].iblk, loc, bloc, ( uint32_t ) cache[self->last].count, first, upper );
            if ( rc == 0) return 0;
        } else {
            uint32_t lower,upper,pivot;
            if(bloc -> start_id > cache[self->last].start_id){
                lower=self->last+1;
                upper=self->cstorage.elem_count;
            } else {
                lower=0;
                upper = self->last;
            }
            while(lower < upper){
                pivot = (lower + upper) / 2;
                if(bloc -> start_id == cache[pivot].start_id){
                    KColumnIdx2 * nc_self=(KColumnIdx2 *)self;
                    nc_self->last = pivot;
                    rc = KColIdxBlockLocateBlob ( & cache[self->last].iblk, loc, bloc, ( uint32_t ) cache[self->last].count, first, upper );
                    if ( rc == 0) return 0;
                    goto BSEARCH_DONE;
                } else if(bloc -> start_id < cache[pivot].start_id){
                    upper=pivot;
                } else {
                    lower =pivot+1;
                }
            }
            assert(lower == upper );
            slot = upper;
        }
    }
BSEARCH_DONE:

    /* file may be empty or non-existent */
    if ( self -> eof == 0 )
        return RC ( rcDB, rcColumn, rcSelecting, rcBlob, rcNotFound );

    /* TBD - compression not supported */
    if ( bloc -> u . blk . compressed )
        return RC ( rcDB, rcColumn, rcSelecting, rcData, rcUnsupported );

    /* determine the number of entries in block */
    orig = bloc -> u . blk . size;
    count = KColBlockLocEntryCount ( bloc, & orig );

    /* determine the size to allocate */
    block_size = KColBlockLocAllocSize ( bloc, orig, count );

    /* allocate a block */
        block = malloc ( block_size );
    if ( block == NULL )
        rc = RC ( rcDB, rcColumn, rcSelecting, rcMemory, rcExhausted );
    else
    {
        size_t num_read;
        rc = KFileReadAll ( self -> f, bloc -> pg, block, orig, & num_read );
        if ( rc == 0 )
        {
            if ( num_read != orig )
                rc = RC ( rcDB, rcIndex, rcReading, rcTransfer, rcIncomplete );
            else
            {
                KColIdxBlock iblk;
                rc = KColIdxBlockInit ( & iblk, bloc, orig, block, block_size, bswap );
                if ( rc == 0 )
                {
                    rc = KColIdxBlockLocateBlob ( & iblk,
                        loc, bloc, count, first, upper );
                    if ( rc == 0 )
                    {
                        KColumnIdx2BlockCache * cache;
                        KDataBufferResize(&((KColumnIdx2 *)self)->cstorage,self->cstorage.elem_count+1);
                        cache=(KColumnIdx2BlockCache *)self -> cstorage.base;
                        if(slot < self->cstorage.elem_count -1){ /** not adding to the end **/
                            memmove(cache+slot+1,cache+slot,sizeof(*cache)*(self->cstorage.elem_count - slot - 1));
                        }
                        cache += slot; 
                        cache -> block = block;
                        cache -> start_id = bloc -> start_id;
                        cache -> count = count;
                        cache -> iblk = iblk;
                        return 0;
                    }
                }
            }
        }
        
            free ( block );
    }

    return rc;
}
