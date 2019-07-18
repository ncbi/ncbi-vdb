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
 */

#include <kfs/extern.h>

struct CallBackFile;
#define KFILE_IMPL struct CallBackFile
#include <kfs/impl.h>

#include <klib/rc.h>
#include <kfs/defs.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>


typedef struct CallBackFile
{
    KFile dad;
    KFile * wrapped;
    void ( CC * cb ) ( char event, rc_t rc, uint64_t pos, size_t req_size, size_t done_size,
                       void *data1, void *data2 );
    void * data1;
    void * data2;
} CallBackFile;


static rc_t CC CallBackFileDestroy ( CallBackFile * self )
{
    KFileRelease ( self -> wrapped );
    free ( self );
    return 0;
}

/* ---------------------------------------------------------------------------- */

static struct KSysFile* CallBackFileGetSysFile ( const CallBackFile *self, uint64_t *offset )
{
    * offset = 0;
    self -> cb( 'Y', 0, 0, 0, 0, self -> data1, self -> data2 );    
    return NULL;
}

static rc_t CallBackFileRandomAccess ( const CallBackFile *self )
{
    rc_t rc = KFileRandomAccess_v1 ( self -> wrapped );
    self -> cb( 'A', rc, 0, 0, 0, self -> data1, self -> data2 );
    return rc;
}

static rc_t CallBackFileSize ( const CallBackFile *self, uint64_t *size )
{
    rc_t rc = KFileSize ( self -> wrapped, size );
    self -> cb( 'S', rc, 0, 0, *size, self -> data1, self -> data2 );
    return rc;
}

static rc_t CallBackFileSetSize ( CallBackFile *self, uint64_t size )
{
    rc_t rc = KFileSetSize ( self -> wrapped, size );
    self -> cb( 'E', rc, 0, size, size, self -> data1, self -> data2 );
    return rc;
}

static rc_t CallBackFileRead ( const CallBackFile *cself, uint64_t pos,
                                void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = KFileRead ( cself -> wrapped, pos,  buffer, bsize, num_read );
    cself -> cb( 'R', rc, pos, bsize, *num_read, cself -> data1, cself -> data2 );    
    return rc;
}

static rc_t CallBackFileWrite ( CallBackFile *self, uint64_t pos,
                                const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc = KFileWrite ( self -> wrapped, pos, buffer, size, num_writ );
    self -> cb( 'W', rc, pos, size, *num_writ, self -> data1, self -> data2 );
    return rc;
}

static uint32_t CC CallBackFileGetType ( const CallBackFile * self )
{
    uint32_t res = KFileType ( self -> wrapped );
    self -> cb( 'T', 0, res, 0, 0, self -> data1, self -> data2 );
    return res;
}

static rc_t CC CallBackFileTimedRead ( const CallBackFile *cself, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
    rc_t rc = KFileTimedRead( cself -> wrapped, pos, buffer, bsize, num_read, tm );
    cself -> cb( 'B', rc, pos, bsize, *num_read, cself -> data1, cself -> data2 );
    return rc;
}

static rc_t CC CallBackFileTimedWrite ( CallBackFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
    rc_t rc = KFileTimedWrite( self -> wrapped, pos, buffer, size, num_writ, tm );
    self -> cb( 'C', rc, pos, size, *num_writ, self -> data1, self -> data2 );
    return rc;
}

static rc_t CC CallBackFileReadChunked ( const CallBackFile *self, uint64_t pos,
    KChunkReader * chunks, size_t bsize, size_t * total_read )
{
    rc_t rc = KFileReadChunked( self -> wrapped, pos, chunks, bsize, total_read );
    self -> cb( 'D', rc, pos, bsize, *total_read, self -> data1, self -> data2 );
    return rc;
}

static rc_t CC CallBackFileTimedReadChunked ( const CallBackFile *self, uint64_t pos,
    KChunkReader * chunks, size_t bsize, size_t * total_read, struct timeout_t * tm )
{
    rc_t rc = KFileTimedReadChunked( self -> wrapped, pos, chunks, bsize, total_read, tm );
    self -> cb( 'F', rc, pos, bsize, *total_read, self -> data1, self -> data2 );    
    return rc;
}

/* ---------------------------------------------------------------------------- */
static KFile_vt_v1 vtCallBackFile =
{
    /* version 1.0 */
    1, 3,

    /* start minor version 0 methods */
    CallBackFileDestroy,
    CallBackFileGetSysFile,
    CallBackFileRandomAccess,
    CallBackFileSize,
    CallBackFileSetSize,
    CallBackFileRead,
    CallBackFileWrite,
    /* end minor version 0 methods */
    
    /* start minor version == 1 */
    CallBackFileGetType,
    /* end minor version == 1 */

    /* start minor version == 2 */
    CallBackFileTimedRead,
    CallBackFileTimedWrite,
    /* end minor version == 2 */

    /* start minor version == 3 */
    CallBackFileReadChunked,
    CallBackFileTimedReadChunked
    /* end minor version == 3 */
};

LIB_EXPORT rc_t CC MakeCallBackFile ( struct KFile **callback_file,
                    struct KFile *to_wrap,
                    void ( CC * cb ) ( char event, rc_t rc, uint64_t pos, size_t req_size, size_t done_size,
                                       void *data1, void *data2 ),
                    void * data1,
                    void * data2 )
{
    rc_t rc = 0;
        
    if ( callback_file == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *callback_file = NULL;
        if ( to_wrap == NULL || cb == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    }

    if ( rc == 0 )
    {
        rc = KFileAddRef ( to_wrap );
        if ( rc == 0 )
        {
            CallBackFile * lf = malloc ( sizeof * lf );
            if ( lf == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                /* now we can enter everything into the rr - struct */
                lf -> wrapped = to_wrap;
                lf -> cb = cb;
                lf -> data1 = data1;
                lf -> data2 = data2;
                rc = KFileInit ( &lf -> dad,
                                 ( const union KFile_vt * ) &vtCallBackFile,
                                 "CallBackFile",
                                 "callbackfile",
                                 true,
                                 false );
                if ( rc != 0 )
                    free( ( void * ) lf );
                else
                    *callback_file = ( KFile * ) &lf -> dad;
            }
        }
    }
    return rc;
}
