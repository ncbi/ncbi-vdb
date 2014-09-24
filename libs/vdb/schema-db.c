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
#include "schema-dump.h"
#include "dbmgr-priv.h"

#include <klib/symbol.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * STblMember
 *  table member
 */

/* Mark
 */
static
void CC STblMemberMark ( void * item, void * data )
{
    const STblMember * self = item;
    if ( item != NULL )
        STableMark ( ( void * )self -> tbl, data );
}

/* Dump
 */
bool CC STblMemberDefDump ( void *item, void *dumper )
{
    SDumper *b = dumper;
    STblMember *self = item;

    b -> rc = SDumperPrint ( b, "\ttable " );
    if ( b -> rc == 0 )
        b -> rc = STableDump ( self -> tbl, b );
    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, " %N;\n", self -> name );

    return ( b -> rc != 0 ) ? true : false;
}

rc_t STblMemberDump ( const STblMember *self, SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}


/*--------------------------------------------------------------------------
 * SDBMember
 *  sub-db member
 */

/* Find
 */
SDBMember * CC SDatabaseFindDB ( const SDatabase *self, const char *name );

/* Mark
 */
static
void CC SDBMemberMark ( void * item, void * data )
{
    const SDBMember * self = item;
    if ( self != NULL )
        SDatabaseMark ( ( void * )self -> db, data );
}

/* Dump
 */
bool CC SDBMemberDefDump ( void *item, void *dumper )
{
    SDumper *b = dumper;
    SDBMember *self = item;

    b -> rc = SDumperPrint ( b, "\tdatabase " );
    if ( b -> rc == 0 )
        b -> rc = SDatabaseDump ( self -> db, b );
    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, " %N;\n", self -> name );

    return ( b -> rc != 0 ) ? true : false;
}

rc_t SDBMemberDump ( const SDBMember *self, SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}


/*--------------------------------------------------------------------------
 * SDatabase
 *  database declaration
 */

#if SLVL >= 6

/* Whack
 */
void CC SDatabaseWhack ( void *item, void *ignore )
{
    SDatabase *self = item;

    BSTreeWhack ( & self -> scope, KSymbolWhack, NULL );
    VectorWhack ( & self -> db, SDBMemberWhack, NULL );
    VectorWhack ( & self -> tbl, STblMemberWhack, NULL );

    free ( self );
}

/* Cmp
 * Sort
 */
int CC SDatabaseCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SDatabase *b = n;

    if ( * a > b -> version )
        return 1;
    return ( int ) ( * a >> 24 ) - ( int ) ( b -> version >> 24 );
}

int CC SDatabaseSort ( const void *item, const void *n )
{
    const SDatabase *a = item;
    const SDatabase *b = n;

    return ( int ) ( a -> version >> 24 ) - ( int ) ( b -> version >> 24 );
}

static
rc_t SDatabaseCompare ( const SDatabase *a, const SDatabase *b, const SDatabase **newer, bool exhaustive )
{
    assert ( ( a -> version >> 24 ) == ( b -> version >> 24 ) );

    * newer = a -> version >= b -> version ? a : b;

    /* TBD - more exhaustive comparison */

    return 0;
}

/* Find
 *  generic object find within database scope
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
 *  returns principal object identified. if NULL but "name" is not
 *  NULL, then the object was only partially identified.
 */
const void * SDatabaseFind ( const SDatabase *self,
    const VSchema *schema, const SNameOverload **name,
    uint32_t *type, const char *expr, const char *ctx )
{
    rc_t rc;
    KSymTable tbl;

    /* initialize to not-found */
    const void *obj = NULL;
    * name = NULL;
    * type = 0;

    /* build a symbol table for database */
    rc = init_db_symtab ( & tbl, schema, self );
    if ( rc == 0 )
    {
        obj = resolve_object ( & tbl, schema, NULL, name, type, expr, ctx, false );
        KSymTableWhack ( & tbl );
    }

    return obj;
}

/* Extend
 */
static
void CC VectorTranslate ( Vector *v, const Vector *dad )
{
    uint32_t block = VectorBlock ( v );
    uint32_t start = VectorStart ( dad ) + VectorLength ( dad );
    VectorInit ( v, start, block );
}

static
rc_t CC SDatabaseExtend ( SDatabase *self, const SDatabase *dad )
{
    self -> dad = dad;
    VectorTranslate ( & self -> db, & dad -> db );
    VectorTranslate ( & self -> tbl, & dad -> tbl );
    return 0;
}

/* Mark
 */
void CC SDatabaseClearMark ( void *item, void *ignore )
{
    SDatabase *self = item;
    self -> marked = false;
}


void CC SDatabaseMark ( void * item, void * data )
{
    SDatabase * self = item;
    if ( self != NULL && ! self -> marked )
    {
        self -> marked = true;
        VectorForEach ( & self -> db, false, SDBMemberMark, data );
        VectorForEach ( & self -> tbl, false, STblMemberMark, data );
        SDatabaseMark ( ( void * )self -> dad, data );
    }
}

void CC SDatabaseNameMark ( const SNameOverload *self, const VSchema *schema )
{
    if ( self != NULL )
    {
        VectorForEach ( & self -> items, false, SDatabaseMark, ( void* ) schema );
    }
}

/* Dump
 *  dump "database" { }
 */
rc_t SDatabaseDump ( const SDatabase *self, struct SDumper *d )
{
    d -> rc = FQNDump ( self != NULL ? self -> name : NULL, d );
    if ( d -> rc == 0 && self != NULL )
        d -> rc = SDumperVersion ( d, self -> version );
    return d -> rc;
}

bool CC SDatabaseDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SDatabase *self = ( const void* ) item;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    b -> rc = SDumperPrint ( b, "\tdatabase %N", self -> name );

    if ( b -> rc == 0 )
        b -> rc = SDumperVersion ( b, self -> version );

    /* TBD - need to overhaul inheritance of databases */
    if ( b -> rc == 0 && self -> dad != NULL )
        b -> rc = SDumperPrint ( b, "= %N", self -> dad -> name );

    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, "\n\t{\n" );

    SDumperIncIndentLevel ( b );

    if ( b -> rc == 0 )
        VectorDoUntil ( & self -> tbl, false, STblMemberDefDump, b );

    if ( b -> rc == 0 )
        VectorDoUntil ( & self -> db, false, SDBMemberDefDump, b );

    SDumperDecIndentLevel ( b );

    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, "\t}\n" );

    return ( b -> rc != 0 ) ? true : false;
}

#endif

/*--------------------------------------------------------------------------
 * VSchema
 */

#if SLVL >= 6

static
rc_t CC db_dbmbr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SDBMember *m )
{
    rc_t rc ;
    const SNameOverload *name;

    /* look for database typename */
    rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return KTokenFailure ( t, klogErr, rc, "database typename" );
    if ( t -> id != eDatabase )
        return KTokenExpected ( t, klogErr, "database typename" );
    name = t -> sym -> u . obj;

    /* look for version */
    if ( next_token ( tbl, src, t ) -> id != eHash )
        m -> db = VectorLast ( & name -> items );
    else
    {
        uint32_t vers;
        next_token ( tbl, src, t );
        rc = maj_min_rel ( tbl, src, t, env, self, & vers, true );
        if ( rc != 0 )
            return rc;
        m -> db = VectorFind ( & name -> items, & vers, NULL, SDatabaseCmp );
    }
    if ( m -> db == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcDatabase, rcNotFound );
        return KTokenRCExplain ( t, klogErr, rc );
    }

    /* get member name */
    if ( t -> sym != NULL )
    {
        KTokenSourceReturn ( src, t );
        next_shallow_token ( tbl, src, t, true );
    }

    /* looking for undefined identifier */
    if ( t -> id == eIdent )
    {
        rc = KSymTableCreateConstSymbol ( tbl, & m -> name, & t -> str, eDBMember, m );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
    }
    else if ( t -> id == eDBMember )
        return KTokenExpected ( t, klogErr, "undefined database member name" );
    else if ( t -> id != eForward && t -> id != eVirtual )
        return KTokenExpected ( t, klogErr, "database member name" );
    else
    {
        m -> name = t -> sym;
        ( ( KSymbol* ) t -> sym ) -> u . obj = m;
        ( ( KSymbol* ) t -> sym ) -> type = eDBMember;
    }

    /* expect we're done */
    return expect ( tbl, src, t, eSemiColon, ";", true );
}

static
rc_t CC database_dbmbr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SDatabase *db, bool tmpl )
{
    rc_t rc;
    SDBMember *m = malloc ( sizeof * m );
    if ( m == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    else
    {
        memset ( m, 0, sizeof * m );
        m -> tmpl = tmpl;

        rc = db_dbmbr ( tbl, src, t, env, self, m );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & db -> db, & m -> cid . id, m );
            if ( rc == 0 )
                return 0;
        }

        SDBMemberWhack ( m, NULL );
    }
    return rc;
}

static
rc_t CC db_tblmbr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, STblMember *m )
{
    rc_t rc ;
    const SNameOverload *name;

    /* look for table typename */
    rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return KTokenFailure ( t, klogErr, rc, "table typename" );
    if ( t -> id != eTable )
        return KTokenExpected ( t, klogErr, "table typename" );
    name = t -> sym -> u . obj;

    /* look for version */
    if ( next_token ( tbl, src, t ) -> id != eHash )
        m -> tbl = VectorLast ( & name -> items );
    else
    {
        uint32_t vers;
        next_token ( tbl, src, t );
        rc = maj_min_rel ( tbl, src, t, env, self, & vers, true );
        if ( rc != 0 )
            return rc;
        m -> tbl = VectorFind ( & name -> items, & vers, NULL, STableCmp );
    }
    if ( m -> tbl == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcTable, rcNotFound );
        return KTokenRCExplain ( t, klogErr, rc );
    }

    /* get member name */
    if ( t -> sym != NULL )
    {
        KTokenSourceReturn ( src, t );
        next_shallow_token ( tbl, src, t, true );
    }

    /* looking for undefined identifier */
    if ( t -> id == eIdent )
    {
        rc = KSymTableCreateConstSymbol ( tbl, & m -> name, & t -> str, eTblMember, m );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
    }
    else
    {
        if ( t -> id == eTblMember )
            return KTokenExpected ( t, klogErr, "undefined table member name" );
        if ( t -> id != eForward && t -> id != eVirtual )
            return KTokenExpected ( t, klogErr, "table member name" );

        m -> name = t -> sym;
        ( ( KSymbol* ) t -> sym ) -> u . obj = m;
        ( ( KSymbol* ) t -> sym ) -> type = eTblMember;
    }

    /* expect we're done */
    return expect ( tbl, src, next_token ( tbl, src, t ), eSemiColon, ";", true );
}

static
rc_t CC database_tblmbr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SDatabase *db, bool tmpl )
{
    rc_t rc;
    STblMember *m = malloc ( sizeof * m );
    if ( m == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    else
    {
        memset ( m, 0, sizeof * m );
        m -> tmpl = tmpl;

        rc = db_tblmbr ( tbl, src, t, env, self, m );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & db -> tbl, & m -> cid . id, m );
            if ( rc == 0 )
                return 0;
        }

        STblMemberWhack ( m, NULL );
    }
    return rc;
}

/*
 * database-body      = '{' [ <database-mbrs> ] '}'
 * database-mbrs      = [ 'template' ] <database-mbr> [ <database-mbrs> ]
 * database-mbr       = 'database' <db-mbr>
 *                    | 'table' <table-mbr>
 *                    | ';'
 */
static
rc_t CC database_mbr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SDatabase *db )
{
    bool tmpl = false;

    switch ( t -> id )
    {
    case  kw_template:
        tmpl = true;
        next_token ( tbl, src, t );
        break;
    case eSemiColon:
        next_token ( tbl, src, t );
        return 0;
    }

    switch ( t -> id )
    {
    case kw_database:
        return database_dbmbr ( tbl, src,
            next_token ( tbl, src, t ), env, self, db, tmpl );
    case kw_table:
        return database_tblmbr ( tbl, src,
            next_token ( tbl, src, t ), env, self, db, tmpl );
    }

    return KTokenExpected ( t, klogErr, "table or database" );
}

static
rc_t CC database_body ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SDatabase *db )
{
    rc_t rc = expect ( tbl, src, t, eLeftCurly, "{", true );
    if ( rc != 0 )
        return rc;

    while ( t -> id != eRightCurly )
    {
        rc = database_mbr ( tbl, src, t, env, self, db );
        if ( rc != 0 )
            return rc;
    }

    return expect ( tbl, src, t, eRightCurly, "}", true );
}


/*
 * push-db-scope
 * pop-db-scope
 */
rc_t push_db_scope ( KSymTable *tbl, const SDatabase *db )
{
    if ( db -> dad != NULL )
    {
        rc_t rc = push_db_scope ( tbl, db -> dad );
        if ( rc != 0 )
            return rc;
    }

    return KSymTablePushScope ( tbl, ( BSTree* ) & db -> scope );
}

void pop_db_scope ( KSymTable *tbl, const SDatabase *db )
{
    while ( db != NULL )
    {
        KSymTablePopScope ( tbl );
        db = db -> dad;
    }
}

/*
 * init-db-symtab
 *  initializes "tbl"
 *  places db in scope
 *  must be balanced by KSymTableWhack
 */
rc_t init_db_symtab ( KSymTable *tbl, const VSchema *schema, const SDatabase *db )
{
    rc_t rc = init_symtab ( tbl, schema );
    if ( rc == 0 )
    {
        rc = push_db_scope ( tbl, db );
        if ( rc == 0 )
            return 0;

        KSymTableWhack ( tbl );
    }

    return rc;
}


/*
 * database-decl      = 'database' <fqn> '#' <maj-min-rel>
 *                      [ '=' <database-name> ] <database-body>
 * database-body      = '{' [ <database-mbrs> ] '}'
 * database-mbrs      = <database-mbr> [ <database-mbrs> ]
 */
static
rc_t database_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, SDatabase *db )
{
    /* db name */
    rc_t rc = create_fqn ( tbl, src, t, env, eDatabase, NULL );
    if ( rc != 0 && GetRCState ( rc ) != rcExists )
        return KTokenFailure ( t, klogErr, rc, "database name" );
    db -> name = t -> sym;

    /* database version */
    if ( next_token ( tbl, src, t ) -> id != eHash )
        return KTokenExpected ( t, klogErr, "#" );
    next_token ( tbl, src, t );
    rc = maj_min_rel ( tbl, src, t, env, self, & db -> version, true );
    if ( rc != 0 )
        return rc;

    /* prepare vectors */
    VectorInit ( & db -> db, 0, 8 );
    VectorInit ( & db -> tbl, 0, 8 );

    /* look for inheritance */
    if ( t -> id == eAssign )
    {
        const SDatabase *dad;
        const SNameOverload *pname;

        /* look for dad */
        rc = next_fqn ( tbl, src, next_token ( tbl, src, t ), env );
        if ( rc != 0 )
            return KTokenFailure ( t, klogErr, rc, "database name" );

        /* insist that dad be a database */
        if ( t -> id != eDatabase )
            return KTokenExpected ( t, klogErr, "database name" );
        pname = t -> sym -> u . obj;

        /* check for version */
        if ( next_token ( tbl, src, t ) -> id != eHash )
            dad = VectorLast ( & pname -> items );
        else
        {
            uint32_t pvers;
            next_token ( tbl, src, t );
            rc = maj_min_rel ( tbl, src, t, env, self, & pvers, true );
            if ( rc != 0 )
                return rc;
            dad = VectorFind ( & pname -> items, & pvers, NULL, SDatabaseCmp );
        }

        /* dad should be found */
        if ( dad == NULL )
        {
            rc = RC ( rcVDB, rcSchema, rcParsing, rcDatabase, rcNotFound );
            return KTokenRCExplain ( t, klogErr, rc );
        }

        /* take the inheritance */
        rc = SDatabaseExtend ( db, dad );
        if ( rc != 0 )
            return KTokenRCExplain ( t, klogInt, rc );
    }

    /* enter database into scope */
    rc = push_db_scope ( tbl, db );
    if ( rc == 0 )
    {
        /* parse the definition */
        rc = database_body ( tbl, src, t, env, self, db );

        /* pop out of scope */
        pop_db_scope ( tbl, db );
    }

    return rc;
}

rc_t database_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    rc_t rc;
    void *ignore;

    SDatabase *db = malloc ( sizeof * db );
    if ( db == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    memset ( db, 0, sizeof * db );
    rc = database_decl ( tbl, src, t, env, self, db );
    if ( rc == 0 )
    {
        SNameOverload *name = ( void* ) db -> name -> u . obj;
        if ( name == NULL )
        {
            rc = SNameOverloadMake ( & name, db -> name, 0, 4 );
            if ( rc == 0 )
            {
                rc = VectorAppend ( & self -> dname, & name -> cid . id, name );
                if ( rc != 0 )
                    SNameOverloadWhack ( name, NULL );
            }
        }

        if ( rc == 0 )
        {
            rc = VectorAppend ( & self -> db, & db -> id, db );
            if ( rc == 0 )
            {
                uint32_t idx;
                rc = VectorInsertUnique ( & name -> items, db, & idx, SDatabaseSort );
                if ( rc == 0 )
                    return rc;

                if ( GetRCState ( rc ) == rcExists )
                {
                    const SDatabase *newer;
                    SDatabase *exist = VectorGet ( & name -> items, idx );
                    rc = SDatabaseCompare ( exist, db, & newer, false );
                    if ( rc == 0 && newer == db )
                    {
                        VectorSwap ( & name -> items, idx, db, & ignore );
                        /* TBD - need to update parent/child relationships */
                        return 0;
                    }
                }

                VectorSwap ( & self -> db, db -> id, NULL, & ignore );
            }
        }
    }
    else if ( GetRCState ( rc ) == rcExists )
    {
        rc = 0;
    }

    SDatabaseWhack ( db, NULL );

    return rc;
}

#endif
