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


typedef struct KBufferedStream KBufferedStream;
#define KSTREAM_IMPL KBufferedStream

#include <kns/extern.h>
#include <kns/stream.h>
#include <kns/impl.h>
#include <kfs/file.h>
#include <kfs/impl.h>
#include <klib/rc.h>

#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stream-priv.h"

#define DEFAULT_BUFFER_SIZE ( 32 * 1024 )

/* the object structure for THIS implementation */
struct  KBufferedStream
{
    /* THIS MUST COME FIRST */
    KStream dad;

    const KStream * in;
    KStream * out;

    size_t buffer_size;

    uint8_t * in_buffer;
    size_t in_marker;
    size_t in_limit;

    uint8_t * out_buffer;
    size_t out_marker;
};


/* the required methods */

static
rc_t CC KBufferedStreamWhack ( KBufferedStream *self )
{
    if ( self -> out_buffer != NULL && self -> out_marker != 0 )
    {
        size_t num_writ;
        KStreamWriteAll ( self -> out, self -> out_buffer, self -> out_marker, & num_writ );
    }

    KStreamRelease ( self -> in );
    KStreamRelease ( self -> out );

    if ( self -> in_buffer != NULL )
        free ( self -> in_buffer );
    if ( self -> out_buffer != NULL )
        free ( self -> out_buffer );

    free ( self );
    return 0;
}

static
rc_t CC KBufferedStreamRead ( const KBufferedStream *cself,
    void *buffer, size_t bsize, size_t *num_read )
{
    KBufferedStream * self = ( KBufferedStream * ) cself;

    size_t avail = self -> in_limit - self -> in_marker;
    if ( avail == 0 )
    {
        size_t limit;
        rc_t rc = KStreamRead ( self -> in, self -> in_buffer, self -> buffer_size, & limit );
        if ( rc != 0 )
        {
            * num_read = 0;
            return rc;
        }

        self -> in_marker = 0;
        avail = self -> in_limit = limit;

        if ( limit == 0 )
        {
            * num_read = 0;
            return 0;
        }
    }

    if ( bsize > avail )
        bsize = avail;

    memmove ( buffer, & self -> in_buffer [ self -> in_marker ], bsize );
    self -> in_marker += bsize;

    * num_read = bsize;
    return 0;
}

static
rc_t CC KBufferedStreamWrite ( KBufferedStream * self,
    const void * buffer, size_t size, size_t * total_writ )
{
    const uint8_t * p = buffer;

    size_t total, num_writ = 0;
    for ( total = 0; total < size; total += num_writ )
    {
        size_t avail = self -> buffer_size - self -> out_marker;
        if ( avail == 0 )
        {
            rc_t rc = KStreamWriteAll ( self -> out, self -> out_buffer, self -> out_marker, & num_writ );
            if ( rc != 0 )
            {
                * total_writ = 0;
                return rc;
            }
            if ( num_writ != self -> out_marker )
            {
                * total_writ = total;
                return RC ( rcNS, rcStream, rcWriting, rcTransfer, rcIncomplete );
            }

            self -> out_marker = 0;
            avail = self -> buffer_size;
        }

        num_writ = size - total;
        if ( num_writ > avail )
            num_writ = avail;

        memmove ( & self -> out_buffer [ self -> out_marker ], & p [ total ], num_writ );
        self -> out_marker += num_writ;
    }

    * total_writ = total;
    return 0;
}

/* the vtable */
static KStream_vt_v1 vtKBufferedStream =
{
    1, 0,
    KBufferedStreamWhack,
    KBufferedStreamRead,
    KBufferedStreamWrite
};


/* MakeBuffered
 *  makes a one or two-way stream buffer
 */
LIB_EXPORT rc_t CC KStreamMakeBuffered ( KStream ** buffered,
    const KStream * in, KStream * out, size_t buffer_size )
{
    rc_t rc;

    if ( buffered == NULL )
        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    else
    {
        bool can_read = ( in == NULL ) ? false : in -> read_enabled;
        bool can_write = ( out == NULL ) ? false : out -> write_enabled;

        if ( ! can_read && ! can_write )
            rc = RC ( rcNS, rcStream, rcConstructing, rcStream, rcNoPerm );
        else
        {
            KBufferedStream * bs = calloc ( 1, sizeof * bs );
            if ( bs == NULL )
                rc = RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KStreamInit ( & bs -> dad, ( const KStream_vt* ) & vtKBufferedStream, 
                    "KBufferedStream", "adapter", can_read, can_write );
                if ( rc == 0 )
                {
                    bs -> buffer_size = buffer_size ? buffer_size : DEFAULT_BUFFER_SIZE;
                    if ( in != NULL )
                        bs -> in_buffer = malloc ( bs -> buffer_size );
                    if ( out != NULL )
                        bs -> out_buffer = malloc ( bs -> buffer_size );
                    if ( ( in != NULL && bs -> in_buffer == NULL ) ||
                         ( out != NULL && bs -> out_buffer == NULL ) )
                    {
                        rc = RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
                    }
                    else
                    {
                        rc = KStreamAddRef ( in );
                        if ( rc == 0 )
                        {
                            rc = KStreamAddRef ( out );
                            if ( rc ==  0 )
                            {
                                bs -> in = in;
                                bs -> out = out;
                                * buffered = & bs -> dad;
                                
                                return 0;
                            }
                            
                            KStreamRelease ( in );
                        }
                    }

                    if ( bs -> in_buffer != NULL )
                        free ( bs -> in_buffer );
                    if ( bs -> out_buffer != NULL )
                        free ( bs -> out_buffer );
                }

                free ( bs );
            }
        }

        * buffered = NULL;
    }

    return rc;
}
