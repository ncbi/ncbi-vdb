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

#include "zlib_wrapper.h"
#include <string.h>

typedef struct {
    size_t size;
    size_t used;
    void *buf;
} szbuf;

static rc_t zlib_compress( szbuf *dst, const void *src, size_t ssize, int32_t strategy, int32_t level )
{
    z_stream s;
    int zr;
    rc_t rc = 0;
    
    memset( &s, 0, sizeof( s ) );
    s.next_in = ( void * )src;
    s.avail_in = ( uInt )ssize;
    s.next_out = dst->buf;
    s.avail_out = ( uInt )dst->size;
    
    dst->used = 0;
    
    zr = deflateInit2( &s, level, Z_DEFLATED, -15, 9, strategy );
    switch ( zr )
    {
        case 0 : break;
        case Z_MEM_ERROR : return RC( rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted );
        case Z_STREAM_ERROR : return RC( rcVDB, rcFunction, rcExecuting, rcParam, rcInvalid );
        default: return RC( rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected );
    }
    zr = deflate( &s, Z_FINISH );
    switch ( zr )
    {
        case Z_STREAM_END : break;
        case Z_OK : s.total_out = 0; break;
        default : rc = RC( rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected ); break;
    }
    zr = deflateEnd( &s );
    if ( zr != Z_OK )
        rc = RC( rcVDB, rcFunction, rcExecuting, rcSelf, rcUnexpected );
    if ( rc == 0 )
        dst->used = ( uint32_t )s.total_out;
    return rc;
}

static rc_t zlib_decompress(void *dst, size_t dsize, size_t *psize, const void *src, size_t ssize) {
    z_stream s;
    int zr;
    rc_t rc;
    
    memset(&s, 0, sizeof(s));
    s.next_in = (void *)src;
    s.avail_in = (uInt)ssize;
    s.next_out = dst;
    s.avail_out = (uInt)dsize;
    
    zr = inflateInit2(&s, -15);
    switch (zr) {
    case 0:
        break;
    case Z_MEM_ERROR:
        return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
    default:
        return RC(rcVDB, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
    }
    zr = inflate(&s, Z_FINISH);
    switch (zr) {
    case Z_STREAM_END:
    case Z_OK:
        *psize = s.total_out;
        rc = 0;
        break;
    case Z_BUF_ERROR:
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt);
        break;
    case Z_MEM_ERROR:
        rc = RC(rcXF, rcFunction, rcExecuting, rcMemory, rcExhausted);
        break;
    default:
        rc = RC(rcVDB, rcFunction, rcExecuting, rcNoObj, rcUnexpected);
        break;
    }
    if (inflateEnd(&s) == Z_OK) return rc;
    
    return rc == 0 ? RC(rcVDB, rcFunction, rcExecuting, rcData, rcCorrupt) : rc;
}

rc_t wrapped_zlib_compress( w_buf * dst, const void * src, size_t ssize, int32_t strategy, int32_t level )
{
    return zlib_compress( ( szbuf * )dst, src, ssize, strategy, level );
}


rc_t wrapped_zlib_decompress( void *dst, size_t dsize, size_t *psize, const void *src, size_t ssize )
{
    return zlib_decompress( dst, dsize, psize, src, ssize );
}


rc_t make_uint8_buffer( uint8_t ** buffer, size_t size )
{
    if ( buffer == NULL )
        return RC( rcRuntime, rcFunction, rcConstructing, rcParam, rcNull );
    else
    {
        uint8_t * b = malloc( size );
        if ( b == NULL )
            return RC( rcRuntime, rcFunction, rcConstructing, rcMemory, rcExhausted );
        else
            *buffer = b;
    }
    return 0;
}


rc_t make_random_uint8_buffer( uint8_t ** buffer, size_t size, uint8_t min_value, uint8_t max_value )
{
    rc_t rc = make_uint8_buffer( buffer, size );
    if ( rc == 0 )
    {
        size_t i;
        uint8_t * b = *buffer;
        int range = ( max_value - min_value );
        for ( i = 0; i < size; ++i )
            b[ i ] = min_value + ( ( rand() % range ) & 0xFF );
    }
    return rc;
}


rc_t prepare_compress_buffer( w_buf * dst, size_t size )
{
    if ( dst == NULL )
        return RC( rcRuntime, rcFunction, rcConstructing, rcParam, rcNull );
    else
    {
        dst->size = size;
        dst->used = 0;
        dst->buf = malloc( size );
        if ( dst->buf == NULL )
            return RC( rcRuntime, rcFunction, rcConstructing, rcMemory, rcExhausted );
    }
    return 0;
}


void finish_compress_buffer( w_buf * dst )
{
    if ( dst != NULL )
    {
        free( ( void * ) dst->buf );
    }
}
