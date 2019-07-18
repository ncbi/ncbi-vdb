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
#define TRACK_REFERENCES 0

#define KONST const
#include "prod-priv.h"
#include "prod-expr.h"
#include "schema-priv.h"
#include "schema-expr.h"
#include "cursor-priv.h"
#include "column-priv.h"
#undef KONST

#include <vdb/cursor.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * VProdResolve
 */

/* ResolveColumn
 *  resolves a column using read expression
 */
rc_t VProdResolveColumnRoot ( const VProdResolve *self,
    VProduction **out, const SColumn *scol )
{
    rc_t rc = VProdResolveColumnRead ( self, out, scol );
    if ( rc == 0 && * out <= FAILED_PRODUCTION )
        return RC ( rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined );
    return rc;
}

rc_t VProdResolveColumn ( const VProdResolve *self,
    VProduction **out, const SColumn *scol, bool alt )
{
    rc_t rc;
    VColumn *vcol;
    VCursor *curs = self -> curs;

    if (alt) {
        /* TODO: Generate warning message */
        return RC(rcVDB, rcCursor, rcOpening, rcSchema, rcInvalid);
    }
    /* read-only cursor may add columns from schema */
    vcol = VCursorCacheGet ( VCursorColumns ( curs ), & scol -> cid );
    if ( vcol == NULL )
    {
        rc = VCursorMakeColumn ( curs, & vcol, scol, self -> cx_bind );
        if ( rc != 0 )
            return rc;

#if OPEN_COLUMN_ALTERS_ROW
        rc = VectorAppend ( VCursorGetRow ( curs ), & vcol -> ord, vcol );
        if ( rc != 0 )
        {
            VColumnWhack ( vcol, NULL );
            return rc;
        }
#endif
        rc = VCursorCacheSet ( VCursorColumns ( curs ), & scol -> cid, vcol );
        if ( rc != 0 )
        {
#if OPEN_COLUMN_ALTERS_ROW
            void *ignore;
            VectorSwap ( VCursorGetRow ( curs ), vcol -> ord, NULL, & ignore );
            vcol -> ord = 0;
#endif
            VColumnWhack ( vcol, NULL );
            return rc;
        }
    }

    /* resolve for read production */
    return VProdResolveColumnRead ( self, out, scol );
}

rc_t VProdResolvePhysical ( const VProdResolve *self, struct VPhysical *phys )
{
    return VProdResolvePhysicalRead ( self, phys );
}

/*--------------------------------------------------------------------------
 * VColumnProd
 *  message redirect to VColumn
 */

rc_t VColumnProdMake ( VProduction **prodp, Vector *owned,
    VColumn *col, int sub, const char *name )
{
    return RC ( rcVDB, rcColumn, rcReading, rcSchema, rcInvalid );
}

void VColumnProdDestroy ( VColumnProd *self )
{
}

/* Read
 */
rc_t VColumnProdRead ( VColumnProd *self, struct VBlob **vblob, int64_t id )
{
    return RC ( rcVDB, rcColumn, rcReading, rcSchema, rcInvalid );
}

