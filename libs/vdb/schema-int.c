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

#include <vdb/vdb-priv.h>
#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <byteswap.h>
#include <assert.h>

#define INTRINSIC_SUB_BYTE_MULTI_BIT_TYPES 0

/* init_keywords
 *  create simple symbols with special ids
 */
static
rc_t init_keywords ( KSymTable *tbl )
{
    int i;
    static struct
    {
        const char *keyword;
        int id;
    } kw [] =
    {
#define KEYWORD( word ) \
        { # word, kw_ ## word }

        KEYWORD ( alias ),
        KEYWORD ( column ),
        KEYWORD ( const ),
        KEYWORD ( control ),
        KEYWORD ( database ),
        KEYWORD ( decode ),
        KEYWORD ( default ),
        KEYWORD ( encode ),
        KEYWORD ( extern ),
        KEYWORD ( false ),
        KEYWORD ( fmtdef ),
        KEYWORD ( function ),
        KEYWORD ( include ),
        KEYWORD ( index ),
        KEYWORD ( limit ),
        KEYWORD ( physical ),
        KEYWORD ( read ),
        KEYWORD ( readonly ),
        KEYWORD ( return ),
        KEYWORD ( schema ),
        KEYWORD ( static ),
        KEYWORD ( table ),
        KEYWORD ( trigger ),
        KEYWORD ( template ),
        KEYWORD ( true ),
        KEYWORD ( type ),
        KEYWORD ( typedef ),
        KEYWORD ( typeset ),
        KEYWORD ( validate ),
        KEYWORD ( version ),
        KEYWORD ( view ),
        KEYWORD ( virtual ),
        KEYWORD ( void ),
        KEYWORD ( writeonly ),
        KEYWORD ( __untyped ),
        KEYWORD ( __row_length ),
        KEYWORD ( __no_header )

#undef KEYWORD
    };

    /* define keywords */
    for ( i = 0; i < sizeof kw / sizeof kw [ 0 ]; ++ i )
    {
        rc_t rc;

        String name;
        StringInitCString ( & name, kw [ i ] . keyword );
        rc = KSymTableCreateSymbol ( tbl, NULL, & name, kw [ i ] . id, NULL );
        if ( rc != 0 )
            return rc;
    }

    return 0;
}


/* byte swapping */
static
void byte_swap_16 ( void *dst, const void *src, uint64_t count )
{
    uint16_t *d = dst;
    const uint16_t *s = src;

    uint64_t i;
    for ( i = 0; i < count; ++ i )
        d [ i ] = bswap_16 ( s [ i ] );
}

static
void byte_swap_32 ( void *dst, const void *src, uint64_t count )
{
    uint32_t *d = dst;
    const uint32_t *s = src;

    uint64_t i;
    for ( i = 0; i < count; ++ i )
        d [ i ] = bswap_32 ( s [ i ] );
}

static
void byte_swap_64 ( void *dst, const void *src, uint64_t count )
{
    uint64_t *d = dst;
    const uint64_t *s = src;

    uint64_t i;
    for ( i = 0; i < count; ++ i )
        d [ i ] = bswap_64 ( s [ i ] );
}

#if SUPPORT_128_BIT
static
void byte_swap_128 ( void *dst, const void *src, uint64_t count )
{
    uint64_t i;
    uint64_t *d = dst;
    const uint64_t *s = src;

    if ( ( const uint64_t* ) d != s )
    {
        for ( count += count, i = 0; i < count; i += 2 )
        {
            d [ i + 0 ] = bswap_64 ( s [ i + 1 ] );
            d [ i + 1 ] = bswap_64 ( s [ i + 0 ] );
        }
    }
    else
    {
        for ( count += count, i = 0; i < count; i += 2 )
        {
            uint64_t tmp = bswap_64 ( s [ i + 0 ] );
            d [ i + 0 ] = bswap_64 ( s [ i + 1 ] );
            d [ i + 1 ] = tmp;
        }
    }
}
#endif


/* InitTypes
 *  set up intrinsic types
 */
static
rc_t VSchemaInitTypes ( VSchema *self, KSymTable *tbl )
{
    int i;
    static struct
    {
        const char *newtype;
        const char *supertype;
        void ( * byte_swap ) ( void *dst, const void *src, uint64_t count );
        uint8_t dim;
        uint8_t domain;
    } dt [] =
    {
        { "any", NULL, NULL, 1, 0 },
        { "void", NULL, NULL, 1, 0 },
        { "opaque", NULL, NULL, 1, 0 },

        /* bundles of bits in machine native order */
        { "B1", "opaque", NULL, 1, ddUint },
        { "B8", "opaque", NULL, 8, ddUint },
        { "B16", "opaque", byte_swap_16, 16, ddUint },
        { "B32", "opaque", byte_swap_32, 32, ddUint },
        { "B64", "opaque", byte_swap_64, 64, ddUint },

        /* the basic unsigned integer types */
        { "U1", "B1", NULL, 1, ddUint },
        { "U8", "B8", NULL, 1, ddUint },
        { "U16", "B16", byte_swap_16, 1, ddUint },
        { "U32", "B32", byte_swap_32, 1, ddUint },
        { "U64", "B64", byte_swap_64, 1, ddUint },

        /* the basic signed integer types */
        { "I8", "B8", NULL, 1, ddInt },
        { "I16", "B16", byte_swap_16, 1, ddInt },
        { "I32", "B32", byte_swap_32, 1, ddInt },
        { "I64", "B64", byte_swap_64, 1, ddInt },

#if INTRINSIC_SUB_BYTE_MULTI_BIT_TYPES
        /* all the basic bit tuples are subtypes
           of opaque, making them essentially atomic,
           required due to native byte order issues */
        { "B2", "opaque", NULL, 2, ddUint },
        { "B4", "opaque", NULL, 4, ddUint },
        { "U2", "B2", NULL, 1, ddUint },
        { "U4", "B4", NULL, 1, ddUint },
        { "I4", "B4", NULL, 1, ddInt },
#endif

#if SUPPORT_128_BIT
        /* 128 bit types */
        { "B128", "opaque", byte_swap_128, 128, ddUint },
        { "U128", "B128", byte_swap_128, 1, ddUint },
        { "I128", "B128", byte_swap_128, 1, ddInt },
#endif

        /* floating point */
        { "F32", "B32", byte_swap_32, 1, ddFloat },
        { "F64", "B64", byte_swap_64, 1, ddFloat },

        /* bool is typed to reflect C/C++ */
        { "bool", "U8", NULL, 1, ddBool },

        /* default character set is UNICODE */
        { "utf8", "B8", NULL, 1, ddUnicode },
        { "utf16", "B16", byte_swap_16, 1, ddUnicode },
        { "utf32", "B32", byte_swap_32, 1, ddUnicode },

        /* contrary to sub-classing, which extends/expands
           a class, sub-typing refines it, making ASCII
           a sub-type of UTF-8 in our system */
        { "ascii", "utf8", NULL, 1, ddAscii }
    };

    for ( i = 0; i < sizeof dt / sizeof dt [ 0 ]; ++ i )
    {
        rc_t rc;
        String name;
        SDatatype *newtype;
        const SDatatype *super = NULL;

        if ( dt [ i ] . supertype != NULL )
        {
            KSymbol *sym;

            /* find supertype in symbol table */
            StringInitCString ( & name, dt [ i ] . supertype );
            sym = KSymTableFind ( tbl, & name );
            assert ( sym != NULL );
            assert ( sym -> type == eDatatype );
            super = sym -> u . obj;
        }

        /* create datatype */
        newtype = malloc ( sizeof * newtype );
        if ( newtype == NULL )
            return RC ( rcVDB, rcSchema, rcConstructing, rcMemory, rcExhausted );

        /* initialize datatype */
        newtype -> super = super;
        newtype -> byte_swap = dt [ i ] . byte_swap;
        newtype -> size = ( ( super != NULL ) ? super -> size : 1 ) * dt [ i ] . dim;
        newtype -> dim = dt [ i ] . dim;
        newtype -> domain = dt [ i ] . domain;

        /* just to cover any lazy programming, take byte_swap
           from super-type if not already specified */
        if ( super != NULL && newtype -> byte_swap == NULL )
            newtype -> byte_swap = super -> byte_swap;

        /* create symbol for new type */
        StringInitCString ( & name, dt [ i ] . newtype );
        newtype -> name = KSymTableFind ( tbl, & name );
        assert ( newtype -> name == NULL || i == 1 );
        if ( newtype -> name == NULL )
            rc = KSymTableCreateConstSymbol ( tbl, & newtype -> name, & name, eDatatype, newtype );
        else
            rc = 0;

        /* add datatype */
        if ( rc == 0 )
            rc = VectorAppend ( & self -> dt, & newtype -> id, newtype );

        if ( rc != 0 )
        {
            free ( newtype );
            return rc;
        }
    }

    return 0;
}


/* MakeIntrinsic
 *  initialize intrinsics
 */
rc_t VSchemaMakeIntrinsic ( VSchema **sp )
{
    rc_t rc = VSchemaMake ( sp, NULL );
    if ( rc == 0 )
    {
        KSymTable tbl;
        VSchema *self = * sp;

        /* create symbol table with no intrinsic scope */
        rc = KSymTableInit ( & tbl, NULL );
        if ( rc == 0 )
        {
            /* make intrinsic scope modifiable */
            KSymTablePushScope ( & tbl, & self -> scope );

            /* add keywords, which are only names */
            rc = init_keywords ( & tbl );
            if ( rc == 0 )
            {
                /* TBD - init intrinsic formats */
                rc = VSchemaInitTypes ( self, & tbl );
                if ( rc == 0 )
                {
                    KSymTableWhack ( & tbl );
                    return 0;
                }
            }

            KSymTableWhack ( & tbl );
        }

        VSchemaRelease ( self );
    }

    * sp = NULL;

    return rc;
}

/* LastIntrinsicTypeId - PRIVATE
 *  used to allow cast to intrinsics
 */
LIB_EXPORT uint32_t CC VSchemaLastIntrinsicTypeId ( const VSchema *self )
{
    uint32_t last;
    for ( last = 0; self != NULL; self = self -> dad )
    {
        if ( self -> dad == NULL )
        {
            last = VectorStart ( & self -> dt ) + VectorLength ( & self -> dt );
            if ( last != 0 )
                -- last;
            break;
        }
    }

    return last;
}
