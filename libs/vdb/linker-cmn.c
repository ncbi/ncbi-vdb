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

#include "linker-priv.h"
#include "schema-priv.h"
#include "dbmgr-priv.h"

#include <kfs/directory.h>
#include <kfs/dyload.h>
#include <kfs/kfs-priv.h>
#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * LFactory
 *  describes an external C function factory
 */

/* Whack
 */
void CC LFactoryWhack ( void *item, void *ignore )
{
    LFactory *self = item;

    /* whack the guy's factory object */
    if ( self -> desc . whack != NULL )
        ( * self -> desc . whack ) ( self -> desc . fself );

    /* douse the dynamic library */
    KSymAddrRelease ( self -> addr );

    /* gone */
    free ( self );
}


/*--------------------------------------------------------------------------
 * LSpecial
 *  describes an external C table recognition function
 */

/* Whack
 */
void CC LSpecialWhack ( void *item, void *ignore )
{
    LSpecial *self = item;
    KSymAddrRelease ( self -> addr );
    free ( self );
}


/*--------------------------------------------------------------------------
 * VLinker
 */

/* Whack
 */
static
rc_t CC VLinkerWhack ( VLinker *self )
{
    KRefcountWhack ( & self -> refcount, "VLinker" );

    VectorWhack ( & self -> fact, LFactoryWhack, NULL );
    VectorWhack ( & self -> special, LSpecialWhack, NULL );
    BSTreeWhack ( & self -> scope, KSymbolWhack, NULL );

    KDyldRelease ( self -> dl );
    VLinkerSever ( self -> dad );

    free ( self );

    return 0;
}


/* StartIdx
 *  returns starting index
 */
#define VLinkerStartIdx( dad, vect, starting ) \
    ( ( ( dad ) == NULL ) ? ( starting ) : \
      ( VectorStart ( & ( dad ) -> vect ) + \
        VectorLength ( & ( dad ) -> vect ) ) )

/* VectorInit
 *  performs vector initialization
 */
#define VLinkerVectorInit( linker, dad, vect, starting, block ) \
    VectorInit ( & ( linker ) -> vect, VLinkerStartIdx ( dad, vect, starting ), block )

/* Make
 *  creates an empty linker
 */
rc_t VLinkerMake ( VLinker **lp, const VLinker *dad, struct KDyld *dl )
{
    rc_t rc;

    VLinker *linker = malloc ( sizeof * linker );
    if ( linker == NULL )
        return RC ( rcVDB, rcMgr, rcConstructing, rcMemory, rcExhausted );

    rc = KDyldAddRef ( linker -> dl = dl );
    if ( rc == 0 )
    {
        linker -> dad = VLinkerAttach ( dad );
        BSTreeInit ( & linker -> scope );

        VLinkerVectorInit ( linker, dad, fact, 1, 64 );
        VLinkerVectorInit ( linker, dad, special, 1, 8 );

        KRefcountInit ( & linker -> refcount, 1, "VLinker", "make", "vld" );
    
        * lp = linker;
        return 0;
    }

    free ( linker );
    return rc;
}

/* Release
 */
rc_t VLinkerRelease ( const VLinker *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VLinker" ) )
        {
        case krefWhack:
            return VLinkerWhack ( ( VLinker* ) self );
        case krefNegative:
            return RC ( rcVDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 */
VLinker *VLinkerAttach ( const VLinker *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "VLinker" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( VLinker* ) self;
}

rc_t VLinkerSever ( const VLinker *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "VLinker" ) )
        {
        case krefWhack:
            return VLinkerWhack ( ( VLinker* ) self );
        case krefNegative:
            return RC ( rcVDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* AddLoadLibraryPath
 *  add a path[s] to loader for locating dynamic libraries
 */
rc_t VLinkerVAddLoadLibraryPath ( const VLinker *self, const char *path, va_list args )
{
    if ( self != NULL )
        return KDyldVAddSearchPath ( self -> dl, path, args );

    return RC ( rcVDB, rcMgr, rcUpdating, rcSelf, rcNull );
}


/* Open
 *  opens libraries for search
 */
rc_t VLinkerOpen ( const VLinker *self, struct KDlset **libs )
{
    rc_t rc = KDyldMakeSet ( self -> dl, libs );
    if ( rc == 0 )
    {
        KDlset *set = * libs;

        rc = KDlsetAddAll ( set );
        if ( rc == 0 )
            return 0;

        KDlsetRelease ( set );
        * libs = NULL;
    }
    return rc;
}


/* EnterIntoScope
 *  pushes linker symbol tables into scope
 */
static
rc_t CC VLinkerEnterIntoScope ( const VLinker *self, KSymTable *tbl )
{
    rc_t rc;

    if ( self -> dad == NULL )
        rc = KSymTableInit ( tbl, NULL );
    else
        rc = VLinkerEnterIntoScope ( self -> dad, tbl );

    if ( rc == 0 )
        rc = KSymTablePushScope ( tbl, & ( ( VLinker* ) self ) -> scope );

    return rc;
}


/* MakeFQN
 */
static
size_t CC VLinkerMakeFQN ( char *buffer, size_t bsize, const KSymbol *name )
{
    size_t sz = 0;

    if ( name -> dad != NULL )
    {
        sz = VLinkerMakeFQN ( buffer, bsize, name -> dad );
        if ( sz < bsize )
            buffer [ sz ++ ] = '_';
    }

    if ( sz < bsize )
    {
        sz += string_copy ( & buffer [ sz ], bsize - sz,
            name -> name . addr, name -> name . size );
    }

    return sz;
}

/* ScanFactory
 *  scans code modules for all named entrypoints
 *  compares version numbers, retaining the highest
 *  that satisfies stated functional interface
 */
typedef struct VLinkerFactoryMatchData VLinkerFactoryMatchData;
struct VLinkerFactoryMatchData
{
    VTransDesc desc;
    uint32_t version;
};

static
bool CC VLinkerBestMatch ( const KSymAddr *sym, void *data )
{
    VLinkerFactoryMatchData *pb = data;

    rc_t rc;
    VTransDesc desc;

    rc_t ( CC * f ) ( VTransDesc* );
    KSymAddrAsFunc ( sym, ( fptr_t* ) & f );

    memset ( & desc, 0, sizeof desc );
    rc = ( * f ) ( & desc );
    if ( rc == 0 )
    {
        /* we don't actually expect to see two versions
           of a factory at all, much less with the same
           major interface version */
        uint32_t maj = desc . itf_version >> 24;
        if ( maj != 0 && maj == ( pb -> desc . itf_version >> 24 ) )
        {
            /* TBD - issue warning */
        }

        /* prototype should have been declared with a version,
           but if not, then any version will do */
        if ( pb -> version == 0 ||
             ( maj == ( pb -> version >> 24 ) &&
               desc . itf_version >= pb -> version ) )
        {
            /* accept latest version */
            if ( desc . itf_version > pb -> desc . itf_version )
            {
                pb -> desc = desc;
                return true;
            }
        }
    }
    return false;
}

static
rc_t VLinkerScanFactory ( VLinker *self, const KDlset *libs,
    KSymTable *tbl, const KSymbol **symp, const KSymbol *name,
    uint32_t version )
{
    rc_t rc;

    /* convert schema fqn into legal C identifier */
    char fqn [ 1024 ];
    size_t sz = VLinkerMakeFQN ( fqn, sizeof fqn, name );
    if ( sz >= sizeof fqn )
        rc = RC ( rcVDB, rcMgr, rcResolving, rcName, rcExcessive );
    else
    {
        KSymAddr *entrypoint;
        VLinkerFactoryMatchData pb;
        memset ( & pb, 0, sizeof pb );
        pb . version = version;

        /* ask loader to locate fqn in library search path */
        rc = KDlsetLastSymbol ( libs, & entrypoint,
            fqn, VLinkerBestMatch, & pb );
        if ( rc == 0 )
        {
            /* allocate factory object */
            LFactory *fact = malloc ( sizeof * fact );
            if ( fact == NULL )
                rc = RC ( rcVDB, rcMgr, rcResolving, rcMemory, rcExhausted );
            else
            {
                /* give it the exact schema name */
                rc = KSymTableDupSymbol ( tbl,
                    ( KSymbol** ) & fact -> name, name, ltFactory, fact );
                if ( rc == 0 )
                {
                    /* finally, insert the symbol into our vector */
                    rc = VectorAppend ( & self -> fact, & fact -> id, fact );
                    if ( rc == 0 )
                    {
                        /* done */
                        fact -> addr = entrypoint;
                        fact -> desc = pb . desc;
                        fact -> external = true;
                        * symp = fact -> name;
                        return 0;
                    }

                    KSymTableRemoveSymbol ( tbl, fact -> name );
                }

                free ( fact );
            }

            KSymAddrRelease ( entrypoint );
        }
    }

    return rc;
}

/* ScanSpecial
 *  scans code modules for the first named entrypoint
 */
static
rc_t CC VLinkerScanSpecial ( VLinker *self, const KDlset *libs,
    KSymTable *tbl, const KSymbol **symp, const KSymbol *name, uint32_t type )
{
    rc_t rc;

    /* convert schema fqn into legal C identifier */
    char fqn [ 1024 ];
    size_t sz = VLinkerMakeFQN ( fqn, sizeof fqn, name );
    if ( sz >= sizeof fqn )
        rc = RC ( rcVDB, rcMgr, rcResolving, rcName, rcExcessive );
    else
    {
        /* ask loader to locate fqn in open library set */
        KSymAddr *entrypoint;
        rc = KDlsetSymbol ( libs, & entrypoint, fqn );
        if ( rc == 0 )
        {
            /* allocate special function object */
            LSpecial *special = malloc ( sizeof * special );
            if ( special == NULL )
                rc = RC ( rcVDB, rcMgr, rcResolving, rcMemory, rcExhausted );
            else
            {
                special -> func = NULL;

                /* give it the exact schema name */
                rc = KSymTableDupSymbol ( tbl,
                    ( KSymbol** ) & special -> name, name, type, special );
                if ( rc == 0 )
                {
                    /* finally, insert the symbol into our vector */
                    rc = VectorAppend ( & self -> special, & special -> id, special );
                    if ( rc == 0 )
                    {
                        /* done */
                        special -> addr = entrypoint;
                        * symp = special -> name;
                        return 0;
                    }

                    KSymTableRemoveSymbol ( tbl, special -> name );
                }

                free ( special );
            }

            KSymAddrRelease ( entrypoint );
        }
    }

    return rc;
}

/* Find
 *  find a named symbol
 *
 *  "func" [ OUT ] - return parameter for func pointer
 *
 *  "proto" [ IN ] - function prototype from schema
 *  "min_version" [ IN ] - minimum version to accept
 *
 *  "external" [ OUT ] - tells whether factory is built-in or external
 */
rc_t VLinkerFindFactory ( VLinker *self, const KDlset *libs,
    VTransDesc *desc, const SFunction *proto, uint32_t min_version, bool *external )
{
    KSymTable tbl;
    rc_t rc = VLinkerEnterIntoScope ( self, & tbl );
    if ( rc == 0 )
    {
        /* factory name if explicit */
        const KSymbol *sym, *name = proto -> u . ext . fact;

        /* use simple function name as implicit factory name */
        if ( name == NULL )
            name = proto -> name;

        /* look for the symbol by name -
           factory names do not support version overloading */
        sym = KSymTableFindSymbol ( & tbl, name );

        /* if not found, search for best match */
        if ( sym == NULL )
        {
            rc = VLinkerScanFactory ( self, libs, & tbl, & sym, name, proto -> version );
            if ( rc != 0 && min_version != proto -> version )
                rc = VLinkerScanFactory ( self, libs, & tbl, & sym, name, min_version );
        }

        /* otherwise ensure the found name is in fact a factory */
        else if ( sym -> type != ltFactory )
            rc = RC ( rcVDB, rcMgr, rcResolving, rcName, rcIncorrect );

        /* examine results */
        if ( rc == 0 )
        {
            const LFactory *fact = sym -> u . obj;

            /* test for incompatible version */
            if ( ( fact -> desc . itf_version >> 24 ) != ( min_version >> 24 ) )
                rc = RC ( rcVDB, rcMgr, rcResolving, rcInterface, rcIncorrect );
            else if ( fact -> desc . itf_version < min_version )
                rc = RC ( rcVDB, rcMgr, rcResolving, rcInterface, rcIncorrect );
            else
            {
                * desc = fact -> desc;
                * external = fact -> external;
            }
        }

        KSymTableWhack ( & tbl );
    }
    return rc;
}

static
void CC VLinkerNameWhackSymbol ( KSymbol *sym )
{
    if ( sym != NULL )
    {
        VLinkerNameWhackSymbol ( sym -> dad );
        free ( sym );
    }
}

static
KSymbol *VLinkerNameToSymbol ( String *name )
{
    KSymbol *sym, *dad = NULL;
    const char *end = string_rchr ( name -> addr, name -> size, ':' );
    if ( end != 0 )
    {
        String sub;
        StringSubstr ( name, & sub, 0, string_len ( name -> addr, end - name -> addr ) );
        dad = VLinkerNameToSymbol ( & sub );
        if ( dad == NULL )
            return NULL;

        BSTreeInit ( & dad -> u . scope );

        name -> addr += sub . size + 1;
        name -> size -= sub . size + 1;
        name -> len -= sub . len + 1;
    }

    sym = malloc ( sizeof * sym );
    if ( sym == NULL )
    {
        VLinkerNameWhackSymbol ( dad );
        return NULL;
    }

    sym -> u . obj = NULL;
    sym -> dad = dad;
    sym -> name = * name;
    sym -> type = 0;

    if ( dad != NULL )
        BSTreeInsert ( & dad -> u . scope, & sym -> n, KSymbolSort );

    return sym;
}

rc_t VLinkerFindNamedFactory ( VLinker *self, const KDlset *libs,
    VTransDesc *desc, const char *fact_name )
{
    KSymTable tbl;
    rc_t rc = VLinkerEnterIntoScope ( self, & tbl );
    if ( rc == 0 )
    {
        KSymbol *name;
        const KSymbol *sym;

        String str;
        StringInitCString ( & str, fact_name );
        name = VLinkerNameToSymbol ( & str );

        /* look for the symbol by name -
           factory names do not support version overloading */
        sym = KSymTableFindSymbol ( & tbl, name );
        
        /* if not found, search for best match */
        if ( sym == NULL )
            rc = VLinkerScanFactory ( self, libs, & tbl, & sym, name, 0 );

        /* otherwise ensure the found name is in fact a factory */
        else if ( sym -> type != ltFactory )
            rc = RC ( rcVDB, rcMgr, rcResolving, rcName, rcIncorrect );

        VLinkerNameWhackSymbol ( name );

        /* examine results */
        if ( rc == 0 )
        {
            const LFactory *fact = sym -> u . obj;
            * desc = fact -> desc;
        }

        KSymTableWhack ( & tbl );
    }
    return rc;
}


rc_t VLinkerFindUntyped ( VLinker *self, const KDlset *libs,
    VUntypedTableTest *func, const SFunction *proto )
{
    KSymTable tbl;
    rc_t rc = VLinkerEnterIntoScope ( self, & tbl );
    if ( rc == 0 )
    {
        /* untyped functions do not have factories */
        const KSymbol *sym = KSymTableFindSymbol ( & tbl, proto -> name );

        /* if not found, search for it */
        if ( sym == NULL )
            rc = VLinkerScanSpecial ( self, libs, & tbl, & sym, proto -> name, ltUntyped );

        /* otherwise ensure the found name is in fact untyped */
        else if ( sym -> type != ltUntyped )
            rc = RC ( rcVDB, rcMgr, rcResolving, rcName, rcIncorrect );

        /* return results */
        if ( rc == 0 )
        {
            const LSpecial *untyped = sym -> u . obj;
            if ( untyped -> addr == NULL )
                * func = untyped -> func;
            else
                KSymAddrAsFunc ( untyped -> addr, ( fptr_t* ) func );
        }

        KSymTableWhack ( & tbl );
    }
    return rc;
}


/* ListExternalSchemaModules
 */
rc_t VLinkerListExternalSchemaModules ( const VLinker *self, struct KNamelist **listp )
{
    rc_t rc;

    assert ( listp != NULL );
    if ( self == NULL )
        rc = RC ( rcVDB, rcDylib, rcListing, rcSelf, rcNull );
    else
    {
        KDlset *libs;
        rc = VLinkerOpen ( self, & libs );
        if ( rc == 0 )
        {
            rc = KDlsetList ( libs, listp );
            KDlsetRelease ( libs );
        }
    }

    return rc;
}
