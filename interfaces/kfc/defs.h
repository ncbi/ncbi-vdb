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

#ifndef _h_kfc_defs_
#define _h_kfc_defs_

#ifndef _h_kfc_callconv_
#include <kfc/callconv.h>
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/*--------------------------------------------------------------------------
 * ctx_t
 *  a thread context block
 */
typedef struct KCtx const * ctx_t;


/*--------------------------------------------------------------------------
 * caps_t
 *  a map of capability bits
 */
typedef uint32_t caps_t;


/*--------------------------------------------------------------------------
 * xobj_t
 *  describes the type of object having problems
 */
typedef struct { const char *name; } const xobj_t [ 1 ];


/*--------------------------------------------------------------------------
 * xstate_t
 *  describes the state of an object having problems
 */
typedef struct { const char *name; } const xstate_t [ 1 ];


/*--------------------------------------------------------------------------
 * xc_t
 *  describes a class of error
 */
typedef struct { const char *name; } const xc_t [ 1 ];


/*--------------------------------------------------------------------------
 * under normal usage, the declarations below will create simple externs.
 * however, they may be redefined to create static objects
 */
#ifndef XC_DEFINE

#define XOBJ( name, desc, rc )                  \
    extern xobj_t name
#define XOBJ_EXT( name, supr, desc, rc )        \
    extern xobj_t name

#define XSTATE( name, desc, rc )                \
    extern xstate_t name
#define XSTATE_EXT( name, supr, desc, rc )      \
    extern xstate_t name

#define XC( name, obj, state )                  \
    extern xc_t name
#define XC_EXT( name, supr )                    \
    extern xc_t name

#endif /* XC_DEFINE */


/*--------------------------------------------------------------------------
 * rc_t - VDB.2 LEGACY
 *  upon success, all functions will return code 0
 *  other codes indicate failure or additional status information
 */
typedef uint32_t rc_t;


/*--------------------------------------------------------------------------
 * ver_t - VDB.2 LEGACY
 *  32 bit 3 part type
 */
typedef uint32_t ver_t;

/* GetMajor
 *  return major component
 */
#define VersionGetMajor( self ) \
    ( ( self ) >> 24 )

/* GetMinor
 *  return minor component
 */
#define VersionGetMinor( self ) \
    ( ( ( self ) >> 16 ) & 0xFF )

/* GetRelease
 *  return release component
 */
#define VersionGetRelease( self ) \
    ( ( self ) & 0xFFFF )


/*--------------------------------------------------------------------------
 * stringize
 *  it is useful to be able to convert PP defines on the command line
 */
#define stringize( tok ) tok_to_string ( tok )
#define tok_to_string( tok ) # tok


/*--------------------------------------------------------------------------
 * NAME_VERS
 *  synthesize versioned type and message names
 */
#define NAME_VERS( name, maj_vers ) \
    MAKE_NAME_VERS1 ( name, maj_vers )
#define MAKE_NAME_VERS1( name, maj_vers ) \
    MAKE_NAME_VERS2 ( name, maj_vers )
#define MAKE_NAME_VERS2( name, maj_vers ) \
    name ## _v ## maj_vers

/*--------------------------------------------------------------------------
 * __mod__, __file__ and __fext__
 *  these guys are slightly different from __FILE__
 *  and they complement __func__
 */
#if ! defined __mod__ && defined __mod_name__
#define __mod__ stringize ( __mod_name__ )
#endif

#if ! defined __file__ && defined __file_name__
#define __file__ stringize ( __file_name__ )
#endif

#if ! defined __fext__ && defined __file_ext__
#define __fext__ stringize ( __file_ext__ )
#endif

/*--------------------------------------------------------------------------
 * PKGNAMESTR
 */
#if ! defined PKGNAMESTR && defined PKGNAME
#define PKGNAMESTR stringize ( PKGNAME )
#endif

/*--------------------------------------------------------------------------
 * memcpy()
 *  as of 11/29/2016 we are prohibiting the use of memcpy
 *  NB - we depend upon including <string.h> BEFORE redefining
 */
#undef memcpy
#if _DEBUGGING && LINUX
#define memcpy "DON'T USE MEMCPY! USE MEMMOVE INSTEAD!!"
#else
#define memcpy memmove
#endif

#ifdef __cplusplus
}
#endif

#endif /*  _h_kfc_defs_ */

