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

#include <vdb/view.h>

#include <klib/refcount.h>
#include <klib/symbol.h>
#include <vdb/table.h>

#include "table-priv.h"
#include "column-priv.h"
#include "schema-parse.h"
#include "cursor-struct.h"

#include <sysalloc.h>
#include <stdio.h>

struct VView
{
    /* open references */
    KRefcount refcount;

    /* schema and view description */
    const SView *           sview;
    const struct VSchema  * schema;
};

typedef struct VViewCursor VViewCursor;
struct VViewCursor
{
    VCursor_vt * vt;

    /* attached reference to view */
    const VView * view;

    /* external row of VColumn* by ord ( owned ) */
    Vector row;

    /* column objects by cid ( not-owned ) */
    VCursorCache col;

    KRefcount refcount;
};

static rc_t VViewCursorAddRef ( const VCursor *self );
static rc_t VViewCursorRelease ( const VCursor *self );
static rc_t VViewCursorVAddColumn ( const VCursor *cself, uint32_t *idx, const char *name, va_list args );
static rc_t VViewCursorVGetColumnIdx ( const VCursor *self, uint32_t *idx, const char *name, va_list args );
static rc_t VViewCursorDatatype ( const VCursor * p_self, uint32_t p_idx, struct VTypedecl * p_type, struct VTypedesc * p_desc );
static rc_t VViewCursorOpen ( const VCursor * p_self );

static VCursor_vt VViewCursor_vt =
{
    VViewCursorAddRef,
    VViewCursorRelease,
    VViewCursorVAddColumn,
    VViewCursorVGetColumnIdx,
    VViewCursorDatatype,
    VViewCursorOpen,
    // VViewCursorIdRange,
    // VViewCursorRowId,
    // VViewCursorSetRowId,
    // VViewCursorFindNextRowId,
    // VViewCursorFindNextRowIdDirect,
    // VViewCursorOpenRow,
    // VViewCursorWrite,
    // VViewCursorCommitRow,
    // VViewCursorCloseRow,
    // VViewCursorRepeatRow,
    // VViewCursorFlushPage,
    // VViewCursorGetBlob,
    // VViewCursorGetBlobDirect,
    // VViewCursorRead,
    // VViewCursorReadDirect,
    // VViewCursorReadBits,
    // VViewCursorReadBitsDirect,
    // VViewCursorCellData,
    // VViewCursorCellDataDirect,
    // VViewCursorDataPrefetch,
    // VViewCursorDefault,
    // VViewCursorCommit,
    // VViewCursorOpenParentRead,
    // VViewCursorOpenParentUpdate,
    // VViewCursorGetUserData,
    // VViewCursorSetUserData
};

rc_t
VCursorMakeFromView  ( const VCursor ** p_cursp, const struct VView * p_view )
{
    rc_t rc;
    VViewCursor *curs;

    /* must have return param */
    assert ( p_cursp != NULL );

    /* must have parent tbl */
    assert ( p_view != NULL );

    /* create a structure */
    curs = calloc ( 1, sizeof * curs );
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
            KRefcountInit ( & curs -> refcount, 1, "VCursor", "make", "vcurs" );
            * p_cursp = ( const VCursor * ) curs;
            return 0;
        }
    }

    * p_cursp = NULL;

    return rc;
}
/* Whack
 */
static
rc_t
VViewWhack ( VView *self )
{
    KRefcountWhack ( & self -> refcount, "VView" );
#if 0
    BSTreeWhack ( & self -> read_col_cache, VColumnRefWhack, NULL );
    BSTreeWhack ( & self -> write_col_cache, VColumnRefWhack, NULL );
    VTableRelease(self -> cache_tbl);

    KMDataNodeRelease ( self -> col_node );
    KMetadataRelease ( self -> meta );
    KTableRelease ( self -> ktbl );
    VSchemaRelease ( self -> schema );
    VLinkerRelease ( self -> linker );
    VDatabaseSever ( self -> db );
    VDBManagerSever ( self -> mgr );
#endif
    free ( self );
    return 0;
}

rc_t
VViewAddRef ( const VView *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VView" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcTable, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t
VViewRelease ( const VView *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VView" ) )
        {
        case krefWhack:
            return VViewWhack ( ( VView* ) self );
        case krefNegative:
            return RC ( rcVDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* OpenView
 *  open a view for read using manager.
 *
 *  "view" [ OUT ] - return parameter for newly opened view
 *
 *  "schema" [ IN ] - schema object with the view schema
 *
 *  View's parameters will have to be bound using VViewBindParameterXXX() before View can be used
 */
LIB_EXPORT
rc_t
VDBManagerOpenView ( struct VDBManager const *   p_mgr,
                     const VView **              p_view,
                     const struct VSchema *      p_schema,
                     const char *                p_name )
{
    rc_t rc;

    if ( p_mgr == NULL )
    {
        rc = RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
    }
    else if ( p_view == NULL || p_schema == NULL )
    {
        rc = RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    else
    {
        /* create a structure */
        VView * view = calloc ( sizeof * view, 1 );
        if ( view == NULL )
        {
            rc = RC ( rcVDB, rcTable, rcConstructing, rcMemory, rcExhausted );
        }
        else
        {
            uint32_t i;
            uint32_t start = VectorStart ( & p_schema -> view );
            uint32_t count = VectorLength ( & p_schema -> view );
            String name;
            StringInitCString ( & name, p_name );
            for ( i = 0; i < count; ++ i )
            {
                const SView * v = ( const SView * ) VectorGet ( & p_schema -> view, start + i );
                /* for now, grab the first view with the given name */
                /*TODO: pick the best version */
                if ( StringCompare ( & v -> name -> name, & name ) == 0 )
                {
                    KRefcountInit ( & view -> refcount, 1, "VView", "make", "vtbl" );
                    view -> sview = v;
                    view -> schema = p_schema;
                    * p_view = view;
                    return 0;
                }
            }
            rc = RC ( rcVDB, rcTable, rcConstructing, rcTable, rcUnknown );
            free ( view );
        }

        * p_view = NULL;
    }

    return rc;
}

static
rc_t
VViewBindParameter ( const VView *   p_view,
                     const void *    p_param,
                     uint32_t        p_index,
                     const void *    p_obj )
{
    rc_t rc = 0;
    if ( p_view == NULL )
    {
        rc = RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
    }
    else
    {   /* locate the view's parameter, make sure it is a table, bind to the given table */
        const KSymbol * param = VectorGet ( & p_view -> sview -> params, p_index );
        if ( param == 0 )
        {
            rc = RC ( rcVDB, rcTable, rcOpening, rcIndex, rcOutofrange );
        }
        else if ( param -> u . obj != p_obj )
        {
            rc = RC ( rcVDB, rcTable, rcOpening, rcParam, rcWrongType );
        }
        //TODO: bind
    }
    return rc;
}

/* BindParameterTable
 *  Bind a view's parameter to a table.
 *
 *  "table" [ IN ] - the table to be bound. Has to have the same type as the corresponding
 *  parameter of the view
 *  "index" [ IN ] - 0-based index of the corresponding parameter in the view's parameter list
 */
VDB_EXTERN
rc_t
VViewBindParameterTable ( const VView * p_self,
                          const VTable * p_table,
                          uint32_t p_index )
{
    if ( p_table == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    return VViewBindParameter ( p_self, p_table, p_index, p_table -> stbl );
}

/* BindParameterView
 *  Bind a view's parameter to a view.
 *
 *  "view" [ IN ] - the view to be bound. Has to have the same type as the corresponding
 *  parameter of "self"
 *  "index" [ IN ] - 0-based index of the corresponding parameter in "self"'s parameter list
 */
VDB_EXTERN
rc_t
VViewBindParameterView ( const VView *           p_self,
                         const struct VView *    p_view,
                         uint32_t                p_index )
{
    if ( p_view == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    return VViewBindParameter ( p_self, p_view, p_index, p_view -> sview );
}

// VViewCursor

rc_t
VViewCursorAddRef ( const VCursor * p_self )
{
    const VViewCursor * self = ( const VViewCursor * ) p_self;

    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VCursor" ) )
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
    KRefcountWhack ( & p_self -> refcount, "VViewCursor" );
    VCursorCacheWhack ( & p_self -> col, NULL, NULL );
    VectorWhack ( & p_self -> row, VViewCursorVColumnWhack_checked, NULL );
    VViewRelease ( p_self -> view );
    free ( p_self );
    return 0;
}

rc_t
VViewCursorRelease ( const VCursor * p_self )
{
    const VViewCursor * self = ( const VViewCursor * ) p_self;

    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VCursor" ) )
        {
        case krefWhack:
            return VViewCursorWhack ( ( VViewCursor* ) self );
        case krefNegative:
            return RC ( rcVDB, rcCursor, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* AddSColumn
 */
static
rc_t VViewCursorAddSColumn ( VViewCursor *      p_self,
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
VViewCursorVAddColumn ( const VCursor *    p_self,
                        uint32_t *         p_idx,
                        const char *       p_name,
                        va_list            p_args )
{
    VViewCursor * self = ( VViewCursor * ) p_self;

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
            rc = VViewCursorAddColspec ( self, p_idx, colspec );
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
VViewCursorVGetColumnIdx ( const VCursor * p_self, uint32_t * p_idx, const char * p_name, va_list args )
{
    const VViewCursor * self = ( const VViewCursor * ) p_self;
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
#if 0
        else if ( self -> state == vcFailed )
        {
            rc = RC ( rcVDB, rcCursor, rcAccessing, rcCursor, rcInvalid );
        }
#endif
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
                rc = VViewCursorGetColspec ( self, p_idx, colspec );
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
VViewCursorDatatype ( const VCursor *       p_self,
                      uint32_t              p_idx,
                      struct VTypedecl *    p_type,
                      struct VTypedesc *    p_desc )
{
    const VViewCursor * self = ( const VViewCursor * ) p_self;
    rc_t rc;

    if ( p_type == NULL && p_desc == NULL )
    {
        rc = RC ( rcVDB, rcCursor, rcAccessing, rcParam, rcNull );
    }
    else
    {
        const VColumn * vcol = ( const VColumn* ) VectorGet ( & self -> row, p_idx );
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

/* Open
 *  open cursor, resolving schema
 *  for the set of opened columns
 *
 *  NB - there is no corresponding "Close"
 *  use "Release" instead.
 */
rc_t
VViewCursorOpen ( const VCursor * p_cself )
{
    rc_t rc;
    VViewCursor * self = ( VViewCursor * ) p_cself;

    return 0;
#if 0
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

    return rc;
#endif
}


