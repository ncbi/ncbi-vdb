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
#include "wcolidx1-priv.h"
#include "werror-priv.h"
#include <kfs/file.h>
#include <kfs/md5.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>


/*--------------------------------------------------------------------------
 * KColumnIdx1Node
 *  a binary search tree node
 */
typedef struct KColumnIdx1Node KColumnIdx1Node;
struct KColumnIdx1Node
{
    BSTNode n;
    KColBlockLoc loc;
};

/* KColumnIdx1NodeFind
 */
static
int64_t CC KColumnIdx1NodeFind ( const void *item, const BSTNode *n )
{
#define a ( * ( const int64_t* ) item )
#define b ( ( const KColumnIdx1Node* ) n )

    if ( a < b -> loc . start_id )
        return -1;
    return a >= ( b -> loc . start_id + b -> loc . id_range );

#undef a
#undef b
}

/* KColumnIdx1NodeSort
 */
static
int64_t CC KColumnIdx1NodeSort ( const BSTNode *item, const BSTNode *n )
{
#define a ( ( const KColumnIdx1Node* ) item )
#define b ( ( const KColumnIdx1Node* ) n )

    if ( ( a -> loc . start_id + a -> loc . id_range ) <= b -> loc . start_id )
        return -1;
    return a -> loc . start_id >= ( b -> loc . start_id + b -> loc . id_range );

#undef a
#undef b
}

/* KColumnIdx1NodeWhack
 */
static
void CC KColumnIdx1NodeWhack ( BSTNode *n, void *ignore )
{
    free ( n );
}

/*--------------------------------------------------------------------------
 * KColumnIdx1
 *  level 1 index
 */

/* Init
 */
static
rc_t KColumnIdx1Inflate ( KColumnIdx1 *self,
    const KColBlockLoc *buffer, uint32_t count )
{
    uint32_t i;
    for ( i = 0; i < count; ++ i )
    {
        KColumnIdx1Node *exist, *n = malloc ( sizeof * n );
        if ( n == NULL )
            return RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );

        n -> loc = buffer [ i ];
        if ( BSTreeInsertUnique ( & self -> bst,
             & n -> n, ( BSTNode** ) & exist, KColumnIdx1NodeSort ) )
        {
            free ( n );
            return RC ( rcDB, rcIndex, rcConstructing, rcData, rcCorrupt );
        }

        ++ self -> count;
    }

    return 0;
}

static
void KColumnIdx1Swap ( KColBlockLoc *buffer, uint32_t count )
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
rc_t KColumnIdx1Init ( KColumnIdx1 *self, uint32_t off, uint32_t count )
{
    rc_t rc;
    KColBlockLoc *buffer = malloc ( 2048 * sizeof * buffer );
    if ( buffer == NULL )
        rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
    else
    {
        size_t num_read;
        uint32_t i, cnt;
        for ( rc = 0, i = 0; i < count; off += num_read, i += cnt )
        {
            /* maximum number of blocks available
               that will fit into buffer */
            cnt = count - i;
            if ( cnt > 2048 )
                cnt = 2048;

            /* read into buffer an even number of blocks */
            rc = KFileReadAll ( self -> f, off,
                buffer, cnt * sizeof * buffer, & num_read );
            if ( rc != 0 )
                break;

            /* detect EOF */
            if ( num_read == 0 )
                break;

            /* detect corrupt short read
               it's not clear how this is intended to work...
               we either get ALL count blocks, or the file
               is corrupt. a short read is possible, but
               is not an error until we hit EOF... */
            if ( ( num_read % sizeof * buffer ) != 0 )
            {
                rc = RC ( rcDB, rcIndex, rcConstructing, rcData, rcCorrupt );
                break;
            }

            /* byte-swap entire buffer */
            if ( self -> bswap )
                KColumnIdx1Swap ( buffer, cnt );

            /* create master index blocks */
            rc = KColumnIdx1Inflate ( self, buffer, cnt );
            if ( rc != 0 )
                break;
        }

        free ( buffer );
    }
    return rc;
}

/* Create
 */
rc_t KColumnIdx1Create ( KColumnIdx1 *self, KDirectory *dir,
    struct KMD5SumFmt *md5, KCreateMode mode,
	uint64_t *data_eof, uint32_t *idx0_count, uint64_t *idx2_eof,
	size_t pgsize, int32_t checksum )
{
    rc_t rc;

    if ( ( mode & kcmValueMask ) == kcmOpen )
    {
        size_t f_pgsize;
        int32_t f_checksum;
	
        /* try to open update first */
        rc = KColumnIdx1OpenUpdate ( self, dir, md5, data_eof, 
            idx0_count, idx2_eof, &f_pgsize, &f_checksum);

        if ( rc == 0 )
        {
            if ( pgsize != f_pgsize || checksum != f_checksum )
            {
                KFileRelease ( self -> f );
                KFileRelease ( self -> fidx );
                rc = RC ( rcDB, rcIndex, rcConstructing, rcParam, rcIncorrect );
            }
            return rc;
        }
        if ( GetRCState ( rc ) != rcNotFound )
            return rc;

        /* if it failed on not found try to create */
    }

    BSTreeInit ( & self -> bst );
    self -> count = 0;
    self -> vers = 0;
    self -> bswap = false;

    rc = KColumnFileCreate ( & self -> f, & self -> fmd5, dir, md5, mode, true, "idx1" );
    if ( rc == 0 )
    {
#if KCOL_CURRENT_VERSION != 1
        rc = KColumnFileCreate ( & self -> fidx, & self -> fidxmd5, dir, md5, mode, false, "idx" );
        if ( rc == 0 )
        {
#endif
            KColumnHdr hdr;
            size_t num_bytes;

            memset ( & hdr, 0, sizeof hdr );
            hdr . dad . endian = eByteOrderTag;
            hdr . dad . version = KCOL_CURRENT_VERSION;
#if KCOL_CURRENT_VERSION == 1
            hdr . u . v1 . page_size = ( uint32_t ) pgsize;
            hdr . u . v1 . checksum = ( uint8_t ) checksum;
#elif KCOL_CURRENT_VERSION == 2
            hdr . u . v2 . page_size = ( uint32_t ) pgsize;
            hdr . u . v2 . checksum = ( uint8_t ) checksum;
#elif KCOL_CURRENT_VERSION == 3
            hdr . u . v3 . page_size = ( uint32_t ) pgsize;
            hdr . u . v3 . checksum = ( uint8_t ) checksum;
#else
#error "lack code for current version"
#endif
            * data_eof = 0;
            * idx0_count = 0;
            * idx2_eof = 0;
#if KCOL_CURRENT_VERSION == 1
            rc = KFileWrite ( self -> f, 0, & hdr,
              KColumnHdrOffset ( hdr, vCUR ),  & num_bytes );
#else
            rc = KFileWrite ( self -> f, 0, & hdr, sizeof hdr . dad, & num_bytes );
            if ( rc == 0 )
            {
                if ( num_bytes != sizeof hdr . dad )
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcTransfer, rcIncomplete );
                else
                {
                    rc = KFileWrite ( self -> fidx, 0, & hdr,
                        KColumnHdrOffset ( hdr, vCUR ),  & num_bytes );
#endif
                    if ( rc == 0 )
                    {
                        if ( num_bytes == KColumnHdrOffset ( hdr, vCUR ) )
                        {
                            self -> vers = KCOL_CURRENT_VERSION;
                            /* Here is the exit with two new files */
                            return 0;
                        }
                    }
#if KCOL_CURRENT_VERSION != 1
                }

                KFileRelease ( self -> fidx );
            }
#endif

            KFileRelease ( self -> f );
        }
    }

    return rc;
}

/* Open
 */
rc_t KColumnIdx1OpenRead ( KColumnIdx1 *self, const KDirectory *dir,
    uint64_t *data_eof, uint32_t *idx0_count, uint64_t *idx2_eof,
    size_t *pgsize, int32_t *checksum )
{
    rc_t rc;

    BSTreeInit ( & self -> bst );
    self -> count = 0;
    self -> vers = 0;
    self -> bswap = false;

    rc = KDirectoryOpenFileRead ( dir,
        ( const KFile** ) & self -> f, "idx1" );
    if ( rc == 0 )
    {
        KColumnHdr hdr;
        size_t num_bytes;
        rc = KFileReadAll ( self -> f, 0, & hdr, sizeof hdr, & num_bytes );
        if ( rc == 0 )
        {
            if ( num_bytes == 0 )
                rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcEmpty );
            else if ( num_bytes < KColumnHdrMinSize ( hdr ) )
                rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
            else
            {
                rc = KDBHdrValidate ( & hdr . dad,
                    num_bytes, 1, KCOL_CURRENT_VERSION );
                if ( GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
                {
                    self -> bswap = true;
                    hdr . dad . endian = bswap_32 ( hdr . dad . endian );
                    hdr . dad . version = bswap_32 ( hdr . dad . version );
                    rc = KDBHdrValidate ( & hdr . dad, num_bytes, 1, KCOL_CURRENT_VERSION );
                }
                if ( rc == 0 )
                {
                    uint32_t off, count;

                    switch ( hdr . dad . version )
                    {
                    case 1:
                        if ( num_bytes < KColumnHdrOffset ( hdr, v1 ) )
                            rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
                        else
                        {
                            if ( self -> bswap )
                            {
                                hdr . u . v1 . data_eof = bswap_64 ( hdr . u . v1 . data_eof );
                                hdr . u . v1 . idx2_eof = bswap_64 ( hdr . u . v1 . idx2_eof );
                                hdr . u . v1 . num_blocks = bswap_32 ( hdr . u . v1 . num_blocks );
                                hdr . u . v1 . page_size = bswap_32 ( hdr . u . v1 . page_size );
                            }

                            * data_eof = hdr . u . v1 . data_eof;
                            * idx2_eof = hdr . u . v1 . idx2_eof;
                            * pgsize = hdr . u . v1 . page_size;
                            * checksum = hdr . u . v1 . checksum;
                            count = hdr . u . v1 . num_blocks;
                            off = KColumnHdrOffset ( hdr, v1 );

                            * idx0_count = 0;
                        }
                        break;

                    default:
                        rc = KDirectoryOpenFileRead ( dir, ( const KFile** ) & self -> fidx, "idx" );
                        if ( rc == 0 )
                        {
                            off = sizeof hdr . dad;
                            rc = KFileReadAll ( self -> fidx, 0, & hdr, sizeof hdr, & num_bytes );
                            if ( rc == 0 )
                            {
                                if ( num_bytes < KColumnHdrOffset ( hdr, v2 ) )
                                    rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
                                else
                                {
                                    rc = KDBHdrValidate ( & hdr . dad,
                                        num_bytes, 2, KCOL_CURRENT_VERSION );
                                    if ( GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
                                    {
                                        if ( ! self->bswap ) /* catch mis-matched endianess */
                                            rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
                                        else
                                        {
                                            hdr . dad . endian = bswap_32 ( hdr . dad . endian );
                                            hdr . dad . version = bswap_32 ( hdr . dad . version );
                                            rc = KDBHdrValidate ( & hdr . dad, num_bytes, 1, KCOL_CURRENT_VERSION );
                                        }
                                    }
                                    else if ( self -> bswap ) /* catch mis-matched endianess */
                                        rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );

                                    if ( rc == 0 ) switch ( hdr . dad . version )
                                    {
                                    case 2:
                                        if ( self -> bswap )
                                        {
                                            hdr. u . v2 . data_eof = bswap_64 ( hdr. u . v2 . data_eof );
                                            hdr. u . v2 . idx2_eof = bswap_64 ( hdr. u . v2 . idx2_eof );
                                            hdr. u . v2 . num_blocks = bswap_32 ( hdr. u . v2 . num_blocks );
                                            hdr. u . v2 . page_size = bswap_32 ( hdr. u . v2 . page_size );
                                        }

                                        * data_eof = hdr . u . v2 . data_eof;
                                        * idx2_eof = hdr . u . v2 . idx2_eof;
                                        * pgsize = hdr . u . v2 . page_size;
                                        * checksum = hdr . u . v2 . checksum;
                                        count = hdr . u . v2 . num_blocks;
                                        * idx0_count = 0;
                                        break;

                                    case 3:
                                        if ( self -> bswap )
                                        {
                                            hdr. u . v3 . data_eof = bswap_64 ( hdr. u . v3 . data_eof );
                                            hdr. u . v3 . idx2_eof = bswap_64 ( hdr. u . v3 . idx2_eof );
                                            hdr. u . v3 . idx0_count = bswap_32 ( hdr. u . v3 . idx0_count );
                                            hdr. u . v3 . num_blocks = bswap_32 ( hdr. u . v3 . num_blocks );
                                            hdr. u . v3 . page_size = bswap_32 ( hdr. u . v3 . page_size );
                                        }

                                        * data_eof = hdr . u . v3 . data_eof;
                                        * idx2_eof = hdr . u . v3 . idx2_eof;
                                        * idx0_count = hdr . u . v3 . idx0_count;
                                        * pgsize = hdr . u . v3 . page_size;
                                        * checksum = hdr . u . v3 . checksum;
                                        count = hdr . u . v3 . num_blocks;
                                        break;

                                    default:
                                        rc = RC ( rcDB, rcColumn, rcOpening, rcColumn, rcBadVersion );
                                    }
                                }
                            }
                        }
                    }

                    if ( rc == 0 )
                    {
                        self -> vers = hdr . dad . version;
                        rc = KColumnIdx1Init ( self, off, count );
                        if ( rc == 0 )
                            return rc;
                    }

                    KFileRelease ( self -> fidx );
                    self -> fidx = NULL;
                }
            }
        }

        KFileRelease ( self -> f );
        self -> f = NULL;
    }

    return rc;
}

rc_t KColumnIdx1OpenUpdate ( KColumnIdx1 *self, KDirectory *dir,
    struct KMD5SumFmt *md5, uint64_t *data_eof, uint32_t *idx0_count,
    uint64_t *idx2_eof, size_t *pgsize, int32_t *checksum )
{
    rc_t rc;

    BSTreeInit ( & self -> bst );
    self -> count = 0;
    self -> vers = 0;
    self -> bswap = false;

    rc = KColumnFileOpenUpdate ( & self -> f, & self -> fmd5, dir, md5, true, "idx1" );
#if 0
    /* why open a transaction? */
    if ( rc == 0 && md5 != NULL )
        rc = KMD5FileBeginTransaction ( self -> fmd5 );
#endif
    if ( rc == 0 )
    {
        KColumnHdr hdr;
        size_t num_bytes;
        rc = KFileReadAll ( self -> f, 0, & hdr, sizeof hdr, & num_bytes );
        if ( rc == 0 )
        {
            /* if file was there, open will pass but if empty should be
               treated as if it were not there at all */
            if ( num_bytes == 0 )
                rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcNotFound );
            else if ( num_bytes < KColumnHdrMinSize ( hdr ) )
                rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
            else
            {
                /* allow open for update only on compatible architecture and version */
                rc = KDBHdrValidate ( & hdr . dad,
                    num_bytes, 1, KCOL_CURRENT_VERSION );
                if ( rc == 0 )
                {
                    uint32_t off, count;

                    switch ( hdr . dad . version )
                    {
                    case 1:
                        /* ensure hdr is minimally v1 size */
                        if ( num_bytes < KColumnHdrOffset ( hdr, v1 ) )
                            rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
                        else
                        {
#if KCOL_CURRENT_VERSION != 1
                            /* convert to current version */
                            rc = KColumnFileCreate ( & self -> fidx, & self -> fidxmd5,
                                dir, md5, kcmOpen, false, "idx" );
                            if ( rc == 0 )
                            {
#endif
                                * idx0_count = 0;
                                * data_eof = hdr . u . v1 . data_eof;
                                * idx2_eof = hdr . u . v1 . idx2_eof;
                                * pgsize = hdr . u . v1 . page_size;
                                * checksum = hdr . u . v1 . checksum;
                                count = hdr . u . v1 . num_blocks;
                                off = KColumnHdrOffset ( hdr, v1 );
#if KCOL_CURRENT_VERSION == 1
                                self -> convert = CONVERT_ON_SAVE_NONE;
#else
                                self -> convert = CONVERT_ON_SAVE_V1;
                            }
#endif
                        }
                        break;

                    default:
                        /* hdr should be incomplete - instead open "idx" */
                        rc = KColumnFileOpenUpdate ( & self -> fidx, & self -> fidxmd5, dir, md5, false, "idx" );
                        if ( rc == 0 )
                        {
                            self -> convert = CONVERT_ON_SAVE_NONE;
                            off = sizeof hdr . dad; /* where to read in idx1 */
                            rc = KFileReadAll ( self -> fidx, 0, & hdr, sizeof hdr, & num_bytes );
                            if ( rc == 0 )
                            {
                                if ( num_bytes < KColumnHdrOffset ( hdr, v2 ) )
                                    rc = RC ( rcDB, rcColumn, rcOpening, rcIndex, rcCorrupt );
                                else
                                {
                                    rc = KDBHdrValidate ( & hdr . dad,
                                        num_bytes, 2, KCOL_CURRENT_VERSION );
                                    if ( rc == 0 ) switch ( hdr . dad . version )
                                    {
                                    case 2:
                                        * data_eof = hdr . u . v2 . data_eof;
                                        * idx2_eof = hdr . u . v2 . idx2_eof;
                                        * pgsize = hdr . u . v2 . page_size;
                                        * checksum = hdr . u . v2 . checksum;
                                        count = hdr . u . v2 . num_blocks;
                                        * idx0_count = 0;
                                        break; /* skip to initialize and return */

                                    case 3:
                                        * data_eof = hdr . u . v3 . data_eof;
                                        * idx2_eof = hdr . u . v3 . idx2_eof;
                                        * idx0_count = hdr . u . v3 . idx0_count;
                                        * pgsize = hdr . u . v3 . page_size;
                                        * checksum = hdr . u . v3 . checksum;
                                        count = hdr . u . v3 . num_blocks;
                                        break; /* skip to initialize and return */

                                    default:
                                        rc = RC ( rcDB, rcColumn, rcOpening, rcColumn, rcBadVersion );
                                    }
                                }
                            }
                        }
                    }

                    if ( rc == 0 ) /* initialize and return */
                    {
                        self -> vers = hdr . dad . version;
                        rc = KColumnIdx1Init ( self, off, count );
                        if ( rc == 0 )
                            return rc;
                    }

                    KFileRelease ( self -> fidx );
                    self -> fidx = NULL;
                }
            }
        }
        
        KFileRelease ( self -> f );
        self -> f = NULL;
    }

    return rc;
}


/* Whack
 */
rc_t KColumnIdx1Whack ( KColumnIdx1 *self )
{
    rc_t rc = KFileRelease ( self -> fidx );
    if ( rc == 0 )
    {
        self -> fidx = NULL;
        self -> fidxmd5 = NULL;
        rc = KFileRelease ( self -> f );
        if ( rc == 0 )
        {
            self -> f = NULL;
            self -> fmd5 = NULL;
            BSTreeWhack ( & self -> bst, KColumnIdx1NodeWhack, NULL );
            BSTreeInit ( & self -> bst );
        }
    }
    return rc;
}

/* Version
 */
#ifndef KColumnIdx1Version
rc_t KColumnIdx1Version ( const KColumnIdx1 *self, uint32_t *version )
{
    * version = ( uint32_t ) self -> vers;
    return 0;
}
#endif

/* IdRange
 *  returns range of ids contained within
 */
bool KColumnIdx1IdRange ( const KColumnIdx1 *self,
    int64_t *first, int64_t *upper )
{
    const KColumnIdx1Node *a, *z;

    assert ( self != NULL );
    assert ( first != NULL );
    assert ( upper != NULL );

    a = ( const KColumnIdx1Node* ) BSTreeFirst ( & self -> bst );
    if ( a == NULL )
        return false;

    z = ( const KColumnIdx1Node* ) BSTreeLast ( & self -> bst );
    assert ( z != NULL );

    * first = a -> loc . start_id;
    * upper = z -> loc . start_id + z -> loc . id_range;
    assert ( * first < * upper );

    return true;
}

/* LocateFirstRowIdBlob
 */
typedef struct FindFirstRowIdData FindFirstRowIdData;
struct FindFirstRowIdData
{
    int64_t start;
    const KColumnIdx1Node * next;
};

static
int64_t CC KColumnIdx1NodeFindFirstRowId ( const void * item, const BSTNode * n )
{
    FindFirstRowIdData * pb = ( FindFirstRowIdData * ) item;

#define a ( pb -> start )
#define b ( ( const KColumnIdx1Node * ) n )

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

rc_t KColumnIdx1LocateFirstRowIdBlob ( const KColumnIdx1 * self,
    KColBlockLoc * bloc, int64_t start )
{
    FindFirstRowIdData pb;
    const KColumnIdx1Node * n;

    assert ( self != NULL );
    assert ( bloc != NULL );

    pb . start = start;
    pb . next = NULL;

    n = ( const KColumnIdx1Node* )
        BSTreeFind ( & self -> bst, & pb, KColumnIdx1NodeFindFirstRowId );

    if ( n != NULL )
    {
        assert ( start >= n -> loc . start_id && start < n -> loc . start_id + n -> loc . id_range );
        * bloc = n -> loc;
        return 0;
    }

    if ( pb . next != NULL )
    {
        assert ( start < pb . next -> loc . start_id );
        * bloc = pb . next -> loc;
        return 0;
    }
        
    return SILENT_RC ( rcDB, rcColumn, rcSelecting, rcBlob, rcNotFound );
}

/* LocateBlock
 *  locates an idx2 block by range
 *  return values:
 */
rc_t KColumnIdx1LocateBlock ( const KColumnIdx1 *self,
    KColBlockLoc *bloc, int64_t first, int64_t upper )
{
    const KColumnIdx1Node *n;

    assert ( self != NULL );
    assert ( bloc != NULL );
    assert ( first < upper );

    n = ( const KColumnIdx1Node* )
        BSTreeFind ( & self -> bst, & first, KColumnIdx1NodeFind );

    if ( n == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcRange, rcNotFound );

    assert ( first >= n -> loc . start_id );
    assert ( first < ( n -> loc . start_id + n -> loc . id_range ) );

    if ( upper > ( n -> loc . start_id + n -> loc . id_range ) )
        return RC ( rcDB, rcIndex, rcSelecting, rcRange, rcInvalid );

    * bloc = n -> loc;
    return 0;
}

/* WriteHeader
 */
rc_t KColumnIdx1WriteHeader ( KColumnIdx1 *self,
    uint64_t data_eof, uint32_t idx0_count, uint64_t idx2_eof,
    size_t pgsize, int32_t checksum )
{
    rc_t rc = 0;
    KColumnHdr hdr;
    bool write_idx1 = false;
    size_t off, off1, num_writ, num_writ1;

    off1 = sizeof hdr . dad;

    hdr . dad . endian = eByteOrderTag;
    hdr . dad . version = ( uint32_t ) self -> vers;

    switch ( self -> vers )
    {
    case 1:
        hdr . u . v1 . data_eof = data_eof;
        hdr . u . v1 . idx2_eof = idx2_eof;
        hdr . u . v1 . num_blocks = ( uint32_t ) self -> count;
        hdr . u . v1 . page_size = ( uint32_t ) pgsize;
        hdr . u . v1 . checksum = ( uint8_t ) checksum;
        memset ( hdr . u . v1 . align1, 0, sizeof hdr . u . v1 . align1 );
        off = off1 = KColumnHdrOffset ( hdr, v1 );
        write_idx1 = true;
        break;
    case 2:
        hdr . u . v2 . data_eof = data_eof;
        hdr . u . v2 . idx2_eof = idx2_eof;
        hdr . u . v2 . ignore = 0;
        hdr . u . v2 . num_blocks = ( uint32_t ) self -> count;
        hdr . u . v2 . page_size = ( uint32_t ) pgsize;
        hdr . u . v2 . checksum = ( uint8_t ) checksum;
        memset ( hdr . u . v2 . align1, 0, sizeof hdr . u . v2 . align1 );
        off = KColumnHdrOffset ( hdr, v2 );
        break;
    case 3:
        hdr . u . v3 . data_eof = data_eof;
        hdr . u . v3 . idx2_eof = idx2_eof;
        hdr . u . v3 . idx0_count = idx0_count;
        hdr . u . v3 . num_blocks = ( uint32_t ) self -> count;
        hdr . u . v3 . page_size = ( uint32_t ) pgsize;
        hdr . u . v3 . checksum = ( uint8_t ) checksum;
        memset ( hdr . u . v3 . align1, 0, sizeof hdr . u . v2 . align1 );
        off = KColumnHdrOffset ( hdr, v3 );
        break;
    default:
        return RC ( rcDB, rcIndex, rcWriting, rcColumn, rcBadVersion );
    }

#if KCOL_CURRENT_VERSION != 1
    if ( self -> fidxmd5 != NULL )
        rc = KMD5FileReset ( self -> fidxmd5 );
    if ( rc == 0 )
    {
        rc = KFileWrite ( self -> fidx, 0, & hdr, off, & num_writ );
        if ( rc == 0 )
        {
            /* don't have a failsafe recover here - no undoing write to idx figured out */
            if ( write_idx1 )
            {
#endif
                if ( self -> fmd5 != NULL )
                    rc = KMD5FileReset ( self -> fmd5 );
                if ( rc == 0 )
                    rc = KFileWrite ( self -> f, 0, & hdr, off1, & num_writ1 );
#if KCOL_CURRENT_VERSION != 1
            }
        }
    }
#endif
    if ( rc == 0 )
    {
#if KCOL_CURRENT_VERSION != 1
        if ( num_writ != off )
            rc = RC ( rcDB, rcIndex, rcWriting, rcTransfer, rcIncomplete );
        else
#endif
        if ( write_idx1 && num_writ1 != off1 )
            rc = RC ( rcDB, rcIndex, rcWriting, rcTransfer, rcIncomplete );
    }

    return rc;
}

/* Commit
 *  records a block location
 */
rc_t KColumnIdx1Commit ( KColumnIdx1 *self, const KColBlockLoc *bloc )
{
    rc_t rc;
    uint64_t pos;
    size_t num_writ;

    switch ( self -> vers )
    {
    case 1:
        pos = KColumnHdrOffset ( NULL_HDR, v1 );
        break;
    default:
        pos = KColumnHdrMinSize ( NULL_HDR );
    }

    pos += self -> count * sizeof * bloc;
    rc = KFileWrite ( self -> f, pos, bloc, sizeof * bloc, & num_writ );
    if ( rc == 0 )
    {
        if ( num_writ != sizeof * bloc )
            rc = RC ( rcDB, rcIndex, rcCommitting, rcTransfer, rcIncomplete );
        else
        {
            KColumnIdx1Node *n, *exist;
            n = malloc ( sizeof * n );
            if ( n == NULL )
                rc = RC ( rcDB, rcIndex, rcCommitting, rcMemory, rcExhausted );
            else
            {
                n -> loc = * bloc;
                if ( BSTreeInsertUnique ( & self -> bst,
                     & n -> n, ( BSTNode** ) & exist, KColumnIdx1NodeSort ) )
                {
                    free ( n );
                    rc = RC ( rcDB, rcIndex, rcCommitting, rcRange, rcExists );
                }
                else
                {
                    ++ self -> count;
                }
            }
        }
    }

    return rc;
}

rc_t KColumnIdx1CommitDone ( KColumnIdx1 *self )
{
    rc_t rc = 0;
    if ( self -> fmd5 != NULL )
    {
        rc = KMD5FileCommit ( self -> fmd5 );
        if ( rc == 0 )
            rc = KMD5FileCommit ( self -> fidxmd5 );
    }
    return rc;
}

/* Revert
 *  reverses effect of commit
 *  return value is used to trigger DLListDoUntil exit
 */
bool KColumnIdx1Revert ( KColumnIdx1 *self, int64_t start_id, uint32_t id_range )
{
    KColumnIdx1Node *n = ( KColumnIdx1Node* )
        BSTreeFind ( & self -> bst, & start_id, KColumnIdx1NodeFind );
    if ( n == NULL )
        return true;

    assert ( n -> loc . start_id == start_id );
    assert ( n -> loc . id_range == id_range );

    BSTreeUnlink ( & self -> bst, & n -> n );
    free ( n );
    -- self -> count;

    if ( self -> fmd5 != NULL )
    {
        assert ( self -> f == (KFile*) self -> fmd5 );
        assert ( self -> fidx == (KFile*) self -> fidxmd5 );

        KMD5FileRevert ( self -> fmd5 );
    }
    
    return false;
}
