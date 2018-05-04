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
#include "cursor-table.h"
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
#include <klib/debug.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* VTableCursor vtable, read side */

static rc_t VTableReadCursorWhack ( const VTableCursor *self );
static rc_t VTableReadCursorOpen ( const VTableCursor *cself );
static rc_t VTableReadCursorSetRowId ( const VTableCursor *cself, int64_t row_id );
static rc_t VTableReadCursorOpenRow ( const VTableCursor *cself );
static rc_t VTableReadCursorWrite ( VTableCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count );
static rc_t VTableReadCursorCommitRow ( VTableCursor *self );
static rc_t VTableReadCursorCloseRow ( const VTableCursor *cself );
static rc_t VTableReadCursorRepeatRow ( VTableCursor *self, uint64_t count );
static rc_t VTableReadCursorFlushPage ( VTableCursor *self );
static rc_t VTableReadCursorDefault ( VTableCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len );
static rc_t VTableReadCursorCommit ( VTableCursor *self );
static rc_t VTableReadCursorOpenParentUpdate ( VTableCursor *self, VTable **tbl );
static rc_t VTableReadCursorMakeColumn ( VTableCursor *self, VColumn **col, const SColumn *scol, Vector *cx_bind );
static rc_t VTableReadCursorInstallTrigger ( struct VTableCursor * self, struct VProduction * prod );

static VCursor_vt VTableCursor_read_vt =
{
    VTableReadCursorWhack,
    VTableCursorVAddColumn,
    VTableCursorVGetColumnIdx,
    VTableReadCursorOpen,
    VTableReadCursorSetRowId,
    VTableReadCursorOpenRow,
    VTableReadCursorWrite,
    VTableReadCursorCommitRow,
    VTableReadCursorCloseRow,
    VTableReadCursorRepeatRow,
    VTableReadCursorFlushPage,
    VTableCursorGetBlob,
    VTableCursorGetBlobDirect,
    VTableCursorRead,
    VTableCursorReadDirect,
    VTableCursorReadBits,
    VTableCursorReadBitsDirect,
    VTableCursorCellData,
    VTableCursorCellDataDirect,
    VTableCursorDataPrefetch,
    VTableReadCursorDefault,
    VTableReadCursorCommit,
    VTableCursorOpenParentRead,
    VTableReadCursorOpenParentUpdate,
    VTableCursorPermitPostOpenAdd,
    VTableCursorSuspendTriggers,
    VTableCursorGetSchema,
    VTableCursorLinkedCursorGet,
    VTableCursorLinkedCursorSet,
    VTableCursorSetCacheCapacity,
    VTableCursorGetCacheCapacity,
    VTableReadCursorMakeColumn,
    VTableCursorGetTable,
    VTableCursorIsReadOnly,
    VTableCursorGetBlobMruCache,
    VTableCursorIncrementPhysicalProductionCount,
    VTableCursorFindOverride,
    VTableCursorLaunchPagemapThread,
    VTableCursorPageMapProcessRequest,
    VTableCursorCacheActive,
    VTableReadCursorInstallTrigger
};

/*--------------------------------------------------------------------------
 * VTableCursor
 *  a row cursor onto a VTable
 */

rc_t VCursorMakeFromTable ( VTableCursor **cursp, const struct VTable *tbl )
{
    return VTableCursorMake ( cursp, tbl, & VTableCursor_read_vt );
}

/* Whack
 */
static
rc_t VTableReadCursorWhack ( const VTableCursor *self )
{
    VTableCursorTerminatePagemapThread( (VTableCursor*)self );
    return VTableCursorWhack ( self );
}

/* MakeColumn
 */
rc_t VTableReadCursorMakeColumn ( VTableCursor *self, VColumn **col, const SColumn *scol, Vector *cx_bind )
{
    return VColumnMake ( col, self -> schema, scol );
}

/* PostOpenAdd
 *  handle opening of a column after the cursor is opened
 */
rc_t VCursorPostOpenAdd ( VTableCursor *self, VColumn *col )
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
rc_t VTableReadCursorOpen ( const VTableCursor *cself )
{
    rc_t rc;
    VTableCursor *self = ( VTableCursor* ) cself;

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

                rc = VCursorIdRange ( & self -> dad, 0, & first, & count );
                if ( rc != 0 )
                {
                    /* permit empty open when run from sradb */
                    if ( GetRCState ( rc ) == rcEmpty && GetRCObject ( rc ) == rcRange &&
                         self -> permit_add_column && VectorLength ( & self -> dad . row ) == 0 )
                    {
                        rc = 0;
                    }
                }
                else if ( count != 0 )
                {
                    /* set initial row id to starting row */
                    self -> dad . start_id =
                    self -> dad . end_id =
                    self -> dad . row_id = first;
                }

                if ( rc != 0 )
                    self -> dad . state = vcFailed;
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
rc_t VTableReadCursorSetRowId ( const VTableCursor *cself, int64_t row_id )
{
    rc_t rc;
    VTableCursor *self = ( VTableCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcSelf, rcNull );
    else if ( self -> dad . state > vcReady )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcCursor, rcBusy );
    else
        rc = VCursorSetRowIdRead ( & self -> dad, row_id );

    return rc;
}


/* OpenRow
 *  open currently closed row indicated by row id
 */
rc_t VTableReadCursorOpenRow ( const VTableCursor *cself )
{
    rc_t rc;
    VTableCursor *self = ( VTableCursor* ) cself;

    if ( self -> dad . state < vcReady )
        rc = RC ( rcVDB, rcCursor, rcOpening, rcRow, rcIncomplete );
    else if ( self -> dad . state > vcReady )
        rc = 0;
    else
        rc = VCursorOpenRowRead ( & self -> dad );

    return rc;
}

/* CloseRow
 *  balances OpenRow message
 *  if there are uncommitted modifications,
 *  discard all changes. otherwise,
 *  advance to next row
 */
rc_t VTableReadCursorCloseRow ( const VTableCursor *cself )
{
    rc_t rc;
    VTableCursor *self = ( VTableCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcClosing, rcSelf, rcNull );
    else if ( self -> dad . state < vcRowOpen )
        rc = 0;
    else
        rc = VCursorCloseRowRead ( & self -> dad );

    return rc;
}

/* not implemented for the read side: */

rc_t VTableReadCursorWrite ( VTableCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorCommitRow ( VTableCursor *self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorRepeatRow ( VTableCursor *self, uint64_t count )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorFlushPage ( VTableCursor *self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorDefault ( VTableCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorCommit ( VTableCursor *self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorOpenParentUpdate ( VTableCursor *self, VTable **tbl )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VTableReadCursorInstallTrigger ( struct VTableCursor * self, struct VProduction * prod )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
