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

#ifndef _h_linker_priv_
#define _h_linker_priv_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_token_
#include <klib/token.h>
#endif

#ifndef _h_vdb_xform_
#include <vdb/xform.h>
#endif

#ifndef _h_vdb_vdb_priv_
#include <vdb/vdb-priv.h>
#endif

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KSymbol;
struct KSymTable;
struct KNamelist;
struct KDyld;
struct KDlset;
struct KSymAddr;
struct SFunction;
struct SchemaEnv;
struct VTransDesc;


/*--------------------------------------------------------------------------
 * linker symbol types
 */
enum
{
    ltFactory = eNumSymtabIDs,
    ltUntyped
};


/*--------------------------------------------------------------------------
 * VLinker
 *  responsible for performing runtime schema resolution
 *  and C factory/function management
 *
 *  a schema will be declared with conditional productions and
 *  references to C functions. at load time, all schema expressions
 *  will be evaluated, unused conditional productions pruned,
 *  function references resolved, function objects created,
 *  and all paths rejected due to errors of any sort eliminated,
 *  leaving the final runtime schema.
 *
 *  the current implementation
 */
typedef struct VLinker VLinker;
struct VLinker
{
    /* dynamic loader */
    struct KDyld *dl;

    /* parent loader */
    const VLinker *dad;

    /* global scope for functions */
    BSTree scope;

    /* factory objects */
    Vector fact;

    /* special funcs */
    Vector special;

    KRefcount refcount;
};


/* Make
 *  creates an empty linker
 */
rc_t VLinkerMake ( VLinker **lp, const VLinker *dad, struct KDyld *dl );

/* MakeIntrinsic
 *  creates an initial, intrinsic linker
 *  pre-loaded with intrinsic factories
 */
rc_t VLinkerMakeIntrinsic ( VLinker **intrinsic );

/* InitFactories
 *  initialize intrinsic factories
 *
 *  "tbl" [ IN ] - linker-scope symbol table
 *
 *  "env" [ IN ] - default schema parser environment
 */
rc_t VLinkerInitFactories ( VLinker *self,
    struct KSymTable *tbl, struct SchemaEnv const *env );
rc_t VLinkerInitFactoriesRead ( VLinker *self,
    struct KSymTable *tbl, struct SchemaEnv const *env );

/* AddFactories
 *  add a list of intrinsic factories
 *
 *  "fact" [ IN ] and "count" [ IN ] - list of intrinsic factories to register
 *
 *  "tbl" [ IN ] - linker-scope symbol table
 *
 *  "env" [ IN ] - default schema parser environment
 */
typedef struct VLinkerIntFactory VLinkerIntFactory;
struct VLinkerIntFactory
{
    rc_t ( CC * f ) ( struct VTransDesc *desc );
    const char *name;
};

rc_t VLinkerAddFactories ( VLinker *self,
    const VLinkerIntFactory *fact, uint32_t count,
    struct KSymTable *tbl, struct SchemaEnv const *env );


/* Release
 */
rc_t VLinkerRelease ( const VLinker *self );

/* Attach
 * Sever
 */
VLinker *VLinkerAttach ( const VLinker *self );
rc_t VLinkerSever ( const VLinker *self );


/* AddLoadLibraryPath
 *  add a path[s] to loader for locating dynamic libraries
 */
rc_t VLinkerVAddLoadLibraryPath ( const VLinker *self,
    const char *path, va_list args );


/* Open
 *  opens libraries for search
 */
rc_t VLinkerOpen ( const VLinker *self, struct KDlset **libs );


/* Find
 *  find a named symbol
 *
 *  "libs" [ IN ] - set of open libraries to search
 *
 *  "desc" [ OUT ] - transform factory description
 *  "func" [ OUT ] - return parameter for func pointer
 *
 *  "proto" [ IN ] - function prototype from schema
 *  "min_version" [ IN ] - minimum version to accept
 *
 *  "external" [ OUT ] - true if function is not built-in
 */
rc_t VLinkerFindFactory ( VLinker *self, struct KDlset const *libs,
    VTransDesc *desc, struct SFunction const *proto, uint32_t min_version, bool *external );
rc_t VLinkerFindNamedFactory ( VLinker *self, struct KDlset const *libs,
    VTransDesc *desc, const char *fact_name );
rc_t VLinkerFindUntyped ( VLinker *self, struct KDlset const *libs,
    VUntypedTableTest *func, struct SFunction const *proto );


/* ListExternalSchemaModules
 */
rc_t VLinkerListExternalSchemaModules ( const VLinker *self,
    struct KNamelist **list );


/*--------------------------------------------------------------------------
 * LFactory
 *  describes an external C function factory
 */
typedef struct LFactory LFactory;
struct LFactory
{
    /* factory function address */
    struct KSymAddr const *addr;

    /* factory name */
    struct KSymbol const *name;

    /* factory description */
    VTransDesc desc;

    /* factory id */
    uint32_t id;

    /* externally supplied factory */
    bool external;
};

/* Whack
 */
void CC LFactoryWhack ( void *item, void *ignore );


/*--------------------------------------------------------------------------
 * LSpecial
 *  describes an external C table recognition function
 *  or a row-length function
 */
typedef struct LSpecial LSpecial;
struct LSpecial
{
    /* function address */
    struct KSymAddr const *addr;

    /* func name */
    struct KSymbol const *name;

    /* for built-in address */
    VUntypedFunc func;

    /* func id */
    uint32_t id;
};

/* Whack
 */
void CC LSpecialWhack ( void *item, void *ignore );

#ifdef __cplusplus
}
#endif

#endif /* _h_linker_priv_ */
