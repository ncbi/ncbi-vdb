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
#include <kfs/chunk-reader.h>
#include <kfs/kfs-priv.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <sysalloc.h>

/* use socket poll function for timeout */
#include "../../kns/poll-priv.h"

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
#include <poll.h>

#ifdef _DEBUGGING
#define SYSDEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_SYS),msg)
#define POS_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_POS),msg)
#else
#define SYSDEBUG(msg)
#define POS_DEBUG(msg)
#endif

#ifndef USE_TIMEOUT
#define USE_TIMEOUT 1
#endif

#ifndef USE_NO_TIMEOUT_TTY
#define USE_NO_TIMEOUT_TTY 0
#endif

#if ! LINUX
#define POLLRDHUP 0
#endif

/*--------------------------------------------------------------------------
 * KSysFile
 *  a Unix file
 */


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
 * TimedRead
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of file.
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
static
rc_t KSysFileRead_v1 ( const KSysFile_v1 * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc;

    assert ( self != NULL );
    assert (num_read != NULL);

    *num_read = 0;

    while ( 1 )
    {
        ssize_t count;
        int lerrno;

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

static
rc_t KSysFileTimedRead_v1 ( const KSysFile_v1 * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    rc_t rc;
    int revents;
    
    assert ( self != NULL );
    assert ( num_read != NULL );

    /* an infinite timeout in this case is the default for KSysFile */
    if ( tm == NULL )
        return KSysFileRead_v1 ( self, pos, buffer, bsize, num_read );

    * num_read = 0;

    /* wait for file to become readable */
    revents = socket_wait ( self -> fd
                            , POLLIN
                            | POLLRDNORM
                            | POLLRDBAND
                            | POLLPRI
                            | POLLRDHUP
                            , tm );

    /* check for error */
    if ( revents < 0 )
    {
        switch ( errno )
        {
        case EFAULT:
        case EINVAL:
            rc = RC ( rcFS, rcFile, rcReading, rcParam, rcInvalid );
            break;
        case EINTR:
            rc = RC ( rcFS, rcFile, rcReading, rcTransfer, rcInterrupted );
            break;
        case ENOMEM:
            rc = RC ( rcFS, rcFile, rcReading, rcMemory, rcExhausted );
            break;
        default:
            rc = RC ( rcFS, rcFile, rcReading, rcError, rcUnknown );
            break;
        }

        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s socket_wait returned '%!'\n",
                                                    self, __func__, errno ) );
        return rc;
    }

    /* check for read availability */
    if ( ( revents & ( POLLRDNORM | POLLRDBAND ) ) != 0 )
    {
        return KSysFileRead_v1 ( self, pos, buffer, bsize, num_read );
    }

    /* check for broken connection */
    if ( ( revents & ( POLLHUP | POLLRDHUP ) ) != 0 )
    {
        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s broken connection\n",
                                                  self, __func__ ) );
        return 0;
    }

    /* anything else in revents is an error */
    if ( ( revents & ~ POLLIN ) != 0 && errno != 0 )
    {
        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s error '%!'\n", self, __func__, errno ) );
        return RC ( rcFS, rcFile, rcReading, rcError, rcUnknown );
    }

    /* finally, call this a timeout */
    DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s timeout\n", self, __func__ ) );
    return RC ( rcFS, rcFile, rcReading, rcTimeout, rcExhausted );
}

/* Write
 * TimedWrite
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed writes. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
static
rc_t KSysFileWrite_v1 ( KSysFile_v1 * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ )
{
    assert ( self != NULL );
    while ( 1 )
    {
        rc_t rc;
        int lerrno;
        ssize_t count;

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

static
rc_t KSysFileTimedWrite_v1 ( KSysFile_v1 * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ, struct timeout_t * tm )
{
    rc_t rc;
    int revents;

    assert ( self != NULL );
    assert ( buffer != NULL );
    assert ( size != 0 );
    assert ( num_writ != NULL );

    /* an infinite timeout in this case is the default for KSysFile */
    if ( tm == NULL )
        return KSysFileWrite_v1 ( self, pos, buffer, size, num_writ );

    * num_writ = 0;

    /* wait for file to become writable */
    revents = socket_wait ( self -> fd
                            , POLLOUT
                            | POLLWRNORM
                            | POLLWRBAND
                            , tm );

    /* check for error */
    if ( revents < 0 )
    {
        switch ( errno )
        {
        case EFAULT:
        case EINVAL:
            rc = RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );
            break;
        case EINTR:
            rc = RC ( rcFS, rcFile, rcWriting, rcTransfer, rcInterrupted );
            break;
        case ENOMEM:
            rc = RC ( rcFS, rcFile, rcWriting, rcMemory, rcExhausted );
            break;
        default:
            rc = RC ( rcFS, rcFile, rcWriting, rcError, rcUnknown );
            break;
        }

        assert ( rc != 0 );
        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s socket_wait returned '%s'\n",
                                                  self, __func__, strerror ( errno ) ) );
        return rc;
    }

    if ( ( revents & ( POLLERR | POLLNVAL ) ) != 0 )
    {
        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s socket_wait returned POLLERR | POLLNVAL\n",
                                                  self, __func__ ) );
        return RC ( rcFS, rcFile, rcWriting, rcNoObj, rcUnknown );
    }

    /* check for broken connection */
    if ( ( revents & POLLHUP ) != 0 )
    {
        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: POLLHUP received\n", self ) );
        return  RC ( rcFS, rcFile, rcWriting, rcTransfer, rcIncomplete );
    }

    /* check for ability to send */
    if ( ( revents & ( POLLWRNORM | POLLWRBAND ) ) != 0 )
    {
        return KSysFileWrite_v1 ( self, pos, buffer, size, num_writ );
    }

    /* anything else in revents is an error */
    if ( ( revents & ~ POLLOUT ) != 0 && errno != 0 )
    {
        DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s error '%s'\n",
                                      self, __func__, strerror ( errno ) ) );
        return RC ( rcFS, rcFile, rcWriting, rcError, rcUnknown );
    }

    /* finally, call this a timeout */
    DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_FILE), ( "%p: %s timeout\n", self, __func__ ) );
    return RC ( rcFS, rcFile, rcWriting, rcTimeout, rcExhausted );
}

static
rc_t CC KSysFileReadChunked_v1 ( const KSysFile_v1 * self, uint64_t pos,
    KChunkReader * chunks, size_t bsize, size_t * total_read )
{
    rc_t rc = 0;
    size_t total, num_read;

    assert ( chunks != NULL );

    for ( total = 0; rc == 0 && total < bsize; total += num_read )
    {
        void * chbuf;
        size_t chsize;
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc == 0 )
        {
            size_t to_read = bsize - total;
            if ( to_read > chsize )
                to_read = chsize;
            
            rc = KFileReadAll_v1 ( & self -> dad, pos + total, chbuf, to_read, & num_read );
            if ( rc == 0 )
                rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, num_read );
            KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
        }

        if ( num_read == 0 )
            break;
    }

    * total_read = total;

    return ( total == 0 ) ? rc : 0;
}

static
rc_t CC KSysFileTimedReadChunked_v1 ( const KFILE_IMPL *self, uint64_t pos,
    KChunkReader * chunks, size_t bsize, size_t * total_read, struct timeout_t * tm )
{
    rc_t rc = 0;
    size_t total, num_read;

    assert ( chunks != NULL );

    for ( total = 0; rc == 0 && total < bsize; total += num_read )
    {
        void * chbuf;
        size_t chsize;
        rc = KChunkReaderNextBuffer ( chunks, & chbuf, & chsize );
        if ( rc == 0 )
        {
            size_t to_read = bsize - total;
            if ( to_read > chsize )
                to_read = chsize;
            
            rc = KSysFileTimedRead_v1 ( self, pos + total, chbuf, to_read, & num_read, tm );
            if ( rc == 0 && num_read != 0 )
                rc = KChunkReaderConsumeChunk ( chunks, pos + total, chbuf, num_read );
            KChunkReaderReturnBuffer ( chunks, chbuf, chsize );
        }

        if ( num_read == 0 )
            break;
    }

    * total_read = total;

    return ( total == 0 ) ? rc : 0;
}


/* Make
 *  create a new file object
 *  from file descriptor
 */
static KFile_vt_v1 vtKSysFile =
{
    /* version 1.3 */
    1, 3,

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
    KSysFileType_v1,
    /* end minor version == 1 */

    /* start minor version == 2 */
    KSysFileTimedRead_v1,
    KSysFileTimedWrite_v1,
    /* end minor version == 2 */

    /* start minor version == 3 */
    KSysFileReadChunked_v1,
    KSysFileTimedReadChunked_v1
    /* end minor version == 3 */
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
    /* version 1.2 */
    1, 2,

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
    KSysFileType_v1,
    /* end minor version == 1 */

    /* start minor version == 2 */
    KSysFileTimedRead_v1,
    KSysFileTimedWrite_v1
    /* end minor version == 2 */
};

/* RandomAccess
 */
static
rc_t KStdIOFileRandomAccess ( const KSysFile_v1 *self )
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* Size
 */
static
rc_t KStdIOFileSize ( const KSysFile_v1 *self, uint64_t *size )
{
    * size = 0;

    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}

/* SetSize
 */
static
rc_t KStdIOFileSetSize ( KSysFile_v1 *self, uint64_t size )
{
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
