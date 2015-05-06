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

#ifndef _h_klib_defs_
#define _h_klib_defs_

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * bitsz_t
 *  where size_t always denotes a quantity of bytes,
 *  bitsz_t denotes a quantity of bits.
 */
typedef uint64_t bitsz_t;


/*--------------------------------------------------------------------------
 * fptr_t
 *  "generic" function pointer type
 *  has very little real use other than to calm down compilers
 */
typedef int ( CC * fptr_t ) ( void );


/*--------------------------------------------------------------------------
 * remove_t
 *  usually when message signatures change, the compiler can
 *  and will pick up the changes and trigger an error. sometimes
 *  it will just issue a warning, and other times will not pick
 *  up on the significance of a change.
 *
 *  to ensure that a change of signature gets caught everywhere
 *  by the compiler, we can introduce an extra parameter that
 *  causes us to visit all dependent code.
 */
typedef struct remove_t remove_t;


/*--------------------------------------------------------------------------
 * KTime_t
 *  64 bit time_t
 *  operations are declared in <klib/time.h>
 */
typedef int64_t KTime_t;
typedef int64_t KTimeMs_t;

/*--------------------------------------------------------------------------
 * KCreateMode
 *  values are defined in <kfs/defs.h>
 */
typedef uint32_t KCreateMode;


/*--------------------------------------------------------------------------
 * kfs defines
 */
#ifndef KFILE_VERS
#define KFILE_VERS 1
#endif

#ifndef KFile
#if KFILE_VERS == 1
#define KFile KFile_v1
#elif KFILE_VERS == 2
#define KFile KFile_v2
#endif
#endif

#ifndef KDIRECTORY_VERS
#define KDIRECTORY_VERS 1
#endif

#ifndef KDirectory
#if KDIRECTORY_VERS == 1
#define KDirectory KDirectory_v1
#elif KDIRECTORY_VERS == 2
#define KDirectory KDirectory_v2
#endif
#endif


#if 1

/*--------------------------------------------------------------------------
 * LPFX
 * SHLX
 * MODX
 *  take their input from make
 */
#ifndef LIBPREFIX
 #define LPFX ""
#else
 #define LPFXSTR2( str ) # str
 #define LPFXSTR( str ) LPFXSTR2 ( str )
 #define LPFX LPFXSTR ( LIBPREFIX )
#endif
#ifndef SHLIBEXT
 #define SHLX ""
#else
 #define SHLXSTR2( str ) "." # str
 #define SHLXSTR( str ) SHLXSTR2 ( str )
 #define SHLX SHLXSTR ( SHLIBEXT )
#endif
#ifndef MODEXT
 #define MODX SHLX
#else
 #define MODXSTR2( str ) "." # str
 #define MODXSTR( str ) MODXSTR2 ( str )
 #define MODX MODXSTR ( MODIBEXT )
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_defs_ */
