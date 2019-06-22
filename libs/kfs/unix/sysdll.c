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

#include "va_copy.h"
#include "sysdir-priv.h"

#include <kfs/extern.h>
#include <kfs/dyload.h>
#include <kfs/directory.h>
#include <klib/refcount.h>
#include <klib/vector.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/out.h>
#include <klib/log.h>
#include <klib/status.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include "os-native.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

/* old Sun includes won't define PATH_MAX */

#ifndef __XOPEN_OR_POSIX
#define __XOPEN_OR_POSIX 1
#endif

#include <limits.h>

#undef __XOPEN_OR_POSIX

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <dlfcn.h>


#if _DEBUGGING
#define DLOPEN_MODE RTLD_NOW
#else
#define DLOPEN_MODE RTLD_LAZY
#endif

#define ALWAYS_ADD_EXE 1
#define USE_DYLOAD 1

/* PATH_MAX is not really universal even on Linux/Unix */
#ifndef PATH_MAX
#define PATH_MAX        4096
#endif

/*--------------------------------------------------------------------------
 * KDirectory
 */
static
void KDirRefRelease ( void *item, void *ignore )
{
    KDirectoryRelease ( ( const void* ) item );
}

/*--------------------------------------------------------------------------
 * KDyld
 *  dynamic library loader
 *
 *  maintains cache of libraries it has opened while they remain open
 *  such that subsequent requests for an open library will return a
 *  new reference to the existing library.
 */
struct KDyld
{
    Vector search;
    KRefcount refcount;
};


/* Whack
 */
static
rc_t KDyldWhack ( KDyld *self )
{
    KRefcountWhack ( & self -> refcount, "KDyld" );

    VectorWhack ( & self -> search, KDirRefRelease, NULL );
    free ( self );

    return 0;
}


/* Make
 *  create a dynamic loader object
 *
 *  "dl" [ OUT ] - return parameter for loader
 */
LIB_EXPORT rc_t CC KDyldMake ( KDyld **dlp )
{
    rc_t rc;

    if ( dlp == NULL )
        rc = RC ( rcFS, rcDylib, rcConstructing, rcParam, rcNull );
    else
    {
        KDyld *dl = malloc ( sizeof * dl );
        if ( dl == NULL )
            rc = RC ( rcFS, rcDylib, rcConstructing, rcMemory, rcExhausted );
        else
        {
            VectorInit ( & dl -> search, 1, 8 );
            KRefcountInit ( & dl -> refcount, 1, "KDyld", "make", "dl" );

            * dlp = dl;
            return 0;
        }

        * dlp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KDyldAddRef ( const KDyld *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDyld" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcDylib, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KDyldRelease ( const KDyld *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KDyld" ) )
        {
        case krefWhack:
            return KDyldWhack ( ( KDyld* ) self );
        case krefNegative:
            return RC ( rcFS, rcDylib, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Attach
 * Sever
 */
static
KDyld *KDyldAttach ( const KDyld *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KDyld" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KDyld* ) self;
}

static
rc_t KDyldSever ( const KDyld *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KDyld" ) )
        {
        case krefWhack:
            return KDyldWhack ( ( KDyld* ) self );
        case krefNegative:
            return RC ( rcFS, rcDylib, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* AddSearchPath
 *  add a search path to loader for locating library files
 */
LIB_EXPORT rc_t CC KDyldVAddSearchPath ( KDyld *self, const char *path, va_list args )
{
    rc_t rc;
    if ( self == NULL )
        rc = RC ( rcFS, rcDylib, rcUpdating, rcSelf, rcNull );
    else
    {
        KDirectory *wd;
        rc = KDirectoryNativeDir ( & wd );
        if ( rc == 0 )
        {
            const KDirectory *dir;
            rc = KDirectoryVOpenDirRead ( wd, & dir, false, path, args );
            if ( rc == 0 )
            {
                rc = VectorAppend ( & self -> search, NULL, dir );
                if ( rc != 0 )
                    KDirectoryRelease ( dir );
            }

            KDirectoryRelease ( wd );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDyldAddSearchPath ( KDyld *self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDyldVAddSearchPath ( self, path, args );
    va_end ( args );

    return rc;
}

static
void KDyldForEach ( const KDyld *self,
    void ( * f ) ( const KDirectory *dir, void *data ), void *data )
{
    VectorForEach ( & self -> search, false,
        ( void ( * ) ( void*, void* ) ) f, data );
}


/* HomeDirectory
 *  returns a KDirectory where the binary for a given function is located
 *
 *  "dir" [ OUT ] - return parameter for home directory ( read-only ), if found
 *
 *  "func" [ IN ] - function pointer within binary to be located
 */
LIB_EXPORT rc_t CC KDyldHomeDirectory ( const KDyld *self, const KDirectory **dir, fptr_t func )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcFS, rcDylib, rcSearching, rcParam, rcNull );
    else
    {
        * dir = NULL;

        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcSearching, rcSelf, rcNull );
        else if ( func == NULL )
            rc = RC ( rcFS, rcDylib, rcSearching, rcFunction, rcNull );
        else
        {
            Dl_info info;
            memset ( & info, 0, sizeof info );
            if ( dladdr ( ( void* ) func, & info ) == 0 )
                rc = RC ( rcFS, rcDylib, rcSearching, rcFunction, rcNotFound );
            else
            {
                KDirectory *wd;
                rc = KDirectoryNativeDir ( & wd );
                if ( rc == 0 )
                {
                    /* turn this into a real path */
                    const KSysDir *sdir = KDirectoryGetSysDir ( wd );
                    if ( sdir == NULL )
                        rc = RC ( rcFS, rcDylib, rcSearching, rcDirectory, rcIncorrect );
                    else
                    {
                        /* "dladdr" will return a simple name rather than a path
                           when the address is within the application itself and
                           the application was found using PATH. this is brilliant
                           design at its best. */
                        char thanks_for_brilliant_APIs [ PATH_MAX ];
                        const char *dli_fname = info . dli_fname;

                        /* check for a path rather than a name */
                        const char *last_slash = strrchr ( info . dli_fname, '/' );
                        if ( last_slash == NULL )
                        {
                            /* simple name - get PATH */
                            const char *PATH = getenv ( "PATH" );
                            rc = RC ( rcFS, rcDylib, rcSearching, rcPath, rcNotFound );
                            if ( PATH != NULL )
                            {
                                /* loop over PATH */
                                const char *path_start, *path_end;
                                for ( path_start = PATH;; path_start = path_end + 1 )
                                {
                                    /* look for non-empty directory */
                                    path_end = strchr ( path_start, ':' );
                                    if ( path_start != path_end && path_start [ 0 ] != 0 )
                                    {
                                        rc_t rc2;
                                        uint32_t path_type;

                                        /* handle last element in list */
                                        if ( path_end == NULL )
                                            last_slash = path_start + strlen ( path_start );
                                        else for ( last_slash = path_end; last_slash > path_start; -- last_slash )
                                        {
                                            if ( last_slash [ -1 ] != '/' )
                                                break;
                                        }

                                        /* create possible path, using up to ':' */
                                        rc2 = string_printf ( thanks_for_brilliant_APIs, sizeof thanks_for_brilliant_APIs, NULL,
                                                              "%.*s/%s", ( int ) ( last_slash - path_start ), path_start, dli_fname );

                                        /* if failed to create path string */
                                        if ( rc2 != 0 )
                                            break;

                                        /* check path against working directory */
                                        path_type = KDirectoryPathType ( wd, "%s", thanks_for_brilliant_APIs );
                                        if ( ( path_type & ~ kptAlias ) == kptFile )
                                        {
                                            uint32_t access = 0;
                                            rc = KDirectoryAccess ( wd, & access, "%s", thanks_for_brilliant_APIs );
                                            if ( rc != 0 )
                                                break;

                                            /* try to do a quick check that the file can be executed.
                                               but it could fail to do the right guess. */
                                            if ( access & 0100 || access & 0010 || access & 0001 ) {
                                                /* this is a file, which can be assumed to be an executable */
                                                dli_fname = thanks_for_brilliant_APIs;
                                                last_slash
                                                    = & thanks_for_brilliant_APIs [ last_slash - path_start ];
                                                rc = 0;
                                                break;
                                            }
                                        }
                                    }

                                    /* exit if no more paths */
                                    if ( path_end == NULL )
                                        break;
                                }
                            }
                        }

                        if ( rc == 0 )
                        {
                            char real [ PATH_MAX ];
                            rc = KSysDirRealPath ( sdir, real, sizeof real, "%.*s"
                                , ( int ) ( last_slash - dli_fname ), dli_fname );

                            if ( rc == 0 )
                                rc = KDirectoryOpenDirRead ( wd, dir, false, "%s", real );

                            DBGMSG(DBG_KFS, DBG_FLAG(DBG_KFS_DIR), ("%s: %R path is '%s'\n", __func__, rc, real));
                        }
                    }

                    KDirectoryRelease ( wd );
                }
            }
        }
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KDylib
 *  Unix dynamic library
 */
struct KDylib
{
    void *handle;
    String path;
    KRefcount refcount;
};


/* Whack
 */
static
rc_t KDylibWhack ( KDylib *self )
{
    KRefcountWhack ( & self -> refcount, "KDylib" );


/* Darwin, especially before 10.5 doesn't/didn't do this well */
#if ! MAC
    /* try to close library */
    if ( self -> handle && dlclose ( self -> handle ) )
    {
        /* report error */
        const char *msg = dlerror ();
        rc_t rc = RC ( rcFS, rcDylib, rcClosing, rcNoObj, rcUnknown );
        LOGERR ( klogInt, rc, msg );
        ( void ) msg;

        return rc;
    }
#endif
    free ( self );
    return 0;
}


/* Make
 */
static
rc_t KDylibMake ( KDylib **libp, const String *path )
{
    KDylib *lib = malloc ( sizeof * lib + path -> size + 1 );
    if ( lib == NULL )
        return RC ( rcFS, rcDylib, rcConstructing, rcMemory, rcExhausted );

    lib -> handle = NULL;
    string_copy ( ( char* ) ( lib + 1 ), path -> size + 1, path -> addr, path -> size );
    StringInit ( & lib -> path, ( char* ) ( lib + 1 ), path -> size, path -> len );
    KRefcountInit ( & lib -> refcount, 1, "KDylib", "make", lib -> path . addr );

    * libp = lib;
    return 0;
}


/* SetLogging
 */
static
rc_t KDylibSetLogging ( const KDylib *self )
{
    rc_t ( CC * set_formatter ) ( KFmtWriter writer, KLogFmtFlags flags, void *data );
    rc_t ( CC * set_writer ) ( KWrtWriter writer, void *data );
    rc_t ( CC * set_level ) ( KLogLevel lvl );
    
    if ( ! self -> handle )
    {
        return 0;
    }

    /* set the current logging level */
    set_level = ( rc_t ( * ) ( KLogLevel ) ) dlsym ( self -> handle, "KLogLevelSet" );
    if ( set_level != NULL )
    {
        KLogLevel lvl = KLogLevelGet ();
        ( * set_level ) ( lvl );
    }

    /* determine current library logging */
    set_writer = ( rc_t ( * ) ( KWrtWriter, void* ) ) dlsym ( self -> handle, "KOutHandlerSet" );
    if ( set_writer != NULL ) {
        const KWrtHandler* handler = KOutHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }

    set_formatter = ( rc_t ( * ) ( KFmtWriter, KLogFmtFlags, void* ) ) dlsym ( self -> handle, "KLogLibFmtHandlerSet" );
    if ( set_formatter != NULL ) {
        KLogFmtFlags flags = KLogLibFmtFlagsGet ();
        const KFmtHandler* fmt_handler = KLogFmtHandlerGet ();
        ( * set_formatter ) ( fmt_handler -> formatter, flags, fmt_handler -> data );
    }
    set_writer = ( rc_t ( * ) ( KWrtWriter, void* ) ) dlsym ( self -> handle, "KLogLibHandlerSet" );
    if ( set_writer != NULL ) {
        const KWrtHandler* handler = KLogLibHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }

    set_formatter = ( rc_t ( * ) ( KFmtWriter, KLogFmtFlags, void* ) ) dlsym ( self -> handle, "KStsLibFmtHandlerSet" );
    if ( set_formatter != NULL ) {
        KStsFmtFlags flags = KStsLibFmtFlagsGet ();
        const KFmtHandler* fmt_handler = KStsFmtHandlerGet ();
        ( * set_formatter ) ( fmt_handler -> formatter, flags, fmt_handler -> data );
    }
    set_writer = ( rc_t ( * ) ( KWrtWriter, void* ) ) dlsym ( self -> handle, "KStsLibHandlerSet" );
    if ( set_writer != NULL ) {
        const KWrtHandler* handler = KStsLibHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }
#if _DEBUGGING
    set_writer = ( rc_t ( * ) ( KWrtWriter, void* ) ) dlsym ( self -> handle, "KDbgHandlerSet" );
    if ( set_writer != NULL ) {
        const KWrtHandler* handler = KDbgHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }
#endif
    return 0;
}

/* LoadLib
 *  load a dynamic library
 *
 *  "lib" [ OUT ] - return parameter for loaded library
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target library
 */
static
rc_t KDyldLoad ( KDyld *self, KDylib *lib, const char *path )
{
/* (VDB-1391) remove dynamic linker interfaces from system */
#if USE_DYLOAD
    rc_t rc;
    const char *msg;
    size_t msg_len;

    lib -> handle = dlopen ( path, path == NULL ? RTLD_LAZY : DLOPEN_MODE );
    if ( lib -> handle != NULL )
        return KDylibSetLogging ( lib );

    msg = dlerror ();
    rc = RC ( rcFS, rcDylib, rcLoading, rcNoObj, rcUnknown );

    msg_len = strlen(msg);
    if ( msg_len > lib -> path . size + 2 )
    {
        const char *cmp = & msg [ lib -> path . size + 2 ];
        if ( strcmp ( cmp, "cannot open shared object file: No such file or directory" ) == 0 )
            rc = RC ( rcFS, rcDylib, rcLoading, rcPath, rcNotFound );
        else if ( strncmp ( cmp, "undefined symbol: ", sizeof "undefined symbol: " - 1 ) == 0 )
            rc = RC ( rcFS, rcDylib, rcLoading, rcDylib, rcIncomplete );
    }
    if (GetRCState(rc) == rcUnknown) {
        static const char imageNotFound[] = " image not found";
        const char *cmp1 = strstr(msg, imageNotFound);
        const char *cmp2 = msg + msg_len - (sizeof(imageNotFound) - 1);
        if (cmp1 == cmp2)
            rc = RC ( rcFS, rcDylib, rcLoading, rcPath, rcNotFound );
    }
    
    DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_DLL), ("%s: %R %s\n", __func__, rc, msg));
    if (GetRCState(rc) == rcUnknown) {
        (void)LOGMSG(klogWarn, (msg));
    }
    
    return rc;
#else
    lib -> handle = NULL;
    return 0;
#endif    
}

static
rc_t KDyldVTryLoadLib ( KDyld *self, KDylib **lib,
    const KDirectory *dir, const char *path, va_list args )
{
    rc_t rc;

    const KSysDir *sdir = KDirectoryGetSysDir ( dir );
    if ( sdir == NULL )
        rc = RC ( rcFS, rcDylib, rcLoading, rcDirectory, rcIncorrect );
    else
    {
        char real [ PATH_MAX ];
        rc = KSysDirVRealPath ( sdir, real, sizeof real, path, args );
        if ( rc == 0 )
        {
            String pstr;
            StringInitCString ( & pstr, real );

            rc = KDylibMake ( lib, & pstr );
            if ( rc == 0 )
            {
                rc = KDyldLoad ( self, * lib, real );
                if ( rc == 0 )
                    return 0;

                free ( * lib );
            }
        }
    }

    * lib = NULL;

    return rc;
}

static
rc_t KDyldTryLoadLib ( KDyld *self, KDylib **lib,
    const KDirectory *dir, const char *path, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, path );
    rc = KDyldVTryLoadLib ( self, lib, dir, path, args );
    va_end ( args );
    return rc;
}

LIB_EXPORT rc_t CC KDyldVLoadLib ( KDyld *self,
    KDylib **lib, const char *path, va_list args )
{
    rc_t rc;

    if ( lib == NULL )
        rc = RC ( rcFS, rcDylib, rcLoading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcLoading, rcSelf, rcNull );
        else if ( path == NULL || path [ 0 ] == 0 )
        {
            String pstr;
            CONST_STRING ( & pstr, "" );

            rc = KDylibMake ( lib, & pstr );
            if ( rc == 0 )
            {
                rc = KDyldLoad ( self, * lib, NULL );
                if ( rc == 0 )
                    return 0;
            
                free ( * lib );
            }
        }
        else
        {
            uint32_t i = VectorStart ( & self -> search );
            uint32_t end = i + VectorLength ( & self -> search );

            if ( i == end )
            {
                char name [ 4096 ];
                int len = ( args == NULL ) ?
                    snprintf  ( name, sizeof name, "%s", path ) :
                    vsnprintf ( name, sizeof name, path, args );
                if ( len < 0 || len >= sizeof name )
                    rc = RC ( rcFS, rcDylib, rcLoading, rcPath, rcExcessive );
                else
                {
                    String pstr;
                    StringInit ( & pstr, name, len, string_len ( name, len ) );

                    rc = KDylibMake ( lib, & pstr );
                    if ( rc == 0 )
                    {
                        rc = KDyldLoad ( self, * lib, name );
                        if ( rc == 0 )
                            return 0;
                    
                        free ( * lib );
                    }
                }
            }
            else
            {
                for ( * lib = NULL; i < end; ++ i )
                {
                    const KDirectory *dir;

                    va_list cpy;
                    va_copy ( cpy, args );

                    dir = ( const void* ) VectorGet ( & self -> search, i );
                    rc = KDyldVTryLoadLib ( self, lib, dir, path, cpy );

                    va_end ( cpy );

                    if ( rc == 0 || GetRCState ( rc ) != rcNotFound )
                        return rc;
                }

                rc = RC ( rcFS, rcDylib, rcLoading, rcPath, rcNotFound );
            }
        }

        * lib = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KDyldLoadLib ( KDyld *self,
    KDylib **lib, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDyldVLoadLib ( self, lib, path, args );
    va_end ( args );

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KDylibAddRef ( const KDylib *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDylib" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcDylib, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KDylibRelease ( const KDylib *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KDylib" ) )
        {
        case krefWhack:
            return KDylibWhack ( ( KDylib* ) self );
        case krefNegative:
            return RC ( rcFS, rcDylib, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

static
void KDylibVectRelease ( void *item, void *ignore )
{
    KDylib *self = item;
    KDylibRelease ( self );
}


/* Attach
 * Sever
 */
static
KDylib *KDylibAttach ( const KDylib *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KDylib" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KDylib* ) self;
}

static
rc_t KDylibSever ( const KDylib *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KDylib" ) )
        {
        case krefWhack:
            return KDylibWhack ( ( KDylib* ) self );
        case krefNegative:
            return RC ( rcFS, rcDylib, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Sort
 */
static
int64_t KDylibSort ( const void *item, const void *n )
{
    const KDylib *a = item;
    const KDylib *b = n;
    return StringOrderNoNullCheck ( & a -> path, & b -> path );
}


/* FullPath
 *  return full path to library
 */
LIB_EXPORT rc_t CC KDylibFullPath ( const KDylib *self, char *path, size_t psize )
{
    rc_t rc;

    if ( psize == 0 )
        rc = RC ( rcFS, rcDylib, rcAccessing, rcBuffer, rcInsufficient );
    else if ( path == NULL )
        rc = RC ( rcFS, rcDylib, rcAccessing, rcBuffer, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcAccessing, rcSelf, rcNull );
        else
        {
            size_t bytes = string_copy ( path, psize,
                self -> path . addr, self -> path . size );
            if ( bytes < psize )
                return 0;

            rc = RC ( rcFS, rcDylib, rcAccessing, rcBuffer, rcInsufficient );
        }

        path [ 0 ] = 0;
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KDlset
 *  ordered set of dynamic libraries
 *  contained libraries remain resident until set is released
 */
struct KDlset
{
    KDyld *dl;
    Vector name, ord;
    KRefcount refcount;
};


/* Whack
 */
static
rc_t KDlsetWhack ( KDlset *self )
{
    KRefcountWhack ( & self -> refcount, "KDlset" );

    VectorWhack ( & self -> name, NULL, NULL );
    VectorWhack ( & self -> ord, KDylibVectRelease, NULL );
    KDyldSever ( self -> dl );
    free ( self );

    return 0;
}

#define STRINGIZE(s) #s
#define LIBNAME(pref, name, suff) STRINGIZE(pref) name STRINGIZE(suff)
/* MakeSet
 *  load a dynamic library
 *
 *  "set" [ OUT ] - return parameter for lib set
 */
LIB_EXPORT rc_t CC KDyldMakeSet ( const KDyld *self, KDlset **setp )
{
    rc_t rc = 0;

    if ( setp == NULL )
        rc = RC ( rcFS, rcDylib, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcConstructing, rcSelf, rcNull );
        else
        {
            KDlset *set = malloc ( sizeof * set );
            if ( set == NULL )
                rc = RC ( rcFS, rcDylib, rcConstructing, rcMemory, rcExhausted );
            else
            {
                set -> dl = KDyldAttach ( self );
                VectorInit ( & set -> name, 0, 16 );
                VectorInit ( & set -> ord, 0, 16 );
                KRefcountInit ( & set -> refcount, 1, "KDlset", "make", "dlset" );
#if ! ALWAYS_ADD_EXE
                {   
                    KDylib *jni;
                    const char* libname = LIBNAME(LIBPREFIX, "vdb_jni.", SHLIBEXT);
                    if ( KDyldLoadLib ( ( KDyld* ) self, & jni, libname ) == 0 )
                    {
                        rc = KDlsetAddLib ( set, jni );
                        KDylibRelease ( jni );
                    }
                    if (rc == 0)
                    {
                        * setp = set;
                        return 0;
                    }
                }
#else
                {
                    KDylib *exe;
                    rc = KDyldLoadLib ( ( KDyld* ) self, & exe, NULL );
                    if ( rc == 0 )
                    {
                        rc = KDlsetAddLib ( set, exe );
                        KDylibRelease ( exe );
                        if ( rc == 0 )
                        {
                            * setp = set;
                            return 0;
                        }
                    }
                }
#endif
                KDlsetRelease ( set );
            }
        }

        * setp = NULL;
    }

    return rc;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KDlsetAddRef ( const KDlset *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDlset" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcDylib, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KDlsetRelease ( const KDlset *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KDlset" ) )
        {
        case krefWhack:
            return KDlsetWhack ( ( KDlset* ) self );
        case krefNegative:
            return RC ( rcFS, rcDylib, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* AddLib
 *  adds a dynamic library to end of ordered set
 *
 *  "lib" [ IN ] - library returned from KDyldLoadLib
 */
static
rc_t KDlsetAddLibInt ( KDlset *self, KDylib *lib )
{
    uint32_t idx;
    rc_t rc = VectorAppend ( & self -> ord, & idx, lib );
    if ( rc == 0 )
    {
        void *ignore;

        rc = VectorInsertUnique ( & self -> name,
            lib, NULL, KDylibSort );
        if ( rc == 0 )
            return 0;

        VectorSwap ( & self -> ord, idx, NULL, & ignore );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDlsetAddLib ( KDlset *self, KDylib *lib )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcFS, rcDylib, rcInserting, rcSelf, rcNull );
    else if ( lib == NULL )
        rc = RC ( rcFS, rcDylib, rcInserting, rcDylib, rcNull );
    else
    {
        rc = KDylibAddRef ( lib );
        if ( rc == 0 )
        {
            rc = KDlsetAddLibInt ( self, lib );
            if ( rc == 0 )
                return 0;

            KDylibRelease ( lib );
        }
    }

    return rc;
}


/* AddAll
 *  adds all dynamic libraries found in dl search path
 */
static
rc_t KDlsetTryLib ( const KDirectory *dir,
    uint32_t type, const char *name, void *data )
{
    KDlset *self = data;

    if ( ( type & ~ kptAlias ) == kptFile )
    {
        rc_t rc;
        KDylib *lib;
#ifdef SHLX
        /* force simple shared library extension */
        if ( sizeof SHLX >= 2 )
        {
            /* SHLX has at least 1 character plus NUL byte */
            uint32_t len = strlen ( name );
            /* name must be at least 1 character larger */
            if ( len <= ( sizeof SHLX - 1 ) )
                return 0;
            /* name must end with shared library extension */
            if ( memcmp ( & name [ len - ( sizeof SHLX - 1 ) ], SHLX, sizeof SHLX - 1 ) != 0 )
                return 0;
        }
#endif
        rc = KDyldTryLoadLib ( self -> dl, & lib, dir, name );
        if ( rc == 0 )
        {
            rc = KDlsetAddLibInt ( self, lib );
            if ( rc == 0 )
                return 0;

            KDylibRelease ( lib );
        }
    }

    return 0;
}

static
void KDlsetVisitDir ( const KDirectory *dir, void *data )
{
    KDirectoryVisit ( dir, false, KDlsetTryLib, data, "." );
}

LIB_EXPORT rc_t CC KDlsetAddAll ( KDlset *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcDylib, rcInserting, rcSelf, rcNull );
    KDyldForEach ( self -> dl, KDlsetVisitDir, self );
    return 0;
}


/*--------------------------------------------------------------------------
 * KSymAddr
 *  symbol address within a dynamic library
 */
struct KSymAddr
{
    KDylib *lib;
    void *addr;
    KRefcount refcount;
};


/* Whack
 */
static
rc_t KSymAddrWhack ( KSymAddr *self )
{
    KRefcountWhack ( & self -> refcount, "KSymAddr" );

    KDylibSever ( self -> lib );
    free ( self );

    return 0;
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KSymAddrAddRef ( const KSymAddr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KSymAddr" ) )
        {
        case krefLimit:
            return RC ( rcFS, rcDylib, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KSymAddrRelease ( const KSymAddr *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KSymAddr" ) )
        {
        case krefWhack:
            return KSymAddrWhack ( ( KSymAddr* ) self );
        case krefNegative:
            return RC ( rcFS, rcDylib, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make
 */
static
rc_t KSymAddrMake ( KSymAddr **symp,
    const KDylib *lib, const char *name )
{
    if ( lib -> handle )
    {
        void *addr = dlsym ( lib -> handle, name );
        const char *estr = dlerror();
        
        if ( addr != NULL || estr == NULL )
        {
            KSymAddr *sym = malloc ( sizeof * sym );
            if ( sym == NULL )
                return RC ( rcFS, rcDylib, rcConstructing, rcMemory, rcExhausted );

            sym -> lib = KDylibAttach ( lib );
            sym -> addr = addr;
            KRefcountInit ( & sym -> refcount, 1, "KSymAddr", "make", name );
            * symp = sym;
            return 0;
        }
    }
    * symp = NULL;
    return RC ( rcFS, rcDylib, rcSelecting, rcName, rcNotFound );
}


/* Symbol
 *  find a symbol within dynamic library
 *
 *  "sym" [ OUT ] - return parameter for exported symbol address
 *
 *  "name" [ IN ] - NUL terminated symbol name in
 *  library-native character set
 */
LIB_EXPORT rc_t CC KDylibSymbol ( const KDylib *self, KSymAddr **sym, const char *name )
{
    rc_t rc;

    if ( sym == NULL )
        rc = RC ( rcFS, rcDylib, rcSelecting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcSelf, rcNull );
        else if ( name == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNull );
        else if ( name [ 0 ] == 0 )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcEmpty );
        else
        {
            return KSymAddrMake ( sym, self, name );
        }

        * sym = NULL;
    }

    return rc;
}

typedef struct KDlsetTrySymData KDlsetTrySymData;
struct KDlsetTrySymData
{
    const KDlset *self;
    const char *name;

    bool ( CC * test ) ( const KSymAddr *sym, void *data );
    void *data;

    KSymAddr *sym;
    rc_t rc;
    bool first;
};

static
bool KDlsetTrySymbol ( void *item, void *data )
{
    KSymAddr *sym;
    KDlsetTrySymData *pb = data;
    pb -> rc = KDylibSymbol ( item, & sym, pb -> name );
    if ( pb -> rc == 0 )
    {
        /* simple case */
        if ( pb -> test == NULL )
        {
            pb -> sym = sym;
            return true;
        }

        /* apply filter function */
        if ( ( * pb -> test ) ( sym, pb -> data ) )
        {
            KSymAddrRelease ( pb -> sym );
            pb -> sym = sym;
            return pb -> first;
        }

        KSymAddrRelease ( sym );
    }
    return false;
}

LIB_EXPORT rc_t CC KDlsetSymbol ( const KDlset *self, KSymAddr **sym, const char *name )
{
    rc_t rc;

    if ( sym == NULL )
        rc = RC ( rcFS, rcDylib, rcSelecting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcSelf, rcNull );
        else if ( name == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNull );
        else if ( name [ 0 ] == 0 )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcEmpty );
        else
        {
            KDlsetTrySymData pb;
            memset ( & pb, 0, sizeof pb );
            pb . self = self;
            pb . name = name;
            pb . rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNotFound );

            if ( VectorDoUntil ( & self -> ord, false, KDlsetTrySymbol, & pb ) )
            {
                * sym = pb . sym;
                return 0;
            }

            rc = pb . rc;
        }

        * sym = NULL;
    }

    return rc;
}


/* FirstSymbol
 * LastSymbol
 *  find a symbol within dynamic library set matching criteria
 *
 *  "sym" [ OUT ] - return parameter for exported symbol address
 *
 *  "name" [ IN ] - NUL terminated symbol name in
 *  library-native character set
 *
 *  "test" [ IN ] and "data" [ IN, OPAQUE ] - callback filter function
 *  return true if symbol matches criteria
 */
LIB_EXPORT rc_t CC KDlsetFirstSymbol ( const KDlset *self, KSymAddr **sym, const char *name,
    bool ( CC * test ) ( const KSymAddr *sym, void *data ), void *data )
{
    rc_t rc;

    if ( sym == NULL )
        rc = RC ( rcFS, rcDylib, rcSelecting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcSelf, rcNull );
        else if ( name == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNull );
        else if ( name [ 0 ] == 0 )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcEmpty );
        else if ( test == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcFunction, rcNull );
        else
        {
            KDlsetTrySymData pb;
            memset ( & pb, 0, sizeof pb );
            pb . self = self;
            pb . name = name;
            pb . test = test;
            pb . data = data;
            pb . rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNotFound );
            pb . first = true;

            if ( VectorDoUntil ( & self -> ord, false, KDlsetTrySymbol, & pb ) )
            {
                * sym = pb . sym;
                return 0;
            }

            rc = pb . rc;
        }

        * sym = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KDlsetLastSymbol ( const KDlset *self, KSymAddr **sym, const char *name,
    bool ( CC * test ) ( const KSymAddr *sym, void *data ), void *data )
{
    rc_t rc;

    if ( sym == NULL )
        rc = RC ( rcFS, rcDylib, rcSelecting, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcSelf, rcNull );
        else if ( name == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNull );
        else if ( name [ 0 ] == 0 )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcEmpty );
        else if ( test == NULL )
            rc = RC ( rcFS, rcDylib, rcSelecting, rcFunction, rcNull );
        else
        {
            KDlsetTrySymData pb;
            memset ( & pb, 0, sizeof pb );
            pb . self = self;
            pb . name = name;
            pb . test = test;
            pb . data = data;
            pb . rc = RC ( rcFS, rcDylib, rcSelecting, rcName, rcNotFound );

            VectorDoUntil ( & self -> ord, false, KDlsetTrySymbol, & pb );
            if ( pb . sym != NULL )
            {
                * sym = pb . sym;
                return 0;
            }

            rc = pb . rc;
        }

        * sym = NULL;
    }

    return rc;
}


/* List - PRIVATE
 *  list the paths to the libraries in the set
 */
typedef struct list_dylib_param list_dylib_param;
struct list_dylib_param
{
    VNamelist *list;
    rc_t rc;
};

static
bool CC list_dylib ( void *item, void *data )
{
    list_dylib_param *pb = data;
    const KDylib *lib = ( const void* ) item;

    /* "lib" was created with KDylibMake
       which creates a NUL terminated path.
       of course, this could seg-fault if bad... */
    assert ( lib -> path . addr [ lib -> path . size ] == 0 );

    pb -> rc = VNamelistAppend ( pb -> list, lib -> path . addr );
    return pb -> rc != 0;
}

LIB_EXPORT rc_t CC KDlsetList ( const KDlset *self, KNamelist **listp )
{
    list_dylib_param pb;

    assert ( listp != NULL );

    if ( self == NULL )
        pb . rc = RC ( rcFS, rcDylib, rcListing, rcSelf, rcNull );
    else
    {
        pb . rc = VNamelistMake ( & pb . list, VectorLength ( & self -> name ) );
        if ( pb . rc == 0 )
        {
            bool fail = VectorDoUntil ( & self -> name, false, list_dylib, & pb );
            if ( ! fail )
                pb . rc = VNamelistToNamelist ( pb . list, listp );

            VNamelistRelease ( pb . list );
        }
    }

    return pb . rc;
}


/* AsObj
 * AsFunc
 *  retrieve symbol address as pointer to object
 */
LIB_EXPORT void * CC KSymAddrAsObj ( const KSymAddr *self )
{
    if ( self != NULL )
        return self -> addr;
    return NULL;
}

LIB_EXPORT void CC KSymAddrAsFunc ( const KSymAddr *self, fptr_t *fp )
{
    if ( self != NULL && fp != NULL )
        * fp = ( fptr_t ) self -> addr;
}


