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
#include "colidx0-priv.h"
#include <kfs/file.h>
#include <klib/rc.h>
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

/* Find
 */
static
int CC KColumnIdx0NodeFind ( const void *item, const BSTNode *n )
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
int CC KColumnIdx0NodeSort ( const BSTNode *item, const BSTNode *n )
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
                return RC ( rcDB, rcColumn, rcConstructing, rcMemory, rcExhausted );
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
rc_t KColumnIdx0Init_v1 ( KColumnIdx0 *self, const KFile *f, bool bswap )
{
    rc_t rc;
    KColBlobLoc *buffer = malloc ( 2048 * sizeof * buffer );
    if ( buffer == NULL )
        rc = RC ( rcDB, rcColumn, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint64_t pos;
        size_t num_read;

        for ( pos = 0;; pos += num_read )
        {
            uint32_t count;

            rc = KFileReadAll ( f, pos,
                buffer, 2048 * sizeof * buffer, & num_read );
            if ( rc != 0 )
                break;
            if ( num_read == 0 )
                break;
            if ( ( num_read % sizeof * buffer ) != 0 )
            {
                rc = RC ( rcDB, rcColumn, rcConstructing, rcIndex, rcCorrupt );
                break;
            }

            count = (uint32_t)( num_read / sizeof * buffer );

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
rc_t KColumnIdx0Init ( KColumnIdx0 *self, const KFile *f, uint32_t total, bool bswap )
{
    rc_t rc;
    KColBlobLoc *buffer = malloc ( 2048 * sizeof * buffer );
    if ( buffer == NULL )
        rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
    else
    {
        uint32_t i, count;

        for ( rc = 0, i = 0; i < total; i += count )
        {
            size_t num_read;

            count = total - i;
            if ( count > 2048 )
                count = 2048;

            rc = KFileReadAll ( f, i * sizeof * buffer,
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

            count = (uint32_t)( num_read / sizeof * buffer );

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
            const KFile *f;
            rc = KDirectoryVOpenFileRead ( dir, & f, "idx0", NULL );
            if ( rc == 0 )
            {
                rc = KColumnIdx0Init_v1 ( self, f, bswap );
                KFileRelease ( f );
            }
        }
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    return rc;
}

rc_t KColumnIdx0OpenRead ( KColumnIdx0 *self, const KDirectory *dir, uint32_t count, bool bswap )
{
    BSTreeInit ( & self -> bst );
    self -> count = 0;

    if ( count != 0 )
    {
        const KFile *f;
        rc_t rc = KDirectoryVOpenFileRead ( dir, & f, "idx0", NULL );
        if ( rc == 0 )
        {
            rc = KColumnIdx0Init ( self, f, count, bswap );
            KFileRelease ( f );
        }
        return rc;
    }

    return 0;
}

/* Whack
 */
void KColumnIdx0Whack ( KColumnIdx0 *self )
{
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
        return RC ( rcDB, rcColumn, rcSelecting, rcBlob, rcNotFound );

    assert ( first >= n -> loc . start_id );
    assert ( first < ( n -> loc . start_id + n -> loc . id_range ) );

    if ( upper > ( n -> loc . start_id + n -> loc . id_range ) )
        return RC ( rcDB, rcColumn, rcSelecting, rcRange, rcInvalid );

    * loc = n -> loc;
    assert ( ! loc -> u . blob . remove );
    return 0;
}
