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


typedef struct KStreamFromFiles KStreamFromFiles;
#define KSTREAM_IMPL KStreamFromFiles

#include <kns/extern.h>
#include <kns/stream.h>
#include <kns/impl.h>
#include <kfs/file.h>
#include <kfs/impl.h>
#include <klib/rc.h>

#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>

#include "stream-priv.h"


/* the object structure for THIS implementation */
struct  KStreamFromFiles
{
    /* THIS MUST COME FIRST */
    KStream dad;

    uint64_t in_pos;
    uint64_t out_pos;

    const KFile *in;
    KFile *out;
};


/* the required methods */

static
rc_t CC KStreamFromFilesWhack ( KStreamFromFiles *self )
{
    KFileRelease ( self -> in );
    KFileRelease ( self -> out );
    free ( self );
    return 0;
}

static
rc_t CC KStreamFromFilesRead ( const KStreamFromFiles *cself,
    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = KFileRead ( cself -> in, cself -> in_pos, buffer, bsize, num_read );
    if ( rc == 0 )
    {
        KStreamFromFiles *self = ( KStreamFromFiles* ) cself;
        self -> in_pos += * num_read;
    }

    return rc;
}

static
rc_t CC KStreamFromFilesWrite ( KStreamFromFiles *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc = KFileWrite ( self -> out, self -> out_pos, buffer, size, num_writ );
    if ( rc == 0 )
        self -> out_pos += * num_writ;

    return rc;
}

/* the vtable */
static KStream_vt_v1 vtKStreamFromFiles =
{
    1, 0,
    KStreamFromFilesWhack,
    KStreamFromFilesRead,
    KStreamFromFilesWrite
};


/* FromKFilePair
 *  create a KStream from a pair of KFiles
 *  maintains a "pos" marker for input and output files
 *
 *  "strm" [ OUT ] - resultant KStream
 *
 *  "read" [ IN, NULL OKAY ] - file to use for stream reading
 *
 *  "write" [ IN, NULL OKAY ] - file to use for stream writing
 *
 * NB - EITHER "read" or "write" may be NULL, but not both.
 */
LIB_EXPORT rc_t CC KStreamFromKFilePair ( KStream **strm,
    const KFile *read, KFile *write )
{
    rc_t rc;

    if ( strm == NULL )
        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    else
    {
        bool can_read = ( read == NULL ) ? false : read -> read_enabled;
        bool can_write = ( write == NULL ) ? false : write -> write_enabled;

        if ( ! can_read && ! can_write )
            rc = RC ( rcNS, rcStream, rcConstructing, rcFile, rcNoPerm );
        else
        {
            KStreamFromFiles *sff = calloc ( 1, sizeof *sff );
            if ( sff == NULL )
                rc = RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KStreamInit ( & sff -> dad, ( const KStream_vt* ) & vtKStreamFromFiles, 
                                   "KStreamFromFilePair", "adapter", can_read, can_write );
                if ( rc == 0 )
                {
                    rc = KFileAddRef ( read );
                    if ( rc == 0 )
                    {
                        rc = KFileAddRef ( write );
                        if ( rc ==  0 )
                        {
                            sff -> in = read;
                            sff -> out = write;
                            *strm = & sff -> dad;

                            return 0;
                        }

                        KFileRelease ( read );
                    }
                }

                free ( sff );
            }
        }

        *strm = NULL;
    }

    return rc;
}
