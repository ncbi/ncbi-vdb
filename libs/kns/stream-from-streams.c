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


typedef struct KStreamFromStreams KStreamFromStreams;
#define KSTREAM_IMPL KStreamFromStreams

#include <kns/extern.h>
#include <kns/stream.h>
#include <kns/impl.h>
#include <klib/rc.h>

#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>

#include "stream-priv.h"

/* the object structure for THIS implementation */
struct  KStreamFromStreams
{
    /* THIS MUST COME FIRST */
    KStream dad;

    const KStream *in;
    KStream *out;
};


/* the required methods */

static
rc_t CC KStreamFromStreamsWhack ( KStreamFromStreams *self )
{
    KStreamRelease ( self -> in );
    KStreamRelease ( self -> out );
    free ( self );
    return 0;
}

static
rc_t CC KStreamFromStreamsRead ( const KStreamFromStreams *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    return KStreamRead ( self -> in, buffer, bsize, num_read );
}

static
rc_t CC KStreamFromStreamsWrite ( KStreamFromStreams *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    return KStreamWrite ( self -> out, buffer, size, num_writ );
}

/* the vtable */
static KStream_vt_v1 vtKStreamFromStreams =
{
    1, 0,
    KStreamFromStreamsWhack,
    KStreamFromStreamsRead,
    KStreamFromStreamsWrite
};


/* FromKStreamPair
 *  create a KStream from a pair of KStreams
 *
 *  "strm" [ OUT ] - resultant KStream
 *
 *  "read" [ IN, NULL OKAY ] - stream to use for input
 *
 *  "write" [ IN, NULL OKAY ] - stream to use for output
 *
 * NB - EITHER "read" or "write" may be NULL, but not both.
 */
LIB_EXPORT rc_t CC KStreamFromKStreamPair ( KStream **strm,
    const KStream *read, KStream *write )
{
    rc_t rc;

    if ( strm == NULL )
        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    else
    {
        bool can_read = ( read == NULL ) ? false : read -> read_enabled;
        bool can_write = ( write == NULL ) ? false : write -> write_enabled;

        if ( ! can_read && ! can_write )
            rc = RC ( rcNS, rcStream, rcConstructing, rcStream, rcNoPerm );
        else
        {
            KStreamFromStreams *sfs = calloc ( 1, sizeof *sfs );
            if ( sfs == NULL )
                rc = RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KStreamInit ( & sfs -> dad, ( const KStream_vt* ) & vtKStreamFromStreams, 
                                   "KStreamFromStreamPair", "adapter", can_read, can_write );
                if ( rc == 0 )
                {
                    rc = KStreamAddRef ( read );
                    if ( rc == 0 )
                    {
                        rc = KStreamAddRef ( write );
                        if ( rc ==  0 )
                        {
                            sfs -> in = read;
                            sfs -> out = write;
                            *strm = & sfs -> dad;

                            return 0;
                        }

                        KStreamRelease ( read );
                    }
                }

                free ( sfs );
            }
        }

        *strm = NULL;
    }

    return rc;
}
