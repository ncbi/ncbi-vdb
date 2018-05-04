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

#define TRACK_REFERENCES 0

#include <vdb/extern.h>

#define KONST const
#include "prod-priv.h"
#include "prod-expr.h"
#include "dbmgr-priv.h"
#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-expr.h"
#include "linker-priv.h"
#include "table-priv.h"
#include "cursor-priv.h"
#include "column-priv.h"
#include "phys-priv.h"
#undef KONST

#include <vdb/manager.h>
#include <vdb/cursor.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * VProdResolve
 */
typedef struct SFunctionPushParam SFunctionPushParam;
struct SFunctionPushParam
{
    /* vectors to allow recursive evaluation of types and functions */
    Vector schema_prior;
    Vector fact_parms, fact_prior;

    VFormatdecl fd;
    VTypedesc desc;
};

static
rc_t SFunctionPush ( const SFunction *self, const VProdResolve *pr,
    const VFormatdecl *lval_fd, const SFuncExpr *expr, SFunctionPushParam *pb )
{
    rc_t rc;
    const VSchema *schema = pr -> schema;

    /* validate input parameter counts against formal list */
    uint32_t count = VectorLength ( & expr -> pfunc );
    if ( count < self -> func . mand )
    {
        rc = RC ( rcVDB, rcFunction, rcResolving, rcParam, rcInsufficient );
        PLOGERR ( klogWarn, ( klogWarn, rc,
                   "function '$(func)' requires $(mand) params but got $(count)",
                   "func=%.*s,mand=%u,count=%u",
                   self->name->name.len, self->name->name.addr,
                   self->func.mand,
                   count
                 ));
    }
    else
    {
        uint32_t end = VectorLength ( & self -> func . parms );
        if ( end < count && ! self -> func . vararg )
        {
            rc = RC ( rcVDB, rcFunction, rcResolving, rcParam, rcExcessive );
            PLOGERR ( klogWarn, ( klogWarn, rc,
                       "function '$(func)' requires $(mand) params but got $(count)",
                       "func=%.*s,mand=%u,count=%u",
                       self->name->name.len, self->name->name.addr,
                       end,
                       count
                     ));
        }
        /* ensure this is not an untyped function */
        else if ( ! self -> validate && self -> rt == NULL )
            rc = RC ( rcVDB, rcFunction, rcResolving, rcFunction, rcIncorrect );
        else
        {
            /* bind schema params */
            rc = SFunctionBindSchemaParms ( self, & pb -> schema_prior, & expr -> schem, pr -> cx_bind );
            if ( rc == 0 )
            {
                /* resolve return type */
                if ( self -> validate )
                {
                    /* set type to "void" */
                    pb -> fd . td . type_id = pb -> fd . td . dim = 1;
                    pb -> fd . fmt = 0;
                    memset ( & pb -> desc, 0, sizeof pb -> desc );
                    pb -> desc . intrinsic_bits = 1;
                    pb -> desc . intrinsic_dim = 1;
                }
                else
                {
                    rc = STypeExprResolveAsFormatdecl
                        ( ( const STypeExpr* ) self -> rt, schema, & pb -> fd, pr -> cx_bind );
                    if ( rc == 0 )
                    {
                        /* pick up cast from lval when return type is "any" or typeset */
                        if ( pb -> fd . td . type_id == 0 ||
                             pb -> fd . td . type_id >= 0x40000000 )
                        {
                            VFormatdecl cast;

                            if ( VFormatdeclCommonAncestor ( & pb -> fd, schema, lval_fd, & cast, NULL ) )
                                pb -> fd = cast;
                            else
                            {
                                const KSymbol *fd_name = NULL;
                                const KSymbol *lval_name = NULL;

                                const SDatatype *dt = VSchemaFindTypeid ( schema, lval_fd -> td . type_id );
                                if ( dt != NULL )
                                    lval_name = dt -> name;

                                dt = VSchemaFindTypeid ( schema, pb -> fd . td . type_id );
                                if ( dt != NULL )
                                    fd_name = dt -> name;
                                else
                                {
                                    const STypeset *ts = VSchemaFindTypesetid ( schema, pb -> fd . td . type_id );
                                    if ( ts != NULL )
                                        fd_name = ts->name;
                                }

                                if ( lval_name != NULL && fd_name != NULL )
                                {
                                    PLOGMSG ( klogWarn, ( klogWarn,
                                               "function '$(func)' could not resolve return type '$(rtype)' in '$(typeset)'",
                                               "func=%.*s,rtype=%.*s,typeset=%.*s",
                                               self->name->name.len, self->name->name.addr,
                                               lval_name->name.len, lval_name->name.addr,
                                               fd_name->name.len, fd_name->name.addr
                                                  ));
                                }
                                else
                                {
                                    PLOGMSG ( klogInt, ( klogInt,
                                               "function '$(func)' could not resolve return type or type names!",
                                               "func=%.*s",
                                               self->name->name.len, self->name->name.addr
                                                 ));
                                }
                            }
                        }

                        /* pick up cast from lval when return dimension is "*" */
                        else if ( pb -> fd . td . dim == 0 )
                        {
                            VTypedesc lh_desc, rh_desc;
                            /* since rh type is T' and is cast to lh type T,
                               missing dimension is sizeof ( T[dim] ) / sizeof ( T' ) */
                            rc = VSchemaDescribeTypedecl ( schema, & lh_desc, & lval_fd -> td );
                            if ( rc == 0 )
                            {
                                VTypedecl rh_td;
                                rh_td . type_id = pb -> fd . td . type_id;
                                rh_td . dim = 1;
                                rc = VSchemaDescribeTypedecl ( schema, & rh_desc, & rh_td );
                                if ( rc == 0 )
                                {
                                    pb -> fd . td . dim = VTypedescSizeof ( & lh_desc ) /
                                        VTypedescSizeof ( & rh_desc );

                                    /* force dimension "*" to be at least 1 */
                                    if ( pb -> fd . td . dim == 0 )
                                        pb -> fd . td . dim = 1;
                                }
                            }
                        }

                        /* evaluate type description */
                        rc = VSchemaDescribeTypedecl ( schema, & pb -> desc, & pb -> fd . td );
                    }
                }

                if ( rc == 0 )
                {
                    /* bind factory params */
                    rc = SFunctionBindFactParms ( self, & pb -> fact_parms, & pb -> fact_prior, & expr -> pfact, pr -> cx_bind );
                    if ( rc == 0 )
                        return 0;
                }

                SFunctionRestSchemaParms ( self, & pb -> schema_prior, pr -> cx_bind );
            }
        }
    }

    LOGERR ( klogWarn, rc, "failed to resolve function expression" );
    return rc;
}

static
void SFunctionPop ( const SFunction *self,
    const VProdResolve *pr, SFunctionPushParam *pb )
{
    SFunctionRestFactParms ( self, & pb -> fact_prior, pr -> cx_bind );
    SFunctionRestSchemaParms ( self, & pb -> schema_prior, pr -> cx_bind );
    VectorWhack ( & pb -> fact_parms, NULL, NULL );
}

static
rc_t VProdResolveFuncParamExpr ( const VProdResolve *self, Vector *out,
    VFormatdecl *fd, const SExpression *expr, const SProduction *sprod )
{
    /* Create a production to resolve the expression; also set its
     * type description */
    VTypedesc desc;
    VProduction *prod = NULL;
    rc_t rc = VProdResolveExpr ( self, & prod, & desc, fd, expr, false );
    if ( rc == 0 )
    {
        if ( prod == NULL )
            rc = RC ( rcVDB, rcFunction, rcResolving, rcProduction, rcIncomplete );
        else
        {
            bool control;
            const char *name;

            if ( sprod == NULL )
            {
                control = false;
                name = "vararg";
            }
            else
            {
                control = sprod -> control;
                name = sprod -> name -> name . addr;
                assert ( name [ sprod -> name -> name . size ] == 0 );
            }

            rc = VSimpleProdMake ( & prod, self -> owned, self-> curs,
                prodSimpleCast, name,fd, & desc, NULL, prod, self -> chain);
            if ( rc == 0 )
            {
                prod -> control = control;
                rc = sprod ?
                    VectorSet ( out, prod -> cid . id = sprod -> cid . id, prod ):
                    VectorAppend ( out, & prod -> cid . id, prod );
                if ( rc != 0 )
                    VProductionWhack ( prod, self -> owned );
            }
        }
    }
    return rc;
}

static
rc_t VProdResolveValidateFuncParams ( const VProdResolve *self, Vector *out,
    const SFormParmlist *formal, const Vector *expr )
{
    rc_t rc;
    VFormatdecl fd;
    const SProduction *sprod;

    uint32_t count = VectorLength ( expr );
    uint32_t end = VectorLength ( & formal -> parms );
    VectorInit ( out, 0, count );

    /* formal param count must be 2 */
    assert ( end == 2 );

    /* input param count must equal formals */
    if ( count != end )
    {
        if ( count > end )
            return RC ( rcVDB, rcFunction, rcResolving, rcParam, rcExcessive );
        return RC ( rcVDB, rcFunction, rcResolving, rcParam, rcInsufficient );
    }

    /* resolve production type */
    sprod = ( const void* ) VectorGet ( & formal -> parms, 0 );
    rc = STypeExprResolveAsFormatdecl
        ( ( const STypeExpr* ) sprod -> fd, self -> schema, & fd, self -> cx_bind );
    if ( rc == 0 )
    {
        /* bring in source parameter */
        const SExpression *pexpr = VectorGet ( expr, 0 );

        /* create copy of resolve block and set to write side */
        VProdResolve pr = * self;
        pr . chain = chainEncoding;

        /* resolve source parameter */
        rc = VProdResolveFuncParamExpr ( & pr, out, & fd, pexpr, sprod );
        if ( rc == 0 )
        {
            /* comparison formal */
            sprod = ( const void* ) VectorGet ( & formal -> parms, 1 );
            rc = STypeExprResolveAsFormatdecl
                ( ( const STypeExpr* ) sprod -> fd, self -> schema, & fd, self -> cx_bind );
            if ( rc == 0 )
            {
                /* bring in comparison parameter */
                pexpr = VectorGet ( expr, 1 );

                /* evaluate on read side */
                pr . chain = chainDecoding;

                /* resolve comparison parameter */
                rc = VProdResolveFuncParamExpr ( & pr, out, & fd, pexpr, sprod );
            }
        }
    }

    return rc;
}

static
rc_t VProdResolveFuncParams ( const VProdResolve *self, Vector *out,
     const SFormParmlist *formal, const Vector *expr )
{
    rc_t rc;
    VFormatdecl fd, ffd;

    uint32_t i, count = VectorLength ( expr );
    uint32_t end = VectorLength ( & formal -> parms );
    VectorInit ( out, 0, count );

    /* assert that there is at least one formal param
       or that we don't have varargs */
    assert ( end != 0 || count == 0 );

    /* resolve named parameters */
    for ( rc = 0, i = 0; rc == 0 && i < count && i < end; ++ i )
    {
        /* resolve production type */
        const SProduction *sprod = ( const void* ) VectorGet ( & formal -> parms, i );
        rc = STypeExprResolveAsFormatdecl
            ( ( const STypeExpr* ) sprod -> fd, self -> schema, & fd, self -> cx_bind );
        if ( rc == 0 )
        {
            /* bring in parameter */
            const SExpression *pexpr = VectorGet ( expr, i );
            ffd = fd;
            rc = VProdResolveFuncParamExpr ( self, out, & fd, pexpr, sprod );
        }
    }

    /* resolve vararg parameters */
    for ( ; rc == 0 && i < count; ++ i )
    {

        const SExpression *pexpr = VectorGet ( expr, i );

#if 0
        /* use type "any" */
        memset ( & fd, 0, sizeof fd );
#else
        /* use last formal type */
        fd = ffd;
#endif

        rc = VProdResolveFuncParamExpr ( self, out, & fd, pexpr, NULL );
    }

    return rc;
}

rc_t VProdResolveScriptExpr ( const VProdResolve *self,
    VProduction **out, const VFormatdecl *lval_fd, const SFuncExpr *expr )
{
    const SFunction *func = expr -> func;

    SFunctionPushParam pb;
    rc_t rc = SFunctionPush ( func, self, lval_fd, expr, & pb );
    if ( rc == 0 )
    {
        Vector *inputs = malloc ( sizeof * inputs );
        if ( inputs == NULL )
            rc = RC ( rcVDB, rcVector, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = VProdResolveFuncParams ( self, inputs,
                & func -> func, & expr -> pfunc );
            if ( rc == 0 )
            {
                /* script name
                   heuristic - known to have been created with NUL term */
                const char *name = expr -> func -> name -> name . addr;

                /* by this time, we have bound all parameters */
                VScriptProd *script;
                rc = VScriptProdMake ( & script, self -> owned, self -> curs,
                    prodScriptFunction, name, & pb . fd, & pb . desc, self -> chain );
                if ( rc == 0 )
                {
                    VCursorCache local;
                    VCursorCacheInit ( & local, 0, 2 );

                    rc = VectorAppend ( & local . cache, NULL, inputs );
                    if ( rc == 0 )
                    {
                        /* create new structure */
                        VProdResolve spr = * self;
                        spr . cache = & local;
                        spr . owned = & script -> owned;

                        /* forget about the inputs */
                        inputs = NULL;

                        /* now evaluate the return expression */
                        rc = VProdResolveExpr ( & spr, & script -> rtn, NULL, & pb . fd,
                            func -> u . script . rtn, false );
                        if ( rc != 0 || script -> rtn == NULL )
                            VProductionWhack ( & script -> dad, self -> owned );
                        else
                        {
                            * out = & script -> dad;
                        }
                    }

                    VCursorCacheWhack ( & local, NULL, NULL );
                }
            }
            else if ( GetRCState ( rc ) == rcIncomplete )
            {
                /* reset errors when input parameters could not be resolved */
                rc = 0;
            }

            if ( inputs != NULL )
            {
                VectorWhack ( inputs, NULL, NULL );
                free ( inputs );
            }
        }

        SFunctionPop ( func, self, & pb );
    }

    return rc;
}

static
rc_t VFunctionProdMakeFactParms ( VFunctionProd *self, const VProdResolve *pr,
    const SFormParmlist *pfact, const Vector *pval, VFactoryParams *fp, const SConstExpr **cxp )
{
    rc_t rc;
    VTypedecl td;
    const VSchema *schema = pr -> schema;
    uint32_t i, last = 0, count = fp -> argc;

    for ( rc = 0, i = 0; rc == 0 && i < count; ++ i )
    {
        /* fetch the formal parameter, which can be NULL
           when the factory parameters allow varargs */
        const SIndirectConst *ic = ( const void* ) VectorGet ( & pfact -> parms, i );
        if ( ic != NULL )
            last = i;
        else
        {
            assert ( i != 0 );

            /* just use the last parameter's type */
            ic = ( const void* ) VectorGet ( & pfact -> parms, last );
            assert ( ic != NULL );
        }

        /* TBD - nobody is using function pointer parameters yet */
        if ( ic -> td == NULL )
        {
            rc = RC ( rcVDB, rcFunction, rcConstructing, rcType, rcNull );
            LOGERR ( klogFatal, rc, "function factory with function param" );
            break;
        }

        /* evaluate constant typedecl */
        rc = STypeExprResolveAsTypedecl ( ( const STypeExpr* ) ic -> td, schema, & td, pr -> cx_bind );
        if ( rc == 0 )
            rc = VSchemaDescribeTypedecl ( schema, & fp -> argv [ i ] . desc, & td );

        if ( rc == 0 )
        {
            /* evaluate constant expression */
            const SExpression *ic_expr = ( const void* ) VectorGet ( pval, i );
            assert ( ic_expr != NULL );
            rc = eval_const_expr ( schema, & td, ic_expr, ( SExpression** ) & cxp [ i ], pr -> cx_bind );
            if ( rc == 0 )
            {
                /* catch undefined factory parameters from outer script */
                if ( cxp [ i ] -> dad . var == eIndirectExpr )
                {
                    if ( fp -> argc > i )
                        fp -> argc = i;
                }
                else if ( cxp [ i ] -> dad . var != eConstExpr )
                {
                    rc = -1;
                    LOGERR (klogFatal, rc, "(cxp[i]->dad.var != eConstExpr)");
                    break;
                }
                else if ( i >= fp -> argc )
                {
                    rc = -1;
                    LOGERR (klogFatal, rc, "(i >= fp->argc)");
                    break;
                }
                else
                {
                    fp -> argv [ i ] . data . u8 = cxp [ i ] -> u . u8;
                    fp -> argv [ i ] . count = cxp [ i ] -> td . dim;
                }
            }
        }
    }

    for ( ; i < count; ++ i )
        cxp [ i ] = NULL;

    return rc;
}

static
rc_t VProdResolveInvokeFactory ( const VProdResolve *self, const VXfactInfo *info,
    VFunctionProd *fprod, const VFactoryParams *cp, const VFunctionParams *dp,
    VTransDesc *td, bool external )
{
    rc_t rc;

    VFuncDesc desc;
    memset ( & desc, 0, sizeof desc );

    rc = ( * td -> factory ) ( td -> fself, info, & desc, cp, dp );
    assert ( rc != -1 );

    /* clobber returned context */
    if ( rc != 0 )
        rc = ResetRCContext ( rc, rcVDB, rcFunction, rcConstructing );

    /* TBD - validate the returned value */
    else if ( external &&
        ( desc . variant == vftInvalid ||
          desc . variant > vftBlob ||
          desc . u . bf == NULL ) )
    {
        rc = RC ( rcVDB, rcFunction, rcConstructing, rcType, rcInvalid );
    }
    else
    {
        fprod -> fself = desc . self;
        fprod -> whack = desc . whack;
        fprod -> u . rf = desc . u . rf;
        fprod -> dad . sub = ( uint8_t ) desc . variant;
    }

    return rc;
}

static
rc_t VProdResolveBuildFuncParms ( const VProdResolve *self, const VXfactInfo *info,
    VFunctionProd *fprod, const VFactoryParams *cp, VTransDesc *td, bool external )
{
    rc_t rc;
    uint32_t i, count;

    /* create a block of function parameters
       start with data on the stack */
    VFunctionParams fpbuff, * fp = & fpbuff;
    count = VectorLength ( & fprod -> parms );
    if ( count > sizeof fpbuff . argv / sizeof fpbuff . argv [ 0 ] )
    {
        fp = malloc ( sizeof fpbuff -
                      sizeof fpbuff . argv +
                      count * sizeof fpbuff . argv [ 0 ] );
        if ( fp == NULL )
            return RC ( rcVDB, rcFunction, rcResolving, rcMemory, rcExhausted );
    }

    fp -> argc = count;

    /* copy types */
    for ( i = 0; i < count; ++ i )
    {
        const VProduction *vprod = ( const void* ) VectorGet ( & fprod -> parms, i );
        fp -> argv [ i ] . fd = vprod -> fd;
        fp -> argv [ i ] . desc = vprod -> desc;
    }

    rc = VProdResolveInvokeFactory ( self, info, fprod, cp, fp, td, external );

    if ( fp != & fpbuff )
        free ( fp );

    return rc;
}

static
rc_t VProdResolveBuildFactParms ( const VProdResolve *self, const VXfactInfo *info,
    VFunctionProd *fprod, const SFormParmlist *pfact, const Vector *pval,
    VTransDesc *td, bool external )
{
    rc_t rc;
    uint32_t i;

    /* count the number of parameters */
    uint32_t count = VectorLength ( pval );

    /* create a block of factory parameters
       start with data on the stack */
    VFactoryParams fpbuff, * fp = & fpbuff;
    const SConstExpr *cxbuff [ 16 ], **cxp = cxbuff;

    /* 9/11/13
       VFactoryParams has a declared array of 16 elements.
       cxbuff is also declared to have 16 elements. the idea
       is that they stay in sync. */
    assert ( sizeof fpbuff . argv / sizeof fpbuff . argv [ 0 ] ==
             sizeof cxbuff / sizeof cxbuff [ 0 ] );

    /* 16 is generally more than enough parameters.
       but in cases where it may not be ( e.g. maps ),
       malloc the memory according to count. */
    if ( count > sizeof fpbuff . argv / sizeof fpbuff . argv [ 0 ] )
    {
        /* allocate both structures in a single block */
        fp = malloc ( sizeof fpbuff -
                      sizeof fpbuff . argv +
                      count * ( sizeof fpbuff . argv [ 0 ] + sizeof cxbuff [ 0 ] ) );

        if ( fp == NULL )
            return RC ( rcVDB, rcFunction, rcResolving, rcMemory, rcExhausted );

        /* reset expression array to point within block */
        cxp = ( const SConstExpr** ) & fp -> argv [ count ];
    }

    fp -> argc = count;
    fp -> align = 0;

    /* the purpose of this function is to bind type expressions to type ids
       before the remainder of the function is evaluated. for this to work,
       we need a mapping of parameterized type ids to expressions.

       the purpose of the cx array is to enable recursion by saving previously
       bound expressions upon entry ... */
    rc = VFunctionProdMakeFactParms ( fprod, self, pfact, pval, fp, cxp );
    assert ( rc != -1 );
    if ( rc == 0 )
        rc = VProdResolveBuildFuncParms ( self, info, fprod, fp, td, external );
    assert ( rc != -1 );

    for ( i = 0; i < count; ++ i )
        SExpressionWhack ( & cxp [ i ] -> dad );

    if ( fp != & fpbuff )
        free ( fp );

    return rc;
}

rc_t VProdResolveFuncExpr ( const VProdResolve *self,
    VProduction **out, const VFormatdecl *lval_fd, const SFuncExpr *expr )
{
    const SFunction *sfunc = expr -> func;
    const char *name = sfunc -> name -> name . addr;

    SFunctionPushParam pb;
    rc_t rc = SFunctionPush ( sfunc, self, lval_fd, expr, & pb );

    assert ( rc != -1 );

    if ( rc == 0 )
    {
        bool external;
        VTransDesc td;
        VXfactInfo info;

        /* pass function type information */
        info . fdesc . fd = pb . fd;
        info . fdesc . desc = pb . desc;

        /* locate factory function */
        rc = VLinkerFindFactory ( self -> ld, self -> libs, & td, sfunc,
            expr -> version_requested ? expr -> version : sfunc -> version,
            & external );
        if ( rc == 0 )
        {
            /* create a production representing function */
            VFunctionProd *fprod;
            rc = VFunctionProdMake ( & fprod, self -> owned,
                self -> curs, vftInvalid, name, & info . fdesc . fd,
                & info . fdesc . desc, self -> chain );
            if ( rc == 0 )
            {
                /* check for a validation function
                   these functions are generally compiler-generated */
                if ( sfunc -> validate )
                {
                    rc = VProdResolveValidateFuncParams ( self, & fprod -> parms,
                        & sfunc -> func, & expr -> pfunc );
                }
                else
                {
                    /* resolve normal function parameters */
                    rc = VProdResolveFuncParams ( self, & fprod -> parms,
                        & sfunc -> func, & expr -> pfunc );
                }

                if ( rc == 0 )
                {
                    /* prepare info block for function factory */
                    info . schema = self -> schema;
                    info . tbl = VCursorGetTable ( self -> curs );
                    info . mgr = info . tbl -> mgr;
                    info . parms = ( struct VCursorParams * ) self -> curs;

                    /* by this point the schema and factory parameters
                       have been bound: named type and constant parameters
                       are entered into the "self -> cx_bind" Vector,
                       factory parameter constants are positionally recorded
                       in "pb . fact_parms", and function parameter productions
                       are recorded in "fprod" */

                    /* the following function will evaluate the parameters
                       and invoke the factory function to construct production */
                    rc = VProdResolveBuildFactParms ( self, & info, fprod,
                        & sfunc -> fact, & pb . fact_parms, & td, external );
                    if ( rc == 0 )
                    {
                        /* successfully created */
                        * out = & fprod -> dad;
                    }
                }

                if ( rc != 0 )
                {
                    /* reset errors when input parameters could not be resolved
                       or if function factory returned an error */
                    if ( GetRCState ( rc ) == rcIncomplete ||
                         ( GetRCTarget ( rc ) == rcFunction && GetRCContext ( rc ) == rcConstructing ) )
                    {
                        rc = 0;
                    }

                    VProductionWhack ( & fprod -> dad, self -> owned );
                }
            }
        }
        else
        {
            /* TBD - perhaps report what went wrong, but if function
               could not be resolved, don't stop the whole process */
            VDB_DEBUG (( "failed to find function '%S' rc %R",
                        & sfunc -> name -> name, rc ));
            rc = 0;
        }

        SFunctionPop ( sfunc, self, & pb );
    }

    return rc;
}


/* resolves physical encoding/decoding expr */
rc_t VProdResolveEncodingExpr ( const VProdResolve *self,
    VProduction **out, VProduction *in, const SPhysEncExpr *expr )
{
    const SPhysical *sphys = expr -> phys;

    /* bind schema and factory params */
    Vector schema_prior;
    rc_t rc = SPhysicalBindSchemaParms ( sphys, & schema_prior, & expr -> schem, self -> cx_bind );
    if ( rc == 0 )
    {
        Vector fact_parms, fact_prior;
        rc = SPhysicalBindFactParms ( sphys, & fact_parms, & fact_prior, & expr -> pfact, self -> cx_bind );
        if ( rc == 0 )
        {
            /* create single input param vector */
            Vector *inputs = malloc ( sizeof * inputs );
            if ( inputs == NULL )
                rc = RC ( rcVDB, rcVector, rcConstructing, rcMemory, rcExhausted );
            else
            {
                VectorInit ( inputs, 0, 1 );
                rc = VectorAppend ( inputs, NULL, in );
                if ( rc == 0 )
                {
                    VFormatdecl fd;

                    if ( self -> chain == chainEncoding )
                        memset ( & fd, 0, sizeof fd );
                    else
                    {
                        rc = STypeExprResolveAsFormatdecl
                            ( ( const STypeExpr* ) sphys -> td, self -> schema, & fd, self -> cx_bind );
                    }

                    if ( rc == 0 )
                    {
                        VScriptProd *script;
                        const char *name = sphys -> name -> name . addr;
                        rc = VScriptProdMake ( & script, self -> owned, self -> curs,
                            prodScriptFunction, name, & fd, NULL, self -> chain );
                        if ( rc == 0 )
                        {
                            /* create a production cache for the encoding */
                            VCursorCache local;
                            VCursorCacheInit ( & local, 0, 2 );

                            rc = VectorAppend ( & local . cache, NULL, inputs );
                            if ( rc == 0 )
                            {
                                const SFunction *func = ( self -> chain == chainEncoding ) ?
                                    & sphys -> encode : & sphys -> decode;

                                VProdResolve spr = * self;
                                spr . cache = & local;
                                spr . owned = & script -> owned;

                                /* forget about input vector */
                                inputs = NULL;

                                /* now evaluate the return expression */
                                rc = VProdResolveExpr ( & spr, & script -> rtn,
                                    & script -> dad . desc, & script -> dad . fd,
                                    func -> u . script . rtn, false );
                                if ( rc != 0 || script -> rtn == NULL )
                                    VProductionWhack ( & script -> dad, self -> owned );
                                else
                                {
                                    * out = & script -> dad;
                                }
                            }

                            VCursorCacheWhack ( & local, NULL, NULL );
                        }
                    }

                    if ( inputs != NULL )
                    {
                        VectorWhack ( inputs, NULL, NULL );
                        free ( inputs );
                    }
                }
            }

            SPhysicalRestFactParms ( sphys, & fact_prior, self -> cx_bind );
            VectorWhack ( & fact_parms, NULL, NULL );
        }

        SPhysicalRestSchemaParms ( sphys, & schema_prior, self -> cx_bind );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * VFunctionProd
 */

/* BuiltInComparison
 */
rc_t VFunctionProdMakeBuiltInComp ( VProduction **prodp, Vector *owned,
    const char *name,  const VProdResolve *resolve, VProduction *orig, VProduction *test )
{
    /* locate factory */
    VTransDesc td;
    rc_t rc = VLinkerFindNamedFactory ( resolve -> ld,
        resolve -> libs, & td, "vdb:compare" );
    if ( rc == 0 )
    {
        const VCursor *curs = resolve -> curs;

        /* build factory params */
        VFuncDesc fd;
        VXfactInfo info;
        VFactoryParams cp;
        VFunctionParams dp;

        info . schema = VCursorGetSchema ( curs );
        info . tbl  = VCursorGetTable ( curs );
        info . mgr = info . tbl -> mgr;
	info . parms = (struct VCursorParams*)curs;
        info . fdesc . fd = orig -> fd;
        info . fdesc . desc = orig -> desc;

        memset ( & fd, 0, sizeof fd );

        /* TBD - fill out % or epsilon for fp compare */
        cp . argc = 0;

        dp . argc = 2;
        dp . argv [ 0 ] . desc = orig -> desc;
        dp . argv [ 0 ] . fd = orig -> fd;
        dp . argv [ 1 ] . desc = test -> desc;
        dp . argv [ 1 ] . fd = test -> fd;

        /* invoke factory */
        rc = ( * td . factory ) ( td . fself, & info, & fd, & cp, & dp );
        if ( rc == 0 )
        {
            VFunctionProd *prod;
            rc = VFunctionProdMake ( & prod, owned, curs,
                fd . variant, name, & orig -> fd, & orig -> desc, chainDecoding );
            if ( rc == 0 )
            {
                prod -> fself = fd . self;
                prod -> whack = fd . whack;
                prod -> u . rf = fd . u . rf;

                /* use orig and test as inputs */
                rc = VectorAppend ( & prod -> parms, NULL, orig );
                if ( rc == 0 )
                {
                    rc = VectorAppend ( & prod -> parms, NULL, test );
                    if ( rc == 0 )
                    {
                        * prodp = & prod -> dad;
                        return 0;
                    }
                }
            }
        }
    }

    return rc;
}
