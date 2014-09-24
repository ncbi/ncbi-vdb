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

#include <klib/extern.h>
#include <klib/symtab.h>
#include <klib/symbol.h>
#include <klib/token.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KSymbol
 *  a name to object mapping
 */


/* Init
 *  initialize a symbol without allocating space
 *
 *  "self" [ IN ] - where to initialize
 *
 *  "name" [ IN ] - symbol name
 *
 *  "type" [ IN ] - symbol type
 *
 *  "obj" [ IN, NULL OKAY ] - optional object mapping
 *
 */
KLIB_EXTERN rc_t CC KSymbolInit ( KSymbol * self, 
    const String * name, uint32_t type, const void * obj)
{
    rc_t rc;

    if ( name == NULL )
        rc = RC ( rcText, rcString, rcConstructing, rcName, rcNull );
    else if ( name -> len == 0 )
        rc = RC ( rcText, rcString, rcConstructing, rcName, rcEmpty );
    else
    {
        string_copy ( ( char* ) ( self + 1 ), name -> size + 1, name -> addr, name -> size );
        StringInit ( & self -> name, ( char* ) ( self + 1 ), name -> size, name -> len );
        self -> u . obj = obj;
        self -> dad = NULL;
        self -> type = type;

        return 0;
    }
    return rc;
}

/* Make
 *  create a symbol
 *
 *  "sym" [ OUT ] - return parameter for symbol
 *
 *  "name" [ IN ] - symbol name
 *
 *  "type" [ IN ] - symbol type
 *
 *  "obj" [ IN, NULL OKAY ] - optional object mapping
 */
LIB_EXPORT rc_t CC KSymbolMake ( KSymbol **symp,
    const String *name, uint32_t type, const void *obj )
{
    rc_t rc;
    if ( symp == NULL )
        rc = RC ( rcText, rcString, rcConstructing, rcParam, rcNull );
    else
    {
        KSymbol *sym = malloc ( sizeof *sym + name -> size + 1 );
        if ( sym == NULL )
            rc = RC ( rcText, rcString, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KSymbolInit (sym, name, type, obj);
            if (rc == 0)
            {
                * symp = sym;
                return 0;
            }
            free (sym);
        }
        * symp = NULL;
    }
    return rc;
}

/* Whack
 */
LIB_EXPORT void CC KSymbolWhack ( BSTNode *n, void *ignore )
{
    KSymbol *self = ( KSymbol* ) n;

    if ( self -> type == eNamespace )
        BSTreeWhack ( & self -> u . scope, KSymbolWhack, ignore );

    free ( self );
}

/* Cmp
 */
LIB_EXPORT int CC KSymbolCmp ( const void *item, const BSTNode *n )
{
    const String *a = item;
    const KSymbol *b = ( const KSymbol* ) n;

    return StringOrderNoNullCheck ( a, & b -> name );
}

/* Sort
 */
LIB_EXPORT int CC KSymbolSort ( const BSTNode *item, const BSTNode *n )
{
    const KSymbol *a = ( const KSymbol* ) item;
    const KSymbol *b = ( const KSymbol* ) n;

    return StringOrderNoNullCheck ( & a -> name, & b -> name );
}



/*--------------------------------------------------------------------------
 * KSymTable
 *  scoped stack of BSTree
 */


/* Init
 *  create an empty symbol table
 *
 *  "intrinsic" [ IN ] - intrinsic scope
 */
LIB_EXPORT rc_t CC KSymTableInit ( KSymTable *self, const BSTree *intrinsic )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcText, rcTree, rcConstructing, rcSelf, rcNull );
    else
    {
        self -> ns = NULL;
        VectorInit ( & self -> stack, 0, 16 );

        if ( intrinsic == NULL )
        {
            self -> intrinsic = 0;
            return 0;
        }

        self -> intrinsic = 1;
        rc = VectorAppend ( & self -> stack, NULL, intrinsic );
    }

    return rc;
}

/* Whack
 *  must be called to clean up stack
 */
#ifndef KSymTableWhack
LIB_EXPORT void CC KSymTableWhack ( KSymTable *self )
{
    if ( self != NULL )
        VectorWhack ( & self -> stack, NULL, NULL );
}
#endif


/* PushScope
 *  pushes a tree onto stack
 *  fails if count == depth
 *  otherwise, pushes scope into top of stack
 *
 *  "scope" [ IN ] - current top scope
 */
LIB_EXPORT rc_t CC KSymTablePushScope ( const KSymTable *cself, BSTree *scope )
{
    KSymTable *self = ( KSymTable* ) cself;
    assert ( self != NULL );
    assert ( scope != NULL );
    return VectorAppend ( & self -> stack, NULL, scope );
}


/* PopScope
 *  removes a tree from stack
 *  noop if count < self->intrinsic,
 *  because stack bottom will be intrinsic scope
 */
LIB_EXPORT void CC KSymTablePopScope ( const KSymTable *cself )
{
    if ( cself != NULL && cself -> stack . len > cself -> intrinsic )
        -- ( ( KSymTable* ) cself ) -> stack . len;
}


/* PushNamespace
 *  pushes a namespace scope onto stack
 */
LIB_EXPORT rc_t CC KSymTablePushNamespace ( const KSymTable *cself, KSymbol *ns )
{
    rc_t rc;
    KSymTable *self = ( KSymTable* ) cself;
    assert ( ns != NULL && ns -> type == eNamespace );
    rc = KSymTablePushScope ( self, & ns -> u . scope );
    if ( rc == 0 )
        self -> ns = ns;
    return rc;
}


/* PopNamespace
 */
LIB_EXPORT void CC KSymTablePopNamespace ( const KSymTable *cself )
{
    KSymTable *self = ( KSymTable* ) cself;
    assert ( self != NULL );
    if ( self -> ns != NULL )
    {
        KSymTablePopScope ( self );
        self -> ns = self -> ns -> dad;
    }
}


/* CreateNamespace
 *  given a name, make it into a namespace,
 *
 *  "name" [ IN ] - name of namespace. if being created within
 *  another namespace, it will be linked to the parent.
 */
LIB_EXPORT rc_t CC KSymTableCreateNamespace ( KSymTable *self, KSymbol **nsp, const String *name )
{
    rc_t rc;

    assert ( self != NULL );
    assert ( nsp != NULL );
    assert ( name != NULL );

    if ( name -> len == 0 )
        rc = RC ( rcText, rcTree, rcInserting, rcString, rcEmpty );
    else if ( VectorLength ( & self -> stack ) <= self -> intrinsic )
        rc = RC ( rcText, rcTree, rcInserting, rcTree, rcReadonly );
    else
    {
        uint32_t len = VectorLength ( & self -> stack );
        BSTree *scope = VectorGet ( & self -> stack, len - 1 );
        KSymbol *ns = ( KSymbol* ) BSTreeFind ( scope, name, KSymbolCmp );
        if ( ns != NULL )
        {
            if ( ns -> type == eNamespace )
            {
                * nsp = ns;
                return 0;
            }

            rc = RC ( rcText, rcTree, rcInserting, rcName, rcExists );
        }
        else
        {
            ns = malloc ( sizeof *ns + name -> size + 1 );
            if ( ns == NULL )
                rc = RC ( rcText, rcTree, rcInserting, rcMemory, rcExhausted );
            else
            {
                /* initialize namespace */
                string_copy ( ( char* ) ( ns + 1 ), name -> size + 1, name -> addr, name -> size );
                StringInit ( & ns -> name, ( char* ) ( ns + 1 ), name -> size, name -> len );
                BSTreeInit ( & ns -> u . scope );
                ns -> dad = self -> ns;
                ns -> type = eNamespace;

                /* now insert into parent scope */
                assert ( scope != NULL );
                BSTreeInsert ( scope, & ns -> n, KSymbolSort );

                /* done */
                * nsp = ns;
                return 0;
            }
        }
    }

    * nsp = NULL;

    return rc;
}


/* CreateSymbol
 *  given a name, create an object reference
 *
 *  "symp" [ OUT, NULL OKAY ] - optional return parameter for
 *  newly created symbol, which is entered into the top scope
 *  and only returned for convenience.
 *
 *  "name" [ IN ] - symbol name. if being created within a
 *  namespace, the symbol will be linked to the parent.
 *
 *  "id" [ IN ] - if the symbol type
 *
 *  "obj" [ IN, NULL OKAY ] - if the object has been created
 *  at the point of symbol definition, it may be provided.
 */
LIB_EXPORT rc_t CC KSymTableCreateSymbol ( KSymTable *self, KSymbol **symp,
    const String *name, uint32_t id, const void *obj )
{
    rc_t rc;
    KSymbol *sym = NULL;

    if ( self == NULL )
        rc = RC ( rcText, rcTree, rcInserting, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcText, rcTree, rcInserting, rcString, rcNull );
    else if ( name -> len == 0 )
        rc = RC ( rcText, rcTree, rcInserting, rcString, rcEmpty );
    else if ( VectorLength ( & self -> stack ) <= self -> intrinsic )
        rc = RC ( rcText, rcTree, rcInserting, rcTree, rcReadonly );
    else
    {
        sym = malloc ( sizeof *sym + name -> size + 1 );
        if ( sym == NULL )
            rc = RC ( rcText, rcTree, rcInserting, rcMemory, rcExhausted );
        else
        {
            KSymbol *exists;
            uint32_t len = VectorLength ( & self -> stack );
            BSTree *scope = VectorGet ( & self -> stack, len - 1 );

            /* initialize symbol */
            string_copy ( ( char* ) ( sym + 1 ), name -> size + 1, name -> addr, name -> size );
            StringInit ( & sym -> name, ( char* ) ( sym + 1 ), name -> size, name -> len );
            sym -> u . obj = obj;
            sym -> dad = self -> ns;
            sym -> type = id;

            assert ( scope != NULL );
            rc = BSTreeInsertUnique ( scope, & sym -> n, ( BSTNode** ) & exists, KSymbolSort );
            if ( rc != 0 )
            {
                free ( sym );
                sym = NULL;
            }
        }
    }

    if ( symp != NULL )
        * symp = sym;

    return rc;
}


/* DupSymbol
 *  given a symbol, create a duplicate
 *
 *  "dup" [ OUT, NULL OKAY ] - optional return parameter for
 *  newly created symbol, which is entered into the top scope
 *  and only returned for convenience.
 *
 *  "sym" [ IN ] - symbol to copy.
 *
 *  "id" [ IN ] - if the symbol type
 *
 *  "obj" [ IN, NULL OKAY ] - if the object has been created
 *  at the point of symbol definition, it may be provided.
 */
static
rc_t CC KSymTableDupNamespaces ( KSymTable *self, const KSymbol *ns )
{
    rc_t rc;

    if ( ns == NULL )
        return 0;

    rc = KSymTableDupNamespaces ( self, ns -> dad );
    if ( rc == 0 )
    {
        KSymbol *dup;
        rc = KSymTableCreateNamespace ( self, & dup, & ns -> name );
        if ( rc == 0 )
            rc = KSymTablePushNamespace ( self, dup );
    }

    return rc;
}

static
void CC KSymTablePopDupNamespaces ( KSymTable *self, const KSymbol *ns )
{
    for ( ; ns != NULL; ns = ns -> dad )
        KSymTablePopNamespace ( self );
}

LIB_EXPORT rc_t CC KSymTableDupSymbol ( KSymTable *self, struct KSymbol **dupp,
    const KSymbol *sym, uint32_t id, const void *obj )
{
    rc_t rc;
    KSymbol *dup = NULL;

    if ( self == NULL )
        rc = RC ( rcText, rcTree, rcInserting, rcSelf, rcNull );
    else if ( sym == NULL )
        rc = RC ( rcText, rcTree, rcInserting, rcString, rcNull );
    else if ( VectorLength ( & self -> stack ) <= self -> intrinsic )
        rc = RC ( rcText, rcTree, rcInserting, rcTree, rcReadonly );
    else
    {
        KSymbol *old = self -> ns;
        rc = KSymTableDupNamespaces ( self, sym -> dad );
        if ( rc == 0 )
        {
            rc = KSymTableCreateSymbol ( self, & dup, & sym -> name, id, obj );
            KSymTablePopDupNamespaces ( self, dup -> dad );
        }
        self -> ns = old;
    }

    if ( dupp != NULL )
        * dupp = dup;

    return rc;
}


/* RemoveSymbol
 *  removes symbol from table
 *
 *  "sym" [ IN ] - symbol to be removed
 */
LIB_EXPORT rc_t CC KSymTableRemoveSymbol ( KSymTable *self, const KSymbol *sym )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcText, rcTree, rcRemoving, rcSelf, rcNull );
    else if ( sym == NULL )
        rc = RC ( rcText, rcTree, rcRemoving, rcParam, rcNull );
    else
    {
        uint32_t i;
        BSTree *scope;

        /* if the symbol lives within a namespace,
           it won't be found directly. */
        if ( sym -> dad != NULL )
        {
            KSymbol *ns = sym -> dad;
            if ( ! BSTreeUnlink ( & ns -> u . scope, ( BSTNode* ) & sym -> n ) )
                return RC ( rcText, rcTree, rcRemoving, rcNode, rcCorrupt );

            /* if the namespace is not now empty, we're done */
            if ( ns -> u . scope . root != NULL )
                return 0;

            /* remove and delete the namespace */
            rc = KSymTableRemoveSymbol ( self, ns );
            if ( rc == 0 )
                KSymbolWhack ( & ns -> n, NULL );
            return rc;
        }

        /* walk the stack trying to find owning scope */
        for ( i = VectorLength ( & self -> stack ); i > self -> intrinsic; )
        {
            scope = VectorGet ( & self -> stack, -- i );
            if ( BSTreeUnlink ( scope, ( BSTNode* ) & sym -> n ) )
                return 0;
        }

        /* can't remove it from intrinsic table,
           so see if it wasn't found or if it's intrinsic */
        scope = VectorGet ( & self -> stack, 0 );
        if ( scope != NULL && BSTreeFind ( scope, & sym -> name, KSymbolCmp ) != NULL )
            rc = RC ( rcText, rcTree, rcRemoving, rcTree, rcReadonly );
        else
            rc = RC ( rcText, rcTree, rcRemoving, rcNode, rcNotFound );
    }

    return rc;
}


/* Find
 *  finds a symbol within the scope stack
 */
LIB_EXPORT KSymbol * CC KSymTableFind ( const KSymTable *self, const String *name )
{
    if ( self != NULL && name != NULL && name -> len != 0 )
    {
        uint32_t i;

        /* this is a pretty bad way to do things, but
           we're a fairly low frequency symbol table... */
        for ( i = VectorLength ( & self -> stack ); i > 0; )
        {
            BSTree *scope = VectorGet ( & self -> stack, -- i );
            KSymbol *sym = ( KSymbol* )
                BSTreeFind ( scope, name, KSymbolCmp );
            if ( sym != NULL )
                return sym;
        }
    }

    return NULL;
}

static
KSymbol * CC KSymTableDeepFindSymbol ( const KSymTable *self, const KSymbol *sym, uint32_t *level )
{
    uint32_t i;
    KSymbol *found;

    if ( sym -> dad != NULL )
    {
        found = KSymTableDeepFindSymbol ( self, sym -> dad, level );
        if ( found == NULL || found -> type != eNamespace )
            return NULL;
        return ( KSymbol* ) BSTreeFind ( & found -> u . scope,
            & sym -> name, KSymbolCmp );
    }

    for ( i = * level; i > 0; )
    {
        BSTree *scope = VectorGet ( & self -> stack, -- i );
        found = ( KSymbol* ) BSTreeFind ( scope, & sym -> name, KSymbolCmp );
        if ( found != NULL )
        {
            * level = i;
            return found;
        }
    }

    * level = 0;
    return NULL;
}

LIB_EXPORT KSymbol * CC KSymTableFindSymbol ( const KSymTable *self, const KSymbol *sym )
{
    if ( self != NULL && sym != NULL )
    {
        uint32_t i;

        /* the simple symbols are easy */
        if ( sym -> dad == NULL )
            return KSymTableFind ( self, & sym -> name );

        /* namespaces are harder...

           we're going to use recursion to find the root namespace
           ( although it's unlikely to go beyond a practical limit )
           and once the root name symbol is found, we need to step
           backwards through the symtab scopes until we find the root.

           the problem is that if that root doesn't produce a hit for
           the requested symbol, we still have to keep looking through
           other scopes
        */

        for ( i = VectorLength ( & self -> stack ); i > 0; )
        {
            KSymbol *found = KSymTableDeepFindSymbol ( self, sym, & i );
            if ( found != NULL )
                return found;
        }
    }

    return NULL;
}


/* FindIntrinsic
 *  find an intrinsic symbol
 */
LIB_EXPORT KSymbol * CC KSymTableFindIntrinsic ( const KSymTable *self, const String *name )
{
    KSymbol *sym = NULL;

    if ( self != NULL && name != NULL && name -> len != 0 )
    {
        uint32_t i;

        for ( i = self -> intrinsic; i > 0; )
        {
            BSTree *scope = VectorGet ( & self -> stack, -- i );
            if ( scope != NULL )
            {
                KSymbol *sym_ = ( KSymbol* )
                    BSTreeFind ( scope, name, KSymbolCmp );
                if ( sym_ != NULL )
                    return sym_;
            }
        }
    }

    return sym;
}


/* FindGlobal
 *  find a symbol at global scope
 */
LIB_EXPORT KSymbol * CC KSymTableFindGlobal ( const KSymTable *self, const String *name )
{
    KSymbol *sym = NULL;

    if ( self != NULL && name != NULL && name -> len != 0 )
    {
        uint32_t i;

        for ( i = self -> intrinsic + 1; i > 0; )
        {
            BSTree *scope = VectorGet ( & self -> stack, -- i );
            if ( scope != NULL )
            {
                KSymbol *sym_ = ( KSymbol* )
                    BSTreeFind ( scope, name, KSymbolCmp );
                if ( sym_ != NULL )
                    return sym_;
            }
        }
    }

    return sym;
}


/* FindShallow
 *  find a symbol in top scope
 */
LIB_EXPORT KSymbol * CC KSymTableFindShallow ( const KSymTable *self, const String *name )
{
    KSymbol *sym = NULL;

    if ( self != NULL && name != NULL && name -> len != 0 )
    {
        uint32_t len = VectorLength ( & self -> stack );
        if ( len > 0 )
        {
            BSTree *scope = VectorGet ( & self -> stack, len - 1 );
            if ( scope != NULL )
            {
                sym = ( KSymbol* )
                    BSTreeFind ( scope, name, KSymbolCmp );
            }
        }
    }

    return sym;
}


/* FindNext
 *  given a symbol that was found in nearest scope
 *  find next symbol of the same simple name in
 *  farther scopes
 */
LIB_EXPORT KSymbol * CC KSymTableFindNext ( const KSymTable *self, const KSymbol *sym, uint32_t *id )
{
    KSymbol *next = NULL;

    uint32_t i = 0;
    if ( id != NULL )
    {
        i = * id;
        * id = 0;
    }

    if ( self != NULL && sym != NULL )
    {
        BSTree *scope;
        uint32_t len = VectorLength ( & self -> stack );

        /* if the previous scope was not given */
        if ( i == 0 || i > len )
        {
            for ( i = len; i > 0; )
            {
                scope = VectorGet ( & self -> stack, -- i );
                next = ( KSymbol* ) BSTreeFind ( scope, & sym -> name, KSymbolCmp );
                if ( next == sym )
                {
                    ++ i;
                    break;
                }
            }

            /* bail if not found */
            if ( i == 0 )
                return NULL;
        }

        /* continue the search */
        for ( -- i ; i > 0; )
        {
            scope = VectorGet ( & self -> stack, -- i );
            next = ( KSymbol* ) BSTreeFind ( scope, & sym -> name, KSymbolCmp );
            if ( next != NULL )
            {
                if ( id != NULL )
                    * id = i + 1;
                break;
            }
        }
    }

    return next;
}
