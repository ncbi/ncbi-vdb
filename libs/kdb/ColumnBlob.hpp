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

#pragma once

/**
* Common code for read- and write side column blobs
*/

typedef struct KColumnBlob KColumnBlob;

#include <klib/refcount.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <klib/checksum.h>
#include <klib/data-buffer.h>

#include <kdb/column.h>

#include <byteswap.h>

#include "colfmt.h"

class KColumnBlobBase
{
protected:
    virtual ~KColumnBlobBase() = 0;
    virtual rc_t whack() = 0;

public:
    /* Public read-side API */
    virtual rc_t addRef() = 0;
    virtual rc_t release() = 0;
    virtual rc_t read ( size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining ) const = 0;
    virtual rc_t readAll ( struct KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size ) const = 0;
    virtual rc_t validate() const = 0;
    virtual rc_t validateBuffer ( struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size ) const = 0;
    virtual rc_t idRange ( int64_t *first, uint32_t *count ) const = 0;

    atomic32_t refcount;
};

template<typename T> class TColumnBlob : public KColumnBlobBase
{
public:
    const T * getBlob() const { return m_blob; }
    T * getBlob() { return m_blob; }

protected:
    T * m_blob = nullptr;

protected: // only created trough a factory, destroyed through release()
    TColumnBlob()
    {
        atomic32_set ( & refcount, 1 );
    }
    virtual ~TColumnBlob()
    {
        KRefcountWhack ( & refcount, "KColumnBlob" );
        free( m_blob );
    }

    virtual rc_t whack()
    {
        delete this;
        return 0;
    }

    virtual rc_t dataRead( size_t offset, void *buffer, size_t bsize, size_t *num_read ) const = 0;
    virtual int32_t checksumType() const = 0;
    virtual const KColBlobLoc& getLoc() const = 0;

public:
    /* Public read-side API */
    virtual rc_t addRef()
    {
        atomic32_inc ( & refcount );
        return 0;
    }
    virtual rc_t release()
    {
        if ( atomic32_dec_and_test ( & refcount ) )
            return whack();
        return 0;
    }

    virtual rc_t read ( size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining ) const
    {
        rc_t rc;
        size_t ignore;
        if ( remaining == NULL )
            remaining = & ignore;

        if ( num_read == NULL )
            rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
        else
        {
            size_t size = getLoc() . u . blob . size;

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

    #ifdef _DEBUGGING
                if ( KDbgTestModConds ( DBG_KFS, DBG_FLAG( DBG_KFS_POS ) ) ||
                        KDbgTestModConds ( DBG_KFS, DBG_FLAG( DBG_KFS_PAGE ) ) )
                {
                    KDbgSetColName( m_blob->col->path );
                }
    #endif
                *num_read = 0;
                while ( * num_read < to_read )
                {
                    size_t nread = 0;

                    rc = dataRead ( offset + *num_read,
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

    virtual rc_t readAll ( struct KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size ) const
    {
        rc_t rc = 0;

        if ( opt_cs_data != NULL )
            memset ( opt_cs_data, 0, cs_data_size );

        if ( buffer == NULL )
            rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
        else
        {
            /* determine blob size */
            size_t bsize = getLoc() . u . blob . size;

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
                    rc = read ( 0, buffer -> base, bsize, & num_read, & remaining );
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
                            switch ( checksumType() )
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
                                rc = dataRead (bsize, opt_cs_data, cs_bytes, & num_read );
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

    virtual rc_t validate() const
    {
        if ( getLoc() . u . blob . size == 0 )
            return 0;

        switch ( checksumType() )
        {
        case kcsCRC32:
            return validateCRC32();
        case kcsMD5:
            return validateMD5();
        case kcsNone:
            return SILENT_RC ( rcDB, rcBlob, rcValidating, rcChecksum, rcNotFound );
        default:
            return RC ( rcDB, rcBlob, rcValidating, rcType, rcUnexpected );
        }
    }

    virtual rc_t validateBuffer ( struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size ) const
    {
        size_t bsize;

        if ( buffer == NULL || cs_data == NULL )
            return RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );

        bsize = KDataBufferBytes ( buffer );
        if ( bsize < getLoc() . u . blob . size )
            return RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
        if ( bsize > getLoc() . u . blob . size )
            return RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );
        if ( bsize == 0 )
            return 0;

        switch ( checksumType() )
        {
        case kcsCRC32:
            return validateBufferCRC32 ( buffer -> base, bsize,
                m_blob -> bswap ? bswap_32 ( cs_data -> crc32 ) : cs_data -> crc32 );
        case kcsMD5:
            return validateBufferMD5 ( buffer -> base, bsize, cs_data -> md5_digest );
        case kcsNone:
            return SILENT_RC ( rcDB, rcBlob, rcValidating, rcChecksum, rcNotFound );
        default:
            return RC ( rcDB, rcBlob, rcValidating, rcType, rcUnexpected );
        }
    }

    rc_t idRange ( int64_t *first, uint32_t *count ) const
    {
        rc_t rc;

        if ( first == NULL || count == NULL )
            rc = RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
        else if ( getLoc() . id_range == 0 )
            rc = RC ( rcDB, rcBlob, rcAccessing, rcRange, rcEmpty );
        else
        {
            * first = getLoc() . start_id;
            * count = getLoc() . id_range;
            return 0;
        }

        if ( first != NULL )
            * first = 0;
        if ( count != NULL )
            * count = 0;

        return rc;
    }

private:

    rc_t validateCRC32 () const
    {
        rc_t rc;
        uint8_t buffer [ 8 * 1024 ];
        size_t to_read, num_read, total, size;

        uint32_t cs, crc32 = 0;

        /* calculate checksum */
        for ( size = m_blob -> loc . u . blob . size, total = 0; total < size; total += num_read )
        {
            to_read = size - total;
            if ( to_read > sizeof buffer )
                to_read = sizeof buffer;

            rc = dataRead ( total, buffer, to_read, & num_read );
            if ( rc != 0 )
                return rc;
            if ( num_read == 0 )
                return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

            crc32 = CRC32 ( crc32, buffer, num_read );
        }

        /* read stored checksum */
        rc = dataRead ( size, & cs, sizeof cs, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read != sizeof cs )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        if ( m_blob -> bswap )
            cs = bswap_32 ( cs );

        if ( cs != crc32 )
            return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

        return 0;
    }

    rc_t validateMD5 () const
    {
        rc_t rc;
        uint8_t buffer [ 8 * 1024 ];
        size_t to_read, num_read, total, size;

        MD5State md5;
        uint8_t digest [ 16 ];

        MD5StateInit ( & md5 );

        /* calculate checksum */
        for ( size = m_blob -> loc . u . blob . size, total = 0; total < size; total += num_read )
        {
            to_read = size - total;
            if ( to_read > sizeof buffer )
                to_read = sizeof buffer;

            rc = dataRead ( total, buffer, to_read, & num_read );
            if ( rc != 0 )
                return rc;
            if ( num_read == 0 )
                return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

            MD5StateAppend ( & md5, buffer, num_read );
        }

        /* read stored checksum */
        rc = dataRead ( size, buffer, sizeof digest, & num_read );
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

    static rc_t validateBufferCRC32 ( const void * buffer, size_t size, uint32_t cs )
    {
        uint32_t const crc32 = CRC32 ( 0, buffer, size );

        return cs == crc32 ? 0 : RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );
    }

    static rc_t validateBufferMD5 ( const void * buffer, size_t size, const uint8_t cs [ 16 ] )
    {
        uint8_t digest [ 16 ];
        MD5State md5;

        MD5StateInit ( & md5 );
        MD5StateAppend ( & md5, buffer, size );
        MD5StateFinish ( & md5, digest );

        return memcmp ( cs, digest, sizeof digest ) == 0 ? 0 : RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );
    }
};
