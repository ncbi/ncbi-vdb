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

#include <klib/extern.h>
#include <klib/sort.h>


/*--------------------------------------------------------------------------
 * ksort
 *  qsort with a function data pointer
 */

#define TSWAP( T, a, b )                                      \
    do                                                        \
    {                                                         \
        T tmp = * ( const T* ) ( a );                         \
        * ( T* ) ( a ) = * ( const T* ) ( b );                \
        * ( T* ) ( b ) = tmp;                                 \
    }                                                         \
    while ( 0 )

#define TCMP( T, a, b )                                       \
    ( ( * ( const T* ) ( a ) < * ( const T* ) ( b ) ) ? -1 :  \
      ( * ( const T* ) ( a ) > * ( const T* ) ( b ) ) )


/* various custom ksort operations
 *  structures will generally want their own functions
 *  these fundamental types can be standardized
 */
LIB_EXPORT void CC ksort_int32_t ( int32_t *pbase, size_t total_elems )
{
#define SWAP( a, b, off, size )  TSWAP ( int32_t, a, b )
#define CMP( a, b )                                             \
    ( * ( const int32_t* ) ( a ) - * ( const int32_t* ) ( b ) )

    KSORT ( pbase, total_elems, sizeof * pbase, 0, sizeof * pbase );

#undef SWAP
#undef CMP
}

LIB_EXPORT void CC ksort_uint32_t ( uint32_t *pbase, size_t total_elems )
{
#define SWAP( a, b, off, size )  TSWAP ( uint32_t, a, b )
#define CMP( a, b ) TCMP ( uint32_t, a, b )

    KSORT ( pbase, total_elems, sizeof * pbase, 0, sizeof * pbase );

#undef SWAP
#undef CMP
}

LIB_EXPORT void CC ksort_int64_t ( int64_t *pbase, size_t total_elems )
{
#define SWAP( a, b, off, size )  TSWAP ( int64_t, a, b )
#define CMP( a, b ) TCMP ( int64_t, a, b )

    KSORT ( pbase, total_elems, sizeof * pbase, 0, sizeof * pbase );

#undef SWAP
#undef CMP
}

LIB_EXPORT void CC ksort_uint64_t ( uint64_t *pbase, size_t total_elems )
{
#define SWAP( a, b, off, size )  TSWAP ( uint64_t, a, b )
#define CMP( a, b ) TCMP ( uint64_t, a, b )

    KSORT ( pbase, total_elems, sizeof * pbase, 0, sizeof * pbase );

#undef SWAP
#undef CMP
}
