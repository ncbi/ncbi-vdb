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

#ifndef _h_klib_symbol_
#define _h_klib_symbol_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KSymbol
 *  a name to object mapping
 */
typedef struct KSymbol KSymbol;
struct KSymbol
{
    /* currently kept in a BSTree
       this could change, though */
    BSTNode n;

    union
    {
        /* external object
           neither known, nor owned,
           i.e. just a borrowed reference */
        const void *obj;

        /* if object is a namespace, its scope */
        BSTree scope;

        /* unresolved forward decl id */
        struct { uint32_t ctx, id; } fwd;

    } u;

    /* enclosing namespace
       for name traceback */
    KSymbol *dad;

    /* symbol name */
    String name;

    /* symbol type */
    uint32_t type;
};


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
 *
 * This make will allocate storage for the KSymbol and
 * it's name.
 */
KLIB_EXTERN rc_t CC KSymbolMake ( KSymbol **sym,
    const String *name, uint32_t type, const void *obj );


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
    const String * name, uint32_t type, const void * obj);



/* Whack
 *  from BSTree
 */
KLIB_EXTERN void CC KSymbolWhack ( BSTNode *n, void *ignore );

/* there is currently no need for a real Destroy function 
 * but a macro to do nothing is included for orthogonality.
 */
#define KSymbolDestroy(s,i) ((void)0)

/* Cmp
 *  compare String* against KSymbol*
 */
KLIB_EXTERN int CC KSymbolCmp ( const void *item, const BSTNode *n );


/* Sort
 *  compare KSymbol* against KSymbol*
 */
KLIB_EXTERN int CC KSymbolSort ( const BSTNode *item, const BSTNode *n );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_symbol_ */
