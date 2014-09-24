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

#ifndef _h_kfs_dyload_
#define _h_kfs_dyload_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDirectory;


/*--------------------------------------------------------------------------
 * KDyld
 *  dynamic library loader
 *
 *  maintains cache of libraries it has opened while they remain open
 *  such that subsequent requests for an open library will return a
 *  new reference to the existing library.
 */
typedef struct KDyld KDyld;


/* Make
 *  create a dynamic loader object
 *
 *  "dl" [ OUT ] - return parameter for loader
 */
KFS_EXTERN rc_t CC KDyldMake ( KDyld **dl );


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KDyldAddRef ( const KDyld *self );
KFS_EXTERN rc_t CC KDyldRelease ( const KDyld *self );


/* AddSearchPath
 *  add a search path to loader for locating library files
 */
KFS_EXTERN rc_t CC KDyldAddSearchPath ( KDyld *self, const char *path, ... );
KFS_EXTERN rc_t CC KDyldVAddSearchPath ( KDyld *self, const char *path, va_list args );


/* HomeDirectory
 *  returns a KDirectory where the binary for a given function is located
 *
 *  "dir" [ OUT ] - return parameter for home directory ( read-only ), if found
 *
 *  "func" [ IN ] - function pointer within binary to be located
 */
KFS_EXTERN rc_t CC KDyldHomeDirectory ( const KDyld *self,
    struct KDirectory const **dir, fptr_t func );


/*--------------------------------------------------------------------------
 * KDylib
 *  dynamic library
 */
typedef struct KDylib KDylib;


/* LoadLib
 *  load a dynamic library
 *
 *  "lib" [ OUT ] - return parameter for loaded library
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target library
 */
KFS_EXTERN rc_t CC KDyldLoadLib ( KDyld *self,
    KDylib **lib, const char *path, ... );
KFS_EXTERN rc_t CC KDyldVLoadLib ( KDyld *self,
    KDylib **lib, const char *path, va_list args );


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KDylibAddRef ( const KDylib *self );
KFS_EXTERN rc_t CC KDylibRelease ( const KDylib *self );


/* FullPath
 *  return full path to library
 */
KFS_EXTERN rc_t CC KDylibFullPath ( const KDylib *self, char *path, size_t psize );


/*--------------------------------------------------------------------------
 * KDlset
 *  set of dynamic libraries
 *  contained libraries remain resident until set is released
 */
typedef struct KDlset KDlset;


/* MakeSet
 *  load a dynamic library
 *
 *  "set" [ OUT ] - return parameter for lib set
 */
KFS_EXTERN rc_t CC KDyldMakeSet ( const KDyld *self, KDlset **set );


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KDlsetAddRef ( const KDlset *self );
KFS_EXTERN rc_t CC KDlsetRelease ( const KDlset *self );


/* AddLib
 *  adds a dynamic library to set
 *  returns "rcExists" error if already present
 *
 *  "lib" [ IN ] - library returned from KDyldLoadLib
 */
KFS_EXTERN rc_t CC KDlsetAddLib ( KDlset *self, KDylib *lib );


/* AddAll
 *  adds all dynamic libraries found in dl search path
 */
KFS_EXTERN rc_t CC KDlsetAddAll ( KDlset *self );


/*--------------------------------------------------------------------------
 * KSymAddr
 *  symbol address within a dynamic library
 */
typedef struct KSymAddr KSymAddr;


/* Symbol
 *  find a symbol within dynamic library
 *
 *  "sym" [ OUT ] - return parameter for exported symbol address
 *
 *  "name" [ IN ] - NUL terminated symbol name in
 *  library-native character set
 */
KFS_EXTERN rc_t CC KDylibSymbol ( const KDylib *self, KSymAddr **sym, const char *name );
KFS_EXTERN rc_t CC KDlsetSymbol ( const KDlset *self, KSymAddr **sym, const char *name );


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
KFS_EXTERN rc_t CC KDlsetFirstSymbol ( const KDlset *self, KSymAddr **sym, const char *name,
    bool ( CC * test ) ( const KSymAddr *sym, void *data ), void *data );
KFS_EXTERN rc_t CC KDlsetLastSymbol ( const KDlset *self, KSymAddr **sym, const char *name,
    bool ( CC * test ) ( const KSymAddr *sym, void *data ), void *data );


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KSymAddrAddRef ( const KSymAddr *self );
KFS_EXTERN rc_t CC KSymAddrRelease ( const KSymAddr *self );


/* AsObj
 * AsFunc
 *  retrieve symbol address as pointer to object
 */
KFS_EXTERN void *CC KSymAddrAsObj ( const KSymAddr *self );
KFS_EXTERN void CC KSymAddrAsFunc ( const KSymAddr *self, fptr_t *fp );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_dyload_ */
