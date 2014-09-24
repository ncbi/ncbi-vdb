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

#ifndef _h_klib_symtab_
#define _h_klib_symtab_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif



/*--------------------------------------------------------------------------
 * forwards
 */
struct BSTree;
struct String;
struct KSymbol;


/*--------------------------------------------------------------------------
 * KSymTable
 *  scoped stack of BSTrees
 */
typedef struct KSymTable KSymTable;
struct KSymTable
{
    /* head of open namespace stack */
    struct KSymbol *ns;

    /* scope stack */
    Vector stack;

    /* intrinsic scope count */
    uint32_t intrinsic;
};


/* Init
 *  create an empty symbol table
 *
 *  "intrinsic" [ IN, NULL OKAY ] - initial protected scope
 *  if not NULL, will be used as initial non-modifiable scope
 */
KLIB_EXTERN rc_t CC KSymTableInit ( KSymTable *self, struct BSTree const *intrinsic );


/* Whack
 *  must be called to clean up stack
 */
#if NOT_MACRO
KLIB_EXTERN void CC KSymTableWhack ( KSymTable *self );
#endif
#define KSymTableWhack( self ) \
    VectorWhack ( & ( self ) -> stack, NULL, NULL )


/* PushScope
 *  pushes a tree onto stack
 *
 *  "scope" [ IN ] - current top scope
 */
KLIB_EXTERN rc_t CC KSymTablePushScope ( const KSymTable *self, struct BSTree *scope );


/* PopScope
 *  removes a tree from stack
 *  noop if count <= self->intrinsic,
 *  because stack bottom will be intrinsic scope
 */
KLIB_EXTERN void CC KSymTablePopScope ( const KSymTable *self );


/* PushNamespace
 *  pushes a namespace scope onto stack
 */
KLIB_EXTERN rc_t CC KSymTablePushNamespace ( const KSymTable *self, struct KSymbol *ns );


/* PopNamespace
 */
KLIB_EXTERN void CC KSymTablePopNamespace ( const KSymTable *self );


/* CreateNamespace
 *  given a name, make it into a namespace,
 *
 *  "name" [ IN ] - name of namespace. if being created within
 *  another namespace, it will be linked to the parent.
 */
KLIB_EXTERN rc_t CC KSymTableCreateNamespace ( KSymTable *self,
    struct KSymbol **ns, struct String const *name );


/* CreateSymbol
 *  given a name, create an object reference
 *
 *  "sym" [ OUT, NULL OKAY ] - optional return parameter for
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
KLIB_EXTERN rc_t CC KSymTableCreateSymbol ( KSymTable *self, struct KSymbol **sym,
    struct String const *name, uint32_t id, const void *obj );
#define KSymTableCreateConstSymbol( self, sym, name, id, obj ) \
    KSymTableCreateSymbol ( self, ( struct KSymbol** ) ( sym ), name, id, obj )


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
KLIB_EXTERN rc_t CC KSymTableDupSymbol ( KSymTable *self, struct KSymbol **dup,
    struct KSymbol const *sym, uint32_t id, const void *obj );


/* RemoveSymbol
 *  removes symbol from table
 *
 *  "sym" [ IN ] - symbol to be removed
 */
KLIB_EXTERN rc_t CC KSymTableRemoveSymbol ( KSymTable *self, struct KSymbol const *sym );


/* Find
 *  finds a symbol within the scope stack
 */
KLIB_EXTERN struct KSymbol* CC KSymTableFind ( const KSymTable *self,
    struct String const *name );
KLIB_EXTERN struct KSymbol* CC KSymTableFindSymbol ( const KSymTable *self,
    struct KSymbol const *sym );


/* FindIntrinsic
 *  find an intrinsic symbol
 *  looks in stack scopes <= self->intrinsic
 */
KLIB_EXTERN struct KSymbol* CC KSymTableFindIntrinsic ( const KSymTable *self,
    struct String const *name );


/* FindGlobal
 *  find a symbol at global scope
 */
KLIB_EXTERN struct KSymbol* CC KSymTableFindGlobal ( const KSymTable *self,
    struct String const *name );


/* FindShallow
 *  find a symbol in top scope
 */
KLIB_EXTERN struct KSymbol* CC KSymTableFindShallow ( const KSymTable *self,
    struct String const *name );


/* FindNext
 *  given a symbol that was found in nearest scope
 *  find next symbol of the same simple name in
 *  farther scopes
 *
 *  "sym" [ IN ] - previously found symbol
 *
 *  "scope" [ IN/OUT, NULL OKAY ] - if NULL or value is 0,
 *  the scope id for "sym" is dynamically located. otherwise,
 *  the value on input is used for continuing a search. on
 *  output and if not NULL, the value is set to the scope id
 *  where the returned symbol was found, or 0 if not found.
 */
KLIB_EXTERN struct KSymbol* CC KSymTableFindNext ( const KSymTable *self,
    struct KSymbol const *sym, uint32_t *scope );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_symtab_ */
