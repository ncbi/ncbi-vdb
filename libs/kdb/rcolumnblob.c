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

#define KONST const
#include "column-priv.h"
#include <klib/checksum.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <klib/debug.h>
#undef KONST

#include <byteswap.h>

#define KCOLUMNBLOB_IMPL KColumnBlob
#include "columnblob-base.h"

#ifdef _DEBUGGING
#define POS_DEBUG(msg) DBGMSG(DBG_KDB,DBG_FLAG(DBG_KDB_POS),msg)
#else
#define POS_DEBUG(msg)
#endif

/*--------------------------------------------------------------------------
 * KColumnBlob
 *  one or more rows of column data
 */

static rc_t KRColumnBlobWhack ( KColumnBlob *self );
static rc_t CC KRColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KRColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
static rc_t CC KRColumnBlobValidate ( const KColumnBlob *self );
static rc_t CC KRColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
static rc_t CC KRColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlobBase_vt KRColumnBlob_vt =
{
    /* Public API */
    KRColumnBlobWhack,
    KColumnBlobBaseAddRef,
    KColumnBlobBaseRelease,
    KRColumnBlobRead,
    KRColumnBlobReadAll,
    KRColumnBlobValidate,
    KRColumnBlobValidateBuffer,
    KRColumnBlobIdRange
};

/* Whack
 */
static
rc_t KRColumnBlobWhack ( KColumnBlob *self )
{
    const KColumn *col = self -> col;
    if ( col != NULL )
    {
        KColumnPageMapWhack ( & self -> pmorig, & col -> df );

        /* cannot recover from errors here,
        since the page maps needed whacking first,
        and the column is needed for that. */
        KColumnSever ( col );
    }

    return KColumnBlobBaseWhack ( self );
}

/* OpenRead
 * OpenUpdate
 */
static
rc_t KColumnBlobOpenRead ( KColumnBlob *self, const KColumn *col, int64_t id )
{
    /* locate blob */
    rc_t rc = KColumnIdxLocateBlob ( & col -> idx, & self -> loc, id, id );
    if ( rc == 0 )
    {
        /* open page map to blob */
        rc = KColumnPageMapOpen ( & self -> pmorig,
            ( KColumnData* ) & col -> df, self -> loc . pg, self -> loc . u . blob . size );
        if ( rc == 0 )
        {
            /* existing blob must have proper checksum bytes */
            if ( self -> loc . u . blob .  size >= col -> csbytes )
            {
                /* remove them from apparent blob size */
                self -> loc . u . blob . size -= col -> csbytes;
                self -> col = KColumnAttach ( col );
                return 0;
            }

            /* the blob is corrupt */
            KColumnPageMapWhack ( & self -> pmorig, & col -> df );
            rc = RC ( rcDB, rcColumn, rcOpening, rcBlob, rcCorrupt );
        }
    }

    return rc;
}

/* Make
 */
rc_t KColumnBlobMake ( KColumnBlob **blobp, bool bswap )
{
    KColumnBlob *blob = malloc ( sizeof * blob );
    if ( blob == NULL )
        return RC ( rcDB, rcBlob, rcConstructing, rcMemory, rcExhausted );

    memset ( blob, 0, sizeof * blob );
    KColumnBlobBaseInit( blob, & KRColumnBlob_vt );

    blob -> bswap = bswap;

    * blobp = blob;
    return 0;
}

/* OpenBlobRead
 *  opens an existing blob containing row data for id
 */
LIB_EXPORT rc_t CC KColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id )
{
    rc_t rc;
    KColumnBlob *blob;

    if ( blobp == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * blobp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );


    rc = KColumnBlobMake ( & blob, self -> idx . idx1 . bswap );
    if ( rc == 0 )
    {
        rc = KColumnBlobOpenRead ( blob, self, id );
        if ( rc == 0 )
        {
            * blobp = blob;
            return 0;
        }

        free ( blob );
    }

    return rc;
}

/* IdRange
 *  returns id range for blob
 *
 *  "first" [ OUT ] - return parameter for first id
 *
 *  "last" [ OUT ] - return parameter for count
 */
static
rc_t CC
KRColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count )
{
    rc_t rc;

    if ( first == NULL || count == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    else if ( self -> loc . id_range == 0 )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcRange, rcEmpty );
    else
    {
        * first = self -> loc . start_id;
        * count = self -> loc . id_range;
        return 0;
    }

    if ( first != NULL )
        * first = 0;
    if ( count != NULL )
        * count = 0;

    return rc;
}

/* KColumnBlobValidate
 *  runs checksum validation on unmodified blob
 */
static
rc_t KColumnBlobValidateCRC32 ( const KColumnBlob *self )
{
    rc_t rc;
    const KColumn *col = self -> col;

    uint8_t buffer [ 8 * 1024 ];
    size_t to_read, num_read, total, size;

    uint32_t cs, crc32 = 0;

    /* calculate checksum */
    for ( size = self -> loc . u . blob . size, total = 0; total < size; total += num_read )
    {
        to_read = size - total;
        if ( to_read > sizeof buffer )
            to_read = sizeof buffer;

        rc = KColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        crc32 = CRC32 ( crc32, buffer, num_read );
    }

    /* read stored checksum */
    rc = KColumnDataRead ( & col -> df,
        & self -> pmorig, size, & cs, sizeof cs, & num_read );
    if ( rc != 0 )
        return rc;
    if ( num_read != sizeof cs )
        return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

    if ( self -> bswap )
        cs = bswap_32 ( cs );

    if ( cs != crc32 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

static
rc_t KColumnBlobValidateMD5 ( const KColumnBlob *self )
{
    rc_t rc;
    const KColumn *col = self -> col;

    uint8_t buffer [ 8 * 1024 ];
    size_t to_read, num_read, total, size;

    MD5State md5;
    uint8_t digest [ 16 ];

    MD5StateInit ( & md5 );

    /* calculate checksum */
    for ( size = self -> loc . u . blob . size, total = 0; total < size; total += num_read )
    {
        to_read = size - total;
        if ( to_read > sizeof buffer )
            to_read = sizeof buffer;

        rc = KColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        MD5StateAppend ( & md5, buffer, num_read );
    }

    /* read stored checksum */
    rc = KColumnDataRead ( & col -> df,
        & self -> pmorig, size, buffer, sizeof digest, & num_read );
    if ( rc != 0 )
        return rc;
    if ( num_read != sizeof digest )
        return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

    /* finish MD5 digest */
    MD5StateFinish ( & md5, digest );

    if ( memcmp ( buffer, digest, sizeof digest ) != 0 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

static
rc_t CC
KRColumnBlobValidate ( const KColumnBlob *self )
{
    if ( self -> loc . u . blob . size != 0 ) switch ( self -> col -> checksum )
    {
    case kcsCRC32:
        return KColumnBlobValidateCRC32 ( self );
    case kcsMD5:
        return KColumnBlobValidateMD5 ( self );
    }

    return 0;
}

/* ValidateBuffer
 *  run checksum validation on buffer data
 *
 *  "buffer" [ IN ] - returned blob buffer from ReadAll
 *
 *  "cs_data" [ IN ] and "cs_data_size" [ IN ] - returned checksum data from ReadAll
 */
static
rc_t KColumnBlobValidateBufferCRC32 ( const void * buffer, size_t size, uint32_t cs )
{
    uint32_t crc32 = CRC32 ( 0, buffer, size );

    if ( cs != crc32 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

static
rc_t KColumnBlobValidateBufferMD5 ( const void * buffer, size_t size, const uint8_t cs [ 16 ] )
{
    MD5State md5;
    uint8_t digest [ 16 ];

    MD5StateInit ( & md5 );

    /* calculate checksum */
    MD5StateAppend ( & md5, buffer, size );

    /* finish MD5 digest */
    MD5StateFinish ( & md5, digest );

    if ( memcmp ( cs, digest, sizeof digest ) != 0 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

static
rc_t CC
KRColumnBlobValidateBuffer ( const KColumnBlob * self,
    const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    size_t bsize;

    if ( buffer == NULL || cs_data == NULL )
        return RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );

    bsize = KDataBufferBytes ( buffer );
    if ( bsize < self -> loc . u . blob . size )
        return RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
    if ( bsize > self -> loc . u . blob . size )
        return RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );

    if ( bsize != 0 ) switch ( self -> col -> checksum )
    {
    case kcsNone:
        break;
    case kcsCRC32:
        return KColumnBlobValidateBufferCRC32 ( buffer -> base, bsize,
            self -> bswap ? bswap_32 ( cs_data -> crc32 ) : cs_data -> crc32 );
    case kcsMD5:
        return KColumnBlobValidateBufferMD5 ( buffer -> base, bsize, cs_data -> md5_digest );
    }

    return 0;
}


/* KColumnBlobRead
 *  read data from blob
 *
 *  "offset" [ IN ] - starting offset into blob
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read. specifically,
 *  "offset" + "num_read" + "remaining" == sizeof blob
 */
static
rc_t CC
KRColumnBlobRead ( const KColumnBlob *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    rc_t rc;
    size_t ignore;
    if ( remaining == NULL )
        remaining = & ignore;

    if ( num_read == NULL )
        rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    else
    {
        size_t size = self -> loc . u . blob . size;
        const KColumn *col = self -> col;

        if ( offset > size )
            offset = size;

        if ( bsize == 0 )
            rc = 0;
        else if ( buffer == NULL )
            rc = RC ( rcDB, rcBlob, rcReading, rcBuffer, rcNull );
        else
        {
            size_t to_read = size - offset;
            if ( to_read > bsize )
                to_read = bsize;

            POS_DEBUG(( "KDB: %s,%lu,%lu\n", self->col->path, offset, to_read ));

#ifdef _DEBUGGING
            if ( KDbgTestModConds ( DBG_KFS, DBG_FLAG( DBG_KFS_POS ) ) ||
                    KDbgTestModConds ( DBG_KFS, DBG_FLAG( DBG_KFS_PAGE ) ) )
            {
                KDbgSetColName( self->col->path );
            }
#endif
            *num_read = 0;
            while ( * num_read < to_read )
            {
                size_t nread = 0;

                rc = KColumnDataRead ( & col -> df, & self -> pmorig, offset + *num_read,
                    & ( ( char * ) buffer ) [ * num_read ], to_read - * num_read, & nread );
                if ( rc != 0 )
                    break;
                if (nread == 0)
                {
                    rc = RC ( rcDB, rcBlob, rcReading, rcFile, rcInsufficient );
                    break;
                }

                *num_read += nread;
            }
#ifdef _DEBUGGING
            if ( KDbgTestModConds ( DBG_KFS, DBG_FLAG( DBG_KFS_POS ) ) ||
                    KDbgTestModConds ( DBG_KFS, DBG_FLAG( DBG_KFS_PAGE ) ) )
            {
                KDbgSetColName( NULL );
            }
#endif

            if ( rc == 0 )
            {
                * remaining = size - offset - * num_read;
                return 0;
            }
        }

        * remaining = size - offset;
        * num_read = 0;
    }

    * remaining = 0;
    return rc;
}

/* ReadAll
 *  read entire blob, plus any auxiliary checksum data
 *
 *  "buffer" [ OUT ] - pointer to a KDataBuffer structure that will be initialized
 *  and resized to contain the entire blob. upon success, will contain the number of bytes
 *  in buffer->elem_count and buffer->elem_bits == 8.
 *
 *  "opt_cs_data [ OUT, NULL OKAY ] - optional output parameter for checksum data
 *  associated with the blob in "buffer", if any exist.
 *
 *  "cs_data_size" [ IN ] - sizeof of * opt_cs_data if not NULL, 0 otherwise
 */
static
rc_t CC
KRColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer,
    KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    rc_t rc = 0;

    if ( opt_cs_data != NULL )
        memset ( opt_cs_data, 0, cs_data_size );

    if ( buffer == NULL )
        rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    else
    {
        /* determine blob size */
        size_t bsize = self -> loc . u . blob . size;

        /* ignore blobs of size 0 */
        if ( bsize == 0 )
            rc = 0;
        else
        {
            /* initialize the buffer */
            rc = KDataBufferMakeBytes ( buffer, bsize );
            if ( rc == 0 )
            {
                /* read the blob */
                size_t num_read, remaining;
                rc = KColumnBlobRead ( self, 0, buffer -> base, bsize, & num_read, & remaining );
                if ( rc == 0 )
                {
                    /* test that num_read is everything and we have no remaining */
                    if ( num_read != bsize || remaining != 0 )
                        rc = RC ( rcDB, rcBlob, rcReading, rcTransfer, rcIncomplete );

                    else
                    {
                        /* set for MD5 - just due to switch ordering */
                        size_t cs_bytes = 16;

                        /* if not reading checksum data, then we're done */
                        if ( opt_cs_data == NULL )
                            return 0;

                        /* see what checksumming is in use */
                        switch ( self -> col -> checksum )
                        {
                        case kcsNone:
                            return 0;

                        case kcsCRC32:
                            /* reset for CRC32 */
                            cs_bytes = 4;

                            /* no break */

                        case kcsMD5:
                            if ( cs_data_size < cs_bytes )
                            {
                                rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcTooShort );
                                break;
                            }

                            /* read checksum information */
                            rc = KColumnDataRead ( & self -> col -> df,
                                & self -> pmorig, bsize, opt_cs_data, cs_bytes, & num_read );
                            if ( rc == 0 )
                            {
                                if ( num_read != cs_bytes )
                                    rc = RC ( rcDB, rcBlob, rcReading, rcTransfer, rcIncomplete );
                                else
                                {
                                    /* success - read the blob AND the checksum data */
                                    return 0;
                                }
                            }
                            break;
                        }
                    }
                }

                KDataBufferWhack ( buffer );
            }
        }

        memset ( buffer, 0, sizeof * buffer );
    }

    return rc;
}
