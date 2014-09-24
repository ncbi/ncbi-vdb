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

#ifndef _h_sysalloc_
#define _h_sysalloc_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#include <malloc.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* malloc, calloc, realloc
 */
KLIB_EXTERN void * CC klib_malloc ( size_t bytes );
KLIB_EXTERN void * CC klib_calloc ( size_t count, size_t size );
KLIB_EXTERN void * CC klib_realloc ( void *obj, size_t bytes );

/* free
 */
KLIB_EXTERN void CC klib_free ( void *obj );

#ifndef _c_sysalloc_

#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc klib_malloc
#define calloc klib_calloc
#define realloc klib_realloc
#define free klib_free

#endif /* _c_sysalloc_ */

#ifdef __cplusplus
}
#endif

#endif /* _h_sysalloc_ */
