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

#ifndef _h_klib_path_
#define _h_klib_path_

#ifndef _h_klib_extern_
#include <klib/extern.h>
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
struct String;


/*--------------------------------------------------------------------------
 * KPath
 *  represents an unbound object query key
 *  may be created from a simple file-system path,
 *  a more formal URN or URL,
 *  or other modes of creation
 *
 *  a path will have these parts:
 *    scheme       : a scheme for retrieval
 *    auth         : login name for authentication
 *    host         : authoritative source
 *    port         : port for connecting with host
 *    path         : host-relative path
 *    query        : parameters for interpretation
 *    fragment     : internal component of object
 *    proj         : project id
 *    name         : alternate or primary name
 *
 *  file-system paths with no modifying parameters
 *  will be given standard "file" scheme. paths having
 *  parameters will be given the scheme "ncbi-file".
 *
 *  standard networking schemes ( "http", "ftp", etc. )
 *  are supported.
 *
 *  NCBI accessions are given the scheme "ncbi-acc".
 *
 *  NCBI remote object id paths receive scheme "ncbi-obj".
 */
typedef struct KPath KPath;


/* MakePath
 *  make a path object from a string conforming to
 *  either a standard POSIX path or a URI
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "path_str" [ IN ] - a UTF-8 NUL-terminated string
 *  representing a POSIX path or URI, or
 *  a string_printf compatible format string
 *
 *  "path_fmt" [ IN ] and "args" [ IN ] - a UTF-8 NUL-terminated fmt string
 *  compatible with string_vprintf, plus argument list
 *
 *  Examples:
 *      "ncbi-file:/home/my-name/data-files"
 *      "ncbi-file://win-server/archive/secure/read12345?encrypted"
 *      "ncbi-file:///c/scanned-data/0001/file.sra?enc&pwfile=/c/Users/JamesMcCoy/ncbi.pwd"

    KLIB_EXTERN rc_t CC MakePath_v1 ( KPath ** new_path, const char *path_str, ... );
    KLIB_EXTERN rc_t CC VMakePath_v1 ( KPath ** new_path, const char *path_fmt, va_list args );
 */
KLIB_EXTERN KPath * CC MakePath ( ctx_t ctx, const char *path_str, ... );
KLIB_EXTERN KPath * CC VMakePath ( ctx_t ctx, const char *path_fmt, va_list args );

/* MakeSysPath
 *  make a path object from an OS native filesystem path string
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "sys_path" [ IN ] - a UTF-8 NUL-terminated string
 *  representing a native filesystem path
 *
 *  "wide_sys_path" [ IN ] - a wide NUL-terminated string
 *  representing a native filesystem path, where
 *  wchar_t is either USC-2 or UTF-32 depending upon libraries


KLIB_EXTERN KPath * CC MakeSysPath ( ctx_t ctx, const char *sys_path );
KLIB_EXTERN KPath * CC VMakeSysPath ( ctx_t ctx, const wchar_t * wide_sys_path );
 */

/* MakeAccPath - TEMPORARY
 *  takes a textual accession representation
 *  creates a VPath representing an accession
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "acc" [ IN ] - a NUL-terminated ASCII fmt string

    LIB_EXPORT rc_t CC MakeAccPath_v1 ( KPath ** new_path, const char * acc, ... );
    LIB_EXPORT rc_t CC VMakeAccPath_v1 ( KPath ** new_path, const char * fmt, va_list args );
 */

KLIB_EXTERN KPath * CC MakeAccPath ( ctx_t ctx, const char * acc, ... );
KLIB_EXTERN KPath * CC VMakeAccPath ( ctx_t ctx, const char * fmt, va_list args );

/* MakeOidPath - TEMPORARY
 *  takes an integer oid
 *  creates a VPath representing an obj-id
 *
 *  "new_path" [ OUT ] - return parameter for new path object
 *
 *  "oid" [ IN ] - a non-zero object id

    LIB_EXPORT rc_t CC MakeOidPath_v1 ( KPath ** new_path, uint32_t oid )
 */

KLIB_EXTERN KPath * CC MakeOidPath ( ctx_t ctx, uint32_t oid );


/* AddRef
 * Release
 *  ignores NULL references
 */
KLIB_EXTERN KPath * CC KPathDuplicate ( const KPath *self, ctx_t ctx );
KLIB_EXTERN rc_t CC KPathRelease ( const KPath *self );


/* IsFSCompatible
 *  asks if the path can be used with the OS' filesystems
 */
KLIB_EXTERN bool CC KPathIsFSCompatible ( const KPath *self, ctx_t ctx );


/* FromUri
 *  asks if the path was created from a formal URI
 */
KLIB_EXTERN bool CC KPathFromUri ( const KPath *self, ctx_t ctx );

/* Read*
 *  read the various parts
 *  copies out data into user-supplied buffer
 *
 *  "buffer" [ OUT ] and "buffer_size" [ IN ] - output buffer
 *  for data read. if sufficient space is available, the copy
 *  will be NUL-terminated.
 *
 *  "num_read" [ OUT, NULL OKAY ] - optional return parameter
 *  for the number of valid bytes in "buffer" after a successful
 *  read. on failure due to insufficient buffer, contains the
 *  number of bytes required for transfer.
 */
KLIB_EXTERN size_t CC KPathReadUri ( const KPath *self, ctx_t ctx,
                                     char * buffer, size_t buffer_size );
    KLIB_EXTERN size_t CC KPathReadScheme ( const KPath * self, ctx_t ctx,
                                        char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadAuth ( const KPath *self, ctx_t ctx,
                                      char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadHost ( const KPath *self, ctx_t ctx,
                                      char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadPortName ( const KPath *self, ctx_t ctx,
                                          char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadPath ( const KPath *self, ctx_t ctx,
                                      char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadQuery ( const KPath *self, ctx_t ctx,
                                       char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadParam ( const KPath *self, ctx_t ctx, 
                                       const char * param, char * buffer, size_t buffer_size );
KLIB_EXTERN size_t CC KPathReadFragment ( const KPath *self, ctx_t ctx,
                                          char * buffer, size_t buffer_size );


/* MakeUri
 *  convert a VPath into a URI
 */
KLIB_EXTERN struct String const * CC KPathMakeUri ( const KPath *self, ctx_t ctx );



/* MakeString
 *  convert a KPath into a String
 *  respects original source of path,
 *  i.e. does not add scheme unnecessarily
 */
KLIB_EXTERN struct String const * CC KPathMakeString ( const KPath *self, ctx_t ctx );


/* Get*
 *  retrieves internal parts
 *  returns pointers to internal String data
 *  Strings remain valid while "self" is valid
 */
KLIB_EXTERN struct String *  CC KPathGetScheme ( const KPath *self, ctx_t ctx, struct String * str );
KLIB_EXTERN struct String *  CC KPathGetAuth ( const KPath *self, ctx_t ctx, struct String * str );
KLIB_EXTERN struct String *  CC KPathGetHost ( const KPath *self, ctx_t ctx, struct String * str );
KLIB_EXTERN struct String *  CC KPathGetPortName ( const KPath *self, ctx_t ctx, struct String * str );
KLIB_EXTERN uint16_t CC KPathGetPortNum ( const KPath *self, ctx_t ctx );
KLIB_EXTERN struct String *  CC KPathGetPath ( const KPath *self, ctx_t ctx, struct String * str );
KLIB_EXTERN struct String *  CC KPathGetQuery ( const KPath *self, ctx_t ctx, struct String * str );
KLIB_EXTERN struct String *  CC KPathGetParam ( const KPath *self, ctx_t ctx, const char * param, struct String * str );
KLIB_EXTERN struct String *  CC KPathGetFragment ( const KPath *self, ctx_t ctx, struct String * str );
/* TEMPORARY */
KLIB_EXTERN uint32_t CC KPathGetOid ( const KPath *self, ctx_t ctx );

/* legacy support */
/*
#define KPathMake LegacyKPathMake
KLIB_EXTERN rc_t KPathMake ( KPath ** new_path, const char * posix_path );
#define KPathMakeFmt LegacyKPathMakeFmt
rc_t KPathMakeFmt ( KPath ** new_path, const char * fmt, ... );
#define KPathMakeVFmt LegacyKPathMakeVFmt
rc_t KPathMakeVFmt ( KPath ** new_path, const char * fmt, va_list args );
#define KPathMakeSysPath LegacyKPathMakeSysPath
KLIB_EXTERN rc_t KPathMakeSysPath ( KPath ** new_path, const char * sys_path );
*/

#ifdef __cplusplus
}
#endif

#endif /* _h_vfs_path_ */
