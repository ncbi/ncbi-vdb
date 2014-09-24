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
#include "column-priv.h"
#include "dbmgr-priv.h"
#include "schema-priv.h"
#include "cursor-priv.h"
#undef KONST

#include <vdb/manager.h>
#include <vdb/cursor.h>
#include <kdb/column.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * VColumn
 */


/* Whack
 *  perform read-only cleanup
 */
void CC VColumnWhack ( void *item, void *data )
{
    VColumn *self = item;
    VCursor *curs = data;

    /* remove from cursor */
    if ( curs != NULL )
    {
        VectorSwap ( & curs -> row, self -> ord, NULL, & item );
        VCursorCacheSwap ( & curs -> col, & self -> scol -> cid, NULL, & item );
    }

    VColumnDestroy ( self );
    free ( self );
}


/* IdRange
 *  returns id range for column or page
 */
rc_t VColumnIdRange ( const VColumn *self, int64_t *first, int64_t *last )
{
    return VColumnIdRangeRead ( self, first, last );
}
