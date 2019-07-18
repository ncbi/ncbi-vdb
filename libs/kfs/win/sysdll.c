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

#ifndef _WIN32_WINNT /* This file requires OS newer than Windows 2000 */
# define _WIN32_WINNT 0x0501
#endif

#include <kfs/extern.h>

#define TRACK_REFERENCES 0

#include "va_copy.h"
#include "sysdir-priv.h"

#include <kfs/dyload.h>
#include <kfs/directory.h>
#include <kfs/kfs-priv.h>
#include <klib/refcount.h>
#include <klib/vector.h>
#include <klib/text.h>
#include <klib/out.h>
#include <klib/log.h>
#include <klib/status.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <WINDOWS.H>

#define ALWAYS_ADD_EXE 1



/*--------------------------------------------------------------------------
 * WString
 */
typedef struct WString WString;
struct WString
{
    const wchar_t *addr;
    size_t size;
    uint32_t len;
};

#define WStringInit( s, val, sz, length ) \
    StringInit ( s, val, sz, length )

#define WStringInitCString( s, cstr ) \
    ( void ) ( ( s ) -> len = ( uint32_t )( wcslen ( ( s ) -> addr = ( cstr ) ) ), \
               ( s ) -> size = ( ( size_t ) ( s ) -> len * sizeof ( wchar_t ) ) )

#define CONST_WSTRING( s, val ) \
    WStringInitCString ( s, L ## val )

static
int64_t WStringCaseCompare ( const WString *a, const WString *b )
{
    int64_t diff;

    uint32_t min_len = a -> len;
    if ( a -> len > b -> len )
        min_len = b -> len;

    diff = _wcsnicmp ( a -> addr, b -> addr, min_len );
    if ( diff == 0 )
        diff = ( int64_t ) a -> len - ( int64_t ) b -> len;

    return diff;
}


/*--------------------------------------------------------------------------
 * KDirectory
 */
static
void CC KDirRefRelease ( void *item, void *ignore )
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
    void ( CC * f ) ( const KDirectory *dir, void *data ), void *data )
{
    VectorForEach ( & self -> search, false,
        ( void ( CC * ) ( void*, void* ) ) f, data );
}


/* HomeDirectory
 *  returns a KDirectory where the binary for a given function is located
 *
 *  "dir" [ OUT ] - return paraeter for home directory, if found
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
            HMODULE h;
            /* casting a function pointer to a string pointer because the Windows API
             * allows a name or an address within to be passed in but doesn't provide
             * a type safe way to do this */
            BOOL success = GetModuleHandleEx ( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                               | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                               ( const TCHAR * )func, & h );
            if ( success )
            {
                wchar_t fname [ MAX_PATH ];
                DWORD name_len = GetModuleFileNameW ( h, fname, ( DWORD ) sizeof fname / sizeof fname [ 0 ] );
                if ( name_len >= sizeof fname / sizeof fname [ 0 ] )
                    rc = RC ( rcFS, rcDylib, rcSearching, rcPath, rcExcessive );
                else
                {
                    /* trim off module name */
                    while ( name_len > 0 )
                    {
                        if ( fname [ -- name_len ] == '\\' )
                        {
                            fname [ name_len ] = 0;
                            break;
                        }
                    }

                    /* now use directory path */
                    rc = KDirectoryMakeFromRealPath ( ( KDirectory** ) dir, fname, false, false );
                }
            }
        }
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KDylib
 *  Windows dynamic library
 */
struct KDylib
{
    HMODULE handle;
    WString path;
    KRefcount refcount;
};


/* Whack
 */
static
rc_t KDylibWhack ( KDylib *self )
{
    KRefcountWhack ( & self -> refcount, "KDylib" );

    /* try to close library */
	if ( !FreeLibrary( self -> handle ) )
    {
        /* report error */
/*        const char *msg = dlerror (); */
        rc_t rc = RC ( rcFS, rcDylib, rcClosing, rcNoObj, rcUnknown );
/*        LOGERR ( klogInt, rc, msg );
        ( void ) msg; */
        return rc;
    }

    free ( self );
    return 0;
}


/* Make
 */
static
rc_t KDylibMake ( KDylib **libp, const WString *path )
{
    wchar_t *cpy;
    KDylib *lib = malloc ( sizeof * lib + path -> size + 4 );
    if ( lib == NULL )
        return RC ( rcFS, rcDylib, rcConstructing, rcMemory, rcExhausted );

    cpy = ( wchar_t* ) ( lib + 1 );
    lib -> handle = NULL;

    memmove ( cpy, path -> addr, path -> size );
    cpy [ path -> len ] = 0;

    WStringInit ( & lib -> path, cpy, path -> size, path -> len );

    KRefcountInit ( & lib -> refcount, 1, "KDylib", "make", "WinDLL" );

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

    /* set the current logging level */
    rc_t ( CC * set_level ) ( KLogLevel lvl ) = ( void* ) GetProcAddress ( self -> handle, "KLogLevelSet" );
    if ( set_level != NULL )
    {
        KLogLevel lvl = KLogLevelGet ();
        ( * set_level ) ( lvl );
    }

    /* determine current library logging */
    set_writer = ( void* ) GetProcAddress ( self -> handle, "KOutHandlerSet" );
    if ( set_writer != NULL )
    {
        const KWrtHandler* handler = KOutHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }

    set_formatter = ( void* ) GetProcAddress ( self -> handle, "KLogLibFmtHandlerSet" );
    if ( set_formatter != NULL )
    {
        KLogFmtFlags flags = KLogLibFmtFlagsGet ();
        const KFmtHandler* fmt_handler = KLogFmtHandlerGet ();
        ( * set_formatter ) ( fmt_handler -> formatter, flags, fmt_handler -> data );
    }
    set_writer = ( void* ) GetProcAddress ( self -> handle, "KLogLibHandlerSet" );
    if ( set_writer != NULL )
    {
        const KWrtHandler* handler = KLogLibHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }

    set_formatter = ( void* ) GetProcAddress ( self -> handle, "KStsLibFmtHandlerSet" );
    if ( set_formatter != NULL )
    {
        KStsFmtFlags flags = KStsLibFmtFlagsGet ();
        const KFmtHandler* fmt_handler = KStsFmtHandlerGet ();
        ( * set_formatter ) ( fmt_handler -> formatter, flags, fmt_handler -> data );
    }
    set_writer = ( void* ) GetProcAddress ( self -> handle, "KStsLibHandlerSet" );
    if ( set_writer != NULL )
    {
        const KWrtHandler* handler = KStsLibHandlerGet ();
        ( * set_writer ) ( handler -> writer, handler -> data );
    }
#if _DEBUGGING
    set_writer = ( void* ) GetProcAddress ( self -> handle, "KDbgHandlerSet" );
    if ( set_writer != NULL )
    {
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
rc_t KDyldLoad ( KDyld *self, KDylib *lib, const wchar_t *path )
{
#if USE_DYLOAD

    DWORD err;
#if WE_WERE_BUILDING_FOR_WINDOWS_7_ALONE
    UINT errMode = GetErrorMode();
#endif

	if ( path == NULL )
	{
		if ( GetModuleHandleExW( 0, NULL, &( lib -> handle ) ) )
			return 0;

        return RC ( rcFS, rcDylib, rcLoading, rcNoObj, rcUnknown );
	}

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX); /* suppress the message box in case of an error */
    lib -> handle = LoadLibraryW ( path );
#if WE_WERE_BUILDING_FOR_WINDOWS_7_ALONE
    SetErrorMode(errMode);
#endif
    if ( lib -> handle != NULL )
        return KDylibSetLogging ( lib );

    err = GetLastError ();
    switch ( err )
    {
    case ERROR_MOD_NOT_FOUND :
        return RC ( rcFS, rcDylib, rcLoading, rcPath, rcNotFound );
    case ERROR_BAD_EXE_FORMAT :
        return RC ( rcFS, rcDylib, rcLoading, rcFormat, rcInvalid );
    }

    return RC ( rcFS, rcDylib, rcLoading, rcNoObj, rcUnknown );

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
        wchar_t real [ MAX_PATH ];

		rc = KSysDirOSPath ( sdir, real, sizeof real, path, args );
		if ( rc == 0 )
		{
			WString pstr;
			WStringInitCString ( & pstr, real );

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
            WString pstr;
            CONST_WSTRING ( & pstr, "" );

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
                    WString pstr;
                    wchar_t wname [ 4096 ];
                    size_t wsize = string_cvt_wchar_copy ( wname, sizeof wname, name, len );
                    WStringInit ( & pstr, wname, wsize * sizeof wname [ 0 ], string_len ( name, len ) );

                    rc = KDylibMake ( lib, & pstr );
                    if ( rc == 0 )
                    {
                        rc = KDyldLoad ( self, * lib, wname );
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
void CC KDylibVectRelease ( void *item, void *ignore )
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
int64_t CC KDylibSort ( const void *item, const void *n )
{
    const KDylib *a = item;
    const KDylib *b = n;
    return WStringCaseCompare ( & a -> path, & b -> path );
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
            size_t i, bytes = wchar_cvt_string_copy ( path, psize,
                self -> path . addr, self -> path . size );
            if ( bytes < psize )
            {
                if ( isalpha ( path [ 0 ] ) && path [ 1 ] == ':' )
                {
                    path [ 1 ] = path [ 0 ];
                    path [ 0 ] = '/';
                }
                for ( i = 0; i < bytes; ++ i )
                {
                    if ( path [ i ] == '\\' )
                        path [ i ] = '/';
                }

                return 0;
            }

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
                    const char* libname = "vdb_jni.dll";
                    rc = KDyldLoadLib ( ( KDyld* ) self, & jni, libname );
                    if ( rc == 0 )
                    {
                        rc = KDlsetAddLib ( set, jni );
                        KDylibRelease ( jni );
                    }
                    /*if (rc == 0)*/ /* if JNI code is not there, C tools should not suffer */
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

                KDlsetRelease ( set );
#endif
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
rc_t CC KDlsetTryLib ( const KDirectory *dir,
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
            size_t len = strlen ( name );
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
void CC KDlsetVisitDir ( const KDirectory *dir, void *data )
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
    FARPROC addr = GetProcAddress ( lib -> handle, name );
    if ( addr != NULL )
    {
        KSymAddr *sym = malloc ( sizeof * sym );
        if ( sym == NULL )
            return RC ( rcFS, rcDylib, rcConstructing, rcMemory, rcExhausted );

        sym -> lib = KDylibAttach ( lib );
        sym -> addr = (void *)addr;
        KRefcountInit ( & sym -> refcount, 1, "KSymAddr", "make", name );
        * symp = sym;
        return 0;
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
bool CC KDlsetTrySymbol ( void *item, void *data )
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
    size_t bytes;
    char utf8 [ 4096 ], *p;

    list_dylib_param *pb = data;
    const KDylib *lib = ( const void* ) item;

    /* "lib" was created with KDylibMake
       which creates a NUL terminated path.
       of course, this could seg-fault if bad... */
    assert ( lib -> path . addr [ lib -> path . len ] == 0 );

    /* convert wide-character to UTF-8 */
    bytes = wchar_cvt_string_copy ( utf8, sizeof utf8, lib -> path . addr, lib -> path . size );
    if ( bytes < sizeof utf8 )
    {
        pb -> rc = VNamelistAppend ( pb -> list, utf8 );
        return pb -> rc != 0;
    }

    p = malloc ( 16 * 1024 );
    if ( p == NULL )
    {
        pb -> rc = RC ( rcFS, rcDylib, rcListing, rcMemory, rcExhausted );
        return true;
    }

    bytes = wchar_cvt_string_copy ( p, 16 * 1024, lib -> path . addr, lib -> path . size );
    if ( bytes >= 16 * 1024 )
        pb -> rc = RC ( rcFS, rcDylib, rcListing, rcPath, rcTooLong );
    else
        pb -> rc = VNamelistAppend ( pb -> list, p );

    free ( p );
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


