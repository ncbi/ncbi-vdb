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

#include "columnblob.hpp"

#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>

using namespace KDBText;
using namespace std;

/*--------------------------------------------------------------------------
 * KTextColumnBlob
 *  one or more rows of column data
 */

static rc_t KTextColumnBlobWhack ( KColumnBlob *self );
static rc_t CC KTextColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KTextColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
static rc_t CC KTextColumnBlobValidate ( const KColumnBlob *self );
static rc_t CC KTextColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
static rc_t CC KTextColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlob_vt KTextColumnBlob_vt =
{
    /* Public API */
    KTextColumnBlobWhack,
    KColumnBlobBaseAddRef,
    KColumnBlobBaseRelease,
    KTextColumnBlobRead,
    KTextColumnBlobReadAll,
    KTextColumnBlobValidate,
    KTextColumnBlobValidateBuffer,
    KTextColumnBlobIdRange
};

#define CAST() assert( bself->vt == &KTextColumnBlob_vt ); ColumnBlob * self = (ColumnBlob *)bself

ColumnBlob::ColumnBlob( const void * data, size_t size, const Column * col, int64_t id, uint64_t count )
:   m_data ( data ),
    m_size( size ),
    m_parent( col ),
    m_firstRow( id ),
    m_count( count )
{
    dad . vt = & KTextColumnBlob_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::ColumnBlob", "ctor", "db" );
}

ColumnBlob::~ColumnBlob()
{
    KRefcountWhack ( & dad . refcount, "KDBText::ColumnBlob" );
}

// API

static
rc_t
KTextColumnBlobWhack ( KColumnBlob *bself )
{
    CAST();

    delete reinterpret_cast<ColumnBlob*>( self );
    return 0;
}

// copied from interfaces/vdb/xform.h
typedef uint8_t VByteOrder;
enum
{
    vboNone,
    vboNative,
    vboLittleEndian,
    vboBigEndian
};

// copied from vdb/blob.c
static
rc_t encode_header_v1(uint8_t *dst,
                      uint64_t dsize,
                      uint64_t *used,
                      uint32_t row_length,
                      bitsz_t data_size)
{
    /* byte-order goes in bits 0..1 */
    uint8_t header_byte;
#if __BYTE_ORDER == __LITTLE_ENDIAN
        header_byte = ( uint8_t) vboLittleEndian;
#else
        header_byte = ( uint8_t) vboBigEndian;
#endif

    /* blob size adjust goes in bits 2..4 */
    header_byte |= ( ( 8 - ( data_size & 7 ) ) & 7 ) << 2;

    /* row-length code goes in bits 5..6 */
    if ( row_length == 1 ) {
        header_byte |= 3 << 5;
        * used = 1;
        if ( dsize < * used )
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
    }
    else if (row_length < 0x100) {
        *used = 2;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length;
    }
    else if (row_length < 0x10000) {
        header_byte |= 1 << 5;
        *used = 3;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length;
        dst[2] = ( uint8_t ) ( row_length >> 8 );
    }
    else {
        header_byte |= 2 << 5;
        *used = 5;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length;
        dst[2] = ( uint8_t ) ( row_length >> 8 );
        dst[3] = ( uint8_t ) ( row_length >> 16 );
        dst[4] = ( uint8_t ) ( row_length >> 24 );
    }
    return 0;
}


static
rc_t CC
KTextColumnBlobRead ( const KColumnBlob *bself, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining )
{
    CAST();

    rc_t rc = 0;
    if ( buffer == nullptr && bsize != 0 )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcBuffer, rcNull );
    }
    else if ( num_read == nullptr )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else if ( offset > self -> getSize() )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcExcessive );
    }
    else
    {
        // encode header
        uint64_t header_size;
        uint8_t header[5];
        rc = encode_header_v1( header, sizeof(header), &header_size, 1, self -> getSize() );
        if ( rc == 0 )
        {
            size_t toRead = header_size + self -> getSize() - offset;
            if ( bsize == 0 )
            {
                * remaining = toRead;
                * num_read = 0;
            }
            else
            {
                if ( toRead > bsize )
                {
                    * remaining = toRead - bsize;
                    toRead -= *remaining;
                }
                else
                {
                    * remaining = 0;
                }

                if ( toRead > 0 )
                {
                    memmove( buffer, header, header_size );
                    memmove( (uint8_t*)buffer + header_size, (const char*)(self -> getData()) + offset, toRead - header_size);
                }
                *num_read = header_size;
            }
        }
    }

    return rc;
}

static
rc_t CC
KTextColumnBlobReadAll ( const KColumnBlob * bself, KDataBuffer * dbuffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    CAST();

    if ( opt_cs_data != NULL )
    {
        memset ( opt_cs_data, 0, cs_data_size ); // we do not populate checksum here
    }

    rc_t rc = 0;
    if ( dbuffer == NULL )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else
    {
        rc = KDataBufferMakeBytes ( dbuffer, self->getSize() );
        if ( rc == 0 && self->getSize() > 0 )
        {
            memmove( dbuffer -> base, self -> getData(), self->getSize() );
        }
    }

    return rc;
}

static
rc_t CC
KTextColumnBlobValidate ( const KColumnBlob *self )
{
    return 0;
}

static
rc_t CC
KTextColumnBlobValidateBuffer ( const KColumnBlob * bself, const KDataBuffer * dbuffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    CAST();

    if ( dbuffer == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }
    if ( cs_data == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }

    // check the buffer's size
    size_t bsize = KDataBufferBytes ( dbuffer );
    if ( bsize < self -> getSize() )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
    }
    if ( bsize > self -> getSize() )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );
    }

    return 0;
}

static
rc_t CC
KTextColumnBlobIdRange ( const KColumnBlob * bself, int64_t *first, uint32_t *count )
{
    CAST();

    if ( first == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    }
    if ( count == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    }

    *first = self->getIdRange().first;
    *count = self->getIdRange().second;
    return 0;
}
