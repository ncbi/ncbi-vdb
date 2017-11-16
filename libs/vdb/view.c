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
#include "schema-parse.h"

#include <sysalloc.h>

struct VView
{
    /* open references */
    KRefcount refcount;

    /* schema and view description */
    const SView * sview;
};

/* Whack
 */
static
rc_t VViewWhack ( VView *self )
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

rc_t CC VViewAddRef ( const VView *self )
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

rc_t CC VViewRelease ( const VView *self )
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
LIB_EXPORT rc_t CC VDBManagerOpenView (
    struct VDBManager const *   p_mgr,
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
VViewBindParameter (
    const VView *   p_view,
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
VDB_EXTERN rc_t CC VViewBindParameterTable (
    const VView * p_self,
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
VDB_EXTERN rc_t CC VViewBindParameterView (
    const VView *           p_self,
    const struct VView *    p_view,
    uint32_t                p_index )
{
    if ( p_view == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    return VViewBindParameter ( p_self, p_view, p_index, p_view -> sview );
}


