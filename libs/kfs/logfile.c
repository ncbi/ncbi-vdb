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

struct LogFile;
#define KFILE_IMPL struct LogFile
#include <kfs/impl.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/time.h>

#include <kfs/defs.h>
#include <kfs/recorder.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <string.h>


typedef struct LogFile
{
    KFile dad;
    KFile * wrapped;
    struct Recorder * rec;
    bool timed;
} LogFile;


static rc_t CC LogFileDestroy ( LogFile * self )
{
    KFileRelease ( self -> wrapped );

    WriteToRecorder ( self -> rec, "D\n" );
    ReleaseRecorder ( self -> rec );

    free ( self );
    return 0;
}

/* for all the shim-functions self has already been tested to be not NULL! */

static struct KSysFile* LogFileGetSysFile ( const LogFile *self, uint64_t *offset )
{
    * offset = 0;
    WriteToRecorder ( self -> rec, "Y\n" );
    return NULL;
}


static rc_t LogFileRandomAccess ( const LogFile *self )
{
    rc_t rc = KFileRandomAccess_v1 ( self -> wrapped );
    WriteToRecorder ( self -> rec, "A\t%R\n", rc );
    return rc;
}

/* ---------------------------------------------------------------------------- */

static rc_t LogFileSize ( const LogFile *self, uint64_t *size )
{
    rc_t rc = KFileSize ( self -> wrapped, size );
    WriteToRecorder ( self -> rec, "S\t%lu\n", *size );
    return rc;
}

static rc_t LogFileSize_timed ( const LogFile *self, uint64_t *size )
{
    KTimeMs_t ms = KTimeMsStamp ();
    rc_t rc = KFileSize ( self -> wrapped, size );
    ms = KTimeMsStamp () - ms;
    WriteToRecorder ( self -> rec, "S\t%lu\t%lu\n", *size, ms );
    return rc;
}

/* ---------------------------------------------------------------------------- */

static rc_t LogFileSetSize ( LogFile *self, uint64_t size )
{
    rc_t rc = KFileSetSize ( self -> wrapped, size );
    WriteToRecorder ( self -> rec, "T\t%lu\n", size );
    return rc;
}

static rc_t LogFileSetSize_timed ( LogFile *self, uint64_t size )
{
    KTimeMs_t ms = KTimeMsStamp ();
    rc_t rc = KFileSetSize ( self -> wrapped, size );
    ms = KTimeMsStamp () - ms;
    WriteToRecorder ( self -> rec, "T\t%lu\t%lu\n", size, ms );
    return rc;
}

/* ---------------------------------------------------------------------------- */

static rc_t LogFileRead ( const LogFile *cself, uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = KFileRead ( cself -> wrapped, pos,  buffer, bsize, num_read );
    WriteToRecorder ( cself -> rec, "R\t%lu\t%lu\t%lu\n", pos, bsize, *num_read );
    return rc;
}

static rc_t LogFileRead_timed ( const LogFile *cself, uint64_t pos, void *buffer, size_t bsize, size_t *num_read )
{
    KTimeMs_t ms = KTimeMsStamp ();
    rc_t rc = KFileRead ( cself -> wrapped, pos,  buffer, bsize, num_read );
    ms = KTimeMsStamp () - ms;
    WriteToRecorder ( cself -> rec, "R\t%lu\t%lu\t%lu\t%lu\n", pos, bsize, *num_read, ms );
    return rc;
}

/* ---------------------------------------------------------------------------- */

static rc_t LogFileWrite ( LogFile *self, uint64_t pos, const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc = KFileWrite ( self -> wrapped, pos,  buffer, size, num_writ );
    WriteToRecorder ( self -> rec, "W\t%lu\t%lu\t%lu\n", pos, size, *num_writ );
    return rc;
}

static rc_t LogFileWrite_timed ( LogFile *self, uint64_t pos, const void *buffer, size_t size, size_t *num_writ )
{
    KTimeMs_t ms = KTimeMsStamp ();
    rc_t rc = KFileWrite ( self -> wrapped, pos,  buffer, size, num_writ );
    ms = KTimeMsStamp () - ms;
    WriteToRecorder ( self -> rec, "W\t%lu\t%lu\t%lu\t%lu\n", pos, size, *num_writ, ms );
    return rc;
}

/* ---------------------------------------------------------------------------- */

static KFile_vt_v1 vtLogFile =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    LogFileDestroy,
    LogFileGetSysFile,
    LogFileRandomAccess,
    LogFileSize,
    LogFileSetSize,
    LogFileRead,
    LogFileWrite
    /* end minor version 0 methods */
};

static KFile_vt_v1 vtLogFile_timed =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    LogFileDestroy,
    LogFileGetSysFile,
    LogFileRandomAccess,
    LogFileSize_timed,
    LogFileSetSize_timed,
    LogFileRead_timed,
    LogFileWrite_timed
    /* end minor version 0 methods */
};

LIB_EXPORT rc_t CC MakeLogFileV ( struct KDirectory * self,
                                  struct KFile const **log_file,
                                  struct KFile *to_wrap,
                                  bool append,
                                  bool timed,
                                  const char * path,
                                  va_list args )
{
    rc_t rc = 0;
        
    if ( log_file == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        *log_file = NULL;
        if ( self == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcSelf, rcNull );
        else if ( to_wrap == NULL || path == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    }

    if ( rc == 0 )
    {
        rc = KFileAddRef ( to_wrap );
        if ( rc == 0 )
        {
            struct Recorder * rec;
            rc = MakeVRecorder ( self, &rec, 4096, append, path, args );
            if ( rc == 0 )
            {
                LogFile * lf = malloc ( sizeof * lf );
                if ( lf == NULL )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    /* now we can enter everything into the rr - struct */
                    lf -> wrapped = to_wrap;
                    lf -> rec = rec;
                    lf -> timed = timed;
                    if ( timed )
                    {
                        rc = KFileInit ( &lf -> dad,
                                         ( const union KFile_vt * ) &vtLogFile_timed,
                                         "LogFile",
                                         "logfile",
                                         true,
                                         false );
                    }
                    else
                    {
                        rc = KFileInit ( &lf -> dad,
                                         ( const union KFile_vt * ) &vtLogFile,
                                         "LogFile",
                                         "logfile",
                                         true,
                                         false );
                    }
                    if ( rc != 0 )
                        free( ( void * ) lf );
                    else
                        *log_file = ( const KFile * ) &lf -> dad;

                    if ( rc != 0 )
                        ReleaseRecorder ( rec );
                }
            }
            if ( rc != 0 )
                KFileRelease ( to_wrap );
        }
    }
    
    if ( rc != 0 )
    {
        rc = KFileAddRef( to_wrap );
        if ( rc == 0 )
            *log_file = to_wrap;
    }
    return rc;
}

LIB_EXPORT rc_t CC MakeLogFile ( struct KDirectory * self,
                                 struct KFile const **log_file,
                                 struct KFile *to_wrap,
                                 bool append,
                                 bool timed,
                                 const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );
    rc = MakeLogFileV ( self, log_file, to_wrap, append, timed, path, args );
    va_end ( args );
    return rc;
}
