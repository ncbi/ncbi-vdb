/*===========================================================================
*
*                            Public Domain Notice
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

#define KSTREAM_IMPL KBufferStream
typedef struct KBufferStream KBufferStream;

#include <kns/extern.h>

#include <klib/rc.h>
#include <klib/text.h> /* String */
#include <kns/impl.h> /* KStream */

#ifndef rcStream
    #define rcStream rcFile
#endif

struct KBufferStream {
    KStream dad;
    String buffer;
};

static
rc_t CC KBufferStreamWhack ( KBufferStream * self )
{
    if ( self != NULL ) {
        memset ( self, 0, sizeof * self );
        free ( self );
    }

    return 0;
}

static
rc_t CC KBufferStreamRead ( const KBufferStream * self, void * buffer,
    size_t bsize, size_t * num_read )
{
    String * src = NULL;

    size_t dummy = 0;
    if ( num_read == NULL ) {
        num_read = & dummy;
    }

    * num_read = 0;

    assert ( self );

    src = ( String * ) & self -> buffer;

    if ( src -> size == 0 ) {
        return 0;
    }

    if ( src -> size < bsize ) {
        bsize = src -> size;
    }

    * num_read = string_copy ( buffer, bsize, src -> addr, src -> size );

    src -> addr += * num_read;
    src -> size -= * num_read;
    src -> len  -= * num_read;

    return 0;
}

static
rc_t CC KBufferStreamWrite ( KBufferStream * self, const void * buffer,
    size_t size, size_t * num_writ )
{
    if ( num_writ != NULL ) {
        * num_writ = 0;
    }

    return 0;
}

static
rc_t CC KBufferStreamTRead ( const KBufferStream * self, void * buffer,
    size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    return KBufferStreamRead ( self, buffer, bsize, num_read );
}

static
rc_t CC KBufferStreamTWrite ( KBufferStream * self, const void * buffer,
    size_t size, size_t * num_writ, struct timeout_t * tm )
{
    return KBufferStreamWrite ( self, buffer, size, num_writ );
}

static KStream_vt_v1 vtKBufferStream = {
    1, 1,
    KBufferStreamWhack,
    KBufferStreamRead,
    KBufferStreamWrite,
    KBufferStreamTRead,
    KBufferStreamTWrite,
};

LIB_EXPORT rc_t CC KStreamMakeFromBuffer ( KStream ** self, const char * buffer,
    size_t size )
{
    rc_t rc= 0;    

    KBufferStream * obj = NULL;

    if ( self == NULL ) {
        return RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    }

    obj = calloc ( 1, sizeof *obj );
    if ( obj == NULL ) {
        return RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
    }

    if ( buffer == NULL ) {
        size = 0;
    }

    rc = KStreamInit ( & obj -> dad, ( const KStream_vt* ) & vtKBufferStream,
        "KBufferStream", "KBufferStream", true, false );
    if ( rc == 0 ) {
        StringInit ( & obj -> buffer, buffer, size, size );
        * self = & obj -> dad;
    }
    else {
        KBufferStreamWhack ( obj );
    }

    return rc;
}

/*
#include "stream.h"
void test ( void ) {
    char b[] = "0123456789ABCDEFGHIJKLMNOPQRSTVWXYZabcdefghijklmnopqrstuvwxyz~";
    int i = 0;
    for ( i = 0; i < sizeof b; ++i) {
        b[i] = i;
    }
    const KStream * s = NULL;
puts("TEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEST");
    rc_t rc = KStreamMakeFromBuffer (   0,  0 , 0,        0 );
    assert(rc);
    rc      = KStreamMakeFromBuffer ( & s,  0 , 0,        0 );
    assert ( ! rc );
    rc = KStreamRelease ( s );
    assert ( ! rc );
    rc      = KStreamMakeFromBuffer ( 0  , "X", 0,        0 );
    assert ( rc );
    rc      = KStreamMakeFromBuffer ( 0  ,  0 , b,        0 );
    assert ( rc );
    rc      = KStreamMakeFromBuffer ( 0  ,  0 , 0, sizeof b );
    assert ( rc );
    rc      = KStreamMakeFromBuffer ( & s, "X", 0,        0 );
    assert ( ! rc );
    char c[99] = "";
    size_t num_read = 0;
    rc = KStreamRead ( 0, 0, 0,   0        );
    assert ( rc );
    rc = KStreamRead ( s, 0, 0,   0        );
    assert ( rc );
    rc = KStreamRead ( 0, 0, 0, & num_read );
    assert ( rc );
    rc = KStreamRelease ( s );
    assert ( ! rc );
}
*/
