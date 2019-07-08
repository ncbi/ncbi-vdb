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

#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/text.h>
#include <klib/printf.h>

#include <kfs/defs.h>
#include <kfs/file.h>
#include <kfs/recorder.h>
#include <kfs/directory.h>
#include <kproc/queue.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <endian.h>

#include <stdio.h>

typedef struct Recorder
{
    KFile * f;
    uint64_t pos;
    KQueue * pool;
    size_t buflen;
} Recorder;

LIB_EXPORT rc_t CC MakeVRecorder ( struct KDirectory * self,
                                   struct Recorder ** recorder,
                                   size_t buflen,
                                   bool append,
                                   const char * path,
                                   va_list args )
{
    rc_t rc = 0;
    
    if ( recorder == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *recorder = NULL;
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcSelf, rcNull );
        else if ( path == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcPath, rcEmpty );
    }
    
    if ( rc == 0 )
    {
        KQueue * pool;
        rc = KQueueMake( &pool, 8 );
        if ( rc == 0 )
        {
            KFile * f;
            rc = KDirectoryVCreateFile ( self, &f, true, 0664, kcmOpen | kcmParents, path, args );
            if ( rc == 0 )
            {
                uint64_t fs = 0;
                if ( append )
                    rc = KFileSize ( f, &fs );
                else
                    rc = KFileSetSize ( f, 0 );
                if ( rc == 0 )
                {
                    Recorder * r = malloc ( sizeof * r );
                    if ( r == NULL )
                        rc = RC ( rcFS, rcFile, rcAllocating, rcMemory, rcExhausted );
                    else
                    {
                        r -> f = f;
                        r -> pos = fs;
                        r -> pool = pool;
                        if ( buflen == 0  )
                            r -> buflen = 1024;
                        else
                            r -> buflen = buflen;
                        *recorder = r;
                    }
                }
                if ( rc != 0 )
                    KFileRelease ( f );
            }
            if ( rc != 0 )
                KQueueRelease ( pool );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC MakeRecorder ( struct KDirectory * self,
                                  struct Recorder ** recorder,
                                  size_t buflen,
                                  bool append,
                                  const char * path,
                                  ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );
    rc = MakeVRecorder ( self, recorder, buflen, append, path, args );
    va_end ( args );
    return rc;
}


LIB_EXPORT rc_t CC ReleaseRecorder ( struct Recorder * self )
{
    if ( self != NULL )
    {
        if ( self -> f != NULL )
            KFileRelease ( self -> f );
            
        if ( self -> pool != NULL )
        {
            void * page;
            while ( KQueuePop( self -> pool, &page, NULL ) == 0 )
            {
                free( page );
            }
            KQueueRelease ( self -> pool );
        }
        
        free ( ( void * ) self );
    }
    return 0;
}

LIB_EXPORT rc_t CC WriteToVRecorder ( struct Recorder * self, const char * fmt, va_list args )
{
    rc_t rc;
    void * page;
    
    if ( KQueuePop ( self -> pool, ( void ** )&page, NULL ) != 0 )
        page = malloc ( self -> buflen );
    if ( page == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        size_t num_writ1;
        rc = string_vprintf ( page, self -> buflen, &num_writ1, fmt, args );
        if ( rc == 0 )
        {
            size_t num_writ2;
            rc = KFileWriteAll ( self -> f, self -> pos, ( const void * ) page, num_writ1, &num_writ2 );
            if ( rc == 0 )
                self -> pos += num_writ2;
        }
        if ( KQueuePush ( self -> pool, page, NULL ) != 0 )
            free ( page );
    }
    return rc;
}

LIB_EXPORT rc_t CC WriteToRecorder ( struct Recorder * self, const char * fmt, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, fmt );
    rc = WriteToVRecorder ( self, fmt, args );
    va_end ( args );
    return rc;
}
