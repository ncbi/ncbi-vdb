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

#include "poolpages.h"
#include <kproc/lock.h>

typedef struct PoolPage
{
    uint64_t pos, data_len, allocated;
    uint8_t * data;
    struct ThePool * pool;
    uint32_t idx, readers, usage, blocks;
    bool writing;
} PoolPage;

typedef struct ThePool
{
    PoolPage * pages;
    KLock * lock;
    uint32_t block_size, page_count;
} ThePool;

rc_t make_pool ( ThePool ** pool, uint32_t block_size, uint32_t page_count )
{
    rc_t rc = 0;
    ThePool * p = calloc ( 1, sizeof * p );
    if ( p == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint32_t idx;
        if ( page_count == 0 ) page_count = 8;
        p -> pages = calloc( page_count, sizeof *( p -> pages ) );
        if ( p -> pages != NULL )
        {
            for ( idx = 0; idx < page_count; ++idx )
            {
                p -> pages[ idx ] . idx = idx;
                p -> pages[ idx ] . pool = p;
            }
            p -> block_size = block_size;
            p -> page_count = page_count;
            rc = KLockMake ( & p -> lock );
            if ( rc == 0 )
                *pool = p;
            else
                free ( ( void * ) p -> pages );
        }
        else
        {
            free ( ( void * ) p );
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        }
    }
    return rc;
}

void pool_release ( ThePool * self )
{
    uint32_t i;
    for ( i = 0; i < self -> page_count; ++i )
    {
        PoolPage * p = & self -> pages[ i ];
        if ( p -> data != NULL )
            free( ( void * ) p -> data );
    }
    free( self -> pages );
    KLockRelease ( self -> lock );
    free ( ( void * ) self );
}

uint32_t pool_page_blocks ( const PoolPage * self )
{
    return self -> blocks;
}

uint32_t pool_page_idx ( const PoolPage * self )
{
    return self -> idx;
}

rc_t pool_page_find ( ThePool * self, PoolPage ** found, uint64_t pos )
{
    rc_t rc = KLockAcquire ( self -> lock );
    *found = NULL;    
    if ( rc == 0 )
    {
        /* now we have exclusive access to the PoolPage */
        uint32_t i;
        for ( i = 0; i < self -> page_count && *found == NULL; ++i )
        {
            PoolPage * p = & self -> pages[ i ];
            if ( ( p -> data != NULL ) &&
                 ( pos >= p -> pos ) &&
                 ( pos < ( p -> pos + p -> data_len ) ) &&
                 ( ! p -> writing ) )
            {
                p -> readers += 1;
                p -> usage += 1;
                *found = p;
            }
        }
        KLockUnlock ( self -> lock );
    }
    return rc;
}

rc_t pool_page_get ( const PoolPage * self, uint64_t pos, void *buffer,
                     size_t bsize, size_t *num_read )
{
    uint64_t shift_by = ( pos - self -> pos );
    size_t to_read = ( self -> data_len - shift_by );

    if ( to_read > bsize ) to_read = bsize;
    memmove( buffer, self -> data + shift_by, to_read );
    *num_read = to_read;
    return 0;
}

rc_t pool_page_find_new ( ThePool * self, PoolPage ** found )
{
    rc_t rc = KLockAcquire ( self -> lock );
    *found = NULL;    
    if ( rc == 0 )
    {
        /* now we have exclusive access to the PoolPage */
        uint32_t i;
        
        /* first try: find a not used entry */
        for ( i = 0; i < self -> page_count && *found == NULL; ++i )
        {
            PoolPage * p = & self -> pages[ i ];
            if ( ( p -> data == NULL ) &&
                 ( ! p -> writing ) )
            {
                *found = p;
            }
        }
        
        /* second try: find the least used entry,
           not currently used for reading or writing */
        if ( *found == NULL )
        {
            uint32_t used = 0xFFFFFFFF;
            for ( i = 0; i < self -> page_count; ++i )
            {
                PoolPage * p = & self -> pages[ i ];
                if ( ( p -> data != NULL ) &&
                     ( ! p -> writing ) &&
                     ( p -> usage < used ) &&
                     ( p -> readers == 0 ) )
                {
                    used = p -> usage;
                    *found = p;
                }
            }
        }
        if ( *found != NULL )
        {
            ( *found ) -> writing = true;
            ( *found ) -> usage = 1;
        }
        KLockUnlock ( self -> lock );
    }
    return rc;
}

rc_t pool_page_release ( PoolPage * self )
{
    rc_t rc = KLockAcquire ( self -> pool -> lock );
    if ( rc == 0 )
    {
        if ( self -> writing )
            self -> writing = false;
        else
        {
            if ( self -> readers > 0 )
                self -> readers -= 1;
        }
        KLockUnlock ( self -> pool -> lock );
    }
    return rc;
}

rc_t pool_page_prepare( PoolPage * self, uint32_t count, uint64_t pos )
{
    rc_t rc = 0;
    uint64_t len;
    self -> blocks = count > 2 ? 2 : count;
    len = ( self -> blocks * self -> pool -> block_size );
    if ( self -> data == NULL )
    {
        /* the page has a NULL-data-ptr : allocate as much buffer as we need */
        self -> data = malloc( len );
        self -> allocated = len;
    }
    else if ( len > self -> allocated )
    {
        /* the page has a valid data-ptr, but it does not have enough buffer : reallocate as much buffer as we need */
        free ( ( void * ) self -> data );
        self -> data = malloc( len );
        self -> allocated = len;
    }
    if ( self -> data == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    
    if ( rc == 0 )
    {
        self -> data_len = len;
        self -> pos = pos;
        self -> usage = 1;
    }
    return rc;
}

rc_t pool_page_read_from_file( PoolPage * self, const KFile * f, size_t * read )
{
    return KFileReadAll ( f, self -> pos, self -> data, self -> data_len, read );
}

rc_t pool_page_write_to_file( const PoolPage * self, KFile * f, size_t * written )
{
    return KFileWriteAll ( f, self -> pos, self -> data, self -> data_len, written );
}

rc_t pool_page_write_to_recorder( const PoolPage * self, struct Recorder * rec )
{
    return WriteToRecorder ( rec, "\tram\tat:%lu\tlen:%lu ( idx = %d )\n",
                             self -> pos, self -> data_len, self -> idx );
}
