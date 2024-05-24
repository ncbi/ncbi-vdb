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

#include "rcolumnblob.h"

#include "rcolumn.h"

#include <klib/checksum.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <klib/debug.h>

#include <byteswap.h>

#ifdef _DEBUGGING
#define POS_DEBUG(msg) DBGMSG(DBG_KDB,DBG_FLAG(DBG_KDB_POS),msg)
#else
#define POS_DEBUG(msg)
#endif

/*--------------------------------------------------------------------------
 * KRColumnBlob
 *  one or more rows of column data
 */

static rc_t KRColumnBlobWhack ( KColumnBlob *self );
static rc_t CC KRColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KRColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
static rc_t CC KRColumnBlobValidate ( const KColumnBlob *self );
static rc_t CC KRColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
static rc_t CC KRColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlob_vt KRColumnBlob_vt =
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

#define CAST() assert( bself->vt == &KRColumnBlob_vt ); KRColumnBlob * self = (KRColumnBlob *)bself

/* Whack
 */
static
rc_t
KRColumnBlobWhack ( KColumnBlob *bself )
{
    CAST();

    const KRColumn *col = self -> col;
    if ( col != NULL )
    {
        KRColumnPageMapWhack ( & self -> pmorig, & col -> df );

        /* cannot recover from errors here,
        since the page maps needed whacking first,
        and the column is needed for that. */
        KColumnSever ( col );
    }

    return KColumnBlobBaseWhack ( bself );
}

/* OpenRead
 * OpenUpdate
 */
rc_t KRColumnBlobOpenRead ( KRColumnBlob *self, const KRColumn *col, int64_t id )
{
    /* locate blob */
    rc_t rc = KRColumnIdxLocateBlob ( & col -> idx, & self -> loc, id, id );
    if ( rc == 0 )
    {
        /* open page map to blob */
        rc = KRColumnPageMapOpen ( & self -> pmorig,
            ( KRColumnData* ) & col -> df, self -> loc . pg, self -> loc . u . blob . size );
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
            KRColumnPageMapWhack ( & self -> pmorig, & col -> df );
            rc = RC ( rcDB, rcColumn, rcOpening, rcBlob, rcCorrupt );
        }
    }

    return rc;
}

/* Make
 */
rc_t KRColumnBlobMake ( KRColumnBlob **blobp, bool bswap )
{
    KRColumnBlob *blob = malloc ( sizeof * blob );
    if ( blob == NULL )
        return RC ( rcDB, rcBlob, rcConstructing, rcMemory, rcExhausted );

    memset ( blob, 0, sizeof * blob );
    KColumnBlobBaseInit( & blob -> dad, & KRColumnBlob_vt );

    blob -> bswap = bswap;

    * blobp = blob;
    return 0;
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
KRColumnBlobIdRange ( const KColumnBlob *bself, int64_t *first, uint32_t *count )
{
    CAST();

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
rc_t validateCRC32 ( const KRColumnBlob *self )
{
    rc_t rc;
    const KRColumn *col = self -> col;

    uint8_t buffer [ 8 * 1024 ];
    size_t to_read, num_read, total, size;

    uint32_t cs, crc32 = 0;

    /* calculate checksum */
    for ( size = self -> loc . u . blob . size, total = 0; total < size; total += num_read )
    {
        to_read = size - total;
        if ( to_read > sizeof buffer )
            to_read = sizeof buffer;

        rc = KRColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        crc32 = CRC32 ( crc32, buffer, num_read );
    }

    /* read stored checksum */
    rc = KRColumnDataRead ( & col -> df,
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
rc_t validateMD5 ( const KRColumnBlob *self )
{
    rc_t rc;
    const KRColumn *col = self -> col;

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

        rc = KRColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        MD5StateAppend ( & md5, buffer, num_read );
    }

    /* read stored checksum */
    rc = KRColumnDataRead ( & col -> df,
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
KRColumnBlobValidate ( const KColumnBlob *bself )
{
    CAST();

    //NB: very similar to KWColumnBlobValidate
    if ( self -> loc . u . blob . size == 0 )
        return 0;

    switch ( self -> col -> checksum )
    {
    case kcsCRC32:
        return validateCRC32 ( self );
    case kcsMD5:
        return validateMD5 ( self );
    case kcsNone:
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcChecksum, rcNotFound );
    default:
        return RC ( rcDB, rcBlob, rcValidating, rcType, rcUnexpected );
    }
}

/* ValidateBuffer
 *  run checksum validation on buffer data
 *
 *  "buffer" [ IN ] - returned blob buffer from ReadAll
 *
 *  "cs_data" [ IN ] and "cs_data_size" [ IN ] - returned checksum data from ReadAll
 */
static
rc_t validateBufferCRC32 ( const void * buffer, size_t size, uint32_t cs )
{
    uint32_t const crc32 = CRC32 ( 0, buffer, size );

    return cs == crc32 ? 0 : RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );
}

static
rc_t validateBufferMD5 ( const void * buffer, size_t size, const uint8_t cs [ 16 ] )
{
    uint8_t digest [ 16 ];
    MD5State md5;

    MD5StateInit ( & md5 );
    MD5StateAppend ( & md5, buffer, size );
    MD5StateFinish ( & md5, digest );

    return memcmp ( cs, digest, sizeof digest ) == 0 ? 0 : RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );
}

static
rc_t CC
KRColumnBlobValidateBuffer ( const KColumnBlob * bself,
    const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    CAST();

    size_t bsize;

    if ( buffer == NULL || cs_data == NULL )
        return RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );

    bsize = KDataBufferBytes ( buffer );
    if ( bsize < self -> loc . u . blob . size )
        return RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
    if ( bsize > self -> loc . u . blob . size )
        return RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );
    if ( bsize == 0 )
        return 0;

    switch ( self -> col -> checksum )
    {
    case kcsCRC32:
        return validateBufferCRC32 ( buffer -> base, bsize,
            self -> bswap ? bswap_32 ( cs_data -> crc32 ) : cs_data -> crc32 );
    case kcsMD5:
        return validateBufferMD5 ( buffer -> base, bsize, cs_data -> md5_digest );
    case kcsNone:
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcChecksum, rcNotFound );
    default:
        return RC ( rcDB, rcBlob, rcValidating, rcType, rcUnexpected );
    }
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
KRColumnBlobRead ( const KColumnBlob *bself,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    CAST();

    rc_t rc;
    size_t ignore;
    if ( remaining == NULL )
        remaining = & ignore;

    if ( num_read == NULL )
        rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    else
    {
        size_t size = self -> loc . u . blob . size;
        const KRColumn *col = self -> col;

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

                rc = KRColumnDataRead ( & col -> df, & self -> pmorig, offset + *num_read,
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
        return rc;
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
KRColumnBlobReadAll ( const KColumnBlob * bself, KDataBuffer * buffer,
    KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    CAST();

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
                rc = KColumnBlobRead ( bself, 0, buffer -> base, bsize, & num_read, & remaining );
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
                            rc = KRColumnDataRead ( & self -> col -> df,
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
