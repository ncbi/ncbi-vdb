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

#ifndef _h_schema_priv_
#define _h_schema_priv_

#ifndef _h_vdb_schema_
#include <vdb/schema.h>
#endif

#ifndef _h_vdb_xform_
#include <vdb/xform.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
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
 * SLVL - schema development level
 *  used to bring schema together in an orderly fashion
 *
 *  level 0: include, fmtdef, alias
 *
 *  level 1: version, typedef, dim, typeset
 *           const-expr, type-expr
 *           eval-uint-expr, eval-type-expr
 *
 *  level 2: const, eval-const-expr
 *
 *  level 3: function, schema-param-decl, fact-param-decl, func-param-decl
 *           indirect-type-expr, indirect-const-expr, prod-expr
 *
 *  level 4: script, return-stmt, prod-stmt
 *           cond-expr, cast-expr, func-expr, name-expr
 *
 *  level 5: physical
 *
 *  level 6: database, table
 *
 *  level 7: physical members
 *
 *  level 8: columns
 */
#ifndef SLVL
#define SLVL 8
#endif


/*--------------------------------------------------------------------------
 * EXAMINE_SCHEMA_CONTENTS
 *  debugging setting for examining contents and detecting inconsistencies
 */
#ifndef EXAMINE_SCHEMA_CONTENTS
#if _DEBUGGING
#define EXAMINE_SCHEMA_CONTENTS 0
#else
#define EXAMINE_SCHEMA_CONTENTS 0
#endif
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct String;
struct KSymbol;
struct KTokenText;
struct KMDataNode;
struct SFunction;
struct SDatabase;
struct VDBManager;
struct SExpression;
struct SDumper;


/*--------------------------------------------------------------------------
 * VCtxId
 *  id within a context
 *  made to be compatible with KSymbol.u.fwd
 */
typedef struct VCtxId VCtxId;
struct VCtxId
{
    uint32_t ctx;
    uint32_t id;
};

/* Cmp
 */
#define VCtxIdCmp( a, b ) \
    ( ( ( a ) -> ctx != ( b ) -> ctx ) ? \
      ( int ) ( a ) -> ctx - ( int ) ( b ) -> ctx : \
      ( int ) ( a ) -> id - ( int ) ( b ) -> id )


/*--------------------------------------------------------------------------
 * KSymbol
 */

/* Copy
 *  deep copy a single symbol
 */
rc_t KSymbolCopy ( BSTree *scope,
    struct KSymbol **cp, struct KSymbol const *orig );

/* CopyScope
 *  deep copy all symbols within a scope
 */
bool CC KSymbolCopyScope ( BSTNode *sym, void *scope );


/* ExamineContents
 *  not much to be done here
 */
#if EXAMINE_SCHEMA_CONTENTS >= 1
bool KSymbolExamineContents ( BSTNode *sym, void *rc );
#endif


/*--------------------------------------------------------------------------
 * KMDataNode
 */

/* FillSchema
 *  a schema buffer fill function that reads from KMDataNode
 *
 * NB - preset "add_v0" to false for normal operation, "true"
 *  if the buffer should be prefixed with "version 0;" on first read
 */
typedef struct KMDataNodeSchemaFillData KMDataNodeSchemaFillData;
struct KMDataNodeSchemaFillData
{
    struct KMDataNode const *node;
    size_t pos;
    char buff [ 64 * 1024 - 1 ];
    bool add_v0;
};

rc_t CC KMDataNodeFillSchema ( void *data, struct KTokenText *tt, size_t save );


/*--------------------------------------------------------------------------
 * VTypedecl
 * VFormatdecl
 */

/* CommonAncestor
 *  find a common ancestor between "self" and "peer"
 *  returns distance as sum of distances from each to "ancestor"
 */
bool VFormatdeclCommonAncestor ( const VFormatdecl *self, const VSchema *schema,
    const VFormatdecl *peer, VFormatdecl *ancestor, uint32_t distances [ 2 ] );


/*--------------------------------------------------------------------------
 * SNameOverload
 *  describes an overloaded name
 *  used to implement versioning 
 */
typedef struct SNameOverload SNameOverload;
struct SNameOverload
{
    /* symbolic name */
    struct KSymbol const *name;

    /* overloaded items */
    Vector items;

    /* contextual id */
    VCtxId cid;
};

/* Make
 */
rc_t SNameOverloadMake ( SNameOverload **name,
    struct KSymbol const *sym, uint32_t start, uint32_t len );

/* Copy
 */
rc_t SNameOverloadCopy ( BSTree *scope,
    SNameOverload **cp, const SNameOverload *orig );
rc_t SNameOverloadVectorCopy ( BSTree *scope,
    const Vector *src, Vector *dest );

/* Whack
 */
void CC SNameOverloadWhack ( void *self, void *ignore );


/*--------------------------------------------------------------------------
 * VSchema
 *  a symbol scope
 *  data types, blob formats, function and sub-schema declarations
 *  database and table declarations
 */
struct VSchema
{
    /* parent schema */
    const VSchema *dad;

    /* global scope for this schema */
    BSTree scope;

    /* paths of opened files */
    BSTree paths;

    /* include path - vector of KDirectory references
       ordered by precedence */
    Vector inc;

    /* alias names - owned by scope, but recorded for dumping */
    Vector alias;

    /* formats - ids start at 1, where 0 is reserved for NULL */
    Vector fmt;

    /* datatypes - ids start at 0, which is reserved for 'any' */
    Vector dt;

    /* typesets - ids start at 0x40000000 */
    Vector ts;

    /* parameterized types, declared within function
       signatures and defined within expressions
       ids start at 0x80000001, where 0x80000000
       is reserved for 'undefined' */
    Vector pt;

    /* symbolic constants */
    Vector cnst;

    /* extern and schema functions */
    Vector func;
    Vector fname;

    /* physical columns */
    Vector phys;
    Vector pname;

    /* tables */
    Vector tbl;
    Vector tname;

    /* databases */
    Vector db;
    Vector dname;

    KRefcount refcount;

    uint32_t file_count;

    /* number of indirect expressions
       these are uniquely identified place-holders
       for cursor-open binding of
       type and constant expressions */
    uint32_t num_indirect;
};


/* Make
 *  creates an empty schema
 */
rc_t VSchemaMake ( VSchema **sp, const VSchema *dad );


/* MakeIntrinsic
 *  creates an initial, intrinsic schema
 */
rc_t VSchemaMakeIntrinsic ( VSchema **intrinsic );


/* Attach
 * Sever
 */
VSchema *VSchemaAttach ( const VSchema *self );
rc_t VSchemaSever ( const VSchema *self );


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
    rc_t ( CC * fill ) ( void *self, struct KTokenText *tt, size_t save ),
    void *data );

/* Find
 *  generic object find within schema global scope
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
const void *VSchemaFind ( const VSchema *self,
    const SNameOverload **name, uint32_t *type,
    const char *expr, const char *ctx, bool dflt );


/* CacheIntrinsicTypeId
 *  for id-caching
 *
 *  reads id atomically from "cache" and returns if non-zero
 *
 *  otherwise retrieves type id by name lookup, and sets
 *  value atomically in "cache" before returning.
 */
uint32_t VSchemaCacheIntrinsicTypeId ( const VSchema *self,
    atomic32_t *cache, const char *typename );


/* Marking
 */
void VSchemaClearMark ( const VSchema *self );
void VSchemaClearPhysMark ( const VSchema *self );
void VSchemaTypeMark ( const VSchema *self, uint32_t type_id );


/* ToText
 *  converts some object to textual representation
 */
rc_t VSchemaToText ( struct VSchema const *self,
    char *buff, size_t bsize, size_t *num_writ, const char *fmt, ... );

#if _DEBUGGING
/* ListSymtab
 *  lists contents of symbol table
 */
void VSchemaListSymtab ( struct VSchema const *self );
#endif


/*--------------------------------------------------------------------------
 * SFormat
 *  describes a blob format
 */
typedef struct SFormat SFormat;
struct SFormat
{
    /* symbolic name */
    struct KSymbol const *name;

    /* superfmt - i.e. current fmt extends superfmt
       implying that super can be decoded by current */
    const SFormat *super;

    /* format id */
    uint32_t id;

    /* marking */
    bool marked;
};

/* Whack
 */
#define SFormatWhack VectMbrWhack

/* Find
 */
SFormat *VSchemaFindFmtid ( const VSchema *self, uint32_t id );

/* Mark
 */
void CC SFormatClearMark ( void *item, void *ignore );
void SFormatMark ( const SFormat *self );

/* Dump
 *  dump "fmtdef", dump object
 */
bool CC SFormatDefDump ( void *item, void *dumper );
rc_t SFormatDump ( const SFormat *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SDatatype
 *  describes a registered datatype
 */
enum
{
    /* these have been exported to <vdb/schema.h> */
    ddBool = vtdBool,
    ddUint = vtdUint,
    ddInt = vtdInt,
    ddFloat = vtdFloat,
    ddAscii = vtdAscii,
    ddUnicode = vtdUnicode
};

typedef struct SDatatype SDatatype;
struct SDatatype
{
    /* symbolic name */
    struct KSymbol const *name;

    /* supertype */
    const SDatatype *super;

    /* special byte-swap function */
    void ( * byte_swap ) ( void *dst, const void *src, uint64_t count );

    /* type id */
    uint32_t id;

    /* number of parent elements, from declaration:
       typedef super [ dim ] name; */
    uint32_t dim;

    /* element size, calculated as
       super -> size * dim */
    uint32_t size;

    /* data domain */
    uint16_t domain;

    /* marking */
    bool marked;
};

/* Whack
 */
#define SDatatypeWhack VectMbrWhack

/* IntrinsicDim
 */
uint32_t SDatatypeIntrinsicDim ( const SDatatype *self );

/* Find
 */
SDatatype *VSchemaFindTypeid ( const VSchema *self, uint32_t id );

/* Mark
 */
void CC SDatatypeClearMark ( void *item, void *ignore );
void SDatatypeMark ( const SDatatype *self );

/* Dump
 */
bool CC SDatatypeDefDump ( void *item, void *dumper );
rc_t SDatatypeDump ( const SDatatype *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * STypeset
 *  describes a set of typedecls
 */
typedef struct STypeset STypeset;
struct STypeset
{
    /* symbolic name */
    struct KSymbol const *name;

    /* typeset id */
    uint32_t id;

    /* marking */
    bool marked;

    /* fmtdecls */
    uint16_t count;
    VTypedecl td [ 1 ];
};

/* Whack
 */
#define STypesetWhack VectMbrWhack

/* Find
 */
STypeset *VSchemaFindTypesetid ( const VSchema *self, uint32_t id );

/* Mark
 */
void CC STypesetClearMark ( void *item, void *ignore );
void STypesetMark ( const STypeset *self, const VSchema *schema );

/* Dump
 */
bool CC STypesetDefDump ( void *item, void *dumper );
rc_t STypesetDump ( const STypeset *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SIndirectType
 *  a parameterized typespec
 */
typedef struct SIndirectType SIndirectType;
struct SIndirectType
{
    /* symbolic name */
    struct KSymbol const *name;

    /* index into binding vector */
    uint32_t type_id;

    /* formal type id */
    uint32_t id;

    /* parameter position */
    uint32_t pos;
};

/* Whack
 */
void CC SIndirectTypeWhack ( void *item, void *ignore );

/* Find
 */
SIndirectType *VSchemaFindITypeid ( const VSchema *self, uint32_t id );

/* Dump
 */
rc_t SIndirectTypeDump ( const SIndirectType *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SConstant
 *  describes a symbolic constant
 */
typedef struct SConstant SConstant;
struct SConstant
{
    /* symbolic name */
    struct KSymbol const *name;

    /* value */
    struct SExpression const *expr;

    /* type */
    VTypedecl td;

    /* id */
    uint32_t id;

    /* marking */
    bool marked;
};

/* Whack
 */
void CC SConstantWhack ( void *item, void *ignore );

/* Mark
 */
void CC SConstantClearMark ( void *item, void *ignore );
void SConstantMark ( const SConstant *self );

/* Dump
 *  dump "const", dump object
 */
bool CC SConstantDefDump ( void *item, void *dumper );
rc_t SConstantDump ( const SConstant *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SIndirectConst
 *  a parameterized constant
 */
typedef struct SIndirectConst SIndirectConst;
struct SIndirectConst
{
    /* symbolic name */
    struct KSymbol const *name;

    /* formal typedecl or NULL if function */
    struct SExpression const *td;

    /* index into binding vector */
    uint32_t expr_id;

    /* offset position from # of indirect types */
    uint32_t pos;
};

/* Whack
 */
void CC SIndirectConstWhack ( void * item, void * ignore );

/* Mark
 */
void CC SIndirectConstMark ( void * item, void * data );

/* Dump
 *  dump "const", dump object
 */
bool CC SIndirectConstDefDump ( void *item, void *dumper );
rc_t SIndirectConstDump ( const SIndirectConst *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SProduction
 *  a named production
 */
typedef struct SProduction SProduction;
struct SProduction
{
    /* symbolic name */
    struct KSymbol const *name;

    /* conditional expression */
    struct SExpression const *expr;

    /* formal fmtdecl */
    struct SExpression const *fd;

    /* contextual id */
    VCtxId cid;

    /* trigger production */
    bool trigger;

    /* control parameter */
    bool control;
};

/* Whack
 */
void CC SProductionWhack ( void * item, void * ignore );

/* Mark
 */
void CC SProductionMark ( void * itme, void * data );

/* Dump
 *  dump production
 */
bool CC SProductionDefDump ( void *item, void *dumper );
rc_t SProductionDump ( const SProduction *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SFormParmlist
 *  zero or more formal parameters
 */
typedef struct SFormParmlist SFormParmlist;
struct SFormParmlist
{
    Vector parms;
    uint16_t mand;
    uint16_t vararg;
};

/* Whack
 */
void SFormParmlistWhack ( SFormParmlist *self,
    void ( CC * pwhack ) ( void*, void* ) );

/* HasParams
 */
#define SFormParmlistHasParams( self ) \
    ( VectorLength ( & ( self ) -> parms ) != 0 || \
      ( self ) -> varag != 0 )

/* Mark
 */
void SFormParmlistMark ( const SFormParmlist *self,
    void ( CC * mark ) ( void*, void* ), const VSchema *schema );

/* Dump
 *  dump param list
 */
rc_t SFormParamlistDump ( const SFormParmlist *self, struct SDumper *d,
    bool ( CC * dump ) ( void*, void* ),
    const char *begin, const char *end, const char *empty );


/*--------------------------------------------------------------------------
 * SFunction
 */
typedef struct SFunction SFunction;
struct SFunction
{
    /* symbolic name */
    struct KSymbol const *name;

    /* return type expression - NULL for untyped or row-length */
    struct SExpression const *rt;

    /* pointer to additional information */
    union
    {
        struct
        {
            /* optional factory id */
            struct KSymbol const *fact;

        } ext;

        struct
        {
            /* return statement conditional expression */
            struct SExpression const *rtn;

            /* schema productions */
            Vector prod;

            uint32_t align;

        } script;

    } u;

    /* schema scope - i.e. indirect types and uint const */
    BSTree sscope;

    /* function scope - i.e. factory and func params, productions */
    BSTree fscope;

    /* factory parameters */
    SFormParmlist fact;

    /* function parameters */
    SFormParmlist func;

    /* schema types - indirect types, defined and owned by VSchema
       held here for positional correlation */
    Vector type;

    /* declared version */
    uint32_t version;

    /* schema parameters - indirect uint constants */
    Vector schem;

    /* function id */
    uint32_t id;

    /* script or extern function */
    bool script;

    /* marking */
    bool marked;

    /* validate, untyped or row-length function */
    bool validate;
    bool untyped;
    bool row_length;
};

/* Whack
 */
void SFunctionDestroy ( SFunction *self );
void CC SFunctionWhack ( void *self, void *ignore );

/* Cmp
 * Sort
 */
int CC SFunctionCmp ( const void *item, const void *n );
int CC SFunctionSort ( const void *item, const void *n );

/* Bind
 *  perform schema and factory param substitution
 *  returns prior param values
 */
rc_t SFunctionBindSchemaParms ( const SFunction *self,
    Vector *prior, const Vector *subst, Vector *cx_bind );
rc_t SFunctionBindFactParms ( const SFunction *self,
    Vector *parms, Vector *prior, const Vector *subst, Vector *cx_bind );

/* Rest-ore
 *  restore schema and factory param substitution
 *  destroys prior param vector
 */
void SFunctionRestSchemaParms ( const SFunction *self, Vector *prior, Vector *cx_bind );
void SFunctionRestFactParms ( const SFunction *self, Vector *prior, Vector *cx_bind );

/* Mark
 */
void CC SFunctionClearMark ( void * self, void * ignore );
void CC SFunctionMark ( void * item, void * data );
void SFuncNameMark ( const SNameOverload *self, const VSchema *schema );

/* Dump
 */
rc_t SFunctionDeclDumpSchemaParms ( const SFunction *self, struct SDumper *d );
rc_t SFunctionDeclDumpFactParms ( const SFunction *self, struct SDumper *d );
bool CC SFunctionDeclDump ( void *item, void *dumper );
rc_t SFunctionBodyDump ( const SFunction *self, struct SDumper *d );
rc_t SFunctionDump ( const SFunction *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SPhysical
 *  physical column declaration
 */
typedef struct SPhysical SPhysical;
struct SPhysical
{
    /* symbolic name */
    struct KSymbol const *name;

    /* optional function for determining row length */
    const SFunction *row_length;

    /* encoding scripts */
    SFunction encode, decode;

    /* column type */
    struct SExpression const *td;

    /* required version */
    uint32_t version;

    /* physical decl id */
    uint32_t id;

    /* marking */
    bool marked;

    /* if only decode side exists */
    bool read_only;

    /* v0 blobs have no headers */
    bool no_hdr;
};

/* Whack
 */
void CC SPhysicalWhack ( void *self, void *ignore );

/* Cmp
 * Sort
 */
int CC SPhysicalCmp ( const void *item, const void *n );
int CC SPhysicalSort ( const void *item, const void *n );

/* Bind
 *  perform schema and factory param substitution
 *  returns prior param values
 */
rc_t SPhysicalBindSchemaParms ( const SPhysical *self,
    Vector *prior, const Vector *subst, Vector *cx_bind );
rc_t SPhysicalBindFactParms ( const SPhysical *self,
     Vector *parms, Vector *prior, const Vector *subst, Vector *cx_bind );

/* Rest-ore
 *  restore schema and factory param substitution
 *  destroys prior param vector
 */
void SPhysicalRestSchemaParms ( const SPhysical *self, Vector *prior, Vector *cx_bind );
void SPhysicalRestFactParms ( const SPhysical *self, Vector *prior, Vector *cx_bind );

/* Mark
 */
void CC SPhysicalClearMark ( void * self, void * ignore );
void CC SPhysicalMark ( void * item, void * data );
void SPhysNameMark ( const SNameOverload *self, const VSchema *schema );

/* Dump
 */
bool CC SPhysicalDefDump ( void *self, void *data );
rc_t SPhysicalDump ( const SPhysical *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * STable
 *  table declaration
 */
typedef struct STable STable;
struct STable
{
    /* symbolic name */
    struct KSymbol const *name;

    /* optional function for determining table type */
    const SFunction *untyped;

    /* optional expression for limiting all columns' blob sizes */
    struct SExpression const *limit;

    /* optional default view declaration */
    const String *dflt_view;

    /* scope */
    BSTree scope;

    /* parents */
    Vector parents;

    /* overrides ( inherited virtual productions )
       contents are grouped by introducing parent */
    Vector overrides;

    /* columns */
    Vector col;
    Vector cname;

    /* explicitly declared physical column members */
    Vector phys;

    /* assignment statements */
    Vector prod;

    /* introduced virtual ( undefined ) productions
       contents are unowned KSymbol pointers */
    Vector vprods;

    /* owned KSymbols that are not in scope */
    Vector syms;

    /* source file & line */
    String src_file;
    uint32_t src_line;

    /* required version */
    uint32_t version;

    /* table id */
    uint32_t id;

    /* marking */
    bool marked;
};

/* Whack
 */
void CC STableWhack ( void *self, void *ignore );

/* CloneExtend
 *  creates an initially transparent table extension
 *  used by cursor to permit addition of implicit productions
 */
rc_t STableCloneExtend ( const STable *self, STable **clone, VSchema *schema );

/* Cmp
 * Sort
 */
int CC STableCmp ( const void *item, const void *n );
int CC STableSort ( const void *item, const void *n );

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
    uint32_t *type, const char *expr, const char *ctx, bool dflt );


/* FindOverride
 *  finds an inherited or introduced overridden symbol
 */
struct KSymbol *STableFindOverride ( const STable *self, const VCtxId *cid );

/* FindOrdAncestor
 *  finds a parent or grandparent by order
 */
const STable *STableFindOrdAncestor ( const STable *self, uint32_t i );

/* Mark
 */
void CC STableClearMark ( void *self, void *ignore );
/*
void CC STableMark ( const STable *self, const VSchema *schema );
*/
void STableNameMark ( const SNameOverload *self, const VSchema *schema );

/* Dump
 *  dump "table" { }
 */
bool CC STableDefDump ( void *self, void *dumper );
rc_t STableDump ( const STable *self, struct SDumper *d );

rc_t VSchemaDumpTableName ( const VSchema *self, uint32_t mode, const STable *stbl,
    rc_t ( CC * flush ) ( void *dst, const void *buffer, size_t bsize ), void *dst );
rc_t VSchemaDumpTableDecl ( const VSchema *self, uint32_t mode, const STable *stbl,
    rc_t ( CC * flush ) ( void *dst, const void *buffer, size_t bsize ), void *dst );


/*--------------------------------------------------------------------------
 * SColumn
 *  column declaration
 */
typedef struct SColumn SColumn;
struct SColumn
{
    /* symbolic name */
    struct KSymbol const *name;

    /* read productions */
    struct SExpression const *read;

    /* validation productions */
    struct SExpression const *validate;

    /* page size limit */
    struct SExpression const *limit;

    /* physical type expression */
    struct SExpression const *ptype;

    /* typedecl */
    VTypedecl td;

    /* contextual id */
    VCtxId cid;

    /* if true, this column is default for name */
    bool dflt;

    /* if read-only, there must be a read expression */
    bool read_only;

    /* if no read or validate expression exists
       and no corresponding physical member exists */
    bool simple;
};

/* Whack
 */
void CC SColumnWhack ( void *item, void *ignore );

/* Cmp
 *  compare a const VTypedecl* against const SColumn*
 * Sort
 *  compare two const SColumn*
 */
int CC SColumnCmp ( const void *item, const void *n );
int CC SColumnSort ( const void *item, const void *n );


/* ImplicitColMember
 *  adds an implicit column member of simple or incomplete type
 *
 *  "cname" [ IN ] - column name
 *
 *  "pname" [ IN ] - name of physical column
 */
rc_t STableImplicitColMember ( STable *self,
    struct String const *cname, struct String const *pname );

/* Dump
 */
bool CC SColumnDefDump ( void *item, void *dumper );
rc_t SColumnDump ( const SColumn *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SPhysMember
 *  physical member
 */
typedef struct SPhysMember SPhysMember;
struct SPhysMember
{
    /* symbolic name */
    struct KSymbol const *name;

    /* column type expression */
    struct SExpression const *type;

    /* assignment expression */
    struct SExpression const *expr;

    /* column datatype */
    VTypedecl td;

    /* contextual id */
    VCtxId cid;

    /* if static, it is implemented in table metadata */
    bool stat;

    /* it simple, the member was implicitly declared */
    bool simple;
};

/* Whack
 */
void CC SPhysMemberWhack ( void *item, void *ignore );

/* ImplicitPhysMember
 *  adds an implicit physical member of simple or incomplete type
 *
 *  "td" [ IN, NULL OKAY ] - type of member, if known
 *
 *  "sym" [ IN, NULL OKAY ] - optional symbol to be overridden
 *
 *  "name" [ IN ] - name of new symbol to be created if "sym" is NULL
 */
rc_t STableImplicitPhysMember ( STable *self, const VTypedecl *td,
    struct KSymbol *sym, struct String const *name );

/* Mark
 */
/*
void CC SPhysMemberMark ( const SPhysMember *self, const VSchema *schema );
*/

/* Dump
 */
bool CC SPhysMemberDefDump ( void *item, void *dumper );
rc_t SPhysMemberDump ( const SPhysMember *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SDatabase
 *  database declaration
 */
typedef struct SDatabase SDatabase;
struct SDatabase
{
    /* symbolic name */
    struct KSymbol const *name;

    /* scope */
    BSTree scope;

    /* parents */
    const SDatabase *dad;

    /* databases */
    Vector db;

    /* tables */
    Vector tbl;

    /* required version */
    uint32_t version;

    /* database id */
    uint32_t id;

    /* marking */
    bool marked;
};

/* Whack
 */
void CC SDatabaseWhack ( void *self, void *ignore );

/* Cmp
 * Sort
 */
int CC SDatabaseCmp ( const void *item, const void *n );
int CC SDatabaseSort ( const void *item, const void *n );

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
const void *SDatabaseFind ( const SDatabase *self,
    const VSchema *schema, const SNameOverload **name,
    uint32_t *type, const char *expr, const char *ctx );

/* Mark
 */
void CC SDatabaseClearMark ( void *self, void *ignore );
void CC SDatabaseMark ( void * item, void * data );
void CC SDatabaseNameMark ( const SNameOverload *self, const VSchema *schema );

/* Dump
 *  dump "database" { }
 */
bool CC SDatabaseDefDump ( void *self, void *dumper );
rc_t SDatabaseDump ( const SDatabase *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * STblMember
 *  table member
 */
typedef struct STblMember STblMember;
struct STblMember
{
    /* symbolic name */
    struct KSymbol const *name;

    /* table declaration */
    const STable *tbl;

    /* contextual id */
    VCtxId cid;

    /* is a template */
    bool tmpl;
};

/* Whack
 */
#define STblMemberWhack VectMbrWhack

void CC STableMark ( void * item, void * data );

/* Dump
 */
bool CC STblMemberDefDump ( void *item, void *dumper );
rc_t STblMemberDump ( const STblMember *self, struct SDumper *d );


/*--------------------------------------------------------------------------
 * SDBMember
 *  sub-db member
 */
typedef struct SDBMember SDBMember;
struct SDBMember
{
    /* symbolic name */
    struct KSymbol const *name;

    /* database declaration */
    const SDatabase *db;

    /* contextual id */
    VCtxId cid;

    /* is a template */
    bool tmpl;
};

/* Whack
 */
#define SDBMemberWhack VectMbrWhack

/* Dump
 */
bool CC SDBMemberDefDump ( void *item, void *dumper );
rc_t SDBMemberDump ( const SDBMember *self, struct SDumper *d );


#ifdef __cplusplus
}
#endif

#endif /* _h_schema_priv_ */
