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
#include "schema-priv.h"
#include "schema-expr.h"
#include "schema-parse.h"
#include "cursor-priv.h"
#include "column-priv.h"
#include "prod-priv.h"
#include "prod-expr.h"
#include "phys-priv.h"
#include "view-priv.h"
#undef KONST

#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <vdb/xform.h>
#include <klib/symbol.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VProdResolve
 */

/* CastExpr
 *  inserts an explicit cast operation
 *
 *  in "C" style languages ( okay, ALGOL style ), a cast expression is
 *  used as a means of coercing the rh expression type to a fixed type,
 *  which is then assigned to the lh side with normal typecasting rules.
 *  specifically, performs an intermediate assignment allowing truncation
 *  to reshape and potentially reformat the rh value ( e.g. float to char ).
 *
 *  in "C++" there were new cast operators introduced to indicate more nuance:
 *    static_cast      - permits up and down-casts with compiler check
 *    const_cast       - modifies cv qualifiers [ not applicable to VDB ]
 *    dynamic_cast     - permits up and down-casts with runtime check
 *    reinterpret_cast - permits casts between unrelated classes with
 *                       compatible binary forms ( a la "C" style ).
 *
 *  in "VDB", the compiler preserves lh and rh type information until
 *  productions are resolved. assignments always involve implicit casts
 *  that permit casting from sub to super-type. our explicit cast expression
 *  performs something analogous to the C++ "static_cast" in that it allows
 *  direct up and down casts, as well as sibling and cousin casts.
 *
 *  specifically, LH = ( T ) RH when T is a sub-type of LH and:
 *    a) RH is a sub-type of T [ implicit rule ]
 *    b) T is a sub-type of RH [ downcast ]
 *    c) T and RH share a common ancestor
 *  in all cases, LH, T and RH must all have identical element sizes
 *
 *  implicit typecasting rules allow LH or RH to be a typeset. the types
 *  are initially refined to the intersection between LH and RH
 *    a) TYPE    x TYPE
 *    b) TYPESET x TYPE
 *    c) TYPE    x TYPESET
 *    d) TYPESET x TYPESET
 *  in the latter case, the intersection may produce more than one possible
 *  result, which would incur an error when evaluating the expression. a
 *  cast operator will remove the ambiguity.
 *
 *  the rh expression may involve a column name, with type overloading. this
 *  creates the same effect as a TYPESET. a cast operator can clarify an ambigous
 *  assignment, and in the case of downcasts, make it possible.
 *
 * NB
 *  to perform a C++ style reinterpret_cast, use the "cast" function
 */
static
rc_t VProdResolveCastExpr ( const VProdResolve *self, VProduction **out, const SBinExpr *expr )
{
    /* extract cast type */
    VFormatdecl cast;
    rc_t rc = STypeExprResolveAsFormatdecl
        ( ( const STypeExpr* ) expr -> left, self -> schema, & cast, self -> cx_bind );
    if ( rc == 0 )
    {
        /* resolve rh expression */
        VTypedesc desc;
        VFormatdecl fd = cast;
        rc = VProdResolveExpr ( self, out, & desc,
            & fd, expr -> right, true );
        if ( rc != 0 || * out == NULL )
            return rc;

        /* casting mode allowed returned production to be:
           a) identical type
           b) sub-type
           c) super-type
           d) have common parent

           in all cases, the sizeof rh production element
           matches "cast" size */
        rc = VSimpleProdMake ( out, self -> owned, self -> curs, prodSimpleCast,
            "cast", & cast, & desc, NULL, * out, self -> chain );
    }

    return rc;
}

/* ParamExpr
 *  resolve a simple parameter by name
 */
LIB_EXPORT rc_t CC VProdResolveParamExpr ( const VProdResolve *self, VProduction **out, const KSymbol *sym )
{
    const SProduction *sprod = sym -> u . obj;
    VProduction *vprod = VCursorCacheGet ( self -> cache, & sprod -> cid );
    if ( vprod != NULL )
    {
        * out = vprod;
        return 0;
    }

    PLOGMSG ( klogWarn, ( klogWarn, "unknown parameter '$(param)' used in expression"
                          , "param=%.*s"
                          , ( int ) sprod -> name -> name . size
                          , sprod -> name -> name . addr ));
    return 0;
}


/* ProdExpr
 *  resolve a simple production by name
 *  create/return a VSimpleProd object
 */
rc_t VProdResolveSProduction ( const VProdResolve *self, VProduction **out, const SProduction *sprod )
{
    rc_t rc;
    VFormatdecl fd;

    /* check cache */
    VProduction *vprod = VCursorCacheGet ( self -> cache, & sprod -> cid );
    if ( vprod != NULL )
    {
        /* return valid or failed production */
        * out = vprod;
        return 0;
    }

    /* pre-fail */
    rc = VCursorCacheSet ( self -> cache, & sprod -> cid, FAILED_PRODUCTION );
    if ( rc == 0 )
    {
        /* resolve production type */
        if ( sprod -> trigger )
            memset ( & fd, 0, sizeof fd );
        else
        {
            rc = STypeExprResolveAsFormatdecl
                ( ( const STypeExpr* ) sprod -> fd, self -> schema, & fd, self -> cx_bind );
        }
    }
    if ( rc == 0 )
    {
        /* resolve assignment expression */
        VTypedesc desc;
        rc = VProdResolveExpr ( self, out, & desc,
            & fd, sprod -> expr, false );
        if ( rc == 0 && * out != NULL )
        {
            const char *name = sprod -> name -> name . addr;
            assert ( name [ sprod -> name -> name . size ] == 0 );
            rc = VSimpleProdMake ( out, self -> owned, self -> curs, prodSimpleCast,
                name, & fd, & desc, & sprod -> cid, * out, self -> chain );
            if ( rc == 0 )
            {
                void *ignore;
                rc = VCursorCacheSwap ( self -> cache, & sprod -> cid, * out, & ignore );
            }
        }
    }

    return rc;
}

static
rc_t VProdResolveProdExpr ( const VProdResolve *self, VProduction **out, const KSymbol *sym )
{
    const SProduction *sprod = sym -> u . obj;
    if ( ! sprod -> trigger )
        return VProdResolveSProduction ( self, out, sprod );

    PLOGMSG ( klogWarn, ( klogWarn, "trigger production '$(trig)' used in expression"
                          , "trig=%.*s"
                          , ( int ) sym -> name . size
                          , sym -> name . addr ));

    return 0;
}


/* ColumnExpr
 *  the trick about resolving a column reference is in its type
 */
typedef struct SColumnBestFit SColumnBestFit;
struct SColumnBestFit
{
    BSTNode n;
    const SColumn *scol;
    VTypedecl td;
    uint32_t distance;
};

static
rc_t VProdResolveBestColumn ( const VProdResolve *self,
    VProduction **out, const BSTree *ordered, bool alt )
{
    rc_t rc;

    /* walk all candidtes */
    const SColumnBestFit *n = ( const SColumnBestFit* ) BSTreeFirst ( ordered );
    for ( rc = 0; n != NULL; n = ( const SColumnBestFit* ) BSTNodeNext ( & n -> n ) )
    {
        /* look for open column */
        const SColumn *scol = n -> scol;

        /* resolve the column as appropriate */
        rc = VProdResolveColumn ( self, out, scol, alt );
        if ( rc != 0 || * out != NULL )
            break;
    }

    return rc;
}

static
int64_t CC order_column ( const BSTNode *item, const BSTNode *n )
{
    const SColumnBestFit *a = ( const SColumnBestFit* ) item;
    const SColumnBestFit *b = ( const SColumnBestFit* ) n;
    if ( a -> distance != b -> distance )
        return (int64_t) a -> distance - (int64_t) b -> distance;
    return VCtxIdCmp ( & a -> scol -> cid, & b -> scol -> cid );
}

static
rc_t VProdResolveColExpr ( const VProdResolve *self, VProduction **out,
    VFormatdecl *fd, const SSymExpr *x, bool casting )
{
    rc_t rc;
    const SNameOverload *sname;
    const KSymbol *sym = x -> _sym;

    BSTree ordered;
    uint32_t i, count;
    SColumnBestFit buff [ 16 ], * nodes = buff;
    /* fail if "fd" has a format */
    if ( fd -> fmt != 0 )
    {
        PLOGMSG ( klogWarn, ( klogWarn, "illegal cast of column '$(name)'"
                   , "name=%.*s"
                   , ( int ) sym -> name . size
                   , sym -> name . addr ));
        return 0;
    }

    /* allocate nodes for indexing columns */
    sname = sym -> u . obj;
    count = VectorLength ( & sname -> items );
    if ( count > sizeof buff / sizeof buff [ 0 ] )
    {
        nodes = malloc ( sizeof * nodes * count );
        if ( nodes == NULL )
            return RC ( rcVDB, rcProduction, rcResolving, rcMemory, rcExhausted );
    }

    /* insert columns into ordered tree */
    BSTreeInit ( & ordered );
    for ( i = VectorStart ( & sname -> items ), count += i; i < count; ++ i )
    {
        /* get SColumn */
        nodes [ i ] . scol = ( const void* ) VectorGet ( & sname -> items, i );

        /* perform type cast and measure distance */
        if ( casting ?
             VTypedeclCommonAncestor ( & nodes [ i ] . scol -> td, self -> schema,
                 & fd -> td, & nodes [ i ] . td, & nodes [ i ] . distance ) :
             VTypedeclToTypedecl ( & nodes [ i ] . scol -> td, self -> schema,
                 & fd -> td, & nodes [ i ] . td, & nodes [ i ] . distance ) )
        {
            BSTreeInsert ( & ordered, & nodes [ i ] . n, order_column );
        }
    }

    /* try to resolve each in order */
    rc = VProdResolveBestColumn ( self, out, & ordered, x -> alt );

    if ( nodes != buff )
        free ( nodes );

    return rc;
}


/* PhysExpr
 */
rc_t VProdResolveSPhysMember ( const VProdResolve *self,
    VProduction **out, const SPhysMember *smbr )
{
    rc_t rc;
    VCursor *curs;
    VPhysical *phys;

    curs = self -> curs;
    phys = VCursorCacheGet ( VCursorPhysicalColumns ( curs ), & smbr -> cid );
    if ( phys != NULL )
    {
        /* this guy should be readable, but it is not guaranteed */
        if ( phys != FAILED_PHYSICAL )
            * out = phys -> out;
        return 0;
    }

    /* pre-fail */
    rc = VCursorCacheSet ( VCursorPhysicalColumns ( curs ), & smbr -> cid, FAILED_PHYSICAL );
    if ( rc == 0 )
    {
        /* create physical object */
        rc = VPhysicalMake ( & phys, curs, smbr );
        if ( rc == 0 )
        {
            /* build physical */
            rc = VProdResolvePhysical ( self, phys );
            if ( rc == 0 && phys -> out > FAILED_PRODUCTION && phys -> b2p > FAILED_PRODUCTION )
            {
                /* set success */
                void *ignore;
                rc = VCursorCacheSwap ( VCursorPhysicalColumns ( curs ), & smbr -> cid, phys, & ignore );
                if ( rc == 0 )
                {
                    * out = phys -> out;
                    return 0;
                }
            }
            if ( GetRCState ( rc ) == rcUndefined )
                rc = 0;

            VPhysicalWhack ( phys, NULL );
        }
    }

    return rc;
}

static
rc_t VProdResolvePhysExpr ( const VProdResolve *self,
    VProduction **out, const KSymbol *sym )
{
    if ( self -> chain == chainEncoding )
    {
        assert ( ! VCursorIsReadOnly ( self -> curs ) );
        PLOGMSG ( klogWarn, ( klogWarn, "illegal access of physical column '$(name)'"
                   , "name=%.*s"
                   , ( int ) sym -> name . size
                   , sym -> name . addr ));
        return 0;
    }

    return VProdResolveSPhysMember ( self, out, sym -> u . obj );
}


/* FwdExpr
 *  handle a forwarded symbol in expression
 *  if the symbol is "virtual", check for its override
 *  map actual symbol type to expression handler
 */
static
rc_t VProdResolveFwdExpr ( const VProdResolve *self, VProduction **out,
    VFormatdecl *fd, const SSymExpr *x, bool casting )
{
    /* virtual names that appear in parent table
       expressions may be overridden in children */
    const KSymbol *sym = x -> _sym;
    if ( sym -> type == eVirtual )
    {
        /* most derived table/view class */
        const KSymbol *sym2 = sym;
        sym = VCursorFindOverride ( self -> curs, ( const VCtxId* ) & sym -> u . fwd );
        if ( sym == NULL )
        {
            PLOGMSG ( klogWarn, ( klogWarn, "virtual reference '$(fwd)' not found in overrides table"
                       , "fwd=%.*s"
                       , ( int ) sym2 -> name . size
                       , sym2 -> name . addr ));
            return 0;
        }
    }

    /* test symbol type */
    switch ( sym -> type )
    {
    case eProduction:
        return VProdResolveProdExpr ( self, out, sym );
    case ePhysMember:
        return VProdResolvePhysExpr ( self, out, sym );
    case eColumn:
        return VProdResolveColExpr ( self, out, fd, x, casting );
    }

    VDB_DEBUG (("%s: unresolved forward reference '%S'",
                 __func__, &sym->name));

    return 0;
}

static
rc_t VProdResolveMembExpr ( const VProdResolve *    p_self,
                            VProduction **          p_out,
                            VFormatdecl *           p_fd,
                            const SMembExpr *       p_x,
                            bool                    p_casting )
{
    /* we know we are in a view */
    const KSymbol * object = VectorGet ( & p_x -> view -> params, p_x -> paramId );
    assert ( p_self -> view );
    assert ( object != NULL );

    /* p_x -> object is a table/view parameter of the view the member-expresison is in.
        Locate the VTable/VView bound to the corresponding parameter of the view,
        make a SSymExpr pointing to p_x -> member, call ProdResolveColExpr/VProdResolveProdExpr in the context of the VTable/VView */

    if ( p_x -> member -> type == eColumn || p_x -> member -> type == eProduction )
    {
        VProdResolve pr = * p_self;
		const void * boundObj;
        pr . name = & object -> name;
        boundObj = VViewGetBoundObject ( pr . view, p_x -> view, p_x -> paramId );
        if ( boundObj != 0 )
        {
            switch ( object -> type )
            {
            case eTable:
                pr . primary_table = boundObj;
                pr . view = NULL;
                break;
            case eView:
                /*TODO: should we update pr . primary_table ? */
                pr . view = boundObj;
                break;
            default:
                return RC ( rcVDB, rcProduction, rcResolving, rcMessage, rcUnsupported );
            }

            {
                const SExpression * ref;
                rc_t rc = SSymExprMake( & ref,
                                        p_x -> member,
                                        p_x -> member -> type == eColumn ? eColExpr : eProdExpr );
                if ( rc == 0 )
                {
                    VProduction * vmember;
                    rc = VProdResolveExpr ( & pr, & vmember, NULL, p_fd, ref, p_casting );
                    SExpressionWhack ( ref );
                    if ( rc == 0 )
                    {
                        if ( p_x -> rowId == NULL )
                        {   /* simple member: tbl . col */
                            * p_out = vmember;
                            return 0;
                        }
                        else
                        {   /* member with a pivot: tbl [ expr ] . col */
                            VProduction * rowId;
                            VFormatdecl fd = { { 0, 0 }, 0 };
                            rc = VProdResolveExpr ( p_self, & rowId, NULL, & fd, p_x -> rowId, p_casting );
                            if ( rc == 0 )
                            {
                                VPivotProd * ret;
                                rc = VPivotProdMake ( & ret, p_self -> owned, vmember, rowId, p_x -> member -> name . addr, p_self -> chain );
                                if ( rc == 0 )
                                {
                                    * p_out = & ret -> dad;
                                    return 0;
                                }
                                VProductionWhack ( rowId, p_self -> owned );
                            }
                            VProductionWhack ( vmember, p_self -> owned );
                        }
                    }
                }
                return rc;
            }
        }
    }
    return RC ( rcVDB, rcProduction, rcResolving, rcMessage, rcUnsupported );
}


/* ResolveExpr
 *  resolves expression and returns resulting production object
 *
 *  "out" [ IN/OUT, INITIALLY NULL ] - return parameter for production.
 *  assmed to be preset to NULL on input.
 *  invalid on output with non-zero return code
 *  non-NULL on successful resolution with zero return code
 *
 *  "fd" [ IN/OUT ] - resultant type of assignment expression
 *  on input gives acceptance criteria, i.e. a fmtdecl, typeset or wildcard
 *  invalid on output with non-zero return code or NULL "out"
 *  otherwise, gives actual assigned type on output
 *
 *  "casting" [ IN ] - true if performing explicit type casting
 *  see VProdResolveCastExpr for rules
 *
 *  "desc" [ OUT, NULL OKAY ] - resultant type description of "out"
 *  invalid on non-zero return or NULL "out"
 *  contains description of fd->td on success
 *
 *  "cache" [ IN ] - modifiable vector of VProductions by id within
 *  a given scope: table or script function.
 *
 *  "param" [ IN, NULL OKAY ] - modifiable vector of VProduction params
 *  by id - only valid within script function
 *
 *  "expr" [ IN ] - expression to be evaluated
 *
 *  returns non-zero if a non-recoverable error occurs, 0 otherwise.
 *  resolution is only successful on zero return code and non-NULL out.
 */
#if _DEBUGGING
#include "schema-dump.h"

static size_t xsz;
static char xbuffer [ 4096 ];

static
rc_t CC VProdResolveCapture ( void *data, const void *buffer, size_t size )
{
    if ( xsz + size >= sizeof xbuffer )
    {
        LOGERR (klogFatal, -1, "( xsz + size >= sizeof xbuffer )");
        return -1;
    }
    memmove ( & xbuffer [ xsz ], buffer, size );
    xsz += size;
    return 0;
}

static
const char *VProdResolvePrintExpr ( const VProdResolve *self, const SExpression *expr )
{
    SDumper dumper;
    SDumperInit ( & dumper, self -> schema, sdmPrint, VProdResolveCapture, NULL );

    xsz = 0;
    SDumperPrint ( & dumper, "%E", expr );
    SDumperWhack ( & dumper );

    xbuffer [ xsz ] = 0;
    return xbuffer;
}

static int indent_level = 0;
#endif

rc_t VProdResolveExpr ( const VProdResolve *self,
    VProduction **out, VTypedesc *desc, VFormatdecl *fd,
    const SExpression *expr, bool casting )
{
    rc_t rc;
    VProduction *prod;

    if ( expr == NULL )
    {
        /* report NULL expression, but don't die */
        PLOGMSG ( klogWarn, ( klogWarn, "NULL expression in '$(tbl)' table schema"
                   , "tbl=%.*s"
                   , ( int ) self -> name -> size
                   , self -> name -> addr ));
        return 0;
    }

    prod = NULL;
    *out = NULL;

#if _DEBUGGING
    ++ indent_level;
    VDB_DEBUG (( "%*cresolving expression '%s'\n", indent_level, ' ',
                 VProdResolvePrintExpr ( self, expr ) ));
#endif

    switch ( expr -> var )
    {
    case eParamExpr:
        /* a script function is making reference to a parameter */
        rc = VProdResolveParamExpr ( self, & prod, ( ( const SSymExpr* ) expr ) -> _sym );
    assert (rc != -1);
        break;

    case eProdExpr:
        /* return a simple production */
        rc = VProdResolveProdExpr ( self, & prod, ( ( const SSymExpr* ) expr ) -> _sym );
    assert (rc != -1);
        break;

    case eFwdExpr:
        /* handle an implicit or overridden reference */
        rc = VProdResolveFwdExpr ( self, & prod, fd, ( const SSymExpr* ) expr, casting );
    assert (rc != -1);
        break;

    case eColExpr:
        /* return a column production */
        rc = VProdResolveColExpr ( self, & prod, fd, ( const SSymExpr* ) expr, casting );
    assert (rc != -1);
        break;

    case ePhysExpr:
        /* return a physical production */
        rc = VProdResolvePhysExpr ( self, & prod, ( ( const SSymExpr* ) expr ) -> _sym );
    assert (rc != -1);
        break;

    case eScriptExpr:
        /* create a script function */
        rc = VProdResolveScriptExpr ( self, & prod, fd, ( const SFuncExpr* ) expr );
    assert (rc != -1);
        break;

    case eFuncExpr:
        /* create an external function */
        rc = VProdResolveFuncExpr ( self, & prod, fd, ( const SFuncExpr* ) expr );
    assert (rc != -1);
        break;

    case eCastExpr:
        /* perform an explicit cast */
        rc = VProdResolveCastExpr ( self, & prod, ( const SBinExpr* ) expr );
    assert (rc != -1);
        break;

    case eCondExpr:
        /* run left and right expressions in order until exit condition */
        rc = VProdResolveExpr ( self, out, desc, fd, ( ( const SBinExpr* ) expr ) -> left, casting );
    assert (rc != -1);
        if ( ( rc == 0 && * out == NULL ) || self -> discover_writable_columns )
        {
            rc = VProdResolveExpr ( self, out, desc, fd, ( ( const SBinExpr* ) expr ) -> right, casting );
    assert (rc != -1);
        }
#if _DEBUGGING
        -- indent_level;
#endif
        return rc;

    case eMembExpr:
        /* return a column or a production */
        rc = VProdResolveMembExpr ( self, & prod, fd, ( const SMembExpr* ) expr, casting );
    assert (rc != -1);
        break;

    default:
        /* report bad expression, but don't die */
        PLOGMSG ( klogWarn, ( klogWarn, "unrecognized expression in '$(tbl)' table schema"
                   , "tbl=%.*s"
                   , ( int ) self -> name -> size
                   , self -> name -> addr ));
#if _DEBUGGING
        -- indent_level;
#endif
        return 0;
    }

    /* guard against returns of NULL or FAILED_PRODUCTION */
    if ( rc == 0 && prod > FAILED_PRODUCTION )
    {
        VDB_DEBUG ( ("%*cresolved expression  '%s'\n", indent_level, ' ', VProdResolvePrintExpr ( self, expr ) ) );

        /* returned production must be on requested chain */
        if ( ( prod -> chain & self -> chain ) == 0 )
        {
            rc = RC ( rcVDB, rcProduction, rcResolving, rcSchema, rcInconsistent );
            VDB_DEBUG ( ( "%*cPRODUCTION RESOLVED ON WRONG FORK: %R\n", indent_level, ' ', rc ) );
        }
        else
        {
            /* fix uncommitted production chain */
            if ( prod -> chain == chainUncommitted )
                prod -> chain = self -> chain;

            /* test for type compatibility - modifies "fd" */
            if ( casting ?
                 VFormatdeclCommonAncestor ( & prod -> fd, self -> schema, fd, fd, NULL ) :
                 VFormatdeclToFormatdecl ( & prod -> fd, self -> schema, fd, fd, NULL ) )
            {
                /* if no type description is requested, we're done */
                if ( desc == NULL )
                    * out = prod;
                else
                {
                    /* otherwise, create a type description */
                    rc = VSchemaDescribeTypedecl ( self -> schema, desc, & fd -> td );
    assert (rc != -1);
                    if ( rc != 0 )
                        VDB_DEBUG ( ( "%*cREQUESTED TYPE CANNOT BE DESCRIBED: %R\n", indent_level, ' ', rc ) );
                    else
                        * out = prod;
                }
            }
            else
            {
#if _DEBUGGING
                char from [ 128 ] = "", to [ 128 ] = "";
                VTypedeclToText ( & prod -> fd . td, self -> schema, from, sizeof from );
                VTypedeclToText ( & fd -> td, self -> schema, to, sizeof to );
                VDB_DEBUG ( ( "%*cexpression '%s' cannot be %s from '%s' to '%s'\n"
                              , indent_level, ' '
                              , VProdResolvePrintExpr ( self, expr )
                              , casting ? "cast" : "typed"
                              , from
                              , to
                             )
                    );
#endif
            }
        }
    }
    else if ( rc != 0 )
    {
        VDB_DEBUG ( ( "failed to resolve expression '%s' prod %p %R\n", VProdResolvePrintExpr ( self, expr ), prod, rc ) );
    }
    else if ( prod == NULL )
    {
        VDB_DEBUG ( ( "expression '%s' was not resolved\n", VProdResolvePrintExpr ( self, expr ) ) );
    }
    else
    {
        VDB_DEBUG ( ( "expression '%s' returned FAILED_PRODUCTION\n", VProdResolvePrintExpr ( self, expr ) ) );
    }

#if _DEBUGGING
    -- indent_level;
#endif

    return rc;
}


/* ResolveColumn
 *  resolves a column from read/validate expression
 */
rc_t VProdResolveColumnRead ( const VProdResolve *self,
    VProduction **out, const SColumn *scol )
{
    rc_t rc;
    VFormatdecl fd;
    const char *name;
    VCursor *curs;
    VColumn *vcol;

    VDB_DEBUG ( ( "resolving column '%N' read expression.\n", scol -> name ) );

    /* potential error if self is NULL */
    curs = self -> curs;
    if ( out == NULL )
    {
        rc =  RC(rcVDB, rcProduction, rcResolving, rcParam, rcNull);
        VDB_DEBUG ( ( "result NULL for column '%N'; no output can be produced by '%s' rc %R\n",
                      scol -> name, __func__, rc ) );
        return rc;
    }

    /* fetch the column */
    vcol = VCursorCacheGet ( VCursorColumns ( curs ), & scol -> cid );
    if ( vcol == NULL )
    {
        VDB_DEBUG ( ( "failed to fetch NULL for column '%N'; no output was produced by '%s'\n",
                      scol -> name, __func__ ) );
        return 0;
    }

    /* if the read production is in place, return it */
    if ( vcol -> in != NULL )
    {
        if ( vcol -> in != FAILED_PRODUCTION )
            * out = vcol -> in;
        return 0;
    }

    /* pre-fail */
    vcol -> in = FAILED_PRODUCTION;

    /* production resolution works with fmtdecl */
    fd . td = scol -> td;
    fd . fmt = 0;

    /* resolve the expression */
    rc = VProdResolveExpr ( self, out, & vcol -> desc, & fd, scol -> read, false );
    assert (rc != -1);

    if ( rc != 0 || *out == NULL )
        return rc;

    /* repair incomplete column declarations */
    if ( scol -> td . type_id == 0 )
    {
        if ( fd . td . type_id == 0 )
        {
            rc = RC ( rcVDB, rcColumn, rcResolving, rcType, rcUndefined );
            VDB_DEBUG (("failed to repair incomplete declaration for column '%N' rc %R\n",
                        scol -> name, rc));
            return rc;
        }
        ( ( SColumn* ) scol ) -> td = fd . td;
    }

    /* create a simple prod to manage fd and desc */
    name = scol -> name -> name . addr;
    assert ( name [ scol -> name -> name . size ] == 0 );
    rc = VSimpleProdMake ( out, self -> owned, self -> curs, prodSimpleCast,
        name, & fd, & vcol -> desc, NULL, * out, self -> chain );

    assert (rc != -1);
    if ( rc != 0 )
    {
        VDB_DEBUG (("failed to create a simple prod to manage fd and desc for column '%N', rc %R\n",
                    scol -> name, rc));
        return rc;
    }

    /* return column input - this is input to the column
       that is intended to be read from the cursor */
    vcol -> in = * out;
    return rc;
}

/* ResolvePhysical
 *  resolves a physical column
 */
rc_t VProdResolvePhysicalRead ( const VProdResolve *self, VPhysical *phys )
{
    rc_t rc;
    VTypedesc desc;
    VFormatdecl fd;
    VProduction *prod;
    VFunctionProd *bs;
    VCursor *curs = self -> curs;

    const char *name;
    const SExpression *enc;
    const SPhysMember *smbr;

    /* a write cursor would have opened this already */
    if ( VCursorIsReadOnly ( curs ) )
    {
        /* open the physical column for read */
        rc = VPhysicalOpenRead ( phys, ( VSchema* ) self -> schema, self -> primary_table );
        if ( rc != 0 )
        {
            /* trying to open a column that doesn't exist
               is not an error, but it didn't resolve */
            if ( GetRCState ( rc ) == rcNotFound )
                return 0;

            return rc;
        }
    }

    /* should be completely resolved */
    smbr = phys -> smbr;
    if ( smbr -> td . type_id == 0 )
        return 0;

    /* production name */
    name = smbr -> name -> name . addr;

    /* type information */
    fd . td = smbr -> td;
    fd . fmt = 0;
    rc = VSchemaDescribeTypedecl ( self -> schema, & desc, & fd . td );
    if ( rc != 0 )
        return rc;

    /* create output adapter */
    rc = VPhysicalProdMake ( & prod, self -> owned,
        curs, phys, prodPhysicalOut, name, & fd, & desc );
    if ( rc != 0 )
        return rc;

    /* create byte-swap function with transparent type information */
    rc = VFunctionProdMake ( & bs, self -> owned,
        curs, prodFuncByteswap, name, & fd, & desc, chainDecoding );
    if ( rc != 0 )
        return rc;

    /* set adapter as input to byte swap */
    rc = VectorAppend ( & bs -> parms, NULL, prod );
    if ( rc != 0 )
        return rc;

    /* take byte-swap function as output */
    phys -> out = & bs -> dad;

    /* NB - we now have byte-order native output via an adapter
       it remains to create a decoding path for physical blobs */


    /* create adapter */
    rc = VPhysicalProdMake ( & prod, self -> owned,
        curs, phys, prodPhysicalKCol, name, & fd, & desc );
    if ( rc != 0 )
        return rc;

    /* create serial-to-blob stage */
    rc = VSimpleProdMake ( & prod, self -> owned, self->curs,
        prodSimpleSerial2Blob, name, & fd, & desc, NULL, prod, chainDecoding );
    if ( rc != 0 )
        return rc;

    /* determine physical encoding */
    enc = phys -> enc;
    if ( enc == NULL )
        enc = smbr -> type;

    /* if unencoded */
    if ( enc == NULL )
        phys -> b2p = prod;
    else
    {
        /* the adapter type should be undefined */
        memset ( & prod -> fd, 0, sizeof prod -> fd );
        prod -> desc . intrinsic_bits = prod -> desc . intrinsic_dim = 1;
        prod -> desc . domain = 0;

        /* create decoding production */
        rc = VProdResolveEncodingExpr ( self, & phys -> b2p,
            prod, ( const SPhysEncExpr* ) enc );
    }

    return rc;
}
