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

#include "va_copy.h"

#define KONST const
#include "dbmgr-priv.h"
#undef KONST

#include "schema-priv.h"
#include "schema-tok.h"
#include "schema-parse.h"

#include <kdb/meta.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/namelist.h>
#include <klib/data-buffer.h>
#include <klib/printf.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <klib/text.h>

#include <vdb/schema-priv.h> /* KSymbolName */
#include <vdb/vdb-priv.h>

#include <kfg/properties.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * generic
 */

void CC VectMbrWhack ( void *item, void *ignore )
{
    free ( item );
}

void CC BSTreeMbrWhack ( BSTNode *n, void *ignore )
{
    free ( n );
}

/*--------------------------------------------------------------------------
 * KSymbol
 */
struct CloneBlock
{
    KSymbol * targetNs;
    rc_t rc;
};

static rc_t CloneSymbol ( BSTree *scope, const KSymbol **cp, const KSymbol *orig, KSymbol * targetNs );

static
bool CC CloneNode ( BSTNode *n, void *data )
{
    const KSymbol * sym = (const KSymbol *)n;
    struct CloneBlock * bl = (struct CloneBlock*) data;
    KSymbol * targetNs = bl -> targetNs;
    const KSymbol * ignore;
    bl -> rc = CloneSymbol ( & targetNs -> u . scope, & ignore, sym, bl -> targetNs );
    return bl -> rc != 0;
}

static
rc_t
CloneSymbol ( BSTree *scope, const KSymbol **cp, const KSymbol *orig, KSymbol * targetNs )
{
    rc_t rc;
    KSymbol * copy = malloc ( sizeof (*copy) + orig->name.size + 1 );
    KSymbol * existing;

    assert ( targetNs == NULL || scope == & targetNs -> u . scope );
    /* copy original including string */
    * copy = * orig;
    string_copy ( ( char* ) ( copy + 1 ), orig -> name . size + 1, orig -> name . addr, orig -> name . size );

    /* fix up pointer to string */
    copy -> name . addr = ( const char* ) ( copy + 1 );
    copy -> dad = targetNs; /* NULL for global names */
    rc = BSTreeInsertUnique ( scope, & copy -> n, (BSTNode **) &existing, KSymbolSort );
    if ( GetRCState ( rc ) == rcExists )
    {
        assert ( existing != NULL );

        free ( copy );
        rc = 0;
        if ( orig -> type == eNamespace )
        {   /* clone the contents of the namespace*/
            struct CloneBlock bl;
            assert ( existing -> type == eNamespace );
            bl . targetNs = existing;
            if ( BSTreeDoUntil ( & orig -> u . scope, false, CloneNode, & bl ) )
            {
                rc = bl . rc;
            }
        }

        if ( rc == 0 )
        {
            *cp = existing;
        }
        else
        {
            *cp = NULL;
        }
    }
    else
    {
        if ( rc == 0  && orig -> type == eNamespace )
        {   /* clone the contents of the namespace*/
            struct CloneBlock bl;
            bl . targetNs = copy;
            BSTreeInit ( & copy -> u . scope );
            if ( BSTreeDoUntil ( & orig -> u . scope, false, CloneNode, & bl ) )
            {
                rc = bl . rc;
            }
        }

        if ( rc == 0 )
        {
            *cp = copy;
        }
        else
        {
            *cp = NULL;
        }
    }
    return rc;
}

static
const KSymbol *
LookupQualIdent ( const BSTree *scope, const KSymbol *orig )
{   /* descend the chain of namespaces leading to 'orig' to locate the corresponding leaf in 'scope' */
    Vector namespaceStack; /* const KSymbol* */
    {
        const KSymbol * ns;
        VectorInit ( & namespaceStack, 0, 32 );
        ns = orig -> dad;
        while ( ns != NULL )
        {
            VectorAppend( & namespaceStack, NULL, ns );
            ns = ns -> dad;
        }
    }
    /* now namespaces are in the vector in order from innermost to outermost, traverse back to front to descend to 'orig' starting from 'scope' */
    {
        const BSTree * curScope = scope;
        uint32_t i = VectorLength ( & namespaceStack );
        while ( i != 0 )
        {
            const KSymbol * origNs = ( const KSymbol* ) VectorGet ( & namespaceStack, i - 1 );
            const KSymbol * newInnerNs = ( const KSymbol* ) BSTreeFind ( curScope, & origNs -> name, KSymbolCmp );
            assert ( newInnerNs != NULL );
            assert ( newInnerNs -> type == eNamespace );
            curScope = & newInnerNs -> u . scope;
            --i;
        }

        { /* now curScope is the innermost namespace in the copy, find the copy of the 'orig' in it */
            const KSymbol * ret = ( const KSymbol* ) BSTreeFind ( curScope, & orig -> name, KSymbolCmp );
            assert ( ret != NULL );
            VectorWhack ( & namespaceStack, NULL, NULL );
            return ret;
        }
    }
}

rc_t KSymbolCopy ( BSTree *scope, const KSymbol **cp, const KSymbol *orig )
{
    rc_t rc;
    assert ( scope != NULL );
    assert ( cp != NULL );
    assert ( orig != NULL );

    if ( orig -> dad != NULL )
    {
        KSymbol * ns = orig -> dad;
        /* locate the topmost namespace */
        while ( ns -> dad != NULL )
        {
            ns = ns -> dad;
        }
        /* copy the entire namespace */
        rc = CloneSymbol( scope, cp, ns, NULL );
        if ( rc == 0 )
        {   /* locate and return the copy of the original symbol */
            *cp = ( KSymbol * ) LookupQualIdent ( scope, orig );
        }
    }
    else
    {   /* global name */
        rc = CloneSymbol( scope, cp, orig, NULL );
    }
    if ( rc == 0 )
    {
        assert ( *cp != NULL );
    }
    return rc;
}

bool CC KSymbolCopyScope ( BSTNode *n, void *scope )
{
    const KSymbol * ignore;
    return KSymbolCopy ( scope, & ignore, ( const KSymbol* ) n ) != 0;
}

#if _DEBUGGING
static
void CC KSymbolList ( BSTNode *item, void *data )
{
    const KSymbol *self = ( const KSymbol* ) item;
    const char *indent = ( const char* ) data;

    OUTMSG (( "%s%.*s = %#p"
             , indent
             , ( int ) self -> name . size, self -> name . addr
             , self
        ));

    if ( self -> type != eNamespace )
        putchar ( '\n' );
    else
    {
        printf ( " ( namespace ):\n" );
        if ( indent [ -1 ] != '.' )
            -- indent;

        BSTreeForEach ( & self -> u . scope, false, KSymbolList, ( void* ) indent );
    }
}

static
void CC BSTreeListSymbols ( const BSTree *self, const char *indent )
{
    OUTMSG (( "%sscope = %#p\n"
             , indent
             , self
        ));

    if ( indent [ -1 ] != '.' )
        -- indent;

    BSTreeForEach ( self, false, KSymbolList, ( void* ) indent );
}
#endif



/* ExamineContents
 *  not much to be done here
 */
#if EXAMINE_SCHEMA_CONTENTS >= 1
bool CC KSymbolExamineContents ( BSTNode *n, void *data )
{
    rc_t *rc = data;
    const KSymbol *sym = ( const KSymbol* ) n;

    /* can test heuristic that name is NULL-terminated,
       that name size and length are identical */

    /* can test that symbol type is a valid id type */
    return false;
}
#endif




/*--------------------------------------------------------------------------
 * KMDataNode
 */

/* FillSchema
 *  a schema buffer fill function that reads from KMDataNode
 */
rc_t CC KMDataNodeFillSchema ( void *data, KTokenText *tt, size_t save )
{
    rc_t rc;
    size_t num_read;
    KMDataNodeSchemaFillData *pb = data;

     /* on very first fill, check for specifying v0 */
    if ( pb -> add_v0 )
    {
        assert ( save == 0 );
        CONST_STRING ( & tt -> str, "version 0;" );
        pb -> add_v0 = false;
        return 0;
    }

   /* save any characters not yet consumed */
    if ( save != 0 )
    {
        assert ( save < sizeof pb -> buff );
        memmove ( pb -> buff,
            & tt -> str . addr [ tt -> str . size - save ], save );
    }

    /* read as many characters as are available */
    rc = KMDataNodeRead ( pb -> node, pb -> pos,
        & pb -> buff [ save ], sizeof pb -> buff - save, & num_read, NULL );
    if ( rc == 0 )
    {
    /******** PROTECT FROM BUGS IN TOKENIZER ****/
    if(num_read == sizeof (pb -> buff) - save){ /** not the last chunk ***/
        int i;
        for(i=num_read+save-1;i > save && (isalnum(pb -> buff[i]) || pb -> buff[i]=='_');i--){}
        if(i > save && pb -> buff[i]=='.'){ /*** leave this physical column for the next buffer **/
            num_read=i-save;
        }
    }
    /********************************************/
        /* reset the buffer in "tt" */
        tt -> str . addr = pb -> buff;
        tt -> str . size = save + num_read;
        tt -> str . len = string_len ( pb -> buff, save + num_read );
        pb -> pos += num_read;
    }
    return rc;
}


/*--------------------------------------------------------------------------
 * SNameOverload
 */

/* Whack
 */
void CC SNameOverloadWhack ( void *item, void *ignore )
{
    SNameOverload *self = item;
    VectorWhack ( & self -> items, NULL, NULL );
    free ( self );
}

/* Make
 */
rc_t SNameOverloadMake ( SNameOverload **np,
    const KSymbol *sym, uint32_t ctx_type, uint32_t start, uint32_t len )
{
    SNameOverload *name = malloc ( sizeof * name );
    if ( name == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    name -> name = sym;
    ( ( KSymbol* ) sym ) -> u . obj = name;
    VectorInit ( & name -> items, start, len );
    name -> cid . ctx = 0;
    name -> cid . ctx_type = ctx_type; /* 0 for top-level objects (table, db, function) */

    * np = name;
    return 0;
}

/* Copy
 */
rc_t SNameOverloadCopy ( BSTree *scope,
    SNameOverload **cp, const SNameOverload *orig )
{
    const KSymbol *sym;
    rc_t rc = KSymbolCopy ( scope, & sym, orig -> name );
    if ( rc == 0 )
    {
        SNameOverload *copy;
        rc = SNameOverloadMake ( & copy, sym, orig -> cid . ctx_type, 0, 0 );
        if ( rc == 0 )
        {
            /* copy contents */
            rc = VectorCopy ( & orig -> items, & copy -> items );
            if ( rc == 0 )
            {
                copy -> cid = orig -> cid;
                * cp = copy;
                return 0;
            }

            SNameOverloadWhack ( copy, NULL );
        }
    }

    * cp = NULL;
    return rc;
}

typedef struct SNameOverloadCopyData SNameOverloadCopyData;
struct SNameOverloadCopyData
{
    BSTree *scope;
    Vector *dest;
    rc_t rc;
};

static
bool CC SNameOverloadVectorCopyItem ( void *item, void *data )
{
    SNameOverloadCopyData *pb = data;

    SNameOverload *copy;
    pb -> rc = SNameOverloadCopy ( pb -> scope, & copy, ( const void* ) item );
    if ( pb -> rc == 0 )
        pb -> rc = VectorAppend ( pb -> dest, & copy -> cid . id, copy );

    return ( pb -> rc != 0 ) ? true : false;
}

rc_t SNameOverloadVectorCopy ( BSTree *scope, const Vector *src, Vector *dest )
{
    SNameOverloadCopyData pb;
    pb . scope = scope;
    pb . dest = dest;
    pb . rc = 0;

    if ( VectorDoUntil ( src, false, SNameOverloadVectorCopyItem, & pb ) )
        return pb . rc;

    return 0;
}

/*--------------------------------------------------------------------------
 * VIncludedPath
 */
typedef struct VIncludedPath VIncludedPath;
struct VIncludedPath
{
    BSTNode n;
    uint32_t ord;
    char path [ 1 ];
};

/* Cmp
 * Sort
 */
static
int64_t CC VIncludedPathCmp ( const void *item, const BSTNode *n )
{
    const char *a = item;
    const VIncludedPath *b = ( const VIncludedPath* ) n;
    return strcmp ( a, b -> path );
}

static
int64_t CC VIncludedPathSort ( const BSTNode *item, const BSTNode *n )
{
    const VIncludedPath *a = ( const VIncludedPath* ) item;
    const VIncludedPath *b = ( const VIncludedPath* ) n;
    return strcmp ( a -> path, b -> path );
}

static
int64_t CC VIncludedPathSortByOrder ( const BSTNode *item, const BSTNode *n )
{
    const VIncludedPath *a = ( const VIncludedPath* ) item;
    const VIncludedPath *b = ( const VIncludedPath* ) n;
    return ( int64_t ) a -> ord - ( int64_t ) b -> ord;
}

/* Make
 */
rc_t CC VIncludedPathMake ( BSTree *paths, uint32_t *count, const char *path )
{
    VIncludedPath *p = malloc ( sizeof * p + strlen ( path ) );
    if ( p == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
    p -> ord = ( * count ) ++;
    strcpy ( p -> path, path );
    BSTreeInsert ( paths, & p -> n, VIncludedPathSort );
    return 0;
}


/*--------------------------------------------------------------------------
 * VSchema
 *  a symbol scope
 *  data types, blob formats, function and sub-schema declarations
 *  database and table declarations
 */


#if _DEBUGGING
/* Listing
 *  list symbol tables
 */
void VSchemaListSymtab ( const VSchema *self )
{
    if ( self != NULL )
    {
        const char *indent = ".                ";
        VSchemaListSymtab ( self -> dad );
        BSTreeListSymbols ( & self -> scope, & indent [ 17 ] );
        putchar ( '\n' );
    }
}
#endif


/* Marking
 */
void VSchemaClearMark ( const VSchema *self )
{
    if ( self == NULL )
        return;

    VectorForEach ( & self -> fmt, false, SFormatClearMark, NULL );
    VectorForEach ( & self -> dt, false, SDatatypeClearMark, NULL );
    VectorForEach ( & self -> ts, false, STypesetClearMark, NULL );
    VectorForEach ( & self -> cnst, false, SConstantClearMark, NULL );
    VectorForEach ( & self -> func, false, SFunctionClearMark, NULL );
    VectorForEach ( & self -> phys, false, SPhysicalClearMark, NULL );
    VectorForEach ( & self -> tbl, false, STableClearMark, NULL );
    VectorForEach ( & self -> db, false, SDatabaseClearMark, NULL );
    /*TODO: self -> view ? */

    if ( self -> dad != NULL )
        VSchemaClearMark ( self -> dad );
}

static void includePathFree(void *item, void *dummy)
{
    free(item);
}

/* Destroy
 */
static
void CC VSchemaDestroy ( VSchema *self )
{
    REFMSG ( "VSchema", "destroy", & self -> refcount );

    BSTreeWhack ( & self -> scope, KSymbolWhack, NULL );
    BSTreeWhack ( & self -> paths, BSTreeMbrWhack, NULL );
    VectorWhack ( & self -> inc, includePathFree, NULL );
    VectorWhack ( & self -> alias, NULL, NULL );
    VectorWhack ( & self -> fmt, SFormatWhack, NULL );
#if SLVL >= 1
    VectorWhack ( & self -> dt, SDatatypeWhack, NULL );
    VectorWhack ( & self -> ts, STypesetWhack, NULL );
#endif
#if SLVL >= 3
    VectorWhack ( & self -> pt, SIndirectTypeWhack, NULL );
#endif
#if SLVL >= 2
    VectorWhack ( & self -> cnst, SConstantWhack, NULL );
#endif
#if SLVL >= 3
    VectorWhack ( & self -> func, SFunctionWhack, NULL );
    VectorWhack ( & self -> fname, SNameOverloadWhack, NULL );
#endif
#if SLVL >= 5
    VectorWhack ( & self -> phys, SPhysicalWhack, NULL );
    VectorWhack ( & self -> pname, SNameOverloadWhack, NULL );
#endif
#if SLVL >= 6
    VectorWhack ( & self -> tbl, STableWhack, NULL );
    VectorWhack ( & self -> tname, SNameOverloadWhack, NULL );

    VectorWhack ( & self -> db, SDatabaseWhack, NULL );
    VectorWhack ( & self -> dname, SNameOverloadWhack, NULL );
#endif

    VectorWhack ( & self -> view, SViewWhack, NULL );
    VectorWhack ( & self -> vname, SNameOverloadWhack, NULL );

    free ( self );
}

/* Whack
 */
static
rc_t CC VSchemaWhack ( VSchema *self )
{
    KRefcountWhack ( & self -> refcount, "VSchema" );

    VSchemaSever ( self -> dad );
    VSchemaDestroy ( self );

    return 0;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC VSchemaAddRef ( const VSchema *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VSchema" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC VSchemaRelease ( const VSchema *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VSchema" ) )
        {
        case krefWhack:
            return VSchemaWhack ( ( VSchema* ) self );
        case krefNegative:
            return RC ( rcVDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 */
VSchema *VSchemaAttach ( const VSchema *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "VSchema" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( VSchema* ) self;
}

rc_t VSchemaSever ( const VSchema *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "VSchema" ) )
        {
        case krefWhack:
            return VSchemaWhack ( ( VSchema* ) self );
        case krefNegative:
            return RC ( rcVDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* StartIdx
 *  returns starting index
 */
#define VSchemaStartIdx( dad, vect, starting ) \
    ( ( ( dad ) == NULL ) ? ( starting ) : \
      ( VectorStart ( & ( dad ) -> vect ) + \
        VectorLength ( & ( dad ) -> vect ) ) )

/* VectorInit
 *  performs vector initialization
 */
#define VSchemaVectorInit( schema, dad, vect, starting, block ) \
    VectorInit ( & ( schema ) -> vect, VSchemaStartIdx ( dad, vect, starting ), block )


/* MakeSchema
 *  make an empty schema
 */
rc_t VSchemaMake ( VSchema **sp,  const VSchema *dad )
{
    VSchema *schema = malloc ( sizeof * schema );
    if ( schema == NULL )
        return RC ( rcVDB, rcMgr, rcConstructing, rcMemory, rcExhausted );

    schema -> dad = VSchemaAttach ( dad );
    BSTreeInit ( & schema -> scope );
    BSTreeInit ( & schema -> paths );

    /* datatypes, typesets and parameterized types all shared
       the same type_id space from 0..0xFFFFFFFF. each is
       assigned a range to avoid overlap:

       datatypes = 0..0x3FFFFFFF
       typesets = 0x40000000..0x7FFFFFFF
       undefined parameterized type = 0x80000000
       parameterized types = 0x80000001..0xFFFFFFFF
    */
    VSchemaVectorInit ( schema, dad, inc, 0, 4 );
    VSchemaVectorInit ( schema, dad, alias, 0, 16 );
    VSchemaVectorInit ( schema, dad, fmt, 1, 16 );
    VSchemaVectorInit ( schema, dad, dt, 0, 128 );
    VSchemaVectorInit ( schema, dad, ts, 0x40000000, 16 );
    VSchemaVectorInit ( schema, dad, pt, 0x80000001, 32 );

    VSchemaVectorInit ( schema, dad, cnst, 0, 32 );
    VSchemaVectorInit ( schema, dad, func, 0, 64 );
    VSchemaVectorInit ( schema, dad, phys, 0, 32 );
    VSchemaVectorInit ( schema, dad, tbl, 0, 16 );
    VSchemaVectorInit ( schema, dad, db, 0, 4 );
    VSchemaVectorInit ( schema, dad, view, 0, 16 );

    VectorInit ( & schema -> fname, 0, 64 );
    VectorInit ( & schema -> pname, 0, 32 );
    VectorInit ( & schema -> tname, 0, 16 );
    VectorInit ( & schema -> dname, 0, 4 );
    VectorInit ( & schema -> vname, 0, 16 );

    KRefcountInit ( & schema -> refcount, 1, "VSchema", "make", "vschema" );
    schema -> file_count = 0;
    schema -> num_indirect = ( dad == NULL ) ? 0 : dad -> num_indirect;

    /* copy the versioned name vectors and names */
    if ( dad != NULL )
    {
        rc_t rc = SNameOverloadVectorCopy ( & schema -> scope, & dad -> fname, & schema -> fname );
        if ( rc == 0 )
            rc = SNameOverloadVectorCopy ( & schema -> scope, & dad -> pname, & schema -> pname );
        if ( rc == 0 )
            rc = SNameOverloadVectorCopy ( & schema -> scope, & dad -> tname, & schema -> tname );
        if ( rc == 0 )
            rc = SNameOverloadVectorCopy ( & schema -> scope, & dad -> dname, & schema -> dname );
        if ( rc != 0 )
        {
            VSchemaWhack ( schema );
            * sp = NULL;
            return rc;
        }
    }

    * sp = schema;
    return 0;
}


/* AddIncludePath
 *  add an include path to schema for locating input files
 */
LIB_EXPORT rc_t CC VSchemaVAddIncludePath ( VSchema *self, const char *path, va_list args )
{
    rc_t rc = -1;
    void *temp = NULL;

    assert(self != NULL);
    if (self == NULL)
        return RC ( rcVDB, rcString, rcAppending, rcSelf, rcNull );
    if (path == NULL)
        path=".";

    if (strchr(path, '%') == NULL)
        temp = string_dup_measure(path, NULL);
    else {
        KDataBuffer buffer;

        memset(&buffer, 0, sizeof(buffer));
        rc = KDataBufferVPrintf(&buffer, path, args);
        if (rc)
            return rc;

        temp = string_dup_measure(buffer.base, NULL);
        KDataBufferWhack(&buffer);
    }
    if (temp == NULL)
        return RC ( rcVDB, rcString, rcAppending, rcMemory, rcExhausted );
    rc = VectorAppend(&self->inc, NULL, temp);
    if (rc)
        free(temp);
    return rc;
}

LIB_EXPORT rc_t CC VSchemaAddIncludePath ( VSchema *self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VSchemaVAddIncludePath ( self, path, args );
    va_end ( args );

    return rc;
}

/*
 * Add ':' separated paths
 */
LIB_EXPORT rc_t CC VSchemaAddIncludePaths(VSchema *self, size_t length, const char *paths)
{
    rc_t rc = 0;
    char const *const max = paths + length;
    char const *cur;
    char const *end;

    assert(self != NULL);
    if (self == NULL)
        return RC ( rcVDB, rcString, rcAppending, rcSelf, rcNull );

    if (paths == NULL || length == 0)
    	return 0;

    for (cur = end = paths; end <= max; ++end) {
        int const ch = end < max ? *end : '\0';

        if (ch == '\0' || ch == ':') {
            size_t const sz = end - cur;
            void *const temp = malloc(sz + 1);

            assert(temp != NULL);
            if (temp == NULL)
                return RC ( rcVDB, rcString, rcAppending, rcMemory, rcExhausted );

            memmove(temp, cur, sz);
            ((char *)temp)[sz] = 0;
            cur = end + 1;

            rc = VectorAppend(&self->inc, NULL, temp);
            if (rc) {
                free(temp);
                break;
            }
            if (ch == '\0')
                break;
        }
    }

    return rc;
}


/* ParseText_v1
 *  parse schema text
 *  add productions to existing schema
 *
 *  "name" [ IN, NULL OKAY ] - optional name
 *  representing text, e.g. filename
 *
 *  "text" [ IN ] and "bytes" [ IN ] - input buffer of text
 */
static
rc_t
VSchemaParseTextInt_v1 ( VSchema *self, const char *name, const char *text, size_t bytes )
{
    KTokenText tt;
    KTokenSource src;
    String str, path;
    rc_t rc;

    if ( name == NULL || name [ 0 ] == 0 )
    {
        CONST_STRING ( & path, "<unnamed>" );
    }
    else
    {
        StringInitCString ( & path, name );
    }

    StringInit ( & str, text, bytes, string_len ( text, bytes ) );
    KTokenTextInit ( & tt, & str, & path );
    KTokenSourceInit ( & src, & tt );

    rc = schema ( & src, self );
    if (rc == 0)
    {
        PARSE_DEBUG( ("Parsed schema v1 from %s\n", name) );
    }
    else
    {
        PARSE_DEBUG( ("Failed to parse v1 schema from %s\n", name) );
    }

    return rc;
}

static
rc_t
VSchemaParseTextInt_v2 ( VSchema *self, const char *name, const char *text, size_t bytes )
{
    //printf("VSchemaParseTextInt_v2:\n%.*s\n", (int)bytes, text);
    if ( VSchemaParse_v2 ( self, text, bytes ) )
    {
        PARSE_DEBUG( ("Parsed schema v2 from %s\n", name) );
        return 0;
    }

    PARSE_DEBUG( ("Failed to parse v2 schema from %s\n", name) );
    return RC ( rcVDB, rcSchema, rcOpening, rcSchema, rcFailed );
}

static
rc_t
VSchemaParseTextInt ( VSchema *self, const char *name, const char *text, size_t bytes )
{
    KConfig * kfg;
    rc_t rc = KConfigMake ( & kfg, NULL );
    if ( rc == 0 )
    {
        uint8_t version;
        rc = KConfigGetSchemaParserVersion( kfg , & version );
        if ( rc == 0 )
        {
            switch (version)
            {
            case 1:
                rc = VSchemaParseTextInt_v1 ( self, name, text, bytes );
                break;
            case 2:
                rc = VSchemaParseTextInt_v2 ( self, name, text, bytes );
                break;
            default:
                rc = RC ( rcVDB, rcSchema, rcParsing, rcFileFormat, rcUnsupported );
                break;
            }
        }
    }
    KConfigRelease ( kfg );
    return rc;
}

LIB_EXPORT rc_t CC VSchemaParseText ( VSchema *self, const char *name,
    const char *text, size_t bytes )
{
    rc_t rc;

    PARSE_DEBUG( ("VSchemaParseText %s\n", name) );

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcSelf, rcNull );
    else if ( bytes == 0 )
        rc = 0;
    else if ( text == NULL )
        rc = RC ( rcVDB, rcSchema, rcParsing, rcParam, rcNull );
    else
    {
        rc = VSchemaParseTextInt ( self, name, text, bytes);
    }

    return rc;
}


/* ParseTextCallback
 *  parse schema text
 *  add productions to existing schema
 *
 *  "name" [ IN, NULL OKAY ] - optional name
 *  representing text, e.g. filename
 *
 *  "fill" [ IN ] and "data" [ IN, OPAQUE ] - for filling buffer
 */
rc_t VSchemaParseTextCallback ( VSchema *self, const char *name,
    rc_t ( CC * fill ) ( void *self, KTokenText *tt, size_t save ), void *data )
{
    KConfig * kfg;
    rc_t rc = KConfigMake ( & kfg, NULL );
    if ( rc == 0 )
    {
        uint8_t version;

        rc = KConfigGetSchemaParserVersion( kfg , & version );
        if ( rc == 0 )
        {
            switch (version)
            {
            case 1:
                {
                    KTokenText tt;
                    KTokenSource src;

                    KTokenTextInitCString ( & tt, "", name );
                    tt . read = fill;
                    tt . data = data;

                    KTokenSourceInit ( & src, & tt );
                    /*NB: this invokes v1 parser, even though the parent schema may have been
                        processed by v2 parser. The reason is the possibility of v0 constructs
                        coming from older objects */
                    rc = schema ( & src, self );
                }
                break;
            case 2:
                {
                    KTokenText tt;
                    rc = fill ( data, & tt, 0 );
                    rc = VSchemaParseTextInt_v2 ( self, name, tt.str.addr, tt.str.size );
                }
                break;
            default:
                rc = RC ( rcVDB, rcSchema, rcParsing, rcFileFormat, rcUnsupported );
                break;
            }
        }
    }
    KConfigRelease ( kfg );
    return rc;

}


/* OpenFile
 *  opens a file, using include paths
 */
rc_t CC VSchemaTryOpenFile ( const VSchema *self, const KDirectory *dir, const KFile **fp,
    char *path, size_t path_max, const char *name, va_list args )
{
    rc_t rc;

    rc = KDirectoryVResolvePath ( dir, true, path, path_max, name, args );

    if ( rc == 0 )
    {
        PARSE_DEBUG( ("VSchemaTryOpenFile: path = '%s'\n", path) );
        /* try to find file in already opened list */
        if ( BSTreeFind ( & self -> paths, path, VIncludedPathCmp ) != NULL )
        {
            PARSE_DEBUG( ("VSchemaTryOpenFile: '%s' already open\n", path) );
            * fp = NULL;
            return 0;
        }
    }
    else
    {
        PARSE_DEBUG( ("VSchemaTryOpenFile:  failed\n", path) );
    }

    if ( rc == 0 )
        rc = KDirectoryOpenFileRead ( dir, fp, "%s", path );

    return rc;
}

static rc_t VSchemaOpenFile_1 ( const VSchema *const self, const KFile **fp, KDirectory const *ndir,
                         char *path, size_t path_max, const char *name, va_list args )
{
    const VSchema *schema = self;

    /* Loop over the list of schema objects */
    for ( schema = self; schema != NULL; schema = schema -> dad ) {
        Vector const *const vec = &schema->inc;
        uint32_t i = VectorStart(vec);
        uint32_t const end = i + VectorLength(vec);

        /* Loop over the list of include paths */
        for ( ; i < end; ++i) {
            char const *const dirname = VectorGet(vec, i);

            if ( dirname != NULL )
            {
                KDirectory const *dir = NULL;
                rc_t rc = KDirectoryOpenDirRead(ndir, &dir, false, dirname);

                if (rc != 0)
                    continue;
#if _DEBUGGING
                PARSE_DEBUG( ("VSchemaOpenFile looking in '%s'\n", dirname ) );
#endif
                *fp = NULL;
                { /* since we are in a loop, it is not safe to pass our copy;
                   * we MUST make a copy and pass that
                   */
                    va_list copy;
                    va_copy(copy, args);
                    rc = VSchemaTryOpenFile(self, dir, fp, path, path_max, name, copy);
                    va_end(copy); /* every va_copy needs a matching va_end */
                }
                KDirectoryRelease(dir);
                if (rc == 0 || GetRCState(rc) != rcNotFound)
                    return rc;
            }
        }
    }

    return RC ( rcVDB, rcSchema, rcOpening, rcPath, rcNotFound );
}

rc_t CC VSchemaOpenFile ( const VSchema *self, const KFile **fp,
    char *path, size_t path_max, const char *name, va_list args )
{
    KDirectory *ndir = NULL;
    rc_t rc = -1;

#if _DEBUGGING
    {
        char full_name [4096];
        size_t num_writ;
        va_list cpy_args;
        va_copy ( cpy_args, args );
        string_vprintf ( full_name, sizeof( full_name ), &num_writ, name, cpy_args );
        va_end ( cpy_args );
        PARSE_DEBUG( ("VSchemaOpenFile('%s')\n", full_name) );
    }
#endif

    assert(fp != NULL);

    *fp = NULL;

    rc = KDirectoryNativeDir(&ndir);
    assert(rc == 0);
    if (rc)
        return rc;

    rc = VSchemaOpenFile_1(self, fp, ndir, path, path_max, name, args);
    KDirectoryRelease(ndir);

    return rc;
}

/* ParseFile
 *  parse schema file
 *  add productions to existing schema
 *
 *  "name" [ IN ] - filename, absolute or valid relative to
 *  working directory or within an include path
 */
LIB_EXPORT rc_t CC VSchemaVParseFile ( VSchema *self, const char *name, va_list args )
{
    rc_t rc = 0;

    assert(self != NULL);
    assert(name != NULL);
    assert(name[0] != '\0');
    if ( self == NULL )
        return RC ( rcVDB, rcSchema, rcOpening, rcSelf, rcNull );
    if ( name == NULL )
        return RC ( rcVDB, rcSchema, rcOpening, rcPath, rcNull );
    if ( name [ 0 ] == 0 )
        return RC ( rcVDB, rcSchema, rcOpening, rcPath, rcEmpty );
    else
    {
        const KFile *f = NULL;
        char path [ 4096 ];

        /* open file using include paths */
        { /* we might need to use args again, so make a copy and pass that */
            va_list copy;
            va_copy(copy, args);
            rc = VSchemaOpenFile ( self, & f, path, sizeof path, name, copy );
            va_end(copy);
        }
        /* try to open the file according to current directory */
        if ( rc != 0 )
        {
            KDirectory *wd;
            rc = KDirectoryNativeDir ( & wd );
            if ( rc == 0 )
            {
                /* args is not used again */
                rc = VSchemaTryOpenFile ( self, wd, & f, path, sizeof path, name, args );
                if ( rc == 0 )
                {
                    PARSE_DEBUG( ("VSchemaTryOpenFile = '%s'\n", path) );
                }
                KDirectoryRelease ( wd );
            }
        }
        else
        {
            PARSE_DEBUG( ("VSchemaOpenFile = '%s'\n", path) );
        }

        /* if the file was opened... */
        if ( rc == 0 && f != NULL )
        {
            const KMMap *mm;
            rc = KMMapMakeRead ( & mm, f );
            if ( rc == 0 )
            {
                size_t size;
                const void *addr;
                rc = KMMapAddrRead ( mm, & addr );
                if ( rc == 0 )
                    rc = KMMapSize ( mm, & size );
                if ( rc == 0 )
                {
                    rc = VIncludedPathMake ( & self -> paths, & self -> file_count, path );
                    if ( rc == 0 )
                    {
                        PARSE_DEBUG( ("VSchemaVParseFile %s\n", path) );
                        rc = VSchemaParseTextInt ( self, path, addr, size );
                    }
                }

                KMMapRelease ( mm );
            }

            KFileRelease ( f );
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaParseFile ( VSchema *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaVParseFile ( self, name, args );
    va_end ( args );

    return rc;
}


/* ResolveFmtdecl
 *  converts a fmtdecl string to fmt id, type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved fmtdecl
 *
 *  "fmtdecl" [ IN ] - type declaration
 */
rc_t CC VSchemaVResolveFmtdecl ( const VSchema *self,
    VFormatdecl *resolved, const char *decl, va_list args )
{
    rc_t rc;

    if ( resolved == NULL )
        rc = RC ( rcVDB, rcSchema, rcResolving, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcSchema, rcResolving, rcSelf, rcNull );
        else if ( decl == NULL )
            rc = RC ( rcVDB, rcSchema, rcResolving, rcType, rcNull );
        else if ( decl [ 0 ] == 0 )
            rc = RC ( rcVDB, rcSchema, rcResolving, rcType, rcEmpty );
        else
        {
            char buffer [ 256 ];
            int len = vsnprintf ( buffer, sizeof buffer, decl, args );
            if ( len < 0 || len >= sizeof buffer )
                rc = RC ( rcVDB, rcSchema, rcResolving, rcType, rcExcessive );
            else
            {
                KSymTable tbl;
                rc = init_symtab ( & tbl, self );
                if ( rc == 0 )
                {
                    KToken t;
                    KTokenText tt;
                    KTokenSource src;
                    String text, path;

                    SchemaEnv env;
                    SchemaEnvInit ( & env, EXT_SCHEMA_LANG_VERSION );

                    StringInit ( & text, buffer, len, string_len ( buffer, len ) );
                    CONST_STRING ( & path, "VSchemaResolve" );
                    KTokenTextInit ( & tt, & text, & path );
                    KTokenSourceInit ( & src, & tt );

                    rc = fmtspec ( & tbl, & src, next_token ( & tbl, & src, & t ),
                        & env, self, resolved );

                    KSymTableWhack ( & tbl );

                    if ( rc == 0 )
                        return 0;
                }
            }
        }

        resolved -> td . type_id = 0;
        resolved -> td . dim = 0;
        resolved -> fmt = 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaResolveFmtdecl ( const VSchema *self,
    VFormatdecl *resolved, const char *fmtdecl, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, fmtdecl );
    rc = VSchemaVResolveFmtdecl ( self, resolved, fmtdecl, args );
    va_end ( args );

    return rc;
}


/* ResolveTypespec
 *  converts a typespec string to fmt id, type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved fmtdecl
 *
 *  "typespec" [ IN ] - type specification
 */
LIB_EXPORT rc_t CC VSchemaVResolveTypespec ( const VSchema *self,
    VTypedecl *resolved, const char *decl, va_list args )
{
    rc_t rc;

    if ( resolved == NULL )
        rc = RC ( rcVDB, rcSchema, rcResolving, rcParam, rcNull );
    else
    {
        VFormatdecl fd;
        rc = VSchemaVResolveFmtdecl ( self, & fd, decl, args );
        if ( rc == 0 )
        {
            if ( fd . fmt != 0 )
                rc = RC ( rcVDB, rcSchema, rcResolving, rcType, rcIncorrect );
            else
            {
                * resolved = fd . td;
                return 0;
            }
        }

        resolved -> type_id = 0;
        resolved -> dim = 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaResolveTypespec ( const VSchema *self,
    VTypedecl *resolved, const char *typespec, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, typespec );
    rc = VSchemaVResolveTypespec ( self, resolved, typespec, args );
    va_end ( args );

    return rc;
}

/* ResolveTypedecl
 *  converts a typedecl string to type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved type declaration
 *
 *  "typedecl" [ IN ] - type declaration
 */
LIB_EXPORT rc_t CC VSchemaVResolveTypedecl ( const VSchema *self,
    VTypedecl *resolved, const char *decl, va_list args )
{
    rc_t rc = VSchemaVResolveTypespec ( self, resolved, decl, args );
    if ( rc == 0 )
    {
        if ( ( resolved -> type_id & 0xC0000000 ) == 0 )
            return 0;

        rc = RC ( rcVDB, rcSchema, rcResolving, rcType, rcIncorrect );

        resolved -> type_id = 0;
        resolved -> dim = 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaResolveTypedecl ( const VSchema *self,
    VTypedecl *resolved, const char *typedecl, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, typedecl );
    rc = VSchemaVResolveTypedecl ( self, resolved, typedecl, args );
    va_end ( args );

    return rc;
}


/* IncludeFiles
 *  generates a list of paths to include file
 */
static
bool CC VIncludedPathList ( BSTNode *n, void *list )
{
    const VIncludedPath *p = ( const VIncludedPath* ) n;
    rc_t rc = VNamelistAppend ( list, p -> path );
    return ( rc != 0 ) ? true : false;
}

LIB_EXPORT rc_t CC VSchemaIncludeFiles ( const VSchema *self, const KNamelist **listp )
{
    rc_t rc;
    if ( listp == NULL )
        rc = RC ( rcVDB, rcSchema, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcSchema, rcListing, rcSelf, rcNull );
        else
        {
            /* create namelist */
            VNamelist *list;
            rc = VNamelistMake ( & list, self -> file_count );
            if ( rc == 0 )
            {
                /* create a shallow copy */
                BSTree paths = self -> paths;

                /* sort by order rather than by path */
                BSTreeResort ( & paths, VIncludedPathSortByOrder );

                /* add paths to list */
                if ( BSTreeDoUntil ( & paths, false, VIncludedPathList, list ) )
                    rc = RC ( rcVDB, rcSchema, rcListing, rcMemory, rcExhausted );

                /* restore path order */
                BSTreeResort ( & paths, VIncludedPathSort );
                assert ( paths . root == self -> paths . root );

                /* cast list, creating a new reference */
                if ( rc == 0 )
                    rc = VNamelistToNamelist ( list, (KNamelist **)listp );

                /* discard original reference */
                VNamelistRelease ( list );

                /* return list */
                if ( rc == 0 )
                    return 0;
            }
        }

        * listp = NULL;
    }
    return rc;
}


/* ListLegacyTables - PRIVATE
 *  scan all tables defined in schema
 *  return the fully-qualified and versioned name of each legacy SRA table
 */
typedef struct STableFindData STableFindData;
struct STableFindData
{
    VNamelist *list;
    rc_t rc;
};

static
bool CC STableTestUntyped ( void *item, void *data )
{
    const STable *stbl = ( const void* ) item;
    STableFindData *pb = data;

    if ( stbl -> untyped != NULL )
    {
        char buffer [ 1024 ];

        /* found a legacy table */
        pb -> rc = string_printf ( buffer, sizeof buffer, NULL, "%N#%V", stbl -> name, stbl -> version );
        if ( pb -> rc != 0 )
        {
            pb -> rc = RC ( rcVDB, rcSchema, rcListing, rcName, rcExcessive );
            return true;
        }

        /* add it to the vector */
        pb -> rc = VNamelistAppend ( pb -> list, buffer );
        if ( pb -> rc != 0 )
            return true;
    }

    return false;
}

static
bool VSchemaFindUntyped ( const VSchema *self, STableFindData *pb )
{
    if ( self -> dad )
    {
        if ( VSchemaFindUntyped ( self -> dad, pb ) )
            return true;
    }

    return VectorDoUntil ( & self -> tbl, false, STableTestUntyped, pb );
}

LIB_EXPORT rc_t CC VSchemaListLegacyTables ( const VSchema *self, KNamelist **listp )
{
    rc_t rc;

    if ( listp == NULL )
        rc = RC ( rcVDB, rcSchema, rcListing, rcParam, rcNull );
    else
    {
        * listp = NULL;

        if ( self == NULL )
            rc = RC ( rcVDB, rcSchema, rcListing, rcSelf, rcNull );
        else
        {
            STableFindData pb;
            rc = VNamelistMake ( & pb . list, 16 );
            if ( rc == 0 )
            {
                pb . rc = 0;
                if ( VSchemaFindUntyped ( self, & pb ) )
                    rc = pb . rc;
                else
                    rc = VNamelistToNamelist ( pb . list, listp );

                VNamelistRelease ( pb . list );
            }
        }
    }

    return rc;
}

/*--------------------------------------------------------------------------
 * VDBManager
 */

LIB_EXPORT rc_t CC VDBManagerMakeSchema ( const VDBManager *self, VSchema **schema )
{
    rc_t rc;
    if ( schema == NULL )
        rc = RC ( rcVDB, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcConstructing, rcSelf, rcNull );
        else
        {
            rc = VSchemaMake ( schema, self -> schema );
            if ( rc == 0 )
                return rc;
        }

        * schema = NULL;
    }

    return rc;
}



/*--------------------------------------------------------------------------
 * VSchemaRuntimeTable
 *  an object that represents a table ( or sub-table ) created at runtime
 *  that can be used to create a VTable object
 */
struct VSchemaRuntimeTable
{
    VSchema *schema;
    KDataBuffer text;
    size_t bytes;
};


/* Print
 *  print schema text into buffer
 */
static
rc_t VSchemaRuntimeTablePrint ( VSchemaRuntimeTable *self, const char *fmt, ... )
{
    int status;
    va_list args;

    char *buffer;
    size_t bsize;

    /* make sure there's a bunch of room */
    if ( self -> text . elem_count - self -> bytes < 4096 )
    {
        rc_t rc = KDataBufferResize ( & self -> text, self -> text . elem_count + 4096 );
        if ( rc != 0 )
            return rc;
    }

    /* the output buffer for this print */
    buffer = self -> text . base;
    bsize = self -> text . elem_count - self -> bytes;

    va_start ( args, fmt );
    status = vsnprintf ( & buffer [ self -> bytes ], bsize, fmt, args );
    va_end ( args );

    if ( status < 0 || status >= bsize )
        return RC ( rcVDB, rcSchema, rcConstructing, rcParam, rcExcessive );

    self -> bytes += status;
    return 0;
}


/* Make
 *  make a table in the schema provided
 *
 *  "tbl" [ OUT ] - return parameter for new schema table object
 *
 *  "typename" [ IN ] - NUL terminated string giving desired typename of table
 *   may include version spec
 *
 *  "supertype_spec" [ IN, NULL OKAY ] - optional supertype spec, giving
 *   either a single super-table type string or multiple comma-separated tables
 */
LIB_EXPORT rc_t CC VSchemaMakeRuntimeTable ( VSchema *self,
    VSchemaRuntimeTable **tblp, const char *type_name, const char *supertype_spec )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcSchema, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcSchema, rcConstructing, rcSelf, rcNull );
        else if ( type_name == NULL )
            rc = RC ( rcVDB, rcSchema, rcConstructing, rcType, rcNull );
        else if ( type_name [ 0 ] == 0 )
            rc = RC ( rcVDB, rcSchema, rcConstructing, rcType, rcEmpty );
        else
        {
            VSchemaRuntimeTable *tbl = malloc ( sizeof * tbl );
            if ( tbl == NULL )
                rc = RC ( rcVDB, rcSchema, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KDataBufferMakeBytes ( & tbl -> text, 4096 * 2 );
                if ( rc == 0 )
                {
                    rc = VSchemaAddRef ( self );
                    if ( rc == 0 )
                    {
                        const char *dflt_vers = "";
                        if ( strchr ( type_name, '#' ) == NULL )
                            dflt_vers = "#1.0";

                        tbl -> schema = self;
                        tbl -> bytes = 0;

                        /* open the table */
                        rc = VSchemaRuntimeTablePrint ( tbl, "table %s%s", type_name, dflt_vers );
                        if ( rc == 0 )
                        {
                            if ( supertype_spec != NULL && supertype_spec [ 0 ] != 0 )
                                rc = VSchemaRuntimeTablePrint ( tbl, " = %s", supertype_spec );
                            if ( rc == 0 )
                            {
                                rc = VSchemaRuntimeTablePrint ( tbl, "\n{\n" );
                                if ( rc == 0 )
                                {
                                    * tblp = tbl;
                                    return 0;
                                }
                            }
                        }

                        VSchemaRelease ( tbl -> schema );
                    }

                    KDataBufferWhack ( & tbl -> text );
                }

                free ( tbl );
            }
        }

        * tblp = NULL;
    }

    return rc;
}

/* Close
 *  close the schema table handle
 *  discards any uncomitted data
 */
LIB_EXPORT rc_t CC VSchemaRuntimeTableClose ( VSchemaRuntimeTable *self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcClosing, rcSelf, rcNull );
    else
    {
        rc = VSchemaRelease ( self -> schema );
        if ( rc == 0 )
        {
            KDataBufferWhack ( & self -> text );
            free ( self );
        }
    }

    return rc;
}


/* Commit
 *  commits table data
 *  closes object to further modification
 */
LIB_EXPORT rc_t CC VSchemaRuntimeTableCommit ( VSchemaRuntimeTable *self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcCommitting, rcSelf, rcNull );
    else
    {
        const char *data = self -> text . base;
        if ( self -> bytes == 0 )
            rc = RC ( rcVDB, rcSchema, rcCommitting, rcSelf, rcEmpty );
        else
        {
            rc = VSchemaRuntimeTablePrint ( self, "}\n" );
            if ( rc == 0 )
                rc = VSchemaParseText ( self -> schema, "runtime-table", data, self -> bytes );
            KDataBufferWhack ( & self -> text );
        }
    }

    return rc;
}


/* AddColumn
 *  add a simple column to the table
 *
 *  "td" [ IN ] - data type of column, obtained from schema object
 *
 *  "encoding" [ IN, NULL OKAY ] - optional compression/encoding type
 *  specified as a NUL-terminated string
 *
 *  "name" [ IN ] - NUL-terminated simple ASCII name
 *   conforming to column name requirements
 */
LIB_EXPORT rc_t CC VSchemaRuntimeTableVAddColumn ( VSchemaRuntimeTable *self,
    const VTypedecl *td, const char *encoding, const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcSelf, rcNull );
    else if ( td == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcType, rcNull );
    else if ( name == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcNull );
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcEmpty );
    else
    {
        char typedecl [ 256 ];
        rc = VTypedeclToText ( td, self -> schema, typedecl, sizeof typedecl );
        if ( rc == 0 )
        {
            rc = VSchemaRuntimeTablePrint ( self, "    extern column " );
            if ( rc == 0 )
            {
                if ( encoding != NULL && encoding [ 0 ] != 0 )
                    rc = VSchemaRuntimeTablePrint ( self, "%s ", encoding );
                else
                    rc = VSchemaRuntimeTablePrint ( self, "%s ", typedecl );
            }
            if ( rc == 0 )
                rc = VSchemaRuntimeTablePrint ( self, name, args );
            if ( rc == 0 )
                rc = VSchemaRuntimeTablePrint ( self, ";\n" );
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableAddColumn ( VSchemaRuntimeTable *self,
    const VTypedecl *td, const char *encoding, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaRuntimeTableVAddColumn ( self, td, encoding, name, args );
    va_end ( args );

    return rc;
}


/* AddTypedColumn
 *  Gerber versions of AddColumn
 *
 *  AddBooleanColumn - simple boolean column with default compression
 *                     provides both packed ( single bit ) and byte versions
 *  AddIntegerColumn - sized integer column with appropriate sign, size and compression
 *  AddFloatColumn   - sized floating point column with lossy compression
 *  AddAsciiColumn   - 8-bit ASCII text column with standard compression
 *  AddUnicodeColumn - UTF8, UTF16, or UTF32 text column with standard compression
 */
LIB_EXPORT rc_t CC VSchemaRuntimeTableVAddBooleanColumn ( VSchemaRuntimeTable *self, const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcNull );
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcEmpty );
    else
    {
        VTypedecl td;
        rc = VSchemaResolveTypedecl ( self -> schema, & td, "bool" );
        if ( rc == 0 )
            rc = VSchemaRuntimeTableVAddColumn ( self, & td, "bool_encoding", name, args );
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableAddBooleanColumn ( VSchemaRuntimeTable *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaRuntimeTableVAddBooleanColumn ( self, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableVAddIntegerColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, bool has_sign,  const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcNull );
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcEmpty );
    else
    {
        VTypedecl td;

        char typedecl [ 256 ];
        int status = snprintf ( typedecl, sizeof typedecl, "%c%u", has_sign ? 'I' : 'U', bits );
        if ( status < 0 || status >= sizeof typedecl )
            rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
        else
        {
            rc = VSchemaResolveTypedecl ( self -> schema, & td, "%s", typedecl );
            if ( rc != 0 && ( ( ( ( bits - 1 ) & bits ) != 0 ) || bits < 8 || bits > 64 ) )
            {
                status = snprintf ( typedecl, sizeof typedecl, "%c1 [ %u ]", ( bits < 4 ) ? 'B' : ( has_sign ? 'I' : 'U' ), bits );
                if ( status < 0 || status >= sizeof typedecl )
                    rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
                else
                    rc = VSchemaResolveTypedecl ( self -> schema, & td, "%s", typedecl );
            }
            if ( rc == 0 )
            {
                char encoding [ 256 ];

                status = snprintf ( encoding, sizeof encoding, "< %s > izip_encoding", typedecl );
                if ( status < 0 || status >= sizeof encoding )
                    rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
                else
                    rc = VSchemaRuntimeTableVAddColumn ( self, & td, encoding, name, args );
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableAddIntegerColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, bool has_sign,  const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaRuntimeTableVAddIntegerColumn ( self, bits, has_sign, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableVAddFloatColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, uint32_t significant_mantissa_bits,  const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcNull );
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcEmpty );
    else
    {
        VTypedecl td;

        char typedecl [ 256 ];
        int status = snprintf ( typedecl, sizeof typedecl, "F%u", bits );
        if ( status < 0 || status >= sizeof typedecl )
            rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
        else
        {
            rc = VSchemaResolveTypedecl ( self -> schema, & td, "%s", typedecl );
            if ( rc == 0 )
            {
                char encoding [ 256 ];

                if ( bits != 32 || significant_mantissa_bits >= bits )
                {
                    status = snprintf ( encoding, sizeof encoding,
                        "< %s > zip_encoding", typedecl );
                }
                else
                {
                    status = snprintf ( encoding, sizeof encoding,
                        "< %s > fzip_encoding < %u >", typedecl, significant_mantissa_bits );
                }
                if ( status < 0 || status >= sizeof encoding )
                    rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
                else
                    rc = VSchemaRuntimeTableVAddColumn ( self, & td, encoding, name, args );
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableAddFloatColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, uint32_t significant_mantissa_bits,  const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaRuntimeTableVAddFloatColumn ( self, bits, significant_mantissa_bits, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableVAddAsciiColumn ( VSchemaRuntimeTable *self, const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcNull );
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcEmpty );
    else
    {
        VTypedecl td;
        rc = VSchemaResolveTypedecl ( self -> schema, & td, "ascii" );
        if ( rc == 0 )
            rc = VSchemaRuntimeTableVAddColumn ( self, & td, "< ascii > zip_encoding", name, args );
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableAddAsciiColumn ( VSchemaRuntimeTable *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaRuntimeTableVAddAsciiColumn ( self, name, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC VSchemaRuntimeTableVAddUnicodeColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcSelf, rcNull );
    else if ( name == NULL )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcNull );
    else if ( name [ 0 ] == 0 )
        rc = RC ( rcVDB, rcSchema, rcUpdating, rcName, rcEmpty );
    else
    {
        VTypedecl td;

        char typedecl [ 256 ];
        int status = snprintf ( typedecl, sizeof typedecl, "utf%u", bits );
        if ( status < 0 || status >= sizeof typedecl )
            rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
        else
        {
            rc = VSchemaResolveTypedecl ( self -> schema, & td, "%s", typedecl );
            if ( rc == 0 )
            {
                char encoding [ 256 ];
                status = snprintf ( encoding, sizeof encoding,
                    "< %s > zip_encoding", typedecl );
                if ( status < 0 || status >= sizeof encoding )
                    rc = RC ( rcVDB, rcSchema, rcUpdating, rcBuffer, rcInsufficient );
                else
                    rc = VSchemaRuntimeTableVAddColumn ( self, & td, encoding, name, args );
            }
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC VSchemaRuntimeTableAddUnicodeColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VSchemaRuntimeTableVAddUnicodeColumn ( self, bits, name, args );
    va_end ( args );

    return rc;
}

/*******************************************************************************
 Private functions
 to access to internals of VSchema to allow them to be used in other projects.
*/

rc_t SDatabaseGetDad(const SDatabase * self, const struct SDatabase ** dad) {
    assert(self && dad); *dad = self->dad; return 0;
}

rc_t STableGetParents(const STable * self, const struct Vector ** parents) {
    assert(self && parents); *parents = &self->parents; return 0;
}

rc_t SViewGetParents(const SView * self, const struct Vector ** parents) {
    assert(self && parents); *parents = &self->parents; return 0;
}

rc_t VSchemaGetDb(const VSchema * self, const Vector ** db) {
    assert(self && db); *db = &self->db; return 0;
}

rc_t VSchemaGetTbl(const VSchema * self, const Vector ** tbl) {
    assert(self && tbl); *tbl = &self->tbl; return 0;
}

rc_t VSchemaGetView(const VSchema * self, const Vector ** view) {
    assert(self && view); *view = &self->view; return 0;
}

rc_t KSymbolNameWhack(KSymbolName * self) {
    if (self != NULL) {
        KSymbolNameElm *name = self->name;
        while (name != NULL) {
            KSymbolNameElm * next = name->next;
            memset(name, 0, sizeof *name);
            free(name);
            name = next;
        }

        memset(self, 0, sizeof *self);
        free(self);
    }

    return 0;
}

static KSymbolName * KSymbolMakeKSymbolName(const KSymbol * self,
    uint32_t version)
{
    KSymbolName * out = NULL;
    KSymbolNameElm * head = NULL;

    assert(self);

    for (; self; self = self->dad) {
        KSymbolNameElm * prev = NULL;
        const String * n = &self->name;
        assert(n);
        prev = calloc(1, sizeof *prev);
        if (prev == NULL)
            return NULL;

        prev->name = n;
        prev->next = head;
        head = prev;
    }

    out = calloc(1, sizeof *out);
    if (out == NULL)
        return NULL;

    out->name = head;
    out->version = version;
    return out;
}

rc_t SDatabaseMakeKSymbolName(const SDatabase * self,
    KSymbolName ** out)
{
    assert(self && out);
    *out = KSymbolMakeKSymbolName(self->name, self->version);
    if (*out == NULL)
        return RC(rcVDB, rcSchema, rcListing, rcMemory, rcExhausted);
    else
        return 0;
}

rc_t STableMakeKSymbolName(const STable * self, KSymbolName ** out) {
    assert(self && out);
    *out = KSymbolMakeKSymbolName(self->name, self->version);
    if (*out == NULL)
        return RC(rcVDB, rcSchema, rcListing, rcMemory, rcExhausted);
    else
        return 0;
}

rc_t SViewMakeKSymbolName(const SView * self, KSymbolName ** out) {
    assert(self && out);
    *out = KSymbolMakeKSymbolName(self->name, self->version);
    if (*out == NULL)
        return RC(rcVDB, rcSchema, rcListing, rcMemory, rcExhausted);
    else
        return 0;
}

/******************************************************************************/
