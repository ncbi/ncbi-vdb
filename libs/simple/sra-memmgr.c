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

#include <simple/extern.h>

#define NO_SOURCE_LOC 1

#include "sra-memmgr.h"
#include "sra-exception.h"

#include <klib/rc.h>

#include <stdlib.h>
#include <assert.h>

#include <sysalloc.h>


/*--------------------------------------------------------------------------
 * SRAMemMgr
 *  should be an actual object
 *  the functions are left global for now
 */

/* Alloc
 *  probably should be made to return SRAMem
 *  but returns a naked pointer
 */
void *SRAMemMgr_Alloc ( SRAMemMgr *self, SRAException *x,
   const SRASourceLoc *loc, const char *func, uint32_t lineno,
   size_t bytes, bool clear )
{
    rc_t rc;

    assert ( x != NULL );
    assert ( func != NULL );
    assert ( func [ 0 ] != 0 );

    if ( self == NULL )
    {
        rc = RC ( rcSRA, rcNoTarg, rcAllocating, rcSelf, rcNull );
        SRAException_Error ( x, loc, func, lineno, 0, rc, "bad memory manager" );
    }
    else
    {
        void *mem = clear ? calloc ( 1, bytes ) : malloc ( bytes );
        if ( mem == NULL )
        {
            rc = RC ( rcSRA, rcNoTarg, rcAllocating, rcMemory, rcExhausted );
            SRAException_Error ( x, loc, func, lineno, 0, rc, "out of memory" );
        }
        else
        {
            assert ( x -> rc == 0 );
            return mem;
        }
    }

    return NULL;
}

/* Free
 *  front-end to free
 */
void SRAMemMgr_Free ( SRAMemMgr *self,
   const SRASourceLoc *loc, const char *func, uint32_t lineno,
   void *mem, size_t bytes )
{
    free ( mem );
}
