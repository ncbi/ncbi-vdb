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

#include "lru_cache.h"
#include <kproc/lock.h>
#include <klib/container.h>
#include <klib/data-buffer.h>
#include <klib/vector.h>

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
#if _DEBUGGING
    on_cache_event handler;
    void * handler_data;
#endif
    size_t page_size;
    uint32_t page_count;
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
                      size_t page_size,
                      uint32_t page_count )
{
    rc_t rc;
    KVector * v;
    
    if ( cache == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    *cache = NULL;
    if ( wrapped == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    if ( page_size == 0 || page_count == 0  )
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
                p -> page_size = page_size;
                p -> page_count = page_count;
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
    while ( rc == 0 && ( self -> num_entries >= self -> page_count ) )
    {
        DLNode * node = DLListPopTail ( & self -> lru );
        if ( self -> num_entries > 0 )
            self -> num_entries--;
        if ( node != NULL )
        {
            lru_entry * e = ( lru_entry * )node;
            rc = KVectorUnset ( self -> entries, e -> block_nr );
#if _DEBUGGING
            if ( rc == 0 && self -> handler != NULL )
                self -> handler( self -> handler_data, CE_DISCARD, e -> pos, self -> page_size, e -> block_nr );
#endif
            release_lru_entry( node, NULL );
        }
    }
    if ( rc == 0 )
        rc = KVectorSetPtr ( self -> entries, entry -> block_nr, entry );
    if ( rc == 0 )
        DLListPushHead ( & self -> lru, ( DLNode * )entry );
    if ( rc == 0 )
        self -> num_entries++;

#if _DEBUGGING
    if ( rc == 0 && self -> handler != NULL )
        self -> handler( self -> handler_data, CE_ENTER, entry -> pos, self -> page_size, entry -> block_nr );
#endif
    return rc;
}

static rc_t new_entry_in_lru_cache ( lru_cache * self,
                      uint64_t pos, void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc = 0;
    uint64_t first_block_nr = ( pos / self -> page_size );
    uint64_t last_block_nr = ( ( pos + bsize ) / self -> page_size );
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
        rc = KDataBufferMakeBytes( &data, self -> page_size * blocks );
        if ( rc == 0 )
        {
            /* read the whole request from the wrapped file */
            uint64_t first_pos = first_block_nr * self -> page_size; 
            rc = KFileReadAll ( self -> wrapped,
                                first_pos,
                                data . base,
                                self -> page_size * blocks,
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
                    uint64_t data_offset = first_block_nr * self -> page_size;
                    while ( rc == 0 && block < blocks )
                    {
                        lru_entry * entry = calloc( 1, sizeof * entry );
                        if ( entry == NULL )
                            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                        else
                        {
                            rc = KDataBufferSub ( &data, &entry -> data, block * self -> page_size, self -> page_size );
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
                        data_offset += self -> page_size;
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
            rc = KDataBufferMakeBytes( & entry -> data, self -> page_size );
            if ( rc == 0 )
            {
                entry -> pos = first_block_nr * self -> page_size;
                entry -> block_nr = first_block_nr;
                rc = KFileReadAll ( self -> wrapped,
                                    entry -> pos,
                                    entry -> data . base,
                                    self -> page_size,
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
    enum lookupres lr = NOT_FOUND;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    if ( buffer == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    rc = KLockAcquire ( self -> lock );
    if ( rc == 0 )
    {
        void * ptr;
        uint64_t block_nr = pos / self -> page_size;
        
#if _DEBUGGING
        if ( self -> handler != NULL )
            self -> handler ( self -> handler_data, CE_REQUEST, pos, bsize, block_nr );
#endif

        rc = KVectorGetPtr ( self -> entries, block_nr, &ptr );
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
#if _DEBUGGING
                    if ( self -> handler != NULL )
                        self -> handler ( self -> handler_data, CE_FOUND, pos, self -> page_size, entry -> block_nr );
#endif
                }
            }
        }

        switch( lr )
        {
            case RD_WRAPPED :   rc = KFileReadAll ( self -> wrapped, pos, buffer, bsize, num_read );
#if _DEBUGGING
                                if ( self -> handler != NULL )
                                    self -> handler( self -> handler_data, CE_FAILED, pos, *num_read, 0 );
#endif
                                break;

            case NOT_FOUND  :   rc = new_entry_in_lru_cache( self, pos, buffer, bsize, num_read );
                                break;

            case DONE : break;
        }
        KLockUnlock ( self -> lock );
    }
    return rc;
}

rc_t set_lru_cache_event_handler( struct lru_cache * self, void * data, on_cache_event handler )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    if ( handler == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
#if _DEBUGGING
    self -> handler = handler;
    self -> handler_data = data;
#endif
    return rc;
}
