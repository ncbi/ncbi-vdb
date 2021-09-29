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
#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-expr.h"
#include "schema-dump.h"

#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SIndirectConst
 *  a parameterized constant
 */
#if SLVL >= 3

/* Whack
 */
void CC SIndirectConstWhack ( void *item, void *ignore )
{
    SIndirectConst * self = item;
    SExpressionWhack ( self -> td );
    free ( self );
}

/* Mark
 */
void CC SIndirectConstMark ( void * item, void * data )
{
    const SIndirectConst * self = item;
    if ( self != NULL )
        SExpressionMark ( ( void* ) self -> td, data );
}

/* Dump
 *  dump "const", dump object
 */
rc_t SIndirectConstDump ( const SIndirectConst *self, struct SDumper *d )
{
    return KSymbolDump ( self != NULL ? self -> name : NULL, d );
}

bool CC SIndirectConstDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SIndirectConst *self = ( const void* ) item;

    /* check for this being a function */
    if ( self -> td == NULL )
        b -> rc = SDumperPrint ( b, "function %N", self -> name );
    else
        b -> rc = SDumperPrint ( b, "%E %N", self -> td, self -> name );

    return ( b -> rc != 0 ) ? true : false;
}
#endif

/*--------------------------------------------------------------------------
 * SIndirectType
 */
#if SLVL >= 3

/* Whack
 */
void CC SIndirectTypeWhack ( void *item, void *ignore )
{
    free ( item );
}


/* Find
 */
SIndirectType *VSchemaFindITypeid ( const VSchema *self, uint32_t id )
{
    SIndirectType *pt = VectorGet ( & self -> pt, id );
    while ( pt == NULL )
    {
        self = self -> dad;
        if ( self == NULL )
            break;
        pt = VectorGet ( & self -> pt, id );
    }
    return pt;
}


/* Dump
 */
rc_t SIndirectTypeDump ( const SIndirectType *self, struct SDumper *d )
{
    return KSymbolDump ( self != NULL ? self -> name : NULL, d );
}

#endif

/*--------------------------------------------------------------------------
 * SFormParamlist
 */
#if SLVL >= 3

/* Whack
 */
void SFormParmlistWhack ( SFormParmlist *self, void ( CC * pwhack ) ( void*, void* ) )
{
    VectorWhack ( & self -> parms, pwhack, NULL );
}


/* Mark
 */
void SFormParmlistMark ( const SFormParmlist *self,
    void ( CC * mark ) ( void*, void* ), const VSchema *schema )
{
    if ( self != NULL )
        VectorForEach ( & self -> parms, false, mark, ( void* ) schema );
}


/* Dump
 *  dump param list
 */
rc_t SFormParamlistDump ( const SFormParmlist *self, SDumper *b,
    bool ( CC * dump ) ( void*, void* ),
    const char *begin, const char *end, const char *empty )
{
    rc_t rc;
    void *item;
    uint32_t i, parm_cnt = VectorLength ( & self -> parms );
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    /* if the guy has no specific parameters */
    if ( parm_cnt == 0 )
    {
        if ( self -> vararg )
            return SDumperPrint ( b, "%s...%s", begin, end );
        return SDumperPrint ( b, empty );
    }

    /* print mandatory params */
    SDumperSepString ( b, begin );
    for ( rc = 0, i = 0; i < ( uint32_t ) self -> mand; ++ i )
    {
        item = VectorGet ( & self -> parms, i );
        rc = SDumperSep ( b );
        if ( rc == 0 && ( * dump ) ( item, b ) )
            rc = b -> rc;
        SDumperSepString ( b, compact ? "," : ", " );
    }
    if ( rc != 0 )
        return rc;

    if ( i != 0 )
        SDumperSepString ( b, compact ? "*" : " * " );
    else
    {
        /* issue "begin" */
        rc = SDumperSep ( b );
        if ( rc != 0 )
            return rc;

        SDumperSepString ( b, compact ? "*" :  "* " );
    }

    /* print optional params */
    for ( ; i < parm_cnt; ++ i )
    {
        item = VectorGet ( & self -> parms, i );
        rc = SDumperSep ( b );
        if ( rc == 0 && ( * dump ) ( item, b ) )
            rc = b -> rc;
        SDumperSepString ( b, compact ? "," : ", " );
    }
    if ( rc != 0 )
        return rc;

    /* print optional vararg */
    if ( self -> vararg )
        return SDumperPrint ( b, compact ? ",...%s" : ", ...%s", end );

    /* close */
    return SDumperPrint ( b, end );
}

#endif

/*--------------------------------------------------------------------------
 * SFunction
 */

#if SLVL >= 3

/* Whack
 */
void SFunctionDestroy ( SFunction *self )
{
    SExpressionWhack ( self -> rt );
#if SLVL >= 4
    if ( self -> script )
    {
        SExpressionWhack ( self -> u . script . rtn );
        VectorWhack ( & self -> u . script . prod, SProductionWhack, NULL );
    }
#endif
    BSTreeWhack ( & self -> sscope, KSymbolWhack, NULL );
    BSTreeWhack ( & self -> fscope, KSymbolWhack, NULL );
    SFormParmlistWhack ( & self -> fact, SIndirectConstWhack );
    SFormParmlistWhack ( & self -> func, SProductionWhack );
    VectorWhack ( & self -> type, NULL, NULL );
    VectorWhack ( & self -> schem, SIndirectConstWhack, NULL );
}

void CC SFunctionWhack ( void *self, void *ignore )
{
    SFunctionDestroy ( self );
    free ( self );
}

/* Cmp
 * Sort
 */
int64_t CC SFunctionCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SFunction *b = n;

    if ( * a > b -> version )
        return 1;
    return ( int64_t ) ( * a >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

int64_t CC SFunctionSort ( const void *item, const void *n )
{
    const SFunction *a = item;
    const SFunction *b = n;

    return ( int64_t ) ( a -> version >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

#if _DEBUGGING && 0
static String no_name = { "<no-name>", sizeof "<no-name>" - 1, sizeof "<no-name>" - 1 };
#define DBG_CXBIND1( op, name, id, val ) \
    OUTMSG (( "%s:%d - cx_bind %s: name=%S, id=%u, val=%p\n", __func__, __LINE__, op, & name, id, val ))
#define DBG_CXBIND2( op, name, id, old, new ) \
    OUTMSG (( "%s:%d - cx_bind %s: name=%S, id=%u, old=%p, new=%p\n", __func__, __LINE__, op, & name, id, old, new ))
#else
#define DBG_CXBIND1( op, name, id, val ) \
    ( ( void ) 0 )
#define DBG_CXBIND2( op, name, id, old, new ) \
    ( ( void ) 0 )
#endif

/* Bind
 *  perform schema and factory param substitution
 *  returns prior param values
 *
 *  9/11/13
 *  "self" is a cursor-local cloned version of schema decl
 */
rc_t SFunctionBindSchemaParms ( const SFunction *self,
    Vector *prior, const Vector *subst, Vector *cx_bind )
{
    rc_t rc = 0;
    uint32_t i, count;
    void *cx_old, *cx_new;

    const SIndirectType *id;
    const SIndirectConst *ic;

    /* count input params
       the first bunch are types
       the remainder are constants */
    uint32_t expected = VectorLength ( & self -> type ) + VectorLength ( & self -> schem );
    uint32_t actual = VectorLength ( subst );

    /* initialize return value
       the prior values act as a stack for recursion */
    VectorInit ( prior, 0, actual );

    /* param counts must match */
    if ( actual < expected )
        rc = RC ( rcVDB, rcFunction, rcEvaluating, rcParam, rcInsufficient );
    else if ( actual > expected )
        rc = RC ( rcVDB, rcFunction, rcEvaluating, rcParam, rcExcessive );
    if ( rc != 0 )
    {
        PLOGERR ( klogWarn, ( klogWarn, rc,
                 "schema parameter count mismatch - function: '$(f)'; expected $(i), received $(count)",
                 "f=%.*s,count=%u,i=%u",
                 self -> name ? self -> name -> name . size : 0,
                 self -> name ? self -> name -> name . addr : NULL,
                 expected, actual ));
        return rc;
    }

    /* bind types */
    count = VectorLength ( & self -> type );
    for ( i = 0; i < count; ++ i )
    {
        /* get the indirect type object */
        id = VectorGet ( & self -> type, i );
        assert ( id != NULL );

        /* get the new type expression */
        cx_new = VectorGet ( subst, id -> pos );
        assert ( cx_new != NULL );

        /* update the binding vector */
        rc = VectorSwap ( cx_bind, id -> type_id, cx_new, & cx_old );
        if ( rc != 0 )
            break;
        DBG_CXBIND2 ( "bind schema type", id -> name -> name, id -> type_id, cx_old, cx_new );

        /* save old value on stack for recursion */
        rc = VectorSet ( prior, id -> pos, cx_old );
        if ( rc != 0 )
        {
            cx_new = cx_old;
            VectorSwap ( cx_bind, id -> type_id, cx_new, & cx_old );
            DBG_CXBIND2 ( "revert bind type", id -> name -> name, id -> type_id, cx_old, cx_new );
            break;
        }
    }

    /* bind constants */
    if ( rc == 0 )
    {
        count = VectorLength ( & self -> schem );
        for ( i = 0 ; i < count; ++ i )
        {
            ic = VectorGet ( & self -> schem, i );
            assert ( ic != NULL );

            /* get the new constant value expression */
            cx_new = VectorGet ( subst, ic -> pos );
            assert ( cx_new != NULL );

            /* update the binding vector */
            rc = VectorSwap ( cx_bind, ic -> expr_id, cx_new, & cx_old );
            if ( rc != 0 )
                break;
            DBG_CXBIND2 ( "bind schema const", ic -> name -> name, ic -> expr_id, cx_old, cx_new );

            /* save old value on stack for recursion */
            rc = VectorSet ( prior, ic -> pos, cx_old );
            if ( rc != 0 )
            {
                cx_new = cx_old;
                VectorSwap ( cx_bind, ic -> expr_id, cx_new, & cx_old );
                DBG_CXBIND2 ( "revert bind const", ic -> name -> name, ic -> expr_id, cx_old, cx_new );
                break;
            }
        }

        /* if there was no error, we're done */
        if ( rc == 0 )
            return 0;

        /* reverse the damage done by binding constants */
        while ( i -- > 0 )
        {
            ic = VectorGet ( & self -> schem, i );
            assert ( ic != NULL );

            /* get the old constant value expression */
            cx_new = VectorGet ( prior, ic -> pos );
            assert ( cx_new != NULL );

            /* restore the binding vector */
            VectorSwap ( cx_bind, ic -> expr_id, cx_new, & cx_old );
            DBG_CXBIND2 ( "revert bind const", ic -> name -> name, ic -> expr_id, cx_old, cx_new );
        }

        /* reset i */
        i = VectorLength ( & self -> type );
    }

    /* reverse the damage done by binding types */
    while ( i -- > 0 )
    {
        /* get the indirect type object */
        id = VectorGet ( & self -> type, i );
        assert ( id != NULL );

        /* get the old type expression */
        cx_new = VectorGet ( prior, id -> pos );
        assert ( cx_new != NULL );

        /* restore the binding vector */
        VectorSwap ( cx_bind, id -> type_id, cx_new, & cx_old );
        DBG_CXBIND2 ( "revert bind type", id -> name -> name, id -> type_id, cx_old, cx_new );
    }

    /* a non-zero rc indicates the Vector is invalid */
    VectorWhack ( prior, NULL, NULL );
    return rc;
}

rc_t SFunctionBindFactParms ( const SFunction *self,
    Vector *parms, Vector *prior, const Vector *subst, Vector *cx_bind )
{
    rc_t rc = 0;
    SIndirectConst *ic;
    void *cx_old, *cx_new;
    uint32_t i, /*count,*/ act_count, form_count;

    /* count input params */
    /*count =*/ act_count = VectorLength ( subst );

    /* must have minimum count */
    if ( act_count < self -> fact . mand )
    {
        rc =  RC ( rcVDB, rcFunction, rcEvaluating, rcParam, rcInsufficient );
        PLOGERR ( klogWarn, ( klogWarn, rc,
                   "missing mandatory factory parameters - function: '$(func)'; expected $(mand), received $(count)",
                   "func=%.*s,mand=%u,count=%u",
                   self -> name -> name . size, self -> name -> name . addr,
                   self -> fact . mand, act_count ));
        return rc;
    }

    /* test against maximum count */
    form_count = VectorLength ( & self -> fact . parms );
    if ( act_count > form_count )
    {
        if ( ! self -> fact . vararg )
        {
            rc = RC ( rcVDB, rcFunction, rcEvaluating, rcParam, rcExcessive );
            PLOGERR ( klogWarn, ( klogWarn, rc,
                       "extra factory parameters - function: '$(func)'; expected $(mand), received $(count)",
                       "func=%.*s,mand=%u,count=%u",
                       self -> name -> name . size, self -> name -> name . addr,
                       form_count, act_count ));
            return rc;
        }
        /*count = form_count;*/
    }

    /* initialize return values
       the "parms" vector is a positional vector of expressions,
       and is the only way to get at varargs.
       the prior values act as a stack for recursion */
    VectorInit ( parms, 0, act_count );
    VectorInit ( prior, 0, form_count );

    /* bind actual formal parameter values */
    for ( i = 0; i < form_count && i < act_count; ++ i )
    {
        /* get the indirect constant object */
        ic = VectorGet ( & self -> fact . parms, i );
        assert ( ic != NULL );

        /* get the new value expression */
        assert ( ic -> pos == i );
        cx_new = VectorGet ( subst, i );
        assert ( cx_new != NULL );

        /* update the positional vector */
        rc = VectorAppend ( parms, NULL, cx_new );
        if ( rc != 0 )
            break;

        /* update the binding vector */
        rc = VectorSwap ( cx_bind, ic -> expr_id, cx_new, & cx_old );
        if ( rc != 0 )
            break;
        DBG_CXBIND2 ( "bind fact const", ic -> name -> name, ic -> expr_id, cx_old, cx_new );

        /* save the old value on stack for recursion */
        rc = VectorSet ( prior, i, cx_old );
        if ( rc != 0 )
        {
            cx_new = cx_old;
            VectorSwap ( cx_bind, ic -> expr_id, cx_new, & cx_old );
            DBG_CXBIND2 ( "revert bind const", ic -> name -> name, ic -> expr_id, cx_old, cx_new );
            break;
        }
    }

    if ( rc == 0 )
    {
        /* this loop should only actually execute
           to record missing optional formals */
        assert ( i == form_count || ( i == act_count && act_count < form_count ) );

        /* record optional formal parameter values */
        for ( ; i < form_count; ++ i )
        {
            /* get the indirect constant object */
            ic = VectorGet ( & self -> fact . parms, i );
            assert ( ic != NULL );

            /* get the new value expression */
            assert ( ic -> pos == i );
            cx_new = VectorGet ( subst, i );

            /* save the same value on stack for recursion */
            rc = VectorSet ( prior, i, cx_new );
            if ( rc != 0 )
                break;

            DBG_CXBIND1 ( "ignore optional fact const", ic -> name -> name, ic -> expr_id, cx_new );
        }
    }

    if ( rc == 0 )
    {
        /* this loop should only actually execute
           to record vararg params beyond all formals */
        assert ( i == form_count );

        /* set vararg values */
        for ( ; i < act_count; ++ i )
        {
            /* get the extra value expression */
            cx_new = VectorGet ( subst, i );
            assert ( cx_new != NULL );

            /* update the positional vector */
            rc = VectorAppend ( parms, NULL, cx_new );
            if ( rc != 0 )
                break;

            DBG_CXBIND1 ( "vararg fact const", no_name, 0, cx_new );
        }

        if ( rc == 0 )
            return 0;

        i = form_count;
    }

    while ( i -- > 0 )
    {
        /* get the indirect type object */
        ic = VectorGet ( & self -> fact . parms, i );
        assert ( ic != NULL );

        /* get the old type expression */
        cx_new = VectorGet ( subst, ic -> pos );
        assert ( cx_new != NULL );

        /* restore the binding vector */
        VectorSwap ( cx_bind, ic -> expr_id, cx_new, & cx_old );
        DBG_CXBIND2 ( "revert bind const", ic -> name -> name, ic -> expr_id, cx_old, cx_new );
    }

    VectorWhack ( parms, NULL, NULL );
    VectorWhack ( prior, NULL, NULL );

    return rc;
}

/* Rest-ore
 *  restore schema and factory param substitution
 *  destroys prior param vector
 */
void SFunctionRestSchemaParms ( const SFunction *self, Vector *prior, Vector *cx_bind )
{
    rc_t rc;
    uint32_t i, count;
    void * cx_old, * ignore;

    /* must have whole thing in prior */
    assert ( VectorLength ( prior ) == VectorLength ( & self -> type ) + VectorLength ( & self -> schem ) );

    /* restore prior values */
    count = VectorLength ( & self -> type );
    for ( i = 0; i < count; ++ i )
    {
        /* get the indirect type object */
        const SIndirectType *id = VectorGet ( & self -> type, i );
        assert ( id != NULL );

        /* get the old type expression */
        cx_old = VectorGet ( prior, id -> pos );

        /* update the binding vector */
        rc = VectorSwap ( cx_bind, id -> type_id, cx_old, & ignore );
        assert ( rc == 0 );

        DBG_CXBIND2 ( "restore bind type", id -> name -> name, id -> type_id, ignore, cx_old );
    }

    count = VectorLength ( & self -> schem );
    for ( i = 0; i < count; ++ i )
    {
        const SIndirectConst *ic = VectorGet ( & self -> schem, i );
        assert ( ic != NULL );

        /* get the old constant value expression */
        cx_old = VectorGet ( prior, ic -> pos );

        /* restore the binding vector */
        rc = VectorSwap ( cx_bind, ic -> expr_id, cx_old, & ignore );
        assert ( rc == 0 );

        DBG_CXBIND2 ( "restore bind const", ic -> name -> name, ic -> expr_id, ignore, cx_old );
    }

    VectorWhack ( prior, NULL, NULL );
}

void SFunctionRestFactParms ( const SFunction *self, Vector *prior, Vector *cx_bind )
{
    rc_t rc;
    uint32_t i, count;
    void * cx_old, * ignore;

    /* must have whole thing in prior */
    assert ( VectorLength ( prior ) == VectorLength ( & self -> fact . parms ) );

    /* restore prior values */
    count = VectorLength ( & self -> fact . parms );
    for ( i = 0; i < count; ++ i )
    {
        const SIndirectConst *ic = VectorGet ( & self -> fact . parms, i );
        assert ( ic != NULL );

        /* get the old constant value expression */
        cx_old = VectorGet ( prior, ic -> pos );

        /* restore the binding vector */
        rc = VectorSwap ( cx_bind, ic -> expr_id, cx_old, & ignore );
        assert ( rc == 0 );

        DBG_CXBIND2 ( "restore bind const", ic -> name -> name, ic -> expr_id, ignore, cx_old );
    }

    VectorWhack ( prior, NULL, NULL );
}


/* Mark
 */
void CC SFunctionClearMark ( void *item, void *ignore )
{
    SFunction *self = item;
    self -> marked = false;
}

void CC SFunctionMark ( void * item, void * data )
{
    SFunction * self = item;
    const VSchema * schema = data;
    if ( self != NULL && ! self -> marked )
    {
        self -> marked = true;
        SExpressionMark ( ( void * )self -> rt, data );
        SFormParmlistMark ( & self -> fact, SIndirectConstMark, schema );
        SFormParmlistMark ( & self -> func, SProductionMark, schema );
        VectorForEach ( & self -> schem, false, SIndirectConstMark, data );

        if ( self -> script )
        {
            SExpressionMark ( ( void * )self -> u . script . rtn, ( void * )schema );
            VectorForEach ( & self -> u . script . prod, false, SProductionMark, data );
        }
    }
}

void SFuncNameMark ( const SNameOverload *self, const VSchema *schema )
{
    if ( self != NULL )
    {
        VectorForEach ( & self -> items, false, SFunctionMark, ( void * ) schema );
    }
}


/* Dump
 */
rc_t SFunctionDump ( const SFunction *self, struct SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}

rc_t SFunctionDeclDumpSchemaParms ( const SFunction *self, SDumper *b )
{
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    /* this first part is weird, because the types and
       constants are kept separately, although they were
       specified in a single list */
    uint32_t i, j, sparm_cnt = VectorLength ( & self -> type ) +
        VectorLength ( & self -> schem );
    if ( sparm_cnt == 0 )
        return 0;

    SDumperSepString ( b, compact ? "<" : "< " );
    for ( i = j = 0; i < sparm_cnt; ++ i )
    {
        const SIndirectType *id = VectorGet ( & self -> type, i - j );
        rc_t rc = SDumperSep ( b );
        if ( rc != 0 )
            return rc;
        if ( id != NULL && id -> pos == i )
            rc = SDumperPrint ( b, "type %N", id -> name );
        else
        {
            const SIndirectConst *ic = VectorGet ( & self -> schem, j );
            assert ( id == NULL || id -> pos > i );
            if ( ic == NULL )
                rc = SDumperWrite ( b, "NULL", 4 );
            else
                SIndirectConstDefDump ( ( void* ) ic, b );
        }
        if ( rc != 0 )
            return rc;
        SDumperSepString ( b, compact ? "," : ", " );
    }

    return SDumperPrint ( b, compact ? ">" : " > " );
}

rc_t SFunctionDeclDumpFactParms ( const SFunction *self, SDumper *b )
{
    if ( SDumperMode ( b ) == sdmCompact )
        return SFormParamlistDump ( & self -> fact, b, SIndirectConstDefDump, "<", ">", "" );
    return SFormParamlistDump ( & self -> fact, b, SIndirectConstDefDump, " < ", " >", "" );
}

bool CC SFunctionDeclDump ( void *item, void *data )
{
    SDumper *b = data;
    const SFunction *self = ( const void* ) item;
    const char *func_class = "extern";

    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    if ( self -> script )
        func_class = "schema";
    else if ( self -> validate )
        func_class = "validate";

    /* a type of function */
    b -> rc = SDumperPrint ( b, "%s function ", func_class );
    if ( b -> rc == 0 )
    {
        if ( self -> untyped )
            b -> rc = SDumperPrint ( b, compact ? "__untyped %N()" : "__untyped %N ()", self -> name );
        else if ( self -> row_length )
            b -> rc = SDumperPrint ( b, compact ? "__row_length %N()" : "__row_length %N ()", self -> name );
        else
        {
            /* could have schema parameters */
            b -> rc = SFunctionDeclDumpSchemaParms ( self, b );

            /* a return type expression, followed by a function name */
            if ( b -> rc == 0 )
            {
                if ( self -> validate )
                    b -> rc = SDumperPrint ( b, "void %N", self -> name );
                else
                    b -> rc = SDumperPrint ( b, "%E %N", self -> rt, self -> name );
            }

            /* version should be given */
            if ( b -> rc == 0 )
                b -> rc = SDumperVersion ( b, self -> version );

            /* factory parameters */
            if ( b -> rc == 0 )
                b -> rc = SFunctionDeclDumpFactParms ( self, b );

            /* function parameters */
            if ( b -> rc == 0 )
            {
                if ( compact )
                    b -> rc = SFormParamlistDump ( & self -> func, b, SProductionDefDump, "(", ")", "()" );
                else
                    b -> rc = SFormParamlistDump ( & self -> func, b, SProductionDefDump, " ( ", " )", " ()" );
            }
        }
    }

    if ( b -> rc == 0 )
    {
#if SLVL >= 4
        if ( self -> script )
        {
            if ( ! compact )
                b -> rc = SDumperWrite ( b, "\n", 1 );
            if ( b -> rc == 0 )
                b -> rc = SFunctionBodyDump ( self, b );
        }
        else
#endif
        {
            if ( self -> u . ext . fact != NULL )
                b -> rc = SDumperPrint ( b, compact ? "=%N" : " = %N", self -> u . ext . fact );
            if ( b -> rc == 0 )
                b -> rc = SDumperPrint ( b, compact ? ";" : ";\n" );
        }
    }

    if ( b -> rc == 0 )
        b -> rc = AliasDump ( self -> name, b );

    return ( b -> rc != 0 ) ? true : false;
}

#endif


/*--------------------------------------------------------------------------
 * VSchema
 */

#if SLVL >= 3

/*
 * formal-symbol     = ID
 */
static
rc_t formal_symbol ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env,  uint32_t id, const void *obj )
{
    rc_t rc;

    /* allow symbol redefines in current scope only */
    if ( t -> sym != NULL )
    {
        KTokenSourceReturn ( src, t );
        next_shallow_token ( tbl, src, t, true );
    }

    /* must have a parameter name */
    if ( t -> id != eIdent )
        return KTokenExpected ( t, klogErr, "undefined identifier" );

    /* create the symbol in current scope */
    rc = KSymTableCreateSymbol ( tbl, & t -> sym, & t -> str, id, obj );
    if ( rc != 0 )
        KTokenRCExplain ( t, klogInt, rc );

    return rc;
}


/*
 * param-formal       = [ 'control' ] <typespec> ID
 */
static
rc_t param_formal ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFormParmlist *sig )
{
    rc_t rc;

    /* create the factory formal parameter,
       which is just a production awaiting an expr */
    SProduction *param = malloc ( sizeof * param );
    if ( param == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    /* finish initialization */
    memset ( param, 0, sizeof * param );

    /* accept 'control' keyword */
    if ( t -> id == kw_control )
    {
        param -> control = true;
        next_token ( tbl, src, t );
    }

    /* if parsing v0 text, then this is a persisted
       column schema. it will have only a formal param
       name, but no type. substitute "any" */
    if ( env -> schema_param_types_absent )
    {
        KTokenSourceReturn ( src, t );
        CONST_STRING ( & t -> str, "any" );
        t -> sym = KSymTableFindIntrinsic ( tbl, & t -> str );
        assert ( t -> sym != NULL );
        t -> id = t -> sym -> type;
    }

    /* should start off with a type */
    rc = vardim_type_expr ( tbl, src, t, env, self, & param -> fd );
    if ( rc != 0 )
    {
        free ( param );
        return rc;
    }

    /* create a name */
    rc = formal_symbol ( tbl, src, t, env, eFuncParam, param );
    if ( rc != 0 )
    {
        SProductionWhack ( param, NULL );
        return rc;
    }
    param -> name = t -> sym;

    /* append to param list */
    rc = VectorAppend ( & sig -> parms, & param -> cid . id, param );
    if ( rc != 0 )
    {
        SProductionWhack ( param, NULL );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    next_token ( tbl, src, t );
    return 0;
}


/*
 * fact-formals       = <fact-formal> [ ',' <fact-formals> ]
 * fact-formal        = <fact-typedecl> ID
 * fact-parmname      = ID
 */
static
rc_t fact_formal ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFormParmlist *sig )
{
    rc_t rc;
    SIndirectConst *param = malloc ( sizeof * param );
    if ( param == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    param -> td = NULL;
    param -> expr_id = 0;

#if ACCEPT_FUNCTION_AS_FACT_PARAM
    /* type could be 'function' */
    if ( t -> id == kw_function )
        next_token ( tbl, src, t );

    /* should be a typedecl */
    else
#endif
    {
        rc = type_expr ( tbl, src, t, env, self, & param -> td );
        if ( rc != 0 )
        {
            free ( param );
            return KTokenFailure ( t, klogErr, rc,
#if ACCEPT_FUNCTION_AS_FACT_PARAM
                                   "function or "
#endif
                                   "data type" );
        }
    }

    /* get its name */
    rc = formal_symbol ( tbl, src, t, env, eFactParam, param );
    if ( rc != 0 )
    {
        SIndirectConstWhack ( param, NULL );
        return rc;
    }
    param -> name = t -> sym;

    /* store as a parameter */
    rc = VectorAppend ( & sig -> parms, & param -> pos, param );
    if ( rc != 0 )
    {
        SIndirectConstWhack ( param, NULL );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    /* set binding constant */
    param -> expr_id = ++ self -> num_indirect;

    next_token ( tbl, src, t );
    return 0;
}


/*
 * formal-params     = <formal-param> [ ',' <formal-params> ]
 */
static
rc_t formal_params ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFormParmlist *sig,
    rc_t ( * formal_param ) ( KSymTable*, KTokenSource*, KToken*,
        const SchemaEnv*, VSchema*, SFormParmlist* ) )
{
    while ( t -> sym != NULL || t -> id == eIdent )
    {
        rc_t rc = ( * formal_param ) ( tbl, src, t, env, self, sig );
        if ( rc != 0 )
            return rc;

        if ( t -> id != eComma )
            break;

        next_token ( tbl, src, t );
    }

    return 0;
}


/*
 * formal-signature   = <formal-params> [ '*' <formal-params> ] [',' '...' ]
 *                    | '*' <formal-params> [',' '...' ]
 *                    | '...'
 */
static
rc_t formal_signature ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFormParmlist *sig,
    rc_t ( * formal_param ) ( KSymTable*, KTokenSource*, KToken*,
        const SchemaEnv*, VSchema*, SFormParmlist* ) )
{
    /* read mandatory parameters */
    rc_t rc = formal_params ( tbl, src, t, env, self, sig, formal_param );

    /* remember the number of mandatory params seen */
    sig -> mand = VectorLength ( & sig -> parms );

    /* read optional parameters */
    if ( rc == 0 && t -> id == eAsterisk )
    {
        next_token ( tbl, src, t );
        rc = formal_params ( tbl, src, t, env, self, sig, formal_param );
        if ( rc == 0 && VectorLength ( & sig -> parms ) == sig -> mand )
            KTokenExpected ( t, klogWarn, "optional parameter" );
    }

    /* accept '...' */
    if ( t -> id == eEllipsis )
    {
        /* but only if there was at least one real parameter */
        if ( VectorLength ( & sig -> parms ) == 0 )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcParam, rcInsufficient );
            return KTokenFailure ( t, klogErr, rc, "vararg parameter requires at least one real parameter" );
        }

        sig -> vararg = true;
        next_token ( tbl, src, t );
    }

    return 0;
}

/*
 * parm-signature     = <parm-formals> [ '*' <parm-formals> ] [',' '...' ]
 *                    | '*' <parm-formals> [',' '...' ]
 *                    | '...'
 */
static
rc_t parm_signature ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFormParmlist *sig )
{
    /* open list */
    rc_t rc = expect ( tbl, src, t, eLeftParen, "(", true );
    if ( rc != 0 )
        return rc;

    /* parse list */
    rc = formal_signature ( tbl, src, t, env, self, sig, param_formal );
    if ( rc != 0 )
        return 0;

    /* expect close */
    return expect ( tbl, src, t, eRightParen, ")", true );
}

/*
 * fact-signature     = <fact-formals> [ '*' <fact-formals> ] [ ',' '...' ]
 *                    | '*' <fact-formals> [ ',' '...' ]
 *                    | '...'
 */
rc_t fact_signature ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFormParmlist *sig )
{
    /* open list */
    rc_t rc = expect ( tbl, src, t, eLeftAngle, "<", true );
    if ( rc != 0 )
        return rc;

    /* parse list */
    rc = formal_signature ( tbl, src, t, env, self, sig, fact_formal );
    if ( rc != 0 )
        return rc;

    /* expect close */
    return expect ( tbl, src, t, eRightAngle, ">", true );
}


/*
 * schema-signature   = <schema-formals>
 * schema-formals     = <schema-formal> [ ',' <schema-formals> ]
 * schema-formal      = <schema-typedecl> ID
 * schema-parmname    = ID
 */
rc_t schema_signature ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFunction *sig )
{
    rc_t rc;

    /* open list */
    if ( t -> id != eLeftAngle )
        return KTokenExpected ( t, klogErr, "<" );

    /* gather schema parameters */
    do
    {
        const SExpression *td;
        const SDatatype *dt;

        /* check parameter type */
        switch ( next_token ( tbl, src, t ) -> id )
        {
        case kw_type:
            td = NULL;
            next_token ( tbl, src, t );
            break;

        case eDatatype:
            dt = t -> sym -> u . obj;
            if ( dt -> domain == ddUint )
            {
                /* evaluate the type expression */
                rc = type_expr ( tbl, src, t, env, self, & td );
                if ( rc != 0 )
                    return KTokenFailure ( t, klogErr, rc, "unsigned integer datatype" );

                /* the type should be totally resolved */
                assert ( td != NULL && td -> var == eTypeExpr );
                assert ( ( ( const STypeExpr* ) td ) -> resolved );
                if ( ( ( const STypeExpr* ) td ) -> fd . td . dim == 1 )
                    break;

                SExpressionWhack ( td );
                return KTokenExpected ( t, klogErr, "single dimensional unsigned integer datatype" );
            }

        default:
            return KTokenExpected ( t, klogErr, "type keyword or unsigned integer datatype" );
        }

        /* get parameter name */
        if ( t -> id != eIdent )
        {
            if ( td != NULL )
                SExpressionWhack ( td );
            return KTokenExpected ( t, klogErr, "parameter name" );
        }

        if ( td == NULL )
        {
            SIndirectType *formal = malloc ( sizeof * formal );
            if ( formal == NULL )
            {
                rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
                return KTokenRCExplain ( t, klogInt, rc );
            }

            /* initialize to raw format,
               undefined type, and no dimension */
            formal -> type_id = 0;

            /* create symbol */
            rc = KSymTableCreateConstSymbol ( tbl, & formal -> name,
                & t -> str, eSchemaType, formal );
            if ( rc == 0 )
            {
                /* record positional */
                rc = VectorAppend ( & sig -> type, & formal -> pos, formal );
                if ( rc == 0 )
                {
                    void *ignore;

                    /* record formal */
                    rc = VectorAppend ( & self -> pt, & formal -> id, formal );
                    if ( rc != 0 )
                        VectorSwap ( & sig -> type, formal -> pos, NULL, & ignore );
                    else
                        formal -> pos += VectorLength ( & sig -> schem );
                }
            }
            if ( rc != 0 )
            {
                free ( formal );
                return KTokenRCExplain ( t, klogInt, rc );
            }

            formal -> type_id = ++ self -> num_indirect;
        }
        else
        {
            SIndirectConst *formal = malloc ( sizeof * formal );
            if ( formal == NULL )
            {
                SExpressionWhack ( td );
                rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
                return KTokenRCExplain ( t, klogInt, rc );
            }

            /* initialize with no value */
            formal -> expr_id = 0;
            formal -> td = td;

            /* create symbol */
            rc = KSymTableCreateConstSymbol ( tbl, & formal -> name,
                & t -> str, eSchemaParam, formal );
            if ( rc == 0 )
            {
                /* record formal */
                rc = VectorAppend ( & sig -> schem, & formal -> pos, formal );
                if ( rc == 0 )
                    formal -> pos += VectorLength ( & sig -> type );
            }
            if ( rc != 0 )
            {
                SIndirectConstWhack ( formal, NULL );
                return KTokenRCExplain ( t, klogInt, rc );
            }

            formal -> expr_id = ++ self -> num_indirect;
        }
    }
    while ( next_token ( tbl, src, t ) -> id == eComma );

    /* expect close */
    return expect ( tbl, src, t, eRightAngle, ">", true );
}

static
rc_t return_type_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFunction *f )
{
    if ( f -> validate )
        return expect ( tbl, src, t, kw_void, "void", true );
    return vardim_type_expr ( tbl, src, t, env, self, & f -> rt );
}


static
rc_t func_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFunction *f, uint32_t type )
{
    rc_t rc;

    /* check for __untyped or __row_length function */
    if ( t -> id == kw___untyped || t -> id == kw___row_length )
    {
        uint32_t sym_type;

        /* can't be a script function */
        if ( type == eScriptFunc )
            return KTokenExpected ( t, klogErr, "script function return type" );

        /* can't be a validate function */
        if ( f -> validate )
            return KTokenExpected ( t, klogErr, "void return type" );

        /* determine variant */
        if ( t -> id == kw___untyped )
        {
            f -> untyped = true;
            sym_type = eUntypedFunc;
        }
        else
        {
            f -> row_length = true;
            sym_type = eRowLengthFunc;
        }

        /* treat keyword as a NULL return type */
        next_token ( tbl, src, t );

        /* create function name */
        rc = create_fqn ( tbl, src, t, env, sym_type, NULL );
        if ( rc != 0 )
        {
            if ( GetRCState ( rc ) == rcExists )
                return rc;
            return KTokenFailure ( t, klogErr, rc, "fully qualified name" );
        }

        /* record symbol */
        f -> name = t -> sym;
        next_token ( tbl, src, t );

        /* consume empty param list */
        rc = expect ( tbl, src, t, eLeftParen, "(", true );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eRightParen, ")", true );
        return rc;
    }


    /* initialize vectors */
    VectorInit ( & f -> fact . parms, 0, 8 );
    VectorInit ( & f -> func . parms, 0, 8 );
    VectorInit ( & f -> type, 0, 8 );
    VectorInit ( & f -> schem, 0, 8 );

    /* get schema signature */
    if ( t -> id == eLeftAngle )
    {
        /* enter schema param scope */
        rc = KSymTablePushScope ( tbl, & f -> sscope );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );

        /* parse schema params */
        rc = schema_signature ( tbl, src, t, env, self, f );

        /* interpret return type within schema param scope */
        if ( rc == 0 )
        {
            rc = return_type_expr ( tbl, src, t, env, self, f );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "return type" );
        }

        /* pop scope */
        KSymTablePopScope ( tbl );

        /* bail on problems */
        if ( rc != 0 )
            return rc;

        /* "t" contains a lookahead token. while unlikely,
           it could have matched something defined in schema
           param scope. re-evaluate if possible */
        if ( t -> sym != NULL ) switch ( t -> sym -> type )
        {
        case eSchemaType:
        case eSchemaParam:
            t -> id = eIdent;
            t -> sym = KSymTableFind ( tbl, & t -> str );
            if ( t -> sym != NULL )
                t -> id = t -> sym -> type;
            break;
        }
    }
    else
    {
        /* get return type within global scope */
        rc = return_type_expr ( tbl, src, t, env, self, f );
        if ( rc != 0 )
            return KTokenFailure ( t, klogErr, rc, "return type" );
    }

    /* get function name */
    rc = create_fqn ( tbl, src, t, env, type, NULL );
    if ( rc != 0 ) switch ( GetRCState ( rc ) )
    {
    case rcExists:
        break;
    case rcUnexpected:
        if ( type == 0 && t -> sym != NULL )
        {
            if ( t -> sym -> type == eFunction || t -> sym -> type == eScriptFunc )
                break;
        }
    default:
        return KTokenFailure ( t, klogErr, rc, "fully qualified name" );
    }

    /* record symbol - new or redefined */
    f -> name = t -> sym;

    /* get version */
    if ( next_token ( tbl, src, t ) -> id == eHash )
    {
        bool allow_release = ( f -> name -> type != eFunction ) ? true : false;
        next_token ( tbl, src, t );
        rc = maj_min_rel ( tbl, src, t, env, self, & f -> version, allow_release );
        if ( rc != 0 )
            return rc;
    }

    /* parse formal parameters - enter schema scope */
    rc = KSymTablePushScope ( tbl, & f -> sscope );
    if ( rc != 0 )
        KTokenRCExplain ( t, klogInt, rc );
    else
    {
        /* enter function scope */
        rc = KSymTablePushScope ( tbl, & f -> fscope );
        if ( rc != 0 )
            KTokenRCExplain ( t, klogInt, rc );
        else
        {
            /* gather factory parameters */
            if ( t -> id == eLeftAngle )
                rc = fact_signature ( tbl, src, t, env, self, & f -> fact );

            /* gather function parameters */
            if ( rc == 0 )
                rc = parm_signature ( tbl, src, t, env, self, & f -> func );

            /* leave function scope */
            KSymTablePopScope ( tbl );
        }

        /* leave schema scope */
        KSymTablePopScope ( tbl );
    }

    /* go a bit further */
    if ( rc == 0 )
    {
        /* detect script body */
        if ( t -> id == eLeftCurly )
        {
#if SLVL >= 4
            /* if user already specified extern function
               or if user specified vararg factory params */
            if ( type == eFunction || f -> fact . vararg )
#endif
                return KTokenExpected ( t, klogErr, "; or =" );
#if SLVL >= 4
            /* if no type was specified */
            if ( type == 0 )
            {
                /* if name was previously defined as a function */
                if ( f -> name -> type == eFunction )
                    return KTokenExpected ( t, klogErr, "; or =" );

                /* name is either undefined or script - clobber to script */
                ( ( KSymbol* ) f -> name ) -> type = eScriptFunc;
            }

            /* parse remainder as script function */
            return script_body ( tbl, src, t, env, self, f );
#endif
        }

        /* detect case where should be script but isn't */
        if ( f -> name -> type == eScriptFunc )
            return KTokenExpected ( t, klogErr, "{" );

        /* definitely an extern function */
        if ( type == 0 )
            ( ( KSymbol* ) f -> name ) -> type = eFunction;

        /* process factory spec */
        if ( t -> id == eAssign )
        {
            /* get factory name */
            next_token ( tbl, src, t );
            rc = create_fqn ( tbl, src, t, env, eFactory, NULL );
            if ( rc != 0 ) switch ( GetRCState ( rc ) )
            {
            case rcExists:
                break;
            case rcUnexpected:
                if ( t -> sym != NULL && t -> sym -> type == eFunction )
                    break;
            default:
                return rc;
            }

            f -> u . ext . fact = t -> sym;
            next_token ( tbl, src, t );
        }

        /* expect a semicolon */
        rc = expect ( tbl, src, t, eSemiColon, ";", true );
    }

    return rc;
}

static
rc_t function_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, uint32_t type, bool validate )
{
    rc_t rc;
    void *ignore;

    /* create object */
    SFunction *f = malloc ( sizeof * f );
    if ( f == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    memset ( f, 0, sizeof * f );
    f -> validate = validate;

    /* parse function decl */
    rc = func_decl ( tbl, src, t, env, self, f, type );

    /* check validation functions for exactly two parameters */
    if ( f -> validate && ( rc == 0 || GetRCState ( rc ) == rcExists ) )
    {
        if ( f -> func . mand != 2 ||
             f -> func . vararg != 0 ||
             VectorLength ( & f -> func . parms ) != 2 )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcFunction, rcInvalid );
            KTokenRCExplain ( t, klogInt, rc );
        }
    }

    if ( rc == 0 )
    {
        /* need an overloaded name entry */
        SNameOverload *name = ( void* ) f -> name -> u . obj;
        if ( name == NULL )
        {
            /* create name */
            rc = SNameOverloadMake ( & name, f -> name, 0, 4 );
            if ( rc == 0 )
            {
                /* insert it - it's allowed to be empty */
                rc = VectorAppend ( & self -> fname, & name -> cid . id, name );
                if ( rc != 0 )
                    SNameOverloadWhack ( name, NULL );
            }
        }

        /* now need to record function */
        if ( rc == 0 )
        {
            /* assume it's new in this schema */
            rc = VectorAppend ( & self -> func, & f -> id, f );
            if ( rc == 0 )
            {
                /* insert into name overload and exit on success */
                uint32_t idx;
                rc = VectorInsertUnique ( & name -> items, f, & idx, SFunctionSort );
                if ( rc == 0 )
                    return 0;

                /* expected failure is that a function already exists */
                if ( GetRCState ( rc ) != rcExists )
                    VectorSwap ( & self -> func, f -> id, NULL, & ignore );
                else
                {
                    /* see if new function trumps old */
                    SFunction *exist = VectorGet ( & name -> items, idx );
                    if ( f -> version > exist -> version )
                    {
                        /* insert our function in name overload */
                        VectorSwap ( & name -> items, idx, f, & ignore );

                        /* if existing is in another schema... */
                        if ( ( const void* ) name != exist -> name -> u . obj )
                            return 0;

                        /* need to swap with old */
                        assert ( exist -> id >= VectorStart ( & self -> func ) );
                        assert ( exist -> id < f -> id );
                        VectorSwap ( & self -> func, f -> id, NULL, & ignore );
                        VectorSwap ( & self -> func, f -> id = exist -> id, f, & ignore );
                        SFunctionWhack ( exist, NULL );
                        return 0;
                    }

                    /* exists is not an error */
                    rc = 0;
                }

                VectorSwap ( & self -> func, f -> id, NULL, & ignore );
            }
        }
    }
    else if ( GetRCState ( rc ) == rcExists )
    {
        rc = 0;
    }

    SFunctionWhack ( f, NULL );
    return rc;
}

/*
 * function-decl      = 'function' <ext-func-decl> ';'
 *                    | 'function' <script-func-decl>
 */
rc_t function_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    return function_decl ( tbl, src, t, env, self,
        env -> script_function_called_schema ? eFunction : 0, false );
}

/*
 * extern-func        = 'extern' 'function' <ext-function-decl> ';'
 */
rc_t extfunc_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    return function_decl ( tbl, src, t, env, self, eFunction, false );
}

/*
 * validate-func      = 'validate' 'function' <validate-function-decl> ';'
 */
rc_t valfunc_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    return function_decl ( tbl, src, t, env, self, eFunction, true );
}

#endif /* SLVL >= 3 */


#if SLVL >= 4

/*
 * script-decl        = 'schema' [ 'function' ] <script-func-decl>
 */
rc_t script_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    if ( t -> id == kw_function )
        next_token ( tbl, src, t );

    return function_decl ( tbl, src, t, env, self, eScriptFunc, false );
}

#endif /* SLVL >= 4 */
