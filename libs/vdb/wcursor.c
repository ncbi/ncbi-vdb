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

#include "cursor-table.h"
#include "dbmgr-priv.h"
#include "linker-priv.h"
#include "schema-priv.h"
#include "table-priv.h"
#include "column-priv.h"
#include "phys-priv.h"
#include "prod-priv.h"
#include "prod-expr.h"
#include "blob-priv.h"

#include <vdb/cursor.h>
#include <vdb/table.h>
#include <kdb/meta.h>
#include <kfs/dyload.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#if VCURSOR_FLUSH_THREAD

#include <kproc/lock.h>
#include <kproc/cond.h>
#include <kproc/thread.h>

#if _DEBUGGING
/* set to 1 to trigger behavior to simulate
   an error within flush thread */
#define FORCE_FLUSH_ERROR_EXIT 0
#endif

#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static bool s_disable_flush_thread = false;

/* VTableCursor vtable, write side */

static rc_t VTableWriteCursorWhack ( const VTableCursor *self );
static rc_t VTableWriteCursorOpen ( const VTableCursor *cself );
static rc_t VTableWriteCursorSetRowId ( const VTableCursor *cself, int64_t row_id );
static rc_t VTableWriteCursorOpenRow ( const VTableCursor *cself );
static rc_t VTableWriteCursorWrite ( VTableCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count );
static rc_t VTableWriteCursorCommitRow ( VTableCursor *self );
static rc_t VTableWriteCursorCloseRow ( const VTableCursor *cself );
static rc_t VTableWriteCursorRepeatRow ( VTableCursor *self, uint64_t count );
static rc_t VTableWriteCursorFlushPage ( VTableCursor *self );
static rc_t VTableWriteCursorDefault ( VTableCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len );
static rc_t VTableWriteCursorCommit ( VTableCursor *self );
static rc_t VTableWriteCursorOpenParentUpdate ( VTableCursor *self, VTable **tbl );
static rc_t VTableWriteCursorMakeColumn ( VTableCursor *self, VColumn **col, const SColumn *scol, Vector *cx_bind );
static rc_t VTableWriteCursorInstallTrigger ( struct VTableCursor * self, struct VProduction * prod );

static VCursor_vt VTableCursor_write_vt =
{
    VTableWriteCursorWhack,
    VTableCursorVAddColumn,
    VTableCursorVGetColumnIdx,
    VTableWriteCursorOpen,
    VTableWriteCursorSetRowId,
    VTableWriteCursorOpenRow,
    VTableWriteCursorWrite,
    VTableWriteCursorCommitRow,
    VTableWriteCursorCloseRow,
    VTableWriteCursorRepeatRow,
    VTableWriteCursorFlushPage,
    VTableCursorGetBlob,
    VTableCursorGetBlobDirect,
    VTableCursorRead,
    VTableCursorReadDirect,
    VTableCursorReadBits,
    VTableCursorReadBitsDirect,
    VTableCursorCellData,
    VTableCursorCellDataDirect,
    VTableCursorDataPrefetch,
    VTableWriteCursorDefault,
    VTableWriteCursorCommit,
    VTableCursorOpenParentRead,
    VTableWriteCursorOpenParentUpdate,
    VTableCursorPermitPostOpenAdd,
    VTableCursorSuspendTriggers,
    VTableCursorGetSchema,
    VTableCursorLinkedCursorGet,
    VTableCursorLinkedCursorSet,
    VTableCursorSetCacheCapacity,
    VTableCursorGetCacheCapacity,
    VTableWriteCursorMakeColumn,
    VTableCursorGetTable,
    VTableCursorIsReadOnly,
    VTableCursorGetBlobMruCache,
    VTableCursorIncrementPhysicalProductionCount,
    VTableCursorFindOverride,
    VTableCursorLaunchPagemapThread,
    VTableCursorPageMapProcessRequest,
    VTableCursorCacheActive,
    VTableWriteCursorInstallTrigger
};

/*--------------------------------------------------------------------------
 * VTableCursor
 *  a row cursor onto a VTable
 */

rc_t VCursorMakeFromTable ( VTableCursor **cursp, const struct VTable *tbl )
{
    return VTableCursorMake ( cursp, tbl, & VTableCursor_write_vt );
}

LIB_EXPORT rc_t CC VDBManagerDisableFlushThread(VDBManager *self)
{
    s_disable_flush_thread = true;
    return 0;
}

/* forward
 *  to avoid reordering whole page
 */
static
rc_t VCursorFlushPageInt ( VTableCursor *self, bool sync );

/* Whack
 */
static
rc_t VTableWriteCursorWhack ( const VTableCursor * p_self )
{
    VTableCursor * self = ( VTableCursor * ) p_self;
#if VCURSOR_FLUSH_THREAD
    if ( self -> flush_thread != NULL )
    {
        rc_t rc = KLockAcquire ( self -> flush_lock );
        if ( rc == 0 )
        {
            while ( self -> flush_state == vfBusy )
            {
                MTCURSOR_DBG (( "VTableWriteCursorWhack: waiting for thread to process\n" ));
                KConditionWait ( self -> flush_cond, self -> flush_lock );
            }
            self -> flush_state = vfExit;
            KConditionSignal ( self -> flush_cond );
            KLockUnlock ( self -> flush_lock );
        }

        MTCURSOR_DBG (( "VTableWriteCursorWhack: waiting on thread to exit\n" ));
        KThreadWait ( self -> flush_thread, NULL );
    }

    MTCURSOR_DBG (( "VTableWriteCursorWhack: finishing\n" ));
    KThreadRelease ( self -> flush_thread );
    KConditionRelease ( self -> flush_cond );
    KLockRelease ( self -> flush_lock );
#endif
    VTableCursorTerminatePagemapThread ( self );
    return VTableCursorWhack ( self );
}

/* CreateCursor
 *  creates a cursor object onto table
 *  multiple read cursors are allowed
 *  only a single write cursor is allowed
 *
 *  "curs" [ OUT ] - return parameter for newly created cursor
 *
 *  "mode" [ IN ] - describes udate behavior
 *    kcmUpdate   : allow inserts or updates
 *    kcmReplace  : replace all existing rows with newly written rows
 *    kcmInsert   : allow only inserts, i.e. new rows
 */
#if VCURSOR_FLUSH_THREAD
static rc_t CC run_flush_thread ( const KThread *t, void *data );
#endif

rc_t VTableCreateCursorWriteInt ( VTable *self, VTableCursor **cursp, KCreateMode mode, bool create_thread )
{
    rc_t rc;

    if ( cursp == NULL )
        rc = RC ( rcVDB, rcCursor, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcVDB, rcCursor, rcCreating, rcTable, rcReadonly );
#if VCURSOR_WRITE_MODES_SUPPORTED
#error "expecting kcmInsert mode only"
#else
        else if ( mode != kcmInsert )
            rc = RC ( rcVDB, rcCursor, rcCreating, rcMode, rcUnsupported );
#endif
        else
        {
            VTableCursor *curs;

#if LAZY_OPEN_COL_NODE
            if ( self -> col_node == NULL )
                KMetadataOpenNodeUpdate ( self -> meta, & self -> col_node, "col" );
#endif
            rc = VTableCursorMake ( & curs, self,&  VTableCursor_write_vt );
            if ( rc == 0 )
            {
                rc = VCursorSupplementSchema ( curs );
#if VCURSOR_FLUSH_THREAD
                if ( rc == 0 && create_thread )
                {
                    rc = KLockMake ( & curs -> flush_lock );
                    if ( rc == 0 )
                        rc = KConditionMake ( & curs -> flush_cond );
                    if ( rc == 0 )
                        rc = KThreadMake ( & curs -> flush_thread, run_flush_thread, curs );
                }
#endif
                if ( rc == 0 )
                {
                    * cursp = curs;
                    return 0;
                }

                VCursorRelease ( & curs -> dad );
            }
        }

        * cursp = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableCreateCursorWrite ( VTable *self, VCursor **cursp, KCreateMode mode )
{
    return VTableCreateCursorWriteInt ( self, (VTableCursor**)cursp, mode, !s_disable_flush_thread );
}


/* MakeColumn
 */
rc_t VTableWriteCursorMakeColumn ( VTableCursor *self, VColumn **col, const SColumn *scol, Vector *cx_bind )
{
    VTable *vtbl;

    if ( self -> read_only )
        return VColumnMake ( col, self -> schema, scol );

    vtbl = self -> tbl;
    return WColumnMake ( col, self -> schema, scol, vtbl -> stbl -> limit, vtbl -> mgr, cx_bind );
}


/* PostOpenAdd
 *  handle opening of a column after the cursor is opened
 */
rc_t VCursorPostOpenAdd ( VTableCursor *self, VColumn *col )
{
    rc_t rc = VCursorPostOpenAddRead ( self, col );

    if ( ! self -> read_only && rc == 0 && self -> dad . state == vcRowOpen )
    {
        int64_t row_id = self -> dad . row_id;
        WColumnOpenRow ( col, & row_id );
    }

    return rc;
}

/* Open
 *  open cursor, resolving schema
 *  for the set of opened columns
 *
 *  NB - there is no corresponding "Close"
 *  use "Release" instead.
 */
static
rc_t VProdResolveAddShallowTriggers ( const VProdResolve *self, const STable *stbl )
{
    rc_t rc;
    VTableCursor *curs;
    uint32_t i = VectorStart ( & stbl -> prod );
    uint32_t end = i + VectorLength ( & stbl -> prod );

    for ( rc = 0, curs = ( VTableCursor *) ( self -> curs ); ( rc == 0 || self -> ignore_column_errors ) && i < end; ++ i )
    {
        SProduction *sprod = VectorGet ( & stbl -> prod, i );
        if ( sprod != NULL && sprod -> trigger )
        {
            VProduction *prod = NULL;
            rc = VProdResolveSProduction ( self, & prod, sprod );
            if ( rc == 0 && prod != NULL )
                rc = VectorAppend ( & curs -> trig, NULL, prod );
        }
    }

    if ( self -> ignore_column_errors )
        return 0;

    return rc;
}

static
rc_t VProdResolveAddTriggers ( const VProdResolve *self, const STable *stbl )
{
    uint32_t i = VectorStart ( & stbl -> overrides );
    uint32_t end = VectorLength ( & stbl -> overrides );
    for ( end += i; i < end; ++ i )
    {
        const STable *dad = STableFindOrdAncestor ( stbl, i );
        rc_t rc = VProdResolveAddShallowTriggers ( self, dad );
        if ( rc != 0 )
            return rc;
    }
    return VProdResolveAddShallowTriggers ( self, stbl );
}

rc_t VTableWriteCursorOpen ( const VTableCursor *cself )
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
                if ( ! self -> read_only )
                {
                    Vector cx_bind;
                    VProdResolve pr;
                    pr . schema = self -> schema;
                    pr . ld = ld;
                    pr . libs = libs;
                    pr . name = & self -> stbl -> name -> name;
                    pr . curs = & self -> dad;
                    pr . cache = & self -> dad . prod;
                    pr . owned = & self -> dad . owned;
                    pr . cx_bind = & cx_bind;
                    pr . chain = chainEncoding;
                    pr . blobbing = false;
                    pr . ignore_column_errors = false;
                    pr . discover_writable_columns = false;

                    VectorInit ( & cx_bind, 1, self -> schema -> num_indirect );

                    if ( ! self -> suspend_triggers )
                        rc = VProdResolveAddTriggers ( & pr, self -> stbl );

                    VectorWhack ( & cx_bind, NULL, NULL );
                }

                if ( rc == 0 )
                {
                    /* TBD - warn if any input columns are unreferenced by schema */

                    int64_t first;
                    uint64_t count;

                    rc = VCursorIdRange ( & self -> dad, 0, & first, & count );
                    if ( rc != 0 )
                    {
                        if ( GetRCState ( rc ) == rcEmpty )
                        {
                            /* writable cursors are expected to be empty */
                            if ( ! self -> read_only ||
                                 /* permit empty open when run from sradb */
                                 ( GetRCObject ( rc ) == rcRange &&
                                   self -> permit_add_column &&
                                   VectorLength ( & self -> dad . row ) == 0 ) )
                            {
                                rc = 0;
                            }
                        }
                    }
                    else if ( count != 0 )
                    {
                        if ( self -> read_only )
                        {
                            /* set initial row id to starting row */
                            self -> dad . start_id =
                            self -> dad . end_id =
                            self -> dad . row_id = first;
                        }
                        else
                        {
                            /* set initial row id to append */
                            self -> dad . start_id =
                            self -> dad . end_id =
                            self -> dad . row_id = first + count;
                        }
                    }
                }

                if ( rc != 0 )
                    self -> dad . state = vcFailed;
            }

            KDlsetRelease ( libs );
        }
    }

    return rc;
}


/* ListWritableColumns
 *  walks list of SPhysicals and trigger SProductions
 *  attempts to resolve all write rules
 *  records any SColumn that can be reached
 *  populates BTree with VColumnRef objects
 */
struct resolve_phys_data
{
    VProdResolve pr;
    const KNamelist *seed;
    uint32_t count;
};

static
void CC resolve_writable_sphys ( void *item, void *data )
{
    struct resolve_phys_data *pb = data;
    const SPhysMember *smbr = ( const void* ) item;
    VProduction *out = NULL;

    if ( pb -> seed == NULL )
        VProdResolveSPhysMember ( & pb -> pr, & out, smbr );
    else
    {
        uint32_t i;
        const KSymbol *sym = smbr -> name;
        const char *sname = sym -> name . addr;
        size_t ssize = sym -> name . size;
        if ( sname [ 0 ] == '.' )
        {
            ++ sname;
            -- ssize;
        }

        /* TBD - this is not too speedy, but it is
           very low frequency in all known cases */
        for ( i = 0; i < pb -> count; ++ i )
        {
            const char *name;
            rc_t rc = KNamelistGet ( pb -> seed, i, & name );
            if ( rc == 0 )
            {
                if ( strlen ( name ) == ssize && memcmp ( sname, name, ssize ) == 0 )
                {
                    VProdResolveSPhysMember ( & pb -> pr, & out, smbr );
                    break;
                }
            }
        }
    }
}

static
void VProdResolveWritableColumns ( struct resolve_phys_data *pb, bool suspend_triggers )
{
    const STable *dad;
    const STable *stbl = ( ( const VTableCursor * ) ( pb -> pr . curs ) )-> stbl;

    /* walk table schema looking for parents */
    uint32_t i = VectorStart ( & stbl -> overrides );
    uint32_t end = VectorLength ( & stbl -> overrides );
    for ( end += i; i < end; ++ i )
    {
        dad = STableFindOrdAncestor ( stbl, i );
        VectorForEach ( & dad -> phys, false, resolve_writable_sphys, pb );
    }

    /* walk current table */
    VectorForEach ( & stbl -> phys, false, resolve_writable_sphys, pb );

    /* add triggers */
    if ( !suspend_triggers && pb -> seed == NULL )
    {
        pb -> pr . chain = chainUncommitted;
        VProdResolveAddTriggers ( & pb -> pr, stbl );
    }
}

rc_t VCursorListSeededWritableColumns ( VTableCursor *self, BSTree *columns, const KNamelist *seed )
{
    rc_t rc;
    KDlset *libs;

    Vector cx_bind;
    struct resolve_phys_data pb;
    pb . pr . schema = self -> schema;
    pb . pr . ld = self -> tbl -> linker;
    pb . pr . name = & self -> stbl -> name -> name;
    pb . pr . curs = & self -> dad;
    pb . pr . cache = & self -> dad . prod;
    pb . pr . owned = & self -> dad . owned;
    pb . pr . cx_bind = & cx_bind;
    pb . pr . chain = chainEncoding;
    pb . pr . blobbing = false;
    pb . pr . ignore_column_errors = true;
    pb . pr . discover_writable_columns = true;
    pb . seed = seed;

    if ( seed != NULL )
    {
        rc = KNamelistCount ( seed, & pb . count );
        if ( rc != 0 )
            return rc;
    }

    VectorInit ( & cx_bind, 1, self -> schema -> num_indirect );

    /* open the dynamic linker libraries */
    rc = VLinkerOpen ( pb . pr . ld, & libs );
    if ( rc == 0 )
    {
        pb . pr . libs = libs;
        VProdResolveWritableColumns ( & pb , self -> suspend_triggers );

        VectorWhack ( & cx_bind, NULL, NULL );
        KDlsetRelease ( libs );

        if ( rc == 0 )
        {
            /* add columns to list */
            uint32_t idx = VectorStart ( & self -> dad . row );
            uint32_t end = VectorLength ( & self -> dad . row );

            for ( end += idx; idx < end; ++idx )
            {
                const VColumn* vcol = ( const VColumn* ) VectorGet ( & self -> dad . row, idx );
                if ( vcol != NULL )
                {
                    VColumnRef *cref;
                    rc = VColumnRefMake ( & cref, self -> schema, vcol -> scol );
                    if ( rc != 0 )
                        break;

                    rc = BSTreeInsert ( columns, & cref -> n, VColumnRefSort );
                    assert ( rc == 0 );
                }
            }
        }
    }

    return rc;
}

rc_t VCursorListWritableColumns ( VTableCursor *self, BSTree *columns )
{
    return VCursorListSeededWritableColumns ( self, columns, NULL );
}

/* SetRowId
 *  seek to given row id
 *
 *  "row_id" [ IN ] - row id to select
 */
rc_t VTableWriteCursorSetRowId ( const VTableCursor *cself, int64_t row_id )
{
    rc_t rc;
    VTableCursor *self = ( VTableCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcSelf, rcNull );
    else if ( self -> dad . state == vcFailed )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcCursor, rcInvalid );
    else if ( self -> dad . state > vcReady || self -> dad . start_id < self -> dad . end_id )
        rc = RC ( rcVDB, rcCursor, rcPositioning, rcCursor, rcBusy );
    else if ( self -> read_only )
        rc = VCursorSetRowIdRead ( & self -> dad, row_id );
    else
    {
        /* the test of start/end range above tells us that
           no rows are buffered, so the row id can be simply set */
        self -> dad . start_id =
        self -> dad . end_id =
        self -> dad . row_id = row_id;
        rc = 0;
    }

    return rc;
}


/* OpenRow
 *  open currently closed row indicated by row id
 */
rc_t VTableWriteCursorOpenRow ( const VTableCursor *cself )
{
    rc_t rc;
    VTableCursor *self = ( VTableCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcOpening, rcSelf, rcNull );
    else if ( self -> dad . state != vcReady )
    {
        switch ( self -> dad . state )
        {
        case vcConstruct:
            rc = RC ( rcVDB, rcCursor, rcOpening, rcRow, rcIncomplete );
            break;
        case vcFailed:
            rc = RC ( rcVDB, rcCursor, rcOpening, rcCursor, rcInvalid );
            break;
        case vcRowOpen:
            rc = 0;
            break;
        default:
            rc = RC ( rcVDB, rcCursor, rcOpening, rcRow, rcBusy );
        }
    }
    else if ( self -> read_only )
        rc = VCursorOpenRowRead ( & self -> dad );
    else
    {
        /* validate that all columns have the same starting row_id */
        int64_t row_id = self -> dad . row_id;
        VectorForEach ( & self -> dad . row, false, WColumnOpenRow, & row_id );
        assert ( row_id == self -> dad . row_id );
        self -> dad . state = vcRowOpen;
        rc = 0;
    }

    return rc;
}

/* CommitRow
 *  commit row after writing
 *  prevents further writes
 */
rc_t VTableWriteCursorCommitRow ( VTableCursor *self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcCommitting, rcSelf, rcNull );
    else if ( self -> read_only )
        rc = RC ( rcVDB, rcCursor, rcCommitting, rcCursor, rcReadonly );
    else if ( self -> dad . state == vcFailed )
        rc = RC ( rcVDB, rcCursor, rcCommitting, rcCursor, rcInvalid );
    else if ( self -> dad . state < vcRowOpen )
        rc = RC ( rcVDB, rcCursor, rcCommitting, rcRow, rcNotOpen );
    else if ( self -> dad . state >= vcRowCommitted )
        rc = 0;
    else
    {
        self -> permit_add_column = self -> permit_post_open_add;

        /* tell columns to slam in any default values */
        rc = 0;
        if ( VectorDoUntil ( & self -> dad . row, false, WColumnRowDefaults, & rc ) )
            return rc;

        /* account for row just written */
        self -> dad . end_id = self -> dad . row_id + 1;

        /* tell columns to commit the row, and allow
           each to return an earlier cutoff id ( half-closed ) */
        if ( VectorDoUntil ( & self -> dad . row, false, WColumnCommitRow, & self -> dad . end_id ) )
        {
            self -> dad . state = vcFailed;
            return RC ( rcVDB, rcCursor, rcCommitting, rcMemory, rcExhausted );
        }

        /* returned result should never be <= start id */
        assert ( self -> dad . end_id > self -> dad . start_id );

        /* if returned result dips down into the range of buffered rows
           then one or more columns has requested an automatic page commit */
        self -> dad . state = ( self -> dad . end_id <= self -> dad . row_id ) ? vcPageCommit : vcRowCommitted;
    }

    return rc;
}

/* RepeatRow
 *  repeats the current row by the count provided
 *  row must have been committed
 *
 *  AVAILABILITY: version 2.6
 *
 *  "count" [ IN ] - the number of times to repeat
 *  the current row.
 */
rc_t VTableWriteCursorRepeatRow ( VTableCursor *self, uint64_t count )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcSelf, rcNull );
    else if ( self -> read_only )
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcCursor, rcReadonly );
    else if ( self -> dad . state == vcFailed )
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcCursor, rcInvalid );
    else if ( self -> dad . state < vcRowOpen )
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcRow, rcNotOpen );
    else if ( self -> dad . state < vcRowCommitted )
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcRow, rcInvalid );
    else if ( count > 0xFFFFFFFFU )
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcParam, rcExcessive );
    else if ( count != 0 )
    {
        /* check the number of rows that would result */
        uint64_t total = ( self -> dad . row_id - self -> dad . start_id ) + count;
        if ( total > 0xFFFFFFFFU )
            rc = RC ( rcVDB, rcCursor, rcUpdating, rcParam, rcExcessive );
        else
        {
            WColumnRepeatRowData pb;
            pb . count = count;
            pb . row_id = self -> dad . row_id;
            pb . end_id = self -> dad . end_id;

            /* tell columns to commit the row, and allow
               each to return an earlier cutoff id ( half-closed ) */
            VectorForEach ( & self -> dad . row, false, WColumnRepeatRow, & pb );

            /* extend the current row-id, dragging end_id along with it */
            if ( self -> dad . end_id >= self -> dad . row_id )
                self -> dad . end_id += count;

            /* move the current row id ahead */
            self -> dad . row_id += count;
        }
    }

    return rc;
}

/* CloseRow
 *  balances OpenRow message
 *  if there are uncommitted modifications,
 *  discard all changes. otherwise,
 *  advance to next row
 */
rc_t VTableWriteCursorCloseRow ( const VTableCursor *cself )
{
    rc_t rc = 0;        /* needed in case FlushPage isn't called */
    VTableCursor *self = ( VTableCursor* ) cself;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcClosing, rcSelf, rcNull );
    else if ( self -> dad . state == vcFailed )
        rc = RC ( rcVDB, rcCursor, rcClosing, rcCursor, rcInvalid );
    else if ( self -> dad . state < vcRowOpen )
        rc = 0;
    else if ( self -> read_only )
        rc = VCursorCloseRowRead ( & self -> dad);
    else
    {
        /* tell each of the columns that no further data may be written
           and to abandon any uncommitted writes */
        VectorForEach ( & self -> dad . row, false, WColumnCloseRow, NULL );

        /* if the row was committed... */
        if ( self -> dad . state >= vcRowCommitted )
        {
            /* close off the page if so requested */
            if ( self -> dad . state == vcPageCommit )
            {
                rc = VCursorFlushPageInt(self, false);
                if ( rc )
                {
                    self -> dad . state = vcFailed;
                    return rc;
                }
            }

            /* advance to next id */
            ++ self -> dad . row_id;
        }

        self -> dad . state = vcReady;
        rc = 0;
    }

    return rc;
}


/* Default
 *  give a default row value for column
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ IN ] - stated element size in bits, required
 *  to be compatible with the actual element size
 *
 *  "buffer" [ IN ] and "boff" [ IN ] - compound pointer and offset
 *  to start of default row data where "boff" is in BITS
 *
 *  "row_len" [ IN ] - the number of elements in default row
 */
rc_t VTableWriteCursorDefault ( VTableCursor *self, uint32_t col_idx,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcSelf, rcNull );
    else if ( buffer == NULL && ( boff != 0 || row_len != 0 ) )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcParam, rcNull );
    else if ( self -> read_only )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
    else if ( elem_bits == 0 )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcParam, rcInvalid );
    else
    {
        VColumn *col = VectorGet ( & self -> dad . row, col_idx );
        if ( col == NULL )
            rc = RC ( rcVDB, rcCursor, rcWriting, rcColumn, rcInvalid );
        else
            rc = WColumnSetDefault ( col, elem_bits, buffer, boff, row_len );
    }

    return rc;
}


/* Write
 *  append bit-aligned column data to row
 *
 *  "col_idx" [ IN ] - index of column to be read, returned by "AddColumn"
 *
 *  "elem_bits" [ IN ] - stated element size in bits, required
 *  to be compatible with the actual element size
 *
 *  "buffer" [ IN ] and "boff" [ IN ] - compound pointer and offset
 *  to start of default row data where "boff" is in BITS
 *
 *  "count" [ IN ] - the number of elements to append
 */
rc_t VTableWriteCursorWrite ( VTableCursor *self, uint32_t col_idx,
    bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcSelf, rcNull );
    else if ( buffer == NULL && count != 0 )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcParam, rcNull );
    else if ( self -> read_only )
        rc = RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
    else if ( self -> dad . state != vcRowOpen )
    {
        switch ( self -> dad . state )
        {
        case vcConstruct:
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcNotOpen );
            break;
        case vcFailed:
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcInvalid );
            break;
        case vcReady:
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcRow, rcNotOpen );
            break;
        default:
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcRow, rcLocked );
        }
    }
    else
    {
        VColumn *col = VectorGet ( & self -> dad . row, col_idx );
        if ( col == NULL )
            rc = RC ( rcVDB, rcCursor, rcWriting, rcColumn, rcInvalid );
        else
            rc = WColumnWrite ( col, elem_bits, buffer, boff, count );
    }

    return rc;
}


/* FlushPage
 *  forces flush of all buffered page data
 *  fails if row is open
 *
 *  pages are normally auto-committed based upon
 *  size and column affinity
 */
typedef struct run_trigger_prod_data run_trigger_prod_data;
struct run_trigger_prod_data
{
    int64_t  id;
    uint32_t cnt;
    rc_t rc;
};

static
bool CC run_trigger_prods ( void *item, void *data )
{
    run_trigger_prod_data *pb = data;
    VProduction *prod = item;

    VBlob *blob;
    int64_t rowId = pb -> id;
    pb -> rc = VProductionReadBlob ( prod, & blob, & rowId, pb -> cnt, NULL);
    if ( pb -> rc != 0 )
        return true;
    if ( blob != NULL )
        TRACK_BLOB ( VBlobRelease, blob );
    ( void ) VBlobRelease ( blob );
    return false;
}

#if VCURSOR_FLUSH_THREAD
static
rc_t CC run_flush_thread ( const KThread *t, void *data )
{
    rc_t rc;
    VTableCursor *self = data;

    /* acquire lock */
    MTCURSOR_DBG (( "run_flush_thread: acquiring lock\n" ));
    rc = KLockAcquire ( self -> flush_lock );
    if ( rc == 0 )
    {
        do
        {
            bool failed;
            run_trigger_prod_data pb;

            /* wait for data */
            if ( self -> flush_state == vfReady )
            {
                MTCURSOR_DBG (( "run_flush_thread: waiting for input\n" ));
                rc = KConditionWait ( self -> flush_cond, self -> flush_lock );
                if ( rc != 0 )
                    break;
            }

            /* bail unless state is busy */
            if ( self -> flush_state != vfBusy )
            {
                MTCURSOR_DBG (( "run_flush_thread: exiting\n" ));
                break;
            }

            /* prepare param block */
            pb . id = self -> flush_id;
            pb . cnt = self -> flush_cnt;
            pb . rc = 0;

            MTCURSOR_DBG (( "run_flush_thread: unlocking and running\n" ));
            KLockUnlock ( self -> flush_lock );

            /* run productions from trigger roots */
            failed = VectorDoUntil ( & self -> trig, false, run_trigger_prods, & pb );

            /* drop page buffers */
            MTCURSOR_DBG (( "run_flush_thread: dropping page buffers\n" ));
            VectorForEach ( & self -> dad . row, false, WColumnDropPage, NULL );

            /* reacquire lock */
            MTCURSOR_DBG (( "run_flush_thread: re-acquiring lock" ));
            rc = KLockAcquire ( self -> flush_lock );
            if ( rc != 0 )
            {
                self -> flush_state = vfBgErr;
                LOGERR ( klogSys, rc, "run_flush_thread: re-acquiring lock failed - exit" );
                return rc;
            }

#if FORCE_FLUSH_ERROR_EXIT
            if ( ! failed )
            {
                pb . rc = RC ( rcVDB, rcCursor, rcFlushing, rcThread, rcCanceled );
                failed = true;
            }
#endif
            /* get out on failure */
            if ( failed )
            {
                self -> flush_state = vfBgErr;
                LOGERR ( klogInt, pb . rc, "run_flush_thread: run_trigger_prods failed - exit" );
                KConditionSignal ( self -> flush_cond );
                rc = pb . rc;
            }

            /* no longer busy */
            else if ( self -> flush_state == vfBusy )
            {
                /* signal waiter */
                self -> flush_state = vfReady;
                MTCURSOR_DBG (( "run_flush_thread: signaling ready\n" ));
                rc = KConditionSignal ( self -> flush_cond );
                if ( rc != 0 )
                    LOGERR ( klogSys, rc, "run_flush_thread: failed to signal foreground thread - exit" );
            }
        }
        while ( rc == 0 );

        MTCURSOR_DBG (( "run_flush_thread: unlocking\n" ));
        KLockUnlock ( self -> flush_lock );
    }

    MTCURSOR_DBG (( "run_flush_thread: exit\n" ));
    return rc;
}
#endif

static
rc_t VCursorFlushPageNoThread ( VTableCursor *self )
{
    int64_t end_id = self -> dad . end_id;

    /* first, tell all columns to bundle up their pages into buffers */
    if ( VectorDoUntil ( & self -> dad . row, false, WColumnBufferPage, & end_id ) )
    {
        VectorForEach ( & self -> dad . row, false, WColumnDropPage, NULL );
        return RC ( rcVDB, rcCursor, rcFlushing, rcMemory, rcExhausted );
    }
    else
    {
        run_trigger_prod_data pb;

        /* supposed to be constant */
        assert ( end_id == self -> dad . end_id );
        /* run all validation and trigger productions */
        pb . id = self -> dad . start_id;
        pb . cnt = self -> dad . end_id - self -> dad . start_id;
        pb . rc = 0;
        if ( ! VectorDoUntil ( & self -> trig, false, run_trigger_prods, & pb ) )
        {
            self -> dad . start_id = self -> dad . end_id;
            self -> dad . end_id = self -> dad . row_id + 1;
            self -> dad . state = vcReady;
        }

        /* drop page buffers */
        VectorForEach ( & self -> dad . row, false, WColumnDropPage, NULL );

        return pb . rc;
    }
}

static
rc_t VCursorFlushPageThread ( VTableCursor *self, bool sync )
{
    rc_t rc = 0;
    int64_t end_id = self -> dad . end_id;

#if VCURSOR_FLUSH_THREAD
    MTCURSOR_DBG (( "VCursorFlushPageInt: going to acquire lock\n" ));
    /* get lock */
    rc = KLockAcquire ( self -> flush_lock );
    if ( rc != 0 )
        return rc;

    MTCURSOR_DBG (( "VCursorFlushPageInt: have lock\n" ));

    /* make sure that background thread is ready */
    while ( self -> flush_state == vfBusy )
    {
        MTCURSOR_DBG (( "VCursorFlushPageInt: waiting for background thread\n" ));
        rc = KConditionWait ( self -> flush_cond, self -> flush_lock );
        if ( rc != 0 )
        {
            LOGERR ( klogSys, rc, "VCursorFlushPageInt: wait failed - exiting" );
            KLockUnlock ( self -> flush_lock );
            return rc;
        }
    }

    if ( self -> flush_state != vfReady )
    {
        if ( self -> flush_state != vfBgErr )
            rc = RC ( rcVDB, rcCursor, rcFlushing, rcCursor, rcInconsistent );
        else
        {
            rc_t rc2;
            MTCURSOR_DBG (( "VCursorFlushPageInt: waiting on thread to exit\n" ));
            rc = KThreadWait ( self -> flush_thread, & rc2 );
            if ( rc == 0 )
            {
                rc = rc2;
                MTCURSOR_DBG (( "VCursorFlushPageInt: releasing thread\n" ));
                KThreadRelease ( self -> flush_thread );
                self -> flush_thread = NULL;
            }
        }

        PLOGERR ( klogInt, (klogInt, rc, "VCursorFlushPageInt: not in ready state[$(state)] - exiting","state=%hu",self -> flush_state ));
        KLockUnlock ( self -> flush_lock );
        return rc;
    }

    MTCURSOR_DBG (( "VCursorFlushPageInt: running buffer page\n" ));

    /* first, tell all columns to bundle up their pages into buffers */
    if ( VectorDoUntil ( & self -> dad . row, false, WColumnBufferPage, & end_id ) )
    {
        VectorForEach ( & self -> dad . row, false, WColumnDropPage, NULL );
        self -> flush_state = vfFgErr;
        rc = RC ( rcVDB, rcCursor, rcFlushing, rcMemory, rcExhausted );
    }
    else
    {
        /* supposed to be constant */
        assert ( end_id == self -> dad . end_id );

        MTCURSOR_DBG (( "VCursorFlushPageInt: pages buffered - capturing id and count\n" ));
        self -> flush_id = self -> dad . start_id;
        self -> flush_cnt = self -> dad . end_id - self -> dad . start_id;

        self -> dad . start_id = self -> dad . end_id;
        self -> dad . end_id = self -> dad . row_id + 1;
        self -> dad . state = vcReady;

        MTCURSOR_DBG (( "VCursorFlushPageInt: state set to busy - signaling bg thread\n" ));
        self -> flush_state = vfBusy;
        rc = KConditionSignal ( self -> flush_cond );
        if ( rc != 0 )
            LOGERR ( klogSys, rc, "VCursorFlushPageInt: condition returned error on signal" );
    }

    MTCURSOR_DBG (( "VCursorFlushPageInt: unlocking\n" ));
    KLockUnlock ( self -> flush_lock );

    if (sync && rc == 0) {
        /* wait for flush to finish before returning */
        MTCURSOR_DBG (( "VCursorFlushPage: going to acquire lock\n" ));
        /* get lock */
        rc = KLockAcquire ( self -> flush_lock );
        if ( rc != 0 )
            return rc;

        MTCURSOR_DBG (( "VCursorFlushPage: have lock\n" ));

        /* wait until background thread has finished */
        while ( self -> flush_state == vfBusy )
        {
            MTCURSOR_DBG (( "VCursorFlushPage: waiting for background thread\n" ));
            rc = KConditionWait ( self -> flush_cond, self -> flush_lock );
            if ( rc != 0 )
            {
                LOGERR ( klogSys, rc, "VCursorFlushPage: wait failed - exiting" );
                KLockUnlock ( self -> flush_lock );
                return rc;
            }
        }

        /* what was the proper rc */
        if ( self -> flush_state != vfReady )
        {
            if ( self -> flush_state != vfBgErr )
                rc = RC ( rcVDB, rcCursor, rcFlushing, rcCursor, rcInconsistent );
            else
            {
                rc_t rc2;
                MTCURSOR_DBG (( "VCursorFlushPage: waiting on thread to exit\n" ));
                rc = KThreadWait ( self -> flush_thread, & rc2 );
                if ( rc == 0 )
                {
                    rc = rc2;
                    MTCURSOR_DBG (( "VCursorFlushPage: releasing thread\n" ));
                    KThreadRelease ( self -> flush_thread );
                    self -> flush_thread = NULL;
                }
            }

            PLOGERR ( klogInt, (klogInt, rc, "VCursorFlushPage: not in ready state[$(state)] - exiting",
                                "state=%hu", self -> flush_state ));

            KLockUnlock ( self -> flush_lock );
            return rc;
        }
        KLockUnlock ( self -> flush_lock );
    }
#endif
    return rc;
}

static
rc_t VCursorFlushPageInt ( VTableCursor *self, bool sync )
{
    switch ( self -> dad . state )
    {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcFlushing, rcCursor, rcNotOpen );
        case vcFailed:
            return RC ( rcVDB, rcCursor, rcFlushing, rcCursor, rcInvalid );
        case vcRowOpen:
            return RC ( rcVDB, rcCursor, rcFlushing, rcCursor, rcBusy );
        default:
            break;
    }
    if ( self -> dad . start_id == self -> dad . end_id )
    {
        /* the cursor should be in unwritten state,
           where the row_id can be reset but drags
           along the other markers. */
        assert ( self -> dad . end_id == self -> dad . row_id );
        return 0;
    }
    if (self->flush_thread)
        return VCursorFlushPageThread(self, sync);
    else
        return VCursorFlushPageNoThread(self);
}

rc_t VTableWriteCursorFlushPage ( VTableCursor *self )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC ( rcVDB, rcCursor, rcFlushing, rcSelf, rcNull );
    else if ( self -> read_only )
        rc = RC ( rcVDB, rcCursor, rcFlushing, rcCursor, rcReadonly );
    else {
        if ( self -> dad . state == vcRowCommitted ) {
            rc = VCursorCloseRow ( (VCursor*)self );
        }
        if ( rc == 0 ) {
            rc = VCursorFlushPageInt ( self, true );
        }
    }

    if ( rc == 0 )
    {
        assert ( self -> dad . row_id == self -> dad . start_id );
        self -> dad . end_id = self -> dad . row_id;
    }

    return rc;
}

rc_t VTableWriteCursorCommit ( VTableCursor *self )
{
    rc_t rc = VCursorFlushPage ( (VCursor*)self );
    if ( rc == 0 )
    {
        VTable *tbl = self -> tbl;
        if ( tbl -> read_col_cache_valid )
        {
            tbl -> read_col_cache_valid = false;
            BSTreeWhack ( & tbl -> read_col_cache, VColumnRefWhack, NULL );
        }
        if ( tbl -> write_col_cache_valid )
        {
            tbl -> write_col_cache_valid = false;
            BSTreeWhack ( & tbl -> write_col_cache, VColumnRefWhack, NULL );
        }
    }
    return rc;
}


/* OpenParent
 *  duplicate reference to parent table
 *  NB - returned reference must be released
 */
rc_t VTableWriteCursorOpenParentUpdate ( VTableCursor *self, VTable **tbl )
{
    rc_t rc;

    if ( tbl == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
        else if ( self -> tbl -> read_only )
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcTable, rcReadonly );
        else
        {
            rc = VTableAddRef ( self -> tbl );
            if ( rc == 0 )
            {
                * tbl = self -> tbl;
                return 0;
            }
        }

        * tbl = NULL;
    }

    return rc;
}

rc_t VTableWriteCursorInstallTrigger ( struct VTableCursor * self, struct VProduction * prod )
{
    return VectorAppend ( & self -> trig, NULL, prod );
}
