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

#include <kdb/page-map.h>

#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <klib/json.h>

#include <bitstr.h>

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

ColumnBlob::ColumnBlob( const KJsonValue * p_json )
: m_json( p_json )
{
    dad . vt = & KTextColumnBlob_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::ColumnBlob", "ctor", "db" );

//** move to inflate() to return rc?
    KDataBufferMake ( & m_data, 1, 1 );
    m_data.elem_count = 0;
    PageMapNew( &m_pm,  0);
//**
}

ColumnBlob::~ColumnBlob()
{
    PageMapRelease( m_pm );

    KDataBufferWhack ( & m_data );
    KRefcountWhack ( & dad . refcount, "KDBText::ColumnBlob" );
}

void
ColumnBlob::addRef( const ColumnBlob * b )
{
    if ( b != nullptr )
    {
        KColumnBlobAddRef( (const KColumnBlob*) b );
    }
}

void
ColumnBlob::release( const ColumnBlob * b )
{
    if ( b != nullptr )
    {
        KColumnBlobRelease( (const KColumnBlob*) b );
    }
}

rc_t
ColumnBlob::inflate( char * p_error, size_t p_error_size )
{
    rc_t rc = 0;

    const KJsonObject * obj = KJsonValueToObject ( m_json );
    if( obj != nullptr )
    {
        // row / start
        const KJsonValue * id = KJsonObjectGetMember ( obj, "row" );
        if ( id == nullptr )
        {   // "start" is an alternative
            id = KJsonObjectGetMember ( obj, "start" );
            if ( id == nullptr )
            {
                string_printf ( p_error, p_error_size, nullptr, "blob: row/start is missing" );
                return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
            }
        }

        int64_t rowId;
        rc = KJsonGetNumber ( id, &rowId );
        if ( rc == 0 )
        {
//             if ( findBlob( rowId ) != m_data.end() )
//             {
//                 string_printf ( p_error, p_error_size, nullptr, "Duplicate row id: %s", rowId );
//                 return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
//             }

            if ( m_startId == 0 )
            {
                m_startId = rowId;
            }

            // count
            int64_t countVal = 1;
            const KJsonValue * count = KJsonObjectGetMember ( obj, "count" );
            if ( count != nullptr &&
                ( KJsonGetNumber ( count, &countVal ) != 0 ||
                    countVal < 1 ) )
            {
                string_printf ( p_error, p_error_size, nullptr, "blob: count is not a positive number" );
                return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
            }

            // value
            const KJsonValue * value = KJsonObjectGetMember ( obj, "value" );
            if ( value == nullptr )
            {
                string_printf ( p_error, p_error_size, nullptr, "blob: value is missing" );
                return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
            }

            const char * valueStr = nullptr;
            if ( KJsonGetString ( value, & valueStr ) == 0 )
            {   // may omit []; a repetition if countVal > 1
                appendRow( valueStr, strlen( valueStr ) * 8, countVal );
            }
            else
            {
                const KJsonArray * valueArr = KJsonValueToArray( value );
                if ( valueArr != nullptr )
                {
                    uint32_t len = KJsonArrayGetLength ( valueArr );
                    for ( uint32_t i = 0; i < len; ++i )
                    {
                        const KJsonValue * v = KJsonArrayGetElement ( valueArr, i );
                        assert( v != nullptr );
                        const char * valueStr = nullptr;
                        if ( KJsonGetString ( value, & valueStr ) == 0 )
                        {   // may omit []; a repetition if countVal > 1
                            appendRow( valueStr, strlen( valueStr ) * 8, countVal );
                        }
                        //TODO: non-strings
                    }
                }
                else
                {   // invalid value
                    string_printf ( p_error, p_error_size, nullptr, "blob: value is not a string/array" );
                    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
                }
            }
        }
        else
        {   // not an integer
            string_printf ( p_error, p_error_size, nullptr, "blob: row/start is not an integer" );
            return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
        }
    }
    else
    {   // not an object
        string_printf ( p_error, p_error_size, nullptr, "blob: not a Json object" );
        return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
    }

    return rc;
}

rc_t
ColumnBlob::appendRow( const void * data, size_t sizeBits, uint32_t count )
{
    if ( sizeBits == 0 )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAppending, rcData, rcEmpty );
    }

    rc_t rc = PageMapAppendRows( m_pm, sizeBits, count, false ); // no "same data as on previous call", for now
    if ( rc == 0 )
    {
        m_count += count;

        // apend data to m_data
        size_t old_size = KDataBufferBits( & m_data );
        rc = KDataBufferResize( &m_data, old_size + sizeBits );
        if ( rc == 0 )
        {
            bitcpy( m_data.base, old_size, data, 0, sizeBits );
        }
        return rc;
    }
}

#if 0
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
static rc_t encode_header_v1(
                             uint8_t *dst,
                             uint64_t dsize,
                             uint64_t *used,
                             uint32_t row_length,
                             bitsz_t data_size,
                             VByteOrder byte_order
) {
    /* byte-order goes in bits 0..1 */
    uint8_t header_byte = byte_order & 3;
    if ( header_byte == vboNative )
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        header_byte = ( uint8_t) vboLittleEndian;
#else
        header_byte = ( uint8_t) vboBigEndian;
#endif
    }

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

// logic copied from vdb/blob.c, VBlobSerialize()
rc_t
ColumnBlob::serialize( KDataBuffer & buf ) const
{
    if ( m_pm == nullptr )
    {   // no rows have been added
        return SILENT_RC ( rcDB, rcBlob, rcCreating, rcData, rcEmpty );
    }

    uint32_t row_length_bits = PageMapHasSimpleStructure( m_pm );
    if ( row_length_bits != 0)
    {   // fixed length rows. header, data
        uint64_t data_bytes = KDataBufferBytes( & m_data );
        rc_t rc = KDataBufferResize( &buf, 5 + data_bytes );
        if (rc == 0) {
            bitsz_t data_bits = KDataBufferBits( & m_data );
            uint64_t sz;
            VByteOrder bo;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            bo = vboLittleEndian;
#else
            bo = vboBigEndian;
#endif
            rc = encode_header_v1( (uint8_t*)buf.base, buf.elem_count, &sz, row_length_bits, data_bits, bo );
            if (rc == 0) {
                memmove(&((uint8_t *)buf.base)[sz], m_data.base, data_bytes );
                buf.elem_count = sz + data_bytes;
            }
        }
    }
    else
    {
        //TODO: header, page map, data
    }

    return 0;
}
#endif

// API

static
rc_t
KTextColumnBlobWhack ( KColumnBlob *bself )
{
    CAST();

    delete reinterpret_cast<ColumnBlob*>( self );
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
    else
    {
        KDataBuffer buf;
        rc = KDataBufferMakeBytes( &buf, bsize );
        if ( rc == 0 )
        {
            // rc = self -> serialize( buf ); // will increase size if needed
            // if ( rc == 0 )
            // {
            //     if ( offset > KDataBufferBytes( & buf ) )
            //     {
            //         rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcExcessive );
            //     }
            //     else
            //     {
            //         size_t toRead = KDataBufferBytes( & buf ) - offset;
            //         if ( bsize == 0 )
            //         {
            //             * remaining = toRead;
            //             * num_read = 0;
            //         }
            //         else
            //         {
            //             if ( toRead > bsize )
            //             {
            //                 * remaining = toRead - bsize;
            //                 toRead -= *remaining;
            //             }
            //             else
            //             {
            //                 * remaining = 0;
            //             }

            //             if ( toRead > 0 )
            //             {
            //                 memmove( buffer, (const char*)(buf . base) + offset, toRead );
            //             }
            //             *num_read = toRead;
            //         }
            //     }
            // }

            rc_t rc2 = KDataBufferWhack( &buf );
            if ( rc == 0 ) rc = rc2;
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
//        rc = self -> serialize( *dbuffer ); // will increase size if needed
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

    KDataBuffer buf;
    rc_t rc = KDataBufferMakeBytes( &buf, 1 );
    if ( rc == 0 )
    {
        // rc = self -> serialize( buf ); // will increase size if needed
        // if ( rc == 0 )
        // {
        //     // check the buffer's size
        //     size_t bsize = KDataBufferBytes ( dbuffer );
        //     if ( bsize < KDataBufferBytes ( &buf ) )
        //     {
        //         return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
        //     }
        //     if ( bsize > KDataBufferBytes ( &buf ) )
        //     {
        //         return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );
        //     }
        // }

        rc_t rc2 = KDataBufferWhack( &buf );
        if ( rc == 0 ) rc = rc2;
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
