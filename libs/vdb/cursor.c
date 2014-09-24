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
#include "cursor-priv.h"
#include "dbmgr-priv.h"
#include "linker-priv.h"
#include "schema-priv.h"
#include "table-priv.h"
#include "column-priv.h"
#undef KONST

#include <vdb/cursor.h>
#include <vdb/table.h>
#include <kfs/dyload.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VCursor
 *  a row cursor onto a VTable
 */


/* Whack
 */
rc_t VCursorWhack ( VCursor *self )
{
    VCursorTerminatePagemapThread(self);
    return VCursorDestroy ( self );
}


/* MakeColumn
 */
rc_t VCursorMakeColumn ( VCursor *self, VColumn **col, const SColumn *scol, Vector *cx_bind )
{
    return VColumnMake ( col, self -> schema, scol );
}

/* PostOpenAdd
 *  handle opening of a column after the cursor is opened
 */
rc_t VCursorPostOpenAdd ( VCursor *self, VColumn *col )
{
    return VCursorPostOpenAddRead ( self, col );
}

/* Open
 *  open cursor, resolving schema
 *  for the set of opened columns
 *
 *  NB - there is no corresponding "Close"
 *  use "Release" instead.
 */
LIB_EXPORT rc_t CC VCursorOpen ( const VCursor *cself )
{
    rc_t rc;
    VCursor *self = ( VCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcOpening, rcSelf, rcNull );
    else
    {
        VLinker *ld = self -> tbl -> linker;

        KDlset *libs;
        rc = VLinkerOpen ( ld, & libs );
        if ( rc == 0 )
        {
            rc = VCursorOpenRead ( self, libs );
            if ( rc == 0 )
            {
                int64_t first;
                uint64_t count;
                
                rc = VCursorIdRange ( self, 0, & first, & count );
                if ( rc != 0 )
                {
                    /* permit empty open when run from sradb */
                    if ( GetRCState ( rc ) == rcEmpty && GetRCObject ( rc ) == rcRange &&
                         self -> permit_add_column && VectorLength ( & self -> row ) == 0 )
                    {
                        rc = 0;
                    }
                }
                else if ( count != 0 )
                {
                    /* set initial row id to starting row */
                    self -> start_id = self -> end_id = self -> row_id = first;
                }

                if ( rc != 0 )
                    self -> state = vcFailed;
            }

            KDlsetRelease ( libs );
        }
    }

    return rc;
}


/* SetRowId
 *  seek to given row id
 *
 *  "row_id" [ IN ] - row id to select
 */
LIB_EXPORT rc_t CC VCursorSetRowId ( const VCursor *cself, int64_t row_id )
{
    rc_t rc;
    VCursor *self = ( VCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcSelf, rcNull );
    else if ( self -> state > vcReady )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcCursor, rcBusy );
    else
        rc = VCursorSetRowIdRead ( self, row_id );

    return rc;
}


/* OpenRow
 *  open currently closed row indicated by row id
 */
LIB_EXPORT rc_t CC VCursorOpenRow ( const VCursor *cself )
{
    rc_t rc;
    VCursor *self = ( VCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcOpening, rcSelf, rcNull );
    else if ( self -> state < vcReady )
        rc = RC ( rcVDB, rcCursor, rcOpening, rcRow, rcIncomplete );
    else if ( self -> state > vcReady )
        rc = 0;
    else
        rc = VCursorOpenRowRead ( self );

    return rc;
}

/* CloseRow
 *  balances OpenRow message
 *  if there are uncommitted modifications,
 *  discard all changes. otherwise,
 *  advance to next row
 */
LIB_EXPORT rc_t CC VCursorCloseRow ( const VCursor *cself )
{
    rc_t rc;
    VCursor *self = ( VCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcClosing, rcSelf, rcNull );
    else if ( self -> state < vcRowOpen )
        rc = 0;
    else
        rc = VCursorCloseRowRead ( self );

    return rc;
}
