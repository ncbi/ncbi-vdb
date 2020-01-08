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

#define TRACK_REFERENCES 0

/*--------------------------------------------------------------------------
 * forwards
 */
struct KSysDir_v1;
struct KSysDirListing;

#define KDIR_IMPL struct KSysDir_v1
#define KNAMELIST_IMPL struct KSysDirListing

#include <kfs/extern.h>
#include "sysdir-priv.h"
#include "sysfile-priv.h"
#include <klib/sort.h>
#include <klib/impl.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/klib-priv.h>
#include <sysalloc.h>

#include "os-native.h"

#ifndef __USE_UNIX98
#define __USE_UNIX98 1
#endif
#include <unistd.h>

/* old Sun includes won't define PATH_MAX */
#ifndef __XOPEN_OR_POSIX
#define __XOPEN_OR_POSIX 1
#endif

#include <limits.h>

/* now they won't define lstat */
#undef __XOPEN_OR_POSIX

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h> /* statvfs */
#include <utime.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KSysDirEnum
 *  a Unix directory enumerator
 */
typedef struct KSysDirEnum KSysDirEnum;
struct KSysDirEnum
{
    DIR *dir;
};

/* Whack
 */
static
void KSysDirEnumWhack ( KSysDirEnum *self )
{
    closedir ( self -> dir );
}

/* Init
 */
static
rc_t KSysDirEnumInit ( KSysDirEnum *self, const char *path )
{
    self -> dir = opendir ( path );
    if ( self -> dir != NULL )
        return 0;

    switch ( errno )
    {
    case EACCES:
        return RC ( rcFS, rcDirectory, rcListing, rcDirectory, rcUnauthorized );
    case EMFILE:
    case ENFILE:
        return RC ( rcFS, rcDirectory, rcListing, rcFileDesc, rcExhausted );
    case ENOENT:
        return RC ( rcFS, rcDirectory, rcListing, rcPath, rcNotFound );
    case ENOMEM:
        return RC ( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );
    case ENOTDIR:
        return RC ( rcFS, rcDirectory, rcListing, rcPath, rcIncorrect );
    }

    return RC ( rcFS, rcDirectory, rcListing, rcNoObj, rcUnknown );
}

/* Next
 */
static
const char *KSysDirEnumNext ( const KSysDirEnum *self )
{
    while ( 1 )
    {
        struct dirent *e = readdir ( self -> dir );
        if ( e == NULL )
            break;

        if ( e -> d_name [ 0 ] == '.' )
        {
            switch ( e -> d_name [ 1 ] )
            {
            case 0:
                continue;
            case '.':
                if ( e -> d_name [ 2 ] == 0 )
                    continue;
                break;
            }
        }

        return e -> d_name;
    }

    return NULL;
}


/*--------------------------------------------------------------------------
 * KSysDirListing
 *  a Unix directory listing
 */
typedef struct KSysDirListing KSysDirListing;
struct KSysDirListing
{
    KNamelist dad;
    const char **namelist;
    int cnt;
};

/* Whack
 */
static
rc_t KSysDirListingWhack ( const KSysDirListing *self )
{
    int i;
    for ( i = 0; i < self -> cnt; ++ i )
        free ( ( void* ) self -> namelist [ i ] );
    free ( self -> namelist );
    return 0;
}

static
rc_t KSysDirListingDestroy ( KSysDirListing *self )
{
    rc_t rc = KSysDirListingWhack ( self );
    if ( rc == 0 )
        free ( self );
    return rc;
}

/* Count
 */
static
rc_t KSysDirListingCount ( const KSysDirListing *self, uint32_t *count )
{
    * count = self -> cnt;
    return 0;
}

/* Get
 */
static
rc_t KSysDirListingGet ( const KSysDirListing *self, uint32_t idx, const char **name )
{
    if ( idx >= ( uint32_t ) self -> cnt )
        return RC ( rcFS, rcNamelist, rcAccessing, rcParam, rcExcessive );
    * name = self -> namelist [ idx ];
    return 0;
}

/* Init
 */
static KNamelist_vt_v1 vtKSysDirListing =
{
    /* version 1.0 */
    1, 0,

    /* start minor version 0 methods */
    KSysDirListingDestroy,
    KSysDirListingCount,
    KSysDirListingGet
    /* end minor version 0 methods */
};

static
int64_t KSysDirListingSort ( const void *a, const void *b, void * ignored )
{
    return strcmp ( * ( const char** ) a, * ( const char** ) b );
}

static
rc_t KSysDirListingInit ( KSysDirListing *self, const char *path, const KDirectory_v1 *dir,
    bool ( * f ) ( const KDirectory_v1*, const char*, void* ), void *data )
{
    rc_t rc;

    self -> namelist = NULL;
    self -> cnt = 0;

    rc = KNamelistInit ( & self -> dad,
        ( const KNamelist_vt* ) & vtKSysDirListing );
    if ( rc == 0 )
    {
        KSysDirEnum list;
        rc = KSysDirEnumInit ( & list, path );
        if ( rc == 0 )
        {
            uint32_t len = 512;
            self -> namelist = malloc ( len * sizeof self -> namelist [ 0 ] );
            if ( self -> namelist == NULL )
                rc = RC ( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );
            else
            {
                void *r;
                const char *name;
                while ( ( name = KSysDirEnumNext ( & list ) ) != NULL )
                {
                    if ( f != NULL )
                    {
                        if ( ! ( * f ) ( dir, name, data ) )
                            continue;
                    }

                    if ( self -> cnt == len )
                    {
                        len += len;
                        r = realloc ( self -> namelist,
                            len * sizeof self -> namelist [ 0 ] );
                        if ( r == NULL )
                        {
                            rc = RC ( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );
                            break;
                        }
                        self -> namelist = r;
                    }

                    self -> namelist [ self -> cnt ] = malloc ( strlen ( name ) + 1 );
                    if ( self -> namelist [ self -> cnt ] == NULL )
                    {
                        rc = RC ( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );
                        break;
                    }
                    strcpy ( ( char* ) self -> namelist [ self -> cnt ], name );
                    ++ self -> cnt;
                }

                if ( rc == 0 )
                {
                    r = realloc ( self -> namelist,
                        self -> cnt * sizeof self -> namelist [ 0 ] );
                    if ( r != NULL )
                    {
                        self -> namelist = r;
                        ksort ( r, self -> cnt, sizeof self -> namelist [ 0 ], KSysDirListingSort, NULL );
                    }
                    else if ( self -> cnt != 0 )
                    {
                        rc = RC ( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );
                    }
                    else
                    {
                        self -> namelist = r;
                    }
                }

                if ( rc != 0 )
                {
                    KSysDirListingWhack ( self );
                    self -> namelist = NULL;
                    self -> cnt = 0;
                }
            }

            KSysDirEnumWhack ( & list );
        }
    }
    return rc;
}

/*--------------------------------------------------------------------------
 * KSysDir
 *  a Unix directory
 */
struct KSysDir_v1
{
    KDirectory_v1 dad;
    uint32_t root;
    uint32_t size;
    char path [ PATH_MAX ];
};

/* KSysDirMake
 *  allocate an uninialized object
 */
static
KSysDir_v1 *KSysDirMake_v1 ( size_t path_size )
{
    KSysDir_v1 *dir = malloc ( ( sizeof * dir - sizeof dir -> path + 2 ) + path_size );
    return dir;
}

/* KSysDirDestroy
 */
static
rc_t KSysDirDestroy_v1 ( KSysDir_v1 * self )
{
    free ( self );
    return 0;
}

/* KSysDirInit
 */
static
rc_t KSysDirInit_v1 ( KSysDir_v1 * self, enum RCContext ctx, uint32_t dad_root,
    const char *path, uint32_t path_size, bool update, bool chroot );


/* KSysDirCanonPath
 */
static
rc_t KSysDirCanonPath_v1 ( const KSysDir_v1 * self, enum RCContext ctx, char *path, size_t psize )
{
    char *low, *dst, *last, *end = path + psize;
    low = dst = last = path + self -> root;

    while ( 1 )
    {
        char *src = strchr ( last + 1, '/' );
        if ( src == NULL )
            src = end;

        /* detect special sequences */
        switch ( src - last )
        {
        case 1:
            if ( last [ 1 ] == '/' )
            {
                /* "//" -> "/" */
                last = src;
            }
            break;

        case 2:
            if ( last [ 1 ] == '.' )
            {
                /* skip over "./" */
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
                dst = strrchr ( path, '/' );
                if ( dst == NULL || dst < low )
                    return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

                last = src;
                if ( src != end )
                    continue;
            }
            break;
        }

        /* if rewriting, copy leaf */
        assert ( src >= last );

        if ( dst != last )
            memmove ( dst, last, src - last );

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
        * dst = 0;

    return 0;
}

/* KSysDirMakePath
 *  creates a full path from partial
 */
rc_t KSysDirMakePath_v1 ( const KSysDir_v1 * self, enum RCContext ctx, bool canon,
    char *buffer, size_t path_max, const char *path, va_list args )
{
    int psize;
    size_t bsize;

    if ( path == NULL )
        return RC ( rcFS, rcDirectory, ctx, rcPath, rcNull );
    if ( path [ 0 ] == 0 )
        return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );

    if ( args != NULL && path [ 0 ] == '%' )
    {
        psize = vsnprintf ( buffer, path_max, path, args );
        if ( psize < 0 || psize >= path_max )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        if ( buffer [ 0 ] != '/' )
        {
            bsize = self -> size;
            if ( bsize + psize >= path_max )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
            memmove ( buffer + bsize, buffer, psize + 1 );
            assert ( self -> path [ bsize - 1 ] == '/' );
            memmove ( buffer, self -> path, bsize );
        }
        else if ( ( bsize = self -> root ) != 0 )
        {
            if ( bsize + psize >= path_max )
                return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
            memmove ( buffer + bsize, buffer, psize + 1 );
            assert ( self -> path [ bsize - 1 ] != '/' );
            memmove ( buffer, self -> path, bsize );
        }
    }
    else
    {
        if ( path [ 0 ] != '/' )
        {
            assert ( self -> path [ self -> size - 1 ] == '/' );
            memmove ( buffer, self -> path, bsize = self -> size );
        }
        else if ( ( bsize = self -> root ) != 0 )
        {
            assert ( self -> path [ bsize - 1 ] != '/' );
            memmove ( buffer, self -> path, bsize );
        }

        if ( args == NULL )
            psize = snprintf ( buffer + bsize, path_max - bsize, "%s", path );
        else
            psize = vsnprintf ( buffer + bsize, path_max - bsize, path, args );

        if ( psize < 0 || bsize + psize >= path_max )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
    }

    /* remove trailing slashes; keep the leading slash */
    while ( bsize + psize > 1 && buffer [ bsize + psize - 1] == '/' )
        buffer [ bsize + -- psize ] = 0;

    if ( psize > 0 && ( canon || self -> root != 0 ) )
        return KSysDirCanonPath_v1 ( self, ctx, buffer, bsize + psize );

    return 0;
}


/* RealPath
 *  returns a real OS path
 */
rc_t KSysDirVRealPath ( const KSysDir_v1 * self,
    char *real, size_t bsize, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcLoading, false,
        full, sizeof full, path, args );
    assert ( bsize >= PATH_MAX );
    if ( rc == 0 && realpath ( full, real ) == NULL )
    {
        switch ( errno )
        {
        case EACCES:
            return RC ( rcFS, rcDylib, rcLoading, rcDirectory, rcUnauthorized );
        case ENOTDIR:
        case EINVAL:
        case ELOOP:
            return RC ( rcFS, rcDylib, rcLoading, rcPath, rcInvalid );
        case EIO:
            return RC ( rcFS, rcDylib, rcLoading, rcTransfer, rcUnknown );
        case ENAMETOOLONG:
            return RC ( rcFS, rcDylib, rcLoading, rcPath, rcExcessive );
        case ENOENT:
            return RC ( rcFS, rcDylib, rcLoading, rcPath, rcNotFound );
        default:
            return RC ( rcFS, rcDylib, rcLoading, rcNoObj, rcUnknown );
        }
    }

    return rc;
}

rc_t KSysDirRealPath_v1 ( const KSysDir_v1 * self,
    char *real, size_t bsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KSysDirVRealPath ( self, real, bsize, path, args );
    va_end ( args );

    return rc;
}

/* KSysDirList
 *  create a directory listing
 *
 *  "list" [ OUT ] - return parameter for list object
 *
 *  "path" [ IN, NULL OKAY ] - optional parameter for target
 *  directory. if NULL, interpreted to mean "."
 */
static
rc_t KSysDirList_v1 ( const KSysDir_v1 * self, KNamelist **listp,
    bool ( * f ) ( const KDirectory_v1 *dir, const char *name, void *data ), void *data,
    const char *path, va_list args )
{
    KSysDir_v1 full;
    rc_t rc = KSysDirMakePath_v1 ( self, rcListing, true,
        full . path, sizeof full . path, path, args );
    if ( rc == 0 )
    {
        rc = KSysDirInit_v1 ( & full, rcListing, self -> root,
            NULL, strlen ( full . path ), 0, 0 );
        if ( rc == 0 )
        {
            KSysDirListing *list = malloc ( sizeof * list );
            if ( list == NULL )
                rc = RC ( rcFS, rcDirectory, rcListing, rcMemory, rcExhausted );
            else
            {
                rc = KSysDirListingInit ( list,
                    full . path, & full . dad, f, data );
                if ( rc != 0 )
                    free ( list );
                else
                    * listp = & list -> dad;
            }
        }
    }
    return rc;
}


/* KSysDirPathType
 *  returns a KPathType
 *
 *  "path" [ IN ] - NUL terminated string in directory-native character set
 */
static
uint32_t KSysDirFullPathType_v1 ( const char *path )
{
    struct stat st;
    int type, alias;

    if ( lstat ( path, & st ) != 0 ) switch ( errno )
    {
    case ENOENT:
        return kptNotFound;
    default:
        return kptBadPath;
    }

    alias = 0;

    if ( S_ISLNK ( st . st_mode ) )
    {
        alias = kptAlias;

        if ( stat ( path, & st ) != 0 ) switch ( errno )
        {
        case ENOENT:
            return kptNotFound | alias;
        default:
            return kptBadPath | alias;
        }
    }

    /* not a bad assumption */
    type = kptFile;

    /* overrides */
    if ( S_ISDIR ( st . st_mode ) )
        type = kptDir;
    else if ( S_ISCHR ( st . st_mode ) )
        type = kptCharDev;
    else if ( S_ISBLK ( st . st_mode ) )
        type = kptBlockDev;
    else if ( S_ISFIFO ( st . st_mode ) )
        type = kptFIFO;
    else if ( S_ISSOCK ( st . st_mode ) )
        type = kptFIFO;

    /* add in alias bit */
    return type | alias;
}

static
uint32_t KSysDirPathType_v1 ( const KSysDir_v1 * self, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcAccessing, false, full, sizeof full, path, args );
    if ( rc == 0 )
        return KSysDirFullPathType_v1 ( full );
    return kptBadPath;
}

/* KSysDirVisit
 *  visit each path under designated directory,
 *  recursively if so indicated
 *
 *  "recur" [ IN ] - if non-zero, recursively visit sub-directories
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
    rc_t ( * f ) ( KDirectory_v1*, uint32_t, const char*, void* );
    void *data;
    KSysDir_v1 dir;
    bool recur;
};

static
rc_t KSysDirVisitDir ( KSysDirVisitData *pb )
{
    /* get a directory listing */
    KSysDirEnum listing;
    rc_t rc = KSysDirEnumInit ( & listing, pb -> dir . path );
    if ( rc == 0 )
    {
        const char *name;
        uint32_t size = pb -> dir . size;

        /* complete directory path */
        pb -> dir . path [ size ] = '/';
        if ( ++ size >= sizeof pb -> dir . path )
            rc = RC ( rcFS, rcDirectory, rcVisiting, rcPath, rcExcessive );
        else for ( pb -> dir . size = size, name = KSysDirEnumNext ( & listing );
                   name != NULL; name = KSysDirEnumNext ( & listing ) )
        {
            uint32_t type, len = strlen ( name );
            if ( size + len >= sizeof pb -> dir . path )
            {
                rc = RC ( rcFS, rcDirectory, rcVisiting, rcPath, rcExcessive );
                break;
            }
            strcpy ( & pb -> dir . path [ size ], name );

            type = KSysDirFullPathType_v1 ( pb -> dir . path );
            if ( type == kptBadPath )
            {
                rc = RC ( rcFS, rcDirectory, rcVisiting, rcPath, rcInvalid );
                break;
            }

            rc = ( * pb -> f ) ( & pb -> dir . dad, type, name, pb -> data );
            if ( rc != 0 )
                break;

            if ( pb -> recur && ( type & ( kptAlias - 1 ) ) == kptDir )
            {
                pb -> dir . size += len;
                rc = KSysDirVisitDir ( pb );
                pb -> dir . size = size;
                if ( rc != 0 )
                    break;
            }
        }


        KSysDirEnumWhack ( & listing );
    }
    return rc;
}

static
rc_t KSysDirVisit_v1 ( const KSysDir_v1 * self, bool recur,
    rc_t ( * f ) ( KDirectory_v1 *dir, uint32_t type, const char *name, void *data ), void *data,
    const char *path, va_list args )
{
    KSysDirVisitData pb;
    rc_t rc = KSysDirMakePath_v1 ( self, rcVisiting, true,
        pb . dir . path, sizeof pb . dir . path, path, args );
    if ( rc == 0 )
    {
        uint32_t path_size;

        switch ( KSysDirFullPathType_v1 ( pb . dir . path ) & ( kptAlias - 1 ) )
        {
        case kptNotFound:
            return RC ( rcFS, rcDirectory, rcVisiting, rcPath, rcNotFound );
        case kptBadPath:
            return RC ( rcFS, rcDirectory, rcVisiting, rcPath, rcInvalid );
        case kptDir:
            break;
        default:
            return RC ( rcFS, rcDirectory, rcVisiting, rcPath, rcIncorrect );
        }

        path_size = strlen ( pb . dir . path );
        while ( path_size > 1 && path_size > self -> root && pb . dir . path [ path_size - 1 ] == '/' )
            -- path_size;

        rc = KSysDirInit_v1 ( & pb . dir, rcVisiting, self -> root,
            NULL, path_size, self -> dad . read_only ? 0 : 1, 0 );
        if ( rc == 0 )
        {
            pb . f = f;
            pb . data = data;
            pb . recur = recur;
            pb . dir . path [ -- pb . dir . size ] = 0;
            rc = KSysDirVisitDir ( & pb );
        }
    }
    return rc;
}

/* KSysDirRelativePath
 *  makes "path" relative to "root"
 *  both "root" and "path" MUST be absolute
 *  both "root" and "path" MUST be canonical, i.e. have no "./" or "../" sequences
 */
static
rc_t KSysDirRelativePath_v1 ( const KSysDir_v1 * self, enum RCContext ctx,
    const char *root, char *path, size_t path_max )
{
    int backup;
    size_t bsize, psize;

    const char *r = root + self -> root;
    const char *p = path + self -> root;

    assert ( r != NULL && r [ 0 ] == '/' );
    assert ( p != NULL && p [ 0 ] == '/' );

    for ( ; * r == * p; ++ r, ++ p )
    {
        /* disallow identical paths */
        if ( * r == 0 )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcInvalid );
    }

    /* paths are identical up to "r","p"
       if "r" is within a leaf name, then no backup is needed
       by counting every '/' from "r" to end, obtain backup count */
    for ( backup = 0; * r != 0; ++ r )
    {
        if ( * r == '/' )
            ++ backup;
    }

    /* the number of bytes to be inserted */
    bsize = backup * 3;

    /* align "p" to last directory separator */
    while ( p [ -1 ] != '/' ) -- p;

    /* the size of the remaining relative path */
    psize = strlen ( p );

    /* open up space if needed */
    if ( p - path < bsize )
    {
        /* prevent overflow */
        if ( bsize + psize >= path_max )
            return RC ( rcFS, rcDirectory, ctx, rcPath, rcExcessive );
        memmove ( path + bsize, p, psize + 1 /* 1 for '\0'*/ );
    }

    /* insert backup sequences */
    for ( bsize = 0; backup > 0; bsize += 3, -- backup )
        memmove ( & path [ bsize ], "../", 3 );

    /* close gap */
    if ( p - path > bsize )
        memmove ( & path [ bsize ], p, strlen ( p ) + 1 );

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
 *  NUL terminated result path in directory-native character set
 *  the resolved path will be directory relative
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target path. NB - need not exist.
 */
static
rc_t KSysDirResolvePath_v1 ( const KSysDir_v1 * self, bool absolute,
    char *resolved, size_t rsize, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcResolving, true, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        uint32_t path_size = strlen ( full );
        PLOGMSG(klogDebug, (klogDebug, "KSysDirResolvePath_v1 = '$(res)'", "res=%s", full));

        if ( absolute )
        {
            /* test buffer capacity */
            if ( path_size - self -> root >= rsize )
                return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );

            /* ready to go */
            strcpy ( resolved, & full [ self -> root ] );
/*             assert ( resolved [ 0 ] == '/' ); */
        }
        else
        {
            rc = KSysDirRelativePath_v1 ( self, rcResolving, self -> path, full, sizeof full /*path_size*/ );
            if ( rc == 0 )
            {
                path_size = strlen ( full );
                if ( path_size >= rsize )
                    return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
                strcpy ( resolved, full );
            }
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
rc_t KSysDirResolveAlias_v1 ( const KSysDir_v1 * self, bool absolute,
    char *resolved, size_t rsize, const char *alias, va_list args )
{
    KSysDir_v1 full;
    rc_t rc = KSysDirMakePath_v1 ( self, rcResolving, true,
        full . path, sizeof full . path, alias, args );
    if ( rc == 0 )
    {
        char link [ PATH_MAX ];
        int len = readlink ( full . path, link, sizeof link );
        if ( len < 0 ) switch ( errno )
        {
        case ENOENT:
            return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcNotFound );
        case ENOTDIR:
            return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcIncorrect );
        case ENAMETOOLONG:
        case ELOOP:
            return RC ( rcFS, rcDirectory, rcResolving, rcPath, rcInvalid );
        case EACCES:
            return RC ( rcFS, rcDirectory, rcResolving, rcDirectory, rcUnauthorized );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcResolving, rcMemory, rcExhausted );
        case EIO:
            return RC ( rcFS, rcDirectory, rcResolving, rcTransfer, rcUnknown );
        default:
            return RC ( rcFS, rcDirectory, rcResolving, rcNoObj, rcUnknown );
        }

        if ( ( size_t ) len == sizeof link )
            return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        link [ len ] = 0;

        if ( link [ 0 ] == '/' )
        {
            full . size = 1;
            strcpy ( full . path, link );
        }
        else
        {
            char *f = strrchr ( full . path, '/' );
            assert ( f != NULL );
            full . size = ++f - full . path;
            if ( full . size + len >= sizeof full . path )
                return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
            strcpy ( f, link );
        }

        full . root = 0;

/*         rc = KSysDirCanonPath ( & full, rcResolving, full . path, len ); */
        rc = KSysDirCanonPath_v1 ( & full, rcResolving, full . path, full . size + len);
        if ( rc == 0 )
        {
            /* the path in full is an absolute path
               if outside of chroot, it's a bad link */
            if ( memcmp ( full . path, self -> path, self -> root + 1 ) != 0 )
                return RC ( rcFS, rcDirectory, rcResolving, rcLink, rcInvalid );

            /* this is the absolute path length */
            len = strlen ( & full . path [ self -> root ] );

            /* if not requesting absolute, make self relative */
            if ( ! absolute )
            {
                rc = KSysDirRelativePath_v1 ( self, rcResolving, self -> path, full . path, sizeof full.path/*len*/ );
                if ( rc != 0 )
                    return rc;
                len = strlen ( full . path );
            }

            if ( ( size_t ) len >= rsize )
                return RC ( rcFS, rcDirectory, rcResolving, rcBuffer, rcInsufficient );

            strcpy ( resolved, & full . path [ self -> root ] );
        }
    }
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
static rc_t KSysDirVAccess ( const KSysDir_v1 * self, uint32_t *access, const char *path,
                             va_list args );
static rc_t KSysDirSetAccess_v1 ( KSysDir_v1 * self, bool recur, uint32_t access, uint32_t mask,
                               const char *path, va_list args );

static
rc_t KSysDirRename_v1 ( KSysDir_v1 * self, bool force, const char *from, const char *to )
{
    char ffrom [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcRenaming, false, ffrom, sizeof ffrom, from, NULL );
    if ( rc == 0 )
    {
        char fto [ PATH_MAX ];
        rc = KSysDirMakePath_v1 ( self, rcRenaming, false, fto, sizeof fto, to, NULL );
        if ( rc == 0 )
        {
            if ( rename ( ffrom, fto ) != 0 ) switch ( errno )
            {
            case EISDIR:
            case EXDEV:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcPath, rcIncorrect );
                break;
            case ENOTEMPTY:
            case EEXIST:
            case EBUSY:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcPath, rcBusy );
                break;
            case EINVAL:
            case ENOTDIR:
            case ENAMETOOLONG:
            case ELOOP:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcPath, rcInvalid );
                break;
            case EACCES:
            case EPERM:
            case EROFS:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcDirectory, rcUnauthorized );
                break;
            case ENOSPC:
                rc= RC ( rcFS, rcDirectory, rcRenaming, rcStorage, rcExhausted );
                break;
            case ENOMEM:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcMemory, rcExhausted );
                break;
            case ENOENT:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcPath, rcNotFound );
                break;
            default:
                rc = RC ( rcFS, rcDirectory, rcRenaming, rcNoObj, rcUnknown );
                break;
            }
        }
        if (force)
        {
            if (GetRCState(rc) == rcUnauthorized)
            {
                uint32_t faccess = 0;
                uint32_t taccess = 0;
                bool fchanged = false;
                bool tchanged = false;

                rc = KSysDirVAccess (self, &taccess, to, NULL);
                if (rc == 0)
                {
                    rc = KSysDirSetAccess_v1 (self, false, 0222, 0222, to, NULL);
                    tchanged = true;
                }
                else if(GetRCState(rc) ==  rcNotFound)
                {
                    rc = 0;
                }

                if (rc == 0)
                {
                    rc = KSysDirVAccess (self, &faccess, from, NULL);
                    if (rc == 0)
                    {
                        rc = KSysDirSetAccess_v1 (self, false, 0222, 0222, from, NULL);
                        if (rc == 0)
                        {
                            fchanged = true;
                            rc = KSysDirRename_v1 (self, false, from, to);
                        }
                    }
                    if (rc == 0)
                    {
                        /* set access on the new name to the access from the old name */
                        KSysDirSetAccess_v1 (self, false, faccess, 0222, to, NULL);
                    }
                    else
                    {
                        /* since something falied, try to restore changed access bits */
                        if (fchanged)
                            KSysDirSetAccess_v1 (self, false, faccess, 0222, from, NULL);
                        if (tchanged)
                            KSysDirSetAccess_v1 (self, false, taccess, 0222, to, NULL);
                    }

                }

            }
        }
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
rc_t KSysDirRemoveEntry_v1 ( char *path, size_t path_max, bool force );

static
rc_t KSysDirEmptyDir_v1 ( char *path, size_t path_max, bool force )
{
    KSysDirEnum list;
    rc_t rc = KSysDirEnumInit ( & list, path );
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcFS, rcDirectory, rcClearing );
    else
    {
        size_t path_size = strlen ( path );
        path [ path_size ] = '/';
        if ( ++ path_size == path_max )
            rc = RC ( rcFS, rcDirectory, rcClearing, rcPath, rcExcessive );
        else
        {
            const char *leaf;
            while ( ( leaf = KSysDirEnumNext ( & list ) ) != NULL )
            {
                size_t leaf_size = strlen ( leaf );
                if ( path_size + leaf_size >= path_max )
                {
                    rc = RC ( rcFS, rcDirectory, rcClearing, rcPath, rcExcessive );
                    break;
                }

                strcpy ( & path [ path_size ], leaf );
                rc = KSysDirRemoveEntry_v1 ( path, path_max, force );
                if ( rc != 0 )
                {
                    rc = ResetRCContext ( rc, rcFS, rcDirectory, rcClearing );
                    break;
                }
            }

            path [ path_size - 1 ] = 0;
        }

        KSysDirEnumWhack ( & list );
    }
    return rc;
}

static
rc_t KSysDirClearDir_v1 ( KSysDir_v1 * self, bool force, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcClearing, false, full, sizeof full, path, args );
    if ( rc == 0 )
        rc = KSysDirEmptyDir_v1 ( full, sizeof full, force );
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
rc_t KSysDirRemoveEntry_v1 ( char *path, size_t path_max, bool force )
{
    if ( unlink ( path ) != 0 )
    {
        switch ( errno )
        {
        case ENOENT:
            return 0;
        case EPERM:
        case EISDIR:
            break;
        case EACCES:
        case EROFS:
            return RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );
        case EBUSY:
            return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcBusy );
        case ENAMETOOLONG:
        case ENOTDIR:
        case ELOOP:
            return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcInvalid );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcRemoving, rcMemory, rcExhausted );
        case EIO:
            return RC ( rcFS, rcDirectory, rcRemoving, rcTransfer, rcUnknown );
        default:
            return RC ( rcFS, rcDirectory, rcRemoving, rcNoObj, rcUnknown );
        }

        while ( rmdir ( path ) != 0 ) switch ( errno )
        {
        case EEXIST:
        case ENOTEMPTY:
            if ( force )
            {
                rc_t rc = KSysDirEmptyDir_v1 ( path, path_max, force );
                if ( rc != 0 )
                    return rc;
                force = false;
                break;
            }
        case EBUSY:
            return RC ( rcFS, rcDirectory, rcRemoving, rcPath, rcBusy );
        case EPERM:
        case EACCES:
        case EROFS:
            return RC ( rcFS, rcDirectory, rcRemoving, rcDirectory, rcUnauthorized );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcRemoving, rcMemory, rcExhausted );
        default:
            return RC ( rcFS, rcDirectory, rcRemoving, rcNoObj, rcUnknown );
        }
    }

    return 0;
}

static
rc_t KSysDirRemove_v1 ( KSysDir_v1 * self, bool force, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcRemoving, false, full, sizeof full, path, args );
    if ( rc == 0 )
        rc = KSysDirRemoveEntry_v1 ( full, sizeof full, force );
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
static
rc_t KSysDirVAccess ( const KSysDir_v1 * self,
    uint32_t *access, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcAccessing, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        struct stat st;
        if ( lstat ( full, & st ) != 0 ) switch ( errno )
        {
        case ENOENT:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNotFound );
        case ENOTDIR:
        case ELOOP:
        case ENAMETOOLONG:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );
        case EACCES:
            return RC ( rcFS, rcDirectory, rcAccessing, rcDirectory, rcUnauthorized );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcAccessing, rcMemory, rcExhausted );
        default:
            return RC ( rcFS, rcDirectory, rcAccessing, rcNoObj, rcUnknown );
        }

        * access = st . st_mode & 07777;
    }
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
 *  "recur" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 */
static
rc_t KSysDirChangeAccess_v1 ( char *path, size_t path_max,
    uint32_t access, uint32_t mask, bool recur );

static
rc_t KSysDirChangeDirAccess_v1 ( char *path, size_t path_max,
    uint32_t access, uint32_t mask )
{
    KSysDirEnum list;
    rc_t rc = KSysDirEnumInit ( & list, path );
    if ( rc == 0 )
    {
        bool eperm = false;
        size_t path_size = strlen ( path );
        path [ path_size ] = '/';
        if ( ++ path_size == path_max )
            rc = RC ( rcFS, rcDirectory, rcUpdating, rcBuffer, rcInsufficient );
        else
        {
            const char *leaf;
            while ( ( leaf = KSysDirEnumNext ( & list ) ) != NULL )
            {
                size_t leaf_size = strlen ( leaf );
                if ( path_size + leaf_size >= path_max )
                {
                    rc = RC ( rcFS, rcDirectory, rcUpdating, rcBuffer, rcInsufficient );
                    break;
                }

                strcpy ( & path [ path_size ], leaf );
                rc = KSysDirChangeAccess_v1 ( path, path_max, access, mask, 1 );
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
            rc = RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    }
    return rc;
}

static
rc_t KSysDirChangeEntryAccess_v1 ( char *path, size_t path_max,
    uint32_t access, uint32_t mask, uint32_t st_mode )
{
    /* keep old bits */
    access &= mask;
    access |= st_mode & ~ mask;

    if ( chmod ( path, access & 07777 ) != 0 ) switch ( errno )
    {
    case EPERM:
    case EACCES:
    case EROFS:
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    case ENOTDIR:
    case ELOOP:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );
    case ENAMETOOLONG:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
    case ENOENT:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNotFound );
    case ENOMEM:
        return RC ( rcFS, rcDirectory, rcUpdating, rcMemory, rcExhausted );
    default:
        return RC ( rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnknown );
    }

    return 0;
}

static
rc_t KSysDirChangeAccess_v1 ( char *path, size_t path_max,
    uint32_t access, uint32_t mask, bool recur )
{
    struct stat st;
    if ( lstat ( path, & st ) != 0 ) switch ( errno )
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

    if ( recur && S_ISDIR ( st . st_mode ) )
    {
        rc_t rc;
        uint32_t enable = access & mask;
        if ( enable != 0 )
        {
            rc = KSysDirChangeEntryAccess_v1 ( path, path_max,
                access, enable, st . st_mode );
            if ( rc != 0 )
                return rc;
        }

       rc = KSysDirChangeDirAccess_v1 ( path, path_max, access, mask );
        if ( rc == 0 )
        {
            uint32_t disable = ~ access & mask;
            if ( disable != 0 )
            {
                rc = KSysDirChangeEntryAccess_v1 ( path, path_max,
                    access, disable, st . st_mode | enable );
            }
        }
        return rc;
    }

    return KSysDirChangeEntryAccess_v1 ( path, path_max,
         access, mask, st . st_mode );
}

static
rc_t KSysDirSetAccess_v1 ( KSysDir_v1 * self, bool recur,
    uint32_t access, uint32_t mask, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcUpdating, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        if ( mask == 0 )
            mask = 07777;

        rc = KSysDirChangeAccess_v1 ( full, sizeof full,
            access, mask & 07777, recur );
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
rc_t KSysDirVDate ( const KSysDir_v1 * self,
    KTime_t * date, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcAccessing, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        struct stat st;
        if ( lstat ( full, & st ) != 0 ) switch ( errno )
        {
        case ENOENT:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNotFound );
        case ENOTDIR:
        case ELOOP:
        case ENAMETOOLONG:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );
        case EACCES:
            return RC ( rcFS, rcDirectory, rcAccessing, rcDirectory, rcUnauthorized );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcAccessing, rcMemory, rcExhausted );
        default:
            return RC ( rcFS, rcDirectory, rcAccessing, rcNoObj, rcUnknown );
        }

        * date = ( KTime_t ) st . st_mtime;
    }
    return rc;
}

/* KSysDirSetDate
 *  set date to object a la Unix "touch"
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 *
 *  "date" [ IN ]  - new mtime
 *
 *  "recur" [ IN ] - if non zero and "path" is a directory,
 *  apply changes recursively.
 */
static
rc_t KSysDirChangeDate_v1 ( char *path, size_t path_max,
			 KTime_t date, bool recur );

static
rc_t KSysDirChangeDirDate_v1 ( char *path, size_t path_max,
			      KTime_t date )
{
    KSysDirEnum list;
    rc_t rc = KSysDirEnumInit ( & list, path );
    if ( rc == 0 )
    {
        bool eperm = false;
        size_t path_size = strlen ( path );
        path [ path_size ] = '/';
        if ( ++ path_size == path_max )
            rc = RC ( rcFS, rcDirectory, rcUpdating, rcBuffer, rcInsufficient );
        else
        {
            const char *leaf;
            while ( ( leaf = KSysDirEnumNext ( & list ) ) != NULL )
            {
                size_t leaf_size = strlen ( leaf );
                if ( path_size + leaf_size >= path_max )
                {
                    rc = RC ( rcFS, rcDirectory, rcUpdating, rcBuffer, rcInsufficient );
                    break;
                }

                strcpy ( & path [ path_size ], leaf );
                rc = KSysDirChangeDate_v1 ( path, path_max, date, 1 );
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
            rc = RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    }
    return rc;
}

static
rc_t KSysDirChangeEntryDate_v1 ( char *path, size_t path_max,
			      struct utimbuf * tb)
{
    if ( utime ( path, tb ) != 0 ) switch ( errno )
    {
    case EPERM:
    case EACCES:
    case EROFS:
        return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
    case ENOTDIR:
    case ELOOP:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );
    case ENAMETOOLONG:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
    case ENOENT:
        return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNotFound );
    case ENOMEM:
        return RC ( rcFS, rcDirectory, rcUpdating, rcMemory, rcExhausted );
    default:
        return RC ( rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnknown );
    }

    return 0;
}

static
rc_t KSysDirChangeDate_v1 ( char *path, size_t path_max,
			 KTime_t date, bool recur )
{
    struct stat st;
    struct utimbuf u;

    if ( lstat ( path, & st ) != 0 ) switch ( errno )
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

    if ( recur && S_ISDIR ( st . st_mode ) )
    {
        rc_t rc = KSysDirChangeEntryDate_v1 ( path, path_max, & u );
        if ( rc != 0 )
            return rc;

        rc = KSysDirChangeDirDate_v1 ( path, path_max, date );
        if ( rc == 0 )
            rc = KSysDirChangeEntryDate_v1 ( path, path_max, & u  );

        return rc;
    }

    return  KSysDirChangeEntryDate_v1 ( path, path_max, & u );
}

static
rc_t KSysDirVSetDate ( KSysDir_v1 *self, bool recur,
	KTime_t date, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcUpdating, false, full, sizeof full, path, args );
    if ( rc == 0 )
        rc = KSysDirChangeDate_v1 ( full, sizeof full, date, recur );

    return rc;
}

static
KSysDir_v1 *KSysDirGetSysdir_v1 ( const KSysDir_v1 *cself )
{
    return ( KSysDir_v1 * ) cself;
}

/* KSysDirCreateParents
 *  creates missing parent directories
 */
static
rc_t make_dir_v1 ( const char *path, uint32_t access )
{
    if ( mkdir ( path, ( int ) access ) != 0 ) switch ( errno )
    {
    case ENOENT:
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNotFound );
    case EEXIST:
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExists );
    case EPERM:
    case EACCES:
    case EROFS:
        return RC ( rcFS, rcDirectory, rcCreating, rcDirectory, rcUnauthorized );
    case ENOTDIR:
    case ELOOP:
        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );
    case ENOMEM:
        return RC ( rcFS, rcDirectory, rcCreating, rcMemory, rcExhausted );
    case ENOSPC:
        return RC ( rcFS, rcDirectory, rcCreating, rcStorage, rcExhausted );
    default:
        return RC ( rcFS, rcDirectory, rcCreating, rcNoObj, rcUnknown );
    }
    return 0;
}

static
rc_t KSysDirCreateParents_v1 ( const KSysDir_v1 * self,
    char *path, uint32_t access, bool strip )
{
    rc_t rc;
    char *p, *par = path + self -> root + 1;
    size_t size = strlen ( par );

    if ( ! strip )
        p = par + size;
    else
    {
        p = strrchr ( par, '/' );
        if ( p == NULL )
            return 0;
        size = p - par;
    }

    while ( 1 )
    {
        /* crop string */
        p [ 0 ] = 0;

        /* try to create directory */
        rc = make_dir_v1 ( path, access );
        if ( GetRCState ( rc ) != rcNotFound )
            break;

        /* back up some more */
        p = strrchr ( par, '/' );
        if ( p == NULL )
        {
            p = par + strlen ( par );
            break;
        }
    }

    par += size;
    assert ( p != NULL );

    /* create directories from here */
    if ( rc == 0 ) while ( p < par )
    {
        p [ 0 ] = '/';
        rc = make_dir_v1 ( path, access );
        if ( rc != 0 || ++ p >= par )
            break;
        p += strlen ( p );
    }

    /* fix up remaining path */
    while ( p < par )
    {
        p [ 0 ] = '/';
        if ( ++ p >= par )
            break;
        p += strlen ( p );
    }

    /* repair stripped path */
    if ( strip )
        par [ 0 ] = '/';

    return rc;
}

/* CreateAlias
 *  creates a path alias according to create mode
 *  such that "alias" => "targ"
 *
 *  "access" [ IN ] - standard Unix directory access mode
 *  used when "mode" has kcmParents set and alias path does
 *  not exist.
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "targ" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object, i.e. the object which
 *  is designated by symlink "alias".
 *
 *  "alias" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target alias, i.e. the symlink that
 *  designates a target "targ".
 */
static
rc_t KSysDirCreateAlias_v1 ( KSysDir_v1 * self,
    uint32_t access, KCreateMode mode,
    const char *targ, const char *alias )
{
    /* create full path to symlink */
    char falias [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcCreating, true, falias, sizeof falias, alias, NULL );
    if ( rc == 0 )
    {
        /* the full path to target RELATIVE TO self */
        char ftarg [ PATH_MAX ];
        rc = KSysDirMakePath_v1 ( self, rcCreating, true, ftarg, sizeof ftarg, targ, NULL );
        if ( rc == 0 )
        {
            /* if "targ" is relative or "self" is chroot'd,
               "ftarg" must be made relative */
            if ( targ [ 0 ] != '/' || self -> root != 0 )
            {
                /* take path to alias as root.
                   generate a path RELATIVE TO alias */
                rc = KSysDirRelativePath_v1 ( self, rcCreating, falias,
                    ftarg, sizeof ftarg /*strlen ( ftarg )*/ );
                if ( rc != 0 )
                    return rc;
            }

            if ( symlink ( ftarg, falias ) == 0 )
                return 0;

            switch ( errno )
            {
            case EEXIST:
                /* alias already exists. unless mode is
                   create-only, force creation by removing old */
                if ( ( mode & kcmValueMask ) != kcmCreate )
                {
                    /* refuse to drop if not an alias */
                    if ( ( KSysDirFullPathType_v1 ( falias ) & kptAlias ) == 0 )
                        return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcIncorrect );

                    /* drop existing alias */
                    rc = KSysDirRemoveEntry_v1 ( falias, sizeof falias, false );
                    if ( rc == 0 )
                        break;
                }
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExists );

            case ENOENT:
                /* a part of the alias path doesn't exist */
                if ( ( mode & kcmParents ) != 0 )
                {
                    KSysDirCreateParents_v1 ( self, falias, access, true );
                    break;
                }
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNotFound );

            case EPERM:
            case EACCES:
            case EROFS:
                return RC ( rcFS, rcDirectory, rcCreating, rcDirectory, rcUnauthorized );
            case ENAMETOOLONG:
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExcessive );
            case ENOTDIR:
            case ELOOP:
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );
            case ENOMEM:
                return RC ( rcFS, rcDirectory, rcCreating, rcMemory, rcExhausted );
            case ENOSPC:
                return RC ( rcFS, rcDirectory, rcCreating, rcStorage, rcExhausted );
            case EIO:
                return RC ( rcFS, rcDirectory, rcCreating, rcTransfer, rcUnknown );
            default:
                return RC ( rcFS, rcDirectory, rcCreating, rcNoObj, rcUnknown );
            }

            /* try again either with existing guy removed
               or missing directories created */
            if ( symlink ( ftarg, falias ) != 0 ) switch ( errno )
            {
            case EEXIST:
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExists );
            case ENOENT:
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNotFound );
            default:
                return RC ( rcFS, rcDirectory, rcCreating, rcNoObj, rcUnknown );
            }

            assert ( rc == 0 );
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
rc_t KSysDirOpenFileRead_v1 ( const KSysDir_v1 * self,
    const KFile_v1 **f, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcOpening, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        int fd = open ( full, O_RDONLY );
        if ( fd < 0 ) switch ( errno )
        {
        case ENOENT:
            return SILENT_RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNotFound );
        case EACCES:
            return RC ( rcFS, rcDirectory, rcOpening, rcDirectory, rcUnauthorized );
        case EISDIR:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcIncorrect );
        case ENOTDIR:
        case ELOOP:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcInvalid );
        case ENAMETOOLONG:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcExcessive );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcOpening, rcMemory, rcExhausted );
        case EMFILE:
        case ENFILE:
            return RC ( rcFS, rcDirectory, rcOpening, rcFileDesc, rcExhausted );
        default:
            return RC ( rcFS, rcDirectory, rcOpening, rcNoObj, rcUnknown );
        }

        rc = KSysFileMake_v1 ( ( KSysFile_v1 ** ) f, fd, full, true, false );
        if ( rc != 0 )
            close ( fd );
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
rc_t KSysDirOpenFileWrite_v1 ( KSysDir_v1 * self,
    KFile_v1 **f, bool update, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcOpening, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        int fd = open ( full, update ? O_RDWR : O_WRONLY );
        if ( fd < 0 ) switch ( errno )
        {
        case ENOENT:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNotFound );
        case EACCES:
        case EROFS:
            return RC ( rcFS, rcDirectory, rcAccessing, rcDirectory, rcUnauthorized );
        case EISDIR:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcIncorrect );
        case ENOTDIR:
        case ELOOP:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );
        case ENAMETOOLONG:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcExcessive );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcOpening, rcMemory, rcExhausted );
        case EMFILE:
        case ENFILE:
            return RC ( rcFS, rcDirectory, rcOpening, rcFileDesc, rcExhausted );
        default:
            return RC ( rcFS, rcDirectory, rcOpening, rcNoObj, rcUnknown );
        }

        rc = KSysFileMake_v1 ( ( KSysFile_v1 ** ) f, fd, full, update, 1 );
        if ( rc != 0 )
            close ( fd );
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
rc_t KSysDirCreateFile_v1 ( KSysDir_v1 * self, KFile_v1 **f, bool update,
    uint32_t access, KCreateMode cmode, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcCreating, true, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        int fd, mode = update ? O_RDWR | O_CREAT : O_WRONLY | O_CREAT;
        switch ( cmode & kcmValueMask )
        {
        case kcmOpen:
            break;
        case kcmInit:
            mode |= O_TRUNC;
            break;
        case kcmCreate:
            mode |= O_EXCL;
            break;
        case kcmSharedAppend:
            mode = O_WRONLY | O_APPEND | O_CREAT;
            break;
        }

        fd = open ( full, mode, ( int ) access );
        while ( fd < 0 )
        {
            /* a common creation error is missing parents */
            if ( ( cmode & kcmParents ) != 0 && errno == ENOENT )
            {
                /* force directory mode to have execute
                   wherever there is read or write on file */
                uint32_t dir_access = access |
                    ( ( access & 0444 ) >> 2 ) | ( ( access & 0222 ) >> 1 );
                /* NEW 2/15/2013 - also force read */
                dir_access |= ( dir_access & 0111 ) << 2;
                KSysDirCreateParents_v1 ( self, full, dir_access, true );

                /* try again */
                fd = open ( full, mode, ( int ) access );
                if ( fd >= 0 )
                    break;
            }

            /* when simply "touching" a file, the request for
               write access may fail if created without write access */
            if ( ( access & 0200 ) == 0 && errno == EACCES )
            {
                mode = O_CREAT;
                if ( ( access & 0400 ) != 0 )
                    mode |= O_RDONLY;
                fd = open ( full, mode, ( int ) access );
                if ( fd >= 0 )
                    break;
            }

            switch ( errno )
            {
            case ENOENT:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcPath, rcNotFound );
                break;
            case EEXIST:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExists );
                break;
            case EACCES:
            case EROFS:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcDirectory, rcUnauthorized );
                break;
            case EISDIR:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcPath, rcIncorrect );
                break;
            case ENOTDIR:
            case ELOOP:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );
                break;
            case ENAMETOOLONG:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcPath, rcExcessive );
                break;
            case ENOSPC:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcStorage, rcExhausted );
                break;
            case ENOMEM:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcMemory, rcExhausted );
                break;
            case EMFILE:
            case ENFILE:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcFileDesc, rcExhausted );
                break;
            default:
                rc = RC ( rcFS, rcDirectory, rcCreating, rcNoObj, rcUnknown );
                break;
            }

            /* disabled 12/12/2012 : it prints an error message, if vdb tries to open
               the same reference-object twice via http. The lock-file for the 2nd try
               does already exist. This is not an error, just a condition. */

            /* PLOGERR (klogErr, (klogErr, rc, "failed to create '$(F)'", "F=%s", full)); */
            return rc;
        }

        rc = KSysFileMake_v1 ( ( KSysFile** ) f, fd, full, update, true );
        if ( rc != 0 )
            close ( fd );
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
rc_t KSysDirFileSize_v1 ( const KSysDir_v1 * self,
    uint64_t *size, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcAccessing, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        struct stat st;
        if ( stat ( full, & st ) != 0 ) switch ( errno )
        {
        case ENOENT:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcNotFound );
        case ENOTDIR:
        case ELOOP:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcInvalid );
        case ENAMETOOLONG:
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcExcessive );
        case EACCES:
            return RC ( rcFS, rcDirectory, rcAccessing, rcDirectory, rcUnauthorized );
        case ENOMEM:
            return RC ( rcFS, rcDirectory, rcAccessing, rcMemory, rcExhausted );
        default:
            return RC ( rcFS, rcDirectory, rcAccessing, rcNoObj, rcUnknown );
        }

        if ( S_ISDIR ( st . st_mode ) )
            return RC ( rcFS, rcDirectory, rcAccessing, rcPath, rcIncorrect );

        * size = st . st_size;
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
rc_t KSysDirSetFileSize_v1 ( KSysDir_v1 * self,
    uint64_t size, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcUpdating, false, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        if ( truncate ( full, size ) != 0 ) switch ( errno )
        {
        case ENOENT:
            return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcNotFound );
        case EACCES:
        case EROFS:
            return RC ( rcFS, rcDirectory, rcUpdating, rcDirectory, rcUnauthorized );
        case EFBIG:
            return RC ( rcFS, rcDirectory, rcUpdating, rcParam, rcExcessive );
        case EINTR:
            return RC ( rcFS, rcDirectory, rcUpdating, rcFunction, rcIncomplete );
        case EINVAL:
            return RC ( rcFS, rcDirectory, rcUpdating, rcParam, rcInvalid );
        case EIO:
            return RC ( rcFS, rcDirectory, rcUpdating, rcTransfer, rcUnknown );
        case EISDIR:
            return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcIncorrect );
        case ELOOP:
            return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcInvalid );
        case ENAMETOOLONG:
            return RC ( rcFS, rcDirectory, rcUpdating, rcPath, rcExcessive );
        default:
            return RC ( rcFS, rcDirectory, rcUpdating, rcNoObj, rcUnknown );
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
rc_t KSysDirOpenDirRead_v1 ( const KSysDir_v1 * self,
     const KDirectory_v1 **subp, bool chroot, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc;

    rc = KSysDirMakePath_v1 ( self, rcOpening, true, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        int t;
        KSysDir_v1 *sub;

        size_t path_size = strlen ( full );
        while ( path_size > 1 && full [ path_size - 1 ] == '/' )
            full [ -- path_size ] = 0;

        t = KSysDirFullPathType_v1 ( full ) & ( kptAlias - 1 );
        if ( t == kptNotFound )
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNotFound );
        if ( t != kptDir )
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcIncorrect );

        sub = KSysDirMake_v1 ( path_size );
        if ( sub == NULL )
            rc = RC ( rcFS, rcDirectory, rcOpening, rcMemory, rcExhausted );
        else
        {
            rc = KSysDirInit_v1 ( sub, rcOpening, self -> root, full, path_size, false, chroot );
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
rc_t KSysDirOpenDirUpdate_v1 ( KSysDir_v1 * self,
    KDirectory_v1 **subp, bool chroot, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc;

    rc = KSysDirMakePath_v1 ( self, rcOpening, true, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        KSysDir_v1 *sub;

        size_t path_size = strlen ( full );
        while ( path_size > 1 && full [ path_size - 1 ] == '/' )
            full [ -- path_size ] = 0;

        switch ( KSysDirFullPathType_v1 ( full ) )
        {
        case kptNotFound:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcNotFound );
        case kptBadPath:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcInvalid );
        case kptDir:
        case kptDir | kptAlias:
            break;
        default:
            return RC ( rcFS, rcDirectory, rcOpening, rcPath, rcIncorrect );
        }

        sub = KSysDirMake_v1 ( path_size );
        if ( sub == NULL )
            rc = RC ( rcFS, rcDirectory, rcOpening, rcMemory, rcExhausted );
        else
        {
            rc = KSysDirInit_v1 ( sub, rcOpening, self -> root, full, path_size, true, chroot );
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
rc_t KSysDirCreateDir_v1 ( KSysDir_v1 * self,
    uint32_t access, KCreateMode mode, const char *path, va_list args )
{
    char full [ PATH_MAX ];
    rc_t rc = KSysDirMakePath_v1 ( self, rcCreating, true, full, sizeof full, path, args );
    if ( rc == 0 )
    {
        if ( ( mode & kcmValueMask ) == kcmCreate )
        {
            switch ( KSysDirFullPathType_v1 ( full ) )
            {
            case kptNotFound:
                break;
            case kptBadPath:
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcInvalid );
            case kptDir:
                return RC ( rcFS, rcDirectory, rcCreating, rcDirectory, rcExists );
            default:
                return RC ( rcFS, rcDirectory, rcCreating, rcPath, rcIncorrect );
            }
        }

        rc = make_dir_v1 ( full, access );
        if ( rc != 0 ) switch ( GetRCState ( rc ) )
        {
        case rcExists:
            rc = 0;
            if ( ( mode & kcmValueMask ) == kcmInit )
                rc = KSysDirEmptyDir_v1 ( full, sizeof full, 1 );
            break;
        case rcNotFound:
            if ( ( mode & kcmParents ) != 0 )
                rc = KSysDirCreateParents_v1 ( self, full, access, false );
            break;
        default:
            break;
        }
    }
    return rc;
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
    /* TBD - could return an inode */
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

static KDirectory_vt_v1 vtKSysDir =
{
    /* version 1.4 */
    1, 4,

    /* start minor version 0*/
    KSysDirDestroy_v1,
    KSysDirList_v1,

    /* the following two messages map to the same method, requiring type casting */
    ( rc_t ( * )  ( const KSysDir_v1*, bool,
        rc_t ( * ) ( const KDirectory_v1*, uint32_t, const char*, void* ), void*,
       const char*, va_list ) ) KSysDirVisit_v1,
    ( rc_t ( * ) ( KSysDir_v1*, bool,
        rc_t ( * ) ( KDirectory_v1*, uint32_t, const char*, void* ), void*,
       const char*, va_list ) ) KSysDirVisit_v1,

    KSysDirPathType_v1,
    KSysDirResolvePath_v1,
    KSysDirResolveAlias_v1,
    KSysDirRename_v1,
    KSysDirRemove_v1,
    KSysDirClearDir_v1,
    KSysDirVAccess,
    KSysDirSetAccess_v1,
    KSysDirCreateAlias_v1,
    KSysDirOpenFileRead_v1,
    KSysDirOpenFileWrite_v1,
    KSysDirCreateFile_v1,
    KSysDirFileSize_v1,
    KSysDirSetFileSize_v1,
    KSysDirOpenDirRead_v1,
    KSysDirOpenDirUpdate_v1,
    KSysDirCreateDir_v1,
    NULL, /* we don't track files*/
    /* end minor version 0*/

    /* start minor version 1*/
    KSysDirVDate,
    KSysDirVSetDate,
    KSysDirGetSysdir_v1,
    /* end minor version 1*/

    /* start minor version 2 */
    KSysDirFileLocator_v1,
    /* end minor version 2 */

    /* start minor version 3 */
    KSysDirFilePhysicalSize_v1,
    KSysDirFileContiguous_v1,
    /* end minor version 3 */

    /* start minor version 4 */
    KSysDirOpenFileWrite_v1
    /* end minor version 4 */
};

/* KSysDirInit
 */
static
rc_t KSysDirInit_v1 ( KSysDir_v1 * self, enum RCContext ctx, uint32_t dad_root,
    const char *path, uint32_t path_size, bool update, bool chroot )
{
    rc_t rc;

    rc = KDirectoryInit ( & self -> dad, ( const KDirectory_vt * ) & vtKSysDir,
                          "KSysDir", path?path:"(null)", update );
    if ( rc != 0 )
        return ResetRCContext ( rc, rcFS, rcDirectory, ctx );

    if ( path != NULL )
        memmove ( self -> path, path, path_size );
    self -> root = chroot ? path_size : dad_root;
    self -> size = path_size + 1;
    self -> path [ path_size ] = '/';
    self -> path [ path_size + 1 ] = 0;

    return 0;
}

extern rc_t CC ReportCWD ( const ReportFuncs *f, uint32_t indent );
extern rc_t CC ReportRedirect ( KWrtHandler* handler,
    const char* filename, bool* to_file, bool finalize );

LIB_EXPORT rc_t CC KDirectoryNativeDir_v1 ( KDirectory_v1 **dirp )
{
    rc_t rc;
    KSysDir_v1 *dir;
    uint32_t size;
    char wd [ PATH_MAX ];

    static bool latch;
    if ( ! latch )
    {
        ReportInitKFS ( ReportCWD, ReportRedirect );
        latch = true;
    }

    if ( dirp == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcParam, rcNull );

    * dirp = NULL;

    if ( realpath ( ".", wd ) == NULL ) switch ( errno )
    {
    case EACCES:
        return RC ( rcFS, rcDirectory, rcAccessing, rcDirectory, rcUnauthorized );
    case EIO:
        return RC ( rcFS, rcDirectory, rcAccessing, rcTransfer, rcUnknown );
    default:
        return RC ( rcFS, rcDirectory, rcAccessing, rcNoObj, rcUnknown );
    }

    size = strlen ( wd );
    if ( size + 2 > sizeof wd )
        return RC ( rcFS, rcDirectory, rcAccessing, rcBuffer, rcInsufficient );

    /* trim trailing slash */
    if ( size > 0 && wd [ size - 1 ] == '/' )
        wd [ -- size ] = 0;

    dir = KSysDirMake_v1 ( size );
    if ( dir == NULL )
        rc = RC ( rcFS, rcDirectory, rcAccessing, rcMemory, rcExhausted );
    else
    {
        rc = KSysDirInit_v1 ( dir, rcAccessing, 0, wd, size, true, false );
        if ( rc == 0 )
        {
            * dirp = & dir -> dad;
            return 0;
        }

        free ( dir );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDirectoryGetDiskFreeSpace_v1 ( const KDirectory * self,
    uint64_t * free_bytes_available, uint64_t * total_number_of_bytes )
{
    if ( self == NULL )
        return RC ( rcFS, rcDirectory, rcAccessing, rcSelf, rcNull );
    else {
        KSysDir_v1 * dir = ( KSysDir_v1 * ) self;
        struct statvfs buf;
        memset ( & buf, 0, sizeof buf );
        if ( statvfs ( dir -> path, & buf) == 0 ) {
            if ( free_bytes_available != NULL ) {
                * free_bytes_available  = buf . f_bavail * buf . f_frsize;
            }
            if ( total_number_of_bytes != NULL ) {
                * total_number_of_bytes = buf . f_blocks * buf . f_frsize;
            }
            return 0;
        }

        return RC ( rcFS, rcDirectory, rcAccessing, rcError, rcUnknown );
    }
}
