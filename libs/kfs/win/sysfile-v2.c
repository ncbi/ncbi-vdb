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
struct KSysFile_v2;
#define KFILE_IMPL struct KSysFile_v2
#define KFILE_VERS 2

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include "sysfile-priv.h"
#include <kfs/kfs-priv.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/text.h>
#include <sysalloc.h>

/* temporary */
/* #include <klib/out.h> */



#include <Windows.h>
/* #include <WinIoCtl.h> nested include in Windows.h? */

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
static rc_t get_file_size ( const KSysFile_v2 *self, uint64_t *size )
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
static bool check_if_sparse ( KSysFile_v2 *self )
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
bool set_sparse ( KSysFile_v2 *self )
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


static bool set_not_sparse ( KSysFile_v2 *self )
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
static rc_t set_zero_region ( KSysFile_v2 *self, uint64_t start, uint64_t size)
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
static bool can_be_made_not_sparse ( KSysFile_v2 *self )
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
void CC KSysFileDestroy_v2 ( KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcDestroying );

    if ( CloseHandle (self -> handle ) == 0 )
    {
        DWORD last_error;

        last_error = GetLastError();
        switch ( last_error )
        {
        case ERROR_INVALID_HANDLE:
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error handle = %zu", self->handle );
            break;
        default:
            INTERNAL_ERROR ( xcErrorUnknown, "error closing system file" );
        }
        return;
    }

    free ( self );
}


static
void CC KSysStdIOFileDestroy_v2 ( KSysFile_v2 *self, ctx_t ctx )
{
    free ( self );
}


/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */
static
KSysFile_v2 * CC KSysFileGetSysFile_v2 ( const KSysFile_v2 * self, ctx_t ctx, uint64_t *offset )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );

    * offset = 0;
    return ( KSysFile_v2 * ) self;
}

/* RandomAccess
 *  returns 0 if random access, error code otherwise
 */
static
bool CC KSysDiskFileRandomAccess_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    INTERNAL_ERROR ( xcFunctionUnsupported, "function is not supported" );
    return false;
}
static
bool CC KSysFileRandomAccess_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    INTERNAL_ERROR ( xcFunctionUnsupported, "function is not supported" );
    return false;
}


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KSysFileType_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcEvaluating );

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
uint64_t CC KSysDiskFileSize_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcAccessing );
#if 1
/*     KOutMsg ("%s:\n",__func__); */
    uint64_t size;
    rc_t rc = get_file_size (self, &size);
    if ( rc )
        return 0 ;

    return size;
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
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error handle = %zu", self->handle );
            break;
        default:
            INTERNAL_ERROR ( xcErrorUnknown, "error accessing file system status" );
            break;
        }
         return 0;
    }

    return sz . QuadPart;
#endif
}


static
uint64_t CC KSysFileSize_v2 ( const KSysFile_v2 *self, ctx_t ctx )
{
    INTERNAL_ERROR ( xcFunctionUnsupported, "function is not supported" );
    return 0;
}


/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
void CC KSysDiskFileSetSize_v2 ( KSysFile_v2 *self, ctx_t ctx, uint64_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcUpdating );

    rc_t rc;
    LARGE_INTEGER p;
    uint64_t prev_size;

/*     KOutMsg ("%s:\n",__func__); */
    /* get previous size for setting or clearing sparse */
    rc = get_file_size ( self, &prev_size);
    if (rc)
        return;
/*     KOutMsg ("%s: %lu\n",__func__, prev_size); */

    p . QuadPart = size;

    if ( ! SetFilePointerEx ( self -> handle, p, & p, FILE_BEGIN ) )
        rc = 1;
    else
    {
        self -> pos = size;
        if ( ! SetEndOfFile( self -> handle ) )
            rc = 1;
    }

    /* failure to set size*/
    if ( rc != 0 )
    {
        DWORD last_error;

        last_error = GetLastError ();
        switch ( last_error  )
        {
        case ERROR_INVALID_HANDLE:
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error handle = %zu", self->handle );
            break;
        default:
            INTERNAL_ERROR ( xcErrorUnknown, "error accessing file system status" );
        }
        return;
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
}


static
void CC KSysFileSetSize_v2 ( KSysFile_v2 *self, ctx_t ctx, uint64_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcUpdating );

    INTERNAL_ERROR ( xcFunctionUnsupported, "function is not supported" );
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
size_t KSysFileReadCommon_v2 ( const KSysFile_v2 *cself, ctx_t ctx, 
    void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    DWORD to_read, bytes_read;
    KSysFile_v2 *self = ( KSysFile* ) cself;
    
    to_read = ( DWORD ) bsize;
    if ( sizeof bsize > sizeof to_read && ( size_t ) to_read != bsize )
        to_read = ~ 0U;

    while ( 1 )
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
                    INTERNAL_ERROR ( xcErrorUnknown, "error reading system file" );
                    return 0;
                }
           }
        }
        
        self -> pos += bytes_read;
        break;
    }

    return bytes_read;
}

static
size_t CC KSysDiskFileRead_v2 ( const KSysFile_v2 *cself, ctx_t ctx, uint64_t pos,
    void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    KSysFile_v2 *self = ( KSysFile* ) cself;

    if ( self -> pos != pos )
    {
        LARGE_INTEGER p;

        if ( !GetFileSizeEx( self -> handle, &p ) )
        {
            DWORD last_error;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_INVALID_HANDLE:
                INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error handle = %zu", self->handle );
                break;
            default:
                INTERNAL_ERROR ( xcErrorUnknown, "error positioning  system file" );
                break;
            }
            return 0;
        }

        /* if we try to read beyond the end of the file... */
        if ( ( LONGLONG ) pos >= p . QuadPart )   
        {   /* We've defined reading beyond EOF as return RC of 0 but bytes read as 0 */
            /*return RC ( rcFS, rcFile, rcPositioning, rcFileDesc, rcInvalid );*/
            INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error handle" );
            return 0;
        }

        p . QuadPart = pos;
        if ( !SetFilePointerEx ( self -> handle, p, & p, FILE_BEGIN ) )
        {
            DWORD last_error;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_INVALID_HANDLE:
                INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error handle = %zu", self->handle );
                break;
            default:
                INTERNAL_ERROR ( xcErrorUnknown, "error positioning  system file" );
                break;
            }
            return 0;
        }

        self -> pos = p . QuadPart;
        if ( pos != p . QuadPart )
        {
            if ( pos > (uint64_t)( p . QuadPart ) )
            {
                INTERNAL_ERROR ( xcErrorUnknown, "positioning error while writing to system file" );
                return 0;
            }

            INTERNAL_ERROR ( xcErrorUnknown, "error positioning  system file" );
            return 0;
        }
    }

    return KSysFileReadCommon_v2 ( cself, ctx, buffer, bsize );
}

static
size_t CC KSysFileRead_v2 ( const KSysFile_v2 *cself, ctx_t ctx,  uint64_t pos,
    void *buffer, size_t bsize )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcReading );

    KSysFile_v2 *self = ( KSysFile* ) cself;

    if ( self -> pos != pos )
    {
        INTERNAL_ERROR ( xcFileDescInvalid, "incorrect file descriptor  handle = %zu", self->handle );
        return 0;
    }

    return KSysFileReadCommon_v2 ( cself, ctx, buffer, bsize );
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
size_t KSysFileWriteCommon_v2 ( KSysFile_v2 *self, ctx_t ctx,
    const void *_buffer, size_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

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
            DWORD last_error;

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
                INTERNAL_ERROR ( xcFileDescInvalid, "invalid system file  handle = %zu", self->handle );
                break;

            case ERROR_NOT_ENOUGH_MEMORY:
                INTERNAL_ERROR ( xcNoMemory, "out of memory for WindowWriteFile" );
                break;

            default:
                INTERNAL_ERROR ( xcErrorUnknown, "error writing  system file" );
                break;
            }
            return 0;
        }

        buffer += iter_writ;
        to_write -= iter_writ;
        bytes_writ += iter_writ;
    }

    self -> pos += bytes_writ;

    return (size_t)bytes_writ;
}

static
size_t CC KSysDiskFileWrite_v2 ( KSysFile_v2 *self, ctx_t ctx, uint64_t pos,
    const void *buffer, size_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    rc_t rc;
    if ( self -> pos != pos )
    {
        LARGE_INTEGER p;
        uint64_t curr_size;

        rc = get_file_size ( self, &curr_size );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcErrorUnknown, "failed to get file size" );
            return 0;
        }

        if ( curr_size < pos )
        {
            KSysDiskFileSetSize_v2 (self, ctx, pos);
            if ( FAILED () )
            {
                INTERNAL_ERROR ( xcErrorUnknown, "failed to set file size" );
                return 0;
            }
        }


        p . QuadPart = pos;

        if ( SetFilePointerEx ( self -> handle, p, & p, FILE_BEGIN ) == 0 )
        {
            DWORD last_error;

            last_error = GetLastError ();
            switch ( last_error )
            {
            case ERROR_INVALID_HANDLE:
                INTERNAL_ERROR ( xcFileDescInvalid, "invalid system file  handle = %zu", self->handle );
                break;
            default:
                INTERNAL_ERROR ( xcErrorUnknown, "error writing  system file" );
                break;
            }
            return 0;
        }

        self -> pos = p . QuadPart;
        if ( pos != p . QuadPart )
        {
            INTERNAL_ERROR ( xcErrorUnknown, "positioning error while writing to system file" );
        }
    }

    return KSysFileWriteCommon_v2 ( self, ctx, buffer, size );
}
static
size_t CC KSysFileWrite_v2 ( KSysFile_v2 *self, ctx_t ctx, uint64_t pos,
    const void *buffer, size_t size )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcWriting );

    if ( self -> pos != pos )
    {
        INTERNAL_ERROR ( xcFileDescInvalid, "incorrect file descriptor  handle = %zu", self->handle );
        return 0;
    }

    return KSysFileWriteCommon_v2 ( self, ctx, buffer, size );
}


/* Make
 *  create a new file object
 *  from file descriptor
 */
static const KFile_vt_v2 vtKSysDiskFile =
{
    /* version 2.0 */
    2, 0,

    /* start minor version 0 methods */
    KSysFileDestroy_v2,
    KSysFileGetSysFile_v2,
    KSysDiskFileRandomAccess_v2,
    KSysDiskFileSize_v2,
    KSysDiskFileSetSize_v2,
    KSysDiskFileRead_v2,
    KSysDiskFileWrite_v2,
    KSysFileType_v2

};
static const KFile_vt_v2 vtKSysOtherFile =
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

};
static const KFile_vt_v2 vtKSysStdIODiskFile =
{
    /* version 2.0 */
    2, 0,

    /* start minor version 0 methods */
    KSysStdIOFileDestroy_v2,
    KSysFileGetSysFile_v2,
    KSysDiskFileRandomAccess_v2,
    KSysDiskFileSize_v2,
    KSysDiskFileSetSize_v2,
    KSysDiskFileRead_v2,
    KSysDiskFileWrite_v2,
    KSysFileType_v2

};
static const KFile_vt_v2 vtKSysStdIOOtherFile =
{
    /* version 2.0 */
    2, 0,

    /* start minor version 0 methods */
    KSysStdIOFileDestroy_v2,
    KSysFileGetSysFile_v2,
    KSysFileRandomAccess_v2,
    KSysFileSize_v2,
    KSysFileSetSize_v2,
    KSysFileRead_v2,
    KSysFileWrite_v2,
    KSysFileType_v2

};


static
KSysFile_v2 * KSysFileMakeVT_v2 ( ctx_t ctx, HANDLE fd, const KFile_vt *vt, const char *path,
    uint64_t initial_pos, bool read_enabled, bool write_enabled )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    KSysFile_v2 *f;

    if( fd == INVALID_HANDLE_VALUE ) /* ? */
    {
        INTERNAL_ERROR ( xcFileDescInvalid, "bad file descriptor error fd = %zu", fd );
        return NULL;
    }


    f = malloc ( sizeof *f );
    if ( f == NULL )
    {
        SYSTEM_ERROR ( xcNoMemory, "out of memory" );
        return NULL;
    }
    else
    {
        TRY ( KFileInit ( & f -> dad, ctx, vt, "KSysFile", path, read_enabled, write_enabled ) )
        {
            f -> handle = fd;
            f -> pos = initial_pos;
            f -> failed_set_sparse = f->is_sparse = false;
            check_if_sparse (f);
            return f;
        }

        free ( f );
    }
    return NULL;
}

/* bit flags */
#define ISDISK  1
#define ISSTDIO 2

static
KSysFile_v2 * KSysFileMakeInt_v2 ( ctx_t ctx, HANDLE fd, const char *path, bool read_enabled, bool write_enabled, unsigned flags )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    DWORD ret;
    const KFile_vt * vt;
    uint64_t initial_pos;


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
            INTERNAL_ERROR ( xcErrorUnknown, "error unknown file type" );
            return NULL;

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

    return KSysFileMakeVT_v2 ( ctx, fd, vt, path, initial_pos, read_enabled, write_enabled );
}

/* extern, but internal to libkfs */
KSysFile_v2 * KSysFileMake_v2 ( ctx_t ctx, HANDLE fd, const char *path, bool read_enabled, bool write_enabled )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    return  KSysFileMakeInt_v2 ( ctx, fd, path, read_enabled, write_enabled, 0 );
}


/* MakeStdIn
 *  creates a read-only file on stdin
 */
LIB_EXPORT const KFile_v2 * CC KFileMakeStdIn_v2 ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    HANDLE fd = GetStdHandle ( STD_INPUT_HANDLE );
    return ( KFile_v2 * ) KSysFileMakeInt_v2 ( ctx, fd, "stdin", true, false, ISSTDIO );
}

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only file on stdout or stderr
 */
LIB_EXPORT const KFile_v2 * CC KFileMakeStdOut_v2 ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    HANDLE fd = GetStdHandle ( STD_OUTPUT_HANDLE );
    return ( const KFile_v2 * ) KSysFileMakeInt_v2 ( ctx, fd, "stdout", false, true, ISSTDIO );
}

LIB_EXPORT const KFile_v2 * CC KFileMakeStdErr_v2 ( ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcFS, rcFile, rcConstructing );

    HANDLE fd = GetStdHandle ( STD_ERROR_HANDLE );
    return ( const KFile_v2 * ) KSysFileMakeInt_v2 ( ctx, fd, "stderr", false, true, ISSTDIO );
}

/* MakeFDFile
 *  creates a file from a file-descriptor
 *  not supported under Windows
 */
LIB_EXPORT const KFile_v2 * CC KFileMakeFDFileRead_v2 ( ctx_t ctx, int fd )
{
    INTERNAL_ERROR ( xcFunctionUnsupported, "function is not supported" );

    return NULL;
}

LIB_EXPORT KFile_v2 * CC KFileMakeFDFileWrite_v2 ( ctx_t ctx, bool update, int fd )
{
    INTERNAL_ERROR ( xcFunctionUnsupported, "function is not supported" );

    return NULL;
}
