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

/*--------------------------------------------------------------------------
 * forwards
 */
struct KSysFile_v1;
#define KFILE_IMPL struct KSysFile_v1

#include <kfs/extern.h>
#include "sysfile-priv.h"
#include <kfs/kfs-priv.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <sysalloc.h>


#ifndef __USE_UNIX98
#define __USE_UNIX98 1
#endif
#include <unistd.h>

#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>

#ifdef _DEBUGGING
#define SYSDEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_SYS),msg)
#define POS_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_POS),msg)
#else
#define SYSDEBUG(msg)
#define POS_DEBUG(msg)
#endif

/*--------------------------------------------------------------------------
 * KSysFile
 *  a Unix file
 */


#if USE_TIMEOUT
/* default timeout amount 3 seconds is 10,000 milliseconds */
#define TIMEOUT_MSECS (3 * 1000)

int32_t KSysFileTimeout = TIMEOUT_MSECS;

enum
{
    select_read = 1,
    select_write = 2,
    select_exception = 4
} select_operations;

static
rc_t KSysFileSelect_v1 (const KSysFile_v1* self, uint32_t operations)
{
    rc_t rc = 0;

    SYSDEBUG(( "%s: Enter (%p, %x)\n", __func__, self, operations));


    if (self->use_to)
    {
        fd_set fdsread;
        fd_set fdswrite;
        fd_set fdsexcept;
        struct timeval to;
        int selected;

        fdsread = self->fds;
        fdswrite = self->fds;
        fdsexcept = self->fds;

        to = self->to;

        for (rc = 0; rc == 0; ) 
        {
            SYSDEBUG(( "%s: call select\n", __func__));

            selected = select (self->fd + 1, 
                               (operations & select_read) ? &fdsread : NULL,
                               (operations & select_write) ? &fdswrite : NULL,
                               (operations & select_exception) ? &fdsexcept : NULL,
                               &to);
            SYSDEBUG(( "%s: select returned '%d' \n", __func__, selected));

            if (selected == 0)
            {

                rc = RC (rcFS, rcFile, rcSelecting, rcTimeout, rcDone);
            }
            else if (selected < 0)
            {
                int lerrno = errno;

                switch (lerrno)
                {
                case EINVAL:
                    rc = RC (rcFS, rcFile, rcSelecting, rcTimeout, rcInvalid);
                    LOGERR (klogInt, rc, "coding error bad timeout or negative nfds");
                    break;

                case EBADF:
                    rc = RC ( rcFS, rcFile, rcSelecting, rcFileDesc, rcInvalid );
                    PLOGERR (klogErr,
                             (klogErr, rc, "system bad file descriptor error fd='$(E)'",
                              "E=%d", self->fd));
                    break;

                case EINTR:/*  A signal was caught. */
                    continue;

                case ENOMEM:
                    rc = RC (rcFS, rcFile, rcSelecting, rcMemory, rcExhausted);
                    LOGERR (klogErr, rc, "out of memory setting up read timeout");
                    break;
                }
                /* function documentation seems to show that unless EINTR was 
                 * set we can now do our read
                 */
            }
            else
            {
                assert (FD_ISSET (self->fd, &fdsread) ||
                        FD_ISSET (self->fd, &fdswrite) ||
                        FD_ISSET (self->fd, &fdsexcept));

                return 0;
            }
        }
    }
    return rc;
}
#endif


/* Destroy
 */
static
rc_t KSysFileDestroy_v1 ( KSysFile_v1 *self )
{
    rc_t rc = 0;
    int lerrno;

    if ( close ( self -> fd ) != 0 ) switch ( lerrno = errno )
    {
    case EBADF:
        rc = RC ( rcFS, rcFile, rcDestroying, rcFunction, rcIncomplete );
        PLOGERR (klogInt,
                 (klogInt, rc, "system bad file descriptor error '$(F)'",
                  "F=%d", self->fd));
        break;

    case EINTR:
        rc = RC ( rcFS, rcFile, rcDestroying, rcFunction, rcIncomplete );
        LOGERR (klogErr, rc, "system interrupted function error");
        return rc;

    default:
        rc = RC ( rcFS, rcFile, rcDestroying, rcNoObj, rcUnknown );
        PLOGERR (klogErr,
                 (klogErr, rc, "unknown system error '$(S)($(E))'",
                 "S=%!,E=%d", lerrno, lerrno));
        return rc;
    }

    free ( self );
    return 0;
}

/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */
static
KSysFile_v1 * KSysFileGetSysFile_v1 ( const KSysFile_v1 *self, uint64_t *offset )
{
    * offset = 0;
    return ( KSysFile_v1 * ) self;
}

/* RandomAccess
 *  ALMOST by definition, the file is random access
 *  certain file types ( notably compressors ) will refuse random access
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t KSysFileRandomAccess_v1 ( const KSysFile_v1 *self )
{
    struct stat st;
    rc_t rc = 0;
    int lerrno;
    if ( fstat ( self -> fd, & st ) != 0 ) switch ( lerrno = errno )
    {
    case EBADF:
        rc = RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcInvalid );
        PLOGERR (klogErr,
                 (klogErr, rc, "system bad file descriptor error fd='$(F)'",
                  "F=%d", self->fd));
        return rc;

    default:
        rc = RC ( rcFS, rcFile, rcAccessing, rcNoObj, rcUnknown );
        PLOGERR (klogErr,
                 (klogErr, rc, "unknown system error '$(F)($(E))'",
                  "F=%!,E=%d", lerrno, lerrno));
        return rc;
    }

    if ( ! S_ISREG ( st . st_mode ) )
    {
        rc = RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
        /* not a deadly error so don't log now         
        LOGERR (klogErr, (klogErr, rc, ""));
        */
        return rc;
    }
    return 0;
}


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t KSysFileType_v1 ( const KSysFile_v1 *self )
{
    struct stat st;

    if ( fstat ( self -> fd, & st ) != 0 )
        return kfdInvalid;

    if ( ! S_ISREG ( st . st_mode ) )
    {
        if ( S_ISCHR ( st . st_mode ) )
            return kfdCharDev;
        if ( S_ISBLK ( st . st_mode ) )
            return kfdBlockDev;
        if ( S_ISFIFO ( st . st_mode ) )
            return kfdFIFO;
        if ( S_ISSOCK ( st . st_mode ) )
            return kfdSocket;
    }

    return kfdFile;
}


/* Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t KSysFileSize_v1 ( const KSysFile_v1 *self, uint64_t *size )
{
    struct stat st;
    rc_t rc = 0;
    int lerrno;

    if ( fstat ( self -> fd, & st ) != 0 ) switch ( lerrno = errno )
    {
    case EBADF:
        rc = RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcInvalid );
        PLOGERR (klogInt,
                 (klogInt, rc, "system bad file descriptor error fd='$(F)'",
                  "F=%d", self->fd));
        return rc;
    default:
        rc = RC ( rcFS, rcFile, rcAccessing, rcNoObj, rcUnknown );
        PLOGERR (klogErr,
                 (klogErr, rc, "unknown system error '$(F)($(E))'",
                  "F=%!,E=%d", lerrno, lerrno));
        return rc;
    }

    if ( S_ISDIR ( st . st_mode ) )
/*         return RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect ); */
        return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );

    * size = st . st_size;

    return 0;
}

/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t KSysFileSetSize_v1 ( KSysFile_v1 *self, uint64_t size )
{
    rc_t rc;
    int lerrno;

    if ( ftruncate ( self -> fd, size ) != 0 ) switch ( lerrno = errno )
    {
    case EBADF:
        rc = RC ( rcFS, rcFile, rcUpdating, rcFileDesc, rcInvalid );
        PLOGERR (klogInt,
                (klogInt, rc, "system bad file descriptor error fd='$(E)'",
                 "E=%d", self->fd));
        return rc;
    default:
        rc = RC ( rcFS, rcFile, rcUpdating, rcNoObj, rcUnknown );
        PLOGERR (klogErr,
                 (klogErr, rc, "unknown system error '$(S)($(E))'",
                  "S=%!,E=%d", lerrno, lerrno));
        return rc;
    }

    return 0;
}

/* Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually read
 */
static
rc_t KSysFileRead_v1 ( const KSysFile_v1 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;

    assert ( self != NULL );
    assert (num_read != NULL);

    *num_read = 0;

    while ( 1 )
    {
        ssize_t count;
        int lerrno;

#if USE_TIMEOUT
        rc = KSysFileSelect_v1 ( self, select_read | select_exception );
        if (rc)
            return rc;
#endif

#ifdef _DEBUGGING
        {
            POS_DEBUG( ( "%s,%lu,%lu\n", KDbgGetColName(), pos, bsize ) );
        }
#endif

        count = pread ( self -> fd, buffer, bsize, pos );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case EINTR:
            continue;
        case EIO:
            rc = RC ( rcFS, rcFile, rcReading, rcTransfer, rcUnknown );
            LOGERR (klogErr, rc, "system I/O error - likely broken pipe");
            return rc;

        case EBADF:
            rc = RC ( rcFS, rcFile, rcReading, rcFileDesc, rcInvalid );
            PLOGERR (klogErr,
                     (klogErr, rc, "system bad file descriptor error fd='$(E)'",
                      "E=%d", self->fd));
            return rc;

        case EISDIR:
            rc = RC ( rcFS, rcFile, rcReading, rcFileDesc, rcIncorrect );
            LOGERR (klogErr, rc, "system misuse of a directory error");
            return rc;

        case EINVAL:
            rc = RC ( rcFS, rcFile, rcReading, rcParam, rcInvalid );
            LOGERR (klogErr, rc, "system invalid argument error");
            return rc;

        default:
            rc = RC ( rcFS, rcFile, rcReading, rcNoObj, rcUnknown );
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown system error '$(S)($(E))'",
                      "S=%!,E=%d", lerrno, lerrno));
            return rc;
        }

        assert ( num_read != NULL );
        * num_read = count;
        break;
    }

    return 0;
}

/* Write
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 */
static
rc_t KSysFileWrite_v1 ( KSysFile_v1 *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ)
{
    assert ( self != NULL );
    while ( 1 )
    {
        rc_t rc;
        int lerrno;
        ssize_t count;

#if USE_TIMEOUT
        rc = KSysFileSelect_v1 (self, select_write | select_exception);
        if (rc)
            return rc;
#endif

        count = pwrite ( self -> fd, buffer, size, pos );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case ENOSPC:
            rc = RC ( rcFS, rcFile, rcWriting, rcStorage, rcExhausted );
            PLOGERR (klogErr,
                     (klogErr, rc, "system bad file descriptor error fd='$(E)'",
                      "E=%d", self->fd));
            return rc;

        case EINTR:
            continue;

        case EFBIG:
            rc = RC ( rcFS, rcFile, rcWriting, rcFile, rcExcessive );
            LOGERR (klogErr, rc, "system file too large error");
            return rc;

        case EIO:
            rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcUnknown );
            LOGERR (klogErr, rc, "system I/O error");
            return rc;

        case EBADF:
            rc = RC ( rcFS, rcFile, rcWriting, rcFileDesc, rcInvalid );
            PLOGERR (klogErr,
                     (klogErr, rc, "system bad file descriptor error fd='$(E)'",
                      "E=%d", self->fd));
            return rc;

        case EISDIR:
            rc = RC ( rcFS, rcFile, rcWriting, rcFileDesc, rcIncorrect );
            LOGERR (klogErr, rc, "system misuse of a directory error");
            return rc;

        case EINVAL:
            rc = RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );
            LOGERR (klogErr, rc, "system invalid argument error");
            return rc;

        default:
            rc = RC ( rcFS, rcFile, rcWriting, rcNoObj, rcUnknown );
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown system error errno='$(S)($(E))'",
                      "S=%!,E=%d", lerrno, lerrno));
            return rc;
        }

        assert ( num_writ != NULL );
        * num_writ = count;
        break;
    }

    return 0;
}


/* Make
 *  create a new file object
 *  from file descriptor
 */
static KFile_vt_v1 vtKSysFile =
{
    /* version 1.1 */
    1, 1,

    /* start minor version 0 methods */
    KSysFileDestroy_v1,
    KSysFileGetSysFile_v1,
    KSysFileRandomAccess_v1,
    KSysFileSize_v1,
    KSysFileSetSize_v1,
    KSysFileRead_v1,
    KSysFileWrite_v1,
    /* end minor version 0 methods */

    /* start minor version == 1 */
    KSysFileType_v1
    /* end minor version == 1 */
};

static
rc_t KSysFileMakeVT ( KSysFile_v1 **fp, int fd, const KFile_vt *vt,
    const char *path, bool read_enabled, bool write_enabled )
{
    rc_t rc;
    KSysFile_v1 *f;

    if ( fd < 0 )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcInvalid );
        PLOGERR (klogInt,
                 (klogInt, rc, "invalid file descriptor $(F)",
                  "F=%d", fd));
        return rc;
    }

    f = calloc ( sizeof * f, 1 );
    if ( f == NULL )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
        LOGERR (klogErr, rc, "out of memory");
        return rc;
    }
    else
    {
#if USE_TIMEOUT
        if (KSysFileTimeout >= 0)
        {
            f->to.tv_sec = KSysFileTimeout / 1000;
            f->to.tv_usec = (KSysFileTimeout % 1000) * 1000;
            f->use_to = true;
            FD_ZERO (&f->fds);
            FD_SET (fd, &f->fds);
        }
#endif

        rc = KFileInit_v1( & f -> dad, vt, "KSysFile", path, read_enabled, write_enabled );
        if ( rc == 0 )
        {
            f -> fd = fd;
            * fp = f;
            return 0;
        }

        free ( f );
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * KFile
 *  Unix-specific standard i/o interfaces
 */

typedef struct KStdIOFile KStdIOFile;
struct KStdIOFile
{
    KSysFile_v1 dad;
    uint64_t pos;
};


/* Destroy
 *  does not close fd
 */
static
rc_t KStdIOFileDestroy ( KSysFile_v1*self )
{
    free ( self );
    return 0;
}

static KFile_vt_v1 vtKStdIOFile =
{
    /* version 1.1 */
    1, 1,

    /* start minor version 0 methods */
    KStdIOFileDestroy,
    KSysFileGetSysFile_v1,
    KSysFileRandomAccess_v1,
    KSysFileSize_v1,
    KSysFileSetSize_v1,
    KSysFileRead_v1,
    KSysFileWrite_v1,
    /* end minor version 0 methods */

    /* start minor version == 1 */
    KSysFileType_v1
    /* end minor version == 1 */
};

/* RandomAccess
 */
static
rc_t KStdIOFileRandomAccess ( const KSysFile_v1 *self )
{
/*     return RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect ); */
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* Size
 */
static
rc_t KStdIOFileSize ( const KSysFile_v1 *self, uint64_t *size )
{
    * size = 0;

/*     return RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect ); */
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}

/* SetSize
 */
static
rc_t KStdIOFileSetSize ( KSysFile_v1 *self, uint64_t size )
{
/*     return RC ( rcFS, rcFile, rcUpdating, rcFileDesc, rcIncorrect ); */
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}

/* Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually read
 */
static
rc_t KStdIOFileRead ( const KSysFile_v1 *dad, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;

    KStdIOFile *self = ( KStdIOFile* ) dad;
    assert ( self != NULL );

    if ( self -> pos != pos )
    {
        rc = RC ( rcFS, rcFile, rcReading, rcParam, rcInvalid );
        PLOGERR (klogErr,
                 (klogErr, rc, "Bad position for STDIO read $(P) instead of $(O)",
                  "P=%lu,O=%lu", pos, self->pos));
        return rc;
    }

    while ( 1 )
    {
        ssize_t count;
        int lerrno;

#if USE_TIMEOUT
        rc = KSysFileSelect_v1 (&self->dad, select_read | select_exception);
        if (rc)
            return rc;
#endif

        count = read ( self -> dad . fd, buffer, bsize );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case EINTR:
            continue;

        case EIO:
            rc = RC ( rcFS, rcFile, rcReading, rcTransfer, rcUnknown );
            LOGERR (klogErr, rc, "system I/O error - broken pipe");
            return rc;

        case EBADF:
            rc = RC ( rcFS, rcFile, rcReading, rcFileDesc, rcInvalid );
            PLOGERR (klogInt,
                     (klogInt, rc, "system bad file descriptor fd='$(E)'",
                      "E=%d", self->dad.fd));
            return rc;

        case EISDIR:
            rc = RC ( rcFS, rcFile, rcReading, rcFileDesc, rcIncorrect );
            LOGERR (klogInt, rc, "system misuse of a directory error");
            return rc;

        case EINVAL:
            rc = RC ( rcFS, rcFile, rcReading, rcParam, rcInvalid );
            LOGERR (klogErr, rc, "system invalid argument error");
            return rc;

        default:
            rc = RC ( rcFS, rcFile, rcReading, rcNoObj, rcUnknown );
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown system error '$(S)($(E))'",
                      "S=%!,E=%d", lerrno, lerrno));
            return rc;
        }

        assert ( num_read != NULL );
        * num_read = count;
        self -> pos += count;
        break;
    }

    return 0;
}

/* Write
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 */
static
rc_t KStdIOFileWrite ( KSysFile_v1 *dad, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ)
{
    rc_t rc;
    KStdIOFile *self = ( KStdIOFile* ) dad;
    assert ( self != NULL );

    if ( self -> pos != pos )
    {
        rc = RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );
        PLOGERR (klogErr,
                 (klogErr, rc, "Bad position for STDIO write $(P) instead of $(O)",
                  "P=%lu,O=%lu", pos, self->pos));
        return rc;
    }

    while ( 1 )
    {
        int lerrno;
        ssize_t count;

#if USE_TIMEOUT
        rc = KSysFileSelect_v1 ( &self->dad, select_read | select_exception);
        if (rc)
            return rc;
#endif

        count = write ( self -> dad . fd, buffer, size );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case ENOSPC:
            rc = RC ( rcFS, rcFile, rcWriting, rcStorage, rcExhausted );
            LOGERR (klogErr, rc, "system device full error");
            return rc;

        case EINTR:
            continue;

        case EFBIG:
            rc = RC ( rcFS, rcFile, rcWriting, rcFile, rcExcessive );
            LOGERR (klogErr, rc, "system file too big error");
            return rc;

        case EIO:
            rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcUnknown );
            LOGERR (klogErr, rc, "system I/O error - broken pipe");
            return rc;
            
        case EBADF:
            rc = RC ( rcFS, rcFile, rcWriting, rcFileDesc, rcInvalid );
            PLOGERR (klogInt,
                     (klogInt, rc, "system bad file descriptor error fd=$(F)",
                      "F=%d", lerrno, self->dad.fd));
            return rc;

        case EINVAL:
            rc = RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );
            LOGERR (klogInt, rc, "system invalid argument error");
            return rc;

        default:
            rc = RC ( rcFS, rcFile, rcWriting, rcNoObj, rcUnknown );
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown system error '$(S)($(E))'",
                      "S=%!,E=%d", lerrno, lerrno));
            return rc;
        }

        assert ( num_writ != NULL );
        * num_writ = count;
        self -> pos += count;
        break;
    }

    return 0;
}

static KFile_vt_v1 vtKStdIOStream =
{
    /* version 1.1 */
    1, 1,

    /* start minor version 0 methods */
    KStdIOFileDestroy,
    KSysFileGetSysFile_v1,
    KStdIOFileRandomAccess,
    KStdIOFileSize,
    KStdIOFileSetSize,
    KStdIOFileRead,
    KStdIOFileWrite,
    /* end minor version 0 methods */

    /* start minor version == 1 */
    KSysFileType_v1
    /* end minor version == 1 */
};

static
rc_t KStdIOFileTest ( KFile_v1 **rp, int fd, bool *seekable, bool *readable, bool *writable )
{
    struct stat st;
    int lerrno;
    rc_t rc = 0;

    if ( rp == NULL )
        return RC ( rcFS, rcFile, rcCreating, rcParam, rcNull );

    * rp = NULL;

    if ( fstat ( fd, & st ) == 0 )
    {
        int fl;

        if ( S_ISREG ( st . st_mode ) )
            * seekable = true;
        else
            * seekable = false;

        fl = fcntl ( fd, F_GETFL ) & O_ACCMODE;
        switch (fl)
        {
        case O_RDONLY:
            *readable = true;
            *writable = false;
            break;
        case O_WRONLY:
            *readable = false;
            *writable = true;
            break;
        case O_RDWR:
            *readable = true;
            *writable = true;
            break;
        }
    }
    else switch (lerrno = errno)
    {
    case EBADF:
        rc = RC ( rcFS, rcFile, rcCreating, rcFileDesc, rcInvalid );
        PLOGERR (klogErr,
                 (klogErr, rc, "system error bad file descriptor '$(F)'",
                  "F=%d", fd));
        break;
    default:
        rc = RC ( rcFS, rcFile, rcCreating, rcNoObj, rcUnknown );
        PLOGERR (klogErr,
                 (klogErr, rc, "unknown system error '$(F) ($(E))'",
                  "F=%!,E=%d", lerrno, lerrno));
        break;
    }

    return rc;
}

static
rc_t KStdIOFileMake ( KFile_v1 **fp, int fd,
    bool seekable, bool read_enabled, bool write_enabled )
{
    rc_t rc;
    KStdIOFile *f;

    if ( seekable )
    {
        return KSysFileMakeVT ( ( KSysFile_v1 ** ) fp, fd,
            ( const KFile_vt * ) & vtKStdIOFile, "stdio-file", read_enabled, write_enabled );
    }

    if ( fd < 0 )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcInvalid );
        PLOGERR (klogInt,
                 (klogInt, rc, "invalid file descriptor $(F)",
                  "F=%d", fd));
        return rc;
    }
    else
    {
        f = calloc ( sizeof *f, 1 );
        if ( f == NULL )
        {
            rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            LOGERR (klogErr, rc, "out of memory");
        }
        else
        {
#if USE_TIMEOUT
            if ((KSysFileTimeout >= 0)
#if USE_NO_TIMEOUT_TTY
                && (! isatty (fd)
#endif
                    )
            {
                KSysFile_v1 *ff = &f->dad;

                ff->to.tv_sec = KSysFileTimeout / 1000;
                ff->to.tv_usec = (KSysFileTimeout % 1000) * 1000;
                ff->use_to = true;
                FD_ZERO (&ff->fds);
                FD_SET (fd, &ff->fds);
            }
#endif
            rc = KFileInit_v1 ( &f->dad.dad, ( const KFile_vt * ) &vtKStdIOStream,
                             "KStdIOFile", "fd", read_enabled, write_enabled );
            if ( rc == 0 )
            {
                f -> dad . fd = fd;
                f -> pos = 0;
                * fp = & f -> dad . dad;
                return 0;
            }

            free ( f );
        }
    }
    return rc;
}

/* MakeStdIn
 *  creates a read-only file on stdin
 */
LIB_EXPORT rc_t CC KFileMakeStdIn ( const KFile_v1 **std_in )
{
    bool seekable;
    bool readable;
    bool writable;
    rc_t rc = KStdIOFileTest ( ( KFile_v1 ** ) std_in, 0, & seekable, & readable, & writable );
    if ( rc != 0 )
        return rc;
    if ( ! readable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcWriteonly );

    return KStdIOFileMake ( ( KFile_v1 ** ) std_in, 0, seekable, true, false );
}

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only file on stdout or stderr
 */
LIB_EXPORT rc_t CC KFileMakeStdOut ( KFile_v1 **std_out )
{
    bool seekable;
    bool readable;
    bool writable;
    rc_t rc = KStdIOFileTest ( std_out, 1, & seekable, & readable, & writable );
    if ( rc != 0 )
        return rc;
    if ( ! writable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcReadonly );

    return KStdIOFileMake ( std_out, 1, false, false, true );
}

LIB_EXPORT rc_t CC KFileMakeStdErr ( KFile_v1 **std_err )
{
    bool seekable;
    bool readable;
    bool writable;
    rc_t rc = KStdIOFileTest ( std_err, 2, & seekable, & readable, & writable );
    if ( rc != 0 )
        return rc;
    if ( ! writable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcReadonly );

    return KStdIOFileMake ( std_err, 2, false, false, true );
}

/* MakeFDFile
 *  creates a file from a file-descriptor
 *  not supported under Windows
 */
LIB_EXPORT rc_t CC KFileMakeFDFileRead ( const KFile_v1 **f, int fd )
{
    bool seekable;
    bool readable;
    bool writable;
    rc_t rc = KStdIOFileTest ( ( KFile_v1 ** ) f, fd, & seekable, & readable, & writable );
    if ( rc != 0 )
        return rc;
    if ( ! readable )
    {
        rc = RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcWriteonly );
        LOGERR (klogErr, rc, "error constructing read file from write only file descriptor");
        return rc;
    }
    return KStdIOFileMake ( ( KFile** ) f, fd, seekable, true, false );
}

LIB_EXPORT rc_t CC KFileMakeFDFileWrite ( KFile_v1 **f, bool update, int fd )
{
    bool seekable;
    bool readable;
    bool writable;
    rc_t rc = KStdIOFileTest ( f, fd, & seekable, & readable, & writable );
    if ( rc != 0 )
        return rc;
    if ( update && ! readable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcWriteonly );
    if ( ! writable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcReadonly );

    return KStdIOFileMake ( f, fd, seekable, update, true );
}

LIB_EXPORT rc_t CC KSysFileMake ( KSysFile_v1 **fp, int fd, const char *path, bool read_enabled, bool write_enabled )
{
    bool seekable;
    bool readable;
    bool writable;
    rc_t rc = KStdIOFileTest ( ( KFile** )fp, fd, & seekable, & readable, & writable );
    if ( rc != 0 )
        return rc;
    if ( read_enabled && ! readable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcWriteonly );
    if ( write_enabled && ! writable )
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcReadonly );
    if (!seekable)
        return KStdIOFileMake ( ( KFile** )fp, fd, false, read_enabled, write_enabled );
    else
        return KSysFileMakeVT ( fp, fd, ( const KFile_vt * ) & vtKSysFile,
            path, read_enabled, write_enabled );
}
