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

#ifndef _h_klib_sort_
#define _h_klib_sort_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_ksort_macro_
#include <klib/ksort-macro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * ksort
 *  qsort with a function data pointer
 */
KLIB_EXTERN void CC ksort ( void *pbase, size_t total_elems, size_t size,
    int ( CC * cmp ) ( const void*, const void*, void *data ), void *data );


/* various custom ksort operations
 *  structures will generally want their own functions
 *  these fundamental types can be standardized
 */
KLIB_EXTERN void CC ksort_int32_t ( int32_t *pbase, size_t total_elems );
KLIB_EXTERN void CC ksort_uint32_t ( uint32_t *pbase, size_t total_elems );
KLIB_EXTERN void CC ksort_int64_t ( int64_t *pbase, size_t total_elems );
KLIB_EXTERN void CC ksort_uint64_t ( uint64_t *pbase, size_t total_elems );


/* KSORT
 *  macro ( see <klib/ksort-macro.h> )
 *  allows creation of a custom qsort with inlined compare and swap
 *  MUCH faster than normal qsort or ksort.
 *
 *  basically you need to define a macro CMP() and another SWAP(),
 *  and the KSORT macro will fill in the rest.
 *
 *  CMP() needs to evaluate to a signed 32-bit integer.
 *  THIS WILL CHANGE TO BECOME LESS_THAN( a, b ).
 *
 *  SWAP() will generally be a custom operation, but can be
 *  defined in terms of a default operation similar to qsort.
 *
 *  an example usage follows:
 */
#if 0
static
void ksort_int64_t ( int64_t *base, size_t count )
{
    /* swap is performed on full element, not byte-for-byte */
#define SWAP( a, b, off, size )                             \
    do                                                      \
    {                                                       \
        int64_t tmp = * ( const int64_t* ) ( a );           \
        * ( int64_t* ) ( a ) = * ( const int64_t* ) ( b );  \
        * ( int64_t* ) ( b ) = tmp;                         \
    }                                                       \
    while ( 0 )

    /* 64-bit comparison producing a signed 32-bit result */
#define CMP( a, b )                                                     \
    ( ( * ( const int64_t* ) ( a ) < * ( const int64_t* ) ( b ) ) ? -1 : \
      ( * ( const int64_t* ) ( a ) > * ( const int64_t* ) ( b ) ) )
}

    /* let the macro fill out the remainder */
    KSORT ( base, count, sizeof * base, 0, sizeof * base );

    /* free up macros for future use */
#undef SWAP
#undef CMP

#endif


/*--------------------------------------------------------------------------
 * kbsearch
 *  bsearch with a function data pointer
 */
KLIB_EXTERN void* CC kbsearch ( const void *key, const void *base, size_t nmemb, size_t size,
    int ( CC * cmp ) ( const void*, const void*, void *data ), void *data );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_sort_ */
