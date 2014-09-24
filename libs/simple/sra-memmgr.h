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

#ifndef _h_simple_SRAMemMgr_
#define _h_simple_SRAMemMgr_

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
 * forwards
 */
struct SRASourceLoc;
struct SRAException;


/*--------------------------------------------------------------------------
 * SRAMemMgr
 *  should be an actual object
 *  the functions are left global for now
 */
typedef struct SRAMemMgr SRAMemMgr;


/* g_mmgr
 *  a define for global SRAMemMgr
 */
#define g_mmgr ( ( SRAMemMgr* ) ~ ( size_t ) 0 )


/* Alloc
 *  probably should be made to return SRAMem
 *  but returns a naked pointer
 */
void *SRAMemMgr_Alloc ( SRAMemMgr *self, struct SRAException *x,
   struct SRASourceLoc const *loc, const char *func, uint32_t lineno,
   size_t bytes, bool clear );

#define SRAMemMgrAlloc( self, x, bytes, clear ) \
    SRAMemMgr_Alloc ( self, x, & s_loc, __func__, __LINE__, bytes, clear )

/* Free
 *  front-end to free
 */
void SRAMemMgr_Free ( SRAMemMgr *self,
   struct SRASourceLoc const *loc, const char *func, uint32_t lineno,
   void *mem, size_t bytes );

#define SRAMemMgrFree( self, mem, bytes ) \
    SRAMemMgr_Free ( self, & s_loc, __func__, __LINE__, mem, bytes )


#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRAMem_ */
