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

#include <klib/refcount.h>
#include <klib/symbol.h>

#include "table-priv.h"
#include "column-priv.h"
#include "schema-parse.h"
#include "prod-expr.h"
#include "phys-priv.h"
#include "linker-priv.h"
#include "dbmgr-priv.h"

#include <sysalloc.h>
#include <stdio.h>

/* ViewCreateCursor
 *  creates a read cursor object onto view
 *
 *  "curs" [ OUT ] - return parameter for newly created cursor
 *
 *  "capacity" [ IN ] - the maximum bytes to cache on the cursor before
 *  dropping least recently used blobs
 */
LIB_EXPORT
rc_t CC
VViewCreateCursor ( struct VView const * p_self, const struct VViewCursor ** p_curs )
{
    rc_t rc = 0;
    if ( p_curs == NULL )
    {
        rc = RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    else if ( p_self == NULL )
    {
        rc = RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
    }
    else
    {
        rc = VViewCursorMake ( p_self, (struct VViewCursor **) p_curs );
    }
    return rc;
}

/* Whack
 */

static
rc_t
VViewWhack ( VView * p_self )
{
    uint32_t start = VectorStart ( & p_self -> sview -> params );
    uint32_t count = VectorLength ( & p_self -> sview -> params );
    uint32_t i;
    for ( i = 0 ; i < count; ++ i)
    {
        const KSymbol * param = VectorGet ( & p_self -> sview -> params, start + i );
        void * obj = VectorGet ( & p_self -> bindings, i );
        if ( obj != NULL )
        {
            if ( param -> type == eView )
            {
                VViewRelease ( obj );
            }
            else
            {
                VTableRelease ( obj );
            }
        }
    }

    VectorWhack ( & p_self -> bindings, 0, 0 );
    VLinkerRelease ( p_self -> linker );
    KRefcountWhack ( & p_self -> refcount, "VView" );

    free ( p_self );
    return 0;
}

rc_t
VViewAddRef ( const VView * p_self )
{
    if ( p_self != NULL )
    {
        switch ( KRefcountAdd ( & p_self -> refcount, "VView" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcTable, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t
VViewRelease ( const VView * p_self )
{
    if ( p_self != NULL )
    {
        switch ( KRefcountDrop ( & p_self -> refcount, "VView" ) )
        {
        case krefWhack:
            return VViewWhack ( ( VView* ) p_self );
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
rc_t CC
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
                    rc = VLinkerMake ( & view -> linker, p_mgr -> linker, p_mgr -> linker -> dl );
                    if ( rc == 0 )
                    {
                        KRefcountInit ( & view -> refcount, 1, "VView", "make", "vtbl" );
                        view -> sview = v;
                        view -> schema = p_schema;
                        VectorInit ( & view -> bindings, 0, 8 );
                        * p_view = view;
                        return 0;
                    }
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
int32_t
BindingIdxByName ( const SView * p_self, const String * p_name )
{
    uint32_t start = VectorStart ( & p_self -> params );
    uint32_t count = VectorLength ( & p_self -> params );
    uint32_t i;
    for ( i = 0 ; i < count; ++ i )
    {
        const KSymbol * p = VectorGet ( & p_self -> params, start + i );
        if ( StringEqual ( & p -> name, p_name ) )
        {
            return start + i;
        }
    }
    return -1;
}

/* BindParameterTable
 *  Bind a view's parameter to a table.
 *
 *  "table" [ IN ] - the table to be bound. Has to have the same type as the corresponding
 *  parameter of the view
 *  "index" [ IN ] - 0-based index of the corresponding parameter in the view's parameter list
 */
LIB_EXPORT
rc_t
VViewBindParameterTable ( const VView *     p_self,
                          const char *      p_param_name,
                          const VTable *    p_table )
{
    if ( p_self == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
    }
    if ( p_table == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    else
    {   /* locate the view's parameter, make sure it is a table, bind to the given table */
		int32_t idx;
        String name;
        StringInitCString( & name, p_param_name );
        idx = BindingIdxByName ( p_self -> sview, & name );
        if ( idx >= 0 )
        {   /* self->bindings is parallel to self->sview->params */
            const KSymbol * param = VectorGet ( & p_self -> sview -> params, idx );
            if ( StringEqual ( & param -> name, & name ) )
            {
                if ( param -> type != eTable || param -> u . obj != p_table -> stbl )
                {
                    return RC ( rcVDB, rcTable, rcOpening, rcParam, rcWrongType );
                }
                else if ( VectorGet ( & p_self -> bindings, idx ) != NULL )
                {
                    return RC ( rcVDB, rcTable, rcOpening, rcTable, rcExists );
                }
				else
				{
					VView * self = (VView*) p_self;
					rc_t rc = VTableAddRef ( p_table );
					if ( rc == 0 )
					{
						return VectorSet( & self -> bindings, idx, p_table );
					}
					return rc;
				}
            }
        }

        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNotFound );
    }
}

/* BindParameterView
 *  Bind a view's parameter to a view.
 *
 *  "view" [ IN ] - the view to be bound. Has to have the same type as the corresponding
 *  parameter of "self"
 *  "index" [ IN ] - 0-based index of the corresponding parameter in "self"'s parameter list
 */
LIB_EXPORT
rc_t
VViewBindParameterView ( const VView *          p_self,
                         const char *           p_param_name,
                         const struct VView *   p_view )
{
    if ( p_self == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
    }
    if ( p_view == NULL )
    {
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    }
    else
    {   /* locate the view's parameter, make sure it is a view, bind to the given view */
        int32_t idx;
		String name;
        StringInitCString( & name, p_param_name );
        idx = BindingIdxByName ( p_self -> sview, & name );
        if ( idx >= 0 )
        {   /* self->bindings is parallel to self->sview->params */
            const KSymbol * param = VectorGet ( & p_self -> sview -> params, idx );
            if ( StringEqual ( & param -> name, & name ) )
            {
                if ( param -> type != eView || param -> u . obj != p_view -> sview )
                {
                    return RC ( rcVDB, rcTable, rcOpening, rcParam, rcWrongType );
                }
                else if ( VectorGet ( & p_self -> bindings, idx ) != NULL )
                {
                    return RC ( rcVDB, rcTable, rcOpening, rcTable, rcExists );
                }
				else
				{
					VView * self = (VView*) p_self;
					rc_t rc = VViewAddRef ( p_view );
					if ( rc == 0 )
					{   /* self->bindings is parallel to self->sview->params */
						return VectorSet( & self -> bindings, idx, p_view );
					}
	                return rc;
				}
            }
        }
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNotFound );
    }
}

const VTable *
VViewPrimaryTable( const VView * p_self )
{
    /* return the table bound to the first parameter
    (may descend through one or more views if the first parameter is a view) */
    uint32_t start = VectorStart ( & p_self -> sview -> params );
    const KSymbol * param = VectorGet ( & p_self -> sview -> params, start );
    if ( param == NULL )
    {
        return NULL; /* not bound */
    }
	else
	{
		const Vector * b = & p_self -> bindings;
		const void * obj = VectorGet ( b, VectorStart ( b ) );
		if ( param -> type == eTable )
		{
			return ( const VTable * ) obj;
		}
		else /* view */
		{
			return VViewPrimaryTable ( ( const VView *) obj );
		}
	}
}

/* Follow the chain of view inheritance to find out which parameter of p_child
   corresponds to p_ancestor's parameter # p_paramId.
   Returns paramId of the child's parameter or -1 if p_ancestor is not found
*/
static
int32_t
SViewAncestorParameterBoundTo ( const SView * p_child, const SView * p_ancestor, uint32_t p_paramId )
{
    uint32_t start = VectorStart ( & p_child -> parents );
    uint32_t count = VectorLength ( & p_child -> parents );
    uint32_t i;
    for ( i = 0 ; i < count; ++ i)
    {
        const SViewInstance * dadInst = VectorGet ( & p_child -> parents, start + i );
        assert ( dadInst != NULL );
        if ( dadInst -> dad == p_ancestor )
        {
            const KSymbol * arg = VectorGet ( & dadInst -> params, p_paramId );
            /* arg is the name of the p_child's parameter used to instantiate this parent */
            assert ( arg != NULL );
            return BindingIdxByName ( p_child, & arg -> name );
        }
        else
        {   /* go through this parent's ancestry */
            int32_t idx = SViewAncestorParameterBoundTo ( dadInst -> dad, p_ancestor, p_paramId );
            if ( idx != -1 ) /* found */
            {   /* idx is the paramId of the parent's parameter; translate it into the child's paramId */
                const KSymbol * arg = VectorGet ( & dadInst -> params, idx );
                /* arg is the name of the child's parameter used to instantiate this parent */
                assert ( arg != NULL );
                return BindingIdxByName ( p_child, & arg -> name );
            }
            /* keep looking */
        }
    }
    return -1;
}

const void *
VViewGetBoundObject( const VView * p_self, const SView * p_sview, uint32_t p_paramId )
{
    if ( p_self -> sview == p_sview )
    {
        return VectorGet ( & p_self -> bindings, p_paramId );
    }
    else
    {   /* look up the inheritance hierarchy */
        uint32_t idx = SViewAncestorParameterBoundTo ( p_self -> sview, p_sview, p_paramId );
        if ( idx != -1 )
        {
            return VectorGet ( & p_self -> bindings, idx );
        }
    }
    return NULL;
}
