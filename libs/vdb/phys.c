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

#include <vdb/extern.h>

#define KONST const
#define SKONST const
#include "phys-priv.h"
#include "prod-priv.h"
#include "blob-priv.h"
#undef KONST
#undef SKONST

#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * VPhysical
 */


/* Whack
 */
void CC VPhysicalWhack ( void *item, void *ignore )
{
    VPhysical *self = item;
    if ( self > FAILED_PHYSICAL )
        VPhysicalDestroy ( self );
}

/* Read
 *  get the blob
 */
rc_t VPhysicalRead ( VPhysical *self, VBlob **vblob, int64_t id, uint32_t cnt, uint32_t elem_bits )
{
    return VPhysicalReadBlob ( self, vblob, id, elem_bits );
}
