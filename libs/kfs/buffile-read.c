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

typedef struct KBufReadFile KBufReadFile;
#define KFILE_IMPL KBufReadFile

#include <kfs/extern.h>

#include <kfs/file.h>
#include <kfs/impl.h>
#include <klib/debug.h>
#include <klib/status.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <kproc/lock.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>



/*-----------------------------------------------------------------------
 * KBufReadFile
 */
struct KBufReadFile
{
    KFile dad;

    volatile uint64_t pos;       /* position of buff within the original file */

    const KFile *f;     /* original file being buffered */
    KLock * lock;

    size_t bsize;       /* size of the buffer */
    volatile size_t num_valid;   /* how much of the buffer is actually valid */

    volatile uint8_t * buff;
};

static
rc_t CC KBufReadFileDestroy ( KBufReadFile *self )
{
    KFileRelease ( self -> f );
    KLockRelease ( self -> lock );
    free ( ( void * ) self -> buff );
    free ( ( void * ) self );
    return 0;
}

static
struct KSysFile* CC KBufReadFileSysFile ( const KBufReadFile *self, uint64_t *offset )
{
    /* does not support SysFile */
    * offset = 0;
    return NULL;
}

static
rc_t CC KBufReadFileRandomAccess ( const KBufReadFile *self )
{
    return KFileRandomAccess ( self -> f );
}

static
rc_t CC KBufReadFileSize ( const KBufReadFile *self, uint64_t *size )
{
    return KFileSize ( self -> f, size );
}

static
rc_t CC KBufReadFileSetSize ( KBufReadFile *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}

static
rc_t CC KBufReadFileTimedRead ( const KBufReadFile * cself, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
#if 0
    return KFileRead ( cself -> f, pos, buffer, bsize, num_read );
#else
    rc_t rc = 0;

    assert ( cself != NULL );
    assert ( buffer != NULL );
    assert ( num_read != NULL );

    /* start assuming nothing will be read */
    *num_read = 0;

    STATUS ( STAT_PRG, "request to read %zu bytes from buffered file %p at offset %lu"
             , bsize
             , cself
             , pos
        );

    if (bsize != 0)
    {

        /* cast might be a no-op */
        KBufReadFile * self = ( KBufReadFile * ) cself;

        rc = KLockAcquire ( self -> lock );
        if ( rc == 0 )
        {
            size_t new_offset = (size_t)(pos % cself->bsize);
            uint64_t new_pos = pos - new_offset;

            STATUS ( STAT_PRG, "buffer window starts at %lx, buffer offset is %zu", new_pos, new_offset );

            /* we need to read if we are on the wrong 'sector page'
             * or the current 'sector page' isn't long enough.
             *
             * If on the wrong page just kill the contents
             */
            if (new_pos != self->pos)
            {
                STATUS ( STAT_QA, "buffer window is invalid" );
                self->num_valid = 0;
                self->pos = new_pos;
            }

            /* now we agree on the 'sector page' even if we have nothing
             * valid in it */
            if ((self->num_valid == 0) || (self->num_valid <= (size_t)new_offset))
            {
                size_t new_num_read;

                STATUS ( STAT_PRG, "about to request %zu bytes from backing file %p at offset %lu"
                         , self->bsize - self->num_valid
                         , self -> f
                         , self->pos + self->num_valid
                    );
            
                if ( self -> f -> vt -> v1 . min < 2 )
                {
                    STATUS ( STAT_GEEK, "using v1.0 interface" );
                    rc = KFileReadAll (self->f, self->pos + self->num_valid,
                                       ( void * ) ( self->buff + self->num_valid ),
                                       self->bsize - self->num_valid,
                                       &new_num_read);
                }
                else
                {
                    STATUS ( STAT_GEEK, "using v1.2 interface" );
                    rc = KFileTimedReadAll (self->f, self->pos + self->num_valid,
                                            ( void * ) ( self->buff + self->num_valid ),
                                            self->bsize - self->num_valid,
                                            &new_num_read, tm);
                }
                if (rc == 0)
                {
                    STATUS ( STAT_PRG, "read %zu bytes from backing", new_num_read );
                    self->num_valid += new_num_read;
                }
            }

            /* now we have all we're gonna get this time */
            if (new_offset < self->num_valid)
            {
                size_t to_copy;

                to_copy = self->num_valid - new_offset;
                if (to_copy > bsize)
                    to_copy = bsize;
                else
                {
                    STATUS ( STAT_QA, "limiting read from %zu to %zu bytes available in buffer"
                             , bsize
                             , to_copy
                        );
                }

                STATUS ( STAT_PRG, "copying %zu bytes to caller buffer from file buffer offset %zu"
                         , to_copy
                         , new_offset
                    );
            
                memmove (buffer, ( const void * ) ( self->buff + new_offset ), to_copy);
                *num_read = to_copy;
            }

            KLockUnlock ( self -> lock );
        }
    }
    
    return rc;
#endif
}

static
rc_t CC KBufReadFileRead ( const KBufReadFile * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read )
{
    return KBufReadFileTimedRead ( self, pos, buffer, bsize, num_read, NULL );
}

static
rc_t CC KBufReadFileTimedWrite ( KBufReadFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t * tm )
{
    return RC ( rcFS, rcFile, rcWriting, rcFunction, rcUnsupported );
}

static
rc_t CC KBufReadFileWrite ( KBufReadFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcFS, rcFile, rcWriting, rcFunction, rcUnsupported );
}

static
uint32_t CC KBufReadFileType ( const KBufReadFile * self )
{
    return KFileType ( self -> f );
}

static
rc_t KBufReadFileMake ( KBufReadFile ** bp, const KFile *f, size_t bsize,
    const KFile_vt *vt, bool read_enabled, bool write_enabled )
{
    rc_t rc;

    KBufReadFile *bf = calloc ( sizeof * bf, 1 );
    if ( bf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        bf -> buff = malloc ( bsize );
        if ( bf -> buff == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KFileInit ( & bf -> dad, vt, "KBufReadFile", "no-name", read_enabled, write_enabled );
            if ( rc == 0 )
            {
                rc = KLockMake ( & bf -> lock );
                if ( rc == 0 )
                {
                    rc = KFileAddRef ( f );
                    if ( rc == 0 )
                    {
                        bf -> f = ( KFile* ) f;
                        bf -> bsize = bsize;
                        * bp = bf;
                        return 0;
                    }

                    KLockRelease ( bf -> lock );
                }
            }
            
            free ( ( void * ) bf -> buff );
        }

        free ( ( void * ) bf );
    }

    return rc;
}

/* MakeBufferedWrite
 *  make a writable file buffer
 *
 *  "buf" [ OUT ] - return parameter for new buffered file
 *
 *  "original" [ IN ] - source file to be buffered. must have read access
 *
 *  "bsize" [ IN ] - buffer size
 */

static
const KFile_vt_v1 vtKBufReadFileRandR_v1 =
{
    /* version */
    1, 2,

    /* 1.0 */
    KBufReadFileDestroy,
    KBufReadFileSysFile,
    KBufReadFileRandomAccess,
    KBufReadFileSize,
    KBufReadFileSetSize,
    KBufReadFileRead,
    KBufReadFileWrite,

    /* 1.1 */
    KBufReadFileType,

    /* 1.2 */
    KBufReadFileTimedRead,
    KBufReadFileTimedWrite
};

LIB_EXPORT 
rc_t CC KBufReadFileMakeRead ( const KFile ** bp, const KFile * original, size_t bsize )
{
    rc_t rc;

    if ( bp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( original == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! original -> read_enabled )
        {
            if ( original -> write_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcWriteonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else
        {
            uint64_t eof;
            rc = KFileSize ( original, & eof );
            if ( rc == 0 )
            {
                KBufReadFile *buf;

                if ( ( uint64_t ) bsize > eof )
                    bsize = ( size_t ) eof;
                
                rc = KBufReadFileMake ( & buf, original, bsize,
                    ( const KFile_vt* ) & vtKBufReadFileRandR_v1, true, false );
                if ( rc == 0 )
                {
                    * bp = & buf -> dad;
                    return 0;
                }
            }
        }

        * bp = NULL;
    }

    return rc;
}
