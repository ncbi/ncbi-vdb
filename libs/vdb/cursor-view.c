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

#include "view-priv.h"

#include <klib/symbol.h>
#include <kfs/dyload.h>
#include <bitstr.h>

typedef struct VViewCursor VViewCursor;
#define VCURSOR_IMPL VViewCursor
#include "cursor-struct.h"

#include "column-priv.h"
#include "schema-priv.h"
#include "table-priv.h"
#include "phys-priv.h"
#include "prod-priv.h"
#include "linker-priv.h"
#include "schema-parse.h"
#include "prod-expr.h"

#include <sysalloc.h>
#include <stdio.h>

struct VViewCursor
{
    VCursor dad;

    /* attached reference to view */
    const VView * view;
};

static rc_t VViewCursorWhack ( const VViewCursor * p_self );
static rc_t VViewCursorVAddColumn ( const VViewCursor *cself, uint32_t *idx, const char *name, va_list args );
static rc_t VViewCursorVGetColumnIdx ( const VViewCursor *self, uint32_t *idx, const char *name, va_list args );
static rc_t VViewCursorOpen ( const VViewCursor * p_self );

static rc_t VViewCursorMakeColumn ( VViewCursor *self, VColumn **col, const SColumn *scol, Vector *cx_bind );
static const struct VTable * VViewCursorGetTable ( const VViewCursor * self );
static bool VViewCursorIsReadOnly ( const VViewCursor * self );
static const struct VSchema * VViewCursorGetSchema ( const VViewCursor * p_self );
static VBlobMRUCache * VViewCursorGetBlobMruCache ( struct VViewCursor * p_self );
static uint32_t VViewCursorIncrementPhysicalProductionCount ( struct VViewCursor * p_self );
static rc_t VViewCursorSetRowId ( const VViewCursor * p_self, int64_t p_id );
static rc_t VViewCursorOpenRow ( const VViewCursor *cself );

static rc_t VViewCursorWrite ( VViewCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count );
static rc_t VViewCursorCommitRow ( VViewCursor *self );
static rc_t VViewCursorRepeatRow ( VViewCursor *self, uint64_t count );
static rc_t VViewCursorFlushPage ( VViewCursor *self );
static rc_t VViewCursorDefault ( VViewCursor *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len );
static rc_t VViewCursorCommit ( VViewCursor *self );
static rc_t VViewCursorOpenParentUpdate ( VViewCursor *self, VTable **tbl );
static rc_t VViewCursorCloseRow ( const VViewCursor *cself );
static rc_t VViewCursorGetBlob ( const VViewCursor * p_self, const VBlob ** p_blob, uint32_t p_col_idx );
static rc_t VViewCursorGetBlobDirect ( const VViewCursor *self, const VBlob **blob, int64_t row_id, uint32_t col_idx );
static rc_t VViewCursorRead ( const VViewCursor *self, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
static rc_t VViewCursorReadDirect ( const VViewCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
static rc_t VViewCursorReadBits ( const VViewCursor * p_self, uint32_t p_col_idx, uint32_t p_elem_bits, uint32_t p_start, void * p_buffer, uint32_t p_off, uint32_t p_blen, uint32_t * p_num_read, uint32_t * p_remaining );
static rc_t VViewCursorReadBitsDirect ( const VViewCursor * p_self, int64_t p_row_id, uint32_t p_col_idx, uint32_t p_elem_bits, uint32_t p_start, void * p_buffer, uint32_t p_off, uint32_t p_blen, uint32_t * p_num_read, uint32_t * p_remaining );
static rc_t VViewCursorCellData ( const VViewCursor *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
static rc_t VViewCursorCellDataDirect ( const VViewCursor *self, int64_t row_id, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
static rc_t VViewCursorDataPrefetch ( const VViewCursor *cself, const int64_t *row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool continue_on_error );
static rc_t VViewCursorOpenParentRead ( const VViewCursor * p_self, const VTable ** p_tbl );

static rc_t VViewCursorPermitPostOpenAdd ( const VViewCursor * self );
static rc_t VViewCursorSuspendTriggers ( const VViewCursor * self );
static rc_t VViewCursorLinkedCursorGet ( const VViewCursor *cself,const char *tbl, struct VCursor const **curs);
static rc_t VViewCursorLinkedCursorSet ( const VViewCursor *cself,const char *tbl, struct VCursor const *curs);
static uint64_t VViewCursorSetCacheCapacity ( VViewCursor *self,uint64_t capacity);
static uint64_t VViewCursorGetCacheCapacity ( const VViewCursor *self);

static const KSymbol * VViewCursorFindOverride ( const VViewCursor * p_self, const struct VCtxId * p_cid );

static rc_t VViewCursorLaunchPagemapThread ( struct VViewCursor *self );
static const PageMapProcessRequest* VViewCursorPageMapProcessRequest ( const struct VViewCursor *self );
static bool VViewCursorCacheActive ( const struct VViewCursor * self, int64_t * cache_empty_end );
static rc_t VViewCursorInstallTrigger ( struct VViewCursor * self, struct VProduction * prod );

static VCursor_vt VViewCursor_vt =
{
    VViewCursorWhack,
    VViewCursorVAddColumn,
    VViewCursorVGetColumnIdx,
    VViewCursorOpen,
    VViewCursorSetRowId,
    VViewCursorOpenRow,
    VViewCursorWrite,
    VViewCursorCommitRow,
    VViewCursorCloseRow,
    VViewCursorRepeatRow,
    VViewCursorFlushPage,
    VViewCursorGetBlob,
    VViewCursorGetBlobDirect,
    VViewCursorRead,
    VViewCursorReadDirect,
    VViewCursorReadBits,
    VViewCursorReadBitsDirect,
    VViewCursorCellData,
    VViewCursorCellDataDirect,
    VViewCursorDataPrefetch,
    VViewCursorDefault,
    VViewCursorCommit,
    VViewCursorOpenParentRead,
    VViewCursorOpenParentUpdate,
    VViewCursorPermitPostOpenAdd,
    VViewCursorSuspendTriggers,
    VViewCursorGetSchema,
    VViewCursorLinkedCursorGet,
    VViewCursorLinkedCursorSet,
    VViewCursorSetCacheCapacity,
    VViewCursorGetCacheCapacity,
    VViewCursorMakeColumn,
    VViewCursorGetTable,
    VViewCursorIsReadOnly,
    VViewCursorGetBlobMruCache,
    VViewCursorIncrementPhysicalProductionCount,
    VViewCursorFindOverride,
    VViewCursorLaunchPagemapThread,
    VViewCursorPageMapProcessRequest,
    VViewCursorCacheActive,
    VViewCursorInstallTrigger
};

// VViewCursor

rc_t
VViewCursorMake ( const VView * p_view, VViewCursor ** p_curs )
{
    rc_t rc;
    VViewCursor * curs = calloc ( 1, sizeof * curs );
    if ( curs == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcConstructing, rcMemory, rcExhausted );
    }
    else
    {
        rc = VViewAddRef ( p_view );
        if ( rc == 0 )
        {
            curs -> dad . vt = & VViewCursor_vt;
            curs -> view = p_view;
            VectorInit ( & curs -> dad . row, 1, 16 );
            VCursorCacheInit ( & curs -> dad . col, 0, 16 );
            VCursorCacheInit ( & curs -> dad . phys, 0, 16 );
            VCursorCacheInit ( & curs -> dad . prod, 0, 16 );
            VectorInit ( & curs -> dad . owned, 0, 64 );
            KRefcountInit ( & curs -> dad . refcount, 1, "VViewCursor", "make", "vcurs" );
            curs -> dad . state = vcConstruct;
            * p_curs = curs;
            return 0;
        }
        free ( curs );
    }
    return rc;
}

rc_t
VViewCursorWhack ( const VViewCursor * p_self )
{
    VViewCursor * self = ( VViewCursor * ) p_self;
    VViewRelease ( self -> view );
    return VCursorWhackInt ( & self -> dad );
}

/* AddSColumn
 */
static
rc_t
VViewCursorAddSColumn ( VViewCursor *      p_self,
                        uint32_t *         p_idx,
                        const SColumn *    p_scol,
                        const VTypedecl *  p_cast,
                        Vector *           p_cx_bind )
{
    rc_t rc = 0;
    VColumn *col;

    /* must be readable */
    if ( p_scol -> read == NULL )
    {
        return RC ( rcVDB, rcCursor, rcUpdating, rcColumn, rcWriteonly );
    }

    /* must not already be there - benign error */
    col = VCursorCacheGet ( & p_self -> dad . col, & p_scol -> cid );
    if ( col != NULL )
    {
        * p_idx = col -> ord;
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcColumn, rcExists );
    }
    else
    {
        /* make object */
        rc = VColumnMake ( & col, p_self -> view -> schema, p_scol );
        if ( rc == 0 )
        {
#if 0
            /* open column if cursor open or type unknown */
            if ( self -> dad . state >= vcReady || scol -> td . type_id == 0 )
            {
                rc = RC ( rcVDB, rcCursor, rcUpdating, rcColumn, ?? );
            }
            else
            {   /* check cast of SColumn against requested type
                this is to handle the case where the column
                was created incomplete, i.e. with unknown type */
                if ( cast != NULL && | VTypedeclToTypedecl ( & scol -> td,
                        self -> schema, cast, & col -> td, NULL ) )
                {
                    rc = RC ( rcVDB, rcCursor, rcUpdating, rcColumn, ?? );
                }
            }
#endif

            if ( rc == 0 )
            {
                /* insert it into vectors */
                rc = VectorAppend ( & p_self -> dad . row, & col -> ord, col );
                if ( rc == 0 )
                {
                    void * ignore;
                    rc = VCursorCacheSet ( & p_self -> dad . col, & p_scol -> cid, col );
                    if ( rc == 0 )
                    {
                        * p_idx = col -> ord;
                        return 0;
                    }
                    VectorSwap ( & p_self -> dad . row, col -> ord, NULL, & ignore );
                }
            }
            VColumnWhack ( col, NULL );
        }
    }

    return rc;
}

/* AddColspec
 *  a "colspec" is either a simple column name or a typed name expression
 *  uses STable to evaluate colspec and find an SColumn
 */
static
rc_t
VViewCursorAddColspec ( VViewCursor * p_self, uint32_t * p_idx, const char * p_colspec )
{
    rc_t rc = 0;

    /* find an appropriate column in schema */
    uint32_t type;
    VTypedecl cast;
    const SNameOverload *name;
    const SColumn *scol = SViewFind ( p_self -> view -> sview,
                                      p_self -> view -> schema,
                                      & cast,
                                      & name,
                                      & type,
                                      p_colspec,
                                      "VViewCursorAddColspec",
                                      true );
    if ( scol == NULL || type != eColumn )
    {
        rc = SILENT_RC ( rcVDB, rcCursor, rcUpdating, rcColumn, rcNotFound );
    }
    else
    {
        Vector cx_bind;
        VectorInit ( & cx_bind, 1, p_self -> view -> schema -> num_indirect );
        rc = VViewCursorAddSColumn ( p_self, p_idx, scol, & cast, & cx_bind );
        VectorWhack ( & cx_bind, NULL, NULL );
    }

    return rc;
}

rc_t
VViewCursorVAddColumn ( const VViewCursor * p_self,
                        uint32_t *          p_idx,
                        const char *        p_name,
                        va_list             p_args )
{
    rc_t rc = 0;
    if ( p_idx == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcParam, rcNull );
    }
    else if ( p_name == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcName, rcNull );
    }
    else if ( p_name [ 0 ] == 0 )
    {
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcName, rcEmpty );
    }
    else if ( p_self -> dad . state != vcConstruct )
    {
        rc = RC ( rcVDB, rcCursor, rcUpdating, rcCursor, rcLocked );
    }
    else
    {
        char colspec [ 1024 ];
        int len = vsnprintf ( colspec, sizeof colspec, p_name, p_args );
        if ( len < 0 || len >= sizeof colspec )
        {
            rc = RC ( rcVDB, rcCursor, rcUpdating, rcName, rcExcessive );
        }
        else
        {
            * p_idx = 0;
            rc = VViewCursorAddColspec ( ( VViewCursor * ) p_self, p_idx, colspec );
        }
    }
    return rc;
}

/* GetColumnIdx
 *  retrieve column index by name spec
 *
 *  "idx" [ OUT ] - return parameter for column index
 *
 *  "name" [ IN ] - NUL terminated column name spec.
 */
rc_t
VViewCursorVGetColumnIdx ( const VViewCursor * p_self,
                           uint32_t *           p_idx,
                           const char *         p_name,
                           va_list args )
{
    rc_t rc;

    if ( p_idx == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        * p_idx = 0;

        if ( p_name == NULL )
        {
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcName, rcNull );
        }
        else if ( p_name [ 0 ] == 0 )
        {
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcName, rcEmpty );
        }
        else if ( p_self -> dad . state == vcFailed )
        {
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcInvalid );
        }
        else
        {
            char colspec [ 1024 ];
            int len = vsnprintf ( colspec, sizeof colspec, p_name, args );
            if ( len < 0 || len >= sizeof colspec )
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcName, rcExcessive );
            }
            else
            {
                /* find an appropriate column in schema */
                uint32_t type;
                VTypedecl cast;
                const SNameOverload *name;
                const SColumn *scol = SViewFind ( p_self -> view -> sview,
                                                p_self -> view -> schema,
                                                & cast,
                                                & name,
                                                & type,
                                                colspec,
                                                "VViewCursorVGetColumnIdx",
                                                true );
                rc = VCursorGetColidx ( & p_self -> dad, scol, name, type, p_idx );
            }
        }
    }

    return rc;
}

typedef struct VProdResolveData VProdResolveData;
struct VProdResolveData
{
    VProdResolve pr;
    rc_t rc;
};

static
bool CC
VViewCursorResolveColumn ( void *item, void *data )
{
    if ( item != NULL )
    {
        void *ignore;

        VColumn *col = item;
        SColumn *scol = ( SColumn* ) col -> scol;
        VProdResolveData *pb = data;
        VViewCursor * self = (VViewCursor *) pb -> pr . curs;
        VProduction * src = NULL;

        pb -> rc = VProdResolveColumnRoot ( & pb -> pr, & src, scol );
        if ( pb -> rc == 0 )
        {
            if ( src > FAILED_PRODUCTION )
            {
                /* repair for incomplete implicit column decl */
                if ( scol -> td . type_id == 0 )
                    scol -> td = src -> fd . td;

                return false;
            }

            pb -> rc = RC ( rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined );
        }

        /* check for tolerance */
        if ( ! pb -> pr . ignore_column_errors )
        {
            PLOGERR ( klogErr, ( klogErr, pb -> rc, "failed to resolve column '$(name)' idx '$(idx)'",
                                    "name=%.*s,idx=%u"
                                    , ( int ) scol -> name -> name . size
                                    , scol -> name -> name . addr
                                    , col -> ord ));
            return true;
        }

        /* remove from row and cache */
        VectorSwap ( & self -> dad . row, col -> ord, NULL, & ignore );
        VCursorCacheSwap ( & self -> dad . col, & scol -> cid, NULL, & ignore );

        /* dump the VColumn */
        VColumnWhack ( col, NULL );

        /* return no-error */
        pb -> rc = 0;
    }

    return false;
}

static
rc_t
VViewCursorResolveColumnProductions ( VViewCursor * p_self, const struct KDlset * p_libs )
{
    Vector cx_bind;
    VProdResolveData pb;
    const VTable * tbl = VViewCursorGetTable ( p_self );

    pb . pr . schema    = p_self -> view -> schema;
    pb . pr . ld        = tbl -> linker;
    pb . pr . libs      = p_libs;
    pb . pr . name      = & p_self -> view -> sview -> name -> name;
    pb . pr . primary_table = tbl;
    pb . pr . view      = p_self -> view;
    pb . pr . curs      = ( VCursor * ) p_self;
    pb . pr . cache     = & p_self -> dad . prod;
    pb . pr . owned     = & p_self -> dad . owned;
    pb . pr . cx_bind   = & cx_bind;

    pb . pr . chain     = chainDecoding;
    pb . pr . blobbing  = false;
    pb . pr . ignore_column_errors = false;
    pb . pr . discover_writable_columns = false;
    pb . rc = 0;

    VectorInit ( & cx_bind, 1, p_self -> view -> schema -> num_indirect );

    if ( ! VectorDoUntil ( & p_self -> dad . row, false, VViewCursorResolveColumn, & pb ) )
    {
        pb . rc = 0;
    }

    VectorWhack ( & cx_bind, NULL, NULL );

    return pb . rc;
}

static
rc_t
VViewCursorOpenRead ( VViewCursor * p_self, const struct KDlset * p_libs )
{
    rc_t rc;

    if ( p_self -> dad . state >= vcReady )
    {
        rc = 0;
    }
    else if ( p_self -> dad . state == vcFailed )
    {
        rc = RC ( rcVDB, rcCursor, rcOpening, rcCursor, rcInvalid );
    }
    else
    {
        rc = VViewCursorResolveColumnProductions ( p_self, p_libs );
        if ( rc == 0 )
        {
            p_self -> dad . row_id = p_self -> dad . start_id = p_self -> dad . end_id = 1;
            p_self -> dad . state = vcReady;
            return rc;
        }
        p_self -> dad . state = vcFailed;
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
rc_t
VViewCursorOpen ( const VViewCursor * p_self )
{
    VViewCursor * self = ( VViewCursor * ) p_self;
    struct KDlset *libs = 0;

    rc_t rc = VLinkerOpen ( self -> view -> linker, & libs );
    if ( rc == 0 )
    {
        rc = VViewCursorOpenRead ( self, libs );
        if ( rc == 0 )
        {
            int64_t first;
            uint64_t count;

            rc = VCursorIdRange ( & p_self -> dad, 0, & first, & count );
            if ( rc == 0 )
            {
                if ( count != 0 )
                {
                    /* set initial row id to starting row */
                    self -> dad . start_id =
                    self -> dad . end_id =
                    self -> dad . row_id = first;
                }
            }
            else
            {
                self -> dad . state = vcFailed;
            }
        }
        KDlsetRelease ( libs );
    }

    return rc;
}

rc_t
VViewCursorMakeColumn ( VViewCursor * p_self, VColumn ** p_col, const SColumn * p_scol, Vector * p_cx_bind )
{
    return VColumnMake ( p_col, p_self -> view -> schema, p_scol );
}

const VTable *
VViewCursorGetTable ( const VViewCursor * p_self )
{
    return VViewPrimaryTable ( p_self -> view );
}

bool
VViewCursorIsReadOnly ( const VViewCursor * p_self )
{
    return true;
}

const struct VSchema *
VViewCursorGetSchema ( const VViewCursor * p_self )
{
    return p_self -> view -> schema;
}

VBlobMRUCache *
VViewCursorGetBlobMruCache ( struct VViewCursor * p_self )
{   /* no blob cache for views, yet */
    return NULL;
}

uint32_t
VViewCursorIncrementPhysicalProductionCount ( struct VViewCursor * p_self )
{
    return 0;
}

rc_t
VViewCursorSetRowId ( const VViewCursor * p_self, int64_t p_id )
{
    VViewCursor * self = ( VViewCursor * ) p_self;
    return VCursorSetRowIdRead ( & self -> dad, p_id );
}

rc_t
VViewCursorOpenRow ( const VViewCursor * p_self )
{
    rc_t rc;
    VViewCursor * self = ( VViewCursor * ) p_self;

    if ( self -> dad . state < vcReady )
    {
        rc = RC ( rcVDB, rcCursor, rcOpening, rcRow, rcIncomplete );
    }
    else if ( self -> dad . state > vcReady )
    {   /* ignore */
        rc = 0;
    }
    else
    {
        rc = VCursorOpenRowRead ( & self -> dad );
    }

    return rc;
}

rc_t
VViewCursorCloseRow ( const VViewCursor * p_self )
{
    VViewCursor * self = ( VViewCursor * ) p_self;

    if ( self -> dad . state == vcRowOpen )
    {
        return VCursorCloseRowRead ( & self -> dad );
    }

    return 0;
}

static
rc_t
VViewCursorGetBlobDirectInt ( const VViewCursor *   p_self,
                              const VBlob **        p_blob,
                              int64_t               p_row_id,
                              uint32_t              p_col_idx )
{
    const VColumn *col = ( const VColumn* ) VectorGet ( & p_self -> dad . row, p_col_idx );
    if ( col == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcColumn, rcInvalid );
    }
    else
    {
        const void * base;
        uint32_t elem_bits, boff, row_len;
        VBlob * blob;
        rc_t rc = VColumnRead ( col, p_self -> dad . row_id, & elem_bits, & base, & boff, & row_len, & blob );
        if ( rc == 0 )
        {
            rc = VBlobAddRef ( blob );
            if ( rc == 0 )
            {
                * p_blob = blob;
                return 0;
            }
        }
        return rc;
    }
}

rc_t
VViewCursorGetBlob ( const VViewCursor * p_self, const VBlob ** p_blob, uint32_t p_col_idx )
{
    rc_t rc;

    if ( p_blob == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            rc = RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
            break;
        case vcReady:
            rc = RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
            break;
        case vcRowOpen:
            return VViewCursorGetBlobDirectInt ( p_self, p_blob, p_self -> dad . row_id, p_col_idx );
        default:
            rc = RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
            break;
        }

        * p_blob = NULL;
    }
    return rc;
}

rc_t
VViewCursorGetBlobDirect ( const VViewCursor * p_self,
                           const VBlob **  p_blob,
                           int64_t         p_row_id,
                           uint32_t        p_col_idx )
{
    rc_t rc;

    if ( p_blob == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            rc = RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
            break;
        case vcReady:
        case vcRowOpen:
            return VViewCursorGetBlobDirectInt ( p_self, p_blob, p_row_id, p_col_idx );
        default:
            rc = RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
            break;
        }

        * p_blob = NULL;
    }
    return rc;
}

static __inline__
bool
bad_elem_bits ( uint32_t elem_size, uint32_t elem_bits )
{
    if ( elem_size != elem_bits )
    {
        if ( elem_size < elem_bits && elem_bits % elem_size != 0 )
            return true;
        return ( elem_size % elem_bits != 0 );
    }
    return false;
}

static
rc_t
CopyCell ( uint32_t     p_elem_size,
           uint32_t     p_elem_bits,
           const void * p_src,
           uint32_t     p_src_off,
           void *       p_dest,
           uint32_t     p_dest_len,
           uint32_t *   p_row_len )
{
    rc_t rc = 0;
    if ( bad_elem_bits ( p_elem_size, p_elem_bits ) )
    {
        rc = RC ( rcVDB, rcCursor, rcReading, rcType, rcInconsistent );
    }
    else if ( * p_row_len != 0 )
    {
        if ( p_dest_len == 0 )
        {
            rc = RC ( rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient );
        }
        if ( p_dest == NULL )
        {
            rc = RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
        }
        else
        {
            uint64_t to_read = * p_row_len * p_elem_size;
            uint64_t bsize = p_dest_len * p_elem_bits;

            /* always return the required buffer size */
            * p_row_len = ( uint32_t ) ( to_read / p_elem_bits );

            /* detect buffer too small */
            if ( to_read > bsize )
            {
                rc = RC ( rcVDB, rcCursor, rcReading, rcBuffer, rcInsufficient );
                to_read = bsize;
            }

            /* copy out data up to limit */
            assert ( p_src_off == 0 );
            memmove ( p_dest, p_src, ( size_t ) ( to_read >> 3 ) );
        }
    }
    return rc;
}

static
rc_t
VViewCursorReadInt ( const VViewCursor *  p_self,
                     int64_t              p_row_id,
                     uint32_t             p_col_idx,
                     uint32_t             p_elem_bits,
                     void *               p_buffer,
                     uint32_t             p_blen,
                     uint32_t *           p_row_len )
{
    rc_t rc;

    if ( p_row_len == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        if ( p_elem_bits == 0 || ( p_elem_bits & 7 ) != 0 )
        {
            rc = RC ( rcVDB, rcCursor, rcReading, rcParam, rcInvalid );
        }
        else
        {
            const VColumn * col = ( const VColumn * ) VectorGet ( & p_self -> dad . row, p_col_idx );
            if ( col != NULL )
            {
                uint32_t elem_size;
                const void *base;
                uint32_t boff;
                rc = VColumnRead ( col, p_row_id, & elem_size, & base, & boff, p_row_len, NULL );
                if ( rc == 0 )
                {
                    return CopyCell ( elem_size, p_elem_bits, base, boff, p_buffer, p_blen, p_row_len );
                }
            }
            else
            {
                return RC ( rcVDB, rcCursor, rcReading, rcColumn, rcInvalid );
            }
        }
    }
    return rc;
}

rc_t
VViewCursorRead ( const VViewCursor * p_self,
                  uint32_t             p_col_idx,
                  uint32_t             p_elem_bits,
                  void *               p_buffer,
                  uint32_t             p_blen,
                  uint32_t *           p_row_len )
{
    if ( p_row_len == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        * p_row_len = 0;
        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
            return RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
        case vcRowOpen:
            return VViewCursorReadInt ( p_self, p_self -> dad . row_id, p_col_idx, p_elem_bits, p_buffer, p_blen, p_row_len);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

static rc_t VViewCursorReadDirect ( const VViewCursor *    p_self,
                                    int64_t                 p_row_id,
                                    uint32_t                p_col_idx,
                                    uint32_t                p_elem_bits,
                                    void *                  p_buffer,
                                    uint32_t                p_blen,
                                    uint32_t *              p_row_len )
{
    if ( p_row_len == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        * p_row_len = 0;
        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
        case vcRowOpen:
            return VViewCursorReadInt ( p_self, p_row_id, p_col_idx, p_elem_bits, p_buffer, p_blen, p_row_len);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

static
rc_t
CopyRowBits ( uint32_t      p_elem_size,
              uint32_t      p_elem_bits,
              uint32_t      p_start,
              const void *  p_src,
              uint32_t      p_src_off,
              void *        p_dest,
              uint32_t      p_off,
              uint32_t      p_dest_len,
              uint32_t *    p_num_read,
              uint32_t *    p_remaining )
{
    rc_t rc = 0;
    if ( bad_elem_bits ( p_elem_size, p_elem_bits ) )
    {
        rc = RC ( rcVDB, rcCursor, rcReading, rcType, rcInconsistent );
    }
    else if ( * p_num_read != 0 )
    {
        uint64_t to_read = * p_num_read * p_elem_size;
        uint64_t doff = p_start * p_elem_bits;
        to_read = to_read > doff ? to_read - doff : 0;
        if ( p_dest_len == 0 )
        {
            * p_num_read = 0;
            * p_remaining = ( uint32_t ) ( to_read / p_elem_bits );
            return 0;
        }

        if ( p_dest == NULL )
        {
            rc = RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
        }
        else
        {
            uint64_t bsize = p_dest_len * p_elem_size;
            if ( to_read <= bsize )
            {
                * p_remaining = 0;
            }
            else
            {
                * p_remaining = (uint32_t)( ( to_read - bsize ) / p_elem_bits );
                to_read = bsize;
            }
            bitcpy ( p_dest, p_off, p_src, p_src_off + doff, ( bitsz_t ) to_read );
            * p_num_read = ( uint32_t ) ( to_read / p_elem_bits );
            return 0;
        }
    }
    return rc;
}

static
rc_t
VViewCursorReadBitsInt ( const VViewCursor *  p_self,
                         uint32_t              p_row_id,
                         uint32_t              p_col_idx,
                         uint32_t              p_elem_bits,
                         uint32_t              p_start,
                         void *                p_buffer,
                         uint32_t              p_off,
                         uint32_t              p_blen,
                         uint32_t *            p_num_read,
                         uint32_t *            p_remaining )
{
    rc_t rc;

    if ( p_elem_bits == 0 )
    {
        rc = RC ( rcVDB, rcCursor, rcReading, rcParam, rcInvalid );
    }
    else
    {
        const VColumn * col;

        col = ( const VColumn * ) VectorGet ( & p_self -> dad . row, p_col_idx );
        if ( col != NULL )
        {
            uint32_t elem_size;
            const void *base;
            uint32_t boff;
            rc = VColumnRead ( col, p_row_id, & elem_size, & base, & boff, p_num_read, NULL );
            if ( rc == 0 )
            {
                return CopyRowBits ( elem_size, p_elem_bits, p_start, base, boff, p_buffer, p_off, p_blen, p_num_read, p_remaining );
            }
        }
        else
        {
            return RC ( rcVDB, rcCursor, rcReading, rcColumn, rcInvalid );
        }
    }
    return rc;
}

rc_t
VViewCursorReadBits ( const VViewCursor *    p_self,
                      uint32_t                p_col_idx,
                      uint32_t                p_elem_bits,
                      uint32_t                p_start,
                      void *                  p_buffer,
                      uint32_t                p_off,
                      uint32_t                p_blen,
                      uint32_t *              p_num_read,
                      uint32_t *              p_remaining )
{
    if ( p_num_read == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        uint32_t dummy;
        if ( p_remaining == NULL )
        {
            p_remaining = & dummy;
        }
        * p_num_read = 0;
        * p_remaining = 0;

        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
            return RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
        case vcRowOpen:
            return VViewCursorReadBitsInt ( p_self, p_self -> dad . row_id, p_col_idx, p_elem_bits, p_start, p_buffer, p_off, p_blen, p_num_read, p_remaining);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

rc_t
VViewCursorReadBitsDirect ( const VViewCursor *    p_self,
                            int64_t                 p_row_id,
                            uint32_t                p_col_idx,
                            uint32_t                p_elem_bits,
                            uint32_t                p_start,
                            void *                  p_buffer,
                            uint32_t                p_off,
                            uint32_t                p_blen,
                            uint32_t *              p_num_read,
                            uint32_t *              p_remaining )
{
    if ( p_num_read == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        uint32_t dummy;
        if ( p_remaining == NULL )
        {
            p_remaining = & dummy;
        }
        * p_num_read = 0;
        * p_remaining = 0;

        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
        case vcRowOpen:
            return VViewCursorReadBitsInt ( p_self, p_row_id, p_col_idx, p_elem_bits, p_start, p_buffer, p_off, p_blen, p_num_read, p_remaining);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

static
rc_t
VViewCursorCellDataInt ( const VViewCursor *  p_self,
                         int64_t               p_row_id,
                         uint32_t              p_col_idx,
                         uint32_t *            p_elem_bits,
                         const void **         p_base,
                         uint32_t *            p_boff,
                         uint32_t *            p_row_len )
{
    rc_t rc;
    const VColumn * col;

    uint32_t dummy [ 3 ];
    if ( p_row_len == NULL )
    {
        p_row_len = & dummy [ 0 ];
    }
    if ( p_boff == NULL )
    {
        p_boff = & dummy [ 1 ];
    }
    if ( p_elem_bits == NULL )
    {
        p_elem_bits = & dummy [ 2 ];
    }

    col = ( const VColumn * ) VectorGet ( & p_self -> dad . row, p_col_idx );
    if ( col != NULL )
    {
        rc = VColumnRead ( col, p_row_id, p_elem_bits, p_base, p_boff, p_row_len, NULL );
        if ( rc == 0 )
        {
            return 0;
        }
    }
    else
    {
        rc = RC ( rcVDB, rcCursor, rcReading, rcColumn, rcInvalid );
    }

    * p_elem_bits = 0;
    * p_boff = 0;
    * p_row_len = 0;

    return rc;
}

rc_t
VViewCursorCellData ( const VViewCursor *  p_self,
                      uint32_t              p_col_idx,
                      uint32_t *            p_elem_bits,
                      const void **         p_base,
                      uint32_t *            p_boff,
                      uint32_t *            p_row_len )
{
    if ( p_base == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        * p_base = NULL;

        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
            return RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
        case vcRowOpen:
            return VViewCursorCellDataInt ( p_self, p_self -> dad . row_id, p_col_idx, p_elem_bits, p_base, p_boff, p_row_len);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

rc_t
VViewCursorCellDataDirect ( const VViewCursor *    p_self,
                            int64_t                 p_row_id,
                            uint32_t                p_col_idx,
                            uint32_t *              p_elem_bits,
                            const void **           p_base,
                            uint32_t *              p_boff,
                            uint32_t *              p_row_len )
{
    if ( p_base == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcParam, rcNull );
    }
    else
    {
        * p_base = NULL;

        switch ( p_self -> dad . state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
        case vcRowOpen:
            return VViewCursorCellDataInt ( p_self, p_row_id, p_col_idx, p_elem_bits, p_base, p_boff, p_row_len);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

rc_t
VViewCursorDataPrefetch ( const VViewCursor *  p_cself,
                          const int64_t *       p_row_ids,
                          uint32_t              p_col_idx,
                          uint32_t              p_num_rows,
                          int64_t               p_min_valid_row_id,
                          int64_t               p_max_valid_row_id,
                          bool                  p_continue_on_error )
{
    return 0;
}

rc_t
VViewCursorOpenParentRead ( const VViewCursor * p_self, const VTable ** p_tbl )
{
    rc_t rc;

    if ( p_tbl == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        const VTable * tbl = VViewCursorGetTable ( p_self );
        rc = VTableAddRef ( tbl );
        if ( rc == 0 )
        {
            * p_tbl = tbl;
            return 0;
        }

        * p_tbl = NULL;
    }

    return rc;
}

const KSymbol * VViewCursorFindOverride ( const VViewCursor * p_self, const struct VCtxId * p_cid )
{
    return SViewFindOverride ( p_self -> view -> sview, p_cid );
}

rc_t VViewCursorPermitPostOpenAdd ( const VViewCursor * self )
{
    return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported );
}

rc_t VViewCursorLinkedCursorGet ( const VViewCursor *cself,const char *tbl, struct VCursor const **curs)
{
    return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported );
}
rc_t VViewCursorLinkedCursorSet ( const VViewCursor *cself,const char *tbl, struct VCursor const *curs)
{
    return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported );
}

uint64_t VViewCursorSetCacheCapacity ( VViewCursor *self,uint64_t capacity)
{
    return 0;
}
uint64_t VViewCursorGetCacheCapacity ( const VViewCursor *self)
{
    return 0;
}

rc_t VViewCursorLaunchPagemapThread ( struct VViewCursor *self )
{
    return 0;
}
const PageMapProcessRequest* VViewCursorPageMapProcessRequest ( const struct VViewCursor *self )
{
    return NULL;
}

bool VViewCursorCacheActive ( const struct VViewCursor * self, int64_t * cache_empty_end )
{
    if ( cache_empty_end != NULL )
    {
        * cache_empty_end = 0;
    }
    return false;
}

// not implemented for views (read-only)
rc_t VViewCursorWrite ( VViewCursor * p_self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorCommitRow ( VViewCursor * p_self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorRepeatRow ( VViewCursor * p_self, uint64_t count )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorFlushPage ( VViewCursor * p_self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorDefault ( VViewCursor * p_self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorCommit ( VViewCursor * p_self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorOpenParentUpdate ( VViewCursor * p_self, VTable **tbl )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorInstallTrigger ( struct VViewCursor * self, struct VProduction * prod )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorSuspendTriggers ( const VViewCursor * self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}

