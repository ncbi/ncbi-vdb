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
#include "dbmgr-priv.h"

#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/namelist.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/* if non-zero, accept a typeset or a typedecl
   for column. otherwise, only allow a typedecl */
#define ALLOW_COLUMN_TYPESET 1

/* make implicit physical expressions explicit on output */
#define DUMP_EXPLICIT_PHYS_MEMBERS 0

/* temporary v1 schema modification */
#define ALLOW_DEFAULT_VIEW_DECL 1


/*--------------------------------------------------------------------------
 * SColumn
 *  column declaration
 */

#if SLVL >= 8

/* Whack
 */
void CC SColumnWhack ( void *item, void *ignore )
{
    SColumn *self = item;
    SExpressionWhack ( self -> read );
    SExpressionWhack ( self -> validate );
    SExpressionWhack ( self -> limit );
    SExpressionWhack ( self -> ptype );
    free ( self );
}

/* Cmp
 * Sort
 */
int CC SColumnCmp ( const void *item, const void *n )
{
    const VTypedecl *a = item;
    const SColumn *b = n;

    if ( a -> type_id != b -> td . type_id )
        return ( int ) a -> type_id - ( int ) b -> td . type_id;
    return ( int ) a -> dim - ( int ) b -> td . dim;
}

int CC SColumnSort ( const void *item, const void *n )
{
    const SColumn *a = item;
    const SColumn *b = n;

    if ( a -> td . type_id != b -> td . type_id )
        return ( int ) a -> td . type_id - ( int ) b -> td . type_id;
    return ( int ) a -> td . dim - ( int ) b -> td . dim;
}

/* Mark
 */
static
void CC SColumnMark ( void * item, void * data )
{
    const SColumn *self = item;
    VSchema * schema = data;
    if ( self != NULL )
    {
        SExpressionMark ( ( void * )self -> read, data );
        SExpressionMark ( ( void * )self -> validate, data );
        SExpressionMark ( ( void * )self -> limit, data );
        VSchemaTypeMark ( schema, self -> td . type_id );
    }
}

/* Dump
 */
rc_t SColumnDump ( const SColumn *self, SDumper *d )
{
    return KSymbolDump ( self != NULL ? self -> name : NULL, d );
}

static
rc_t SColumnDumpExpr ( SDumper *d, const SExpression *e )
{
    rc_t rc;
    const SBinExpr *c;
    bool compact = SDumperMode ( d ) == sdmCompact ? true : false;

    if ( e -> var != eCondExpr )
        return SDumperPrint ( d, compact ? "=%E;" : " = %E;\n", e );

    c = ( const SBinExpr* ) e;

    if ( ! compact )
        SDumperIncIndentLevel ( d );
    rc = SDumperPrint ( d, compact ? "=%E" : "\n\t= %E\n", c -> left );
    while ( rc == 0 )
    {
        if ( c -> right -> var != eCondExpr )
            break;
        c = ( const SBinExpr* ) c -> right;
        rc = SDumperPrint ( d, compact ? "|%E" : "\n\t| %E\n", c -> left );
    }

    rc = SDumperPrint ( d, compact ? "|%E" : "\n\t| %E\n", c -> right );
    if ( ! compact )
        SDumperDecIndentLevel ( d );
    return rc;
}

bool CC SColumnDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SColumn *self = ( const void* ) item;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    /* handle simple cases first */
    if ( self -> validate == NULL && self -> limit == NULL )
    {
        /* simple */
#if ! DUMP_EXPLICIT_PHYS_MEMBERS
        if ( self -> simple )
        {
            if ( self -> ptype != NULL )
            {
                b -> rc = SDumperPrint ( b, compact ? "column %E %N;" : "\tcolumn %E %N;\n"
                                         , self -> ptype
                                         , self -> name
                    );
            }
            else
            {
                b -> rc = SDumperPrint ( b, compact ? "column %T %N;" : "\tcolumn %T %N;\n"
                                         , & self -> td
                                         , self -> name
                    );
            }
        }
        else
#endif
        {
            assert ( self -> read != NULL );
            b -> rc = SDumperPrint ( b, compact ? "%s%scolumn %T %N = %E;" : "\t%s%scolumn %T %N = %E;\n"
                                     , self -> dflt ? "default " : ""
                                     , self -> read_only ? "readonly " : ""
                                     , & self -> td
                                     , self -> name
                                     , self -> read
                );
        }
    }

    /* canonical representation */
    else
    {
#if ! DUMP_EXPLICIT_PHYS_MEMBERS
        if ( self -> ptype != NULL )
        {
            b -> rc = SDumperPrint ( b, compact ? "%scolumn %E %N{" : "\t%scolumn %E %N\n\t{\n"
                                     , self -> dflt ? "default " : ""
                                     , self -> ptype
                                     , self -> name
                );
        }
        else
#endif
        {
            b -> rc = SDumperPrint ( b, compact ? "%s%scolumn %T %N{" : "\t%s%scolumn %T %N\n\t{\n"
                                     , self -> dflt ? "default " : ""
                                     , self -> read_only ? "readonly " : ""
                                     , & self -> td
                                     , self -> name
                );
        }
        if ( b -> rc == 0 )
        {
            if ( ! compact )
                SDumperIncIndentLevel ( b );

#if DUMP_EXPLICIT_PHYS_MEMBERS
            if ( self -> read != NULL )
#else
            if ( ! self -> simple && self -> read != NULL )
#endif
            {
                b -> rc = SDumperPrint ( b, compact ? "read" : "\tread" );
                if ( b -> rc == 0 )
                    b -> rc = SColumnDumpExpr ( b, self -> read );
            }

            if ( b -> rc == 0 && self -> validate != NULL )
            {
                b -> rc = SDumperPrint ( b, compact ? "validate" : "\tvalidate" );
                if ( b -> rc == 0 )
                    b -> rc = SColumnDumpExpr ( b, self -> validate );
            }

            if ( b -> rc == 0 && self -> limit != NULL )
                b -> rc = SDumperPrint ( b, compact ? "limit = %E;" : "\tlimit = %E;\n", self -> limit );

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
 * SPhysMember
 *  column declaration
 */

#if SLVL >= 7

/* Whack
 */
void CC SPhysMemberWhack ( void *item, void *ignore )
{
    SPhysMember *self = item;

    SExpressionWhack ( self -> type );
    SExpressionWhack ( self -> expr );
    free ( self );
}

/* Mark
 */
void CC SPhysMemberMark ( void * item, void * data )
{
    const SPhysMember * self = item;
    const VSchema * schema = data;
    if ( self != NULL )
    {
        SExpressionMark ( ( void * )self -> type, data );
        SExpressionMark ( ( void * )self -> expr, data );
        VSchemaTypeMark ( schema, self -> td . type_id );
    }
}

/* Dump
 */
rc_t SPhysMemberDump ( const SPhysMember *self, struct SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}

bool CC SPhysMemberDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SPhysMember *self = ( const void* ) item;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

#if ! DUMP_EXPLICIT_PHYS_MEMBERS
    if ( self -> simple )
        return false;
#endif

    b -> rc = SDumperPrint ( b, compact ? "%s column " : "\t%s column "
                             , self -> stat ? "static" : "physical"
        );
    if ( b -> rc == 0 )
    {
        if ( self -> type != NULL )
            b -> rc = SExpressionDump ( self -> type, b );
        else
            b -> rc = SDumperPrint ( b, "%T", & self -> td );
    }
    if ( b -> rc == 0 )
    {
        if ( compact )
        {
            if ( self -> expr == NULL )
                b -> rc = SDumperPrint ( b, " %N;", self -> name );
            else
                b -> rc = SDumperPrint ( b, " %N=%E;", self -> name, self -> expr );
        }
        else
        {
            if ( self -> expr == NULL )
                b -> rc = SDumperPrint ( b, " %N;\n", self -> name );
            else
                b -> rc = SDumperPrint ( b, " %N = %E;\n", self -> name, self -> expr );
        }
    }

    return ( b -> rc != 0 ) ? true : false;
}

#endif

/*--------------------------------------------------------------------------
 * STableOverrides
 *  describes extended parent
 */

#if SLVL >= 6
typedef struct STableOverrides STableOverrides;
struct STableOverrides
{
    const STable *dad;
    Vector overrides;
    uint32_t ctx;
};

/* Cmp
 * Sort
 */
static
int CC STableOverridesCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const STableOverrides *b = n;

    return ( int ) * a - ( int ) b -> ctx;
}

static
int CC STableOverridesSort ( const void *item, const void *n )
{
    const STableOverrides *a = item;
    const STableOverrides *b = n;

    return ( int ) a -> ctx - ( int ) b -> ctx;
}

static
int CC STableOverridesKSort ( const void **item, const void **n, void *ignore )
{
    return STableOverridesSort ( * item, * n );
}

/* Whack
 */
static
void CC STableOverridesWhack ( void *item, void *ignore )
{
    STableOverrides *self = item;
    VectorWhack ( & self -> overrides, NULL, NULL );
    free ( self );
}

/* Make
 */
static
rc_t STableOverridesMake ( Vector *parents, const STable *dad, const Vector *overrides )
{
    rc_t rc;
    STableOverrides *to;

    /* first question is whether parent exists */
    if ( VectorFind ( parents, & dad -> id, NULL, STableOverridesCmp ) != NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcTable, rcExists );

    /* create a new override object */
    to = malloc ( sizeof * to );
    if ( to == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    /* shallow clone */
    rc = VectorCopy ( overrides, & to -> overrides );
    if ( rc != 0 )
    {
        free ( to );
        return rc;
    }
    to -> dad = dad;
    to -> ctx = dad -> id;

    /* insert into parent override vector */
    rc = VectorInsert ( parents, to, NULL, STableOverridesSort );
    if ( rc != 0 )
    {
        STableOverridesWhack ( to, NULL );
        return rc;
    }

    return 0;
}

static
bool CC STableOverridesClone ( void *item, void *data )
{
    const STableOverrides *self = ( const void* ) item;
    rc_t rc = STableOverridesMake ( data, self -> dad, & self -> overrides );
    return ( rc != 0 && GetRCState ( rc ) != rcExists ) ? true : false;
}

#endif

/*--------------------------------------------------------------------------
 * STable
 *  table declaration
 */

#if SLVL >= 6

/* Whack
 */
void CC STableWhack ( void *item, void *ignore )
{
    STable *self = item;

    if ( self -> dflt_view != NULL )
        StringWhack ( self -> dflt_view );

#if SLVL >= 8
    VectorWhack ( & self -> col, SColumnWhack, NULL );
    VectorWhack ( & self -> cname, SNameOverloadWhack, NULL );
#endif
#if SLVL >= 7
    VectorWhack ( & self -> phys, SPhysMemberWhack, NULL );
#endif
    VectorWhack ( & self -> prod, SProductionWhack, NULL );
    VectorWhack ( & self -> vprods, NULL, NULL );
    VectorWhack ( & self -> syms, ( void ( CC * ) ( void*, void* ) ) KSymbolWhack, NULL );

    VectorWhack ( & self -> parents, NULL, NULL );
    VectorWhack ( & self -> overrides, STableOverridesWhack, NULL );

    SExpressionWhack ( self -> limit );

    BSTreeWhack ( & self -> scope, KSymbolWhack, NULL );

    free ( self );
}

/* Cmp
 * Sort
 */
int CC STableCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const STable *b = n;

    if ( * a > b -> version )
        return 1;
    return ( int ) ( * a >> 24 ) - ( int ) ( b -> version >> 24 );
}

int CC STableSort ( const void *item, const void *n )
{
    const STable *a = item;
    const STable *b = n;

    return ( int ) ( a -> version >> 24 ) - ( int ) ( b -> version >> 24 );
}


/* Find
 *  generic object find within table scope
 *
 *  "td" [ OUT, NULL OKAY ] - returns cast type expression
 *  if given or "any" if not
 *
 *  "name" [ OUT ] - returns list of overloaded objects if found
 *
 *  "type" [ OUT ] - returns object type id, e.g.:
 *    eDatatype, eTypeset, eFormat, eFunction, ePhysical, eTable, ...
 *
 *  "expr" [ IN ] - NUL terminated name expression identifying object
 *
 *  "ctx" [ IN ] - NUL terminated context string for evaluation,
 *  substitutes for filename in logging reports
 *
 *  "dflt" [ IN ] - if true, resolve default value
 *
 *  returns principal object identified. if NULL but "name" is not
 *  NULL, then the object was only partially identified.
 */
const void *STableFind ( const STable *self,
    const VSchema *schema, VTypedecl *td, const SNameOverload **name,
    uint32_t *type, const char *expr, const char *ctx, bool dflt )
{
    rc_t rc;
    KSymTable tbl;

    /* initialize to not-found */
    const void *obj = NULL;
    * name = NULL;
    * type = 0;

    /* build a symbol table for table */
    rc = init_tbl_symtab ( & tbl, schema, self );
    if ( rc == 0 )
    {
        obj = resolve_object ( & tbl, schema, td, name, type, expr, ctx, dflt );
        KSymTableWhack ( & tbl );
    }

    return obj;
}


/* FindOverride
 *  finds an inherited or introduced overridden symbol
 */
KSymbol *STableFindOverride ( const STable *self, const VCtxId *cid )
{
    const STableOverrides *to;

    /* it may be on the existing table */
    if ( cid -> ctx == self -> id )
        return VectorGet ( & self -> vprods, cid -> id );

    to = ( const void* ) VectorFind ( & self -> overrides,
        & cid -> ctx, NULL, STableOverridesCmp );
    if ( to == NULL )
        return NULL;

    return VectorGet ( & to -> overrides, cid -> id );
}

/* FindOrdAncestor
 *  finds a parent or grandparent by order
 */
const STable *STableFindOrdAncestor ( const STable *self, uint32_t i )
{
    const STableOverrides *to = ( const void* ) VectorGet ( & self -> overrides, i );
    if ( to == NULL )
        return NULL;
    return to -> dad;
}


/* Extend
 */
static
bool CC STableHasDad ( void *item, void *data )
{
    if ( item == data )
        return true;
    return false;
}

static
bool STableTestForTypeCollision ( const SNameOverload *a, const SNameOverload *b )
{
    uint32_t ax, bx, ctx;

    uint32_t aend = VectorLength ( & a -> items );
    uint32_t bend = VectorLength ( & b -> items );

    if ( aend == 0 || bend == 0 )
        return false;

    ctx = a -> cid . ctx;
    ax = VectorStart ( & a -> items );
    bx = VectorStart ( & b -> items );

    for ( aend += ax, bend += bx; ax < aend && bx < bend; )
    {
        int diff;
        const SColumn *acol = ( const void* ) VectorGet ( & a -> items, ax );
        const SColumn *bcol = ( const void* ) VectorGet ( & b -> items, bx );
        assert ( acol != NULL && bcol != NULL );

        /* if they are both from some other shared parent
           maybe they are even the same column */
        if ( acol == bcol || acol -> cid . ctx == bcol -> cid . ctx )
        {
            ++ ax;
            ++ bx;
            continue;
        }

        /* don't bother comparing if either is from
           originating table, as they are required
           to appear in both lists */
        if ( acol -> cid . ctx == ctx )
        {
            ++ ax;
            continue;
        }

        if ( bcol -> cid . ctx == ctx )
        {
            ++ bx;
            continue;
        }

        /* test the column types */
        diff = SColumnSort ( acol, bcol );

        /* if they are the same type, this is a collision */
        if ( diff == 0 )
            return true;

        /* let it slide */
        if ( diff < 0 )
            ++ ax;
        else
            ++ bx;
    }

    return false;
}

static
bool STableTestForSymCollision ( const KSymbol *sym, void *data )
{
    const KSymTable *tbl = ( const void* ) data;
    const KSymbol *found = KSymTableFindSymbol ( tbl, sym );
    if ( found != NULL && found != sym ) switch ( found -> type )
    {
    case eColumn:
        if ( sym -> type == eColumn )
        {
            /* when colliding columns originated in the same
               table, consider them to be compatible extensions */
            const SNameOverload *found_col, *sym_col;
            sym_col = sym -> u . obj;
            found_col = found -> u . obj;
            assert ( sym_col != NULL && found_col != NULL );
            if ( sym_col -> cid . ctx == found_col -> cid . ctx )
                return STableTestForTypeCollision ( sym_col, found_col );
        }
    case eProduction:
    case ePhysMember:
        PLOGMSG ( klogErr, ( klogErr, "duplicate symbol '$(sym)' in parent table hierarchy"
                             , "sym=%S"
                             , & sym -> name
                      ));
        return true;
    }
    return false;
}

static
bool CC STableTestColCollisions ( void *item, void *data )
{
    const SNameOverload *no = ( const void* ) item;
    return STableTestForSymCollision ( no -> name, data );
}

static
bool CC STableTestPhysCollisions ( void *item, void *data )
{
    const SPhysMember *phys = ( const void* ) item;
    return STableTestForSymCollision ( phys -> name, data );
}

static
bool CC STableTestProdCollisions ( void *item, void *data )
{
    const SProduction *prod = ( const void* ) item;
    return STableTestForSymCollision ( prod -> name, data );
}

static
bool STableTestForCollisions ( void *item, void *data )
{
    const STable *self = ( const void* ) item;

    /* test column names */
    if ( VectorDoUntil ( & self -> cname, false, STableTestColCollisions, data ) )
        return true;

    /* test physical names */
    if ( VectorDoUntil ( & self -> phys, false, STableTestPhysCollisions, data ) )
        return true;

    /* test production names */
    if ( VectorDoUntil ( & self -> prod, false, STableTestProdCollisions, data ) )
        return true;

    return false;
}

static
bool CC STableOverridesTestForCollisions ( void *item, void *data )
{
    const STableOverrides *to = ( const void* ) item;
    return STableTestForCollisions ( ( void* ) to -> dad, data );
}

static
bool CC STableCopyColumnNames ( void *item, void *data )
{
    rc_t rc;
    STable *self= data;
    SNameOverload *copy;
    const SNameOverload *orig = ( const void* ) item;
    const KSymbol *sym = ( const KSymbol* )
        BSTreeFind ( & self -> scope, & orig -> name -> name, KSymbolCmp );
    if ( sym == NULL )
    {
        rc = SNameOverloadCopy ( & self -> scope, & copy, orig );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & self -> cname, & copy -> cid . id, copy );
            if ( rc != 0 )
                SNameOverloadWhack ( copy, NULL );
        }
    }
    else
    {
        copy = ( void* ) sym -> u . obj;
        assert ( copy -> cid . ctx == orig -> cid . ctx );
        rc = VectorMerge ( & copy -> items, true, & orig -> items, SColumnSort );
    }

    return ( rc != 0 ) ? true : false;
}

static
bool CC STableScanVirtuals ( void *item, void *data )
{
    KSymTable *tbl = data;
    STableOverrides *to = item;
    BSTree *scope = VectorLast ( & tbl -> stack );
    uint32_t i = VectorStart ( & to -> overrides );
    uint32_t end = VectorLength ( & to -> overrides );
    for ( end += i; i < end; ++ i )
    {
        const KSymbol *orig = ( const void* ) VectorGet ( & to -> overrides, i );
        assert ( orig != NULL );
        if ( orig -> type == eVirtual )
        {
            void *ignore;

            /* since the virtual productions in one parent could be
               defined by another parent, test for the possibility */
            const KSymbol *def = KSymTableFindSymbol ( tbl, orig );
            if ( def != NULL )
                VectorSwap ( & to -> overrides, i, def, & ignore );
            else
            {
                /* copy the original */
                KSymbol *copy;
                rc_t rc = KSymbolCopy ( scope, & copy, orig );
                if ( rc != 0 )
                    return true;

                /* replace the parent virtual with an updatable copy */
                VectorSwap ( & to -> overrides, i, copy, & ignore );
            }
        }
    }
    return false;
}

static
rc_t STableExtend ( KSymTable *tbl, STable *self, const STable *dad )
{
    rc_t rc;

    /* reject if direct parent already there */
    if ( VectorDoUntil ( & self -> parents, false, STableHasDad, ( void* ) dad ) )
        return RC ( rcVDB, rcSchema, rcParsing, rcTable, rcExists );

    /* if parent is already in ancestry, treat as redundant */
    if ( VectorFind ( & self -> overrides, & dad -> id, NULL, STableOverridesCmp ) != NULL )
        return VectorAppend ( & self -> parents, NULL, dad );

    /* enter scope for this table */
    rc = push_tbl_scope ( tbl, self );
    if ( rc != 0 )
        return rc;

    /* test for any collisions */
    if ( STableTestForCollisions ( ( void* ) dad, tbl ) ||
         VectorDoUntil ( & dad -> overrides, false, STableOverridesTestForCollisions, tbl ) )
    {
        pop_tbl_scope ( tbl, self );
        return RC ( rcVDB, rcSchema, rcParsing, rcName, rcExists );
    }

    /* pop table scope */
    pop_tbl_scope ( tbl, self );

    /* add "dad" to parent list */
    rc = VectorAppend ( & self -> parents, NULL, dad );
    if ( rc != 0 )
        return rc;

    /* copy column names from parent - should already contain all grandparents */
    if ( VectorDoUntil ( & dad -> cname, false, STableCopyColumnNames, self ) )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    /* add "dad" to overrides */
    rc = STableOverridesMake ( & self -> overrides, dad, & dad -> vprods );
    if ( rc == 0 )
    {
        /* add all grandparents */
        if ( VectorDoUntil ( & dad -> overrides, false, STableOverridesClone, & self -> overrides ) )
            rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    }
    else if ( GetRCState ( rc ) == rcExists )
    {
        rc = 0;
    }
    return rc;
}

/* CloneExtend
 *  creates an initially transparent table extension
 *  used by cursor to permit addition of implicit productions
 */
rc_t STableCloneExtend ( const STable *self, STable **clone, VSchema *schema )
{
    rc_t rc;
    KSymTable tbl;

    STable *table = malloc ( sizeof * table );
    if ( table == NULL )
        return RC ( rcVDB, rcSchema, rcUpdating, rcMemory, rcExhausted );

    /* need to take a deep look at scope */
    rc = init_symtab ( & tbl, schema );
    if ( rc != 0 )
        return rc;

    /* this takes care of initializing "scope" */
    memset ( table, 0, sizeof * table );

    /* take name and version, since this is an anonymous extension */
    table -> name = KSymTableFindSymbol ( & tbl, self -> name );
    KSymTableWhack ( & tbl );
    assert ( table -> name != NULL );
    table -> src_file = self -> src_file;
    table -> src_line = self -> src_line;
    table -> version = self -> version;

    /* take on table-wide blob limit */
    if ( self -> limit != NULL )
    {
        table -> limit = self -> limit;
        atomic32_inc ( & ( ( SExpression* ) table -> limit ) -> refcount );
    }

    /* initialize all vectors for single-inheritance */
    VectorInit ( & table -> parents, 0, 1 );
    VectorInit ( & table -> overrides, 0, VectorLength ( & self -> overrides ) + 1 );
    VectorInit ( & table -> col, 0, 16 );
    VectorInit ( & table -> cname, 0, 16 );
    VectorInit ( & table -> phys, 0, 16 );
    VectorInit ( & table -> prod, 0, 64 );
    VectorInit ( & table -> vprods, 1, 16 );
    VectorInit ( & table -> syms, 1, 32 );

    rc = init_tbl_symtab ( & tbl, schema, table );
    if ( rc == 0 )
        rc = STableExtend ( & tbl, table, self );
    if ( rc == 0 )
    {
        rc = push_tbl_scope ( & tbl, table );
        if ( rc == 0 )
        {
            if ( VectorDoUntil ( & table -> overrides, false, STableScanVirtuals, & tbl ) )
                rc = RC ( rcVDB, rcSchema, rcUpdating, rcMemory, rcExhausted );
        }
    }

    KSymTableWhack ( & tbl );

    if ( rc == 0 )
    {
        /* add table to schema */
        rc = VectorAppend ( & schema -> tbl, & table -> id, table );
        if ( rc == 0 )
        {
            void *dad;
            uint32_t idx;
            SNameOverload *name = ( void* ) table -> name -> u . obj;

            /* find ourselves in table overloads */
            dad = VectorFind ( & name -> items, & table -> version, & idx, STableCmp );
            assert ( dad != NULL );
            assert ( dad == ( void* ) self );

            /* set clone in our place */
            VectorSwap ( & name -> items, idx, table, & dad );

            * clone = table;
            return 0;
        }
    }

    STableWhack ( table, NULL ), table = NULL;
    return rc;
}

/* ImplicitPhysMember
 *  adds an implicit physical member
 */
rc_t STableImplicitPhysMember ( STable *self,
    const VTypedecl *td, KSymbol *sym, const String *name )
{
    rc_t rc;
    SPhysMember *m = malloc ( sizeof * m );
    if ( m == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    else
    {
        memset ( m, 0, sizeof * m );

        /* if discovered as static column, give it a type */
        if ( td != NULL )
            m -> td = * td;

        /* create name symbol as required */
        if ( sym == NULL )
        {
            rc = KSymbolMake ( & sym, name, ePhysMember, m );
            if ( rc == 0 )
                BSTreeInsert ( & self -> scope, & sym -> n, KSymbolSort );
        }
        if ( sym != NULL )
        {
            /* complete handshake with symbol */
            m -> name = sym;
            sym -> u . obj = m;
            sym -> type = ePhysMember;

            /* add member to table */
            m -> cid . ctx = self -> id;
            rc = VectorAppend ( & self -> phys, & m -> cid . id, m );
            if ( rc == 0 )
                return 0;
        }

        SPhysMemberWhack ( m, NULL );
    }
    return rc;
}

/* ImplicitColMember
 *  adds an implicit column member of simple or incomplete type
 *
 *  "cname" [ IN ] - column name
 *
 *  "pname" [ IN ] - name of physical column
 */
rc_t STableImplicitColMember ( STable *self,
    const String *cname, const String *pname )
{
    rc_t rc;

    /* create SColumn */
    SColumn *col = malloc ( sizeof * col );
    if ( col == NULL )
        rc = RC ( rcVDB, rcTable, rcUpdating, rcMemory, rcExhausted );
    else
    {
        const KSymbol *psym;
        memset ( col, 0, sizeof * col );

        /* look up SPhysMember */
        psym = ( const KSymbol* ) BSTreeFind ( & self -> scope, pname, KSymbolCmp );
        if ( psym == NULL )
            rc = RC ( rcVDB, rcTable, rcUpdating, rcColumn, rcNotFound );
        else
        {
            KSymbol *csym;

            /* create column symbol */
            rc = KSymbolMake ( & csym, cname, eColumn, col );
            if ( rc == 0 )
            {
                BSTNode *exist;

                /* complete handshake with symbol */
                col -> name = csym;

                /* insert into table scope and reject on collision
                   deep check should have been executed previously */
                rc = BSTreeInsertUnique ( & self -> scope, & csym -> n, & exist, KSymbolSort );
                if ( rc == 0 )
                {
                    /* take column typedecl ( if known ) from physical */
                    SPhysMember *phys = ( void* ) psym -> u . obj;
                    col -> td = phys -> td;

                    /* cross-link the external and physical columns */
                    rc = SSymExprMake ( & col -> read, psym, ePhysExpr );
                    if ( rc == 0 )
                        rc = SSymExprMake ( & phys -> expr, csym, eColExpr );
                    if ( rc == 0 )
                    {
                        /* add column to table */
                        col -> cid . ctx = self -> id;
                        rc = VectorAppend ( & self -> col, & col -> cid . id, col );
                        if ( rc == 0 )
                        {
                            void *ignore;
                            SNameOverload *name;

                            /* create a column name with a single typed column */
                            rc = SNameOverloadMake ( & name, csym, 0, 1 );
                            if ( rc == 0 )
                            {
                                /* being the only column, "col" may be simply
                                   inserted rather than using a sorted insert */
                                rc = VectorAppend ( & name -> items, NULL, col );
                                if ( rc == 0 )
                                {
                                    /* finally, insert name into table */
                                    rc = VectorAppend ( & self -> cname, & name -> cid . id, name );
                                    if ( rc == 0 )
                                    {
                                        name -> cid . ctx = self -> id;
                                        return 0;
                                    }
                                }

                                SNameOverloadWhack ( name, NULL );
                            }

                            VectorSwap ( & self -> col, col -> cid . id, NULL, & ignore );
                        }

                        SExpressionWhack ( phys -> expr ), phys -> expr = NULL;
                    }

                    BSTreeUnlink ( & self -> scope, & csym -> n );
                }

                KSymbolWhack ( & csym -> n, NULL );
            }
        }

        SColumnWhack ( col, NULL );
    }
    return rc;
}


/* Compare
 */
static
int KSymbolDeepCompare ( const KSymbol *a, const KSymbol *b )
{
    int diff;

    /* the same symbol */
    if ( a == b )
        return 0;

    /* parents are first */
    if ( a -> dad != NULL )
    {
        if ( b -> dad != NULL )
        {
            /* both symbols have parents */
            diff = KSymbolDeepCompare ( a -> dad, b -> dad );
            if ( diff != 0 )
                return diff;
        }
        else
        {
            /* "b" is a root */
            diff = KSymbolDeepCompare ( a -> dad, b );
            if ( diff != 0 )
                return diff;
            return 1;
        }
    }
    else if ( b -> dad != NULL )
    {
        /* "a" is a root */
        diff = KSymbolDeepCompare ( a, b -> dad );
        if ( diff != 0 )
            return diff;
        return -1;
    }

    /* perform textual comparison */
    return KSymbolSort ( & a -> n, & b -> n );
}

static
int CC STableNameSort ( const void **a, const void **b, void *ignore )
{
    int diff;
    const STable *tb = * b;
    const STable *ta = * a;
    if ( tb == NULL )
        return ta != NULL;
    if ( ta == NULL )
        return -1;
    diff = KSymbolDeepCompare ( ta -> name, tb -> name );
    if ( diff != 0 )
        return diff;
    if ( ta -> version < tb -> version )
        return -1;
    if ( ta -> version > tb -> version )
        return 1;
    return 0;
}

enum
{
    stbl_cmp_insertion = 1 << 0,
    stbl_cmp_deletion  = 1 << 1,
    stbl_cmp_mismatch  = 1 << 2,
    stbl_cmp_newer     = 1 << 3,
    stbl_cmp_older     = 1 << 4
};

static
rc_t STableCompare ( const STable *a, const STable *b, const STable **newer, bool exhaustive )
{
    rc_t stage_rc, cmp_rc = 0;
    uint32_t stage_bits, cmp_bits = 0;

    int diff;
    Vector va, vb;
    uint32_t ia, ib, ca, cb;

    assert ( ( a -> version >> 24 ) == ( b -> version >> 24 ) );

    PARSE_DEBUG (( "STableCompare: testing %N #%.3V against #%.3V\n",
                   a -> name, a -> version, b -> version ));

    /* guess the newer of the two based upon version alone */
    * newer = a -> version >= b -> version ? a : b;

    /* test #1 - immediate parents */
    ca = VectorLength ( & a -> parents );
    cb = VectorLength ( & b -> parents );
    if ( ca != cb || ca != 0 )
    {
        /* make a copy */
        cmp_rc = VectorCopy ( & a -> parents, & va );
        if ( cmp_rc != 0 )
            return cmp_rc;
        cmp_rc = VectorCopy ( & b -> parents, & vb );
        if ( cmp_rc != 0 )
        {
            VectorWhack ( & va, NULL, NULL );
            return cmp_rc;
        }
        if ( ca > 1 )
            VectorReorder ( & va, STableNameSort, NULL );
        if ( cb > 1 )
            VectorReorder ( & vb, STableNameSort, NULL );

        for ( stage_bits = ia = ib = 0; ia < ca && ib < cb; )
        {
            const STable *pa = ( const void* ) VectorGet ( & va, ia );
            const STable *pb = ( const void* ) VectorGet ( & vb, ib );
            if ( pa == pb )
            {
                ++ ia, ++ ib;
                continue;
            }
            diff = KSymbolDeepCompare ( pa -> name, pb -> name );
            if ( diff < 0 )
            {
                stage_bits |= stbl_cmp_insertion;
                ++ ia;
            }
            else if ( diff > 0 )
            {
                stage_bits |= stbl_cmp_deletion;
                ++ ib;
            }
            else
            {
                if ( pa -> version > pb -> version )
                    stage_bits |= stbl_cmp_newer;
                else if ( pa -> version < pb -> version )
                    stage_bits |= stbl_cmp_older;

                ++ ia, ++ ib;
            }
        }

        VectorWhack ( & va, NULL, NULL );
        VectorWhack ( & vb, NULL, NULL );

        PARSE_DEBUG (( "STableCompare: %N #%.3V vs. #%.3V - PARENT COMPARISON:\n%s%s%s%s%s"
                       , a -> name, a -> version, b -> version
                       , ( stage_bits == 0 )                 ? "  no differences detected\n" : ""
                       , ( stage_bits & stbl_cmp_insertion ) ? "  insertion(s) detected\n" : ""
                       , ( stage_bits & stbl_cmp_deletion )  ? "  deletion(s) detected\n" : ""
                       , ( stage_bits & stbl_cmp_newer )     ? "  newer ancestor(s) detected\n" : ""
                       , ( stage_bits & stbl_cmp_older )     ? "  older ancestor(s) detected\n" : ""
            ));

        if ( stage_bits != 0 )
        {
            /* if the table versions are the same, the parents should not have insertions or deletions */
            if ( a -> version == b -> version && ( stage_bits & ( stbl_cmp_insertion | stbl_cmp_deletion ) ) != 0 )
            {
                stage_rc = RC ( rcVDB, rcSchema, rcParsing, rcTable, rcInconsistent );
                PLOGERR ( klogErr, ( klogErr, stage_rc, "STableCompare: illegal redeclaration of table '$(tbl)' - differing parents."
                                     , "tbl=%N#%.3V"
                                     , a -> name, b -> version ) );
                if ( ! exhaustive )
                    return stage_rc;
                if ( cmp_rc == 0 )
                    cmp_rc = stage_rc;
            }

            /* if the tables have same parent names but conflicting versions */
            if ( ( stage_bits & ( stbl_cmp_newer | stbl_cmp_older ) ) == ( stbl_cmp_newer | stbl_cmp_older ) )
            {
                stage_rc = RC ( rcVDB, rcSchema, rcParsing, rcTable, rcInconsistent );
                PLOGERR ( klogErr, ( klogErr, stage_rc, "STableCompare: illegal redeclaration of table '$(tbl)' - both older and newer parent versions."
                                     , "tbl=%N#%.3V"
                                     , a -> name, b -> version ) );
                if ( ! exhaustive )
                    return stage_rc;
                if ( cmp_rc == 0 )
                    cmp_rc = stage_rc;
            }
            else
            {
                /* if "a" claims to be newer than "b" */
                if ( a -> version > b -> version )
                {
                    if ( ( stage_bits & stbl_cmp_older ) != 0 )
                    {
                        stage_rc = RC ( rcVDB, rcSchema, rcParsing, rcTable, rcInconsistent );
                        PLOGERR ( klogErr, ( klogErr, stage_rc, "STableCompare: illegal redeclaration of table '$(tbl)' - version $(new_vers) has older parents than version $(old_vers)."
                                             , "tbl=%N,new_vers=#%.3V,old_vers=#%.3V"
                                             , a -> name, a -> version, b -> version ) );
                        if ( ! exhaustive )
                            return stage_rc;
                        if ( cmp_rc == 0 )
                            cmp_rc = stage_rc;
                    }
                }

                /* if "b" claims to be newer than "a" */
                else if ( a -> version < b -> version )
                {
                    if ( ( stage_bits & stbl_cmp_newer ) != 0 )
                    {
                        stage_rc = RC ( rcVDB, rcSchema, rcParsing, rcTable, rcInconsistent );
                        PLOGERR ( klogErr, ( klogErr, stage_rc, "STableCompare: illegal redeclaration of table '$(tbl)' - version $(old_vers) has newer parents than version $(new_vers)."
                                             , "tbl=%N,new_vers=#%.3V,old_vers=#%.3V"
                                             , a -> name, b -> version, a -> version ) );
                        if ( ! exhaustive )
                            return stage_rc;
                        if ( cmp_rc == 0 )
                            cmp_rc = stage_rc;
                    }
                }

                /* they are the same - check parent versions */
                else if ( ( stage_bits & ( stbl_cmp_newer | stbl_cmp_older ) ) != 0 )
                {
                    PLOGMSG ( klogInfo, ( klogInfo, "STableCompare: table '$(tbl)' differs in parent hierarchy"
                                          " - latest declaration chosen automatically.", "tbl=%N#%.3V"
                                          , a -> name, b -> version ));

                    * newer = ( stage_bits & stbl_cmp_newer ) ? a : b;
                }
            }

            cmp_bits |= stage_bits;
        }
    }

    /* test #2 - immediate extern columns */
    /* test #3 - immediate physical columns */
    /* test #4 - immediate productions */
    /* test #5 - immediate virtual productions */
    /* test #6 - deep parents */
    /* test #7 - deep extern columns */
    /* test #8 - deep physical columns */
    /* test #9 - deep productions */
    /* test #10 - deep virtual productions */

    return cmp_rc;
}


/* Mark
 */
void CC STableClearMark ( void *item, void *ignore )
{
    STable *self = item;
    self -> marked = false;
}


void CC STableMark ( void * item, void * data )
{
    STable * self = item;
    if ( self != NULL && ! self -> marked )
    {
        self -> marked = true;
        SFunctionMark ( ( void * )self -> untyped, data );
        VectorForEach ( & self -> col, false, SColumnMark, data );
        VectorForEach ( & self -> phys, false, SPhysMemberMark, data );
        VectorForEach ( & self -> prod, false, SProductionMark, data );
        VectorForEach ( & self -> parents, false, STableMark, data );
    }
}

void STableNameMark ( const SNameOverload *self, const VSchema *schema )
{
    if ( self != NULL )
    {
        VectorForEach ( & self -> items, false, STableMark, ( void * )schema );
    }
}


/* Dump
 *  dump "table" { }
 */
rc_t STableDump ( const STable *self, struct SDumper *d )
{
    d -> rc = FQNDump ( self != NULL ? self -> name : NULL, d );
    if ( d -> rc == 0 && self != NULL )
        d -> rc = SDumperVersion ( d, self -> version );
    return d -> rc;
}

#if _DEBUGGING
static
bool CC SProductionDumpVirtuals ( void *item, void *data )
{
    SDumper *b = data;
    const KSymbol *sym = ( const void* ) item;

    switch ( sym -> type )
    {
    case eVirtual:
        b -> rc = SDumperPrint ( b, "\t *  virtual %N = 0;\n", sym );
        break;
    case eProduction:
    {
        const SProduction *prod = sym -> u . obj;
        b -> rc = SDumperPrint ( b, "\t *  %E %N;\n", prod -> fd, sym );
        break;
    }
    case eColumn:
    {
        b -> rc = SDumperPrint ( b, "\t *  column %N;\n", sym );
        break;
    }
    case ePhysMember:
    {
        const SPhysMember *phys = sym -> u . obj;
        if ( phys -> type != NULL )
            b -> rc = SDumperPrint ( b, "\t *  physical %E %N;\n", phys -> type, sym );
        else
            b -> rc = SDumperPrint ( b, "\t *  physical %T %N;\n", & phys -> td, sym );
        break;
    }}

    return ( b -> rc != 0 ) ? true : false;
}

static
bool CC SProductionDumpOverrides ( void *item, void *data )
{
    SDumper *b = data;
    const STableOverrides *to = ( const void* ) item;
    if ( VectorLength ( & to -> overrides ) == 0 )
        return false;

    b -> rc = SDumperPrint ( b, "\n\t/* %N inherited virtual productions\n", to -> dad -> name );
    if ( b -> rc != 0 )
        return true;
    if ( VectorDoUntil ( & to -> overrides, false, SProductionDumpVirtuals, b ) )
        return true;
    b -> rc = SDumperPrint ( b, "\t */\n" );

    return ( b -> rc != 0 ) ? true : false;
}
#endif

static
bool STableDumpBody ( const STable *self, SDumper *b )
{
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( self -> untyped != NULL )
    {
        b -> rc = SDumperPrint ( b, compact ? "__untyped=%N();" : "\t__untyped = %N ();\n"
                                 , self -> untyped -> name
            );
        if ( b -> rc != 0 )
            return true;
    }
    if ( self -> limit != NULL )
    {
        b -> rc = SDumperPrint ( b, compact ? "column default limit=%E;" : "\tcolumn default limit = %E;\n"
                                 , self -> limit
            );
        if ( b -> rc != 0 )
            return true;
    }
#if SLVL >= 8
    if ( VectorDoUntil ( & self -> col, false, SColumnDefDump, b ) )
        return true;
#endif
    if ( VectorDoUntil ( & self -> prod, false, SProductionDefDump, b ) )
        return true;
#if SLVL >= 7
    if ( VectorDoUntil ( & self -> phys, false, SPhysMemberDefDump, b ) )
        return true;
#endif
#if _DEBUGGING
    if ( SDumperMode ( b ) == sdmPrint )
    {
        if ( VectorDoUntil ( & self -> overrides, false, SProductionDumpOverrides, b ) )
            return true;

        if ( VectorLength ( & self -> vprods ) != 0 )
        {
            b -> rc = SDumperPrint ( b, "\n\t/* %N virtual productions\n", self -> name );
            if ( b -> rc != 0 )
                return true;
            if ( VectorDoUntil ( & self -> vprods, false, SProductionDumpVirtuals, b ) )
                return true;
            b -> rc = SDumperPrint ( b, "\t */\n" );
            if ( b -> rc != 0 )
                return true;
        }
    }
#endif

    return false;
}

static
bool CC STableDumpParents ( void *item, void *data )
{
    SDumper *b = data;
    const STable *self = ( const void* ) item;

    b -> rc = SDumperPrint ( b, "\v%N%V", self -> name, self -> version );

    SDumperSepString ( b, SDumperMode ( b ) == sdmCompact ? "," : ", " );

    return ( b -> rc != 0 ) ? true : false;
}

bool CC STableDefDump ( void *item, void *data )
{
    bool rtn;
    SDumper *b = data;
    const STable *self = ( const void* ) item;
    bool compact = SDumperMode ( b ) == sdmCompact ? true : false;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    b -> rc = SDumperPrint ( b, compact ? "table %N" : "\ttable %N", self -> name );

    if ( b -> rc == 0 )
        b -> rc = SDumperVersion ( b, self -> version );

    if ( b -> rc == 0 )
    {
        SDumperSepString ( b, compact ? "=" : " = " );
        VectorDoUntil ( & self -> parents, false, STableDumpParents, b );
    }

    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, compact ? "{" : "\n\t{\n" );

    if ( b -> rc != 0 )
        return true;

    if ( ! compact )
        SDumperIncIndentLevel ( b );
    rtn = STableDumpBody ( self, b );
    if ( ! compact )
        SDumperDecIndentLevel ( b );

    if ( rtn )
        return true;

    b -> rc = SDumperPrint ( b, compact ? "}" : "\t}\n" );

    return ( b -> rc != 0 ) ? true : false;
}

#endif


/*--------------------------------------------------------------------------
 * VSchema
 */

#if SLVL >= 7

/*
 * physical-name      = '.' ID
 */
void physical_name ( const KSymTable *tbl, KTokenSource *src, KToken *t, const SchemaEnv *env )
{
    KToken t2;
    if ( KTokenizerNext ( kDefaultTokenizer, src, & t2 ) -> id != eIdent ||
         t -> str . addr + 1 != t2 . str . addr )
    {
        /* this is just a dot */
        KTokenSourceReturn ( src, & t2 );
    }
    else
    {
        /* this is a physical name */
        t -> str . size += t2 . str . size;
        t -> str . len += t2 . str . len;
        t -> id = eIdent;
        t -> sym = KSymTableFind ( tbl, & t -> str );
        if ( t -> sym != NULL )
            t -> id = t -> sym -> type;
    }
}

/*
 *    physical-decl      = 'physical' [ 'column' ] KCOL '{' <physical-stmts> '}'
 *    physical-stmts     = <physical-stmt> ';' [ <physical-stmts> ]
 *    physical-stmt      = 'read' ( '=' | '+= ) <cond-expr>
 *                       | 'write' ( '=' | '+=' ) <cond-expr>
 *                       | '__untyped' '=' <untyped-expr>
 */
static
rc_t physical_mbr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SPhysMember *m )
{
    rc_t rc;

    /* determine static flag */
    if ( t -> id == kw_static )
    {
        next_token ( tbl, src, t );
        m -> stat = true;
    }

    /* skip keywords */
    if ( t -> id == kw_physical )
        next_token ( tbl, src, t );
    if ( t -> id == kw_column )
        next_token ( tbl, src, t );

    /* if member type begins with schema parameters... */
    if ( t -> id == eLeftAngle )
        rc = phys_encoding_expr ( tbl, src, t, env, self, & m -> td, & m -> type );
    else
    {
        /* get either a typename or a physical column name */
        rc = next_fqn ( tbl, src, t, env );
        if ( rc == 0 )
        {
            /* column is probably based upon a physical structure */
            if ( t -> id == ePhysical )
                rc = phys_encoding_expr ( tbl, src, t, env, self, & m -> td, & m -> type );            
            else if ( t -> id != eDatatype )
                return KTokenExpected ( t, klogErr, "typename or physical column type" );
            else
                rc = typedecl ( tbl, src, t, env, self, & m -> td );
        }
    }
    if ( rc != 0 )
        return KTokenFailure ( t, klogErr, rc, "typename or physical column type" );

    /* get name */
    if ( t -> id != ePeriod )
        return KTokenExpected ( t, klogErr, "physical name starting with period" );
    physical_name ( tbl, src, t, env );
    if ( t -> id == eIdent )
    {
        /* enter name into scope */
        rc = KSymTableCreateConstSymbol ( tbl, & m -> name, & t -> str, ePhysMember, m );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
    }
    else if ( t -> id == ePhysMember )
        return KTokenExpected ( t, klogErr, "undefined physical member name" );
    else if ( t -> id != eForward && t -> id != eVirtual )
        return KTokenExpected ( t, klogErr, "physical member name" );
    else
    {
        m -> name = t -> sym;
        ( ( KSymbol* ) t -> sym ) -> u . obj = m;
        ( ( KSymbol* ) t -> sym ) -> type = ePhysMember;
    }


    /* get the assignment expression */
    if ( next_token ( tbl, src, t ) -> id == eAssign )
    {
        rc = cond_expr ( tbl, src, next_token ( tbl, src, t ), env, self, & m -> expr );
        if ( rc != 0 )
            return KTokenFailure ( t, klogErr, rc, "assignment expression" );
    }

    /* expect we're done */
    return expect ( tbl, src, t, eSemiColon, ";", true );
}

static
rc_t physical_member ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    rc_t rc;
    SPhysMember *m = malloc ( sizeof * m );
    if ( m == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    else
    {
        memset ( m, 0, sizeof * m );
        rc = physical_mbr ( tbl, src, t, env, self, m );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & table -> phys, & m -> cid . id, m );
            if ( rc == 0 )
                return 0;
        }

        SPhysMemberWhack ( m, NULL );
    }
    return rc;
}

static
rc_t implicit_physical_member ( KSymTable *tbl, const SchemaEnv *env,
    STable *table, SColumn *c, KSymbol *sym )
{
    rc_t rc;
    SPhysMember *m = malloc ( sizeof * m );
    if ( m == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    else
    {
        /* create initialized simple physical member */
        memset ( m, 0, sizeof * m );
        m -> simple = true;

        /* capture type information */
        m -> td = c -> td;
        if ( c -> ptype != NULL )
        {
            m -> type = c -> ptype;
            atomic32_inc ( & ( ( SExpression* ) c -> ptype ) -> refcount );
        }

        /* link up with name */
        m -> name = sym;
        sym -> u . obj = m;

        /* now create simple input expression */
        rc = SSymExprMake ( & m -> expr, c -> name, eColExpr );
        if ( rc == 0 )
            rc = SSymExprMake ( & c -> read, sym, ePhysExpr );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & table -> phys, & m -> cid . id, m );
            if ( rc == 0 )
                return 0;
        }

        SPhysMemberWhack ( m, NULL );
    }
    return rc;
}
#endif

#if SLVL >= 8
/*
 * column-stmt        = 'read' '=' <cond-expr>
 *                    | 'validate' '=' <cond-expr>
 *                    | 'limit' '=' UINT_EXPR
 *                    | ';'
 */
static
rc_t column_stmt ( KSymTable *tbl, KTokenSource *src, KToken *t,
     const SchemaEnv *env, VSchema *self, SColumn *c, const char **expected )
{
    rc_t rc;

    switch ( t -> id )
    {
    case eSemiColon:
        return 0;

    case kw_read:

        if ( c -> simple )
            break;

        if ( c -> read != NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcProduction, rcExists );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        rc = expect ( tbl, src, next_token ( tbl, src, t ), eAssign, "=", true );
        if ( rc == 0 )
        {
            rc = cond_expr ( tbl, src, t, env, self, & c -> read );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "read expression" );
            else
            {
                if ( c -> read_only )
                    * expected = "}";
                else if ( c -> validate == NULL && c -> limit == NULL )
                    * expected = "validate or limit or }";
                else if ( c -> validate == NULL )
                    * expected = "validate or }";
                else if ( c -> limit == NULL )
                    * expected = "limit or }";
                else
                    * expected = "}";
                return 0;
            }
        }
        break;

    case kw_validate:

        if ( c -> read_only || c -> simple )
            break;

        if ( c -> validate != NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcProduction, rcExists );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        rc = expect ( tbl, src, next_token ( tbl, src, t ), eAssign, "=", true );
        if ( rc == 0 )
        {
            rc = cond_expr ( tbl, src, t, env, self, & c -> validate );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "validate expression" );
            else
            {
                if ( c -> read == NULL && c -> limit == NULL )
                    * expected = "read or limit or }";
                else if ( c -> read == NULL )
                    * expected = "read or }";
                else if ( c -> limit == NULL )
                    * expected = "limit or }";
                else
                    * expected = "}";
                return 0;
            }
        }
        break;

    case kw_limit:

        if ( c -> read_only )
            break;

        if ( c -> limit != NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcConstraint, rcExists );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        rc = expect ( tbl, src, next_token ( tbl, src, t ), eAssign, "=", true );
        if ( rc == 0 )
        {
            rc = const_expr ( tbl, src, t, env, self, & c -> limit );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "limit constraint" );
            else
            {
                if ( c -> read == NULL && c -> validate == NULL )
                    * expected = "read or validate or }";
                else if ( c -> read == NULL )
                    * expected = "read or }";
                else if ( c -> validate == NULL )
                    * expected = "validate or }";
                else
                    * expected = "}";
                return 0;
            }
        }
        break;
    }

    return KTokenExpected ( t, klogErr, * expected );
}

/*
 * column-body        = '{' <column-stmts> '}'
 * column-stmts       = <column-stmt> ';' [ <column-stmts> ]
 */
static
rc_t column_body ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SColumn *c )
{
    const char *expected;

    rc_t rc = expect ( tbl, src, t, eLeftCurly, "{", true );
    if ( rc != 0 )
        return rc;

    expected = c -> read_only ? "read or }" :
        ( c -> simple ? "limit or }" :  "read or validate or limit or }" );

    while ( t -> id != eRightCurly )
    {
        rc = column_stmt ( tbl, src, t, env, self, c, & expected );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eSemiColon, ";", true );
        if ( rc != 0 )
            return rc;
    }

    if ( c -> read == NULL && c -> validate == NULL )
        c -> simple = true;

    return expect ( tbl, src, t, eRightCurly, "}", true );
}

/*
 * column-decl        = <typedecl> ID <column-body>
 */
static
rc_t typed_column_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table, SColumn *c )
{
    rc_t rc = 0;

    /* if column was forwarded, give it a type */
    if ( t -> id == eForward || t -> id == eVirtual )
    {
        c -> name = t -> sym;
        t -> sym -> type = eColumn;
    }

    /* catch case where column exists */
    else if ( t -> id == eColumn )
    {
        const SNameOverload *name = t -> sym -> u . obj;
        if ( VectorFind ( & name -> items, & c -> td, NULL, SColumnCmp ) != NULL )
            return KTokenExpected ( t, klogErr, "new column name" );
        c -> name = t -> sym;
    }
    else
    {
        /* allow names defined in scopes other than table and intrinsic */
        if ( t -> sym != NULL )
        {
            KTokenSourceReturn ( src, t );
            next_shallow_token ( tbl, src, t, true );
        }

        if ( t -> id != eIdent )
            return KTokenExpected ( t, klogErr, "column name" );

        rc = KSymTableCreateConstSymbol ( tbl, & c -> name, & t -> str, eColumn, NULL );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
    }

    /* we have "[ 'readonly' | ... ] 'column' TYPE ID" by now.
       accept a simple assignment for read/validate,
       or a semi-colon for implicit physical, or a compound
       statement body for read/validate/limit statements */
    switch ( next_token ( tbl, src, t ) -> id )
    {
    case eSemiColon:

        /* acceptable unless readonly */
        if ( c -> read_only )
            rc = KTokenExpected ( t, klogErr, "= or {" );

        /* this looks like a simple column */
        c -> simple = true;
        break;

    case eAssign:

        /* if a simple column ( implicit physical ),
           then there cannot be a read expression */
        if ( c -> simple )
            rc = KTokenExpected ( t, klogErr, "; after simple column" );
        else
        {
            /* simple read expression */
            next_token ( tbl, src, t );
            rc = cond_expr ( tbl, src, t, env, self, & c -> read );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "read expression" );
            else
                rc = expect ( tbl, src, t, eSemiColon, ";", true );
        }
        break;

    case eLeftCurly:
        /* this is a canonical-form column */
        rc = column_body ( tbl, src, t, env, self, c );
        break;

    default:
        rc = KTokenExpected ( t, klogErr, c -> read_only ? "= or {" : "; or = or {" );
    }

    /* check for a simple column */
    if ( rc == 0 && c -> simple )
    {
        String physname;
        char physnamebuff [ 256 ];

        /* check for existence of corresponding physical member */
        if ( c -> name -> name . size >= sizeof physnamebuff )
        {
            /* this is a very long column name... */
            rc = RC ( rcVDB, rcSchema, rcParsing, rcName, rcExcessive );
            KTokenFailure ( t, klogErr, rc, "column name" );
        }
        else
        {
            KSymbol *sym;

            /* tack a dot onto the beginning and look up the symbol */
            physnamebuff [ 0 ] = '.';
            memcpy ( & physnamebuff [ 1 ], c -> name -> name . addr, c -> name -> name . size );
            StringInit ( & physname, physnamebuff, c -> name -> name . size + 1, c -> name -> name . len + 1 );
            sym = KSymTableFind ( tbl, & physname );

            /* if the symbol exists, then this CANNOT be a simple column */
            if ( sym != NULL && ! ( sym -> type == eForward || sym -> type == eVirtual ) )
            {
                /* check for explicit physical type */
                if ( c -> ptype != NULL )
                {
                    rc = RC ( rcVDB, rcSchema, rcParsing, rcName, rcExists );
                    KTokenFailure ( t, klogErr, rc, "implicit physical column previously declared" );
                }
                else
                {
                    rc = RC ( rcVDB, rcSchema, rcParsing, rcExpression, rcNotFound );
                    KTokenFailure ( t, klogErr, rc, "missing column read or validate expression" );
                }
            }
            else if ( ( c -> td . type_id & 0xC0000000 ) != 0 )
            {
                rc = RC ( rcVDB, rcSchema, rcParsing, rcType, rcIncorrect );
                KTokenFailure ( t, klogErr, rc, "simple columns cannot have typeset as type" );
            }
            else
            {
                if ( sym != NULL )
                    sym -> type = ePhysMember;
                else
                {
                    rc = KSymTableCreateSymbol ( tbl, & sym, & physname, ePhysMember, NULL );
                    if ( rc != 0 )
                        KTokenFailure ( t, klogErr, rc, "failed to create symbol" );
                }
                if ( rc == 0 )
                {
                    rc = implicit_physical_member ( tbl, env, table, c, sym );
                }
            }
        }
    }

    return rc;
}

static
rc_t column_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table, SColumn *c )
{
    rc_t rc;

    /* intercept physical type unless marked read-only */
    if ( ! c -> read_only )
    {
        switch ( t -> id )
        {
        case eNamespace:
            rc = next_fqn ( tbl, src, t, env );
            if ( rc != 0 )
                return KTokenFailure ( t, klogErr, rc, "typename or physical column type" );
            if ( t -> id != ePhysical )
                break;
        case ePhysical:
        case eLeftAngle:
            rc = phys_encoding_expr ( tbl, src, t, env, self, & c -> td, & c -> ptype );
            if ( rc != 0 )
                return KTokenFailure ( t, klogErr, rc, "typename or physical column type" );

            /* this column MUST be simple */
            c -> simple = true;
            return typed_column_decl ( tbl, src, t, env, self, table, c );
        }
    }

#if ALLOW_COLUMN_TYPESET
    /* read typedecl or typeset */
    rc = typespec ( tbl, src, t, env, self, & c -> td );
#else
    /* read hard typedecl */
    rc = typedecl ( tbl, src, t, env, self, & c -> td );
#endif
    if ( rc != 0 )
        return KTokenExpected ( t, klogErr, "column type" );

    /* finish the parse with a normal type */
    return typed_column_decl ( tbl, src, t, env, self, table, c );
}

static
rc_t column_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    rc_t rc;
    SColumn *c;
    bool dflt, read_only;

    /* parse 'virtual' and 'default' keywords */
    for ( dflt = read_only = false;
          t -> id != kw_column; next_token ( tbl, src, t ) )
    {
        switch ( t -> id )
        {
        case kw_default:
            if ( dflt )
                break;
            dflt = true;
            continue;

        case kw_extern:
            continue;

        case kw_readonly:
            if ( read_only )
                break;
            read_only = true;
            continue;
        }

        return KTokenExpected ( t, klogErr, "column" );
    }

    /* consume 'column' keyword and look for 'default' or 'limit' */
    switch ( next_token ( tbl, src, t ) -> id )
    {
    case kw_default:
    {
        KToken t2 = * t;
        KTokenSource src2 = * src;
        if ( next_token ( tbl, src, t ) -> id != kw_limit )
        {
            * t = t2;
            * src = src2;
            break;
        }
    }
    case kw_limit:

        if ( dflt || read_only )
            break;

        if ( table -> limit != NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcConstraint, rcExists );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        rc = expect ( tbl, src, next_token ( tbl, src, t ), eAssign, "=", true );
        if ( rc == 0 )
        {
            rc = const_expr ( tbl, src, t, env, self, & table -> limit );
            if ( rc != 0 )
                KTokenFailure ( t, klogErr, rc, "limit constraint" );
            else
                rc = expect ( tbl, src, t, eSemiColon, ";", true );
        }
        return rc;
    }

    /* create column object */
    c = malloc ( sizeof * c );
    if ( c == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    memset ( c, 0, sizeof * c );
    c -> dflt = dflt;
    c -> read_only = read_only;

    /* parse decl */
    rc = column_decl ( tbl, src, t, env, self, table, c );
    if ( rc == 0 )
    {
        /* give column its own id within table */
        rc = VectorAppend ( & table -> col, & c -> cid . id, c );
        if ( rc != 0 )
            KTokenRCExplain ( t, klogInt, rc );
        else
        {
            void *ignore;
            SNameOverload *name = ( void* ) c -> name -> u . obj;
            if ( name == NULL )
            {
                rc = SNameOverloadMake ( & name, c -> name, 0, 4 );
                if ( rc == 0 )
                {
                    name -> cid . ctx = -1;
                    rc = VectorAppend ( & table -> cname, & name -> cid . id, name );
                    if ( rc != 0 )
                        SNameOverloadWhack ( name, NULL );
                }
            }

            if ( rc == 0 )
            {
                rc = VectorInsertUnique ( & name -> items, c, NULL, SColumnSort );
                if ( rc == 0 )
                    return 0;
            }

            /* reverse column insertion */
            VectorSwap ( & table -> col, c -> cid . id, NULL, & ignore );

            /* what went wrong */
            KTokenRCExplain ( t, klogInt, rc );
        }
    }

    SColumnWhack ( c, NULL );
    return rc;
}
#endif

#if SLVL >= 6

static
rc_t untyped_tbl_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    const SNameOverload *name;

    /* expecting a function assignment */
    rc_t rc = expect ( tbl, src,
        next_token ( tbl, src, t ), eAssign, "=", true );
    if ( rc == 0 )
        rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;
    if ( t -> id != eUntypedFunc )
        return KTokenExpected ( t, klogErr, "untyped function expression" );
    name = t -> sym -> u . obj;

    rc = expect ( tbl, src,
        next_token ( tbl, src, t ), eLeftParen, "(", true );
    if ( rc == 0 )
        rc = expect ( tbl, src, t, eRightParen, ")", true );
    if ( rc == 0 )
        rc = expect ( tbl, src, t, eSemiColon, ";", true );

    if ( rc == 0 )
    {
        table -> untyped = VectorLast ( & name -> items );
        if ( table -> untyped == NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcFunction, rcNotFound );
            KTokenRCExplain ( t, klogErr, rc );
        }
    }

    return rc;
}

#if SLVL >= 8
static
rc_t default_view_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    rc_t rc;
    bool string_too_long;

    if ( next_token ( tbl, src, t ) -> id != eString )
        return KTokenExpected ( t, klogErr, "default view declaration" );

    string_too_long = false;
    if ( t -> str . size >= 236 + 2 )
    {
        KTokenExpected ( t, klogWarn, "default view declaration less than 236 characters" );
        string_too_long = true;
    }

    rc = expect ( tbl, src, t, eSemiColon, ";", true );
    if ( rc == 0 && ! string_too_long )
    {
        String decl = t -> str;
        ++ decl . addr;
        decl . size -= 2;
        decl . len -= 2;

        if ( table -> dflt_view != NULL )
            StringWhack ( table -> dflt_view );

        rc = StringCopy ( & table -> dflt_view, & decl );
    }

    return rc;
}
#endif

/*
 * table-local-decl   = [ 'virtual' ] 'column' <column-decl>
 *                    | 'physical' [ 'column' ] <physical-decl>
 *                    | <production_stmt>
 */
static
rc_t table_local_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    rc_t rc;

    switch ( t -> id )
    {
#if SLVL >= 8
    case kw_default:
        if ( env -> default_view_decl )
        {
            KToken t2;
            if ( next_token ( tbl, src, & t2 ) -> id == kw_view )
                return default_view_decl ( tbl, src, t, env, self, table );
             KTokenSourceReturn ( src, & t2 );
        }
        /* no break */
    case kw_extern:
    case kw_column:
    case kw_readonly:
        return column_declaration ( tbl, src, t, env, self, table );
#endif
#if SLVL >= 7
    case kw_static:
    case kw_physical:
        return physical_member ( tbl, src, t, env, self, table );
#endif
    case kw___untyped:
        return untyped_tbl_expr ( tbl, src, t, env, self, table );
#if 0
    case kw_index:
        break;
#endif
    case eSemiColon:
        next_token ( tbl, src, t );
        rc = 0;
        break;

    default:
        rc = production_stmt ( tbl, src, t, env, self, & table -> prod, eTable );
        if ( rc == 0 )
            rc = expect ( tbl, src, t, eSemiColon, ";", true );
    }

    return rc;
}

/*
 * table-body         = '{' [ <table-decl-list> ] '}'
 *
 * table-decl-list    = <tbl-local-decl> ';' [ <table-decl-list> ]
 */
typedef struct STableScanData STableScanData;
struct STableScanData
{
    STable *self;
    rc_t rc;
};

static
bool CC table_fwd_scan ( BSTNode *n, void *data )
{
    STableScanData *pb = data;
    KSymbol *sym = ( KSymbol* ) n;
    STable *self = pb -> self;

    /* process forwarded symbols */
    if ( sym -> type == eForward )
    {
        /* this symbol was introduced in THIS table */
        sym -> u . fwd . ctx = self -> id;

        /* add it to the introduced virtual productions and make it virtual */
        pb -> rc = VectorAppend ( & self -> vprods, & sym -> u . fwd . id, sym );
        if ( pb -> rc != 0 )
            return true;
        sym -> type = eVirtual;
    }
    /* symbols other than fwd or virtual are ignored */
    else if ( sym -> type != eVirtual )
    {
        return false;
    }

    /* add symbol to vector to track ownership */
    pb -> rc = VectorAppend ( & self -> syms, NULL, sym );
    if ( pb -> rc != 0 )
        return true;

    /* remove from symbol table */
    BSTreeUnlink ( & self -> scope, & sym -> n );
    return false;
}

static
rc_t table_body ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    rc_t rc = expect ( tbl, src, t, eLeftCurly, "{", true );
    if ( rc != 0 )
        return rc;

    while ( t -> id != eRightCurly )
    {
        rc = table_local_decl ( tbl, src, t, env, self, table );
        if ( rc != 0 )
            return rc;
    }

    rc = expect ( tbl, src, t, eRightCurly, "}", true );
    if ( rc == 0 )
    {
        STableScanData pb;
        pb . self = table;
        pb . rc = 0;

        /* scan table scope for unresolved forward references */
        if ( BSTreeDoUntil ( & table -> scope, false, table_fwd_scan, & pb ) )
            KTokenRCExplain ( t, klogInt, rc = pb . rc );
    }

    return rc;
}

/*
 * table-syntax
 */
static
bool CC table_prod_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    const SProduction *prod = ( const SProduction* ) item;

    if ( ! prod -> trigger )
        * rc = eval_expr_syntax ( prod -> fd );
    if ( * rc == 0 )
        * rc = eval_expr_syntax ( prod -> expr );

    return ( * rc != 0 ) ? true : false;
}

static
rc_t table_stmt_syntax ( const STable *table )
{
    rc_t rc = 0;
    VectorDoUntil ( & table -> prod, false, table_prod_syntax, & rc );
    return rc;
}

#if SLVL >= 8
static
bool CC table_typed_column_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    const SColumn *col = ( const SColumn* ) item;

    if ( col -> read != NULL )
        * rc = eval_expr_syntax ( col -> read );
    if ( * rc == 0 && col -> validate != NULL )
        * rc = eval_expr_syntax ( col -> validate );
    if ( * rc == 0 && col -> limit != NULL )
        * rc = eval_expr_syntax ( col -> limit );

    return ( * rc != 0 ) ? true : false;
}

static
rc_t table_column_syntax ( const STable *table )
{
    rc_t rc = 0;
    VectorDoUntil ( & table -> col, false, table_typed_column_syntax, & rc );
    return rc;
}
#endif

#if SLVL >= 7
static
bool CC table_physcol_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    const SPhysMember *col = ( const SPhysMember* ) item;

    if ( col -> expr == NULL )
        return false;

    * rc = eval_expr_syntax ( col -> expr );
    return ( * rc != 0 ) ? true : false;
}

static
rc_t table_physical_syntax ( const STable *table )
{
    rc_t rc = 0;
    VectorDoUntil ( & table -> phys, false, table_physcol_syntax, & rc );
    return rc;
}
#endif


/*
 * push-tbl-scope
 * pop-tbl-scope
 */
void pop_tbl_scope ( KSymTable *tbl, const STable *table )
{
    uint32_t i, count = VectorLength ( & table -> overrides );
    for ( ++ count, i = 0; i < count; ++ i )
        KSymTablePopScope ( tbl );
}

rc_t push_tbl_scope ( KSymTable *tbl, const STable *table )
{
    rc_t rc;
    uint32_t i = VectorStart ( & table -> overrides );
    uint32_t count = VectorLength ( & table -> overrides );
    for ( count += i; i < count; ++ i )
    {
        const STableOverrides *to = ( const void* ) VectorGet ( & table -> overrides, i );
        rc = KSymTablePushScope ( tbl, ( BSTree* ) & to -> dad -> scope );
        if ( rc != 0 )
        {
            for ( count = VectorStart ( & table -> overrides ); i > count; -- i )
                KSymTablePopScope ( tbl );
            return rc;
        }
    }

    rc = KSymTablePushScope ( tbl, ( BSTree* ) & table -> scope );
    if ( rc != 0 )
    {
        for ( i = VectorStart ( & table -> overrides ); i < count; ++ i )
            KSymTablePopScope ( tbl );
    }

    return rc;
}

/*
 * init-tbl-symtab
 *  initializes "tbl"
 *  places table in scope
 *  must be balanced by KSymTableWhack
 */
rc_t init_tbl_symtab ( KSymTable *tbl, const VSchema *schema, const STable *table )
{
    rc_t rc = init_symtab ( tbl, schema );
    if ( rc == 0 )
    {
        rc = push_tbl_scope ( tbl, table );
        if ( rc == 0 )
            return 0;

        KSymTableWhack ( tbl );
    }

    return rc;
}


/*
 * table-decl         = 'table' <fqn> '#' <maj-min-rel>
 *                      [ '=' <table-name> ] <table-body>
 * table-body         = '{' [ <table-stmts> ] '}'
 * table-stmts        = <table-stmt> ';' [ <table-stmts> ]
 */
static
rc_t table_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STable *table )
{
    /* table name */
    rc_t rc = create_fqn ( tbl, src, t, env, eTable, NULL );
    if ( rc != 0 && GetRCState ( rc ) != rcExists )
        return KTokenFailure ( t, klogErr, rc, "table name" );
    table -> name = t -> sym;
    table -> src_file = t -> txt -> path;
    table -> src_line = t -> lineno;

    /* table version */
    if ( next_token ( tbl, src, t ) -> id != eHash )
        return KTokenExpected ( t, klogErr, "#" );
    next_token ( tbl, src, t );
    rc = maj_min_rel ( tbl, src, t, env, self, & table -> version, true );
    if ( rc != 0 )
        return rc;

    /* prepare vectors */
    VectorInit ( & table -> parents, 0, 4 );
    VectorInit ( & table -> overrides, 0, 4 );
    VectorInit ( & table -> col, 0, 16 );
    VectorInit ( & table -> cname, 0, 16 );
    VectorInit ( & table -> phys, 0, 16 );
    VectorInit ( & table -> prod, 0, 64 );
    VectorInit ( & table -> vprods, 1, 16 );
    VectorInit ( & table -> syms, 1, 32 );

    /* look for inheritance */
    if ( t -> id == eAssign ) do
    {
        const STable *dad;
        const SNameOverload *name;

        /* look for dad */
        rc = next_fqn ( tbl, src, next_token ( tbl, src, t ), env );
        if ( rc != 0 )
            return KTokenFailure ( t, klogErr, rc, "table name" );

        /* insist that dad be a table */
        if ( t -> id != eTable )
            return KTokenExpected ( t, klogErr, "table name" );
        name = t -> sym -> u . obj;

        /* check for version */
        if ( next_token ( tbl, src, t ) -> id != eHash )
            dad = VectorLast ( & name -> items );
        else
        {
            uint32_t vers;
            next_token ( tbl, src, t );
            rc = maj_min_rel ( tbl, src, t, env, self, & vers, true );
            if ( rc != 0 )
                return rc;
            dad = VectorFind ( & name -> items, & vers, NULL, STableCmp );
#if _DEBUGGING && 1
            if ( dad != NULL && dad -> version > vers )
            {
                PLOGMSG ( klogInfo, ( klogInfo
                                      , "table_decl: table '$(tbl)' specifies parent table '$(parent)' - should be $(actual_version)"
                                      , "tbl=%N#%.3V,parent=%N#%.3V,actual_version=#%.3V"
                                      , table -> name, table -> version
                                      , dad -> name, vers
                                      , dad -> version
                              ));
            }
#endif
        }

        /* dad should be found */
        if ( dad == NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcTable, rcNotFound );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        /* take the inheritance */
        rc = STableExtend ( tbl, table, dad );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
    }
    while ( t -> id == eComma );

    /* enter table into scope */
    rc = push_tbl_scope ( tbl, table );
    if ( rc == 0 )
    {
        /* scan override tables for virtual symbols */
        if ( VectorDoUntil ( & table -> overrides, false, STableScanVirtuals, tbl ) )
            rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

        /* parse the definition */
        else
            rc = table_body ( tbl, src, t, env, self, table );

        /* pop out of scope */
        pop_tbl_scope ( tbl, table );
    }

    /* fix forward references */
    if ( rc == 0 )
    {
        rc = table_stmt_syntax ( table );
#if SLVL >= 8
        if ( rc == 0 )
            rc = table_column_syntax ( table );
#endif
#if SLVL >= 7
        if ( rc == 0 )
            rc = table_physical_syntax ( table );
#endif
    }

    return rc;
}

static
void CC column_set_context ( void *item, void *data )
{
    SColumn *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC name_set_context ( void *item, void *data )
{
    SNameOverload *self = item;
    if ( ( int32_t ) self -> cid . ctx < 0 )
        self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC physical_set_context ( void *item, void *data )
{
    SPhysMember *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC production_set_context ( void *item, void *data )
{
    SProduction *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC symbol_set_context ( void *item, void *data )
{
    KSymbol *self = item;
    self -> u . fwd . ctx = * ( const uint32_t* ) data;
}

static
void table_set_context ( STable *self )
{
    VectorForEach ( & self -> col, false, column_set_context, & self -> id );
    VectorForEach ( & self -> cname, false, name_set_context, & self -> id );
    VectorForEach ( & self -> phys, false, physical_set_context, & self -> id );
    VectorForEach ( & self -> prod, false, production_set_context, & self -> id );
    VectorForEach ( & self -> vprods, false, symbol_set_context, & self -> id );
}

#if NO_UPDATE_TBL_REF || 0
#define schema_update_tbl_ref( self, exist, table ) \
    0
#else
typedef struct update_tbl_ref_data update_tbl_ref_data;
struct update_tbl_ref_data
{
    const STable *exist, *table;
    rc_t rc;
};

static
bool CC table_update_tbl_ref ( void *item, void *data )
{
    STable *self = item;
    update_tbl_ref_data *pb = data;

    bool is_ancestor;

    /* check for having this guy as an immediate parent */
    uint32_t i = VectorStart ( & self -> parents );
    uint32_t count = VectorLength ( & self -> parents );
    for ( count += i; i < count; ++ i )
    {
        STable *dad = VectorGet ( & self -> parents, i );
        if ( ( const STable* ) dad == pb -> exist )
        {
            void *ignore;
            VectorSwap ( & self -> parents, i, pb -> table, & ignore );
            PARSE_DEBUG (( "table_update_tbl_ref: replaced parent '%N#%.3V' with version #%.3V in table '%N#%.3V'.\n"
                           , dad -> name, dad -> version
                           , pb -> table -> version
                           , self -> name, self -> version
                ));
            break;
        }
    }

    /* check for having him in the ancestry somewhere */
    i = VectorStart ( & self -> overrides );
    count = VectorLength ( & self -> overrides );
    for ( is_ancestor = false, count += i; i < count; ++ i )
    {
        STableOverrides *to = VectorGet ( & self -> overrides, i );
        if ( to -> dad == pb -> exist )
        {
            is_ancestor = true;

            /* rewrite overrides to have updated parent */
            VectorWhack ( & to -> overrides, NULL, NULL );
            pb -> rc = VectorCopy ( & pb -> table -> vprods, & to -> overrides );
            if ( pb -> rc != 0 )
                return true;
            to -> dad = pb -> table;
            to -> ctx = pb -> table -> id;
            VectorReorder ( & self -> overrides, STableOverridesKSort, NULL );
            PARSE_DEBUG (( "table_update_tbl_ref: replaced ancestor '%N#%.3V' with version #%.3V in table '%N#%.3V'.\n"
                           , pb -> exist -> name, pb -> exist -> version
                           , pb -> table -> version
                           , self -> name, self -> version
                ));
        }
    }

    /* if he's not an ancestor, we're done */
    if ( ! is_ancestor )
        return false;

    /* remove columns from old parent */
    i = VectorStart ( & self -> cname );
    count = VectorLength ( & self -> cname );
    for ( count += i; i < count; ++ i )
    {
        Vector cv;
        uint32_t cx, cnt;
        SNameOverload *name = VectorGet ( & self -> cname, i );

        /* names originating in existing parent get mapped */
        if ( name -> cid . ctx == pb -> exist -> id )
        {
            name -> cid . ctx = pb -> table -> id;
            PARSE_DEBUG (( "table_update_tbl_ref: updated context of column name '%N' from %u to %u.\n"
                           , name -> name
                           , pb -> exist -> id
                           , pb -> table -> id
                ));
        }

        /* now copy over columns, but eliminate all from old parent */
        cx = VectorStart ( & name -> items );
        cnt = VectorLength ( & name -> items );
        VectorInit ( & cv, cx, cnt );
        for ( cnt += cx; cx < cnt; ++ cx )
        {
            SColumn *c = VectorGet ( & name -> items, cx );
            if ( c -> cid . ctx == pb -> exist -> id )
            {
                PARSE_DEBUG (( "table_update_tbl_ref: dropping column index %u from name '%N'.\n"
                               , cx
                               , name -> name
                    ));
            }
            else
            {
                pb -> rc = VectorAppend ( & cv, NULL, c );
                if ( pb -> rc != 0 )
                    return true;
            }
        }
        VectorWhack ( & name -> items, NULL, NULL );
        name -> items = cv;
    }

    /* add in all columns from new parent */
    if ( VectorDoUntil ( & pb -> table -> cname, false, STableCopyColumnNames, self ) )
    {
        pb -> rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return true;
    }

    return false;
}

static
bool CC tblmbr_update_tbl_ref ( void *item, void *data )
{
    STblMember *self = item;
    update_tbl_ref_data *pb = data;

    if ( self -> tbl == pb -> exist )
    {
        PARSE_DEBUG (( "tblmbr_update_tbl_ref: updated table member '%N %N' from %V to %V.\n"
                       , pb -> exist -> name
                       , self -> name
                       , pb -> exist -> version
                       , pb -> table -> version
                ));
        self -> tbl = pb -> table;
    }

    return false;
}

static
bool CC db_update_tbl_ref ( void *item, void *data )
{
    SDatabase *self = item;

    /* update table members */
    if ( VectorDoUntil ( & self -> tbl, false, tblmbr_update_tbl_ref, data ) )
        return true;

    return VectorDoUntil ( & self -> db, false, db_update_tbl_ref, data );
}

static
rc_t schema_update_tbl_ref ( VSchema *self, const STable *exist, const STable *table )
{
    update_tbl_ref_data pb;
    pb . exist = exist;
    pb . table = table;
    pb . rc = 0;

    if ( ! VectorDoUntil ( & self -> tbl, false, table_update_tbl_ref, & pb ) )
          VectorDoUntil ( & self -> db, false, db_update_tbl_ref, & pb );

    return pb . rc;
}
#endif

rc_t table_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    rc_t rc;
    void *ignore;

    STable *table = calloc ( 1, sizeof * table );
    if ( table == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    rc = table_decl ( tbl, src, t, env, self, table );
    if ( rc == 0 )
    {
        SNameOverload *name = ( void* ) table -> name -> u . obj;
        if ( name == NULL )
        {
            rc = SNameOverloadMake ( & name, table -> name, 0, 4 );
            if ( rc == 0 )
            {
                rc = VectorAppend ( & self -> tname, & name -> cid . id, name );
                if ( rc != 0 )
                    SNameOverloadWhack ( name, NULL );
            }
        }

        if ( rc == 0 )
        {
            rc = VectorAppend ( & self -> tbl, & table -> id, table );
            if ( rc == 0 )
            {
                uint32_t idx;

                /* set the table id on all members */
                table_set_context ( table );

                /* add to named table overrides */
                rc = VectorInsertUnique ( & name -> items, table, & idx, STableSort );
                if ( rc == 0 )
                    return 0;

                if ( GetRCState ( rc ) == rcExists )
                {
                    const STable *newer;
                    STable *exist = VectorGet ( & name -> items, idx );
                    rc = STableCompare ( exist, table, & newer, false );
                    if ( rc == 0 && newer == table )
                    {
                        /* put the new one in place of the existing */
                        VectorSwap ( & name -> items, idx, table, & ignore );

                        /* tell everyone to use new table */
                        return schema_update_tbl_ref ( self, exist, table );
                    }
                }

                VectorSwap ( & self -> tbl, table -> id, NULL, & ignore );
            }
        }
    }
    else if ( GetRCState ( rc ) == rcExists )
    {
        rc = 0;
    }

    STableWhack ( table, NULL );

    return rc;
}

#endif
