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

#include "wcolumnblob.h"

#include "wcolumn.h"
#include "dbmgr.h"
#include "wtable.h"
#include "wkdb.h"
#include "werror.h"

#include <klib/rc.h>
#include <klib/data-buffer.h>

#include <kfs/impl.h>

#include <stdio.h>
#include <byteswap.h>

#define KCOLUMNBLOB_IMPL KColumnBlob
#include "columnblob-base.h"

/*--------------------------------------------------------------------------
 * KColumnBlob
 *  one or more rows of column data
 */

static rc_t KWColumnBlobWhack ( KColumnBlob *self );
static rc_t CC KWColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KWColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
static rc_t CC KWColumnBlobValidate ( const KColumnBlob *self );
static rc_t CC KWColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
static rc_t CC KWColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlob_vt KWColumnBlob_vt =
{
    /* Public API */
    KWColumnBlobWhack,
    KColumnBlobBaseAddRef,
    KColumnBlobBaseRelease,
    KWColumnBlobRead,
    KWColumnBlobReadAll,
    KWColumnBlobValidate,
    KWColumnBlobValidateBuffer,
    KWColumnBlobIdRange
};

#define CAST() assert( bself->vt == &KWColumnBlob_vt ); KWColumnBlob * self = (KWColumnBlob *)bself

/* Whack
 */
static
rc_t KWColumnBlobWhack ( KColumnBlob *bself )
{
    CAST();

    KWColumn *col = self -> col;
    if ( col != NULL )
    {
        KWColumnPageMapWhack ( & self -> pmorig, & col -> df );
        if ( ! self -> read_only )
            KWColumnPageMapWhack ( & self -> pmnew, & col -> df );

        /* cannot recover from errors here,
        since the page maps needed whacking first,
        and the column is needed for that. */
        KColumnSever ( col );
    }

    free ( self );
    return 0;
}

/* OpenRead
 * OpenUpdate
 */
rc_t KWColumnBlobOpenRead ( KWColumnBlob *self, const KWColumn *col, int64_t id )
{
    /* locate blob */
    rc_t rc = KWColumnIdxLocateBlob ( & col -> idx, & self -> loc, id, id );
    if ( rc == 0 )
    {
        /* indicates not written */
        /*assert ( self -> num_writ == 0 );*/

        /* open page map to blob */
        rc = KWColumnPageMapOpen ( & self -> pmorig,
            ( KWColumnData* ) & col -> df, self -> loc . pg, self -> loc . u . blob . size );
        if ( rc == 0 )
        {
            /* existing blob must have proper checksum bytes */
            if ( self -> loc . u . blob . size >= col -> csbytes )
            {
                /* remove them from apparent blob size */
                self -> loc . u . blob . size -= col -> csbytes;
                return 0;
            }

            /* the blob is corrupt */
            KWColumnPageMapWhack ( & self -> pmorig, & col -> df );
            rc = RC ( rcDB, rcBlob, rcOpening, rcBlob, rcCorrupt );
        }
    }

    return rc;
}

rc_t KWColumnBlobOpenUpdate ( KWColumnBlob *self, KWColumn *col, int64_t id )
{
    /* open existing blob */
    rc_t rc = KWColumnBlobOpenRead ( self, col, id );
    if ( rc == 0 )
    {
        /* create a new page map for replacement */
        rc = KWColumnPageMapCreate ( & self -> pmnew, & col -> df );
        if ( rc == 0 )
        {
            /* initialize for writing checksums */
            switch ( col -> checksum )
            {
            case kcsCRC32:
                self -> crc32 = 0;
                break;
            case kcsMD5:
                MD5StateInit ( & self -> md5 );
                break;
            }
        }

        /* tear down results of opening for read */
        KWColumnPageMapWhack ( & self -> pmorig, & col -> df );
    }

    return rc;
}

/* Create
 */
rc_t KWColumnBlobCreate ( KWColumnBlob *self, KWColumn *col )
{
    rc_t rc;

    if ( col -> md5 != NULL )
    {
	/* this perhaps should be split into wcoldat.c and wcolidx.c */
	rc = KMD5FileBeginTransaction ( col -> df . fmd5 );
	if ( rc == 0 )
	{
	    rc = KMD5FileBeginTransaction ( col -> idx . idx2 . fmd5 );
	    if ( rc == 0 )
	    {
		rc = KMD5FileBeginTransaction ( col -> idx . idx0 . fmd5 );
		if ( rc == 0 )
		{
		    rc = KMD5FileBeginTransaction ( col -> idx . idx1 . fmd5 );
		    if ( rc == 0 )
		    {
			rc = KMD5FileBeginTransaction ( col -> idx . idx1 . fidxmd5 );
			if ( rc != 0 )
			    KMD5FileCommit ( col -> idx . idx1 .  fmd5 );
		    }
		    if ( rc != 0 )
			KMD5FileCommit ( col -> idx . idx0 .  fmd5 );
		}
		if ( rc != 0 )
		    KMD5FileCommit ( col -> idx . idx2 .  fmd5 );
	    }
	    if ( rc != 0 )
		KMD5FileCommit ( col -> df . fmd5 );
	}
	if ( rc != 0 )
	    return rc;
    }

    /* no location yet */
    memset ( & self -> loc, 0, sizeof self -> loc );

    /* invalid existing page map */
    memset ( & self -> pmorig, 0, sizeof self -> pmorig );

    /* create a new page map */
    rc = KWColumnPageMapCreate ( & self -> pmnew, & col -> df );
    if ( rc == 0 )
    {
        /* initialize for writing checksums */
        switch ( col -> checksum )
        {
        case kcsCRC32:
            self -> crc32 = 0;
            break;
        case kcsMD5:
            MD5StateInit ( & self -> md5 );
            break;
        }
    }

    return rc;
}

/* Make
 */
rc_t KWColumnBlobMake ( KWColumnBlob **blobp, bool bswap )
{
    KWColumnBlob *blob = malloc ( sizeof * blob );
    if ( blob == NULL )
        return RC ( rcDB, rcBlob, rcConstructing, rcMemory, rcExhausted );

    memset ( blob, 0, sizeof * blob );
    KColumnBlobBaseInit( & blob -> dad, & KWColumnBlob_vt );

    blob -> bswap = bswap;

    * blobp = blob;
    return 0;
}

/* IdRange
 *  returns id range for blob
 *
 *  "first" [ OUT, NULL OKAY ] - optional return parameter for first id
 *
 *  "last" [ OUT, NULL OKAY ] - optional return parameter for last id
 */
static
rc_t CC
KWColumnBlobIdRange ( const KColumnBlob *bself, int64_t *first, uint32_t *count )
{
    CAST();

    rc_t rc;

    if ( first == NULL || count == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    else if ( self == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcSelf, rcNull );
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
rc_t validateCRC32 ( const KWColumnBlob *self )
{
    rc_t rc;
    const KWColumn *col = self -> col;

    uint8_t buffer [ 8 * 1024 ];
    size_t to_read, num_read, total, size;

    uint32_t cs, crc32 = 0;

    /* calculate checksum */
    for ( size = self -> loc . u . blob. size, total = 0; total < size; total += num_read )
    {
        to_read = size - total;
        if ( to_read > sizeof buffer )
            to_read = sizeof buffer;

        rc = KWColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        crc32 = CRC32 ( crc32, buffer, num_read );
    }

    /* read stored checksum */
    rc = KWColumnDataRead ( & col -> df,
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
rc_t validateMD5 ( const KWColumnBlob *self )
{
    rc_t rc;
    const KWColumn *col = self -> col;

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

        rc = KWColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        MD5StateAppend ( & md5, buffer, num_read );
    }

    /* read stored checksum */
    rc = KWColumnDataRead ( & col -> df,
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
KWColumnBlobValidate ( const KColumnBlob *bself )
{
    CAST();

    if ( self -> num_writ != 0 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcBusy );

    //NB: very similar to KRColumnBlobValidate
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
KWColumnBlobValidateBuffer ( const KColumnBlob * bself,
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
KWColumnBlobRead ( const KColumnBlob *bself,
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
        size_t size = self -> num_writ;
        const KWColumn *col = self -> col;

        if ( size == 0 )
            size = self -> loc . u . blob . size;

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

            *num_read = 0;
            while (*num_read < to_read)
		    {
                size_t nread = 0;

                rc = KWColumnDataRead ( & col -> df, & self -> pmorig, offset + *num_read,
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
KWColumnBlobReadAll ( const KColumnBlob * bself, KDataBuffer * buffer,
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
                            rc = KWColumnDataRead ( & self -> col -> df,
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

/* KColumnBlobAppend
 *  append data to open blob
 *
 *  "buffer" [ IN ] and "size" [ IN ] - blob data
 */
LIB_EXPORT rc_t CC KColumnBlobAppend ( KColumnBlob *bself, const void *buffer, size_t size )
{
    CAST();

    KWColumn *col;
    size_t total, num_writ;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcWriting, rcSelf, rcNull );

    /* data fork will refuse write anyway, but... */
    if ( self -> read_only )
        return RC ( rcDB, rcBlob, rcWriting, rcBlob, rcReadonly );

    col = self -> col;

    for ( total = 0; total < size; total += num_writ )
    {
        rc_t rc = KWColumnDataWrite ( & col -> df,
            & self -> pmnew, self -> num_writ + total,
            ( const char* ) buffer + total, size - total, & num_writ );
        if ( rc != 0 )
            return rc;
        if ( num_writ == 0 )
            return RC ( rcDB, rcBlob, rcWriting, rcTransfer, rcIncomplete );
    }

    self -> num_writ += size;

    switch ( col -> checksum )
    {
    case kcsCRC32:
        self -> crc32 = CRC32 ( self -> crc32, buffer, size );
        break;
    case kcsMD5:
        MD5StateAppend ( & self -> md5, buffer, size );
        break;
    }

    return 0;
}

/* KColumnBlobAssignRange
 *  assign a total id range to blob at any time before commit
 *
 *  "first" [ IN ] and "count" [ IN ] - range parameters for assign
 */
LIB_EXPORT rc_t CC KColumnBlobAssignRange ( KColumnBlob *bself, int64_t first, uint32_t count )
{
    CAST();

    rc_t rc;
    const KWColumn *col;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcUpdating, rcSelf, rcNull );

    if ( count == 0 )
        return RC ( rcDB, rcBlob, rcUpdating, rcRange, rcEmpty );

    if ( self -> read_only )
        return RC ( rcDB, rcBlob, rcUpdating, rcBlob, rcReadonly );

    if ( self -> loc . id_range != 0 )
    {
        /* allow benign reassignment */
        if ( self -> loc . start_id == first &&
             self -> loc . id_range == count )
            return 0;

        /* can't be reset */
        return RC ( rcDB, rcBlob, rcUpdating, rcBlob, rcBusy );
    }

    col = self -> col;

    /* try to open read map */
    rc = KWColumnBlobOpenRead ( self, col, first );
    if ( rc == 0 )
    {
        /* blob already exists
           again, allow benign reassignment */
        if ( self -> loc . start_id == first &&
             self -> loc . id_range == count )
            return 0;

        /* conflicting assignment */
        KWColumnPageMapWhack ( & self -> pmorig, & col -> df );
        memset ( & self -> loc, 0, sizeof self -> loc );
        memset ( & self -> pmorig, 0, sizeof self -> pmorig );
        return RC ( rcDB, rcBlob, rcUpdating, rcRange, rcIncorrect );
    }

    /* expect status of not found */
    if ( GetRCState ( rc ) != rcNotFound )
        return rc;

    /* assign the range */
    self -> loc . pg = 0;
    self -> loc . u . blob . remove = 0;
    self -> loc . start_id = first;
    self -> loc . id_range = count;
    return 0;
}

/* KColumnBlobCommit
 *  commit changes to blob
 *  close to further updates
 */
static
char zero [ 4096 ];

static
rc_t KColumnBlobZeroPad ( KWColumnBlob *self )
{
    KWColumn *col = self -> col;
    size_t pad_bytes = self -> num_writ % col -> df . pgsize;
    if ( pad_bytes != 0 )
    {
        size_t total, num_writ;

        pad_bytes = col -> df . pgsize - pad_bytes;
        for ( total = 0; total < pad_bytes; total += num_writ )
        {
            rc_t rc;

            size_t to_write = pad_bytes - total;
            if ( to_write > sizeof zero )
                to_write = sizeof zero;

            rc = KWColumnDataWrite ( & col -> df,
                & self -> pmnew, self -> num_writ + total,
                zero, to_write, & num_writ );
            if ( rc != 0 )
                return rc;
            if ( num_writ == 0 )
                return RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );
        }
    }
    return 0;
}

static
rc_t KColumnBlobDoCommit ( KWColumnBlob *self )
{
    rc_t rc;
    KColBlobLoc loc;
    KWColumn *col = self -> col;

    /* finish checksum */
    if ( col -> csbytes != 0 )
    {
        MD5State md5;
        uint32_t crc32;
        uint8_t digest [ 16 ];

        void *cs;
        size_t num_writ;

        switch ( col -> checksum )
        {
        case kcsCRC32:
            crc32 = self -> crc32;
            if ( self -> bswap )
                crc32 = bswap_32 ( crc32 );
            cs = & crc32;
            break;
        case kcsMD5:
        default: /* to quiet compiler warnings */
            /* work on copy in case of failure */
            md5 = self -> md5;
            MD5StateFinish ( & md5, digest );
            cs = digest;
            break;
        }

        rc = KWColumnDataWrite ( & col -> df,
            & self -> pmnew, self -> num_writ,
            cs, col -> csbytes, & num_writ );
        if ( rc != 0 )
            return rc;
        if ( num_writ != col -> csbytes )
            return RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );

        self -> num_writ += num_writ;
    }

    /* extract index information */
    rc = KWColumnPageMapId ( & self -> pmnew, & col -> df, & loc . pg );
    if ( rc == 0 )
    {
        loc . u . blob . size = ( uint32_t ) self -> num_writ;
        loc . u . blob . remove = 0;
        loc . start_id = self -> loc . start_id;
        loc . id_range = self -> loc . id_range;

        /* pad out rest of page */
        if ( col -> df . pgsize > 1 )
            rc = KColumnBlobZeroPad ( self );
        if ( rc == 0 )
        {
            /* commit data fork */
            rc = KWColumnDataCommit ( & col -> df,
                & self -> pmnew, self -> num_writ );
            if ( rc == 0 )
            {
                /* commit index fork */
                rc = KWColumnIdxCommit ( & col -> idx, col -> md5,
                    & loc, col -> commit_freq, col -> df . eof,
                    col -> df . pgsize, col -> checksum );
                if ( rc == 0 || rc == kdbReindex )
                {
                    rc_t status = rc;

                    /* release old pages */
                    if ( self -> loc . u . blob . size == 0 )
                        rc = 0;
                    else
                    {
                        rc = KWColumnDataFree ( & col -> df,
                            & self -> pmorig, self -> loc . u . blob . size + col -> csbytes );
                        if ( GetRCState ( rc ) == rcInvalid )
                            rc = 0;
                    }

                    if ( rc == 0 )
                    {
                        /* transfer new map */
                        self -> pmorig = self -> pmnew;
                        memset ( & self -> pmnew, 0, sizeof self -> pmnew );

                        /* fill out location */
                        loc . u . blob . size -= col -> csbytes;
                        self -> loc = loc;

                        /* HACK - should open new pm on demand
                           but since the code does not yet support it,
                           disallow further writes */
                        self -> read_only = true;

                        /* mark blob as clean */
                        self -> num_writ = 0;

			/* these must not be a point of failure
			   The only failure from the KMD5FileCommit
			   behind these is on NULL parameter */
			rc = KWColumnDataCommitDone ( & col -> df );
			if ( rc == 0 )
			    rc = KWColumnIdxCommitDone ( & col -> idx );

                        return status;
                    }
                }

                /* revert data fork */
                KWColumnDataFree ( & col -> df,
                    & self -> pmnew, self -> num_writ );
		KMD5FileRevert ( self -> col -> df . fmd5 );
            }
        }
    }

    /* remove checksum bytes */
    self -> num_writ -= col -> csbytes;
    return rc;
}

LIB_EXPORT rc_t CC KColumnBlobCommit ( KColumnBlob *bself )
{
    CAST();

    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcCommitting, rcSelf, rcNull );

    if ( self -> loc . id_range == 0 )
        return RC ( rcDB, rcBlob, rcCommitting, rcRange, rcInvalid );

    if ( self -> num_writ == 0 && self -> loc . u . blob . size != 0 )
        return 0;

    assert ( self -> read_only == false );

    rc = KColumnBlobDoCommit ( self );

    if ( rc == kdbReindex )
        rc = KColumnReindex ( & self -> col -> dad );

    return rc;
}
