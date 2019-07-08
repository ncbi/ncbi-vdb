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

typedef struct lru_page
{
    DLNode node;    /* to make it a valid entry into a DLL */
    uint64_t pos;
    uint64_t block_nr;
    KDataBuffer data;
} lru_page;

typedef struct lru_cache
{
    DLList lru;
    KVector * page_lookup;
    KLock * lock;
    const KFile * wrapped;
#if _DEBUGGING
    on_cache_event handler;
    void * handler_data;
#endif
    size_t page_size;
    uint32_t max_pages;
    volatile uint32_t num_pages;
} lru_cache;

static void CC release_lru_page( DLNode * n, void * data )
{
    if ( n != NULL )
    {
        lru_page * page = ( lru_page * )n;
        KDataBufferWhack ( &page-> data );
        free( ( void * ) page );
    }
}

void release_lru_cache ( lru_cache * self )
{
    if ( self != NULL )
    {
        KLockRelease ( self -> lock );
        if ( self -> page_lookup != NULL )
            KVectorRelease ( self -> page_lookup );
        DLListWhack ( &self -> lru, release_lru_page, NULL );
        free( ( void * ) self );
    }
}

rc_t make_lru_cache ( lru_cache ** cache,
                      const KFile * wrapped,
                      size_t page_size,
                      uint32_t max_pages )
{
    rc_t rc;
    KVector * v;
    
    if ( cache == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    *cache = NULL;
    if ( wrapped == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    if ( page_size == 0 || max_pages == 0  )
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
                p -> page_lookup = v;
                p -> page_size = page_size;
                p -> max_pages = max_pages;
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

static bool read_from_data_buffer( KDataBuffer * data,      /* the data-buffer to read from */
                                   int64_t offset,          /* the offset into the data-buffer */
                                   void * buffer,           /* the buffer to read into */
                                   size_t bsize,            /* how many bytes to read */
                                   size_t * num_read )      /* how many bytes have been read */
{
    int64_t available = ( ( int64_t ) data -> elem_count - offset );
    bool res = ( available > 0 );
    if ( res )
    {
        uint8_t * src = data -> base;
        size_t to_move = bsize > available ? available : bsize;
        memmove( buffer, src + offset, to_move );
        if ( num_read != NULL )
            *num_read = to_move;
    }
    return res;
}

static bool read_from_page( lru_page * page,
                             uint64_t pos,
                             void * buffer,
                             size_t bsize,
                             size_t * num_read )
{
    int64_t offset = ( pos - page -> pos );
    if ( offset >= 0 )
        return read_from_data_buffer( &page -> data, offset, buffer, bsize, num_read );
    return false;
}

static lru_page * get_tail_page( lru_cache * self )
{
    lru_page * res = NULL;
    DLNode * node = DLListPopTail ( & self -> lru );
    if ( node != NULL )
    {
        rc_t rc;
        
        res = ( lru_page * )node;

        if ( self -> num_pages > 0 )
            self -> num_pages--;
        
        rc = KVectorUnset ( self -> page_lookup, res -> block_nr );
        
#if _DEBUGGING
        if ( rc == 0 && self -> handler != NULL )
            self -> handler( self -> handler_data, CE_DISCARD, res -> pos, self -> page_size, res -> block_nr );
#endif
    }
    return res;
}

static rc_t get_a_page( lru_cache * self, lru_page ** page )
{
    rc_t rc = 0;
    *page = NULL;

    /* this should not happen, there should never be more than self->num_pages in the cache */
    while ( self -> num_pages > self -> max_pages )
    {
        lru_page * p = get_tail_page( self );
        release_lru_page( ( DLNode * )p, NULL );
    }
    
    if ( self -> num_pages == self -> max_pages )
    {
        *page = get_tail_page( self );
        if ( *page == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcItem, rcInvalid );
    }
    else
    {
        lru_page * p = calloc( 1, sizeof * p );
        if ( p == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcItem, rcNull );
        else
        {
            rc = KDataBufferMakeBytes( &( p -> data ), self -> page_size );
            if ( rc == 0 )
                *page = p;
            else
                free( ( void * ) p );
        }
    }
    return rc;
}

static rc_t push_to_lru_cache( lru_cache * self, lru_page * page )
{
    rc_t rc = KVectorSetPtr ( self -> page_lookup, page -> block_nr, page );
    if ( rc == 0 )
    {
        DLListPushHead ( & self -> lru, & page -> node );
        self -> num_pages++;
    }

#if _DEBUGGING
    if ( rc == 0 && self -> handler != NULL )
        self -> handler( self -> handler_data, CE_ENTER, page -> pos, self -> page_size, page -> block_nr );
#endif
    return rc;
}

static rc_t new_entry_in_lru_cache ( lru_cache * self,
                                     uint64_t pos,
                                     void * buffer,
                                     size_t bsize,
                                     size_t * num_read,
                                     struct timeout_t *tm )
{
    rc_t rc = 0;
    uint64_t first_block_nr = ( pos / self -> page_size );
    uint64_t last_block_nr = ( ( pos + bsize - 1 ) / self -> page_size );
    uint64_t block_count = ( ( last_block_nr - first_block_nr ) + 1 );
    uint64_t blocks = 1; /* we are not testing block #0, this has been tested by the
                           caller already */
    bool done = false;
    
    /* we are testing how many blocks are not cached... */
    while ( !done && blocks < block_count )
    {
        void * ptr;
        done = ( 0 == KVectorGetPtr ( self -> page_lookup, first_block_nr + blocks, &ptr ) );
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
            size_t elem_count;
            uint64_t first_pos = first_block_nr * self -> page_size; 
            rc = KFileTimedReadAll ( self -> wrapped,
                                first_pos,
                                data . base,
                                self -> page_size * blocks,
                                &elem_count,
                                tm );
            data . elem_count = elem_count;
            if ( rc == 0 )
            {
                /* give the buffer to the caller */
                int64_t offset = ( pos - first_pos );
                if ( !read_from_data_buffer( &data, offset, buffer, bsize, num_read ) )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcTransfer, rcInvalid );
                else
                {
                    /* now portion the data-buffer into blocks and insert them... */
                    uint64_t block = 0;
                    uint64_t data_offset = first_block_nr * self -> page_size;
                    uint64_t available = data . elem_count;
                    while ( rc == 0 && block < blocks )
                    {
                        lru_page * page;
                        rc = get_a_page( self, &page );
                        if ( rc == 0 )
                        {
                            uint8_t * src = data . base;
                            
                            src += ( block * self -> page_size );
                            if ( available > self -> page_size )
                                page -> data . elem_count = self -> page_size;
                            else
                                page -> data . elem_count = available;
                            memmove( page -> data . base, src, page -> data . elem_count );
                            page -> pos = data_offset;
                            page -> block_nr = first_block_nr + block;
                            rc = push_to_lru_cache( self, page );
                            available -= page -> data . elem_count;
                            if ( rc == 0 )
                            {
                                block++;
                                data_offset += self -> page_size;
                            }
                        }
                        
                        if ( rc != 0 )
                            release_lru_page( ( DLNode * )page, NULL );
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
        lru_page * page;
        rc = get_a_page( self, &page );
        if ( rc == 0 )
        {
            size_t elem_count;
            page -> pos = first_block_nr * self -> page_size;
            page -> block_nr = first_block_nr;
            rc = KFileTimedReadAll ( self -> wrapped,
                                     page -> pos,
                                     page -> data . base,
                                     self -> page_size,
                                     & elem_count,
                                     tm );
            page -> data . elem_count = elem_count;
            if ( rc == 0 )
            {
                if ( read_from_page( page, pos, buffer, bsize, num_read ) )
                    rc = push_to_lru_cache( self, page );
                else
                    rc = RC ( rcFS, rcFile, rcConstructing, rcTransfer, rcInvalid );
            }
            if ( rc != 0 )
                release_lru_page( ( DLNode * )page, NULL );
        }
    }
    return rc;
}

enum lookupres { DONE, RD_WRAPPED, NOT_FOUND };

rc_t read_lru_cache ( lru_cache * self,
                      uint64_t pos,
                      void * buffer,
                      size_t bsize,
                      size_t * num_read,
                      struct timeout_t * tm )
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

        rc = KVectorGetPtr ( self -> page_lookup, block_nr, &ptr );
        if ( rc == 0 )
        {
            /* we found the block in the entries! */
            lr = RD_WRAPPED;
            if ( ptr != NULL )
            {
                lru_page * page = ptr;
                if ( read_from_page( page, pos, buffer, bsize, num_read ) )
                {
                    /* put this entry at the top of the LRU-list */
                    DLListUnlink ( & self -> lru, ( DLNode * )page );
                    DLListPushHead ( & self -> lru, ( DLNode * )page );
                    lr = DONE;  /* we are done! */
#if _DEBUGGING
                    if ( self -> handler != NULL )
                        self -> handler ( self -> handler_data, CE_FOUND, pos, self -> page_size, page -> block_nr );
#endif
                }
            }
        }

        switch( lr )
        {
        case RD_WRAPPED :   rc = KFileTimedReadAll ( self -> wrapped, pos, buffer, bsize, num_read, tm );
#if _DEBUGGING
                                if ( self -> handler != NULL )
                                    self -> handler( self -> handler_data, CE_FAILED, pos, *num_read, 0 );
#endif
                                break;

        case NOT_FOUND  :   rc = new_entry_in_lru_cache( self, pos, buffer, bsize, num_read, tm );
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
