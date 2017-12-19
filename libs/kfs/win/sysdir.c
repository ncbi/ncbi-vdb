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
struct KSysDir;

#define UNICODE 1
#define _UNICODE 1

#define KDIR_IMPL struct KSysDir

#include "sysfile-priv.h"
#include <klib/namelist.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>
#include <klib/klib-priv.h>
#include <klib/time.h>

#include <sysalloc.h>

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <assert.h>
#include <WINDOWS.H>
#include <WINNT.H>

#include "lnk_tools.c"

#include <os-native.h>

#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK 0xA000000C
#endif


/* Missing functions from our text library
 * size is bytes; max_chars is number of elements
 */

/* utf16_utf32
 *  converts UTF16 text to a single UTF32 character
 *  returns the number of UTF16 words consumed, such that:
 *    return > 0 means success
 *    return == 0 means insufficient input
 *    return < 0 means bad input or bad argument
 */
static
int utf16_utf32 ( uint32_t *dst, const wchar_t *begin, const wchar_t *end )
{
    uint32_t ch;

    if ( dst == NULL || begin == NULL || end == NULL )
        return -1;

    if ( begin == end )
        return 0;

    /* windows utf16 */

    ch = (uint32_t)(begin [0]);

    if ((ch < 0xD800) || (ch <= 0xE000))
    {
        *dst = ch;
        return 1;
    }
    else
    {
        uint32_t ch;

        /* need at least 2 words */
        if (begin >= end)
            return -1;

        /* extreme checks */
        if (((begin[0] & 0xFC00) != 0xD8) ||
            ((begin[1] & 0xFC00) != 0xDC))
            return -1;

        ch = (begin[0] & 0x03FF) << 10 |
            (begin[1] & 0x03FF);
        return 2;
    }
}


/* utf32_utf16
 *  converts a single UTF32 character to UTF16 text
 *  returns the number of UTF16 words generated, such that:
 *    return > 0 means success
 *    return == 0 means insufficient output
 *    return < 0 means bad character or bad argument
 */
static 
int utf32_utf16 ( wchar_t *begin, wchar_t *end, uint32_t ch )
{
    if (ch < 0x10000)
    {
        if ((ch <= 0xDFFF) && (ch >= 0xD800))
            return -1;

        begin[0] = (uint16_t)ch;
        return 1;
    }
    else if ((ch >= 0x10FFFF) || (end <= begin))
        return -1;
    else
    {
        uint32_t cch;

        cch = ch - 0x10000;
        /* cch <= 0xFFFFF since ch < 0x10FFFF */

        begin[0] = 0xD800 | (cch >> 10); /* upper 10 bits */
        begin[1] = 0xDC00 | (cch & 0x3FF); /* lower 10 bita */
        return 2;
    }
}


static int wstrcase_cmp (const wchar_t * a, size_t asize,
                         const wchar_t * b, size_t bsize,
                         uint32_t max_chars)
{
    uint32_t num_chars;
    const wchar_t *aend, *bend;

    assert ( a != NULL && b != NULL );

    /* set up end limit triggers */
    aend = a + asize;
    bend = b + bsize;

    num_chars = 0;

    while ( a < aend && b < bend )
    {
        uint32_t ach, bch;

        /* read a character from a */
        int len = utf16_utf32 ( & ach, a, aend );
        if ( len <= 0 )
        {
            asize -= ( size_t ) ( aend - a );
            break;
        }
        a += len;

        /* read a character from b */
        len = utf16_utf32 ( & bch, b, bend );
        if ( len <= 0 )
        {
            bsize -= ( size_t ) ( bend - b );
            break;
        }
        b += len;

        /* compare characters with case */
        if ( ach != bch )
        {
            /* only go lower case if they differ */
            ach = towlower ( ( wint_t ) ach );
            bch = towlower ( ( wint_t ) bch );

            if ( ach != bch )
            {
                if ( ach < bch )
                    return -1;
                return 1;
            }
        }

        /* if char count is sufficient, we're done */
        if ( ++ num_chars == max_chars )
            return 0;
    }

    /* one or both reached end < max_chars */
    if (asize < bsize)
        return -1;
    return asize > bsize;
}

/*--------------------------------------------------------------------------
 * KSysDirEnum
 *  a Windows directory enumerator
 */
typedef struct KSysDirEnum KSysDirEnum;
struct KSysDirEnum
{
    HANDLE handle;
    WIN32_FIND_DATAW fd;
    int found;
    bool first;
};

/* Whack
 */
static
void KSysDirEnumWhack ( KSysDirEnum *self )
{
    FindClose( self->handle );
}

/* Init
 */
static
rc_t KSysDirEnumInit ( KSysDirEnum *self, const wchar_t *path )
{
    uint32_t err;
    rc_t rc;

    self -> first = true;
    self -> handle = FindFirstFileW ( path, & self -> fd );
    if ( self -> handle != INVALID_HANDLE_VALUE )
    {
        self -> found = 1;
        return 0;
    }

    self -> found = 0;
    err = GetLastError ();
    switch ( err )
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        rc = RC ( rcFS, rcDirectory, rcListing, rcPath, rcNotFound );
        break;
    default :
        rc = RC ( rcFS, rcDirectory, rcListing, rcNoObj, rcUnknown );
    }

    PLOGERR ( klogInfo,
             ( klogInfo, rc, "error FindFirstFileW - $(E) - $(C)",
              "E=%!,C=%u", err, err ) ); 

    return rc;
}


static
rc_t KSysDirEnumInitAll ( KSysDirEnum *self, wchar_t *path, uint32_t path_length )
{
    /* prepare the path for KSysDirEnumInit() */
    path [ path_length + 0 ] = '\\';
    path [ path_length + 1 ] = '*';
    path [ path_length + 2 ] = '.';
    path [ path_length + 3 ] = '*';
    path [ path_length + 4 ] = 0;

    return KSysDirEnumInit ( self, path );
}

/* Next
 */
static
const wchar_t *KSysDirEnumNext ( const KSysDirEnum *cself )
{
    KSysDirEnum* self = (KSysDirEnum*)cself;

    while( self->found )
    {
        if ( self -> first )
            self -> first = false;
        else
            self->found = FindNextFileW( self->handle, &self->fd );

        if ( self->found )
        {
            /* filter out the '.' and '..' entries */
            if ( self ->fd.cFileName[ 0 ] == '.' )
            {
                switch ( self->fd.cFileName[ 1 ] )
                {
                case 0:
                    continue;
                case '.':
                    if ( self->fd.cFileName[ 2 ] == 0 )
                        continue;
                    break; 
                }
            }
            return self->fd.cFileName;
        }
    }

    return NULL;
}


/*--------------------------------------------------------------------------
 * KSysDirListing
 *  a Windows directory listing
 */
typedef VNamelist KSysDirListing;

static
int KSysDirListingSort ( const void *a, const void *b )
{
    size_t A,B,M;
    A = wchar_string_size (a);
    B = wchar_string_size (b);
    /* close enough for max chars? */
    M = (A>B) ? A : B;

    return wstrcase_cmp (a, A, b, B, ( uint32_t ) M);
}

static
rc_t KSysDirListingInit ( KSysDirListing *self, const wchar_t *path, const KDirectory *dir,
    bool ( CC * f ) ( const KDirectory*, const char*, void* ), void *data )
{
    KSysDirEnum list;
    rc_t rc = KSysDirEnumInit ( & list, path );
    if ( rc == 0 )
    {
        const wchar_t *name;
        char utf8_name[ MAX_PATH ];
        size_t utf8_size, utf16_size;

        while ( ( name = KSysDirEnumNext ( & list ) ) != NULL )
        {
            utf16_size = wchar_string_size ( name );
            utf8_size = wchar_cvt_string_copy ( utf8_name, sizeof( utf8_name ), name, utf16_size );
            if ( utf8_size >= sizeof( utf8_name ) )
            {
                rc = RC(rcFS, rcDirectory, rcListing, rcName, rcExcessive );
                break;
            }

            if ( f != NULL )
            {
                if ( ! ( * f ) ( dir, utf8_name, data ) )
                    continue;
            }
            
            rc = VNamelistAppend( self, utf8_name );
            if ( rc != 0 )
            {
                break;
            }

        }

        KSysDirEnumWhack ( & list );
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * KSysDir
 *  a Windows directory
 */
typedef struct KSysDir KSysDir;
struct KSysDir
{
    KDirectory dad;
    uint32_t root;
    uint32_t length;
    wchar_t path [ MAX_PATH ];
};


/* helper function to translate a windows-error-code into rc-code */
static
rc_t translate_file_error( DWORD error, enum RCContext ctx )
{
    rc_t rc;
    switch ( error )
    {
        case ERROR_FILE_NOT_FOUND :
        case ERROR_PATH_NOT_FOUND :
        case ERROR_INVALID_DRIVE :
            rc = RC ( rcFS, rcDirectory, ctx, rcPath, rcNotFound ); break;

        case ERROR_ALREADY_EXISTS:
        case ERROR_FILE_EXISTS :
            rc = RC ( rcFS, rcDirectory, ctx, rcPath, rcExists ); break;

    /*    case ERROR_PATH_NOT_FOUND : */
        case ERROR_INVALID_NAME :
        case ERROR_BAD_PATHNAME :
            rc = RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid ); break;

        case ERROR_ACCESS_DENIED :
        case ERROR_INVALID_ACCESS :
        case ERROR_SHARING_VIOLATION :
        case ERROR_LOCK_VIOLATION :
        case ERROR_PATH_BUSY :
        case ERROR_WRITE_PROTECT :
        case ERROR_DELETE_PENDING :
            rc = RC ( rcFS, rcDirectory, ctx, rcDirectory, rcUnauthorized ); break;

        case ERROR_NOT_ENOUGH_MEMORY :
        case ERROR_OUTOFMEMORY :
            rc = RC ( rcFS, rcDirectory, ctx, rcMemory, rcExhausted ); break;

        case ERROR_TOO_MANY_OPEN_FILES :
            rc = RC ( rcFS, rcDirectory, ctx, rcFileDesc, rcExhausted ); break;

        case ERROR_HANDLE_DISK_FULL :
            rc = RC ( rcFS, rcDirectory, ctx, rcStorage, rcExhausted ); break;

        case ERROR_BUFFER_OVERFLOW :
        case ERROR_FILENAME_EXCED_RANGE :
            rc = RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        
        default : RC ( rcFS, rcDirectory, ctx, rcNoObj, rcUnknown );
    }
    return rc;
}


/* helper */

static rc_t print_error_for( DWORD error, const wchar_t * path, const char * function, enum RCContext ctx, KLogLevel level )
{
    rc_t rc = translate_file_error( error, ctx );
#if _DEBUGGING
    char buffer[ 4096 ];
    size_t src_size, dst_size, len;
    wchar_cvt_string_measure ( path, &src_size, &dst_size );
    len = wchar_cvt_string_copy ( buffer, sizeof buffer, path, src_size );
    buffer[ len ] = 0;
    PLOGERR ( level,
              ( level, rc, "error $(F) - $(E) - $(C) for $(D)",
                "F=%s,E=%!,C=%u,D=%s", function, error, error, buffer ) ); 
#endif
    return rc;
}


static void wchar_2_char( const wchar_t * path, char * buffer, size_t buflen )
{
    size_t src_size, dst_size, len;
    wchar_cvt_string_measure ( path, &src_size, &dst_size );
    len = wchar_cvt_string_copy ( buffer, buflen, path, src_size );
    buffer[ len ] = 0;
}


static
uint32_t KSysDirPathTypeFromFindData ( WIN32_FIND_DATA *find_data, 
                                       const wchar_t * path,
                                       const uint32_t type )
{
    uint32_t res = type;

    if( ( find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
    {
        res = kptDir;
    }
    else if ( ( find_data->dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) != 0 )
    {
        res = kptCharDev;
    }

    /* add in alias bit */
    if ( ( find_data->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) != 0 )
    {
        if ( ( find_data->dwReserved0 & IO_REPARSE_TAG_SYMLINK ) != 0 )
            res |= kptAlias;
    }
    else
    {
        if ( has_lnk_extension( path ) ) /* lnk_tools.c */
            if ( lnk_file_validate( path ) ) /* lnk_tools.c */
                res |= kptAlias;
    }
    return res;
}

static
uint32_t KSysDirResolvePathAndDetectPathType ( const wchar_t *path )
{
    uint32_t res = kptNotFound;
    wchar_t *resolved;
    if ( win_resolve_path( path, &resolved, 1 ) ) /* lnk_tools.c */
    {
        WIN32_FIND_DATA find_data;
        HANDLE f_findfile = FindFirstFileW( resolved, &find_data );
        if ( f_findfile != INVALID_HANDLE_VALUE )
        {
            FindClose( f_findfile );
            res = KSysDirPathTypeFromFindData ( &find_data, resolved, kptFile );
        }
        else
        {
            uint32_t err = GetLastError();
            switch( err )
            {
            case ERROR_BAD_NETPATH:
            case ERROR_BAD_NET_NAME:
                /* see if the netpath is a server
                   NB - our special wcsdup allocated extra space for this */
                wcscat ( resolved, L"\\*" );
                f_findfile = FindFirstFileW( resolved, &find_data );
                if ( f_findfile != INVALID_HANDLE_VALUE )
                {
                    FindClose ( f_findfile );
                    res = kptDir;
                    break;
                }
                /* no break */
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                res = kptNotFound;
                break;

            default:
                res = kptBadPath;
            }
        }
        free( resolved );
    }
    return res;
}

/* KSysDirPathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
static
uint32_t KSysDirFullFSPathType ( const wchar_t * path )
{
    WIN32_FIND_DATA find_data;
    HANDLE f_findfile = FindFirstFileW( path, &find_data );
    if ( f_findfile == INVALID_HANDLE_VALUE )
    {
        DWORD status = GetLastError ();
        switch( status )
        {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_BAD_NETPATH:
        case ERROR_BAD_NET_NAME:
            /* try to follow the path, section by section
               if a section cannot be found try to resolve it as
               MS-shell-link ( .lnk file ) */
            return KSysDirResolvePathAndDetectPathType ( path );
        default:
            DBGMSG ( DBG_KFS, DBG_FLAG_ANY, ( "FindFirstFileW: WARNING - unrecognized return code - %u.\n", status ) );
            print_error_for( status, path, "FindFirstFileW", rcResolving, klogErr );
            return kptBadPath;
        }
    }
    FindClose( f_findfile );
    return KSysDirPathTypeFromFindData ( &find_data, path, kptFile );
}


static
uint32_t KSysDirFullPathType ( const wchar_t *path )
{
    /* recognize of odd, POSIX'ish patterns and handle them directly */
    if ( path [ 0 ] == '/' || path [ 0 ] == '\\' )
    {
        /* looking exactly for "root" */
        if ( path [ 1 ] == 0 )
            return kptFakeRoot;
    }

	/* regognize this 'c:\' as a valid path...*/
	if ( iswalpha( path[ 0 ] ) && path [ 1 ] == ':' && path [ 2 ] == '\\' && path[ 3 ] == 0 )
	{
		uint32_t path_type = kptBadPath;
		uint32_t mask = 0;
		if ( path[ 0 ] >= 'A' && path[ 0 ] <= 'Z' )
		{
			mask = ( 1 << ( path[ 0 ] - 'A' ) );
		}
		else if ( path[ 0 ] >= 'a' && path[ 0 ] <= 'z' )
		{
			mask = ( 1 << ( path[ 0 ] - 'a' ) );
		}
		if ( mask > 0 )
		{
			DWORD drivebitmask = GetLogicalDrives();	/* each logical drive has its own bit set */
			if ( ( drivebitmask & mask ) == mask )
				path_type = kptDir;
		}
		return path_type;
	}
	
    /* let the file system tell us */
    return KSysDirFullFSPathType ( path );
}


/* KSysDirMake
 *  allocate an uninialized object
 */
static
KSysDir *KSysDirMake ( size_t path_size )
{
    KSysDir *dir = malloc ( sizeof *dir - sizeof dir->path +
        4 * sizeof dir -> path [ 0 ] + path_size );
    return dir;
}


/* KSysDirDestroy
 */
static
rc_t CC KSysDirDestroy ( KSysDir *self )
{
    free ( self );
    return 0;
}

/* KSysDirCanonPath
 */
static
rc_t KSysDirCanonPath ( const KSysDir *self, enum RCContext ctx, wchar_t *path, uint32_t path_length )
{
    wchar_t *low, *dst, *last, *end = path + path_length;

    if ( self -> root != 0 )
        low = path + self -> root;
    else if ( path [ 1 ] == ':' )
        low = path + 2;
    else
        low = path;
    dst = last = low;

    while( 1 )
    {
        wchar_t *src = wcschr ( last + 1, '\\' );
        if ( src == NULL )
            src = end;

        /* detect special sequences */
        switch ( src - last )
        {
        case 1: 
            if ( last [ 1 ] == '\\' && last != path ) /* keep leading double slash */
            {
                /* "\\\\" -> "\\" */
                last = src;
            }
            break;

        case 2:
            if ( last [ 1 ] == '.' )
            {
                /* skip over */
                last = src;
                if ( src != end )
                    continue;
            }
            break;

        case 3:
            if ( last [ 1 ] == '.' && last [ 2 ] == '.' )
            {
                /* remove previous leaf in path */
                dst [ 0 ] = 0;
                dst = wcsrchr ( path, '\\' );
                if ( dst == NULL || dst < low )
                    return RC( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

                last = src;
                if ( src != end )
                    continue;
            }
            break;
        }

        /* if rewriting, copy leaf */
        assert ( src >= last );

        /* if rewriting, copy leaf */
        if ( dst != last )
            memmove ( dst, last, ( src - last ) * sizeof * dst );

        /* move destination ahead */
        dst += src - last;
        
        /* if we're done, go */
        if ( src == end )
            break;

        /* find next separator */
        last = src;
    }

    /* NUL terminate if modified */
    if ( dst != end )
        *dst = 0;

    return 0;
}

/* KSysDirMakePath
 *  creates a full path from partial

 self ....... has the first (base) part of the path in wchar_t !!!
 canon ...... if true the assembled path will be "canonilized" as last step
 buffer ..... into this buffer the full-path will be assembled ( wchar_t !!! )
 path_max ... the size of the buffer in bytes
 path ....... the partial path in utf8, can contain string-subst-elements !!!
 args ....... arguments to construct the partial path in utf8 ( can be NULL )
 */
static
rc_t KSysDirMakeSimulatedFSPath ( const KSysDir* self, enum RCContext ctx, bool canon,
    wchar_t *buffer, size_t path_max, const char *path, va_list args, bool fake_posix )
{
    int temp_size_in_bytes;
    uint32_t i, temp_length_in_utf8_chars;
    uint32_t buffer_length_in_wchars;
    char temp_utf8_buffer [ MAX_PATH ];

    /* check if the given partial path is not NULL and not empty */
    if( path == NULL )
        return RC( rcFS, rcDirectory, ctx, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

    /* We construct in temp_utf8_buffer the relative path the user has given:
    If there are no args we copy with _snprintf else we use vsnprintf,
    !!! the args are always utf8, the given path is in utf8 !!!
    that is the reason for the temporary utf8-buffer */
    temp_size_in_bytes = ( args == NULL ) ?
        _snprintf ( temp_utf8_buffer, sizeof temp_utf8_buffer, "%s", path ):
        vsnprintf( temp_utf8_buffer, sizeof temp_utf8_buffer, path, args );

    /* we check if _snprnitf/vsnprintf was sucessful */
    if ( temp_size_in_bytes < 0 || temp_size_in_bytes >= sizeof temp_utf8_buffer )
        return RC( rcFS, rcDirectory, ctx, rcPath, rcExcessive );

    /* we measure the number of utf8-chars we have in our temp-buffer
        only for international chars in the temp-buffer there will be
        path_length_in_utf8_chars != path_size_in_bytes */
    temp_length_in_utf8_chars = string_len ( temp_utf8_buffer, temp_size_in_bytes );

    /* normally we don't receive native Windows paths here.
       but there is a use below ( when creating native directory )
       that feeds a Windows path, so deal with it here. */
    if ( ( isalpha ( temp_utf8_buffer [ 0 ] ) && temp_utf8_buffer [ 1 ] == ':' ) ||
         ( temp_utf8_buffer [ 0 ] == '\\' && temp_utf8_buffer [ 1 ] == '\\' )    ||
         ( temp_utf8_buffer [ 0 ] == '/' && temp_utf8_buffer [ 1 ] == '/' ) )
    {
    /* in the case the path is a absolute path for windows (starting with "C:" for instance)
        we completely ignore the path in self and use the given path only.
        !!! except we are chrooted, in this case the given path is invalid 
        ( no abs. path for chrooted dir's ) */
        if ( self -> root != 0 )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

        /* we detected a drive or UNC path - require a further character */
        if ( temp_utf8_buffer [ 2 ] == 0 )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

        buffer_length_in_wchars = 0;
    }


    /**************************************************/
    /* THESE ARE EXPECTED TO BE POSIX-STYLE PATHS NOW */
    /**************************************************/

    /* relative path to directory */
    else if ( temp_utf8_buffer [ 0 ] != '/' )
    {
        /* copy base of path from self */
        assert ( self -> length >= 3 );
        buffer_length_in_wchars = self -> length;
    }
    else
    {
        /* POSIX full path, should include drive letter or UNC slashes */

        /* get chroot'd path length */
        buffer_length_in_wchars = self -> root;

        /* if the full path includes a drive letter */
        if ( isalpha ( temp_utf8_buffer [ 1 ] ) && temp_utf8_buffer [ 2 ] == '/' )
        {
            /* fail if chroot'd */
            if ( self -> root != 0 )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

            /* rewrite drive letter */
            temp_utf8_buffer [ 0 ] = tolower ( temp_utf8_buffer [ 1 ] );
            temp_utf8_buffer [ 1 ] = ':';
        }
        /* detect UNC path */
        else if ( temp_utf8_buffer [ 1 ] == '/' )
        {
            /* fail if chroot'd */
            if ( self -> root != 0 )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

            /* fail if just '//' */
            if ( temp_utf8_buffer [ 2 ] == 0 )
            {
                if ( ! fake_posix )
                    return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

                temp_utf8_buffer [ 1 ] = 0;
                temp_length_in_utf8_chars = 1;
                temp_size_in_bytes = 1;
            }
        }
        else if ( self -> root == 0 )
        {
            /* this is a "full" path that does not appear to be convertible
               to a Windows full path, unless we are chroot'd */
            if ( ! fake_posix )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

            /* allow path like "/C" */
            if ( isalpha ( temp_utf8_buffer [ 1 ] ) && temp_utf8_buffer [ 2 ] == 0 )
            {
                temp_utf8_buffer [ 0 ] = tolower ( temp_utf8_buffer [ 1 ] );
                temp_utf8_buffer [ 1 ] = ':';
                temp_utf8_buffer [ 2 ] = '/';
                temp_utf8_buffer [ 3 ] = 0;
                temp_length_in_utf8_chars = 3;
                temp_size_in_bytes = 3;
            }
        }
        else
        {
            /* this needs to be a valid UNC or drive path */
            assert ( self -> root >= 3 );
        }
    }

    /* check for buffer overrun */
    if ( buffer_length_in_wchars + temp_length_in_utf8_chars >= path_max / sizeof * buffer )
        return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );

    /* prepend UTF-16 directory path */
    if ( buffer_length_in_wchars != 0 )
    {
        memmove ( buffer, self -> path, buffer_length_in_wchars * sizeof * buffer );

        /* if path is relative, expect trailing '\\'
            if path is full, expect NO trailing '\\' */
        assert ( ( buffer_length_in_wchars == self ->length &&
                    buffer [ buffer_length_in_wchars - 1 ] == '\\' ) ||
                 ( buffer_length_in_wchars == self ->root &&
                   buffer [ buffer_length_in_wchars - 1 ] != '\\' ) );
    }

    /* append the temp_utf8_buffer to the  user-supplied relative path */
    buffer_length_in_wchars += (uint32_t)string_cvt_wchar_copy ( & buffer [ buffer_length_in_wchars ],
        path_max - buffer_length_in_wchars * sizeof buffer [ 0 ],
        temp_utf8_buffer, temp_size_in_bytes );
    /* the job of the temp_utf8_buffer is done now... */

    /* detect exhausted buffer */
    if ( buffer_length_in_wchars >= path_max / sizeof * buffer )
        return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );

    /* must be either:
       1) a full drive-letter path, or
       2) an UNC path.
       minimum path length is 3 */
    assert ( buffer_length_in_wchars >= 3 || fake_posix );
    assert ( buffer[ buffer_length_in_wchars ] == 0 );

    if ( buffer_length_in_wchars > 3 )
    {
        /* remove any trailing slash added by caller */
        while ( buffer_length_in_wchars > 3 && buffer [ buffer_length_in_wchars - 1 ] == '/' )
            buffer [ -- buffer_length_in_wchars ] = 0;
    }

    /* convert forward to backward slashes */
    for ( i = 0; i < buffer_length_in_wchars; ++ i )
    {
        if ( buffer [ i ] == '/' )
            buffer [ i ] = '\\';
    }

    /* if caller wants canonical representation
       or I'm chrooted, rewrite */
    if ( buffer_length_in_wchars > 2 && ( canon || self -> root > 2 ) )
    {
        return KSysDirCanonPath ( self, ctx, buffer, buffer_length_in_wchars );
    }

    return 0;
}

static
rc_t KSysDirMakePath ( const KSysDir* self, enum RCContext ctx, bool canon,
    wchar_t *buffer, size_t path_max, const char *path, va_list args )
{
    return KSysDirMakeSimulatedFSPath ( self, ctx, canon, buffer, path_max, path, args, false );
}

LIB_EXPORT rc_t KSysDirOSPath ( const KSysDir *self,
    wchar_t *real, size_t real_size, const char *path, va_list args )
{
    return KSysDirMakePath ( self, rcLoading, true, real, real_size, path, args );
}

/* KSysDirInit - forward declaration
 */
static
rc_t KSysDirInit ( KSysDir *self, enum RCContext ctx, uint32_t dad_root,
                   const wchar_t *path, size_t path_size, uint32_t path_length, 
                   bool update, bool chroot );

/* KSysDirList
 *  create a directory listing
 *
 *  "list" [ OUT ] - return parameter for list object
 *
 *  "path" [ IN, NULL OKAY ] - optional parameter for target
 *  directory. if NULL, interpreted to mean "."
 */
static
rc_t CC KSysDirList ( const KSysDir *self, KNamelist **listp,
    bool ( CC * f ) ( const KDirectory *dir, const char *name, void *data ), void *data,
                   const char *path, va_list args )
{
    KSysDir full;
    rc_t rc = KSysDirMakePath ( self, rcListing, true, full.path, sizeof full.path, path, args );
    if ( rc == 0 )
    {
        size_t size_in_bytes;
        uint32_t len_in_chars = utf16_string_measure( full.path, &size_in_bytes );

        /* require space for a '\\*.*' and NUL */
        if ( len_in_chars + 5 > sizeof full.path / sizeof full . path [ 0 ] )
            rc = RC ( rcFS, rcDirectory, rcListing, rcPath, rcExcessive );
        else
        {
            rc = KSysDirInit( &full, rcListing, self->root, NULL, size_in_bytes, len_in_chars, 0, 0 );
            if ( rc == 0 )
            {
                KSysDirListing *list;

                len_in_chars = full.length;
                full . path [ len_in_chars + 0 ] = '*';
                full . path [ len_in_chars + 1 ] = '.';
                full . path [ len_in_chars + 2 ] = '*';
                full . path [ len_in_chars + 3 ] = 0;

                rc = VNamelistMake ( &list, 5 ); 
                if ( rc == 0 )
                {
                    rc = KSysDirListingInit( list, full.path, & full.dad, f, data );
                    if ( rc != 0 )
                    {
                        VNamelistRelease ( list );
                    }
                    else
                    {
                        rc = VNamelistToNamelist ( list, listp );
                        VNamelistRelease ( list );
                    }
                }
            }
        }
    }
    return rc;
}

static
uint32_t CC KSysDirPathType ( const KSysDir *self, const char *path, va_list args )
{
    wchar_t full[ MAX_PATH ];
    rc_t rc = KSysDirMakePath( self, rcAccessing, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        return KSysDirFullPathType( full );
    }
    return kptBadPath;
}

/* KSysDirVisit
 *  visit each path under designated directory,
 *  recursively if so indicated
 *
 *  "recurse" [ IN ] - if non-zero, recursively visit sub-directories
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - function to execute
 *  on each path. receives a base directory and relative path
 *  for each entry, where each path is also given the leaf name
 *  for convenience. if "f" returns non-zero, the iteration will
 *  terminate and that value will be returned. NB - "dir" will not
 *  be the same as "self".
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
typedef struct KSysDirVisitData KSysDirVisitData;
struct KSysDirVisitData
{
    rc_t ( CC * f ) ( KDirectory*, uint32_t, const char*, void* );
    void *data;
    KSysDir dir;
    bool recurse;
};

static
rc_t KSysDirVisitDir ( KSysDirVisitData *pb )
{
    /* get a directory listing */
    rc_t rc;
    KSysDirEnum listing;
    uint32_t path_length;
    size_t path_size;

    /* measure length and size of the given path, we will need both... */
    path_length = wchar_string_measure ( pb->dir.path, &path_size );

    /* add a trailing backslash (windows!) if it is not there... */
    if ( pb->dir.path[ path_length - 1 ] != '\\' )
    {
        /* check if there is space for another character */
        if ( ( path_size + sizeof pb->dir.path [ 0 ] ) >= sizeof pb->dir.path )
        {
            return RC( rcFS, rcDirectory, rcVisiting, rcPath, rcExcessive );
        }
        pb->dir.path[ path_length + 0 ] = '\\';
        pb->dir.path[ path_length + 1 ] = 0;
        ++ path_length;
        path_size += sizeof pb->dir.path[ 0 ];
        pb->dir.length = path_length;
    }

    /* check if there is space for 6 more bytes ( '*.*' ) */
    if ( ( path_size + 3 * sizeof pb->dir.path[ 0 ] ) >= sizeof pb->dir.path )
    {
        return RC( rcFS, rcDirectory, rcVisiting, rcPath, rcExcessive );
    } 
    /* append '*.*' to make KSysDirEnumInit work under Windows! */
    pb -> dir . path [ path_length + 0 ] = '*';
    pb -> dir . path [ path_length + 1 ] = '.';
    pb -> dir . path [ path_length + 2 ] = '*';
    pb -> dir . path [ path_length + 3 ] = 0;

    rc = KSysDirEnumInit ( &listing, pb->dir.path );
    if( rc == 0 )
    {
        const wchar_t *name;

        /* truncate the appended '*.*' to visit the entries */
        pb -> dir . path [ path_length ] = 0;

        for ( name = KSysDirEnumNext( &listing ); 
              name != NULL; 
              name = KSysDirEnumNext( &listing ) )
        {
            uint32_t type, name_length;
            size_t name_size;
            char temp_utf8_buffer [ MAX_PATH ];

            /* measure length and size of the element-name, we will need both... */
            name_length = wchar_string_measure ( name, &name_size );
            /* check if we have enought space for path and element-name */
            if ( path_size + name_size >= sizeof pb->dir.path )
            {
                rc = RC( rcFS, rcDirectory, rcVisiting, rcPath, rcExcessive );
                break;
            }

            /* append the element-name to the path */
            wcscpy ( &pb->dir.path[ path_length ], name );
            type = KSysDirFullPathType( pb->dir.path );
            if( type == kptBadPath )
            {
                rc = RC( rcFS, rcDirectory, rcVisiting, rcPath, rcInvalid );
                break;
            }

            /* the callback-function expects the name as utf8 !!! */
            wchar_cvt_string_copy ( temp_utf8_buffer, sizeof temp_utf8_buffer, 
                                    name, name_size );
            rc = (*pb->f)( &pb->dir.dad, type, temp_utf8_buffer, pb->data );
            if ( rc != 0 )
                break;

            /* if recursive visiting is requested and the element is a directory */
            if ( pb->recurse && ( type & ( kptAlias - 1 ) ) == kptDir )
            {
                /* append the element-name-length temporary to the length of the path */
                pb->dir.length += name_length;
                /* call this function recursive */
                rc = KSysDirVisitDir( pb );
                /* restore the original path-length (for the caller function) */
                pb->dir.length = path_length;
                if ( rc != 0 )
                    break;
            }

        } /* for () */

        KSysDirEnumWhack( &listing );
    }
    return rc;
}


static
rc_t Enumerate_DriveLetters( const KSysDir *self, 
	rc_t ( CC * f ) ( KDirectory *dir, uint32_t type, const char *name, void *data ), void *data )
{
	rc_t rc = 0;
	DWORD drivebitmask = GetLogicalDrives();	/* each logical drive has its own bit set */
	if ( drivebitmask == 0 )
		rc = translate_file_error( GetLastError(), rcListing );
	else
	{
		uint32_t i, n, mask = 1;
		for ( i = 0; i < 26 && rc == 0; ++i, mask <<= 1 )
		{
			if ( ( drivebitmask & mask ) == mask )
			{
				char drive[ 5 ];
				drive[ 0 ] = 'A' + i;
				drive[ 1 ] = 0;
				rc = f( ( KDirectory * ) self, kptDir, ( const char * )drive, data );
			}
		}
	}
	return rc;
}


static
rc_t CC KSysDirVisit ( const KSysDir *self, bool recurse,
    rc_t ( CC * f ) ( KDirectory *dir, uint32_t type, const char *name, void *data ), void *data,
    const char *path, va_list args )
{
    KSysDirVisitData pb;
    rc_t rc = KSysDirMakeSimulatedFSPath( self, rcVisiting, true, pb.dir.path, sizeof pb.dir.path, path, args, true );
    if ( rc == 0 )
    {
        size_t path_size;
        uint32_t path_length;

		uint32_t path_type = KSysDirFullPathType( pb.dir.path );
        switch( path_type & ( kptAlias - 1 ) )
        {
            case kptNotFound:
                return RC( rcFS, rcDirectory, rcVisiting, rcPath, rcNotFound );
            case kptBadPath:
                return RC( rcFS, rcDirectory, rcVisiting, rcPath, rcInvalid );
            case kptDir:
                break;
            case kptFakeRoot:
				return Enumerate_DriveLetters( self, f, data );
				
			/* call code to enumerate drives */
            default:
                return RC( rcFS, rcDirectory, rcVisiting, rcPath, rcIncorrect );
        }

        path_length = utf16_string_measure( pb.dir.path, &path_size );
        rc = KSysDirInit ( & pb . dir, rcVisiting, self -> root,
                           NULL, path_size, path_length,
                           self -> dad . read_only ? 0 : 1, 0 );
        if ( rc == 0 )
        {
            pb . f = f;
            pb . data = data;
            pb . recurse = recurse;
            rc = KSysDirVisitDir ( & pb );
        }
    }
    return rc;
}

/* KSysDirRelativePath
 *  makes "path" relative to "root"
 *  both "root" and "path" MUST be absolute
 *  both "root" and "path" MUST be canonical, i.e. have no "./" or "../" sequences
 *  both root and path are in windows-native format!
 */
static
rc_t KSysDirRelativePath ( const KSysDir *self, enum RCContext ctx,
    const wchar_t *root, wchar_t *path, size_t path_max )
{
    size_t psize;
    uint32_t backup, blength_in_chars, dst, diff_from_here;

    const wchar_t *r = root + self->root;
    const wchar_t *p = path + self->root;

    /* stop gap fix..  not actually comparing the utf16 values correctly */
    for ( ; towlower (*r) == towlower (*p); ++ r, ++ p )
    {
        /* disallow identical paths */
        if ( * r == 0 )
            return RC( rcFS, rcDirectory, ctx, rcPath, rcInvalid );
    }

    /* paths are identical up to "r","p"
       if "r" is within a leaf name, then no backup is needed
       by counting every '\\' from "r" to end, obtain backup count */
    for ( backup = 0; * r != 0; ++ r )
    {
        if ( * r == '\\' )
            ++ backup;
    }

    /* the number of characters to be inserted */
    blength_in_chars = backup * 3;

    /* align "p" to last directory separator */
    if ( p > path ) {
        while ( p [ -1 ] != '\\' ) -- p;
    }

    /* the size of the remaining relative path */
    psize = wcslen ( p );
    diff_from_here = ( uint32_t )( p - path );

    /* open up space if needed */
    if ( diff_from_here < blength_in_chars )
    {
        /* prevent overflow */
        if ( ( blength_in_chars + psize ) * sizeof( *path ) >= path_max )
            return RC( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        memmove ( & path[ blength_in_chars ], p, psize * ( sizeof *p ) );
    }

    /* insert backup sequences */
    for ( dst = 0; backup > 0; -- backup )
    {
        path [ dst++ ] = '.';
        path [ dst++ ] = '.';
        path [ dst++ ] = '\\';
    }

    /* close gap */
    if ( diff_from_here > blength_in_chars )
        wcscpy ( & path [ blength_in_chars ], p );
    path[ blength_in_chars + psize ] = 0;

    return 0;
}

/* KSysDirResolvePath
 *  resolves path to an absolute or directory-relative path
 *
 *  "absolute" [ IN ] - if non-zero, always give a path starting
 *  with '/'. NB - if the directory is chroot'd, the absolute path
 *  will still be relative to directory root.
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - buffer for
 *  NUL terminated result path in directory-native character sets
 *  the resolved path will be directory relative
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target path. NB - need not exist.
 */
static
rc_t CC KSysDirResolvePath ( const KSysDir *self, bool absolute,
    char *resolved, size_t rsize, const char *path, va_list args )
{
    wchar_t temp [ MAX_PATH ];
    size_t temp_size;
    uint32_t temp_length;

    /* convert the utf8-input-parameter path into wchar_t */
    rc_t rc = KSysDirMakePath ( self, rcResolving, true, temp, sizeof temp, path, args );
    if ( rc != 0 )
        return rc;

    temp[ 0 ] = tolower( temp[ 0 ] ); /* this is important:
                                         otherwise the comparison for is_on_same_drive_letter fails
                                         AND
                                         KSysDirRelativePath() fails too! */

    temp_length = wchar_string_measure ( temp, &temp_size );
    if ( absolute )
    {
        /* test buffer capacity */
        if ( temp_length - self->root >= rsize )
            return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
    }
    else
    {
        /* we are on windows, only if the path has a drive letter and it is the same
           one as in KSysDir itself, we should try to create a relative path */
        wchar_t colon = ':';
        bool is_on_same_drive_letter = ( iswascii ( temp[ 0 ] ) && iswascii ( self->path[ 0 ] ) &&
                                         ( temp[ 1 ] == colon ) && ( self->path[ 1 ] == colon ) &&
                                         ( towlower ( temp[ 0 ] ) == towlower ( self->path[ 0 ] ) ) );
        if ( is_on_same_drive_letter )
        {
            rc = KSysDirRelativePath( self, rcResolving, self->path, temp, sizeof temp );
            if ( rc == 0 )
            {
                uint32_t temp_length = wchar_string_measure ( temp, &temp_size );
                if ( temp_length >= rsize )
                    return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
            }
        }
        else
        {
            /* treat it as if absolute were requested ( see above ) */
            if ( temp_length - self->root >= rsize )
                return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        }
    }

    if ( rc == 0 )
    {
        uint32_t i;    
        /* convert it back to utf8 */
        utf16_cvt_string_copy ( resolved, rsize, temp, temp_size );

        /* convert it back to POSIX */
        if ( isalpha ( resolved[ 0 ] ) && resolved[ 1 ] == ':' )
        {
            /* rewrite drive letter */
            resolved[ 1 ] = tolower ( resolved [ 0 ] );
            resolved[ 0 ] = '/';
        }
        
        /* convert backward to forward slashes */
        for ( i = 0; resolved[ i ]; ++ i )
        {
            if ( resolved[ i ] == '\\' )
                resolved[ i ] = '/';
        }
    }

    return rc;
}

/* KSysDirResolveAlias
 *  resolves an alias path to its immediate target
 *  NB - the resolved path may be yet another alias
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting an object presumed to be an alias.
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - buffer for
 *  NUL terminated result path in directory-native character set
 */
static
rc_t CC KSysDirResolveAlias ( const KSysDir *self, bool absolute,
                              char *resolved, size_t rsize,
                              const char *alias, va_list args )
{
    KSysDir temp;
    size_t temp_size;
    uint32_t temp_length, path_type;
    wchar_t * w_resolved;

    rc_t rc = KSysDirMakePath( self, rcResolving, true, temp.path, sizeof temp.path, alias, args );
    if ( rc != 0 )
        return rc;

    temp_length = wchar_string_measure ( temp.path, &temp_size );
    path_type = KSysDirFullPathType ( temp.path );
    if ( path_type == kptFile || path_type == kptDir )
    {
        /* if the path points to a file or a dir, then there is no alias involved at all */
        if ( temp_size >= rsize )
        {
            return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        }
        /* we have to convert temp.path back from wchar_t to char ! */
        wchar_cvt_string_copy ( resolved, rsize, temp.path, temp_size );
        return 0;
    }


    /* trying to attach a .lnk to the path, if it resolves it is a link... */
    if ( temp_size + 10 >= rsize )
    {
        return RC( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
    }
    temp.path[ temp_length + 0 ] = '.';
    temp.path[ temp_length + 1 ] = 'l';
    temp.path[ temp_length + 2 ] = 'n';
    temp.path[ temp_length + 3 ] = 'k';
    temp.path[ temp_length + 4 ] = 0;

    if ( lnk_file_resolve( temp.path, &w_resolved ) != LNK_RES_ERROR )
    {
        size_t w_size;
        uint32_t w_len;

        /* we have to copy the resolved path into temp to use KSysDirCanonPath() */
        w_len = wchar_string_measure ( w_resolved, &w_size );
        if ( w_size > sizeof temp.path )
        {
            free( w_resolved );
            return RC( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        }
        wcscpy( temp.path, w_resolved );
        free( w_resolved );

        rc = KSysDirCanonPath( &temp, rcResolving, temp.path, w_len );
        if ( rc == 0 )
        {
            /* the path in full is an absolute path
               if outside of chroot, it's a bad link */
            if (wstrcase_cmp (temp.path, self->root + 1,
                              self->path, self->root + 1,self->root + 1) != 0)
                return RC( rcFS, rcDirectory, rcResolving, rcLink, rcInvalid );

            /* this is the absolute path length */
            w_len = wchar_string_measure ( temp.path, &w_size );

            /* if not requesting absolute, make self relative */
            if( !absolute )
            {
                rc = KSysDirRelativePath( self, rcResolving, self->path, 
                                          temp.path, w_len );
                if ( rc != 0 )
                    return rc;
                w_len = wchar_string_measure ( temp.path, &w_size );
            }
            if ( ( size_t ) w_len >= rsize )
                return RC(rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );

            w_len = wchar_string_measure ( &(temp.path[ self -> root ]), &w_size );
            wchar_cvt_string_copy ( resolved, rsize, &(temp.path[ self -> root ]), w_size );
        }
    }


#if 0
    /* NEXT - attach ".lnk" to the path and see if it resolves
       if not, the supplied path simply does not exist */
        if ( ( wcslen( full.path ) + 5 ) >= rsize )
        {
            return RC( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        }
        wcscpy( resolved, full.path );
        wcscat( resolved, L".lnk" );
        refnum = FindFirstFile( resolved, &info );
        if ( refnum == INVALID_HANDLE_VALUE )
        {
            wcscpy( resolved, L"\0" );
            return RC( rcFS, rcDirectory, rcResolving, rcPath, rcNotFound );
        }
        FindClose( refnum );

        {

            /* get a COM reference to the Explorer. we should be
               able to do this in C with no problem */
            HRESULT rslt = ERROR_INSUFFICIENT_BUFFER;
            IShellLink *shellLink;
            rslt = CoCreateInstance( &IID_IShellLink, 0, CLSCTX_INPROC_SERVER, &IID_IShellLink, &shellLink );
            if( !rslt )
            {
                /* get a file interface that isn't attached to anything */
                IPersistFile *persistFile;
                rslt = shellLink->lpVtbl->QueryInterface( shellLink, &IID_IPersistFile, (void**)&persistFile );
                if( !rslt )
                {
                    /* now try to do the thing
                       the link name needs to be in Unicode */
                    rslt = persistFile->lpVtbl->Load(persistFile, resolved, STGM_READ );
                    if( !rslt )
                    {
                        /* Unicode is no longer necessary */
                        rslt = shellLink->lpVtbl->Resolve( shellLink, 0, SLR_NO_UI + SLR_ANY_MATCH );
                        if ( !rslt )
                        {
                            /* read what the path is, i.e. read the shortcut file */
                            rslt = shellLink->lpVtbl->GetPath( shellLink, resolved, rsize, &info, 0 );
                            if( rslt )
                            {
                                wcscpy( resolved, L"\0" );
                            }
                        }
                    }
                    persistFile->lpVtbl->Release( persistFile );
                }
                shellLink->lpVtbl->Release( shellLink );
            }
            if ( wcslen( resolved ) == 0 )
            {
                return RC( rcFS, rcDirectory, rcResolving, rcPath, rcInvalid );
            }

        }

        len = wcslen( resolved );
        if( resolved[0] == '/' )
        {
            full.size = 1;
            wcscpy( full.path, resolved );
        }
        else
        {
            wchar_t *f = wcsrchr( full.path, '/' );
            full.size = f - full.path + 1;
            if ( full.size + len >= sizeof full.path )
            {
                return RC( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
            }
            wcscpy( f, resolved );
        }
        full.root = 0;

        rc = KSysDirCanonPath( &full, rcResolving, full.path, len );
        if ( rc == 0 )
        {
            size_t f, s;

            f = wchar_string_size (full.path);
            s = wchar_string_size (self->path);
            /* the path in full is an absolute path
               if outside of chroot, it's a bad link */
            if ( wstrcase_cmp (full.path, f, self->path, s, self->root + 1 ) != 0 )
            {
                return RC( rcFS, rcDirectory, rcResolving, rcLink, rcInvalid );
            }

            /* this is the absolute path length */
            len = wchar_string_size( &full.path[self->root] );

            /* if not requesting absolute, make self relative */
            if( !absolute )
            {
            rc = KSysDirRelativePath( self, rcResolving, self->path, full.path, sizeof full.path /*len*/ );
                if ( rc != 0 )
                    return rc;
                len = wchar_string_size(full.path);
            }
            if ( ( size_t ) len >= rsize )
                return RC(rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );

            wcscpy ( resolved, & full . path [ self -> root ] );
        }
#endif
    return rc;
}

/* KSysDirRename
 *  rename an object accessible from directory, replacing
 *  any existing target object of the same type
 *
 *  "from" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 *
 *  "to" [ IN ] - NUL terminated string in directory-native
 *  character set denoting existing object
 */
static
rc_t CC KSysDirRename ( KSysDir *self, bool force, const char *from, const char *to )
{
    wchar_t current_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcRenaming, false, current_name, sizeof current_name, from, NULL );
    if ( rc == 0 )
    {

        wchar_t new_name[ MAX_PATH ];
        rc = KSysDirMakePath ( self, rcRenaming, false, new_name, sizeof new_name, to, NULL );
        if ( rc == 0 )
        {
            DWORD err = 0;
            uint32_t try = 0;

            do
            {
                BOOL success = false;
                if ( force ) {
                    DWORD dwFlags = MOVEFILE_REPLACE_EXISTING;
                    success = MoveFileEx ( current_name, new_name, dwFlags );
                }
                else {
                    success = MoveFileW ( current_name, new_name );
                }
                if ( success )
                {
                    rc = 0;
                }
                else
                {
                    err = GetLastError();
                    switch( err )
                    {
                        case ERROR_FILE_NOT_FOUND:
                        case ERROR_PATH_NOT_FOUND:
                        case ERROR_INVALID_DRIVE:
                            return RC ( rcFS, rcDirectory, rcRenaming, rcFile, rcNotFound );
                        case ERROR_ACCESS_DENIED:
                            try++;
                            KSleepMs( 500 ); /* sleep for a half a second */
                            rc = RC ( rcFS, rcDirectory, rcRenaming, rcFile, rcUnauthorized );
                            break;
                       case ERROR_SHARING_VIOLATION:
                           return RC ( rcFS, rcDirectory, rcRenaming, rcFile, rcBusy );
                        default:
                            return RC ( rcFS, rcDirectory, rcRenaming, rcNoObj, rcUnknown );
                    }
                }
            } while ( err == ERROR_ACCESS_DENIED && try < 30 );
        }
    }
    return rc;
}


/* helper function for KSysDirCreateParents() */
static
rc_t directory_exists( const wchar_t *path, bool *exists )
{
    /* try it with CreateFileW() */
    *exists = win_path_exists( path );
    return 0;
#if 0
    wchar_t temp[ MAX_PATH ];
    WIN32_FIND_DATA find_data;
    HANDLE h_find;
    size_t path_size;
    uint32_t path_length = utf16_string_measure( path, &path_size );

    *exists = false;

    if ( ( path_size + 10 ) > sizeof temp )
        return RC( rcFS, rcDirectory, rcCreating, rcMemory, rcExhausted );

    wcscpy( temp, path );
    if ( temp[ path_length - 1 ] != '\\' )
        temp[ path_length++ ] = '\\';
    temp[ path_length + 0 ] = '*';
    temp[ path_length + 1 ] = '.';
    temp[ path_length + 2 ] = '*';
    temp[ path_length + 3 ] = 0;

    h_find = FindFirstFileW( temp, &find_data );
    if ( h_find != INVALID_HANDLE_VALUE )
    {
        *exists = true;
        FindClose( h_find );
    }

    return 0;
#endif
}


static
rc_t KSysDirRemoveEntry ( wchar_t *path, size_t path_max, bool force );


static
rc_t KSysDirEmptyDir ( wchar_t *path, size_t path_max, bool force )
{
    rc_t rc;
    KSysDirEnum list;
    size_t path_size;
    uint32_t path_length = wchar_string_measure ( path, &path_size );

    if ( ( path_size + 10 ) > path_max )
        return RC( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );

    rc = KSysDirEnumInitAll ( & list, path, path_length );
    if ( rc != 0 )
    {
        rc = ResetRCContext ( rc, rcFS, rcDirectory, rcClearing );
    }
    else
    {
        const wchar_t *leaf;

        /* we keep only the appended '\\' for the loop... */
        path_length++;
        path_size += sizeof *path;

        for ( leaf = KSysDirEnumNext( &list ); 
              leaf != NULL; 
              leaf = KSysDirEnumNext( &list ) )
        {
            size_t leaf_size;
            uint32_t leaf_length = wchar_string_measure ( leaf, &leaf_size );
            if ( path_size + leaf_size >= path_max )
            {
                rc = RC ( rcFS, rcDirectory, rcClearing, rcPath, rcExcessive );
                break;
            }

            /* wcscpy adds termination, so wprintf is safe to call */
            wcscpy ( & path [ path_length ], leaf );

            rc = KSysDirRemoveEntry ( path, path_max, force );
            if ( rc != 0 )
            {
                rc = ResetRCContext ( rc, rcFS, rcDirectory, rcClearing );
                break;
            }
        }
        KSysDirEnumWhack ( & list );
        /* restore the original path... */
        path [ path_length - 1 ] = 0;
    }
    return rc;
}


/* KSysDirClearDir
 *  remove all directory contents
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "force" [ IN ] - if non-zero and directory entry is a
 *  sub-directory, remove recursively
 */
static
rc_t CC KSysDirClearDir ( KSysDir *self, bool force, const char *path, va_list args )
{
    wchar_t dir_name [ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcClearing, false, dir_name, sizeof dir_name, path, args );
    if ( rc == 0 )
        rc = KSysDirEmptyDir ( dir_name, sizeof dir_name, force );
    return rc;
}


/* KSysDirRemove
 *  remove an accessible object from its directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "force" [ IN ] - if non-zero and target is a directory,
 *  remove recursively
 */
static
rc_t KSysDirRemoveEntry ( wchar_t *path, size_t path_max, bool force )
{
    if ( !DeleteFileW( path ) )
    {
        DWORD file_error = GetLastError();

        switch ( file_error )
        {
        case ERROR_PATH_NOT_FOUND :
            return 0;
/*
        case ERROR_ACCESS_DENIED :
            !!! Do not use this error code here, it occurs if path is not a file, but
            a directory instead. Handling it here would prevent the remaining code
            from beeing executed !!!
            return RC( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );
*/

        default :
#if _DEBUGGING && 0
    OUTMSG (( "DeleteFileW returned '%#X'\n", file_error ));
#endif
            break;
        }

        /* we have not been able to delete it as a file,
           we try to delete it as a directory... */
        if ( !RemoveDirectoryW( path ) )
        {
            rc_t rc;
            DWORD error = GetLastError();

            /* find out if the reason is that it is not empty and force = true --->
               in this case delete all files and directories in it 
               and then try again... */
            switch ( error )
            {
            case ERROR_DIR_NOT_EMPTY :
                if ( force )
                {
                    rc = KSysDirEmptyDir ( path, path_max, force );
                    if ( rc == 0 )
                    {
                        if ( !RemoveDirectoryW( path ) )
                        {
                            rc = RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );
                            print_error_for( error, path, "RemoveDirectoryW", rcRemoving, klogErr );
                        }
                    }
                    return rc;
                }
                else
                    rc = RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );
                break;

            case ERROR_ACCESS_DENIED :
                rc = RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );
                break;

            case ERROR_DIRECTORY: /* not a directory */
                /* looks like it was a file after all; report the original error */
                error = file_error;
                print_error_for( file_error, path, "DeleteFileW", rcRemoving, klogInfo);
                return RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );

            default :
                rc = RC ( rcFS, rcDirectory, rcCreating, rcNoObj, rcUnknown );
                break;
            }

            print_error_for( error, path, "RemoveDirectoryW", rcRemoving, klogInfo);
            return rc;
        }
    }
    return 0;
}


static
rc_t CC KSysDirRemove ( KSysDir *self, bool force, const char *path, va_list args )
{
    wchar_t dir_name [ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcRemoving, false, dir_name, sizeof dir_name, path, args );
    if ( rc == 0 )
        rc = KSysDirRemoveEntry ( dir_name, sizeof dir_name, force );
    return rc;
}

/* KSysDirAccess
 *  get access to object
 *
 *  "access" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */

#define DEFAULT_WIN_ACCESS   0555
#define DEFAULT_WRITE_ACCESS 0222


/* FromMSDN */
#define UNIX_EPOCH_IN_WIN       116444736000000000
#define UINX_TIME_UNITS_IN_WIN  10000000
static __inline__
void KTimeToWinTime ( KTime_t unix, LPFILETIME win )
{
    uint64_t ll = ( ( unix * UINX_TIME_UNITS_IN_WIN ) + UNIX_EPOCH_IN_WIN );
    win->dwLowDateTime = (DWORD)ll;
    win->dwHighDateTime = ll >> 32;
}


static __inline__
KTime_t WinTimeToKTime ( LPFILETIME win )
{
    uint64_t ll = (uint64_t)win->dwLowDateTime + ((int64_t)win->dwHighDateTime << 32);

/*     DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_DIR),("%s %x %x %lx %lx\n", */
/*                                           __func__,win->dwLowDateTime,win->dwHighDateTime, */
/*                                           ll,( ll - UNIX_EPOCH_IN_WIN ) / 10000000)); */

    /* if its negative, so be it */
    return ( ll - UNIX_EPOCH_IN_WIN ) / UINX_TIME_UNITS_IN_WIN;
}


static __inline
rc_t get_attributes ( const wchar_t * wpath, uint32_t * access, KTime_t * date )
{
    WIN32_FIND_DATA fd;
    rc_t rc;
    HANDLE h = FindFirstFile ( wpath, &fd );
    if ( h != INVALID_HANDLE_VALUE )
    {
        if ( access != NULL )
        {
/* TBD - track user's main group and group Everyone */
            *access = DEFAULT_WIN_ACCESS |
                (((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
                 ? 0 : DEFAULT_WRITE_ACCESS);
        }
        if ( date != NULL )
        {
            *date = WinTimeToKTime ( &fd.ftLastWriteTime );
        }
        FindClose ( h );
        return 0;
    }

/* TBD check values in error */
    if ( access != NULL )
        *access = 0;
    if ( date != NULL )
        *date = 0;

    rc = print_error_for( GetLastError(), wpath, "FindFirstFile", rcAccessing, klogErr );
    return rc;
}


static
rc_t CC KSysDirVAccess ( const KSysDir *self,
    uint32_t *access, const char *path, va_list args )
{
    wchar_t winpath [ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcAccessing, false, winpath, sizeof winpath, path, args );
    if ( rc == 0 )
        rc = get_attributes ( winpath, access, NULL );
    return rc;
}

/* KSysDirSetAccess
 *  set access to object a la Unix "chmod"
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "access" [ IN ] and "mask" [ IN ] - definition of change
 *  where "access" contains new bit values and "mask defines
 *  which bits should be changed.
 *
 *  "recurse" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 */
static
rc_t KSysDirChangeAccess ( char *path, size_t path_max,
    uint32_t access, uint32_t mask, bool recurse );

    
static
rc_t KSysDirChangeDirAccess ( char *path, size_t path_max,
    uint32_t access, uint32_t mask )
{
    /*
    KSysDirEnum list;
    rc_t rc = KSysDirEnumInit ( & list, path );
    if ( rc == 0 )
    {
        bool eperm = false;
        size_t path_size = strlen ( path );
        path [ path_size ] = '/';
        if ( ++ path_size == path_max )
            rc = RC(rcFS, rcDirectory, rcUpdating, rcBuffer, rcInsufficient );
        else
        {
            const char *leaf;
            while ( ( leaf = KSysDirEnumNext ( & list ) ) != NULL )
            {
                size_t leaf_size = strlen ( leaf );
                if ( path_size + leaf_size >= path_max )
                {
                    rc = RC(rcFS, rcDirectory, rcUpdating, rcBuffer, rcInsufficient );
                    break;
                }

                strcpy ( & path [ path_size ], leaf );
                rc = KSysDirChangeAccess ( path, path_max, access, mask, 1 );
                if ( rc != 0 )
                {
                    if ( GetRCState ( rc ) != rcUnauthorized )
                        break;
                    eperm = true;
                    rc = 0;
                }
            }

            path [ path_size - 1 ] = 0;
        }

        KSysDirEnumWhack ( & list );

        if ( rc == 0 && eperm )
            rc = RC(rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    }
    return rc;
    */
    return 0;
}


static
rc_t KSysDirChangeEntryAccess ( char *path, size_t path_max,
    uint32_t access, uint32_t mask, uint32_t st_mode )
{
#if 0
    /* keep old bits
       we have no chmod in Windows - leave it blank... */
    access &= mask;
    access |= st_mode & ~ mask;

    if ( chmod ( path, access & 07777 ) != 0 )
        switch ( errno )
        {
        case EPERM:
        case EACCES:
        case EROFS:
            return RC(rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
        case ENOTDIR:
        case ELOOP:
            return RC(rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );
        case ENAMETOOLONG:
            return RC(rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
        case ENOENT:
            return RC(rcFS, rcDirectory, rcUpdating, rcPath, rcNotFound );
        case ENOMEM:
            return RC(rcFS, rcDirectory, rcUpdating, rcMemory, rcExhausted );
        default:
            return RC(rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnknown );
        }
#endif
    return 0;
}


static
rc_t KSysDirChangeAccess ( char *path, size_t path_max,
    uint32_t access, uint32_t mask, bool recurse )
{
    /*
    struct stat st;
    if ( stat ( path, & st ) != 0 ) switch ( errno )
    {
    case ENOENT:
        return RC(rcFS, rcDirectory, rcUpdating, rcPath, rcNotFound );
    case ENOTDIR:
    case ELOOP:
        return RC(rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );
    case ENAMETOOLONG:
        return RC(rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
    case EACCES:
        return RC(rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    case ENOMEM:
        return RC(rcFS, rcDirectory, rcUpdating, rcMemory, rcExhausted );
    default:
        return RC(rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnknown );
    }

    if ( recurse && S_ISDIR ( st . st_mode ) )
    {
        rc_t rc;
        uint32_t enable = access & mask;
        if ( enable != 0 )
        {
            rc = KSysDirChangeEntryAccess ( path, path_max,
                access, enable, st . st_mode );
            if ( rc != 0 )
                return rc;
        }

       rc = KSysDirChangeDirAccess ( path, path_max, access, mask );
        if ( rc == 0 )
        {
            uint32_t disable = ~ access & mask;
            if ( disable != 0 )
            {
                rc = KSysDirChangeEntryAccess ( path, path_max,
                    access, disable, st . st_mode | enable );
            }
        }
        return rc;
    }

    return KSysDirChangeEntryAccess ( path, path_max,
         access, mask, st . st_mode );
    */
    return 0;
}


static
rc_t CC KSysDirSetAccess ( KSysDir *self, bool recurse,
    uint32_t access, uint32_t mask, const char *path, va_list args )
{
    rc_t rc = 0;
    /*
    char full[MAX_PATH];
    rc_t rc = KSysDirMakePath ( self, rcUpdating, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        if ( mask == 0 )
            mask = 07777;

        rc = KSysDirChangeAccess ( full, sizeof full,
            access, mask & 07777, recurse );
    }
    */
    return rc;
}


/* make_dir()
 *  helper function that encapsulates the OS-specific call
 *  to create a directory - the return codes are used by the
 *  caller-functions to decide what to do in case of a error...
 *  the callers are: KSysDirCreateParents() and KSysDirCreateDir()
 *  special on windows: path is wchar_t and we ignore access !!!
 *  TBD: translate access into a windows security descriptor...
 *       find out the other possible ERROR_* 's produced 
 */
static
rc_t make_dir ( const wchar_t *path, uint32_t access )
{
    rc_t rc = 0;
    /* try to create the directory */
    if ( !CreateDirectoryW ( path, NULL ) )
    {
        DWORD error = GetLastError();
        rc = translate_file_error( error, rcCreating );
/*
        Do not print an error code here, it is valid that this can happen!
        rc = print_error_for( error, path, "CreateDirectoryW", rcCreating, klogErr );
*/
    }
    return rc;
}


#if OLD_CREATE_PARENTS
static
rc_t check_and_make( wchar_t *path, uint32_t access )
{
    bool exists;
    rc_t rc = directory_exists( path, &exists );
    if ( rc == 0 && !exists )
    {
        rc = make_dir ( path, access );
    }
    return rc;
}
#endif


/* KSysDirCreateParents
 *  creates missing parent directories
 *  Windows special: path is wide-char, separator is back-slash, 
 *  starts with drive-letter...
 */
static
rc_t KSysDirCreateParents ( const KSysDir *self, wchar_t *path, uint32_t access, bool strip )
{
#if ! OLD_CREATE_PARENTS
    rc_t rc;
    size_t len;
    wchar_t *p, *par = path;

    /* if directory is chroot'd, skip past root and slash */
    if ( self -> root != 0 )
        par += self -> root + 1;
    else
    {
        /* skip drive letter */
        if ( path [ 1 ] == ':' )
            par += 2;
    /* skip slashes, network or otherwise */
        while ( par [ 0 ] == '\\' )
            ++ par;
    }

    len = wcslen ( par );

    if ( ! strip )
        p = par + len;
    else
    {
        p = wcsrchr ( par, '\\' );
        if ( p == NULL )
            return 0;
        len = p - par;
    }

    while ( 1 )
    {
        /* crop string */
        p [ 0 ] = 0;

        /* try to create directory */
        rc = make_dir ( path, access );
        if ( GetRCState ( rc ) != rcNotFound )
            break;

        /* back up some more */
        p = wcsrchr ( par, '\\' );
        if ( p == NULL )
        {
            p = par + wcslen ( par );
            break;
        }
    }

    par += len;
    assert ( p != NULL );

    /* create directories from here */
    if ( rc == 0 ) while ( p < par )
    {
        p [ 0 ] = '\\';
        rc = make_dir ( path, access );
        if ( rc != 0 || ++ p >= par )
            break;
        p += wcslen ( p );
    }

    /* repair stripped path */
    if ( strip )
        par [ 0 ] = '\\';

    return rc;

#else

    rc_t rc;
    wchar_t *separator = path;
    bool finished;

    do
    {
        /* find the next separator */
        separator = wcschr( separator + 1, '\\' );

        /* we are finished, if not found */
        finished = (bool)( separator == NULL );
        if ( !finished )
        {
            /* temporary terminate at the separator */
            *separator = 0;
            rc = check_and_make( path, access );
            finished = (bool)( rc != 0 );
            /* put the terminator back in place... */
            *separator = '\\';
        }
    } while ( !finished );

    /* finally test and make the whole path... */
    rc = check_and_make( path, access );

    return rc;
#endif
}

/* KSysDirCreateAlias
 *  creates a path alias according to create mode
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target alias
 *
 *  "access" [ IN ] - standard Unix directory access mode
 *  used when "mode" has kcmParents set and alias path does
 *  not exist.
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 */
static
rc_t CC KSysDirCreateAlias ( KSysDir *self, uint32_t access, KCreateMode mode,
                             const char *targ, const char *alias )
{
    wchar_t w_target[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcCreating, true, w_target, sizeof w_target, targ, NULL );
    if ( rc == 0 )
    {
        wchar_t w_alias[ MAX_PATH ];
        rc = KSysDirMakePath ( self, rcCreating, true, w_alias, sizeof w_alias, alias, NULL );
        if ( rc == 0 )
        {
            bool alias_ok = true;
            if ( ! has_lnk_extension( w_alias ) ) /* lnk_tools.c */
                alias_ok = add_lnk_extension( w_alias, sizeof w_alias ); /* lnk_tools.c */

            if ( lnk_file_exists( w_alias ) )
            {
                DeleteFileW( w_alias );
                alias_ok = ( ! lnk_file_exists( w_alias ) );
            }

            if ( alias_ok )
            {
                /* if "alias" is relative or "self" is chroot'd,
                   "w_alias" must be made relative */
                if ( alias [ 0 ] != '/' || self -> root != 0 )
                {
                    rc = KSysDirRelativePath ( self, rcCreating, w_alias, w_target, sizeof w_target );
                    if ( rc != 0 )
                        return rc;
                }
                if ( win_CreateLink( w_target, w_alias, NULL ) ) /* lnk_tools.c */
                    rc = 0;
                else
                    rc = translate_file_error( GetLastError (), rcCreating );
            }
            else
                rc = RC ( rcFS, rcDirectory, rcCreating, rcMemory, rcExhausted ); 
        }
    }
    return rc;
}


/* KSysDirOpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
static
rc_t CC KSysDirOpenFileRead ( const KSysDir *self,
    const KFile **f, const char *path, va_list args )
{
    wchar_t file_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath( self, rcOpening, false, file_name, sizeof file_name, path, args );
    if ( rc == 0 )
    {
        HANDLE file_handle = CreateFileW( file_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( file_handle == INVALID_HANDLE_VALUE )
        {
            rc = print_error_for( GetLastError(), file_name, "CreateFileW", rcOpening, klogInfo );
        }
        else
        {
            char buffer[ MAX_PATH ];
            wchar_2_char( file_name, buffer, sizeof buffer );
            rc = KSysFileMake ( ( KSysFile** ) f, file_handle, buffer, true, false );
            if ( rc != 0 )
                CloseHandle ( file_handle );
        }
    }
    return rc;
}

/* KSysDirOpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 */
static
rc_t CC KSysDirOpenFileWrite ( KSysDir *self,
    KFile **f, bool update, const char *path, va_list args )
{
    wchar_t file_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcOpening, false, file_name, sizeof file_name, path, args );
    if ( rc == 0 )
    {
        DWORD dwDesiredAccess = update ? GENERIC_READ | GENERIC_WRITE : GENERIC_WRITE;
        HANDLE file_handle = CreateFileW( file_name, dwDesiredAccess, FILE_SHARE_READ, NULL, 
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

        if ( file_handle == INVALID_HANDLE_VALUE )
        {
            rc = print_error_for( GetLastError(), file_name, "CreateFileW", rcAccessing, klogErr );

        }
        else
        {
            char buffer[ MAX_PATH ];
            wchar_2_char( file_name, buffer, sizeof buffer );
            rc = KSysFileMake ( ( KSysFile** ) f, file_handle, buffer, update, true );
            if ( rc != 0 )
                CloseHandle ( file_handle );
        }
    }
    return rc;
}

/* KSysDirOpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 */
static
rc_t CC KSysDirOpenFileSharedWrite ( KSysDir *self,
    KFile **f, bool update, const char *path, va_list args )
{
    wchar_t file_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcOpening, false, file_name, sizeof file_name, path, args );
    if ( rc == 0 )
    {
        DWORD dwDesiredAccess = update ? GENERIC_READ | GENERIC_WRITE : GENERIC_WRITE;
        HANDLE file_handle = CreateFileW( file_name, dwDesiredAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

        if ( file_handle == INVALID_HANDLE_VALUE )
        {
            rc = print_error_for( GetLastError(), file_name, "CreateFileW", rcAccessing, klogErr );

        }
        else
        {
            char buffer[ MAX_PATH ];
            wchar_2_char( file_name, buffer, sizeof buffer );
            rc = KSysFileMake ( ( KSysFile** ) f, file_handle, buffer, update, true );
            if ( rc != 0 )
                CloseHandle ( file_handle );
        }
    }
    return rc;
}

/* KSysDirCreateFile
 *  opens a file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "update" [ IN ] - if non-zero, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 */
static
rc_t CC KSysDirCreateFile ( KSysDir *self, KFile **f, bool update,
    uint32_t access, KCreateMode cmode, const char *path_fmt, va_list args )
{
    wchar_t file_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath( self, rcCreating, true, file_name, sizeof file_name, path_fmt, args );
    if ( rc == 0 )
    {
        HANDLE file_handle;
        DWORD dwDesiredAccess = update ? GENERIC_READ | GENERIC_WRITE : GENERIC_WRITE;
        DWORD dwCreationDisposition = CREATE_ALWAYS;
        DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
        DWORD dwShareMode = FILE_SHARE_READ;

        switch ( cmode & kcmValueMask )
        {
        case kcmOpen : /* open if it exists, create if it does not exist */
            dwCreationDisposition = OPEN_ALWAYS;
            break;

        case kcmInit : /* always create, if it already exists truncate to zero */
            dwCreationDisposition = CREATE_ALWAYS;
            break;

        case kcmCreate : /* create and open only if does not already exist */
            dwCreationDisposition = CREATE_NEW;
            break;
        case kcmSharedAppend :
            dwCreationDisposition = OPEN_ALWAYS;
            dwDesiredAccess = FILE_APPEND_DATA;
            dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
            dwShareMode |= FILE_SHARE_WRITE;
            break;
        }

        file_handle = CreateFileW ( file_name, dwDesiredAccess, dwShareMode,
            NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL );
        while ( file_handle == INVALID_HANDLE_VALUE )
        {
            DWORD error;

            if ( ( cmode & kcmParents ) != 0 )
            {
                /* maybe there were missing parent directories */
                uint32_t dir_access = access |
                    ( ( access & 0444 ) >> 2 ) | ( ( access & 0222 ) >> 1 );
                KSysDirCreateParents ( self, file_name, dir_access, true );

                /* try creating the file again */
                file_handle = CreateFileW ( file_name, dwDesiredAccess, dwShareMode,
                    NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL );
                if ( file_handle != INVALID_HANDLE_VALUE )
                    break;
            }

            error = GetLastError();
            rc = translate_file_error( error, rcCreating );

            /* disabled 12/12/2012 : it prints an error message, if vdb tries to open
               the same reference-object twice via http. The lock-file for the 2nd try
               does already exist. This is not an error, just a condition. */

            /*
            PLOGERR ( klogErr,
                      ( klogErr, rc, "error CreateFileW - $(E) - $(C)",
                        "E=%!,C=%u", error, error ) ); 
            */

            /* Unix code has a special case when creating an empty file, which is
               to say, creating a directory entry without needing to write to file */
            return rc;
        }

        {
            char buffer[ MAX_PATH ];
            char path[4096];
            int size = ( args == NULL) ?
                snprintf  ( path, sizeof path, "%s", path_fmt) :
                vsnprintf ( path, sizeof path, path_fmt, args );
            if ( size < 0 || size >= ( int ) sizeof path )
                rc = RC ( rcFS, rcFile, rcCreating, rcPath, rcExcessive );
            else
            {
                wchar_2_char( file_name, buffer, sizeof buffer );
                rc = KSysFileMake ( ( KSysFile** ) f, file_handle, path, update, true );
            }
            if ( rc != 0 )
                CloseHandle ( file_handle );
        }
    }
    return rc;
}

/* KSysDirFileSize
 *  returns size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KSysDirFileSize ( const KSysDir *self,
    uint64_t *size, const char *path, va_list args )
{
    wchar_t file_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath( self, rcAccessing, false, file_name, sizeof file_name, path, args );
    if ( rc == 0 )
    {
        WIN32_FILE_ATTRIBUTE_DATA file_data;
        if ( GetFileAttributesEx ( file_name, GetFileExInfoStandard, &file_data ) )
        {
            *size = file_data.nFileSizeHigh;
            *size <<= 32;
            *size |= file_data.nFileSizeLow;
        }
        else
        {
            rc = print_error_for( GetLastError(), file_name, "GetFileAttributesEx", rcAccessing, klogErr );
        }
    }
    return rc;
}

/* KSysDirSetFileSize
 *  sets size in bytes of target file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KSysDirSetFileSize ( KSysDir *self,
    uint64_t size, const char *path, va_list args )
{
    wchar_t file_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcUpdating, false, file_name, sizeof file_name, path, args );
    if ( rc == 0 )
    {
        HANDLE file_handle = CreateFileW( file_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( file_handle != INVALID_HANDLE_VALUE )
        {
            DWORD file_pos_low, file_pos_high, file_set_res;

            file_pos_low = (DWORD)( size & 0xFFFFFFFF );
            size >>= 32;
            file_pos_high = (DWORD)( size & 0xFFFFFFFF );
            file_set_res = SetFilePointer ( file_handle, file_pos_low, (PLONG)&file_pos_high, FILE_BEGIN );
            if ( file_set_res != INVALID_SET_FILE_POINTER )
            {
                if ( SetEndOfFile ( file_handle ) )
                    rc = 0; /* success !!! */
                else
                    rc = translate_file_error( GetLastError(), rcUpdating );
            }
            CloseHandle ( file_handle );
        }
        else
        {
            rc = print_error_for( GetLastError(), file_name, "CreateFileW", rcUpdating, klogErr );
        }
    }
    return rc;
}

/* KSysDirOpenDirRead
 * KSysDirOpenDirUpdate
 *  opens a sub-directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "chroot" [ IN ] - if non-zero, the new directory becomes
 *  chroot'd and will interpret paths beginning with '/'
 *  relative to itself.
 */
static
rc_t CC KSysDirOpenDirRead ( const KSysDir *self,
     const KDirectory **subp, bool chroot, const char *path, va_list args )
{
    wchar_t dir_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcOpening, true, dir_name, sizeof dir_name, path, args );
    if ( rc == 0 )
    {
        int t;
        KSysDir *sub;

        size_t dir_size;
        uint32_t dir_length = utf16_string_measure( dir_name, &dir_size );
        uint32_t length_org = dir_length;
        while ( dir_length > 0 && dir_name [ dir_length - 1 ] == '/' )
            dir_name [ -- dir_length ] = 0;
        if ( dir_length != length_org )
            dir_length = utf16_string_measure( dir_name, &dir_size );

        t = KSysDirFullPathType ( dir_name ) & ( kptAlias - 1 );
        if ( t == kptNotFound )
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNotFound );
        if ( t != kptDir )
            return RC(rcFS, rcDirectory, rcOpening, rcPath, rcIncorrect );

        sub = KSysDirMake ( dir_size );
        if ( sub == NULL )
            rc = RC(rcFS, rcDirectory, rcOpening, rcMemory, rcExhausted );
        else
        {
            rc = KSysDirInit ( sub, rcOpening, self -> root, dir_name, 
                               dir_size, dir_length, false, chroot );
            if ( rc == 0 )
            {
                * subp = & sub -> dad;
                return 0;
            }

            free ( sub );
        }
    }
    return rc;
}

static
rc_t CC KSysDirOpenDirUpdate ( KSysDir *self,
    KDirectory **subp, bool chroot, const char *path, va_list args )
{
    wchar_t dir_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcOpening, true, dir_name, sizeof dir_name, path, args );
    if ( rc == 0 )
    {
        KSysDir *sub;

        size_t dir_size;
        uint32_t dir_length = utf16_string_measure( dir_name, &dir_size );
        uint32_t length_org = dir_length;
        while ( dir_length > 0 && dir_name [ dir_length - 1 ] == '/' )
            dir_name [ -- dir_length ] = 0;
        if ( dir_length != length_org )
            dir_length = utf16_string_measure( dir_name, &dir_size );

        switch ( KSysDirFullPathType ( dir_name ) )
        {
        case kptNotFound:
            return RC( rcFS, rcDirectory, rcOpening, rcPath, rcNotFound );
        case kptBadPath:
            return RC( rcFS, rcDirectory, rcOpening, rcPath, rcInvalid );
        case kptDir:
        case kptDir | kptAlias:
            break;
        default:
            return RC( rcFS, rcDirectory, rcOpening, rcPath, rcIncorrect );
        }

        sub = KSysDirMake ( dir_size );
        if ( sub == NULL )
            rc = RC( rcFS, rcDirectory, rcOpening, rcMemory, rcExhausted );
        else
        {
            rc = KSysDirInit ( sub, rcOpening, self -> root, dir_name, 
                               dir_size, dir_length, true, chroot );
            if ( rc == 0 )
            {
                * subp = & sub -> dad;
                return 0;
            }

            free ( sub );
        }
    }
    return rc;
}

/* KSysDirCreateDir
 *  create a sub-directory
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target directory
 *
 *  "access" [ IN ] - standard Unix directory permissions
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 */
static
rc_t CC KSysDirCreateDir ( KSysDir *self,
    uint32_t access, KCreateMode mode, const char *path, va_list args )
{
    wchar_t dir_name[ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcCreating, true, dir_name, sizeof dir_name, path, args );
    if ( rc == 0 )
    {
        if ( ( mode & kcmValueMask ) == kcmCreate )
        {
            switch ( KSysDirFullPathType ( dir_name ) )
            {
            case kptNotFound:
                break;
            case kptBadPath:
                return RC(rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );
            case kptDir:
                return RC(rcFS, rcDirectory, rcCreating, rcDirectory, rcExists );
            default:
                return RC(rcFS, rcDirectory, rcCreating, rcPath, rcIncorrect );
            }
        }
        rc = make_dir ( dir_name, access );
        if ( rc != 0 )
        {
            switch ( GetRCState ( rc ) )
            {
            case rcExists:
                rc = 0;
                if ( ( mode & kcmValueMask ) == kcmInit )
                    rc = KSysDirEmptyDir ( dir_name, sizeof dir_name, 1 );
                break;
            case rcNotFound:
                if ( ( mode & kcmParents ) != 0 )
                    rc = KSysDirCreateParents ( self, dir_name, access, false );
                break;
            }
        }
    }
    return rc;
}

/* KSysDirDate
 *  get access to object
 *
 *  "date" [ OUT ] - return parameter for Unix access mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
static
rc_t CC KSysDirVDate ( const KSysDir *self,
    KTime_t * date, const char *path, va_list args )
{
    wchar_t full [ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcAccessing, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        rc = get_attributes ( full, NULL, date );
    }
    return rc;
}


static
rc_t change_item_date( wchar_t *path, LPFILETIME win_time, bool dir_flag )
{
    rc_t rc;
    HANDLE file_handle;

    if ( dir_flag )
        file_handle = CreateFileW( path, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                                   OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
    else
        file_handle = CreateFileW( path, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                                   OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( file_handle == INVALID_HANDLE_VALUE )
    {
        rc = print_error_for( GetLastError(), path, "CreateFileW", rcUpdating, klogErr );
    }
    else
    {
        if ( SetFileTime ( file_handle, NULL, NULL, win_time ) )
        {
            rc = 0;
        }
        else
        {
            rc = print_error_for( GetLastError(), path, "SetFileTime", rcUpdating, klogErr );
        }
        CloseHandle ( file_handle );
    }

    return rc;
}


static
rc_t change_dir_date( wchar_t *path, size_t path_max, LPFILETIME win_time, bool recurse )
{
    KSysDirEnum list;
    const wchar_t *leaf;
    size_t path_size;
    uint32_t path_length;

    rc_t rc = change_item_date( path, win_time, true );
    if ( rc != 0 || !recurse )
        return rc;

    path_length = wchar_string_measure ( path, &path_size );
    if ( ( path_size + 10 ) > path_max )
        return RC( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );

    rc = KSysDirEnumInitAll ( & list, path, path_length );
    if ( rc != 0 )
        return ResetRCContext ( rc, rcFS, rcDirectory, rcUpdating );

    /* we keep only the appended '\\' for the loop... */
    path_length++;
    path_size += sizeof *path;

    for ( leaf = KSysDirEnumNext( &list ); 
          leaf != NULL && rc == 0; 
          leaf = KSysDirEnumNext( &list ) )
    {
        size_t leaf_size;
        int32_t path_type;
        uint32_t leaf_length = wchar_string_measure ( leaf, &leaf_size );
        if ( path_size + leaf_size >= path_max )
            rc = RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
        else
        {

            /* wcscpy adds termination, so wprintf is safe to call */
            wcscpy ( & path [ path_length ], leaf );

            path_type = KSysDirFullPathType ( path );
            switch( path_type )
            {
            case kptFile : rc = change_item_date( path, win_time, false );
                           break;
            case kptDir  : rc = change_dir_date( path, path_max, win_time, true );
                           break;
            }
        }
    }

    KSysDirEnumWhack ( & list );
    /* restore the original path... */
    path [ path_length - 1 ] = 0;

    return rc;
}


static
rc_t KSysDirChangeDate ( wchar_t *path, size_t path_max,
            KTime_t date, bool recurse )
{
    FILETIME win_time;
    int32_t path_type;
    rc_t rc;

    KTimeToWinTime ( date, &win_time );
    path_type = KSysDirFullPathType ( path );
    switch( path_type )
    {
    case kptFile : rc = change_item_date( path, &win_time, false );
                   break;

    case kptDir  : rc = change_dir_date( path, path_max, &win_time, recurse );
                   break;

    default      : rc = RC( rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnsupported );
                   break;
    }
    return rc;
}    


/*
    struct stat st;
    struct utimbuf u;

    if ( stat ( path, & st ) != 0 ) switch ( errno )
    {
    case ENOENT:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNotFound );
    case ENOTDIR:
    case ELOOP:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );
    case ENAMETOOLONG:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
    case EACCES:
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    case ENOMEM:
        return RC ( rcFS, rcDirectory, rcUpdating, rcMemory, rcExhausted );
    default:
        return RC ( rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnknown );
    }
    u . actime = u . modtime = date;

    if ( recurse && S_ISDIR ( st . st_mode ) )
    {
        rc_t rc;

    rc = KSysDirChangeEntryDate ( path, path_max, & u );
    if ( rc != 0 )
        return rc;

    rc = KSysDirChangeDirDate ( path, path_max, date );
        if ( rc == 0 )
        {
        rc = KSysDirChangeEntryDate ( path, path_max, & u  );
        }
        return rc;
    }

    return  KSysDirChangeEntryDate ( path, path_max, & u );
*/

static
rc_t CC KSysDirVSetDate ( KSysDir * self, bool recurse,
    KTime_t date, const char *path, va_list args )
{
    wchar_t full [ MAX_PATH ];
    rc_t rc = KSysDirMakePath ( self, rcUpdating, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        rc = KSysDirChangeDate ( full, sizeof full, date, recurse );
    }
    return rc;
}

static
KSysDir *CC KSysDirGetSysdir ( const KSysDir *cself )
{
    return ( KSysDir* ) cself;
}


/* FileLocator
 *  returns a 64-bit key pertinent only to the particular file
 *  system device holding that file.
 *
 *  It can be used as a form of sort key except that it is not 
 *  guaranteed to be unique.
 *
 *  "locator" [ OUT ] - return parameter for file locator
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
static
rc_t CC KSysDirFileLocator_v1 ( const KSysDir_v1 *self,
    uint64_t *locator, const char *path, va_list args )
{
    /* TBD - could return an inode or equivalent */
    assert ( locator != NULL );
    * locator = 0;
    return RC ( rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported );
}

/* FilePhysicalSize
 *  returns physical allocated size in bytes of target file.  It might
 * or might not differ from FileSize
 *
 *  "size" [ OUT ] - return parameter for file size
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
static
rc_t CC KSysDirFilePhysicalSize_v1 ( const KSysDir_v1 *self,
    uint64_t *size, const char *path, va_list args )
{
    /* TBD - can be completed */
    assert ( size != NULL );
    * size = 0;
    return RC ( rcFS, rcDirectory, rcAccessing, rcFunction, rcUnsupported );
}

/* FileContiguous
 *  returns true if the file is "contiguous".  Chunked or sparse files are not
 *  contiguous while most data files are.  Virtual generated files would likely
 *  not be contiguous.  
 *
 *  "contiguous" [ OUT ] - return parameter for file contiguous
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
static
rc_t CC KSysDirFileContiguous_v1 ( const KSysDir_v1 *self,
    bool *contiguous, const char *path, va_list args )
{
    assert ( contiguous != NULL );
    * contiguous = true;
    return 0;
}

static KDirectory_vt_v1 vtKSysDir =
{
    /* version 1.4 */
    1, 4,

    /* start minor version 0*/
    KSysDirDestroy,
    KSysDirList,

    /* the following two messages map to the same method, requiring type casting */
    ( rc_t ( CC * )  ( const KSysDir*, bool,
        rc_t ( CC * ) ( const KDirectory*, uint32_t, const char*, void* ), void*,
        const char*, va_list ) ) KSysDirVisit,
    ( rc_t ( CC * )  ( KSysDir*, bool,
        rc_t ( CC * ) ( KDirectory*, uint32_t, const char*, void* ), void*,
        const char*, va_list ) ) KSysDirVisit,

    KSysDirPathType,
    KSysDirResolvePath,
    KSysDirResolveAlias,
    KSysDirRename,
    KSysDirRemove,
    KSysDirClearDir,
    KSysDirVAccess,
    KSysDirSetAccess,
    KSysDirCreateAlias,
    KSysDirOpenFileRead,
    KSysDirOpenFileWrite,
    KSysDirCreateFile,
    KSysDirFileSize,
    KSysDirSetFileSize,
    KSysDirOpenDirRead,
    KSysDirOpenDirUpdate,
    KSysDirCreateDir,
    NULL, /* we don't track files*/
    /* end minor version 0 */

    /* start minor version 1 */
    KSysDirVDate,
    KSysDirVSetDate,
    KSysDirGetSysdir,
    /* end minor version 1 */

    /* start minor version 2 */
    KSysDirFileLocator_v1,
    /* end minor version 2 */

    /* start minor version 3 */
    KSysDirFilePhysicalSize_v1,
    KSysDirFileContiguous_v1,
    /* end minor version 3 */

    /* start minor version 4 */
    KSysDirOpenFileSharedWrite
    /* end minor version 4 */
};

/* KSysDirInit
 */
#if TRACK_REFERENCES
static
const char *convert_wide_path ( const wchar_t *path, const size_t path_size )
{
    /* copy wide string to static */
    static char static_path [ MAX_PATH ];
    wchar_cvt_string_copy ( static_path, sizeof static_path, path, path_size );

    return static_path;
}
#else
#define convert_wide_path( path, path_size ) "ignore"
#endif

static
rc_t KSysDirInit ( KSysDir *self, enum RCContext ctx, uint32_t dad_root,
                   const wchar_t *path, size_t path_size, uint32_t path_length, 
                   bool update, bool chroot )
{
    rc_t rc;
    if ( path == NULL )
    {
        rc = KDirectoryInit( &self->dad, (const KDirectory_vt*)&vtKSysDir, 
                            "KSysDir", NULL, update );
    }
    else
    {
        rc = KDirectoryInit( &self->dad, (const KDirectory_vt*)&vtKSysDir, 
                             "KSysDir", convert_wide_path ( path, path_size ), update );
    }

    if ( rc != 0 )
    {
        return ResetRCContext ( rc, rcFS, rcDirectory, ctx );
    }

    if ( path != NULL )
    {
        memmove( self->path, path, path_size );
    }

    self->root = chroot ? path_length : dad_root;
    self->length = path_length + 1;
    self->path[ path_length ] = '\\';
    self->path[ path_length + 1 ] = 0;
    return 0;
}


/* MakeFromRealPath
 *  creates a KDirectory from a Windows path
 */
rc_t KDirectoryMakeFromRealPath ( KDirectory **dirp, const wchar_t *real, bool update, bool chroot )
{
    rc_t rc;
    size_t size;
    uint32_t length = wchar_string_measure ( real, & size );
    if ( length + 4 > MAX_PATH )
        rc = RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExcessive );
    else
    {
        KSysDir *dir = KSysDirMake ( size );
        if ( dir == NULL )
            rc = RC ( rcFS, rcDirectory, rcAccessing, rcMemory, rcExhausted );
        else
        {
            rc = KSysDirInit ( dir, rcAccessing, 0, real, size, length, update, chroot );
            if ( rc == 0 )
            {
                * dirp = & dir -> dad;
                return 0;
            }
            KSysDirDestroy ( dir );
        }
    }

    * dirp = NULL;
    return rc;
}

/* KDirectoryNativeDir
 *  returns a native file-system directory node reference
 *  the directory root will be "/" and set to the native
 *  idea of current working directory
 *
 *  NB - the returned reference will be non-const, allowing
 *  modification operations to be attempted. these operations
 *  may still fail if the underlying FS disallows them.
 *
 *  "dir" [ OUT ] - return parameter for native directory
 */
extern rc_t CC ReportCWD ( const ReportFuncs *f, uint32_t indent );
extern rc_t CC ReportRedirect ( KWrtHandler* handler,
    const char* filename, bool* to_file, bool finalize );

LIB_EXPORT rc_t CC KDirectoryNativeDir ( KDirectory **dirp )
{
    rc_t rc;

    static bool latch;
    if ( ! latch )
    {
        ReportInitKFS ( ReportCWD, ReportRedirect );
        latch = true;
    }

    if ( dirp == NULL )
        rc = RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );
    else
    {
        wchar_t wd [ MAX_PATH ];
        DWORD error;
        DWORD wd_len = GetCurrentDirectoryW ( sizeof wd / sizeof wd [ 0 ], wd );
        if ( wd_len != 0 )
            return KDirectoryMakeFromRealPath ( dirp, wd, true, false );

        error = GetLastError();
        switch ( error )
        {
        case ERROR_ACCESS_DENIED:
            rc = RC ( rcFS, rcDirectory, rcAccessing, rcDirectory, rcUnauthorized );
            break;
        default:
            rc = RC ( rcFS, rcDirectory, rcAccessing, rcNoObj, rcUnknown );
        }
        PLOGERR ( klogErr,
                  ( klogErr, rc, "error GetCurrentDirectoryW - $(E) - $(C)",
                    "E=%!,C=%u", error, error ) ); 

        * dirp = NULL;
    }

    return rc;
}


/* RealPath
 *  exposes functionality of system directory
 */
LIB_EXPORT rc_t CC KSysDirRealPath ( struct KSysDir const *self,
    char *real, size_t bsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KSysDirVRealPath ( self, real, bsize, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KSysDirVRealPath ( struct KSysDir const *self,
    char *real, size_t bsize, const char *path, va_list args )
{
    /* Windows is ... challenged when it comes to answering
       this question. What is needed is to 1) convert the path
       to a Windows-style wchar path, then 2) resolve each of
       its components, etc. to come up with a real path, then
       3) rewrite the path as a UTF-8 POSIX path */
    return KSysDirResolvePath ( self, true, real, bsize, path, args );
}

LIB_EXPORT rc_t CC KDirectoryGetDiskFreeSpace_v1 ( const KDirectory * self,
    uint64_t * free_bytes_available, uint64_t * total_number_of_bytes )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );
    else {
        KSysDir_v1 * dir = ( KSysDir_v1 * ) self;

	LPCTSTR lpszMultibyte = dir -> path;

	unsigned __int64 i64FreeBytesToCaller = 0;
	unsigned __int64 i64TotalBytes = 0;
	unsigned __int64 i64FreeBytes = 0;

	if ( GetDiskFreeSpaceEx (lpszMultibyte,
	    ( PULARGE_INTEGER ) & i64FreeBytesToCaller,
            ( PULARGE_INTEGER ) & i64TotalBytes,
            ( PULARGE_INTEGER ) & i64FreeBytes ) )
	{
            if ( free_bytes_available != NULL ) {
                * free_bytes_available  = i64FreeBytes;
            }
            if ( total_number_of_bytes != NULL ) {
                * total_number_of_bytes = i64TotalBytes;
            }
            return 0;
        }

        return RC ( rcFS, rcDirectory, rcAccessing, rcError, rcUnknown );
    }
}
