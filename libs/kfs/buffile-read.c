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
#include <klib/log.h>
#include <klib/rc.h>
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

    uint64_t pos;       /* position of buff within the original file */

    const KFile *f;           /* original file being buffered */

    size_t bsize;       /* size of the buffer */
    size_t num_valid;   /* how much of the buffer is actually valid */

    uint8_t buff [ 1 ];
};

static
rc_t CC KBufReadFileDestroy ( KBufReadFile *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
        free ( self );
    return rc;
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
rc_t CC KBufReadFileRead ( const KBufReadFile *cself, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc = 0;

    assert (cself);
    assert (buffer);
    assert (num_read);

    /* start assuming nothing will be read */
    *num_read = 0;

    if (bsize != 0)
    {
        KBufReadFile * self = (KBufReadFile *)cself;
        uint64_t new_pos; 
        size_t new_offset;

        /* cast might be a no-op */
        new_offset = (size_t)(pos % cself->bsize);
        new_pos = pos - new_offset;

        /* we need to read if we are on the wrong 'sector page'
         * or the current 'sector page' isn't long enough.
         *
         * If on the wrong page just kill the contents
         */
        if (new_pos != cself->pos)
        {
            self->num_valid = 0;
            self->pos = new_pos;
        }

        /* new we agree on the 'sector page' even if we have nothing
         * valid in it */
        if ((self->num_valid == 0) || (self->num_valid <= (size_t)new_offset))
        {
            size_t new_num_read;
            rc = KFileReadAll (self->f, self->pos + self->num_valid,
                               self->buff + self->num_valid, 
                               self->bsize - self->num_valid,
                               &new_num_read);
            if (rc == 0)
                self->num_valid += new_num_read;
        }

        /* now we have all we're gonna get this time */
        if (new_offset < self->num_valid)
        {
            size_t to_copy;

            to_copy = self->num_valid - new_offset;
            if (to_copy > bsize)
                to_copy = bsize;
            
            memmove (buffer, self->buff + new_offset, to_copy);
            *num_read = to_copy;
        }
    }
    return rc;
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

    KBufReadFile *buf = calloc ( sizeof * buf - 1 + bsize, 1 );
    if ( buf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KFileInit ( & buf -> dad, vt, "KBufReadFile", "no-name", read_enabled, write_enabled );
        if ( rc == 0 )
        {
            rc = KFileAddRef ( f );
            if ( rc == 0 )
            {
                buf -> f = ( KFile* ) f;
                buf -> bsize = bsize;
                * bp = buf;
                return 0;
            }
        }

        free ( buf );
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
const KFile_vt_v1 vtKBufReadFileRandWR_v1 =
{
    /* version */
    1, 1,

    /* 1.0 */
    KBufReadFileDestroy,
    KBufReadFileSysFile,
    KBufReadFileRandomAccess,
    KBufReadFileSize,
    KBufReadFileSetSize,
    KBufReadFileRead,
    KBufReadFileWrite,

    /* 1.1 */
    KBufReadFileType
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
            KBufReadFile *buf;
            rc = KBufReadFileMake ( & buf, original, bsize,
                ( const KFile_vt* ) & vtKBufReadFileRandWR_v1, true, false );
            if ( rc == 0 )
            {
                * bp = & buf -> dad;
                return 0;
            }
        }

        * bp = NULL;
    }

    return rc;
}
