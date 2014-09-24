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

#ifndef _h_simple_SRAString_
#define _h_simple_SRAString_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * SRAString
 */
typedef struct SRAString SRAString;
struct SRAString
{
    void *allocation;
    const char *addr;
    size_t size;
};


/* Init
 *  initialize a string from a pointer to text and size
 *  plus an optional pointer to allocation
 */
SIMPLE_EXTERN SRAString SRAStringMake ( void *allocation, const char *addr, size_t size );


/* Whack
 *  destroy string
 */
SIMPLE_EXTERN void SRAStringWhack ( SRAString *self );

#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRAString_ */
