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
#include "wcolumn-priv.h"
#include "wcolidx0-priv.h"
#include "widxblk-priv.h"
#include "werror-priv.h"
#include <kfs/file.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>


/*--------------------------------------------------------------------------
 * KColumnIdx0Node
 *  a b-tree node
 */
typedef struct KColumnIdx0Node KColumnIdx0Node;
struct KColumnIdx0Node
{
    BSTNode n;
    KColBlobLoc loc;
};

/* Match
 */
static
int64_t CC KColumnIdx0NodeMatch ( const void *item, const BSTNode *n )
{
#define a ( ( const KColBlobLoc* ) item )
#define b ( ( const KColumnIdx0Node* ) n )

    if ( a -> start_id < b -> loc . start_id )
        return -1;
    else if ( a -> start_id > b -> loc . start_id )
        return 1;
    else
        return (int64_t) a -> id_range - (int64_t) b -> loc . id_range;

#undef a
#undef b
}

/* Find
 */
static
int64_t CC KColumnIdx0NodeFind ( const void *item, const BSTNode *n )
{
#define a ( * ( const int64_t* ) item )
#define b ( ( const KColumnIdx0Node* ) n )

    if ( a < b -> loc . start_id )
        return -1;
    return a >= ( b -> loc . start_id + b -> loc . id_range );

#undef a
#undef b
}

/* Sort
 */
static
int64_t CC KColumnIdx0NodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KColumnIdx0Node* ) item )
#define b ( ( const KColumnIdx0Node* ) n )

    if ( ( a -> loc . start_id + a -> loc . id_range ) <= b -> loc . start_id )
        return -1;
    return a -> loc . start_id >= ( b -> loc . start_id + b -> loc . id_range );

#undef a
#undef b
}

/* Whack
 */
static
void CC KColumnIdx0NodeWhack ( BSTNode *n, void *ignore )
{
    free ( n );
}

/* Next
 */
#define KColumnIdx0NodeNext( node ) \
    ( const KColumnIdx0Node* ) BSTNodeNext ( & ( node ) -> n )


/*--------------------------------------------------------------------------
 * KColumnIdx0
 *  level 0 index - event journaling
 */

/* Init
 */
static
rc_t KColumnIdx0Inflate ( KColumnIdx0 *self,
     const KColBlobLoc *buffer, uint32_t count )
{
    uint32_t i;
    KColumnIdx0Node *n;

    for ( n = NULL, i = 0; i < count; ++ i )
    {
        KColumnIdx0Node *exist;

        if ( n == NULL )
        {
            n = malloc ( sizeof * n );
            if ( n == NULL )
                return RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
        }

        n -> loc = buffer [ i ];
        if ( BSTreeInsertUnique ( & self -> bst,
             & n -> n, ( BSTNode** ) & exist, KColumnIdx0NodeSort ) )
        {
            assert ( n -> loc . start_id == exist -> loc . start_id );
            assert ( n -> loc . id_range == exist -> loc . id_range );

            assert ( ! n -> loc . u . blob . remove );
            exist -> loc . pg = n -> loc . pg;
            exist -> loc . u . blob . size = n -> loc . u . blob . size;
        }
        else
        {
            ++ self -> count;
            n = NULL;
        }
    }

    free ( n );

    return 0;
}

static
void KColumnIdx0Swap ( KColBlobLoc *buffer, uint32_t count )
{
    uint32_t i;
    for ( i = 0; i < count; ++ i )
    {
        buffer [ i ] . pg = bswap_64 ( buffer [ i ] . pg );
        buffer [ i ] . u . gen = bswap_32 ( buffer [ i ] . u . gen );
        buffer [ i ] . id_range = bswap_32 ( buffer [ i ] . id_range );
        buffer [ i ] . start_id = bswap_64 ( buffer [ i ] . start_id );
    }
}

static
rc_t KColumnIdx0Init_v1 ( KColumnIdx0 *self, bool bswap )
{
    rc_t rc;
    KColBlobLoc *buffer = malloc ( 2048 * sizeof * buffer );
    if ( buffer == NULL )
        rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
    else
    {
        size_t num_read;

        BSTreeInit ( & self -> bst );
        self -> count = 0;

        for ( self -> eof = 0;; self -> eof += num_read )
        {
            uint32_t count;

            rc = KFileReadAll ( self -> f, self -> eof,
                buffer, 2048 * sizeof * buffer, & num_read );
            if ( rc != 0 )
                break;
            if ( num_read == 0 )
                break;
            if ( ( num_read % sizeof * buffer ) != 0 )
            {
                rc = RC ( rcDB, rcIndex, rcConstructing, rcData, rcCorrupt );
                break;
            }

            count = num_read / sizeof * buffer;

            if ( bswap )
                KColumnIdx0Swap ( buffer, count );

            rc = KColumnIdx0Inflate ( self, buffer, count );
            if ( rc != 0 )
                break;
        }

        free ( buffer );
    }
    return rc;
}

static
rc_t KColumnIdx0Init ( KColumnIdx0 *self, uint32_t total, bool bswap )
{
    rc_t rc;
    KColBlobLoc *buffer = malloc ( 2048 * sizeof * buffer );
    if ( buffer == NULL )
        rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
    else
    {
        size_t num_read;
        uint32_t i, count;

        BSTreeInit ( & self -> bst );
        self -> count = 0;
        self -> eof = 0;

        for ( rc = 0, i = 0; i < total; self -> eof += num_read, i += count )
        {
            count = total - i;
            if ( count > 2048 )
                count = 2048;

            rc = KFileReadAll ( self -> f, i * sizeof * buffer,
                buffer, count * sizeof * buffer, & num_read );
            if ( rc != 0 )
                break;

            /* detect EOF */
            if ( num_read == 0 )
            {
                rc = RC ( rcDB, rcIndex, rcConstructing, rcData, rcCorrupt );
                break;
            }

            /* detect short read -
               see comment in idx1. */
            if ( ( num_read % sizeof * buffer ) != 0 )
            {
                rc = RC ( rcDB, rcIndex, rcConstructing, rcTransfer, rcIncomplete );
                break;
            }

            count = num_read / sizeof * buffer;

            if ( bswap )
                KColumnIdx0Swap ( buffer, count );

            rc = KColumnIdx0Inflate ( self, buffer, count );
            if ( rc != 0 )
                break;
        }

        free ( buffer );
    }
    return rc;
}

/* Create
 */
rc_t KColumnIdx0Create_v1 ( KColumnIdx0 *self, KDirectory *dir,
    struct KMD5SumFmt *md5, KCreateMode mode, bool bswap )
{
    rc_t rc = KColumnFileCreate ( & self -> f, & self -> fmd5,
        dir, md5, mode, true, "idx0" );

    if ( rc == 0 )
        rc = KColumnIdx0Init_v1 ( self, bswap );

    return rc;
}

rc_t KColumnIdx0Create ( KColumnIdx0 *self, KDirectory *dir,
    uint32_t count, struct KMD5SumFmt *md5, KCreateMode mode, bool bswap )
{
    rc_t rc = KColumnFileCreate ( & self -> f, & self -> fmd5,
        dir, md5, mode, true, "idx0" );

    if ( rc == 0 )
        rc = KColumnIdx0Init ( self, count, bswap );

    return rc;
}

/* Open
 */
rc_t KColumnIdx0OpenRead_v1 ( KColumnIdx0 *self, const KDirectory *dir, bool bswap )
{
    rc_t rc;
    uint64_t eof;

    BSTreeInit ( & self -> bst );
    self -> count = 0;

    rc = KDirectoryVFileSize ( dir, & eof, "idx0", NULL );
    if ( rc == 0 )
    {
        if ( eof != 0 )
        {
            rc = KDirectoryOpenFileRead ( dir,
                ( const KFile** ) & self -> f, "idx0" );

            if ( rc == 0 )
                rc = KColumnIdx0Init_v1 ( self, bswap );
        }
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    return rc;
}

rc_t KColumnIdx0OpenRead ( KColumnIdx0 *self,
    const KDirectory *dir, uint32_t count, bool bswap )
{
    BSTreeInit ( & self -> bst );
    self -> count = 0;

    if ( count != 0 )
    {
        rc_t rc = KDirectoryOpenFileRead ( dir,
            ( const KFile** ) & self -> f, "idx0" );

        if ( rc == 0 )
            rc = KColumnIdx0Init ( self, count, bswap );

        return rc;
    }

    return 0;
}

/* Whack
 */
void KColumnIdx0Whack ( KColumnIdx0 *self )
{
    KFileRelease ( self -> f );
    self -> f = NULL;
    self -> fmd5 = NULL;
    BSTreeWhack ( & self -> bst, KColumnIdx0NodeWhack, NULL );
    BSTreeInit ( & self -> bst );
}

/* IdRange
 *  returns range of ids contained within
 */
bool KColumnIdx0IdRange ( const KColumnIdx0 *self,
    int64_t *first, int64_t *upper )
{
    const KColumnIdx0Node *a, *z;

    assert ( self != NULL );
    assert ( first != NULL );
    assert ( upper != NULL );

    a = ( const KColumnIdx0Node* ) BSTreeFirst ( & self -> bst );
    if ( a == NULL )
        return false;

    z = ( const KColumnIdx0Node* ) BSTreeLast ( & self -> bst );
    assert ( z != NULL );

    * first = a -> loc . start_id;
    * upper = z -> loc . start_id + z -> loc . id_range;
    assert ( * first < * upper );

    return true;
}


/* FindFirstRowId
 */
typedef struct FindFirstRowIdData FindFirstRowIdData;
struct FindFirstRowIdData
{
    int64_t start;
    const KColumnIdx0Node * next;
};

static
int64_t CC KColumnIdx0NodeFindFirstRowId ( const void * item, const BSTNode * n )
{
    FindFirstRowIdData * pb = ( FindFirstRowIdData * ) item;

#define a ( pb -> start )
#define b ( ( const KColumnIdx0Node * ) n )

    if ( a < b -> loc . start_id )
    {
        if ( pb -> next == NULL )
            pb -> next = b;
        else if ( b -> loc . start_id < pb -> next -> loc . start_id )
            pb -> next = b;
        return -1;
    }

    return a >= ( b -> loc . start_id + b -> loc . id_range );

#undef a
#undef b
}

rc_t KColumnIdx0FindFirstRowId ( const KColumnIdx0 * self,
    int64_t * found, int64_t start )
{
    FindFirstRowIdData pb;
    const KColumnIdx0Node * n;

    assert ( self != NULL );
    assert ( found != NULL );

    pb . start = start;
    pb . next = NULL;

    n = ( const KColumnIdx0Node* )
        BSTreeFind ( & self -> bst, & pb, KColumnIdx0NodeFindFirstRowId );

    if ( n != NULL )
    {
        assert ( start >= n -> loc . start_id && start < n -> loc . start_id + n -> loc . id_range );
        * found = start;
        return 0;
    }

    if ( pb . next != 0 )
    {
        assert ( pb . next -> loc . start_id > start );
        * found = pb . next -> loc . start_id;
        return 0;
    }

    return SILENT_RC ( rcDB, rcColumn, rcSelecting, rcRow, rcNotFound );
}

/* LocateBlob
 *  locate an existing blob
 */
rc_t KColumnIdx0LocateBlob ( const KColumnIdx0 *self,
    KColBlobLoc *loc, int64_t first, int64_t upper )
{
    const KColumnIdx0Node *n;

    assert ( self != NULL );
    assert ( loc != NULL );
    assert ( first < upper );

    n = ( const KColumnIdx0Node* )
        BSTreeFind ( & self -> bst, & first, KColumnIdx0NodeFind );

    if ( n == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcBlob, rcNotFound );

    assert ( first >= n -> loc . start_id );
    assert ( first < ( n -> loc . start_id + n -> loc . id_range ) );

    if ( upper > ( n -> loc . start_id + n -> loc . id_range ) )
        return RC ( rcDB, rcIndex, rcSelecting, rcRange, rcInvalid );

    * loc = n -> loc;
    assert ( ! loc -> u . blob . remove );
    return 0;
}

/* Commit
 *  records an index location for addition or removal
 *  returns any prior value for rollback
 */
rc_t KColumnIdx0Commit ( KColumnIdx0 *self,
    const KColBlobLoc *loc, KColBlobLoc *prior, bool bswap )
{
    rc_t rc = 0;
    size_t num_writ;
    KColumnIdx0Node *n;

    assert ( self != NULL );
    assert ( loc != NULL );
    assert ( prior != NULL );

    /* this assert tests two things:
       1) that loc -> id_range != 0
       2) that loc -> start_id + loc -> id_range does not wrap around */
    assert ( ( loc -> start_id + loc -> id_range ) > loc -> start_id );

    /* if the index already exists, it's being overwritten */
    n = ( KColumnIdx0Node* )
        BSTreeFind ( & self -> bst, loc, KColumnIdx0NodeMatch );
    if ( n != NULL )
    {
        /* just hold onto the prior value */
        * prior = n -> loc;
        assert ( n -> loc . u . blob . size != 0 );
        assert ( ! n -> loc . u . blob . remove );
        assert ( n -> loc . start_id == loc -> start_id );
        assert ( n -> loc . id_range == loc -> id_range );
    }
    else
    {
        /* no prior value */
        memset ( prior, 0, sizeof * prior );

        /* record an insert/overwrite */
        if ( ! loc -> u . blob . remove )
        {
            n = malloc ( sizeof * n );
            if ( n == NULL )
                return RC ( rcDB, rcBlob, rcCommitting, rcMemory, rcExhausted );

            n -> loc . u . blob . remove = 0;
            n -> loc . start_id = loc -> start_id;
            n -> loc . id_range = loc -> id_range;
        }
    }

    /* journal index to idx0 */
    if ( ! bswap )
        rc = KFileWrite ( self -> f, self -> eof, loc, sizeof * loc, & num_writ );
    else
    {
        KColBlobLoc loc_copy;
        loc_copy . pg = bswap_64 ( loc -> pg );
        loc_copy . u . gen = bswap_32 ( loc -> u . gen );
        loc_copy . id_range = bswap_32 ( loc -> id_range );
        loc_copy . start_id = bswap_64 ( loc -> start_id );

        rc = KFileWrite ( self -> f, self -> eof,
            & loc_copy, sizeof loc_copy, & num_writ );
    }
    if ( rc == 0 )
    {
        /* detect complete write */
        if ( num_writ == sizeof * loc )
        {
            self -> eof += sizeof * loc;

            /* detect insert/overwrite */
            if ( n != NULL )
            {
                /* if deleting, "n" is prior value */
                if ( loc -> u . blob . remove )
                {
                    /* perform a delete */
                    assert ( self -> count > 0 );
                    BSTreeUnlink ( & self -> bst, & n -> n );
                    -- self -> count;
                    free ( n );
                }
                else
                {
                    /* insert new or overwrite old */
                    n -> loc . pg = loc -> pg;
                    n -> loc . u . blob . size = loc -> u . blob . size;
                    if ( prior -> u . blob . size == 0 )
                    {
                        /* complete insertion */
                        BSTreeInsert ( & self -> bst, & n -> n, KColumnIdx0NodeSort );
                        if ( ++ self -> count >= 64 * 1024 )
                            return kdbReindex;
                    }
                }
            }

            /* done */
            return 0;
        }

        /* got here due to incomplete write to idx0 */
        rc = RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );
    }

    /* free allocated insertion block */
    if ( prior -> u . blob . size == 0 )
        free ( n );

    return rc;
}

rc_t KColumnIdx0CommitDone ( KColumnIdx0 *self )
{
    assert ( self != NULL );

    if ( self -> fmd5 == NULL )
        return 0;

    assert ( self -> f == KMD5FileToKFile ( self -> fmd5 ) );
    return KMD5FileCommit ( self -> fmd5 );
}

/* Revert
 *  reverses effect of commit
 */
void KColumnIdx0Revert ( KColumnIdx0 *self,
    const KColBlobLoc *loc, const KColBlobLoc *prior )
{
    rc_t rc = 0;
    KColumnIdx0Node *n;

    assert ( self != NULL );
    assert ( loc != NULL );
    assert ( prior != NULL );

    /* exit MD5 transaction */
    if ( self -> fmd5 != NULL )
        KMD5FileRevert ( self -> fmd5 );

    /* remove the blip from idx0 */
    assert ( self -> eof >= sizeof * loc );
    rc = KFileSetSize ( self -> f, self -> eof - sizeof * loc );
    if ( rc != 0 )
        exit ( 99 );
    self -> eof -= sizeof * loc;

    /* next, repair the binary search tree */
    if ( prior -> u . blob . size != 0 )
    {
        if ( ! loc -> u . blob . remove )
        {
            /* revert an update */
            n = ( KColumnIdx0Node* )
                BSTreeFind ( & self -> bst, prior, KColumnIdx0NodeMatch );
            if ( n == NULL )
                rc = RC ( rcDB, rcIndex, rcReverting, rcNoObj, rcUnknown );
        }
        else
        {
            /* revert a delete */
            n = malloc ( sizeof * n );
            if ( n == NULL )
                rc = RC ( rcDB, rcIndex, rcReverting, rcMemory, rcExhausted );
            else
            {
                n -> loc . u . blob . remove = 0;
                n -> loc . start_id = prior -> start_id;
                n -> loc . id_range = prior -> id_range;
                BSTreeInsert ( & self -> bst, & n -> n, KColumnIdx0NodeSort );
                ++ self -> count;
            }
        }

        if ( n == NULL )
            exit ( 98 );

        /* put back location information */
        n -> loc . pg = prior -> pg;
        n -> loc . u . blob . size = prior -> u . blob . size;
    }
    else if ( ! loc -> u . blob . remove )
    {
        /* revert an insert */
        n = ( KColumnIdx0Node* )
            BSTreeFind ( & self -> bst, loc, KColumnIdx0NodeMatch );
        if ( n == NULL )
            exit ( 97 );

        assert ( self -> count > 0 );
        BSTreeUnlink ( & self -> bst, & n -> n );
        -- self -> count;
        free ( n );
    }
}

/* DefineBlocks
 *  scans existing blob locators
 *  invokes handler with ranges of ids from similar blob entries
 */
rc_t KColumnIdx0DefineBlocks ( const KColumnIdx0 *self,
    rc_t ( * define_block ) ( struct KColBlockLocInfo const *info, void *data ),
    void *data, size_t pgsize )
{
    rc_t rc;
    const KColumnIdx0Node *start;

    assert ( self != NULL );
    assert ( pgsize > 0 );
    assert ( define_block != NULL );

    start = ( const KColumnIdx0Node* ) BSTreeFirst ( & self -> bst );
    for ( rc = 0; start != NULL; )
    {
        KColBlockLocInfo info;

        uint32_t count;
        uint32_t id_type, pg_type;
        const KColumnIdx0Node *next, *end;

        id_type = pg_type = btypePredictable;

        /* build a block from start to prior */
        for ( next = KColumnIdx0NodeNext ( end = start ), count = 1;
              next != NULL; next = KColumnIdx0NodeNext ( end = next ), ++ count )
        {
            size_t pgbytes;
            int64_t end_id;
            int id_next, pg_next;

            size_t hdr_size_div4;
            size_t entry_size_div4;

            /* look at transition from end to next */
            id_next = pg_next = btypePredictable;

            if ( end -> loc . id_range != next -> loc . id_range )
                id_next = btypeMagnitude;
            if ( end -> loc . u . blob . size != next -> loc . u . blob . size )
                pg_next = btypeMagnitude;

            pgbytes = ( ( end -> loc . u . blob . size + pgsize - 1 ) / pgsize ) * pgsize;
            if ( end -> loc . start_id + end -> loc . id_range != next -> loc . start_id )
                id_next -= btypeMagnitude;
            if ( end -> loc . pg + pgbytes != next -> loc . pg )
                pg_next -= btypeMagnitude;

            /* check for combine-ability */
            if ( count != 1 && ( id_type != id_next || pg_type != pg_next ) )
                break;

            /* can be stopped by excessive id range */
            end_id = next -> loc . start_id + next -> loc . id_range;
            if ( ( ( end_id - start -> loc . start_id ) >> 32 ) != 0 )
                break;

            /* only produces change when count == 1
               in which case we're confident that
               block size will not be an issue */
            id_type &= id_next;
            pg_type &= pg_next;

            /* calculate block header size based upon types:
               id = { 0:0, 1:4, 2:8, 3:0  }
               pg = { 0:0, 1:4, 2:8, 3:12 } */
            hdr_size_div4 = pg_type;
            if ( id_type != btypePredictable )
                hdr_size_div4 += id_type;

            /* calculate block entry size as inverse of types:
               { 0:12, 1:8, 2:4, 3:0 } */
            entry_size_div4 = ( id_type ^ 3 ) + ( pg_type ^ 3 );

            /* block size ( div 4 ) cannot exceed 64 ( div 4 ) */
            if ( ( hdr_size_div4 + entry_size_div4 * ( count + 1 ) ) > ( 0x10000 / 4 ) )
                break;
        }

        /* we're at the end of a block */
        info . start_id = start -> loc . start_id;
        info . start_pg = start -> loc . pg;
        info . end_id = end -> loc . start_id + end -> loc . id_range;
        info . end_pg = end -> loc . pg + end -> loc . u . blob . size;
        info . size = end -> loc . u . blob . size;
        info . count = count;
        info . id_type = ( uint16_t ) id_type;
        info . pg_type = ( uint16_t ) pg_type;

        /* keep page end on even page boundary */
        if ( pgsize != 1 )
            info . end_pg = ( ( info . end_pg + pgsize - 1 ) / pgsize ) * pgsize;

        /* define a block */
        rc = ( * define_block ) ( & info, data );
        if ( rc != 0 )
            break;

        /* the next block becomes our start */
        start = next;
    }
    
    return rc;
}

/* TranscribeBlocks
 *  writes all blocks within a range
 */
void KColumnIdx0TranscribeBlocks ( const KColumnIdx0 *self,
    int64_t first, int64_t upper, struct KColWIdxBlock *iblk )
{
    const KColumnIdx0Node *n;

    assert ( self != NULL );
    assert ( first < upper );
    assert ( iblk != NULL );

    n = ( const KColumnIdx0Node* )
        BSTreeFind ( & self -> bst, & first, KColumnIdx0NodeFind );
    assert ( n != NULL );
    assert ( n -> loc . start_id == first );

    while ( n != NULL && n -> loc . start_id < upper )
    {
        assert ( ( n -> loc . start_id + n -> loc . id_range ) <= upper );

        KColWIdxBlockSetEntry ( iblk,
            n -> loc . start_id, n -> loc . id_range,
            n -> loc . pg, n -> loc . u . blob . size );

        n = KColumnIdx0NodeNext ( n );
    }
}

/* Truncate
 *  whacks bst contents and truncates file
 */
void KColumnIdx0Truncate ( KColumnIdx0 *self )
{
    assert ( self != NULL );
    KFileSetSize ( self -> f, self -> eof = 0 );
    BSTreeWhack ( & self -> bst, KColumnIdx0NodeWhack, NULL );
    self -> count = 0;
}
