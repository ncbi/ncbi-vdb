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

typedef struct KBufWriteFile KBufWriteFile;
#define KFILE_IMPL KBufWriteFile

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
 * KBufWriteFile
 */
struct KBufWriteFile
{
    KFile dad;

    uint64_t pos;

    KFile *f;

    size_t bsize;
    size_t num_valid;

    uint8_t buff [ 1 ];
};


/*
static
rc_t CC KBufWriteFileDestroy ( KBufWriteFile *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
        free ( self );
    return rc;
}
*/

static
rc_t KBufWriteFileFlush ( KBufWriteFile *self, uint64_t dpos )
{
    rc_t rc;
    size_t total_writ, partial;

    /* flush buffer */
    for ( rc = 0, total_writ = 0; total_writ < self -> num_valid; total_writ += partial )
    {
        rc = KFileWrite ( self -> f, dpos + total_writ,
            & self -> buff [ total_writ ],
            self -> num_valid - total_writ, & partial );
        if ( rc != 0 )
            break;
        if ( partial == 0 )
        {
            rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcIncomplete );
            break;
        }
    }

    if ( rc == 0 )
        self -> num_valid = 0;

    return rc;
}

static
rc_t CC KBufWriteFileFlushDestroy ( KBufWriteFile *self )
{
    rc_t rc = KBufWriteFileFlush ( self, self -> pos );
    if ( rc == 0 )
        rc = KFileRelease ( self -> f );
    if ( rc == 0 )
        free ( self );
    return rc;
}

static
struct KSysFile* CC KBufWriteFileNoSysFile ( const KBufWriteFile *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}

static
rc_t CC KBufWriteFileRandomAccess ( const KBufWriteFile *self )
{
    return KFileRandomAccess ( self -> f );
}

static
rc_t CC KBufWriteFileSize ( const KBufWriteFile *self, uint64_t *size )
{
    return KFileSize ( self -> f, size );
}

static
rc_t CC KBufWriteFileSetSize ( KBufWriteFile *self, uint64_t size )
{
    if ( self -> pos + self -> num_valid > size )
    {
        if ( self -> pos < size )
        {
            size_t total, num_writ, to_write = ( size_t ) ( size - self -> pos );
            for ( total = 0; total < to_write; total += num_writ )
            {
                rc_t rc = KFileWrite ( self -> f, self -> pos + total,
                    & self -> buff [ total ], to_write - total, & num_writ );
                if ( rc != 0 )
                    return rc;
                if ( num_writ == 0 )
                    return RC ( rcFS, rcFile, rcReading, rcTransfer, rcIncomplete );
            }
        }

        self -> pos = 0;
        self -> num_valid = 0;
    }

    return KFileSetSize ( self -> f, size );
}

static
rc_t CC KBufWriteFileNoRead ( const KBufWriteFile *cself, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    return RC ( rcFS, rcFile, rcReading, rcFunction, rcUnsupported );
}

static
rc_t CC KBufWriteFileWrite ( KBufWriteFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc;
    size_t total, partial, trailing;

    const uint8_t *bbuff = buffer;
    uint64_t send = pos + size;

    /* case 1: empty write */
    if ( size == 0 )
    {
        * num_writ = 0;
        return 0;
    }

    /* perform write */
    for ( rc = 0, total = trailing = 0; total < size; total += partial )
    {
        size_t boff = total - trailing;
        uint64_t dpos = self -> pos;
        uint64_t dend = self -> pos + self -> num_valid;
        uint64_t dlim = self -> pos + self -> bsize;
        uint64_t spos = pos + boff;

        /* case 2: left within buffer */
        if ( dpos <= spos && spos <= dend && spos < dlim )
        {
            partial = ( size_t ) ( ( dlim < send ? dlim : send ) - spos );
            memmove ( & self -> buff [ spos - dpos ], & bbuff [ boff ], partial );
            if ( spos + partial > dend )
            {
                dend = spos + partial;
                self -> num_valid = ( size_t ) ( dend - dpos );
            }
        }

        /* case 3: right within buffer */
        else if ( spos < dpos && dpos < send && send <= dlim )
        {
            partial = ( size_t ) ( send - dpos );
            memmove ( & self -> buff [ spos - dpos ], & bbuff [ boff ], partial );
            if ( send > dend )
            {
                dend = send;
                self -> num_valid = ( size_t ) ( dend - dpos );
            }

            /* adjust other components for trailing copy */
            assert ( trailing == 0 );
            trailing = partial;
            send = dpos;
        }

        /* case 4: no intersection */
        else if ( send <= dpos || dend <= spos )
        {
            /* flush buffer as-is */
            rc = KBufWriteFileFlush ( self, dpos );
            if ( rc != 0 )
                break;

            /* empty buffer centered on this write */
            self -> pos = spos;

            /* if write is >= buffer size, just write it directy */
            partial = size - total;
            if ( partial < self -> bsize )
            {
                memmove ( self -> buff, & bbuff [ boff ], partial );
                self -> num_valid = partial;
            }
            else
            {
                rc = KFileWrite ( self -> f, spos, & bbuff [ boff ], partial, & partial );
                if ( rc != 0 )
                    break;
                if ( partial == 0 )
                {
                    rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcIncomplete );
                    break;
                }
            }
        }

        /* case 5: completely engulfs */
        else
        {
            assert ( spos < dpos && dlim < send );
            rc = KFileWrite ( self -> f, spos, & bbuff [ boff ], ( size_t ) ( send - spos ), & partial );
            if ( rc != 0 )
                break;
            if ( partial == 0 )
            {
                rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcIncomplete );
                break;
            }

            /* anything in the buffer is invalid now */
            self -> num_valid = 0;
        }
    }

    * num_writ = total;

    return rc;
}

static
uint32_t CC KBufWriteFileType ( const KBufWriteFile * self )
{
    return KFileType ( self -> f );
}

static
rc_t KBufWriteFileMake ( KBufWriteFile ** bp, const KFile *f, size_t bsize,
    const KFile_vt *vt, bool read_enabled, bool write_enabled )
{
    rc_t rc;

    KBufWriteFile *buf = malloc ( sizeof * buf - 1 + bsize );
    if ( buf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        memset ( buf, 0, sizeof * buf );
        rc = KFileInit ( & buf -> dad, vt, "KBufWriteFile", "no-name", read_enabled, write_enabled );
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
const KFile_vt_v1 vtKBufWriteFileRandWR_v1 =
{
    /* version */
    1, 1,

    /* 1.0 */
    KBufWriteFileFlushDestroy,
    KBufWriteFileNoSysFile,
    KBufWriteFileRandomAccess,
    KBufWriteFileSize,
    KBufWriteFileSetSize,
    KBufWriteFileNoRead,
    KBufWriteFileWrite,

    /* 1.1 */
    KBufWriteFileType
};

LIB_EXPORT rc_t KBufWriteFileMakeWrite ( KFile ** bp, KFile * original, size_t bsize )
{
    rc_t rc;

    if ( bp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( original == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! original -> write_enabled )
        {
            if ( original -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else
        {
            KBufWriteFile *buf;
            rc = KBufWriteFileMake ( & buf, original, bsize,
                ( const KFile_vt* ) & vtKBufWriteFileRandWR_v1, false, true );
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
