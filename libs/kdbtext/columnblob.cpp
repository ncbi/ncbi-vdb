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

ColumnBlob::ColumnBlob( const KJsonValue * p_json )
: m_json( p_json )
{
    KRefcountInit ( & refcount, 1, "KDBText::ColumnBlob", "ctor", "db" );

//** move to inflate() to return rc?
    memset( &m_data, 0, sizeof( m_data ) );
    m_data.elem_count = 0;
    PageMapNew( &m_pm,  0);
//**
}

ColumnBlob::~ColumnBlob()
{
    PageMapRelease( m_pm );
    KDataBufferWhack ( & m_data );
}

rc_t
ColumnBlob::whack()
{
    delete this;
    return 0;
}

rc_t
ColumnBlob::addRef()
{
    return 0;
}

rc_t ColumnBlob::release()
{
    return 0;
}

const uint64_t MaxHeaderSizeBytes = 9;

rc_t ColumnBlob::read ( size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining ) const
{
    rc_t rc = 0;
    if ( buffer == nullptr && bsize != 0 )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcBuffer, rcNull );
    }
    else if ( num_read == nullptr )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else if ( offset != 0 )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcUnsupported );
    }
    else
    {
        KDataBuffer buf;
        rc = KDataBufferMakeBytes( &buf, MaxHeaderSizeBytes );
        if ( rc == 0 )
        {
            rc = serialize( buf );
            if ( rc == 0 )
            {
                size_t toRead = KDataBufferBytes( & buf );
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
                        memmove( buffer, buf . base, toRead );
                    }
                    *num_read = toRead;
                }
            }

            rc_t rc2 = KDataBufferWhack( &buf );
            if ( rc == 0 ) rc = rc2;
        }
    }

    return rc;
}

rc_t ColumnBlob::readAll ( struct KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size ) const
{
    if ( opt_cs_data != NULL )
    {
        memset ( opt_cs_data, 0, cs_data_size ); // we do not populate checksum here
    }

    rc_t rc = 0;
    if ( buffer == NULL )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else
    {
        rc = KDataBufferMakeBytes( buffer, MaxHeaderSizeBytes );
        if ( rc == 0 )
        {
            rc = serialize( *buffer );
        }
    }

    return rc;
}

rc_t ColumnBlob::validate() const
{
    return 0;
}

rc_t ColumnBlob::validateBuffer ( struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size ) const
{
    if ( buffer == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }
    if ( cs_data == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }

    KDataBuffer buf;
    rc_t rc = KDataBufferMakeBytes( &buf, MaxHeaderSizeBytes );
    if ( rc == 0 )
    {
        rc = serialize( buf );
        if ( rc == 0 )
        {
            // check the buffer's size
            size_t bsize = KDataBufferBytes ( buffer );
            if ( bsize < KDataBufferBytes ( &buf ) )
            {
                rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
            }
            if ( rc == 0 && bsize > KDataBufferBytes ( &buf ) )
            {
                rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );
            }
        }

        rc_t rc2 = KDataBufferWhack( &buf );
        if ( rc == 0 ) rc = rc2;
    }
    return rc;
}

rc_t ColumnBlob::idRange ( int64_t *first, uint32_t *count ) const
{
    if ( first == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    }
    if ( count == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    }

    *first = getIdRange().first;
    *count = getIdRange().second;
    return 0;
}

rc_t
Error( char * p_error, size_t p_error_size, const char * text )
{
    string_printf ( p_error, p_error_size, nullptr, text );
    return SILENT_RC( rcDB, rcColumn, rcCreating, rcParam, rcInvalid );
}

rc_t
ColumnBlob::inflate( char * p_error, size_t p_error_size, int8_t p_intSizeBits )
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
                return Error( p_error, p_error_size, "blob: row/start is missing" );
            }
        }
        rc = KJsonGetNumber ( id, &m_startId );
        if ( rc == 0 )
        {
            // count if present
            int64_t countVal = 1;
            const KJsonValue * count = KJsonObjectGetMember ( obj, "count" );
            if ( count != nullptr &&
                ( KJsonGetNumber ( count, &countVal ) != 0 ||
                    countVal < 1 ) )
            {
                return Error( p_error, p_error_size, "blob: count is not a positive number" );
            }

            // value
            const KJsonValue * value = KJsonObjectGetMember ( obj, "value" );
            if ( value == nullptr )
            {
                return Error( p_error, p_error_size, "blob: value is missing" );
            }

            if ( p_intSizeBits != 0 )
            {   // integers
                rc = KDataBufferMake ( & m_data, p_intSizeBits, 0 );
                if ( rc == 0 )
                {
                    int64_t valueInt;
                    if ( KJsonGetNumber ( value, & valueInt ) == 0 )
                    {   // may omit []; a repetition if countVal > 1
                        appendRow( & valueInt, 1, countVal );
                    }
                    else
                    {
                        const KJsonArray * valueArr = KJsonValueToArray( value );
                        if ( valueArr != nullptr )
                        {
                            uint32_t len = KJsonArrayGetLength ( valueArr );
                            // if count is given, should match the length of the array
                            if ( count == nullptr || len == countVal )
                            {
                                for ( uint32_t i = 0; i < len; ++i )
                                {
                                    const KJsonValue * v = KJsonArrayGetElement ( valueArr, i );
                                    assert( v != nullptr );
                                    if ( KJsonGetNumber ( v, & valueInt ) == 0 )
                                    {
                                        appendRow( & valueInt, 1 );
                                    }
                                }
                            }
                            else
                            {
                                return Error( p_error, p_error_size, "blob: count does not match the length of the value array" );
                            }
                        }
                        else
                        {   // invalid value
                            return Error( p_error, p_error_size, "blob: value is not a integer or an array of integers" );
                        }
                    }
                }
            }
            else
            {   // strings
                rc = KDataBufferMake ( & m_data, 8, 0 );
                if ( rc == 0 )
                {
                    const char * valueStr = nullptr;
                    if ( KJsonGetString ( value, & valueStr ) == 0 )
                    {   // may omit []; a repetition if countVal > 1
                        appendRow( valueStr, strlen( valueStr ), countVal );
                    }
                    else
                    {
                        const KJsonArray * valueArr = KJsonValueToArray( value );
                        if ( valueArr != nullptr )
                        {
                            uint32_t len = KJsonArrayGetLength ( valueArr );
                            // if count is given, should match the length of the array
                            if ( count == nullptr || len == countVal )
                            {
                                for ( uint32_t i = 0; i < len; ++i )
                                {
                                    const KJsonValue * v = KJsonArrayGetElement ( valueArr, i );
                                    assert( v != nullptr );
                                    const char * valueStr = nullptr;
                                    if ( KJsonGetString ( v, & valueStr ) == 0 )
                                    {
                                        appendRow( valueStr, strlen( valueStr ), 1 );
                                    }
                                }
                            }
                            else
                            {
                                return Error( p_error, p_error_size, "blob: count does not match the length of the value array" );
                            }
                        }
                        else
                        {   // invalid value
                            return Error( p_error, p_error_size, "blob: value is not a string or an array of strings" );
                        }
                    }
                }
            }
        }
        else
        {   // not an integer
            return Error( p_error, p_error_size, "blob: row/start is not an integer" );
        }
    }
    else
    {   // not an object
        return Error( p_error, p_error_size, "blob: not a Json object" );
    }

    return rc;
}

rc_t
ColumnBlob::appendRow( const void * p_data, size_t p_sizeInElems, uint32_t p_repeatCount )
{
    if ( p_sizeInElems == 0 )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAppending, rcData, rcEmpty );
    }
    rc_t rc = PageMapAppendRows( m_pm, p_sizeInElems, p_repeatCount, false ); // no "same data as on previous call", for now
    if ( rc == 0 )
    {
        m_count += p_repeatCount;

        // apend data to m_data
        size_t old_size_bits = KDataBufferBits( & m_data );
        rc = KDataBufferResize( &m_data, m_data.elem_count + p_sizeInElems );
        if ( rc == 0 )
        {
            bitcpy( m_data.base, old_size_bits, p_data, 0, p_sizeInElems * m_data.elem_bits );
        }
    }
    return rc;
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
static rc_t encode_header_v1(
                             uint8_t *dst,
                             uint64_t dsize,
                             uint64_t *used,
                             uint32_t row_length_elems,
                             bitsz_t data_size,
                             VByteOrder byte_order
)
{
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
    if ( row_length_elems == 1 ) {
        header_byte |= 3 << 5;
        * used = 1;
        if ( dsize < * used )
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
    }
    else if (row_length_elems < 0x100) {
        *used = 2;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length_elems;
    }
    else if (row_length_elems < 0x10000) {
        header_byte |= 1 << 5;
        *used = 3;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length_elems;
        dst[2] = ( uint8_t ) ( row_length_elems >> 8 );
    }
    else {
        header_byte |= 2 << 5;
        *used = 5;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);
        dst[0] = header_byte;
        dst[1] = ( uint8_t ) row_length_elems;
        dst[2] = ( uint8_t ) ( row_length_elems >> 8 );
        dst[3] = ( uint8_t ) ( row_length_elems >> 16 );
        dst[4] = ( uint8_t ) ( row_length_elems >> 24 );
    }
    return 0;
}

static rc_t encode_header_v2(
                             uint8_t *dst,
                             uint64_t dsize,
                             uint64_t *used,
                             uint64_t hdr_size,
                             uint64_t map_size,
                             bitsz_t data_size
)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t header_byte = 0x80 | ( (uint8_t)data_size & 7 );
#else
    uint8_t header_byte = 0x88 | ( (uint8_t)data_size & 7 );
#endif

    assert(hdr_size >> 32 == 0);
    assert(map_size >> 32 == 0);

    if ((hdr_size >> 8) == 0) {
        if ((map_size >> 8) == 0) {
            *used = 3;
            if (dsize < *used)
                return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);

            dst[0] = header_byte;
            dst[1] = hdr_size;
            dst[2] = map_size;
        }
        else if ((map_size >> 16) == 0) {
            *used = 4;
            if (dsize < *used)
                return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);

            dst[0] = header_byte | 0x10;
            dst[1] = hdr_size;
            dst[2] = map_size;
            dst[3] = map_size >> 8;
        }
        else {
            *used = 6;
            if (dsize < *used)
                return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);

            dst[0] = header_byte | 0x20;
            dst[1] = hdr_size;
            dst[2] = map_size;
            dst[3] = map_size >> 8;
            dst[4] = map_size >> 16;
            dst[5] = map_size >> 24;
        }
    }
    else {
        *used = 9;
        if (dsize < *used)
            return RC(rcVDB, rcBlob, rcConstructing, rcBuffer, rcInsufficient);

        dst[0] = header_byte | 0x30;

        dst[1] = hdr_size;
        dst[2] = hdr_size >> 8;
        dst[3] = hdr_size >> 16;
        dst[4] = hdr_size >> 24;

        dst[5] = map_size;
        dst[6] = map_size >> 8;
        dst[7] = map_size >> 16;
        dst[8] = map_size >> 24;
    }
    return 0;
}

// API

rc_t
ColumnBlob::serialize( KDataBuffer & p_buf ) const
{
    rc_t rc = 0;

    const KDataBuffer & data = getData();
    const size_t data_bytes = KDataBufferBytes( & data );
    const size_t data_bits = KDataBufferBits( & data );
    const PageMap & pageMap = getPageMap();

    uint32_t row_length_elems = PageMapHasSimpleStructure( & pageMap );
    if ( row_length_elems != 0 )
    {   // fixed row length
        uint64_t header_size = 0;
        rc = encode_header_v1( (uint8_t *) p_buf.base,
                                p_buf.elem_count,
                                & header_size,
                                row_length_elems,
                                KDataBufferBits( &data ),
                                vboNative );
        if ( rc == 0 )
        {   // append data
            rc = KDataBufferResize ( &p_buf, header_size + data_bytes );
            if ( rc == 0 )
            {
                memmove( (uint8_t*)p_buf.base + header_size , data . base, data_bytes );
            }
        }
    }
    else
    {   // include page map
        //TODO: headers if any
        KDataBuffer pm;
        rc = KDataBufferMakeBytes(&pm, 0);
        if (rc == 0)
        {
            uint64_t pm_size;
            rc = PageMapSerialize( & pageMap, &pm, 0, &pm_size);
            if ( rc == 0 )
            {
                uint64_t header_size;
                rc = encode_header_v2( (uint8_t *) p_buf.base, p_buf.elem_count, &header_size, 0 /*headers.elem_count*/, pm.elem_count, data_bits );
                if ( rc == 0 )
                {
                    rc = KDataBufferResize( &p_buf, header_size + /*headers.elem_count +*/ pm_size + data_bytes);
                    if (rc == 0) {
                        size_t offset = header_size;
                        //memmove((uint8_t *)p_buf.base + offset, headers.base, headers.elem_count);
                        // offset += headers.elem_count
                        memmove((uint8_t *)p_buf.base + offset, pm.base, pm_size);
                        offset += pm.elem_count;
                        memmove( (uint8_t*)p_buf.base + offset , data.base, data_bytes );
                    }
                }
            }
            KDataBufferWhack( &pm );
        }
    }

    return rc;
}


