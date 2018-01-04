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
#include <bitstr.h>

typedef struct VViewCursor VViewCursor;
#define VCURSOR_IMPL VViewCursor
#include "cursor-struct.h"

#include "column-priv.h"
#include "schema-priv.h"
#include "table-priv.h"
#include "phys-priv.h"
#include "prod-priv.h"
#include "schema-parse.h"
#include "prod-expr.h"

#include <sysalloc.h>
#include <stdio.h>

struct VViewCursor
{
    struct VCursor_vt * vt;

    /* row id */
    int64_t row_id;

    /* half-closed page range */
    int64_t start_id;
    int64_t end_id;

    /* attached reference to view */
    const VView * view;

    /* user data */
    void *user;
    void ( CC * user_whack ) ( void *data );

    /* external row of VColumn* by ord ( owned ) */
    Vector row;

    /* column objects by cid ( not-owned ) */
    VCursorCache col;

    /* physical columns by cid ( owned ) */
    VCursorCache phys;

    /* productions by cid ( not-owned ) */
    VCursorCache prod;

    /* intermediate productions ( owned ) */
    Vector owned;

    KRefcount refcount;

    /* foreground state */
    uint8_t state;
};

static rc_t VViewCursorAddRef ( const VCURSOR_IMPL *self );
static rc_t VViewCursorRelease ( const VCURSOR_IMPL *self );
static rc_t VViewCursorVAddColumn ( const VCURSOR_IMPL *cself, uint32_t *idx, const char *name, va_list args );
static rc_t VViewCursorVGetColumnIdx ( const VCURSOR_IMPL *self, uint32_t *idx, const char *name, va_list args );
static rc_t VViewCursorDatatype ( const VCURSOR_IMPL * p_self, uint32_t p_idx, struct VTypedecl * p_type, struct VTypedesc * p_desc );
static rc_t VViewCursorOpen ( const VCURSOR_IMPL * p_self );
static rc_t VViewCursorIdRange ( const VCURSOR_IMPL *self, uint32_t idx, int64_t *first, uint64_t *count );

static VCursorCache * VViewCursorColumns ( VCURSOR_IMPL * self );
static VCursorCache * VViewCursorPhysicalColumns ( VCURSOR_IMPL * self );
static rc_t VViewCursorMakeColumn ( VCURSOR_IMPL *self, VColumn **col, const SColumn *scol, Vector *cx_bind );
static Vector * VViewCursorGetRow ( struct VCURSOR_IMPL * p_self );
static const struct VTable * VViewCursorGetTable ( const VCURSOR_IMPL * self );
static bool VViewCursorIsReadOnly ( const VCURSOR_IMPL * self );
static const struct VSchema * VViewCursorGetSchema ( const VCURSOR_IMPL * p_self );
static VBlobMRUCache * VViewCursorGetBlobMruCache ( struct VCURSOR_IMPL * p_self );
static uint32_t VViewCursorIncrementPhysicalProductionCount ( struct VCURSOR_IMPL * p_self );
static rc_t VViewCursorRowId ( const VCURSOR_IMPL *self, int64_t *id );
static rc_t VViewCursorSetRowId ( const VCURSOR_IMPL * p_self, int64_t p_id );
static rc_t VViewCursorFindNextRowId ( const VCURSOR_IMPL *self, uint32_t idx, int64_t *next );
static rc_t VViewCursorFindNextRowIdDirect ( const VCURSOR_IMPL *self, uint32_t idx, int64_t start_id, int64_t *next );
static rc_t VViewCursorOpenRow ( const VCURSOR_IMPL *cself );

static rc_t VViewCursorWrite ( VCURSOR_IMPL *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count );
static rc_t VViewCursorCommitRow ( VCURSOR_IMPL *self );
static rc_t VViewCursorRepeatRow ( VCURSOR_IMPL *self, uint64_t count );
static rc_t VViewCursorFlushPage ( VCURSOR_IMPL *self );
static rc_t VViewCursorDefault ( VCURSOR_IMPL *self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len );
static rc_t VViewCursorCommit ( VCURSOR_IMPL *self );
static rc_t VViewCursorOpenParentUpdate ( VCURSOR_IMPL *self, VTable **tbl );
static rc_t VViewCursorCloseRow ( const VCURSOR_IMPL *cself );
static rc_t VViewCursorGetBlob ( const VCURSOR_IMPL * p_self, const VBlob ** p_blob, uint32_t p_col_idx );
static rc_t VViewCursorGetBlobDirect ( const VCURSOR_IMPL *self, const VBlob **blob, int64_t row_id, uint32_t col_idx );
static rc_t VViewCursorRead ( const VCURSOR_IMPL *self, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
static rc_t VViewCursorReadDirect ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );
static rc_t VViewCursorReadBits ( const VCURSOR_IMPL * p_self, uint32_t p_col_idx, uint32_t p_elem_bits, uint32_t p_start, void * p_buffer, uint32_t p_off, uint32_t p_blen, uint32_t * p_num_read, uint32_t * p_remaining );
static rc_t VViewCursorReadBitsDirect ( const VCURSOR_IMPL * p_self, int64_t p_row_id, uint32_t p_col_idx, uint32_t p_elem_bits, uint32_t p_start, void * p_buffer, uint32_t p_off, uint32_t p_blen, uint32_t * p_num_read, uint32_t * p_remaining );
static rc_t VViewCursorCellData ( const VCURSOR_IMPL *self, uint32_t col_idx, uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
static rc_t VViewCursorCellDataDirect ( const VCURSOR_IMPL *self, int64_t row_id, uint32_t col_idx,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );
static rc_t VViewCursorDataPrefetch ( const VCURSOR_IMPL *cself, const int64_t *row_ids, uint32_t col_idx, uint32_t num_rows, int64_t min_valid_row_id, int64_t max_valid_row_id, bool continue_on_error );
static rc_t VViewCursorOpenParentRead ( const VCURSOR_IMPL * p_self, const VTable ** p_tbl );
static rc_t VViewCursorGetUserData ( const VCURSOR_IMPL * p_self, void ** p_data );
static rc_t VViewCursorSetUserData ( const VCURSOR_IMPL * p_self, void * p_data, void ( CC * p_destroy ) ( void *data ) );

static rc_t VViewCursorPermitPostOpenAdd ( const VCURSOR_IMPL * self );
static rc_t VViewCursorSuspendTriggers ( const VCURSOR_IMPL * self );
static rc_t VViewCursorPageIdRange ( const VCURSOR_IMPL *self, uint32_t idx, int64_t id, int64_t *first, int64_t *last );
static rc_t VViewCursorIsStaticColumn ( const VCURSOR_IMPL *self, uint32_t col_idx, bool *is_static );
static rc_t VViewCursorLinkedCursorGet ( const VCURSOR_IMPL *cself,const char *tbl, struct VCursor const **curs);
static rc_t VViewCursorLinkedCursorSet ( const VCURSOR_IMPL *cself,const char *tbl, struct VCursor const *curs);
static uint64_t VViewCursorSetCacheCapacity ( VCURSOR_IMPL *self,uint64_t capacity);
static uint64_t VViewCursorGetCacheCapacity ( const VCURSOR_IMPL *self);

static const KSymbol * VViewCursorFindOverride ( const VCURSOR_IMPL * p_self, const struct VCtxId * p_cid );

static rc_t VViewCursorLaunchPagemapThread ( struct VCURSOR_IMPL *self );
static const PageMapProcessRequest* VViewCursorPageMapProcessRequest ( const struct VCURSOR_IMPL *self );
static bool VViewCursorCacheActive ( const struct VCURSOR_IMPL * self, int64_t * cache_empty_end );
static rc_t VViewCursorInstallTrigger ( struct VCURSOR_IMPL * self, struct VProduction * prod );

static VCursor_vt VViewCursor_vt =
{
    VViewCursorAddRef,
    VViewCursorRelease,
    VViewCursorVAddColumn,
    VViewCursorVGetColumnIdx,
    VViewCursorDatatype,
    VViewCursorOpen,
    VViewCursorIdRange,
    VViewCursorRowId,
    VViewCursorSetRowId,
    VViewCursorFindNextRowId,
    VViewCursorFindNextRowIdDirect,
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
    VViewCursorGetUserData,
    VViewCursorSetUserData,

    VViewCursorPermitPostOpenAdd,
    VViewCursorSuspendTriggers,

    VViewCursorGetSchema,

    VViewCursorPageIdRange,
    VViewCursorIsStaticColumn,
    VViewCursorLinkedCursorGet,
    VViewCursorLinkedCursorSet,
    VViewCursorSetCacheCapacity,
    VViewCursorGetCacheCapacity,

    VViewCursorColumns,
    VViewCursorPhysicalColumns,
    VViewCursorMakeColumn,
    VViewCursorGetRow,
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
            curs -> vt = & VViewCursor_vt;
            curs -> view = p_view;
            VectorInit ( & curs -> row, 1, 16 );
            VCursorCacheInit ( & curs -> col, 0, 16 );
            VCursorCacheInit ( & curs -> phys, 0, 16 );
            VCursorCacheInit ( & curs -> prod, 0, 16 );
            VectorInit ( & curs -> owned, 0, 64 );
            KRefcountInit ( & curs -> refcount, 1, "VViewCursor", "make", "vcurs" );
            curs -> state = vcConstruct;
            * p_curs = curs;
            return 0;
        }
        free ( curs );
    }
    return rc;
}

rc_t
VViewCursorAddRef ( const VCURSOR_IMPL * p_self )
{
    if ( p_self != NULL )
    {
        switch ( KRefcountAdd ( & p_self -> refcount, "VViewCursor" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcCursor, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

static
void CC
VViewCursorVColumnWhack_checked( void *item, void *data )
{
    if ( item != NULL )
    {
        VColumnWhack( item, data );
    }
}

rc_t
VViewCursorWhack ( VViewCursor * p_self )
{
    VViewRelease ( p_self -> view );

    if ( p_self -> user_whack != NULL )
    {
        ( * p_self -> user_whack ) ( p_self -> user );
    }

    VectorWhack ( & p_self -> row, VViewCursorVColumnWhack_checked, NULL );
    VCursorCacheWhack ( & p_self -> col, NULL, NULL );
    VCursorCacheWhack ( & p_self -> col, NULL, NULL );
    VCursorCacheWhack ( & p_self -> phys, VPhysicalWhack, NULL );
    VCursorCacheWhack ( & p_self -> prod, NULL, NULL );
    VectorWhack ( & p_self -> owned, VProductionWhack, NULL );

    KRefcountWhack ( & p_self -> refcount, "VViewCursor" );

    free ( p_self );
    return 0;
}

rc_t
VViewCursorRelease ( const VCURSOR_IMPL * p_self )
{
    if ( p_self != NULL )
    {
        switch ( KRefcountDrop ( & p_self -> refcount, "VViewCursor" ) )
        {
        case krefWhack:
            return VViewCursorWhack ( ( VCURSOR_IMPL* ) p_self );
        case krefNegative:
            return RC ( rcVDB, rcCursor, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
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
    col = VCursorCacheGet ( & p_self -> col, & p_scol -> cid );
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
            if ( self -> state >= vcReady || scol -> td . type_id == 0 )
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
                rc = VectorAppend ( & p_self -> row, & col -> ord, col );
                if ( rc == 0 )
                {
                    void * ignore;
                    rc = VCursorCacheSet ( & p_self -> col, & p_scol -> cid, col );
                    if ( rc == 0 )
                    {
                        * p_idx = col -> ord;
                        return 0;
                    }
                    VectorSwap ( & p_self -> row, col -> ord, NULL, & ignore );
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
VViewCursorVAddColumn ( const VCURSOR_IMPL *    p_self,
                        uint32_t *              p_idx,
                        const char *            p_name,
                        va_list                 p_args )
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
            rc = VViewCursorAddColspec ( ( VCURSOR_IMPL * ) p_self, p_idx, colspec );
        }
    }
    return rc;
}

static
rc_t
VViewCursorGetColspec ( const VViewCursor * p_self, uint32_t * p_idx, const char * p_colspec )
{
    rc_t rc;

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
                                      "VViewCursorGetColspec",
                                      true );
    if ( scol == NULL || type != eColumn )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
    }
    else
    {
        /* if the column-spec gave us the exact column, return it */
        VColumn *col = VCursorCacheGet ( & p_self -> col, & scol -> cid );
        if ( col != NULL )
        {
            * p_idx = col -> ord;
            return 0;
        }

        /* prepare for failure */
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );

        /* there should be a name overload object */
        if ( name != NULL )
        {
            uint32_t count, i = VectorStart ( & name -> items );
            uint32_t end = VectorLength ( & name -> items );
            for ( end += i, count = 0; i < end; ++ i )
            {
                scol = ( const void* ) VectorGet ( & name -> items, i );
                if ( scol != NULL )
                {
                    col = VCursorCacheGet ( & p_self -> col, & scol -> cid );
                    if ( col != NULL )
                    {
                        * p_idx = col -> ord;
                        ++ count;
                    }
                }
            }

            if ( count == 1 )
            {
                return 0;
            }
            if ( count != 0 )
            {
                return RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcAmbiguous );
            }
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
VViewCursorVGetColumnIdx ( const VCURSOR_IMPL * p_self,
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
        else if ( p_self -> state == vcFailed )
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
                rc = VViewCursorGetColspec ( p_self, p_idx, colspec );
            }
        }
    }

    return rc;
}

/* Datatype
 *  returns typedecl and/or typedef for column data
 *
 *  "idx" [ IN ] - column index
 *
 *  "type" [ OUT, NULL OKAY ] - returns the column type declaration
 *
 *  "def" [ OUT, NULL OKAY ] - returns the definition of the type
 *  returned in "type_decl"
 *
 * NB - one of "type" and "def" must be non-NULL
 */
rc_t
VViewCursorDatatype ( const VCURSOR_IMPL *  p_self,
                      uint32_t              p_idx,
                      struct VTypedecl *    p_type,
                      struct VTypedesc *    p_desc )
{
    rc_t rc;

    if ( p_type == NULL && p_desc == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        const VColumn * vcol = ( const VColumn* ) VectorGet ( & p_self -> row, p_idx );
        if ( vcol != NULL )
        {
            return VColumnDatatype ( vcol, p_type, p_desc );
        }
        else
        {
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
            if ( p_type != NULL )
            {
                memset ( p_type, 0, sizeof * p_type );
            }
            if ( p_desc != NULL )
            {
                memset ( p_desc, 0, sizeof * p_desc );
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
        VViewCursor * self = (VViewCursor *) pb -> pr . curs;;
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
        VectorSwap ( & self -> row, col -> ord, NULL, & ignore );
        VCursorCacheSwap ( & self -> col, & scol -> cid, NULL, & ignore );

        /* dump the VColumn */
        VColumnWhack ( col, NULL );

        /* return no-error */
        pb -> rc = 0;
    }

    return false;
}

static
rc_t
VViewCursorResolveColumnProductions ( VCURSOR_IMPL * p_self, const struct KDlset * p_libs )
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
    pb . pr . cache     = & p_self -> prod;
    pb . pr . owned     = & p_self -> owned;
    pb . pr . cx_bind   = & cx_bind;

    pb . pr . chain     = chainDecoding;
    pb . pr . blobbing  = false;
    pb . pr . ignore_column_errors = false;
    pb . pr . discover_writable_columns = false;
    pb . rc = 0;

    VectorInit ( & cx_bind, 1, p_self -> view -> schema -> num_indirect );

    if ( ! VectorDoUntil ( & p_self -> row, false, VViewCursorResolveColumn, & pb ) )
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

    if ( p_self -> state >= vcReady )
    {
        rc = 0;
    }
    else if ( p_self -> state == vcFailed )
    {
        rc = RC ( rcVDB, rcCursor, rcOpening, rcCursor, rcInvalid );
    }
    else
    {
        rc = VViewCursorResolveColumnProductions ( p_self, p_libs );
        if ( rc == 0 )
        {
            p_self -> row_id = p_self -> start_id = p_self -> end_id = 1;
            p_self -> state = vcReady;
            return rc;
        }
        p_self -> state = vcFailed;
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
VViewCursorOpen ( const VCURSOR_IMPL * p_self )
{
    rc_t rc = 0;
    VViewCursor * self = ( VCURSOR_IMPL * ) p_self;
    struct KDlset *libs = 0;
#if 0
TODO: enable
    VLinker *ld = self -> tbl -> linker;

    rc_t rc = VLinkerOpen ( ld, & libs );
    if ( rc == 0 )
    {
#endif
        rc = VViewCursorOpenRead ( self, libs );
        if ( rc == 0 )
        {
            int64_t first;
            uint64_t count;

            rc = VViewCursorIdRange ( p_self, 0, & first, & count );
            if ( rc == 0 )
            {
                if ( count != 0 )
                {
                    /* set initial row id to starting row */
                    self -> start_id = self -> end_id = self -> row_id = first;
                }
            }
            else
            {
                self -> state = vcFailed;
            }
        }
#if 0
        KDlsetRelease ( libs );
    }
#endif

    return rc;
}

typedef struct VCursorIdRangeData VCursorIdRangeData;
struct VCursorIdRangeData
{
    int64_t first, last;
    rc_t rc;
};

static
bool CC
column_id_range ( void *item, void *data )
{
    if ( ( size_t ) item > 8 )
    {
        int64_t first, last;
        VCursorIdRangeData *pb = data;

        rc_t rc = VColumnIdRange ( ( const void* ) item, & first, & last );

        if ( GetRCState ( rc ) == rcEmpty )
            return false;

        if ( ( pb -> rc = rc ) != 0 )
            return true;

        if ( first < pb -> first )
            pb -> first = first;
        if ( last > pb -> last )
            pb -> last = last;
    }

    return false;
}

rc_t CC
VViewCursorIdRange ( const VCURSOR_IMPL *   p_self,
                     uint32_t               p_idx,
                     int64_t *              p_first,
                     uint64_t *             p_count )
{
    rc_t rc;
    VViewCursor * self = ( VCURSOR_IMPL * ) p_self;

    if ( p_first == NULL && p_count == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        int64_t dummy;
        uint64_t dummy_count;

        if ( p_first == NULL )
        {
            p_first = & dummy;
        }
        else if ( p_count == NULL )
        {
            p_count = & dummy_count;
        }
        if ( self -> state < vcReady )
        {
            if ( self -> state == vcFailed )
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcInvalid );
            }
            else
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcNotOpen );
            }
        }
        else if ( p_idx == 0 )
        {
            VCursorIdRangeData pb;

            pb . first = INT64_MAX;
            pb . last = INT64_MIN;
            pb . rc = SILENT_RC ( rcVDB, rcCursor, rcAccessing, rcRange, rcEmpty );

            if ( ! VectorDoUntil ( & self -> row, false, column_id_range, & pb ) )
            {
                * p_first = pb . first;
                * p_count = pb . last >= pb . first ? pb . last + 1 - pb . first : 0;
                return pb . rc;
            }

            rc = pb . rc;
        }
        else
        {
            const VColumn *vcol = ( const VColumn* ) VectorGet ( & self -> row, p_idx );
            if ( vcol == NULL )
            {
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
            }
            else
            {
                int64_t last;

                rc = VColumnIdRange ( vcol, p_first, & last );
                if  (rc == 0 )
                {
                    * p_count = last + 1 - * p_first;
                }
                return rc;
            }
        }

        * p_first = 0;
        * p_count = 0;
    }

    return rc;
}

VCursorCache *
VViewCursorColumns ( VCURSOR_IMPL * p_self )
{
    return & p_self -> col;
}

VCursorCache *
VViewCursorPhysicalColumns ( VCURSOR_IMPL * p_self )
{
    return & p_self -> phys;
}

rc_t
VViewCursorMakeColumn ( VCURSOR_IMPL * p_self, VColumn ** p_col, const SColumn * p_scol, Vector * p_cx_bind )
{
    return VColumnMake ( p_col, p_self -> view -> schema, p_scol );
}

Vector *
VViewCursorGetRow ( VCURSOR_IMPL * p_self )
{
    return & p_self -> row;
}

const VTable *
VViewCursorGetTable ( const VCURSOR_IMPL * p_self )
{
    return VViewPrimaryTable ( p_self -> view );
}

bool
VViewCursorIsReadOnly ( const VCURSOR_IMPL * p_self )
{
    return true;
}

const struct VSchema *
VViewCursorGetSchema ( const VCURSOR_IMPL * p_self )
{
    return p_self -> view -> schema;
}

VBlobMRUCache *
VViewCursorGetBlobMruCache ( struct VCURSOR_IMPL * p_self )
{   /* no blob cache for views, yet */
    return NULL;
}

uint32_t
VViewCursorIncrementPhysicalProductionCount ( struct VCURSOR_IMPL * p_self )
{
    return 0;
}

rc_t
VViewCursorRowId ( const VCURSOR_IMPL * p_self, int64_t * p_id )
{
    if ( p_id == NULL )
    {
        return RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        * p_id = p_self -> row_id;
        return 0;
    }
}

rc_t
VViewCursorSetRowId ( const VCURSOR_IMPL * p_self, int64_t p_id )
{
    VViewCursor * self = ( VCURSOR_IMPL * ) p_self;
    self -> row_id = p_id;
    return 0;
}

rc_t
VViewCursorFindNextRowId ( const VCURSOR_IMPL * p_self, uint32_t p_idx, int64_t * p_next )
{
    if ( p_next == NULL )
    {
        return RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        return VCursorRowFindNextRowId ( & p_self -> row, p_idx, p_self -> row_id + 1, p_next );
    }
}

rc_t
VViewCursorFindNextRowIdDirect ( const VCURSOR_IMPL *  p_self,
                                 uint32_t              p_idx,
                                 int64_t               p_start_id,
                                 int64_t *             p_next )
{
    if ( p_next == NULL )
    {
        return RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        return VCursorRowFindNextRowId ( & p_self -> row, p_idx, p_start_id, p_next );
    }
}

rc_t
VViewCursorOpenRow ( const VCURSOR_IMPL * p_self )
{
    rc_t rc;
    VViewCursor * self = ( VCURSOR_IMPL * ) p_self;

    if ( self -> state < vcReady )
    {
        rc = RC ( rcVDB, rcCursor, rcOpening, rcRow, rcIncomplete );
    }
    else if ( self -> state > vcReady )
    {   /* ignore */
        rc = 0;
    }
    else
    {
        self -> state = vcRowOpen;
        rc = 0;
    }

    return rc;
}

rc_t
VViewCursorCloseRow ( const VCURSOR_IMPL * p_self )
{
    VViewCursor * self = ( VCURSOR_IMPL * ) p_self;

    if ( self -> state == vcRowOpen )
    {
        ++ self -> row_id;
        self -> state = vcReady;
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
    const VColumn *col = ( const VColumn* ) VectorGet ( & p_self -> row, p_col_idx );
    if ( col == NULL )
    {
        return RC ( rcVDB, rcCursor, rcReading, rcColumn, rcInvalid );
    }
    else
    {
        const void * base;
        uint32_t elem_bits, boff, row_len;
        VBlob * blob;
        rc_t rc = VColumnRead ( col, p_self -> row_id, & elem_bits, & base, & boff, & row_len, & blob );
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
VViewCursorGetBlob ( const VCURSOR_IMPL * p_self, const VBlob ** p_blob, uint32_t p_col_idx )
{
    rc_t rc;

    if ( p_blob == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        switch ( p_self -> state )
        {
        case vcConstruct:
            rc = RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
            break;
        case vcReady:
            rc = RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
            break;
        case vcRowOpen:
            return VViewCursorGetBlobDirectInt ( p_self, p_blob, p_self -> row_id, p_col_idx );
        default:
            rc = RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
            break;
        }

        * p_blob = NULL;
    }
    return rc;
}

rc_t
VViewCursorGetBlobDirect ( const VCURSOR_IMPL * p_self,
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
        switch ( p_self -> state )
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

/*TODO: there is a copy of this in column-cmn.c, share */
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
CopyRow ( uint32_t      p_elem_size,
          uint32_t      p_elem_bits,
          const void *  p_src,
          uint32_t      p_src_off,
          void *        p_dest,
          uint32_t      p_dest_len,
          uint32_t *    p_row_len )
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
            memcpy ( p_dest, p_src, ( size_t ) ( to_read >> 3 ) );
        }
    }
    return rc;
}

static
rc_t
VViewCursorReadInt ( const VCURSOR_IMPL * p_self,
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
            const VColumn * col = ( const VColumn * ) VectorGet ( & p_self -> row, p_col_idx );
            if ( col != NULL )
            {
                uint32_t elem_size;
                const void *base;
                uint32_t boff;
                rc = VColumnRead ( col, p_row_id, & elem_size, & base, & boff, p_row_len, NULL );
                if ( rc == 0 )
                {
                    return CopyRow ( elem_size, p_elem_bits, base, boff, p_buffer, p_blen, p_row_len );
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
VViewCursorRead ( const VCURSOR_IMPL * p_self,
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
        switch ( p_self -> state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
            return RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
        case vcRowOpen:
            return VViewCursorReadInt ( p_self, p_self -> row_id, p_col_idx, p_elem_bits, p_buffer, p_blen, p_row_len);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

static rc_t VViewCursorReadDirect ( const VCURSOR_IMPL *    p_self,
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
        switch ( p_self -> state )
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
VViewCursorReadBitsInt ( const VCURSOR_IMPL *  p_self,
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

        col = ( const VColumn * ) VectorGet ( & p_self -> row, p_col_idx );
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
VViewCursorReadBits ( const VCURSOR_IMPL *    p_self,
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

        switch ( p_self -> state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
            return RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
        case vcRowOpen:
            return VViewCursorReadBitsInt ( p_self, p_self -> row_id, p_col_idx, p_elem_bits, p_start, p_buffer, p_off, p_blen, p_num_read, p_remaining);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

rc_t
VViewCursorReadBitsDirect ( const VCURSOR_IMPL *    p_self,
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

        switch ( p_self -> state )
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
VViewCursorCellDataInt ( const VCURSOR_IMPL *  p_self,
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

    col = ( const VColumn * ) VectorGet ( & p_self -> row, p_col_idx );
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
VViewCursorCellData ( const VCURSOR_IMPL *  p_self,
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

        switch ( p_self -> state )
        {
        case vcConstruct:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcNotOpen );
        case vcReady:
            return RC ( rcVDB, rcCursor, rcReading, rcRow, rcNotOpen );
        case vcRowOpen:
            return VViewCursorCellDataInt ( p_self, p_self -> row_id, p_col_idx, p_elem_bits, p_base, p_boff, p_row_len);
        default:
            return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcInvalid );
        }
    }
}

rc_t
VViewCursorCellDataDirect ( const VCURSOR_IMPL *    p_self,
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

        switch ( p_self -> state )
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
VViewCursorDataPrefetch ( const VCURSOR_IMPL *  p_cself,
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
VViewCursorOpenParentRead ( const VCURSOR_IMPL * p_self, const VTable ** p_tbl )
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

rc_t
VViewCursorGetUserData ( const VCURSOR_IMPL * p_self, void ** p_data )
{
    rc_t rc;

    if ( p_data == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        * p_data = p_self -> user;
        return 0;
    }

    return rc;
}

rc_t
VViewCursorSetUserData ( const VCURSOR_IMPL *   p_self,
                         void *                 p_data,
                         void ( CC * p_destroy ) ( void *data ) )
{
    VCURSOR_IMPL * self = ( VCURSOR_IMPL* ) p_self;

    self -> user = p_data;
    self -> user_whack = p_destroy;

    return 0;
}

const KSymbol * VViewCursorFindOverride ( const VCURSOR_IMPL * p_self, const struct VCtxId * p_cid )
{
    return SViewFindOverride ( p_self -> view -> sview, p_cid );
}

rc_t VViewCursorPermitPostOpenAdd ( const VCURSOR_IMPL * self )
{
    return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported );
}

rc_t VViewCursorPageIdRange ( const VCURSOR_IMPL *self, uint32_t idx, int64_t id, int64_t *first, int64_t *last )
{   /*TODO: body identical to VTableCursorPageIdRange, reuse */
    rc_t rc;

    if ( first == NULL && last == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        int64_t dummy;
        if ( first == NULL )
            first = & dummy;
        else if ( last == NULL )
            last = & dummy;

        if ( self == NULL )
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
        else
        {
            const VColumn *vcol = ( const VColumn* ) VectorGet ( & self -> row, idx );
            if ( vcol == NULL )
                rc = RC ( rcVDB, rcCursor, rcAccessing, rcColumn, rcNotFound );
            else
                return VColumnPageIdRange ( vcol, id, first, last );
        }

        * first = * last = 0;
    }

    return rc;
}

rc_t VViewCursorIsStaticColumn ( const VCURSOR_IMPL *self, uint32_t col_idx, bool *is_static )
{   /*TODO: body identical to VTableCursorIsStaticColumn, reuse */
    rc_t rc;

    if ( is_static == NULL )
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
        else
        {
            uint32_t start = VectorStart ( & self -> row );
            uint32_t end = start + VectorLength ( & self -> row );
            if ( col_idx < start || col_idx >= end )
                rc = RC ( rcVDB, rcCursor, rcSelecting, rcId, rcInvalid );
            else
            {
                VColumn *col = VectorGet ( & self -> row, col_idx );
                return VColumnIsStatic ( col, is_static );
            }
        }

        * is_static = false;
    }

    return rc;
}

rc_t VViewCursorLinkedCursorGet ( const VCURSOR_IMPL *cself,const char *tbl, struct VCursor const **curs)
{
    return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported );
}
rc_t VViewCursorLinkedCursorSet ( const VCURSOR_IMPL *cself,const char *tbl, struct VCursor const *curs)
{
    return RC ( rcVDB, rcCursor, rcReading, rcCursor, rcUnsupported );
}

uint64_t VViewCursorSetCacheCapacity ( VCURSOR_IMPL *self,uint64_t capacity)
{
    return 0;
}
uint64_t VViewCursorGetCacheCapacity ( const VCURSOR_IMPL *self)
{
    return 0;
}

rc_t VViewCursorLaunchPagemapThread ( struct VCURSOR_IMPL *self )
{
    return 0;
}
const PageMapProcessRequest* VViewCursorPageMapProcessRequest ( const struct VCURSOR_IMPL *self )
{
    return NULL;
}

bool VViewCursorCacheActive ( const struct VCURSOR_IMPL * self, int64_t * cache_empty_end )
{
    if ( cache_empty_end != NULL )
    {
        * cache_empty_end = 0;
    }
    return false;
}

// not implemented for views (read-only)
rc_t VViewCursorWrite ( VCURSOR_IMPL * p_self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t count )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorCommitRow ( VCURSOR_IMPL * p_self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorRepeatRow ( VCURSOR_IMPL * p_self, uint64_t count )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorFlushPage ( VCURSOR_IMPL * p_self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorDefault ( VCURSOR_IMPL * p_self, uint32_t col_idx, bitsz_t elem_bits, const void *buffer, bitsz_t boff, uint64_t row_len )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorCommit ( VCURSOR_IMPL * p_self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorOpenParentUpdate ( VCURSOR_IMPL * p_self, VTable **tbl )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorInstallTrigger ( struct VCURSOR_IMPL * self, struct VProduction * prod )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}
rc_t VViewCursorSuspendTriggers ( const VCURSOR_IMPL * self )
{
    return RC ( rcVDB, rcCursor, rcWriting, rcCursor, rcReadonly );
}

