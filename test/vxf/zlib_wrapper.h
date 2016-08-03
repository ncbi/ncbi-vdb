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

#ifndef _h_zlib_wrapper_
#define _h_zlib_wrapper_

#include <klib/rc.h>
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    size_t size;
    size_t used;
    void * buf;
} w_buf;

/*
    strategy ... Z_RLE
    level    ... Z_BEST_SPEED
*/

rc_t wrapped_zlib_compress( w_buf * dst, const void * src, size_t ssize, int32_t strategy, int32_t level );
rc_t wrapped_zlib_decompress( void *dst, size_t dsize, size_t *psize, const void *src, size_t ssize );

rc_t make_uint8_buffer( uint8_t ** buffer, size_t size );
rc_t make_random_uint8_buffer( uint8_t ** buffer, size_t size, uint8_t min_value, uint8_t max_value );
rc_t prepare_compress_buffer( w_buf * dst, size_t size );
void finish_compress_buffer( w_buf * dst );

#ifdef __cplusplus
}
#endif

#endif
