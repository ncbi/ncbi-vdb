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
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SProduction
 *  describes a schema production/statement
 */
#if SLVL >= 3

/* Whack
 */
void CC SProductionWhack ( void *item, void *ignore )
{
    SProduction *s = item;
    SExpressionWhack ( s -> expr );
    SExpressionWhack ( s -> fd );
    free ( s );
}

/* Mark
 */
void CC SProductionMark ( void * item, void * data )
{
    const SProduction * self = item;
    if ( self != NULL )
    {
        SExpressionMark ( ( void * )self -> fd, data );
        SExpressionMark ( ( void * )self -> expr, data );
    }
}

/* Dump
 *  dump production
 */
rc_t SProductionDump ( const SProduction *self, struct SDumper *d )
{
    if ( self == NULL )
        return KSymbolDump ( NULL, d );
    if ( ! self -> control )
        return KSymbolDump ( self -> name, d );
    return SDumperPrint ( d, "control %N", self -> name );
}

bool CC SProductionDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SProduction *self = ( const void* ) item;

    /* function params are waiting for an expression */
    if ( self -> expr == NULL )
        b -> rc = SDumperPrint ( b, "%E %N", self -> fd, self -> name );
    else
    {
        bool compact = SDumperMode ( b ) == sdmCompact ? true : false;
        if ( self -> trigger )
        {
            b -> rc = SDumperPrint ( b, compact ? "trigger %N=%E;" : "\ttrigger %N = %E;\n"
                                     , self -> name
                                     , self -> expr );
        }
        else
        {
            b -> rc = SDumperPrint ( b, compact ? "%E %N=%E;" : "\t%E %N = %E;\n"
                                     , self -> fd
                                     , self -> name
                                     , self -> expr );
        }
    }

    return ( b -> rc != 0 ) ? true : false;
}

#endif


/*--------------------------------------------------------------------------
 * SFunction
 *  describes a schema function
 */
#if SLVL >= 4

/* Dump
 */
rc_t SFunctionBodyDump ( const SFunction *self, SDumper *b )
{
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( VectorLength ( & self -> u . script . prod ) == 0 )
    {
        b -> rc = SDumperPrint ( b, compact ?
            "{return %E;}" : "\t{ return %E; }\n",
            self -> u . script . rtn );
    }
    else
    {
        b -> rc = SDumperPrint ( b, compact ? "{" : "\t{\n" );
        if ( b -> rc == 0 )
        {
            if ( ! compact )
                SDumperIncIndentLevel ( b );
            if ( ! VectorDoUntil ( & self -> u . script . prod, false, SProductionDefDump, b ) )
                b -> rc = SDumperPrint ( b, compact ? "return %E;" : "\treturn %E;\n", self -> u . script . rtn );
            if ( ! compact )
                SDumperDecIndentLevel ( b );
        }
        if ( b -> rc == 0 )
            b -> rc = SDumperPrint ( b, compact ? "}" : "\t}\n" );
    }

    return b -> rc;
}

#endif

/*--------------------------------------------------------------------------
 * SPhysical
 */

#if SLVL >= 5

/* Whack
 */
void CC SPhysicalWhack ( void *n, void *ignore )
{
    SPhysical *self = n;

    /* if any schema or factory parameters were given, the objects
       have been shared between encode and decode schema funcs, and
       the schema names are also shared between funcs. wipe out the
       encode copy and leave ownership with decode */
    BSTreeInit ( & self -> encode . sscope );
    VectorInit ( & self -> encode . fact . parms, 0, 1 );
    VectorInit ( & self -> encode . type, 0, 1 );
    VectorInit ( & self -> encode . schem, 0, 1 );

    /* now allow normal destruction to proceed */
    SFunctionDestroy ( & self -> encode );
    SFunctionDestroy ( & self -> decode );
    SExpressionWhack ( self -> td );

    free ( self );
}

/* Cmp
 * Sort
 */
int64_t CC SPhysicalCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SPhysical *b = n;

    if ( * a > b -> version )
        return 1;
    return ( int64_t ) ( * a >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

int64_t CC SPhysicalSort ( const void *item, const void *n )
{
    const SPhysical *a = item;
    const SPhysical *b = n;

    return ( int64_t ) ( a -> version >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

/* Bind
 *  perform schema and factory param substitution
 *  returns prior param values
 */
rc_t SPhysicalBindSchemaParms ( const SPhysical *self,
    Vector *prior, const Vector *subst, Vector *cx_bind )
{
    return SFunctionBindSchemaParms ( & self -> decode, prior, subst, cx_bind );
}

rc_t SPhysicalBindFactParms ( const SPhysical *self,
    Vector *parms, Vector *prior, const Vector *subst, Vector *cx_bind )
{
    return SFunctionBindFactParms ( & self -> decode, parms, prior, subst, cx_bind );
}

/* Rest-ore
 *  restore schema and factory param substitution
 *  destroys prior param vector
 */
void SPhysicalRestSchemaParms ( const SPhysical *self, Vector *prior, Vector *cx_bind )
{
    SFunctionRestSchemaParms ( & self -> decode, prior, cx_bind );
}

void SPhysicalRestFactParms ( const SPhysical *self, Vector *prior, Vector *cx_bind )
{
    SFunctionRestFactParms ( & self -> decode, prior, cx_bind );
}

/* Mark
 */
void CC SPhysicalClearMark ( void *item, void *ignore )
{
    SPhysical *self = item;
    self -> marked = false;
    self -> encode . marked = false;
    self -> decode . marked = false;
}

void VSchemaClearPhysMark ( const VSchema *self )
{
    if ( self -> dad != NULL )
        VSchemaClearPhysMark ( self -> dad );
    VectorForEach ( & self -> phys, false, SPhysicalClearMark, NULL );
}

void CC SPhysicalMark ( void * item, void * data )
{
    SPhysical *self = item;
    if ( self != NULL && ! self -> marked )
    {
        self -> marked = true;
        SExpressionMark ( ( void * )self -> td, data );
        SFunctionMark ( ( void * )& self -> encode, data );
        SFunctionMark ( ( void * )& self -> decode, data );
    }
}

void SPhysNameMark ( const SNameOverload *self, const VSchema *schema )
{
    if ( self != NULL )
    {
        VectorForEach ( & self -> items, false, SPhysicalMark, ( void* ) schema );
    }
}

/* Dump
 */
rc_t SPhysicalDump ( const SPhysical *self, SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}

bool CC SPhysicalDefDump ( void *n, void *data )
{
    SDumper *b = data;
    const SPhysical *self = ( const void* ) n;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    /* a physical column */
    b -> rc = SDumperPrint ( b, "physical " );

    /* could have schema parameters */
    if ( b -> rc == 0 )
        b -> rc = SFunctionDeclDumpSchemaParms ( & self -> decode, b );

    /* issue __no_header for v0 encodings */
    if ( b -> rc == 0 && self -> no_hdr )
        b -> rc = SDumperPrint ( b, "__no_header " );

    /* a return type expression followed by column typename */
    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, "%E %N", self -> td, self -> name );

    /* versions are mandatory */
    if ( b -> rc == 0 )
        b -> rc = SDumperVersion ( b, self -> version );

    /* could have factory parameters */
    if ( b -> rc == 0 )
        b -> rc = SFunctionDeclDumpFactParms ( & self -> decode, b );

    /* physical body */
    if ( b -> rc == 0 )
    {
        b -> rc = SDumperPrint ( b, compact ? "{" : "\n\t{\n" );
        if ( b -> rc == 0 )
        {
            if ( ! compact )
                SDumperIncIndentLevel ( b );

            if ( self -> encode . u . script . rtn != NULL )
            {
                b -> rc = SDumperPrint ( b, compact ? "encode" : "\tencode\n" );
                if ( b -> rc == 0 )
                    b -> rc = SFunctionBodyDump ( & self -> encode, b );
            }
            if ( b -> rc == 0 )
                b -> rc = SDumperPrint ( b, compact ? "decode" : "\tdecode\n" );
            if ( b -> rc == 0 )
                b -> rc = SFunctionBodyDump ( & self -> decode, b );

            if ( ! compact )
                SDumperDecIndentLevel ( b );
        }
        if ( b -> rc == 0 )
            b -> rc = SDumperPrint ( b, compact ? "}" : "\t}\n" );
    }

    return ( b -> rc != 0 ) ? true : false;
}

#endif


/*--------------------------------------------------------------------------
 * VSchema
 */

#if SLVL >= 4

/*
 * production-stmt    = <func-fmtdecl> ID <assign-expr>
 *                    | 'trigger' ID <assign-expr>
 */
rc_t production_stmt ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, Vector *v, uint32_t ptype )
{
    rc_t rc;
    String id;
    KSymbol *sym;

    SProduction *prod = malloc ( sizeof *prod );
    if ( prod == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    memset ( prod, 0, sizeof * prod );

    /* look for trigger production */
    if ( ptype == eTable && t -> id == kw_trigger )
    {
        prod -> trigger = true;
        next_token ( tbl, src, t );
    }
    else
    {
        KToken t2 = * t;

        /* variable type */
        rc = type_expr ( tbl, src, t, env, self, & prod -> fd );
        if ( rc != 0 )
        {
            free ( prod );
            return KTokenExpected ( t, klogErr, "type declaration" );
        }

        /* disallow formatted types within table */
        if ( ptype == eTable && ( ( const STypeExpr* ) prod -> fd ) -> fmt != NULL )
        {
            SExpressionWhack ( prod -> fd );
            free ( prod );
            return KTokenExpected ( & t2, klogErr, "unformatted type declaration" );
        }
    }

    /* catch defined identifiers */
    if ( t -> sym != NULL && t -> id != eForward && t -> id != eVirtual )
    {
        /* if this is not a table or the symbol is not a production */
        if ( ptype != eTable || t -> id != eProduction )
        {
            /* re-interpret this token in schema and intrinsic scopes */
            KTokenSourceReturn ( src, t );
            next_shallow_token ( tbl, src, t, true );
        }
    }

    /* require an identifier or a forward reference */
    sym = NULL;
    if ( t -> id == eForward || t -> id == eVirtual )
        sym = t -> sym;
    else if ( t -> id != eIdent )
    {
        SProductionWhack ( prod, NULL );
        return KTokenExpected ( t, klogErr, "production name" );
    }

    /* remember identifier */
    id = t -> str;

    /* within the production, only '=' is accepted for assignment
       this could certainly change in the future... */
    if ( next_token ( tbl, src, t ) -> id != eAssign )
    {
        SProductionWhack ( prod, NULL );
        return KTokenExpected ( t, klogErr, "=" );
    }

    /* parse conditional expression */
    rc = cond_expr ( tbl, src, next_token ( tbl, src, t ),
        env, self, & prod -> expr );
    if ( rc == 0 )
    {
        /* create symbolic name for production and insert */
        if ( sym == NULL )
            rc = KSymTableCreateConstSymbol ( tbl, & prod -> name, & id, eProduction, prod );
        else
        {
            /* convert forward reference or virtual to production */
            sym -> type = eProduction;
            sym -> u . obj = prod;
            prod -> name = sym;
        }
        if ( rc == 0 )
        {
            /* ctx = 0 for params, ctx == 1 for productions */
            prod -> cid . ctx = 1;
            rc = VectorAppend ( v, & prod -> cid . id, prod );
        }
        if ( rc != 0 )
            KTokenRCExplain ( t, klogInt, rc );
    }

    /* cleanup */
    if ( rc != 0 )
        SProductionWhack ( prod, NULL );

    return rc;
}

/*
 * schema-stmt        = 'return' <cond-expr>
 *                    | <production-stmt>
 */
static
rc_t script_stmt ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFunction *f )
{
    rc_t rc;

    if ( t -> id == kw_return )
    {
        /* disallow multiple return statements */
        if ( f -> u . script . rtn != NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcToken, rcExists );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        /* just get a conditional expression */
        rc = cond_expr ( tbl, src, next_token ( tbl, src, t ),
            env, self, & f -> u . script . rtn );
    }
    else
    {
        rc = production_stmt ( tbl, src, t, env, self,
            & f -> u . script . prod, eScriptFunc );
    }

    if ( rc == 0 )
        rc = expect ( tbl, src, t, eSemiColon, ";", true );

    return rc;
}


/*
 * schema-body        = '{' <script-stmts> '}'
 *
 * script-stmts       = <script-stmt> ';' [ <script-stmts> ]
 */
static
rc_t schema_body ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFunction *f )
{
    /* always open braces */
    rc_t rc = expect ( tbl, src, t, eLeftCurly, "{", true );
    if ( rc != 0 )
        return rc;

    /* expect at least one statement */
    rc = script_stmt ( tbl, src, t, env, self, f );
    if ( rc != 0 )
        return rc;

    /* handle statement list */
    while ( t -> id != eRightCurly )
    {
        rc = script_stmt ( tbl, src, t, env, self, f );
        if ( rc != 0 )
            return rc;
    }

    /* must have a return statement */
    if ( f -> u . script . rtn  == NULL )
        return KTokenExpected ( t, klogErr, "return statement" );

    /* looks like a script function */
    f -> script = true;

    /* close braces */
    return expect ( tbl, src, t, eRightCurly, "}", true );
}

static
bool CC script_prod_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    SProduction *prod = item;

    assert ( prod -> name != NULL );
    assert ( prod -> name -> type == eProduction );

    * rc = eval_expr_syntax ( prod -> fd );
    if ( * rc == 0 )
        * rc = eval_expr_syntax ( prod -> expr );

    return ( * rc != 0 ) ? true : false;
}

static
bool CC script_sym_syntax ( BSTNode *n, void *data )
{
    rc_t *rc = data;
    KSymbol *sym = ( KSymbol* ) n;

    if ( sym -> type == eForward )
    {
        * rc = RC ( rcVDB, rcSchema, rcParsing, rcName, rcUndefined );
        return true;
    }

    return false;
}

static
rc_t script_syntax ( const VSchema *self, SFunction *f )
{
    rc_t rc = eval_expr_syntax ( f -> u . script . rtn );
    if ( rc == 0 )
    {
        BSTreeDoUntil ( & f -> fscope,
            false, script_sym_syntax, & rc );
    }
    if ( rc == 0 )
        VectorDoUntil ( & f -> u . script . prod, false, script_prod_syntax, & rc );
    return rc;
}

/*
 * script-body        = '{' <script-stmts> '}'
 */
rc_t script_body ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SFunction *f )
{
    /* enter schema scope */
    rc_t rc = KSymTablePushScope ( tbl, & f -> sscope );
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
            /* parse '{' statements; '}' */
            VectorInit ( & f -> u . script . prod, 0, 8 );
            rc = schema_body ( tbl, src, t, env, self, f );

            /* leave function scope */
            KSymTablePopScope ( tbl );
        }

        /* leave schema scope */
        KSymTablePopScope ( tbl );
    }

    if ( rc == 0 )
        rc = script_syntax ( self, f );

    return rc;
}

#endif /* SLVL >= 4 */

#if SLVL >= 5

static
rc_t rowlen_func_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SPhysical *p )
{
    const SNameOverload *name;

    /* expecting a function assignment */
    rc_t rc = expect ( tbl, src,
        next_token ( tbl, src, t ), eAssign, "=", true );
    if ( rc == 0 )
        rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;
    if ( t -> id != eRowLengthFunc )
        return KTokenExpected ( t, klogErr, "row length function expression" );
    name = t -> sym -> u . obj;

    rc = expect ( tbl, src,
        next_token ( tbl, src, t ), eLeftParen, "(", true );
    if ( rc == 0 )
        rc = expect ( tbl, src, t, eRightParen, ")", true );
    if ( rc == 0 )
        rc = expect ( tbl, src, t, eSemiColon, ";", true );

    if ( rc == 0 )
    {
        p -> row_length = VectorLast ( & name -> items );
        if ( p -> row_length == NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcFunction, rcNotFound );
            KTokenRCExplain ( t, klogErr, rc );
        }
    }

    return rc;
}

/*
 * physical-stmt      = 'encode' <script-body>
 *                    | 'decode' <script-body>
 */
static
rc_t physical_stmt ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SPhysical *p, bool encoding )
{
    rc_t rc;
    String symstr;
    SFunction *f = encoding ? & p -> encode : & p -> decode;

    /* simulate a schema function signature */
    SProduction *parm = malloc ( sizeof * parm );
    if ( parm == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }
    memset ( parm, 0, sizeof * parm );
    VectorInit ( & f -> func . parms, 0, 1 );
    rc = VectorAppend ( & f -> func . parms, & parm -> cid . id, parm );
    if ( rc != 0 )
    {
        free ( parm );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    /* enter schema scope */
    rc = KSymTablePushScope ( tbl, & f -> sscope );
    if ( rc != 0 )
        KTokenRCExplain ( t, klogInt, rc );
    else
    {
        /* enter param scope */
        rc = KSymTablePushScope ( tbl, & f -> fscope );
        if ( rc != 0 )
            KTokenRCExplain ( t, klogInt, rc );
        else
        {
            /* create special input symbol */
            CONST_STRING ( & symstr, "@" );
            rc = KSymTableCreateConstSymbol ( tbl, & parm -> name,
                & symstr, eFuncParam, parm );
            if ( rc != 0 )
                KTokenRCExplain ( t, klogInt, rc );
            else
            {
                VectorInit ( & f -> u . script . prod, 0, 8 );
                rc = schema_body ( tbl, src, t, env, self, f );
            }

            /* exit param scope */
            KSymTablePopScope ( tbl );
        }

        /* exit schema scope */
        KSymTablePopScope ( tbl );
    }

    return rc;
}

/*
 * physical-decl      = 'physical' [ 'column' ] <typedecl>
 *                       <fqn> '#' <maj-min-rel> <phys-body>
 * phys-body          =  '{' <physical-stmts> '}'
 * physical-stmt      = 'encode' <script-body>
 *                    | 'decode' <script-body>
 *                    | '__row_length = <func-expr>
 *                    | ';'
 */
static
rc_t physical_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SPhysical *p )
{
    rc_t rc;
    const char *expected;

    /* could have schema parameters */
    if ( t -> id == eLeftAngle )
    {
        /* enter schema param scope */
        rc = KSymTablePushScope ( tbl, & p -> decode . sscope );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );

        /* parse schema params */
        rc = schema_signature ( tbl, src, t, env, self, & p -> decode );

        /* copy schema parameters to encode function
           NB - must be cleared before destruction */
        p -> encode . sscope = p -> decode . sscope;
        p -> encode . type = p -> decode . type;
        p -> encode . schem = p -> decode. schem;

        /* interpret return type within schema param scope */
        if ( rc == 0 )
        {
            rc = type_expr ( tbl, src, t, env, self, & p -> td );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "column typedecl" );
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
            KTokenSourceReturn ( src, t );
            next_token ( tbl, src, t );
            break;
        }
    }
    else
    {
        /* allow v0 columns to be declared with no blob header */
        if ( t -> id == kw___no_header )
        {
            p -> no_hdr = true;
            next_token ( tbl, src, t );
        }

        /* get column type expression */
        rc = type_expr ( tbl, src, t, env, self, & p -> td );
        if ( rc != 0 )
            return KTokenExpected ( t, klogErr, "column typedecl" );
    }

    /* expect fully-qualified name */
    rc = create_fqn ( tbl, src, t, env, ePhysical, NULL );
    if ( rc != 0 && GetRCState ( rc ) != rcExists )
        return KTokenFailure ( t, klogErr, rc, "column typename" );
    p -> name = t -> sym;

    /* must have a version */
    if ( next_token ( tbl, src, t ) -> id != eHash )
        return KTokenExpected ( t, klogErr, "#" );
    next_token ( tbl, src, t );
    rc = maj_min_rel ( tbl, src, t, env, self, & p -> version, false );
    if ( rc != 0 )
        return rc;

    /* factory params */
    if ( t -> id == eLeftAngle )
    {
        rc = KSymTablePushScope ( tbl, & p -> decode . sscope );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
        rc = KSymTablePushScope ( tbl, & p -> decode . fscope );
        if ( rc != 0 )
            KTokenRCExplain ( t, klogInt, rc );
        else
        {
            rc = fact_signature ( tbl, src, t, env, self, & p -> decode . fact );
            if ( rc == 0 )
            {
                /* copy factory parameters to encode function
                   NB - must be cleared before destruction */
                p -> encode . fact = p -> decode . fact;

                /* clone factory parameter symbols */
                if ( BSTreeDoUntil ( & p -> decode . fscope, false, KSymbolCopyScope, & p -> encode . fscope ) )
                    rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
            }

            KSymTablePopScope ( tbl );
        }

        KSymTablePopScope ( tbl );
        if ( rc != 0 )
            return rc;
    }

    /* allow shorthand for decode-only */
    if ( t -> id == eAssign )
    {
        next_token ( tbl, src, t );
        rc = physical_stmt ( tbl, src, t, env, self, p, false );
        if ( rc == 0 )
            p -> read_only = true;
        return rc;
    }

    /* open body */
    rc = expect ( tbl, src, t, eLeftCurly, "{", true );
    if ( rc != 0 )
        return rc;

    /* gather both parts */
    expected = p -> no_hdr ? "decode" : "decode or encode";
    while ( t -> id != eRightCurly )
    {
        if ( t -> id == eSemiColon )
            next_token ( tbl, src, t );
        else if ( t -> id == kw_decode && p -> decode . u . script . rtn == NULL )
        {
            next_token ( tbl, src, t );
            rc = physical_stmt ( tbl, src, t, env, self, p, false );
            if ( rc != 0 )
                return rc;
            expected = ( p -> encode . u . script . rtn == NULL ) ? "encode" : "}";
        }
        else if ( t -> id == kw_encode && p -> encode . u . script . rtn == NULL && ! p -> no_hdr )
        {
            next_token ( tbl, src, t );
            rc = physical_stmt ( tbl, src, t, env, self, p, true );
            if ( rc != 0 )
                return rc;
            expected = ( p -> decode . u . script . rtn == NULL ) ? "decode" : "}";
        }
        else if ( t -> id == kw___row_length && p -> row_length == NULL && p -> no_hdr )
        {
            rc = rowlen_func_expr ( tbl, src, t, env, self, p );
            if ( rc != 0 )
                return rc;
        }
        else
        {
            return KTokenExpected ( t, klogErr, expected );
        }
    }

    /* must have at least a decoding */
    if ( p -> decode . u . script . rtn == NULL )
        return KTokenExpected ( t, klogErr, "decode" );

    /* close */
    rc = expect ( tbl, src, t, eRightCurly, "}", true );
    if ( rc == 0 )
    {
        /* detect read-only decl */
        if ( p -> encode . u . script . rtn == NULL )
            p -> read_only = true;
        else
        {
            /* patch up input parameter to encoding */
            SProduction *parm = VectorGet ( & p -> encode . func . parms, 0 );
            assert ( parm != NULL );
            parm -> fd = p -> td;
            atomic32_inc ( & ( ( SExpression* ) p -> td ) -> refcount );
        }
    }

    return rc;
}

static
void init_function ( SFunction *f )
{
    memset ( f, 0, sizeof * f );
    f -> script = true;
}

rc_t physical_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    rc_t rc;
    void *ignore;

    SPhysical *p = malloc ( sizeof * p );
    if ( p == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    p -> name = NULL;
    p -> row_length = NULL;
    init_function ( & p -> encode );
    init_function ( & p -> decode );
    p -> td = NULL;
    p -> read_only = false;
    p -> no_hdr = false;

    if ( t -> id == kw_column )
        next_token ( tbl, src, t );

    rc = physical_decl ( tbl, src, t, env, self, p );
    if ( rc == 0 )
    {
        SNameOverload *name = ( void* ) p -> name -> u . obj;
        if ( name == NULL )
        {
            rc = SNameOverloadMake ( & name, p -> name, 0, 8 );
            if ( rc == 0 )
            {
                rc = VectorAppend ( & self -> pname, & name -> cid . id, name );
                if ( rc != 0 )
                    SNameOverloadWhack ( name, NULL );
            }
        }

        if ( rc == 0 )
        {
            rc = VectorAppend ( & self -> phys, & p -> id, p );
            if ( rc == 0 )
            {
                uint32_t idx;
                rc = VectorInsertUnique ( & name -> items, p, & idx, SPhysicalSort );
                if ( rc == 0 )
                    return 0;
                if ( GetRCState ( rc ) != rcExists )
                    VectorSwap ( & self -> phys, p -> id, NULL, & ignore );
                else
                {
                    SPhysical *exist = VectorGet ( & name -> items, idx );
                    if ( p -> version > exist -> version )
                    {
                        VectorSwap ( & name -> items, idx, p, & ignore );
                        if ( ( const void* ) name != exist -> name -> u . obj )
                            return 0;
                        assert ( exist -> id >= VectorStart ( & self -> phys ) );
                        assert ( exist -> id < p -> id );
                        VectorSwap ( & self -> phys, p -> id, NULL, & ignore );
                        VectorSwap ( & self -> phys, p -> id = exist -> id, p, & ignore );
                        SPhysicalWhack ( exist, NULL );
                        return 0;
                    }

                    /* exists is not an error */
                    rc = 0;
                }

                VectorSwap ( & self -> phys, p -> id, NULL, & ignore );
            }
        }
    }
    else if ( GetRCState ( rc ) == rcExists )
    {
        rc = 0;
    }

    SPhysicalWhack ( p, NULL );

    return rc;
}

#endif /* SLVL >= 5 */
