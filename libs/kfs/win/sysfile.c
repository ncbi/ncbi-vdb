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

#include <kfs/extern.h>


/*--------------------------------------------------------------------------
 * forwards
 */
struct KSysFile_v1;
#define KFILE_IMPL struct KSysFile_v1

#include "sysfile-priv.h"
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <sysalloc.h>

/* temporary */
/* #include <klib/out.h> */

/* do not include windows.h it is already included by os-native.h */
#include <os-native.h>
#include <WinIoCtl.h>

/*--------------------------------------------------------------------------
 * KSysFile
 *  a Windows file
 */

/* minimum set size or write beyond size difference to trigger setting sparse */
/* tune this if too many or too few sparse files */
#define MIN_SET_SPARSE_DIFF   (16*1024)
#define MIN_SPARSE_BLOCK_DIFF  (4*1024)


/* ----------
 * Some functions to isolate the calls to Windows functions as I feel dirty
 * just using them.
 * really its to isolate some calls the very different style of parmaters
 * for the calls from the usualy project approach.
 *
 * if the compiler inlines them it's all good.
 */

/*
 * Get file size 
 */
static rc_t get_file_size ( const KSysFile_v1 *self, uint64_t *size )
{
    LARGE_INTEGER sz;

    if ( GetFileSizeEx ( self -> handle, & sz ) == 0 )
    {
        rc_t rc;
        DWORD last_error;

        last_error = GetLastError ();
        switch ( last_error )
        {
        case ERROR_INVALID_HANDLE:
            rc = RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcInvalid );
            break;
        default:
            rc = RC ( rcFS, rcFile, rcAccessing, rcNoObj, rcUnknown );
            break;
        }
        PLOGERR (klogErr,
                 (klogErr, rc, "error accessing file system status - $(E)($(C))",
                  "E=%!,C=%u", last_error, last_error)); 
        return rc;
    }
    *size = sz.QuadPart;

    return 0;
}


/* returns (and side effect sets in structure)
 * if the file is already a sparse file
 */
static bool check_if_sparse ( KSysFile_v1 *self )
{
    BY_HANDLE_FILE_INFORMATION info;
    BOOL worked;

    if (self->is_sparse)
        return true;

    /*
     * we don't use the GetFileInformationBy HandleEx as we don't want to 
     * exclude Win XP yet.
     */
    worked = GetFileInformationByHandle (self->handle, &info);

    self->is_sparse = 
        ((info.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
         == FILE_ATTRIBUTE_SPARSE_FILE);

    return self->is_sparse;
}

/*
 * make a file sparse set_it == true
 * return is like Windows funcs with true being good
 *
 * we can't set if the Windows Volume doesn't allow it
 * but we'll let the function fail rather than try to get the value at 
 * CREATE (open) time because we'd be then checking at all file CREATE
 * whether we'd ever make it sparse or not
 */
static
bool set_sparse ( KSysFile_v1 *self )
{
    FILE_SET_SPARSE_BUFFER b = { true };
    DWORD ret;
    BOOL worked;
    bool rreett = false;

    /* don't duplicate effort */
    if (self->is_sparse)
        rreett =  true;

    else if (self->failed_set_sparse)
        rreett = false;

    else
    {
        worked = DeviceIoControl ( self->handle, FSCTL_SET_SPARSE, &b, sizeof b,
                              NULL, 0, &ret, NULL );
/*         KOutMsg ("%s: %u\n",__func__,worked); */

        /* not trusting bool is BOOL cause I don't trust Microsoft */
        self->failed_set_sparse = (worked == 0);
        self->is_sparse = !self->failed_set_sparse;
        rreett =  self->is_sparse;
    }
/*     KOutMsg ("%s: %d\n",__func__,rreett); */
    return rreett;
}


static bool set_not_sparse ( KSysFile_v1 *self )
{
    FILE_SET_SPARSE_BUFFER b = { false };
    DWORD ret;
    BOOL worked;

    /* don't duplicate effort */
    if (!check_if_sparse (self))
        return true;

    if (self->failed_set_sparse)
        return false;

    worked = DeviceIoControl (self->handle, FSCTL_SET_SPARSE, &b, sizeof b,
                              NULL, 0, &ret, NULL);

    /* not trusting bool is BOOL cause I don't trust Microsoft */
    self->failed_set_sparse = (worked == 0);
    self->is_sparse = self->failed_set_sparse;
    return ! self->is_sparse;
}

/*
 * this one works for non-sparse files too but what evs.
 */
static rc_t set_zero_region ( KSysFile_v1 *self, uint64_t start, uint64_t size)
{
    FILE_ZERO_DATA_INFORMATION b = { ( DWORD ) start, ( LONG ) ( start + size ) };
    DWORD ret;
    BOOL worked;

    worked = DeviceIoControl (self->handle, FSCTL_SET_ZERO_DATA,
                            &b, sizeof b,
                            NULL, 0,
                            &ret, NULL);

    /* TODO: check error codes with GetLastError and better rc values */
    return (worked != 0) ? 0 : RC (rcFS, rcFile, rcWriting, rcBuffer, rcUnexpected);
}


/*
 * returns true if we can convert this file into a non-sparse file
 *
 * We went simple and fast.  We ask about zero regions.
 * if we specifically get back that there are none we say true
 * otherwise we say false.
 *
 * self can be modified so it can not be const
 */
static bool can_be_made_not_sparse ( KSysFile_v1 *self )
{
#if 1
    return false;
#else
    /* this is backwards - the list of os non-zero regions not zero regions */
    LARGE_INTEGER fo;
    LARGE_INTEGER l;
    uint64_t size;
    FILE_ALLOCATED_RANGE_BUFFER i;
    FILE_ALLOCATED_RANGE_BUFFER o [16]; /* some none 0 number */
    uint64_t count;
    DWORD ret;
    BOOL worked;
    rc_t rc;

    /* if is isn't sparse we can't make it not sparse */
    /* first might be not yet set */
    if (!check_if_sparse (self))
        return true;

    /* we can't scan for zero regions if we can't get a size */
    rc = get_file_size (self, &size);
    if (rc)
        return false;

    /* Microsoft APIs can be fairly odd */
    fo.QuadPart = 0;
    l.QuadPart = size;
    i.FileOffset = fo;
    i.Length = l;

    worked = DeviceIoControl (self->handle, FSCTL_QUERY_ALLOCATED_RANGES,
                              &i, sizeof i, o, sizeof o, & ret, NULL);
    /* we can't change to non-sparse if we can't scan for zero regions */
    if (worked == 0)
        return false;

    return (ret == 0);
#endif
}


/* Destroy
 */
static
rc_t CC KSysFileDestroy_v1 ( KSysFile_v1 *self )
{
    rc_t rc = 0;

    if ( CloseHandle (self -> handle ) == 0 )
    {
        DWORD last_error;

        last_error = GetLastError();
        switch ( last_error )
        {
        case ERROR_INVALID_HANDLE:
            break;
        default:
            rc = RC (rcFS, rcFile, rcDestroying, rcNoObj, rcUnknown);
            PLOGERR (klogErr,
                     (klogErr, rc, "error closing system file - $(E)$($(C))",
                      "E=%!,C=%u", last_error, last_error)); 
        }
    }
    DeleteCriticalSection(&self -> lock);
    free ( self );
    return rc;
}


static
rc_t CC KSysStdIOFileDestroy_v1 ( KSysFile_v1 *self )
{
    DeleteCriticalSection(&self -> lock);
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
KSysFile_v1 * CC KSysFileGetSysFile_v1 ( const KSysFile_v1 *self, uint64_t *offset )
{
    * offset = 0;
    return ( KSysFile_v1 * ) self;
}

/* RandomAccess
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KSysDiskFileRandomAccess_v1 ( const KSysFile_v1 *self )
{
    return 0;
}
static
rc_t CC KSysFileRandomAccess_v1 ( const KSysFile_v1 *self )
{
/*     return RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect ); */
        return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KSysFileType_v1 ( const KSysFile_v1 *self )
{
    switch ( GetFileType ( self -> handle ) )
    {
    case FILE_TYPE_DISK:
        return kfdFile;
    case FILE_TYPE_CHAR:
        return kfdCharDev;
    case FILE_TYPE_PIPE:
        return kfdSocket;
    }

    return kfdInvalid;
}


/* Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KSysDiskFileSize_v1 ( const KSysFile_v1 *self, uint64_t *size )
{
#if 1
/*     KOutMsg ("%s:\n",__func__); */
    return get_file_size (self, size);
#else

    LARGE_INTEGER sz;

    if ( GetFileSizeEx ( self -> handle, & sz ) == 0 )
    {
        rc_t rc;
        DWORD last_error;

        last_error = GetLastError ();
        switch ( last_error )
        {
        case ERROR_INVALID_HANDLE:
            rc = RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcInvalid );
            break;
        default:
            rc = RC ( rcFS, rcFile, rcAccessing, rcNoObj, rcUnknown );
            break;
        }
        PLOGERR (klogErr,
                 (klogErr, rc, "error accessing file system status - $(E)($(C))",
                  "E=%!,C=%u", last_error, last_error)); 
        return rc;
    }

    *size = sz . QuadPart;
    return 0;
#endif
}


static
rc_t CC KSysFileSize_v1 ( const KSysFile_v1 *self, uint64_t *size )
{
    *size = 0;
/*     return RC ( rcFS, rcFile, rcAccessing, rcFileDesc, rcIncorrect ); */
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KSysDiskFileSetSize_v1 ( KSysFile_v1 *self, uint64_t size )
{
    rc_t rc = 1;
    LARGE_INTEGER p;
    uint64_t prev_size;

    EnterCriticalSection(&self->lock);
/*     KOutMsg ("%s:\n",__func__); */
    /* get previous size for setting or clearing sparse */
    rc = get_file_size ( self, &prev_size);
    if (rc) {
        LeaveCriticalSection(&self->lock);
        return rc;
    }
/*     KOutMsg ("%s: %lu\n",__func__, prev_size); */

    p . QuadPart = size;

    if ( SetFilePointerEx ( self -> handle, p, & p, FILE_BEGIN ) )
    {
        self -> pos = size;
        if ( SetEndOfFile( self -> handle ) )
        {
            rc = 0;
        }
    }

    /* failure to set size*/
    if ( rc != 0 )
    {
        DWORD last_error;

        last_error = GetLastError ();
        switch ( last_error  )
        {
        case ERROR_INVALID_HANDLE:
            rc = RC ( rcFS, rcFile, rcUpdating, rcFileDesc, rcInvalid );
        default:
            rc = RC ( rcFS, rcFile, rcUpdating, rcNoObj, rcUnknown );
        }
        PLOGERR ( klogErr,
                 ( klogErr, rc, "error setting filesize - $(E) - $(C) to $(D)",
                   "E=%!,C=%u,D=%lu", last_error, last_error, size ) ); 
    }

    /* check for wanting to be sparse file */
    if (size > prev_size)
    {
        uint64_t diff;

/*         KOutMsg ("%s: size(%lu) larger than prev_size(%lu)\n",__func__, size, prev_size); */

        diff = size - prev_size;

        /* if block size if big enough we'll try to make it a sparse block */
        if (diff >= MIN_SPARSE_BLOCK_DIFF)
        {
/*             KOutMsg ("%s: diff(%lu) larger than block constant(%lu)\n",__func__, diff, MIN_SPARSE_BLOCK_DIFF); */
            /* set sparse? */
            if (!check_if_sparse(self)) /* isn't sparse now */
            {
                if (diff >= MIN_SET_SPARSE_DIFF)
                {
/*                     KOutMsg ("%s: diff(%lu) larger than sparse constant(%lu)\n",__func__, diff, MIN_SET_SPARSE_DIFF); */
                    (void)set_sparse (self);
                }
            }
            /* ordered to try to set before looking to set the zero region */
            if (self->is_sparse)
            {
                /* set sparse region at end */
                set_zero_region (self, prev_size, diff);
            }
        }
    }
    else if (can_be_made_not_sparse (self))
        (void)set_not_sparse (self);

    LeaveCriticalSection(&self->lock);
    return rc;
}


static
rc_t CC KSysFileSetSize_v1 ( KSysFile_v1 *self, uint64_t size )
{
/*     return RC ( rcFS, rcFile, rcUpdating, rcFileDesc, rcIncorrect ); */
        return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
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
rc_t KSysFileReadCommon_v1 ( const KSysFile_v1 *cself,
    void *buffer, size_t bsize, size_t *num_read )
{
    DWORD to_read, bytes_read;
    KSysFile_v1 *self = ( KSysFile* ) cself;

    to_read = ( DWORD ) bsize;
    if ( sizeof bsize > sizeof to_read && ( size_t ) to_read != bsize )
        to_read = ~ 0U;

    for (;;)
    {
        bytes_read = 0;
        if ( ReadFile ( self -> handle, buffer, to_read, & bytes_read, NULL ) == 0 )
        {
            DWORD last_error;

            switch ( last_error = GetLastError () )
            {
            case ERROR_HANDLE_EOF:
                break;
            case ERROR_IO_PENDING:
                continue; 
            default:
                {
                    rc_t rc = RC ( rcFS, rcFile, rcReading, rcNoObj, rcUnknown);
                    PLOGERR ( klogErr,
                            ( klogErr, rc, "error reading system file - $(E)($(C))",
                                            "E=%!,C=%u", last_error, last_error ) ); 
                    return rc;
                }
           }
        }
        
        self -> pos += bytes_read;
        * num_read = bytes_read;
        break;
    }

    return 0;
}

static
rc_t CC KSysDiskFileRead_v1 ( const KSysFile_v1 *cself, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;
    KSysFile_v1 *self = ( KSysFile* ) cself;

    EnterCriticalSection(&self->lock);

    if ( self -> pos != pos )
    {
        LARGE_INTEGER p;

        if ( !GetFileSizeEx( self -> handle, &p ) )
        {
            rc_t rc;
            DWORD last_error;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_INVALID_HANDLE:
                rc = RC ( rcFS, rcFile, rcPositioning, rcFileDesc, rcInvalid );
                break;
            default:
                rc = RC ( rcFS, rcFile, rcPositioning, rcNoObj, rcUnknown );
                break;
            }
            PLOGERR ( klogErr,
                     ( klogErr, rc, "error positioning system file - $(E)($(C)) to $(D)",
                       "E=%!,C=%u,D=%u", last_error, last_error, pos ) ); 
            LeaveCriticalSection(&self->lock);
            return rc;
        }

        /* if we try to read beyond the end of the file... */
        if ( ( LONGLONG ) pos >= p . QuadPart )   
        {   /* We've defined reading beyond EOF as return RC of 0 but bytes read as 0 */
            /*return RC ( rcFS, rcFile, rcPositioning, rcFileDesc, rcInvalid );*/
            LeaveCriticalSection(&self->lock);
            return 0;
        }

        p . QuadPart = pos;
        if ( !SetFilePointerEx ( self -> handle, p, & p, FILE_BEGIN ) )
        {
            rc_t rc;
            DWORD last_error;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_INVALID_HANDLE:
                rc = RC ( rcFS, rcFile, rcPositioning, rcFileDesc, rcInvalid );
                PLOGERR (klogErr,
                         (klogErr, rc, "invalid system file handle - $(E)($(C))",
                          "E=%!,C=%u", last_error, last_error)); 
                LeaveCriticalSection(&self->lock);
                return rc;
            default:
                rc = RC ( rcFS, rcFile, rcPositioning, rcNoObj, rcUnknown );
                PLOGERR ( klogErr,
                          ( klogErr, rc, "error positioning system file - $(E)($(C)) to $(D)",
                            "E=%!,C=%u,D=%lu", last_error, last_error, pos ) ); 
                LeaveCriticalSection(&self->lock);
                return rc;
            }
        }

        self -> pos = p . QuadPart;
        if ( pos != p . QuadPart )
        {
            if ( pos > (uint64_t)( p . QuadPart ) )
            {
                * num_read = 0;
                LeaveCriticalSection(&self->lock);
                return 0;
            }

            LeaveCriticalSection(&self->lock);
            return RC ( rcFS, rcFile, rcPositioning, rcNoObj, rcUnknown );
        }
    }

    rc = KSysFileReadCommon_v1 ( cself, buffer, bsize, num_read );
    LeaveCriticalSection(&self->lock);
    return rc;    
}

static
rc_t CC KSysDiskFileTimedRead_v1 ( const KSysFile_v1 * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    return KSysDiskFileRead_v1 ( self, pos, buffer, bsize, num_read );
}

static
rc_t CC KSysDiskFileReadChunked_v1 ( const KSysFile_v1 * self, uint64_t pos,
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
            rc = KSysDiskFileRead_v1 ( self, pos + total, chbuf, chsize, & num_read );
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

static
rc_t CC KSysDiskFileTimedReadChunked_v1 ( const KFILE_IMPL *self, uint64_t pos,
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
            rc = KSysDiskFileTimedRead_v1 ( self, pos + total, chbuf, chsize, & num_read, tm );
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

static
rc_t CC KSysFileRead_v1 ( const KSysFile_v1 * cself, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc;
    KSysFile_v1 * self = ( KSysFile * ) cself;
    
    EnterCriticalSection(&self->lock);
    if ( self -> pos != pos )
    {
        LeaveCriticalSection(&self->lock);
        *num_read = 0;
        return RC ( rcFS, rcFile, rcPositioning, rcFileDesc, rcIncorrect );
    }

    rc = KSysFileReadCommon_v1 ( self, buffer, bsize, num_read );
    LeaveCriticalSection(&self->lock);
    return rc;    
}

static
rc_t CC KSysFileTimedRead_v1 ( const KSysFile_v1 * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    return KSysFileRead_v1 ( self, pos, buffer, bsize, num_read );
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
            rc = KSysFileRead_v1 ( self, pos + total, chbuf, chsize, & num_read );
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
            rc = KSysFileTimedRead_v1 ( self, pos + total, chbuf, chsize, & num_read, tm );
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

#define WINDOWS_HEAP_LIMIT (32*1024)

static
rc_t KSysFileWriteCommon_v1 ( KSysFile_v1 *self, 
    const void *_buffer, size_t size, size_t *num_writ)
{
    DWORD to_write;
    DWORD bytes_writ;
    DWORD iter_writ;
    const char * buffer = _buffer;

    to_write = (DWORD) size;

    if (sizeof size > sizeof to_write && ( size_t ) to_write != size )
        to_write = ~ 0U;

    for ( bytes_writ = 0; to_write > 0; )
    {
        DWORD iter_to_write = 
            (to_write > WINDOWS_HEAP_LIMIT)? WINDOWS_HEAP_LIMIT : to_write;

        
        for (iter_writ = 0;
             ! WriteFile ( self -> handle, buffer, iter_to_write, &iter_writ, NULL);
             iter_writ = 0)
        {
            rc_t rc;
            DWORD last_error;
            const char * fmt;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_IO_PENDING:
                Sleep (100); /* sure let's give it a chance to settle */
                if (iter_writ)
                {
                    buffer += iter_writ;
                    iter_to_write -= iter_writ;
                    to_write -= iter_writ;
                    bytes_writ += iter_writ;
                }
                continue; /* back to while() */
 
            case ERROR_INVALID_HANDLE:
                rc = RC ( rcFS, rcFile, rcWriting, rcFileDesc, rcInvalid );
                fmt = "invalid system file handle - $(E)($(C))";
                break;

            case ERROR_NOT_ENOUGH_MEMORY:
                rc = RC (rcFS, rcFile, rcWriting, rcMemory, rcExhausted);
                fmt = "error out of memory for WindowsWriteFile - $(E)($(C))";
                break;

            default:
                rc = RC ( rcFS, rcFile, rcWriting, rcNoObj, rcUnknown );
                fmt = "error writing system file - $(E)($(C))";
                break;
            }

            PLOGERR (klogErr,
                     (klogErr, rc, fmt, "E=%!,C=%u", last_error, last_error)); 
            return rc;
        }
        buffer += iter_writ;
        to_write -= iter_writ;
        bytes_writ += iter_writ;
    }

    self -> pos += bytes_writ;

    if (num_writ != NULL)
        * num_writ = (size_t)bytes_writ;

    return 0;
}

static
rc_t CC KSysDiskFileWrite_v1 ( KSysFile_v1 * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ )
{
    rc_t rc;
    EnterCriticalSection(&self->lock);
    if ( self -> pos != pos )
    {
        LARGE_INTEGER p;
        uint64_t curr_size;

        rc = get_file_size ( self, &curr_size );
        if ( rc != 0 ) 
        {
            LeaveCriticalSection(&self->lock);
            return rc;
        }

        if ( curr_size < pos )
        {
            LeaveCriticalSection(&self->lock);
            rc = KSysDiskFileSetSize_v1 (self, pos);
            if (rc)
                return rc;
            EnterCriticalSection(&self->lock);
        }


        p . QuadPart = pos;

        if ( SetFilePointerEx ( self -> handle, p, & p, FILE_BEGIN ) == 0 )
        {
            DWORD last_error;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_INVALID_HANDLE:
                rc = RC ( rcFS, rcFile, rcWriting, rcFileDesc, rcInvalid );
                PLOGERR (klogErr,
                         (klogErr, rc, "invalid system file handle - $(E)($(C))",
                          "E=%!,C=%u", last_error, last_error)); 
                break;
            default:
                rc = RC ( rcFS, rcFile, rcWriting, rcNoObj, rcUnknown );
                PLOGERR (klogErr,
                         (klogErr, rc, "error writing system file - $(E)($(C))",
                          "E=%!,C=%u", last_error, last_error)); 
                break;
            }
            LeaveCriticalSection(&self->lock);
            return rc;
        }

        self -> pos = p . QuadPart;
        if ( pos != p . QuadPart )
        {
            LeaveCriticalSection(&self->lock);
            return RC ( rcFS, rcFile, rcPositioning, rcNoObj, rcUnknown );
        }
    }

    rc = KSysFileWriteCommon_v1 ( self, buffer, size, num_writ );
    LeaveCriticalSection(&self->lock);
    return rc;    
}

static
rc_t CC KSysDiskFileTimedWrite_v1 ( KSysFile_v1 * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ, struct timeout_t * tm )
{
    return KSysDiskFileWrite_v1 ( self, pos, buffer, size, num_writ );
}

static
rc_t CC KSysFileWrite_v1 ( KSysFile_v1 * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ )
{
    rc_t rc;
    EnterCriticalSection(&self->lock);
    if ( self -> pos != pos )
    {
        *num_writ = 0;
        LeaveCriticalSection(&self->lock);
        return RC ( rcFS, rcFile, rcPositioning, rcFileDesc, rcIncorrect );
    }

    rc = KSysFileWriteCommon_v1 ( self, buffer, size, num_writ );
    LeaveCriticalSection(&self->lock);
    return rc;    
}

static
rc_t CC KSysFileTimedWrite_v1 ( KSysFile_v1 * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ, struct timeout_t * tm )
{
    return KSysFileWrite_v1 ( self, pos, buffer, size, num_writ);
}

/* Make
 *  create a new file object
 *  from file descriptor
 */
static const KFile_vt_v1 vtKSysDiskFile =
{
    /* version 1.3 */
    1, 3,

    /* start minor version 0 methods */
    KSysFileDestroy_v1,
    KSysFileGetSysFile_v1,
    KSysDiskFileRandomAccess_v1,
    KSysDiskFileSize_v1,
    KSysDiskFileSetSize_v1,
    KSysDiskFileRead_v1,
    KSysDiskFileWrite_v1,
    /* end minor version 0 methods */

    /* start minor version == 1 */
    KSysFileType_v1,
    /* end minor version == 1 */

    /* start minor version == 2 */
    KSysDiskFileTimedRead_v1,
    KSysDiskFileTimedWrite_v1,
    /* end minor version == 2 */

    /* start minor version == 3 */
    KSysDiskFileReadChunked_v1,
    KSysDiskFileTimedReadChunked_v1
    /* end minor version == 3 */
};
static const KFile_vt_v1 vtKSysOtherFile =
{
    /* version 1.2 */
    1, 2,

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
static const KFile_vt_v1 vtKSysStdIODiskFile =
{
    /* version 1.3 */
    1, 3,

    /* start minor version 0 methods */
    KSysStdIOFileDestroy_v1,
    KSysFileGetSysFile_v1,
    KSysDiskFileRandomAccess_v1,
    KSysDiskFileSize_v1,
    KSysDiskFileSetSize_v1,
    KSysDiskFileRead_v1,
    KSysDiskFileWrite_v1,
    /* end minor version 0 methods */

    /* start minor version == 1 */
    KSysFileType_v1,
    /* end minor version == 1 */

    /* start minor version == 2 */
    KSysDiskFileTimedRead_v1,
    KSysDiskFileTimedWrite_v1,
    /* end minor version == 2 */

    /* start minor version == 3 */
    KSysDiskFileReadChunked_v1,
    KSysDiskFileTimedReadChunked_v1
    /* end minor version == 3 */
};
static const KFile_vt_v1 vtKSysStdIOOtherFile =
{
    /* version 1.3 */
    1, 3,

    /* start minor version 0 methods */
    KSysStdIOFileDestroy_v1,
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
rc_t KSysFileMakeVT ( KSysFile_v1 **fp, HANDLE fd, const KFile_vt *vt, const char *path,
    uint64_t initial_pos, bool read_enabled, bool write_enabled )
{
    rc_t rc;
    KSysFile_v1 *f;

    if( fd == INVALID_HANDLE_VALUE ) /* ? */
        return RC ( rcFS, rcFile, rcConstructing, rcFileDesc, rcInvalid );

    f = malloc ( sizeof *f );
    if ( f == NULL )
        rc = RC(rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    else
    {
        rc = KFileInit ( & f -> dad, vt, "KSysFile", path, read_enabled, write_enabled );
        if ( rc == 0 )
        {
            f -> handle = fd;
            f -> pos = initial_pos;
            f -> failed_set_sparse = f->is_sparse = false;
            check_if_sparse (f);
            InitializeCriticalSection(& f -> lock);
            *fp = f;
            return 0;
        }

        free ( f );
    }
    return rc;
}

/* bit flags */
#define ISDISK  1
#define ISSTDIO 2

static
rc_t KSysFileMakeInt ( KSysFile_v1 **fp, HANDLE fd, const char *path, bool read_enabled, bool write_enabled, unsigned flags )
{
    DWORD ret;
    const KFile_vt * vt;
    uint64_t initial_pos;

    if (fp == NULL)
        return RC (rcFS, rcFile, rcConstructing, rcSelf, rcNull);

    *fp = NULL; /* pre-fail */

    ret = GetFileType (fd);

    switch (ret)
    {
    case FILE_TYPE_DISK:
        flags |= ISDISK;
        initial_pos = -1;
        break;

    case FILE_TYPE_UNKNOWN:
        ret = GetLastError();
        switch (ret)
        {
        default:
            return RC (rcFS, rcFile, rcConstructing, rcFileDesc, rcUnknown);

            /* specific errors can be added here */

        case NO_ERROR:
            break;
        }

        initial_pos = 0;
        flags &= ~ISDISK;
        break;

    default:
        initial_pos = 0;
        flags &= ~ISDISK;
        break;
    }

    switch (flags & (ISDISK|ISSTDIO))
    {
    case 0:
        vt = (const KFile_vt*)&vtKSysOtherFile;
        break;

    case ISDISK:
        vt = (const KFile_vt*)&vtKSysDiskFile;
        break;

    case ISSTDIO:
        vt = (const KFile_vt*)&vtKSysStdIOOtherFile;
        break;

    case ISDISK|ISSTDIO:
        vt = (const KFile_vt*)&vtKSysStdIODiskFile;
        break;
    }

    return KSysFileMakeVT ( fp, fd, vt, path, initial_pos, read_enabled, write_enabled );
}

/* extern, but internal to libkfs */
rc_t KSysFileMake ( KSysFile_v1 **fp, HANDLE fd, const char *path, bool read_enabled, bool write_enabled )
{
    return KSysFileMakeInt ( fp, fd, path, read_enabled, write_enabled, 0 );
}


/* MakeStdIn
 *  creates a read-only file on stdin
 */
LIB_EXPORT rc_t CC KFileMakeStdIn ( const KFile **fp )
{
    HANDLE fd = GetStdHandle ( STD_INPUT_HANDLE );
    return KSysFileMakeInt ( (KSysFile**)fp, fd, "stdin", true, false, ISSTDIO );
}

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only file on stdout or stderr
 */
LIB_EXPORT rc_t CC KFileMakeStdOut ( KFile **fp )
{
    HANDLE fd = GetStdHandle ( STD_OUTPUT_HANDLE );
    return KSysFileMakeInt ( (KSysFile**)fp, fd, "stdout", false, true, ISSTDIO );
}

LIB_EXPORT rc_t CC KFileMakeStdErr ( KFile **fp )
{
    HANDLE fd = GetStdHandle ( STD_ERROR_HANDLE );
    return KSysFileMakeInt ( (KSysFile**)fp, fd, "stderr", false, true, ISSTDIO );
}

/* MakeFDFile
 *  creates a file from a file-descriptor
 *  not supported under Windows
 */
LIB_EXPORT rc_t CC KFileMakeFDFileRead ( const KFile **f, int fd )
{
    return RC (rcFS, rcFile, rcConstructing, rcFunction, rcUnsupported);
}

LIB_EXPORT rc_t CC KFileMakeFDFileWrite ( KFile **f, bool update, int fd )
{
    return RC (rcFS, rcFile, rcConstructing, rcFunction, rcUnsupported);
}
