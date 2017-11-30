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
#include <klib/container.h>
#include <klib/data-buffer.h>
#include <klib/vector.h>

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
    uint32_t * scratch;
    KLock * lock;
    uint32_t block_size, page_count, scratch_select;
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
            p -> scratch = calloc( page_count, sizeof *( p -> scratch ) );
            if ( p -> scratch )
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
                free( ( void * ) p -> pages );
            }
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
    free( self -> scratch );
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

uint32_t pool_page_usage ( const PoolPage * self )
{
    return self -> usage;
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
            else
            {
                if ( p -> usage > 1 )
                    p -> usage -= 1;
            }
        }
        
        /* second try: find the least used entry,
           not currently used for reading or writing */
        if ( *found == NULL )
        {
            uint32_t min_used = 0xFFFFFFFF;
            /* step 1 : find the min-usage value that is not busy */
            for ( i = 0; i < self -> page_count; ++i )
            {
                PoolPage * p = & self -> pages[ i ];
                if ( ( p -> data != NULL ) &&
                     ( ! p -> writing ) &&
                     ( p -> usage < min_used ) &&
                     ( p -> readers == 0 ) )
                {
                    min_used = p -> usage;
                }
            }
            /* step 2 : record the pages that have this value in the scratch */
            if ( min_used < 0xFFFFFFFF )
            {
                uint32_t dst = 0;
                for ( i = 0; i < self -> page_count; ++i )
                {
                    PoolPage * p = & self -> pages[ i ];
                    if ( ( p -> data != NULL ) &&
                         ( ! p -> writing ) &&
                         ( p -> usage == min_used ) &&
                         ( p -> readers == 0 ) )
                    {
                        self -> scratch[ dst++ ] = p -> idx;
                    }
                }
                if ( dst > 0 )
                {
                    if ( self -> scratch_select >= dst )
                        self -> scratch_select = 0;
                    *found = & self -> pages[ self -> scratch[ self -> scratch_select ++ ] ];
                }
            }
        }

        if ( *found != NULL )
        {
            ( *found ) -> writing = true;
            ( *found ) -> usage = 1;
        }
        else
            rc = SILENT_RC ( rcFS, rcFile, rcReading, rcItem, rcNotFound );
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

rc_t pool_page_write_to_file( const PoolPage * self, KFile * f, size_t to_write, size_t * written )
{
    return KFileWriteAll ( f, self -> pos, self -> data, to_write, written );
}

rc_t pool_page_write_to_recorder( const PoolPage * self, struct Recorder * rec )
{
    return WriteToRecorder ( rec, "\tram\tat:%lu\tlen:%lu ( idx = %d )\n",
                             self -> pos, self -> data_len, self -> idx );
}

/* -------------------------------------------------- */

typedef struct lru_entry
{
    DLNode node;    /* to make it a valid entry into a DLL */
    uint64_t pos;
    uint64_t block_nr;
    KDataBuffer data;
} lru_entry;

typedef struct lru_cache
{
    DLList lru;
    KVector * entries;
    KLock * lock;
    const KFile * wrapped;
    struct Recorder * rec;
    size_t block_size;    
    uint32_t max_entries;
    uint32_t num_entries;
} lru_cache;

static void CC release_lru_entry( DLNode * n, void * data )
{
    if ( n != NULL )
    {
        lru_entry * entry = ( lru_entry * )n;
        KDataBufferWhack ( &entry -> data );
        free( ( void * ) entry );
    }
}

void release_lru_cache ( lru_cache * self )
{
    if ( self != NULL )
    {
        KLockRelease ( self -> lock );
        if ( self -> entries != NULL )
            KVectorRelease ( self -> entries );
        DLListWhack ( &self -> lru, release_lru_entry, NULL );
        free( ( void * ) self );
    }
}

rc_t make_lru_cache ( lru_cache ** cache,
                      const KFile * wrapped,
                      size_t block_size,
                      uint32_t max_entries )
{
    rc_t rc;
    KVector * v;
    
    if ( cache == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    *cache = NULL;
    if ( wrapped == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    if ( block_size == 0 || max_entries == 0  )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcInvalid );

    rc = KVectorMake ( &v );
    if ( rc == 0 )
    {
        lru_cache * p = calloc ( 1, sizeof * p );
        if ( p == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KLockMake ( & p -> lock );
            if ( rc == 0 )
            {
                /* the work of DLListInit() aka setting head+tail to NULL
                   is already done by calloc() */
                p -> entries = v;
                p -> block_size = block_size;
                p -> max_entries = max_entries;
                p -> wrapped = wrapped;
                *cache = p;
            }
            else
                free ( ( void * ) p );
        }
        if ( rc != 0 )
            KVectorRelease ( v );
    }
    return rc;
}

void set_recorder_for_lru_cache ( lru_cache * self, struct Recorder * rec )
{
    if ( self != NULL )
        self -> rec = rec;
}

static bool read_from_data_buffer( KDataBuffer * data, int64_t offset,
                             uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    int64_t available = ( data -> elem_count - offset );
    bool res = ( available > 0 );
    if ( res )
    {
        uint8_t * src = data -> base;
        size_t to_move = bsize > available ? available : bsize;
        src += offset;
        memmove( buffer, src, to_move );
        if ( num_read != NULL )
            *num_read = to_move;
    }
    return res;
}

static bool read_from_entry( lru_entry * entry,
                             uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    int64_t offset = ( pos - entry -> pos );
    return read_from_data_buffer( &entry -> data, offset, pos, buffer, bsize, num_read );
}

static rc_t push_to_lru_cache( lru_cache * self, lru_entry * entry )
{
    rc_t rc = 0;
    while ( rc == 0 && ( self -> num_entries >= self -> max_entries ) )
    {
        DLNode * node = DLListPopTail ( & self -> lru );
        if ( self -> num_entries > 0 )
            self -> num_entries--;
        if ( node != NULL )
        {
            lru_entry * e = ( lru_entry * )node;
            rc = KVectorUnset ( self -> entries, e -> block_nr );
            release_lru_entry( node, NULL );
        }
    }
    if ( rc == 0 )
        rc = KVectorSetPtr ( self -> entries, entry -> block_nr, entry );
    if ( rc == 0 )
        DLListPushHead ( & self -> lru, ( DLNode * )entry ); 
    return rc;
}

static rc_t new_entry_in_lru_cache ( lru_cache * self,
                      uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc = 0;
    uint64_t first_block_nr = ( pos / self -> block_size );
    uint64_t last_block_nr = ( ( pos + bsize ) / self -> block_size );
    uint64_t block_count = ( ( last_block_nr - first_block_nr ) + 1 );
    uint64_t blocks = 1; /* we are not testing block #0, this has been tested by the
                           caller already */
    bool done = false;
    /* we are testing how many blocks are not cached... */
    while ( !done && blocks < block_count )
    {
        void * ptr;
        done = ( 0 == KVectorGetPtr ( self -> entries, first_block_nr + blocks, &ptr ) );
        if ( !done )
            blocks++;
    }
    
    if ( blocks > 1 )
    {
        /* we have to produce multiple blocks */
        KDataBuffer data;
        rc = KDataBufferMakeBytes( &data, self -> block_size * blocks );
        if ( rc == 0 )
        {
            /* read the whole request from the wrapped file */
            uint64_t first_pos = first_block_nr * self -> block_size; 
            rc = KFileReadAll ( self -> wrapped,
                                first_pos,
                                data . base,
                                self -> block_size * blocks,
                                &data . elem_count );
            if ( rc == 0 )
            {
                /* give the buffer to the caller */
                int64_t offset = ( pos - first_pos );
                if ( !read_from_data_buffer( &data, offset, pos, buffer, bsize, num_read ) )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcTransfer, rcInvalid );
                else
                {
                    /* now portion the data-buffer into blocks and insert them... */
                    uint64_t block = 0;
                    uint64_t data_offset = first_block_nr * self -> block_size;
                    while ( rc == 0 && block < blocks )
                    {
                        lru_entry * entry = calloc( 1, sizeof * entry );
                        if ( entry == NULL )
                            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                        else
                        {
                            rc = KDataBufferSub ( &data, &entry -> data, block * self -> block_size, self -> block_size );
                            if ( rc == 0 )
                            {
                                entry -> pos = data_offset;
                                entry -> block_nr = first_block_nr + block;
                                rc = push_to_lru_cache( self, entry );
                            }
                            if ( rc != 0 )
                                release_lru_entry( ( DLNode * )entry, NULL );
                        }
                        block++;
                        data_offset += self -> block_size;
                    }
                }
            }
            /* the sub-buffer's will keep the data alive and ref-counted... */
            KDataBufferWhack ( &data );
        }
    }
    else
    {
        /* we have to produce just one block */
        lru_entry * entry = calloc( 1, sizeof * entry );
        if ( entry == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KDataBufferMakeBytes( & entry -> data, self -> block_size );
            if ( rc == 0 )
            {
                entry -> pos = first_block_nr * self -> block_size;
                entry -> block_nr = first_block_nr;
                rc = KFileReadAll ( self -> wrapped,
                                    entry -> pos,
                                    entry -> data . base,
                                    self -> block_size,
                                    &entry -> data . elem_count );
                if ( rc == 0 )
                {
                    if ( read_from_entry( entry, pos, buffer, bsize, num_read ) )
                        rc = push_to_lru_cache( self, entry );
                    else
                        rc = RC ( rcFS, rcFile, rcConstructing, rcTransfer, rcInvalid );
                }
            }
            if ( rc != 0 )
                release_lru_entry( ( DLNode * )entry, NULL );
        }
    }
    return rc;
}

enum lookupres { DONE, RD_WRAPPED, NOT_FOUND };

rc_t read_lru_cache ( lru_cache * self,
                      uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    if ( buffer == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    enum lookupres lr = NOT_FOUND;
    
    rc = KLockAcquire ( self -> lock );
    if ( rc == 0 )
    {
        void * ptr;
        rc = KVectorGetPtr ( self -> entries, pos / self -> block_size, &ptr );
        if ( rc == 0 )
        {
            /* we found the block in the entries! */
            lr = RD_WRAPPED;
            if ( ptr != NULL )
            {
                lru_entry * entry = ptr;
                if ( read_from_entry( entry, pos, buffer, bsize, num_read ) )
                {
                    /* put this entry at the top of the LRU-list */
                    DLListUnlink ( & self -> lru, ( DLNode * )entry );
                    DLListPushHead ( & self -> lru, ( DLNode * )entry );
                    lr = DONE;
                }
            }
        }
        switch( lr )
        {
            case RD_WRAPPED : rc = KFileReadAll ( self -> wrapped, pos, buffer, bsize, num_read ); break;
            case NOT_FOUND : rc = new_entry_in_lru_cache( self, pos, buffer, bsize, num_read ); break;
            case DONE : break;
        }
        KLockUnlock ( self -> lock );
    }
    return rc;
}
