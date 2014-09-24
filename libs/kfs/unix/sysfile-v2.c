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
struct KSysFile_v2;
#define KFILE_IMPL struct KSysFile_v2
#define KFILE_VERS 2

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <kfs/extern.h>
#include "sysfile-priv.h"
#include <kfs/kfs-priv.h>
#include <klib/rc.h>
#include <klib/log.h>
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
#else
#define SYSDEBUG(msg)
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
void KSysFileSelect_v2 ( const KSysFile_v2 *self, ctx_t ctx, uint32_t operations )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcSelecting );

    SYSDEBUG( ( "%s: Enter (%p, %x)\n", __func__, self, operations ) );


    if ( self -> use_to )
    {
        fd_set fdsread;
        fd_set fdswrite;
        fd_set fdsexcept;
        struct timeval to;

        fdsread = self -> fds;
        fdswrite = self -> fds;
        fdsexcept = self -> fds;

        to = self -> to;

        while ( ! FAILED () )
        {
            int selected;
            SYSDEBUG ( ( "%s: call select\n", __func__ ) );

            selected = select ( self -> fd + 1, 
                               ( operations & select_read ) ? &fdsread : NULL,
                               ( operations & select_write ) ? &fdswrite : NULL,
                               ( operations & select_exception ) ? &fdsexcept : NULL,
                               &to );
            SYSDEBUG(( "%s: select returned '%d' \n", __func__, selected));

            if ( selected == 0 )
                INTERNAL_ERROR ( xcFileTimeout, "timeout while selecting file" );

            else if ( selected < 0 )
            {
                int lerrno = errno;

                switch ( lerrno )
                {
                case EINVAL:
                    INTERNAL_ERROR ( xcFileTimeout, "timeout while selecting file : coding error bad timeout or negative nfds" );
                    break;

                case EBADF:
                    INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
                    break;

                case EINTR:/*  A signal was caught. */
                    continue;

                case ENOMEM:
                    SYSTEM_ERROR ( xcNoMemory, "out of memory setting up read timeout" );
                    break;

                default:
                    INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
                }
                /* function documentation seems to show that unless EINTR was 
                 * set we can now do our read
                 */
            }
            else
            {
                assert ( FD_ISSET ( self -> fd, &fdsread )  ||
                         FD_ISSET ( self -> fd, &fdswrite ) ||
                         FD_ISSET ( self -> fd, &fdsexcept ) );

                break;
            }
        }
    }
}
#endif


/* Destroy
 */
static
void KSysFileDestroy_v2 ( KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcDestroying );

    int lerrno;

    while ( 1 )
    {
        if ( close ( self -> fd ) != 0 )
        {
            switch ( lerrno = errno )
            {
            case EBADF:
                INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
                break;
                
            case EINTR:
                continue;
                
            default:
                INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
                break;
            }
            
            return;
        }
    }
    
    free ( self );
}

/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */
static
KSysFile_v2 * KSysFileGetSysFile_v2 ( const KSysFile_v2 *self, ctx_t ctx, uint64_t *offset )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcRetrieving );

    * offset = 0;
    return ( KSysFile_v2 * ) self;
}

/* RandomAccess
 *  ALMOST by definition, the file is random access
 *  certain file types ( notably compressors ) will refuse random access
 *
 *  returns true if random access
 */
static
bool KSysFileRandomAccess_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );

    struct stat st;

    int lerrno;

    /* "fstat" gets metadata for an open file */
    if ( fstat ( self -> fd, & st ) != 0 )
    {
        switch ( lerrno = errno )
        {
        case EBADF:
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
            break;
            
        default:
            INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
            break;
        }

        return false;
    }

    /* here we check if the file supports random access by asking
       if it is a "regular" file. It's the only type we actually
       create via open, but we can be given an fd we didn't open,
       and it might not be a regular file. */
    if ( ! S_ISREG ( st . st_mode ) )
        return false;

    return true;
}


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t KSysFileType_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcEvaluating );

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
uint64_t KSysFileSize_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );

    struct stat st;

    int lerrno;

    if ( fstat ( self -> fd, & st ) != 0 )
    {
        switch ( lerrno = errno )
        {
        case EBADF:
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
            break;
            
        default:
            INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
            break;
        }

        return 0;
    }

    if ( ! S_ISREG ( st . st_mode ) )
    {
        // what would be the error for a non regular file
        SYSTEM_ERROR ( xcErrorUnknown, "?" );
        return 0;
    }

    return st . st_size;
}

/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
void KSysFileSetSize_v2 ( KSysFile_v2 *self, ctx_t ctx, uint64_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );

    int lerrno;

    if ( ftruncate ( self -> fd, size ) != 0 ) 
        switch ( lerrno = errno )
        {
        case EBADF:
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
            break;
            
        default:
            INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
            break;
        }
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
size_t KSysFileRead_v2 ( const KSysFile_v2 *self, ctx_t ctx,
     uint64_t pos,void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    ssize_t count = 0;

    assert ( self != NULL );

    while ( 1 )
    {
        int lerrno;

#if USE_TIMEOUT
        ON_FAIL ( KSysFileSelect_v2 ( self, ctx, select_read | select_exception ) )
        {
            return 0;
        }
#endif

        count = pread ( self -> fd, buffer, bsize, pos );

        if ( count < 0 ) 
        {
            switch ( lerrno = errno )
            {
            case EINTR:
                continue;

            case EIO:
                SYSTEM_ERROR ( xcTransferIncomplete, "system I/O error - likely broken pipe" );
                break;
                
            case EBADF:
                INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
                break;
                
            case EISDIR:
                INTERNAL_ERROR ( xcFileDescInvalid, "system misuse of a directory error");
                break;
                
            case EINVAL:
                SYSTEM_ERROR ( xcParamInvalid, "system invalid argument error");
                break;

            default:
                INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
                break;
            }
            
            return 0;
        }

        break;
    }

    return count;
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
size_t KSysFileWrite_v2 ( KSysFile_v2 *self, ctx_t ctx, 
      uint64_t pos, const void *buffer, size_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    ssize_t count = 0;

    assert ( self != NULL );

    while ( 1 )
    {
        int lerrno;

#if USE_TIMEOUT
        ON_FAIL ( KSysFileSelect_v2 ( self, ctx, select_read | select_exception ) )
        {
            return 0;
        }
#endif

        count = pwrite ( self -> fd, buffer, size, pos );

        if ( count < 0 ) 
        {
            switch ( lerrno = errno )
            {
            case ENOSPC:
                INTERNAL_ERROR ( xcStorageExhausted, "system bad file descriptor error fd = %d", self -> fd );
                break;

            case EINTR:
                continue;
                
            case EFBIG:
                INTERNAL_ERROR ( xcFileExcessive, "system file too large error" );
                break;
                
            case EIO:
                SYSTEM_ERROR ( xcTransferIncomplete, "system I/O error - likely broken pipe" );
                break;
                
            case EBADF:
                INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self->fd );
                break;
                
            case EISDIR:
                INTERNAL_ERROR ( xcFileDescInvalid, "system misuse of a directory error");
                break;

            case EINVAL:
                SYSTEM_ERROR ( xcParamInvalid, "system invalid argument error");
                break;

            default:
                INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
                break;
            }

            return 0;
        }

        break;
    }

    return count;
}


/* Make
 *  create a new file object
 *  from file descriptor
 */
static KFile_vt_v2 vtKSysFile =
{
    /* version 2.0 */
    2, 0,

    /* start minor version 0 methods */
    KSysFileDestroy_v2,
    KSysFileGetSysFile_v2,
    KSysFileRandomAccess_v2,
    KSysFileSize_v2,
    KSysFileSetSize_v2,
    KSysFileRead_v2,
    KSysFileWrite_v2,
    KSysFileType_v2
    /* end minor version 0 methods */
};

static
KSysFile_v2 * KSysFileMakeVT_v2 ( ctx_t ctx, int fd, const KFile_vt *vt,
    const char *path, bool read_enabled, bool write_enabled )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    KSysFile_v2 *f;

    if ( fd < 0 )
    {
        INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", fd );
        return NULL;
    }

    f = calloc ( sizeof * f, 1 );
    if ( f == NULL )
    {
        SYSTEM_ERROR ( xcNoMemory, "out of memory" );
        return NULL;
    }

#if USE_TIMEOUT
    if ( KSysFileTimeout >= 0 )
    {
        f -> to.tv_sec = KSysFileTimeout / 1000;
        f -> to.tv_usec = ( KSysFileTimeout % 1000 ) * 1000;
        f -> use_to = true;
        FD_ZERO ( &f -> fds );
        FD_SET ( fd, &f -> fds );
    }
#endif

    TRY ( KFileInit_v2 ( & f -> dad, ctx, vt, "KSysFile", path, read_enabled, write_enabled ) );
    {
        f -> fd = fd;
        return f;
    }

    free ( f );
    
    return NULL;
}

LIB_EXPORT KSysFile_v2 * CC KSysFileMake_v2 ( ctx_t ctx, int fd, const char *path, bool read_enabled, bool write_enabled )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    return KSysFileMakeVT_v2 ( ctx, fd, ( const KFile_vt * ) & vtKSysFile,
        path, read_enabled, write_enabled );
}

/*--------------------------------------------------------------------------
 * KFile
 *  Unix-specific standard i/o interfaces
 */

typedef struct KStdIOFile_v2 KStdIOFile_v2;
struct KStdIOFile_v2
{
    KSysFile_v2 dad;
    uint64_t pos;
};


/* Destroy
 *  does not close fd
 */
static
void KStdIOFileDestroy_v2 ( KSysFile_v2 *self, ctx_t ctx )
{
    free ( self );
}

static KFile_vt_v2 vtKStdIOFile =
{
    /* version 1.1 */
    2, 0,

    /* start minor version 0 methods */
    KStdIOFileDestroy_v2,
    KSysFileGetSysFile_v2,
    KSysFileRandomAccess_v2,
    KSysFileSize_v2,
    KSysFileSetSize_v2,
    KSysFileRead_v2,
    KSysFileWrite_v2,
    KSysFileType_v2
    /* end minor version 0 methods */
};

/* RandomAccess
 */
static
bool KStdIOFileRandomAccess_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    INTERNAL_ERROR ( xcFunctionUnimplemented, "function is not supported" );
    return false;
}


/* Size
 */
static
uint64_t KStdIOFileSize_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    INTERNAL_ERROR ( xcFunctionUnimplemented, "function is not supported" );

    return 0;
}

/* SetSize
 */
static
void KStdIOFileSetSize_v2 ( KSysFile_v2 *self, ctx_t ctx, uint64_t size )
{
    INTERNAL_ERROR ( xcFunctionUnimplemented, "function is not supported" );
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
size_t KStdIOFileRead_v2 ( const KSysFile_v2 *dad, ctx_t ctx, 
      uint64_t pos, void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    KStdIOFile_v2 *self = ( KStdIOFile_v2* ) dad;
    ssize_t count = 0;

    assert ( self != NULL );

    if ( self -> pos != pos )
        INTERNAL_ERROR ( xcParamInvalid, "system invalid argument error");
    

    while ( 1 )
    {
        int lerrno;

#if USE_TIMEOUT
        ON_FAIL ( KSysFileSelect_v2 ( &self->dad, ctx,  select_read | select_exception ) );
        {
            return 0;
        }
#endif

        count = read ( self -> dad . fd, buffer, bsize );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case EINTR:
            continue;
            
        case EIO:
            SYSTEM_ERROR ( xcTransferIncomplete, "system I/O error - likely broken pipe" );
            break;
            
        case EBADF:
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self -> dad . fd );
            break;
            
        case EISDIR:
            INTERNAL_ERROR ( xcFileDescInvalid, "system misuse of a directory error");
            break;
            
        case EINVAL:
            SYSTEM_ERROR ( xcParamInvalid, "system invalid argument error");
            break;
            
        default:
            INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
            break;
        }

        self -> pos += count;
        break;
    }

    return count;
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
size_t KStdIOFileWrite_v2 ( KSysFile_v2 *dad, ctx_t ctx, 
      uint64_t pos, const void *buffer, size_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    KStdIOFile_v2 *self = ( KStdIOFile_v2* ) dad;
    ssize_t count = 0
;
    assert ( self != NULL );

    if ( self -> pos != pos )
        INTERNAL_ERROR ( xcParamInvalid, "system invalid argument error");

    while ( 1 )
    {
        int lerrno;

#if USE_TIMEOUT
        ON_FAIL ( KSysFileSelect_v1 ( &self -> dad, select_read | select_exception ) );
        {
            return 0;
        }
#endif

        count = write ( self -> dad . fd, buffer, size );

        if ( count < 0 ) 
        {
            switch ( lerrno = errno )
            {
            case ENOSPC:
                INTERNAL_ERROR ( xcStorageExhausted, "system bad file descriptor error fd = %d", self -> dad . fd );
                break;
                
            case EINTR:
                continue;
                
            case EFBIG:
                INTERNAL_ERROR ( xcFileExcessive, "system file too large error" );
                break;
                
            case EIO:
                SYSTEM_ERROR ( xcTransferIncomplete, "system I/O error - likely broken pipe" );
                break;
                
            case EBADF:
                INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", self -> dad . fd );
                break;
                
            case EISDIR:
                INTERNAL_ERROR ( xcFileDescInvalid, "system misuse of a directory error");
                break;
                
            case EINVAL:
                SYSTEM_ERROR ( xcParamInvalid, "system invalid argument error");
                break;
                
            default:
                INTERNAL_ERROR ( xcFileUnexpected, "unexpected error code: %!", lerrno );
                break;
            }

            return 0;
        }

        self -> pos += count;
        break;
    }

    return count;
}

static KFile_vt_v2 vtKStdIOStream =
{
    /* version 1.1 */
    2, 0,

    /* start minor version 0 methods */
    KStdIOFileDestroy_v2,
    KSysFileGetSysFile_v2,
    KStdIOFileRandomAccess_v2,
    KStdIOFileSize_v2,
    KStdIOFileSetSize_v2,
    KStdIOFileRead_v2,
    KStdIOFileWrite_v2,
    KSysFileType_v2
    /* end minor version 0 methods */
};

static
void KStdIOFileTest_v2 ( ctx_t ctx, int fd, bool *seekable, bool *readable, bool *writable )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcCreating );

    int lerrno;
    struct stat st;

    if ( fstat ( fd, & st ) == 0 )
    {
        int fl;

        if ( S_ISREG ( st . st_mode ) )
            * seekable = true;
        else
            * seekable = false;

        fl = fcntl ( fd, F_GETFL ) & O_ACCMODE;
        switch ( fl )
        {
        case O_RDONLY:
            * readable = true;
            * writable = false;
            break;
        case O_WRONLY:
            * readable = false;
            * writable = true;
            break;
        case O_RDWR:
            * readable = true;
            * writable = true;
            break;
        }
    }
    else switch ( lerrno = errno )
    {
    case EBADF:
        USER_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", fd );
        break;
    default:
        USER_ERROR ( xcErrorUnknown, "unexpected error code: %!", lerrno );
        break;
    }
}

static
KFile_v2 * KStdIOFileMake_v2 ( ctx_t ctx, int fd, bool seekable, bool read_enabled, bool write_enabled )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );


    if ( seekable )
    {
        /* a seekable fd means it can be treated like a normal file */

        // !!! not sure if the cast is correct, or if the function needs to return a different type
        return ( KFile_v2 * ) KSysFileMakeVT_v2 ( ctx, fd, ( const KFile_vt * ) & vtKStdIOFile,
            "stdio-file", read_enabled, write_enabled );
    }

    if ( fd < 0 )
        USER_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %d", fd );
    else
    {
        KStdIOFile_v2 *f;
        /* create a streamable version */
        f = calloc ( sizeof *f, 1 );
        if ( f == NULL )
            SYSTEM_ERROR ( xcNoMemory, "out of memory" );
        else
        {
#if USE_TIMEOUT
            if ( ( KSysFileTimeout >= 0 )
#if USE_NO_TIMEOUT_TTY
                 && ( ! isatty ( fd ) )
#endif
                )
            {
                KSysFile_v2 *ff = &f -> dad;
                
                ff -> to.tv_sec = KSysFileTimeout / 1000;
                ff -> to.tv_usec = ( KSysFileTimeout % 1000 ) * 1000;
                ff -> use_to = true;
                FD_ZERO ( &ff -> fds );
                FD_SET ( fd, &ff -> fds );
            }
#endif
            TRY ( KFileInit_v2 ( &f -> dad . dad, ctx, ( const KFile_vt * ) &vtKStdIOStream,
                                "KStdIOFile", "fd", read_enabled, write_enabled ) )
            {
                f -> dad . fd = fd;
                f -> pos = 0;
                return & f -> dad . dad;
            }

            free ( f );
        }
    }
    return NULL;
}

/* MakeStdIn
 *  creates a read-only file on stdin
 */
LIB_EXPORT const KFile_v2 * CC KFileMakeStdIn_v2 ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    bool seekable, readable, writable;

    TRY ( KStdIOFileTest_v2 ( ctx, 0, & seekable, & readable, & writable ) )
    {
        if ( ! readable )
            USER_ERROR ( xcFileWriteOnly, "file descriptor 0 is write only!" );
        else
            return KStdIOFileMake_v2 ( ctx, 0, seekable, true, false );
    }

    return NULL;

#if 0
    TRY ( KStdIOFileTest_v2 ( ctx, 0, & seekable, & readable, & writable ) )
    {
        if ( ! readable )
            USER_ERROR ( xcFileWriteOnly, "file descriptor 0 is write only!" );
        
        return KStdIOFileMake_v2 ( 0, seekable, true, false );
    }

    return NULL;
#endif
}

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only file on stdout or stderr
 */
LIB_EXPORT const KFile_v2 * CC KFileMakeStdOut_v2 ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    bool seekable, readable, writable;

    TRY ( KStdIOFileTest_v2 ( ctx, 1, & seekable, & readable, & writable ) )
    {
        if ( ! writable ) // !!! Should this return NULL here or is it ok to continue?
            USER_ERROR ( xcFileReadOnly, "file descriptor 1 is read only!" );

        return KStdIOFileMake_v2 ( ctx, 1, seekable, false, true );
    }

    return NULL;
}

LIB_EXPORT const KFile_v2 * CC KFileMakeStdErr_v2 ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    bool seekable, readable, writable;

    TRY ( KStdIOFileTest_v2 ( ctx, 2, & seekable, & readable, & writable ) )
    {
        if ( ! writable )
            USER_ERROR ( xcFileReadOnly, "file descriptor 2 is read only!" );

        return KStdIOFileMake_v2 ( ctx, 2, seekable, false, true );
    } 

    return NULL;
}

/* MakeFDFile
 *  creates a file from a file-descriptor
 *  not supported under Windows
 */
LIB_EXPORT const KFile_v2 * CC KFileMakeFDFileRead_v2 ( ctx_t ctx, int fd )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    bool seekable, readable, writable;

    TRY ( KStdIOFileTest_v2 ( ctx, fd, & seekable, & readable, & writable ) )
    {
        if ( ! readable )
            USER_ERROR ( xcFileWriteOnly, "file descriptor is write only!" );

        return KStdIOFileMake_v2 ( ctx, fd, seekable, true, false );
    }

    return NULL;
}

LIB_EXPORT  KFile_v2 * CC KFileMakeFDFileWrite_v2 ( ctx_t ctx, bool update, int fd )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    bool seekable, readable, writable;

    TRY ( KStdIOFileTest_v2 ( ctx, fd, & seekable, & readable, & writable ) )
    {
        if ( update && ! readable )
            USER_ERROR ( xcFileWriteOnly, "file descriptor is write only!" );
        if ( ! writable )
            USER_ERROR ( xcFileReadOnly, "file descriptor 2 is read only!" );

        return KStdIOFileMake_v2 ( ctx, fd, seekable, update, true );
    }

    return NULL;
}
