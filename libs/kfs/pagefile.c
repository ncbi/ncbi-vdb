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

#include <kfs/extern.h>
#include <kfs/pagefile.h>
#include <kfs/impl.h>
#include <klib/container.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <atomic.h>
#include <sysalloc.h>


#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define REQUIRE_RANDOM_ACCESS 0

#ifdef _DEBUGGING
#define PAGE_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_PAGE),msg)
#else
#define PAGE_DEBUG(msg)
#endif


/*--------------------------------------------------------------------------
 * KPageMem
 *  the raw page memory
 */

/* must be kept in sync with kdb/btree.c */
#define PGBITS 15
#define PGSIZE ( 1U << PGBITS )

static
rc_t KPageMemNew ( void **ppage )
{
    void *page = calloc ( PGSIZE, 1 );
    if ( page == NULL )
        return RC ( rcFS, rcBlob, rcAllocating, rcMemory, rcExhausted );

    * ppage = page;
    return 0;
}

static
rc_t KPageMemMake ( void **ppage )
{
    void *page = malloc ( PGSIZE );
    if ( page == NULL )
        return RC ( rcFS, rcBlob, rcAllocating, rcMemory, rcExhausted );

    * ppage = page;
    return 0;
}

static
void KPageMemWhack ( void *page )
{
    free ( page );
}


/*--------------------------------------------------------------------------
 * KPageBacking
 *  a reference KFile wrapper
 */
typedef struct KPageBacking KPageBacking;
struct KPageBacking
{
    uint64_t eof;
    KFile *backing;
    KRefcount refcount;
    bool write_through;
    bool have_eof;
};

/* Whack
 */
static
void KPageBackingWhack ( KPageBacking *self )
{
    if(self -> backing) KFileRelease ( self -> backing );
    free ( self );
}

/* Attach
 */
static
KPageBacking *KPageBackingAttach ( const KPageBacking *self )
{
    if ( self != NULL ) switch ( KRefcountAddDep ( & self -> refcount, "KPageBacking" ) )
    {
    case krefOkay:
        return ( KPageBacking* ) self;
    }

    return NULL;
}

/* Sever
 */
static
void KPageBackingSever ( const KPageBacking *self )
{
    if ( self != NULL ) switch ( KRefcountDropDep ( & self -> refcount, "KPageBacking" ) )
    {
    case krefWhack:
        KPageBackingWhack ( ( KPageBacking* ) self );
        break;
    }
}

/* Release
 */
static
void KPageBackingRelease ( const KPageBacking *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KPageBacking" ) )
    {
    case krefWhack:
        KPageBackingWhack ( ( KPageBacking* ) self );
        break;
    }
}

/* Drop
 *  drop backing file
 */
static
rc_t KPageBackingDrop ( KPageBacking *self )
{
    KFile *backing;

    if ( self == NULL )
        return RC ( rcFS, rcFile, rcDetaching, rcSelf, rcNull );

    backing = self -> backing;
    if ( backing != NULL && atomic_test_and_set_ptr ( ( atomic_ptr_t* ) & self -> backing, NULL, backing ) == backing )
        KFileRelease ( backing );

    self -> write_through = false;

    return 0;
}

/* SetSize
 *  sets size of backing store
 */
#define BACKING_FILE_GROWTH 1024 /** number of pages to grow at a time**/
#define BACKING_FILE_MASK ( BACKING_FILE_GROWTH - 1 )
static
rc_t KPageBackingSetSize ( KPageBacking *self, uint32_t pg_count )
{
    uint64_t new_eof;

    if ( pg_count > BACKING_FILE_GROWTH )
        pg_count = ( pg_count + BACKING_FILE_MASK ) & ~ BACKING_FILE_MASK;

    new_eof =  ( uint64_t ) pg_count << PGBITS;
    if ( new_eof == self -> eof )
        return 0;

    self -> eof = new_eof;
    return KFileSetSize ( self -> backing, self -> eof );
}

/* Read
 *  read a page from backing
 */
#if 0
static
rc_t KFileReadAll ( const KFile *self, uint64_t pos, uint8_t *buff, size_t bsize, size_t *num_read )
{
    rc_t rc;
    size_t total, partial;

    for ( rc = 0, total = 0; total < bsize; total += partial )
    {
        rc = KFileRead ( self, pos + total, & buff [ total ], bsize - total, & partial );
        if ( rc != 0 )
            break;
        if ( partial == 0 )
            break;
    }

    if ( total != 0 )
    {
        * num_read = total;
        return 0;
    }

    * num_read = 0;
    return rc;
}
#endif

static
rc_t KPageBackingRead ( const KPageBacking *self, void **ppage, uint32_t pg_id )
{
    rc_t rc;
    uint64_t pos;

    /* can't read if backing isn't there */
    if ( self -> backing == NULL )
    {
        * ppage = NULL;
        return RC ( rcFS, rcFile, rcReading, rcBlob, rcNotFound );
    }

    /* the offset to the first byte after requested page */
    pos = ( uint64_t ) pg_id << PGBITS;

    /* allocate a page */
    rc = KPageMemMake ( ppage );
    if ( rc == 0 )
    {
        /* retrieve memory */
        void *page = * ppage;

        /* read page from file */
        size_t num_read;
        rc = KFileReadAll ( self -> backing, pos -= PGSIZE, page, PGSIZE, & num_read );
        if ( rc == 0 )
        {
            if ( num_read != 0 )
            {
                /* keep track of eof */
                pos += num_read;
                if ( self -> eof < pos )
                    ( ( KPageBacking* ) self ) -> eof = pos;

                /* detect a partial page */
                if ( num_read < PGSIZE )
                    memset ( & ( ( uint8_t* ) page ) [ num_read ], 0, PGSIZE - num_read );

                /* return page */
                * ppage = page;
                return 0;
            }

            rc = RC ( rcFS, rcFile, rcReading, rcBlob, rcNotFound );
        }

        KPageMemWhack ( page );
    }

    * ppage = NULL;
    return rc;
}


/* Write
 *  write a page to backing store
 */
#if 0
static
rc_t KFileWriteAll ( KFile *self, uint64_t pos, const uint8_t *buff, size_t size, size_t *num_writ )
{
    rc_t rc;
    size_t total, partial;

    for ( rc = 0, total = 0; total < size; total += partial )
    {
        rc = KFileWrite ( self, pos + total, & buff [ total ], size - total, & partial );
        if ( rc != 0 )
            break;
        if ( partial == 0 )
        {
            rc = RC ( rcFS, rcBlob, rcWriting, rcTransfer, rcIncomplete );
            break;
        }
    }

    if ( total != 0 )
    {
        * num_writ = total;
        return 0;
    }

    * num_writ = 0;
    return rc;
}
#endif

static
rc_t KPageBackingWrite ( KPageBacking *self, const void *page, uint32_t pg_id )
{
    rc_t rc;
    uint64_t pos;
    size_t num_writ;

    /* allow backing to be removed */
    if ( self -> backing == NULL )
    {
        self -> write_through = false;
        return 0;
    }

    /* the offset to the first byte after requested page */
    pos = ( uint64_t ) pg_id << PGBITS;

    /* write the page */
    rc = KFileWriteAll ( self -> backing, pos -= PGSIZE, page, PGSIZE, & num_writ );
    if ( rc == 0 )
    {
        pos += num_writ;
        if ( self -> eof < pos )
            self -> eof = pos;

        if ( num_writ == PGSIZE )
            return 0;

        /* technically it is permissible for the file to deny us a complete page
           but for this test, the file should accept whole pages.
        */
        rc = RC ( rcFS, rcBlob, rcWriting, rcTransfer, rcIncomplete );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KPage
 *  a reference counted page
 *  meant to be held within a BSTree and DLList
 */
struct KPage
{
    /* head for LRU cache */
    DLNode ln;

    /* backing file */
    KPageBacking *backing;

    /* raw memory */
    void *page;

    KRefcount refcount;

    uint32_t page_id;
    bool read_only;
    bool dirty;
};



/* Whack
 */
static
rc_t KPageWhack ( KPage *self )
{
    if ( self -> backing )
    {
        if ( self -> dirty )
            KPageBackingWrite ( self -> backing, self -> page, self -> page_id );
        KPageBackingSever ( self -> backing );
    }
    KPageMemWhack ( self -> page );
    free ( self );
    return 0;
}


/* New
 *  make a new page
 */
static
rc_t KPageNew ( KPage **ppage, KPageBacking * backing, uint32_t page_id )
{
    rc_t rc;
    KPage *page = malloc ( sizeof * page );
    if ( page == NULL )
        rc = RC ( rcFS, rcBlob, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KPageMemNew ( & page -> page );
        if ( rc == 0 )
        {
            if ( backing != NULL )
                page -> backing = KPageBackingAttach ( backing );
            else
                page -> backing = NULL;
            KRefcountInit ( & page -> refcount, 1, "KPage", "new", "page" );
            page -> page_id = page_id;
            page -> read_only = false;
            page -> dirty = false;

            * ppage = page;
            return 0;
        }

        free ( page );
    }

    * ppage = NULL;
    return rc;
}


/* Make
 *  make a page from backing
 */
static
rc_t KPageMake ( KPage **ppage, KPageBacking *backing, uint32_t page_id )
{
    rc_t rc;
    KPage *page = malloc ( sizeof * page );
    if ( page == NULL )
        rc = RC ( rcFS, rcBlob, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KPageBackingRead ( backing, & page -> page, page_id );
        if ( rc == 0 )
        {
            page -> backing = KPageBackingAttach ( backing );
            KRefcountInit ( & page -> refcount, 1, "KPage", "make", "page" );
            page -> page_id = page_id;
            page -> read_only = false;
            page -> dirty = false;

            * ppage = page;
            return 0;
        }

        free ( page );
    }

    * ppage = NULL;
    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KPageAddRef ( const KPage *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KPage" ) )
    {
    case krefOkay:
        break;
    default:
        return RC ( rcFS, rcBlob, rcAttaching, rcConstraint, rcViolated );
    }

    return 0;
}

LIB_EXPORT rc_t CC KPageRelease ( const KPage *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KPage" ) )
    {
    case krefOkay:
        if ( self -> dirty && self -> backing && self -> backing -> write_through )
        {
            rc_t rc = KPageBackingWrite ( self -> backing, self -> page, self -> page_id );
            if ( rc != 0 )
                return rc;

            ( ( KPage* ) self ) -> dirty = false;
        }
        break;
    case krefWhack:
        return KPageWhack ( ( KPage* ) self );
    default:
        return RC ( rcFS, rcBlob, rcReleasing, rcConstraint, rcViolated );
    }

    return 0;
}


/* Attach
 * Sever
 *  ignores NULL references
 */
static
void KPageAttach ( const KPage *self )
{
    if ( self != NULL )
        KRefcountAddDep ( & self -> refcount, "KPage" );
}

static
rc_t KPageSever ( const KPage *self )
{
    if ( self != NULL ) switch ( KRefcountDropDep ( & self -> refcount, "KPage" ) )
    {
    case krefOkay:
        break;
    case krefWhack:
        return KPageWhack ( ( KPage* ) self );
    default:
        return RC ( rcFS, rcBlob, rcReleasing, rcConstraint, rcViolated );
    }

    return 0;
}



/* Id
 *  returns page id
 *
 *  "page_id" [ OUT ] - return parameter for page id
 */
LIB_EXPORT rc_t CC KPageId ( const KPage *self, uint32_t *page_id )
{
    rc_t rc;

    if ( page_id == NULL )
        rc = RC ( rcFS, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcBlob, rcAccessing, rcSelf, rcNull );
        else
        {
            * page_id = self -> page_id;
            return 0;
        }

        * page_id = 0;
    }

    return rc;
}


/* ConstSize
 *  returns constant page size
 */
LIB_EXPORT size_t CC KPageConstSize ( void )
{
    return PGSIZE;
}


/* AccessRead
 * AccessUpdate
 *  gain access to page memory
 *  update access marks page as modified
 */
LIB_EXPORT rc_t CC KPageAccessRead ( const KPage *self, const void **mem, size_t *bytes )
{
    rc_t rc;

    size_t dummy;
    if ( bytes == NULL )
        bytes = & dummy;

    if ( mem == NULL )
        rc = RC ( rcFS, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcBlob, rcAccessing, rcSelf, rcNull );
        else
        {
            assert ( self -> page != NULL );
            * mem = self -> page;
            * bytes = PGSIZE;
            return 0;
        }

        * mem = NULL;
    }

    * bytes = 0;

    return rc;
}

LIB_EXPORT rc_t CC KPageAccessUpdate ( KPage *self, void **mem, size_t *bytes )
{
    rc_t rc;

    size_t dummy;
    if ( bytes == NULL )
        bytes = & dummy;

    if ( mem == NULL )
        rc = RC ( rcFS, rcBlob, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcBlob, rcAccessing, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcFS, rcBlob, rcAccessing, rcBlob, rcReadonly );
        else
        {
            assert ( self -> page != NULL );
            self -> dirty = true;
            * mem = self -> page;
            * bytes = PGSIZE;
            return 0;
        }

        * mem = NULL;
    }

    * bytes = 0;

    return rc;
}


/*--------------------------------------------------------------------------
 * KPageFile
 *  presents some level of page management on top of a random-access KFile
 */
struct KPageFile
{
    void   **page_idx;
    uint8_t  page_idx_depth;
    DLList by_access;
    KPageBacking *backing;
    KRefcount refcount;
    uint32_t count;
    uint32_t ccount;
    uint32_t climit;
    bool read_only;
};


static
void * KPageFile_whack_recursive( void **idx, uint8_t depth, uint32_t count,
                                  DLList * mru, uint32_t * ccount )
{
    if ( idx != NULL )
    {
        if ( depth == 0 )
        {
            rc_t rc;
            KPage * page = ( KPage* )idx;
            DLListUnlink( mru, &page->ln );
            rc=KPageSever( page );
            if ( ccount != NULL )
                ccount[ 0 ]--;
            assert( rc == 0 );
        }
        else
        {
            int i;
            uint8_t offset;

            depth--;
            offset = ( count >> ( 8 * depth ) ) & 0xff;
            idx[ offset ] = KPageFile_whack_recursive( (void**)idx[ offset ], depth, count, mru, ccount );
            for ( i = ( int )offset + 1; i < 256; i++ )
            {
                if ( idx[ i ] != NULL )
                {
                    idx[ i ] = KPageFile_whack_recursive( (void**)idx[ i ], depth, 0, mru, ccount );
                }
            }   

            if ( depth == 0 )
            {
                if ( offset == 0 )
                {
                    free( idx );
                    return NULL;
                }
                else
                {
                    return idx;
                }
            }
            else if ( offset == 0 || ( offset == 1 && idx[ 1 ] == NULL ) )
            {
                void *tmp = idx[ 0 ];
                free( idx );
                return tmp;
            }
            else
            {
                return idx;
            }
        }
    }
    return NULL;
}

#define PAGE_IDX_DEPTH(A) ((A)>(1<<24)?4:((A)>(1<<16)?3:((A)>256?2:((A)>0?1:0))))

static
rc_t KPageFileSetPageCount( KPageFile * self, uint32_t count )
{
    void **tmp; 
    uint8_t new_depth = PAGE_IDX_DEPTH( count );

    /******* Adjust depth and change structure of the index if needed ****/
    if ( new_depth > self->page_idx_depth )
    {
        if ( self->page_idx == NULL )
        {
            self->page_idx_depth = new_depth;
        }
        else while ( self->page_idx_depth < new_depth )
        {
            tmp = malloc( 256 * sizeof( *tmp ) );
            memset( tmp, 0, 256 * sizeof( *tmp ) );
            tmp[ 0 ] = self->page_idx;
            self->page_idx = tmp;
            self->page_idx_depth++;
        }
    }

    if ( count < self->count )
    {
        self->page_idx = KPageFile_whack_recursive( self->page_idx, self->page_idx_depth,
                                                    count, &self->by_access, &self->ccount );
        self->page_idx_depth = new_depth;
    }
    self->count=count;
    if ( self -> read_only )
        return 0;

    if ( self -> backing )
        return KPageBackingSetSize ( self -> backing, count );
    else if ( self->count > self->climit )
    {
        return RC ( rcFS, rcFile, rcProcessing, rcBuffer, rcExhausted );
    }

    return 0;
}
/* Whack
 */

static
rc_t KPageFileWhack ( KPageFile *self )
{
    /* first, visit each cached page in flush order */
    self->page_idx = KPageFile_whack_recursive( self->page_idx, self->page_idx_depth,
                                                0, &self->by_access, &self->ccount );
    /* release the backing file */
    if ( self -> backing )
        KPageBackingRelease ( self -> backing );

    /* delete the object */
    free ( self );
    return 0;
}

/* Make
 *  creates a page file
 *
 *  "pf" [ OUT ] - return parameter for page file
 *
 *  "backing" [ IN ] - backing file
 *   NB - attaches a new reference to file
 *        does not take ownership
 *
 *  "climit" [ IN ] - cache size limit
 */

#define MIN_CACHE_PAGE 2

LIB_EXPORT rc_t CC KPageFileMakeRead ( const KPageFile **pf, const KFile * backing, size_t climit )
{
    rc_t rc;

    if ( pf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( backing == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! backing -> read_enabled )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        else
        {
            KPageBacking *bf = calloc ( 1, sizeof * bf );
            if ( bf == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                KPageFile *f = malloc ( sizeof * f );
                if ( f == NULL )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    rc = KFileAddRef ( backing );
                    if ( rc == 0 )
                    {
                        f->page_idx = NULL;
                        f->page_idx_depth = 0;
                        DLListInit ( & f -> by_access );
                        KRefcountInit ( & f -> refcount, 1, "KPageFile", "make", "pgfile" );
                        f -> count = 0;
                        f -> ccount = 0;
                        f -> climit = ( uint32_t ) ( climit >>  PGBITS );
                        if ( f -> climit < MIN_CACHE_PAGE )
                            f -> climit = MIN_CACHE_PAGE;

                        PAGE_DEBUG( ( "PAGE: KPageFileMakeRead {%p} limit = %u\n", f, f->climit ) );

                        f -> read_only = true;

                        /* finish the backing file */
                        KRefcountInit ( & bf -> refcount, 1, "KPageBacking", "make", "backing" );
                        f -> backing = bf;
                        bf -> write_through = false;

                        /* attached reference */
                        bf -> backing = ( KFile* ) backing;

                        /* if backing is random access, get its size */
                        rc = KFileSize ( backing, & bf -> eof );
                        if ( rc == 0 )
                        {
                            bf -> have_eof = true;
                            rc = KPageFileSetPageCount( f, ( uint32_t ) ( ( bf -> eof + PGSIZE - 1 ) >> PGBITS ) );
                            if ( rc != 0 )
                                return rc;
                        }
                        * pf = f;
                        return 0;
                    }
                    free ( f );
                }
                free ( bf );
            }
        }
        * pf = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KPageFileMakeUpdate ( KPageFile **pf, KFile * backing, size_t climit, bool write_through )
{
    rc_t rc = 0;

    if ( pf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( backing != NULL )
        {
            if ( ! backing -> read_enabled )
            {
                if ( backing -> write_enabled )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcWriteonly );
                else
                    rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
            }
            else if ( ! backing -> write_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
        }

        if ( rc == 0 )
        {
            KPageFile *f = malloc ( sizeof * f );
            if ( f == NULL )
            {
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            }
            else 
            {
                f->page_idx = NULL;
                f->page_idx_depth = 0;
                DLListInit ( & f -> by_access );
                KRefcountInit ( & f -> refcount, 1, "KPageFile", "make", "pgfile" );
                f -> count = 0;
                f -> ccount = 0;
                f -> climit = ( uint32_t ) ( climit >>  PGBITS );
                if ( f -> climit < MIN_CACHE_PAGE )
                    f -> climit = MIN_CACHE_PAGE;

                PAGE_DEBUG( ( "PAGE: KPageFileMakeUpdate {%p} limit = %u\n", f, f->climit ) );

                f -> read_only = false;
                f -> backing = NULL; /** for now **/
            }

            if ( rc == 0 && backing )
            {
                KPageBacking *bf = calloc ( 1, sizeof * bf );
                if ( bf == NULL )
                {
                    rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                }
                else
                {
                    rc = KFileAddRef ( backing );
                    if ( rc == 0 )
                    {
                        /* finish the backing file */
                        KRefcountInit ( & bf -> refcount, 1, "KPageBacking", "make", "backing" );
                        bf -> write_through = write_through;
                        /* attached reference */
                        bf -> backing = backing;

                        /* see if file is random access */
                        rc = KFileSize ( backing, & bf -> eof );
                        if ( rc == 0 )
                        {
                            bf -> have_eof = true;
                        }
                    }
                }

                if ( rc == 0 )
                {
                    f -> backing = bf;
                    rc = KPageFileSetPageCount( f, ( uint32_t ) ( ( bf -> eof + PGSIZE - 1 ) >> PGBITS ) );
                }
                else if ( bf != NULL )
                {
                    free ( bf );
                }
            }

            if ( rc == 0 )
            {
                * pf = f;
            }
            else if ( f != NULL )
            {
                free( f );
            }
        }

        if ( rc != 0 )
        {
            *pf = NULL;
        }
    }
    return rc;
}


/* AddRef
 * Release
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KPageFileAddRef ( const KPageFile *self )
{
    if ( self != NULL ) switch ( KRefcountAdd ( & self -> refcount, "KPageFile" ) )
    {
    case krefOkay:
        break;
    default:
        return RC ( rcFS, rcFile, rcAttaching, rcConstraint, rcViolated );
    }

    return 0;
}

LIB_EXPORT rc_t CC KPageFileRelease ( const KPageFile *self )
{
    if ( self != NULL ) switch ( KRefcountDrop ( & self -> refcount, "KPageFile" ) )
    {
    case krefOkay:
        break;
    case krefWhack:
        return KPageFileWhack ( ( KPageFile* ) self );
    default:
        return RC ( rcFS, rcFile, rcReleasing, rcConstraint, rcViolated );
    }

    return 0;
}


/* Size
 *  returns size in bytes of file and cache
 *
 *  "lsize" [ OUT, NULL OKAY ] - return parameter for logical size
 *
 *  "fsize" [ OUT, NULL OKAY ] - return parameter for file size
 *
 *  "csize" [ OUT, NULL OKAY ] - return parameter for cache size
 */
LIB_EXPORT rc_t CC KPageFileSize ( const KPageFile *self,
    uint64_t *lsize, uint64_t *fsize, size_t *csize )
{
    rc_t rc = 0;

    size_t dummysz;
    uint64_t dummy64;

    if ( lsize == NULL )
        lsize = & dummy64;
    if ( fsize == NULL )
        fsize = & dummy64;
    if ( csize == NULL )
        csize = & dummysz;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcAccessing, rcSelf, rcNull );
    else
    {
        if ( self -> backing == NULL || self -> backing -> backing == NULL )
            * fsize = 0;
        else if ( ! self -> backing -> have_eof )
            * fsize = self -> backing -> eof;
        else
            rc = KFileSize ( self -> backing -> backing, fsize );

        if ( rc == 0 )
        {
            * lsize = ( uint64_t ) self -> count << PGBITS;
            * csize = ( uint64_t ) self -> ccount << PGBITS;
            return 0;
        }
    }

    * lsize = 0;
    * fsize = 0;
    * csize = 0;

    return rc;
}

/* SetSize
 *  extends or truncates underlying file
 *  may affect cache contents
 *
 *  "size" [ IN ] - logical size
 */
LIB_EXPORT rc_t CC KPageFileSetSize ( KPageFile *self, uint64_t size )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcFile, rcUpdating, rcSelf, rcNull );
    else
    {
        uint32_t new_count = ( uint32_t ) ( ( size + PGSIZE - 1 ) >> PGBITS );
    rc=KPageFileSetPageCount(self,new_count);
    if(rc) return rc;
    assert(self->count == new_count);
    }
    return rc;
}


static rc_t KPageFileIndexInsert( KPageFile * self, KPage * page )
{
    void        **tmp;
    uint8_t     depth;
    uint32_t    page_id = page->page_id;
    uint8_t     offset;

    assert( page->page_id > 0 );

    if ( page->page_id > self->count ) /**** This is an autogrowth ***/
    {
        rc_t rc= KPageFileSetPageCount( self, page->page_id );
        if ( rc != 0 )
            return rc;
    }

    if ( self->page_idx == NULL )
    {
        self->page_idx = malloc( 256 * sizeof( *tmp ) );
        if( self->page_idx == NULL )
            return RC( rcFS, rcIndex, rcInserting, rcMemory, rcInsufficient );
        memset( self->page_idx, 0, 256 * sizeof( void * ) );
    }

    tmp = self->page_idx;
    depth = self->page_idx_depth;
    assert( PAGE_IDX_DEPTH( page_id ) <= depth );

    while ( depth > 1 )
    {
        offset = ( ( page_id - 1 ) >> ( depth - 1 ) * 8 ) & 0xff;
        if ( tmp[ offset ] == NULL )
        {
            tmp[ offset ] = malloc( 256 * sizeof( *tmp ) );
            if ( tmp [ offset ] == NULL )
                return RC( rcFS, rcIndex, rcInserting, rcMemory, rcInsufficient );
            memset( tmp[ offset ], 0, 256 * sizeof( void * ) );
        }
        assert( tmp[ offset ] );
        tmp = ( void** )tmp[ offset ];
        depth--;
    }

    assert( depth == 1 );
    offset = ( page_id - 1 ) & 0xff;
    if ( tmp[ offset ] && tmp[ offset ] != page )
        return RC( rcFS, rcIndex, rcInserting,rcId,rcDuplicate );

    tmp[ offset ] = page;
    return 0;
}


static rc_t KPageFileIndexDelete( KPageFile *self, uint32_t page_id )
{
    void ** tmp = self->page_idx;
    uint8_t depth = self->page_idx_depth;
    uint8_t offset;

    assert( page_id > 0 );
    assert( PAGE_IDX_DEPTH( page_id ) <= depth );

    while( depth > 1 )
    {
        offset = ( ( page_id - 1 ) >> ( depth - 1 ) * 8 ) & 0xff;
        if( tmp[ offset ] == 0 )
            return RC( rcFS, rcIndex, rcRemoving,rcId,rcInconsistent );

        assert( tmp[ offset ] );
        tmp = ( void** )tmp[ offset ];
        depth--;
    }

    offset=( page_id - 1 ) & 0xff;

    if ( tmp[ offset ] == 0 )
        return RC( rcFS, rcIndex, rcRemoving,rcId,rcInconsistent );

    tmp[ offset ] = 0;
    return 0;
}


static KPage * KPageFileIndexFind( KPageFile *self, uint32_t page_id )
{
    void ** tmp = self->page_idx;
    uint8_t depth = self->page_idx_depth;
    uint8_t offset;

    assert( page_id > 0 );
    if ( PAGE_IDX_DEPTH( page_id ) > depth )
        return NULL;

    if ( tmp == NULL )
        return NULL;

    while ( depth > 1 )
    {
        offset = ( ( page_id - 1 ) >> ( depth - 1 ) * 8 ) & 0xff;
        if ( tmp[ offset ] == NULL )
            return NULL;

        assert( tmp[ offset ] );
        tmp = ( void** )tmp[ offset ];
        depth--;
    }

    offset = ( page_id - 1 ) & 0xff;
    assert( tmp[ offset ] == NULL || ( ( KPage * )tmp[ offset ] )->page_id == page_id );
    return ( KPage * )tmp[ offset ];
}

    
/* CachePage
 *  insert a page into cache
 */
static rc_t KPageFileCachePage ( KPageFile *self, KPage *page )
{
    rc_t rc;

    /* perform insert */
    rc = KPageFileIndexInsert( self, page );
    if ( rc != 0 )
        return rc;  

    PAGE_DEBUG( ( "PAGE: {%p}.[%s] insert #%u\n", self, KDbgGetColName(), page->page_id ) );

    DLListPushHead ( & self -> by_access, & page -> ln );

    /* attach reference to page */
    KPageAttach ( page );

    /* check limit */
    if ( ++ self -> ccount <= self -> climit )
        return 0;

    do
    {
        DLNode *last = DLListPopTail ( & self -> by_access );
        KPage *doomed = ( KPage* ) last;

        PAGE_DEBUG( ( "PAGE: {%p}.[%s] delete #%u\n", self, KDbgGetColName(), doomed->page_id ) );

        rc = KPageFileIndexDelete( self, doomed->page_id );
        if ( rc == 0 )
            rc = KPageSever ( doomed );
    }
    while ( -- self -> ccount > self -> climit && rc == 0 );

    return rc;
}


/* Alloc
 *  allocates a new page
 *  the page will be zeroed and initially unmodified
 *
 *  "page" [ OUT ] - return parameter for page object
 *
 *  "page_id" [ OUT, NULL OKAY ] - optional return parameter for page id
 */
LIB_EXPORT rc_t CC KPageFileAlloc ( KPageFile *self, KPage **ppage, uint32_t *page_id )
{
    rc_t rc;

    uint32_t dummy;
    if ( page_id == NULL )
        page_id = & dummy;

    if ( ppage == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcFS, rcBlob, rcAllocating, rcFile, rcReadonly );
        else
        {
            /* create new page */
            rc = KPageNew ( ppage, self -> backing, self -> count + 1 );
            if ( rc == 0 )
            {
                /* insert into cache */
                rc = KPageFileCachePage ( self, * ppage );
                if ( rc == 0 )
                {
                    * page_id = self -> count;
                    return 0;
                }

                KPageRelease ( * ppage );
            }
        }

        * ppage = NULL;
    }

    * page_id = 0;

    return rc;
}


/* Get
 *  returns an existing page
 *
 *  "page" [ OUT ] - return parameter for page object
 *
 *  "page_id" [ IN ] - id of page to retrieve
 */
LIB_EXPORT rc_t CC KPageFileGet ( KPageFile *self, KPage **ppage, uint32_t page_id )
{
    rc_t rc;

    if ( ppage == NULL )
        rc = RC ( rcFS, rcFile, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcReading, rcSelf, rcNull );
        else if ( page_id == 0 )
            rc = RC ( rcFS, rcFile, rcReading, rcId, rcNull );
        else
        {
            KPage *page = KPageFileIndexFind( self, page_id ); /** try to see if MRU pages work **/
            if ( page != NULL )
            {
                rc = KPageAddRef ( * ppage = page );
                if ( rc == 0 )
                {
                    PAGE_DEBUG( ( "PAGE: {%p}.[%s] found #%u\n", self, KDbgGetColName(), page_id ) );

                    /* put page at head of list if not already there */
                    if ( DLListHead ( & self -> by_access ) != & page -> ln )
                    {
                        DLListUnlink ( & self -> by_access, & page -> ln );
                        DLListPushHead ( & self -> by_access, & page -> ln );
                    }
                    return 0;
                }
                * ppage = NULL;
                return rc;
            }

            rc = KPageMake ( ppage, self -> backing, page_id );
            if ( rc == 0 )
            {
                /* insert into cache */
                rc = KPageFileCachePage ( self, * ppage );
                if ( rc == 0 )
                    return 0;

                KPageRelease ( * ppage );
            }
        }

        * ppage = NULL;
    }

    return rc;
}


/* PosGet
 *  returns a page corresponding to position
 *
 *  "page" [ OUT ] - return parameter for page object
 *
 *  "offset" [ IN ] - offset to a byte within file
 */
LIB_EXPORT rc_t CC KPageFilePosGet ( KPageFile *self, KPage **page, uint64_t offset )
{
    rc_t rc;

    if ( page == NULL )
        rc = RC ( rcFS, rcFile, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcReading, rcSelf, rcNull );
        else
        {
            uint32_t aid, count = self -> count;
            uint32_t zpgid = ( uint32_t ) ( offset >> PGBITS );

            if ( self -> read_only || zpgid < count )
                return KPageFileGet ( self, page, zpgid + 1 );

            /* potentially extend count so that
               the oid assigned will be zpgid + 1 */
            self -> count = zpgid;
            rc = KPageFileAlloc ( self, page, & aid );
            if ( rc == 0 )
            {
                assert ( zpgid + 1 == aid );
                return 0;
            }

            self -> count = count;
        }

        * page = NULL;
    }

    return rc;
}


/* DropBacking
 *  used immediately prior to releasing
 *  prevents modified pages from being flushed to disk
 */
LIB_EXPORT rc_t CC KPageFileDropBacking ( KPageFile *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcDetaching, rcSelf, rcNull );

    return self -> backing ? KPageBackingDrop ( self -> backing ) : 0;
}
