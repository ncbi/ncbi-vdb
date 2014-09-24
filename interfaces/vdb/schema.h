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

#ifndef _h_vdb_schema_
#define _h_vdb_schema_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KNamelist;
struct VDBManager;


/*--------------------------------------------------------------------------
 * VSchema
 *  a virtual table schema
 *
 *  a schema is built from column, type and sub-schema declarations. The
 *  pseudo-BNF below gives a simplified description of the structure,
 *  with certain contextual constraints omitted.
 *
 *  at the global level, both read and write column declarations are permitted.
 *  type declarations are permitted at any level but always enter into global
 *  scope.
 *
 *  column names may be overloaded to allow for multiple data types under a
 *  single name. a column is identified by its ( name, datatype ) pair. the
 *  pairing ( name, NULL ) identifies the default type for name.
 *
 *  column declarations are intentionally created by assignment ( '=' ) or
 *  are updated with additional productions by appending ( '+=' ). this
 *  distinction is intended to prevent unintended consequences.
 *
 *  column productions come in the form of a simple column assignment, a
 *  data conversion function, or a sub-schema function. the simple column
 *  production takes a column name, the functions take some combination of
 *  constant and column name parameters.
 *
 *  a column assignment source ( column, function or sub-schema ) will have
 *  its own data type. when assigned directly, it must match the destination
 *  type either exactly or as the result of implicit type promotion. as an
 *  alternative, an explicit up or down cast ( or casts ) may be performed as
 *  an aid in source resolution.
 *
 *  read column declarations introduce a virtual column by ( name, datatype ).
 *  globally, they may be declared as "extern" to make them accessible via the
 *  API. extern columns may have any of their productions tagged as "default"
 *  to assign significance to the pair ( name, NULL ). when more than one type
 *  for a name is tagged as default, the most recently resolved is arbitrarily
 *  chosen. read source columns may be virtual or physical if the latter is
 *  within scope.
 *
 *  write column declarations introduce virtual columns by ( name, datatype ),
 *  and define physical column ( encoding, decoding ) bindings. virtual columns
 *  may be introduced as "extern" at global scope, where they have no source
 *  productions given that they take their input from the API. non-extern
 *  write columns are virtual and allow normal column productions with the
 *  exception that they may not be sourced by physical columns.
 *
 *  one special write column declaration introduces physical columns by
 *  name only ( name overloading cannot exist in the physical layer ) and
 *  requires a special production that binds together two sub-schema functions;
 *  one for encoding and the other for decoding.
 */
typedef struct VSchema VSchema;



/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VSchemaAddRef ( const VSchema *self );
VDB_EXTERN rc_t CC VSchemaRelease ( const VSchema *self );


/* Make
 *  create a schema object
 */
VDB_EXTERN rc_t CC VDBManagerMakeSchema ( struct VDBManager const *self, VSchema **schema );


/* AddIncludePath
 *  add an include path to schema for locating input files
 */
VDB_EXTERN rc_t CC VSchemaAddIncludePath ( VSchema *self, const char *path, ... );
VDB_EXTERN rc_t CC VSchemaVAddIncludePath ( VSchema *self, const char *path, va_list args );


/* ParseText
 *  parse schema text
 *  add productions to existing schema
 *
 *  "name" [ IN, NULL OKAY ] - optional name
 *  representing text, e.g. filename
 *
 *  "text" [ IN ] and "bytes" [ IN ] - input buffer of text
 */
VDB_EXTERN rc_t CC VSchemaParseText ( VSchema *self, const char *name,
    const char *text, size_t bytes );


/* ParseFile
 *  parse schema file
 *  add productions to existing schema
 *
 *  "name" [ IN ] - filename, absolute or valid relative to
 *  working directory or within an include path
 */
VDB_EXTERN rc_t CC VSchemaParseFile ( VSchema *self, const char *name, ... );
VDB_EXTERN rc_t CC VSchemaVParseFile ( VSchema *self, const char *name, va_list args );


/* Dump
 *  dump schema as text
 *
 *  "mode" [ IN ] - selects textual conversion mode ( see VSchemaDumpMode )
 *
 *  "decl" [ IN, NULL OKAY ] - selects a declaration to dump
 *
 *  "flush" [ IN ] and "dst" [ IN, OPAQUE ] - callback for delivering
 *  buffered schema text
 */
enum VSchemaDumpMode
{
    sdmPrint,            /* standard display for reading and printing */
    sdmCompact           /* compacted white-space representation      */
};

VDB_EXTERN rc_t CC VSchemaDump ( const VSchema *self, uint32_t mode, const char *decl,
    rc_t ( CC * flush ) ( void *dst, const void *buffer, size_t bsize ), void *dst );


/* IncludeFiles
 *  generates a list of paths to include file
 */
VDB_EXTERN rc_t CC VSchemaIncludeFiles ( const VSchema *self, struct KNamelist const **list );


/*--------------------------------------------------------------------------
 * VTypedecl
 *  a runtime type id and vector dimension
 *
 *  describes the general form of a type declaration: "TYPE [ DIM ]"
 */
typedef struct VTypedecl VTypedecl;
struct VTypedecl
{
    /* runtime type id */
    uint32_t type_id;

    /* vector dimension */
    uint32_t dim;
};


/* Resolve
 *  converts a typedecl string to type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved type declaration
 *
 *  "typedecl" [ IN ] - type declaration
 */
VDB_EXTERN rc_t CC VSchemaResolveTypedecl ( const VSchema *self,
    VTypedecl *resolved, const char *typedecl, ... );
VDB_EXTERN rc_t CC VSchemaVResolveTypedecl ( const VSchema *self,
    VTypedecl *resolved, const char *typedecl, va_list args );


/* ToText
 *  convert a VTypedecl into canonical text
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated type declaration string
 */
VDB_EXTERN rc_t CC VTypedeclToText ( const VTypedecl *self,
    const VSchema *schema, char *buffer, size_t bsize );


/* ToSupertype
 *  attempt to cast a typedecl to a size-equivalent supertype decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "cast" [ OUT ] - return parameter for supertype decl
 *  valid only if function returns true.
 *
 *  returns true if cast succeeded
 */
VDB_EXTERN bool CC VTypedeclToSupertype ( const VTypedecl *self,
    const VSchema *schema, VTypedecl *cast );


/* ToType
 *  attempt to cast a typedecl to a size-equivalent ancestor decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "ancestor" [ IN ] - target type or typeset for cast
 *
 *  "cast" [ OUT ] - return parameter for ancestor decl
 *  valid only if function returns true. if "ancestor" was a typeset,
 *  the value of "cast" will be the closest matching ancestral type.
 *
 *  "distance" [ OUT, NULL OKAY ] - optional linear measure of casting
 *  generations. valid only if function returns true. a value of 0 means
 *  that "self" is a direct match with "ancestor", a value of 1 means that
 *  "self" is an immediate subtype of "ancestor", etc.
 *
 *  returns true if cast succeeded
 */
VDB_EXTERN bool CC VTypedeclToType ( const VTypedecl *self,
    const VSchema *schema,  uint32_t ancestor,
    VTypedecl *cast, uint32_t *distance );


/* ToTypedecl
 *  attempt to cast a typedecl to a size-equivalent ancestor decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "ancestor" [ IN ] - target typedecl or typeset for cast
 *
 *  "cast" [ OUT, NULL OKAY ] - return parameter for ancestor decl
 *  valid only if function returns true. if "ancestor" was a typeset,
 *  the value of "cast" will be the closest matching ancestral type.
 *
 *  "distance" [ OUT, NULL OKAY ] - optional linear measure of casting
 *  generations. valid only if function returns true. a value of 0 means
 *  that "self" is a direct match with "ancestor", a value of 1 means that
 *  "self" is an immediate subtype of "ancestor", etc.
 *
 *  returns true if cast succeeded
 */
VDB_EXTERN bool CC VTypedeclToTypedecl ( const VTypedecl *self,
    const VSchema *schema, const VTypedecl *ancestor,
    VTypedecl *cast, uint32_t *distance );


/* CommonAncestor
 *  find a common ancestor between "self" and "peer"
 *  returns distance as sum of distances from each to "ancestor"
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "peer" [ IN ] - typedecl or typeset to compare against self
 *
 *  "ancestor" [ OUT, NULL OKAY ] - return parameter for closest
 *  common ancestor decl. valid only if function returns true.
 *
 *  "distance" [ OUT, NULL OKAY ] - optional linear measure of casting
 *  generations. valid only if function returns true. a value of 0 means
 *  that "self" is a direct match with "peer", a value of 1 means that
 *  "self" is an immediate subtype of "peer", or vice-versa, etc.
 *  measured as the sum of distances self=>ancestor and peer=>ancestor
 */
VDB_EXTERN bool CC VTypedeclCommonAncestor ( const VTypedecl *self, const VSchema *schema,
    const VTypedecl *peer, VTypedecl *ancestor, uint32_t *distance );


/*--------------------------------------------------------------------------
 * VTypedesc
 *  describes the fundamental properties of a VTypedecl
 *  replaces the deprecated structure VTypedef
 *
 *  VTypedecl describes an element in terms of the element's
 *  super-type id and vector dimension.
 *
 *  VTypedesc describes an element in terms of the element's
 *  intrinsic type domain and size, and the vector dimension
 *  is adjusted to reflect the intrinsic super-type.
 *
 *  the effect is as if a VTypedecl were cast toward its most
 *  fundamental supertype using "VTypedeclToSupertype", and
 *  the final size and dimension are taken from there:
 *
 *    given - VTypedecl td; const VSchema *schema;
 *    while ( VTypedeclToSupertype ( & td, schema, & td ) )
 *        ( void ) 0;
 *
 *    then -
 *    VTypedesc desc =
 *    {
 *      bit_sizeof_type ( td . type_id ),
 *      td . dim,
 *      domain_of_type ( td . type_id )
 *    }
 */
enum
{
    /* boolean domain */
    vtdBool = 1,

    /* numeric domains */
    vtdUint, vtdInt, vtdFloat,

    /* textual domains */
    vtdAscii, vtdUnicode
};

typedef struct VTypedesc VTypedesc;
struct VTypedesc
{
    /* sizeof intrinsic supertype */
    uint32_t intrinsic_bits;

    /* recalculated vector dimension */
    uint32_t intrinsic_dim;

    /* data domain */
    uint32_t domain;
};


/* Sizeof
 */
VDB_EXTERN uint32_t CC VTypedescSizeof ( const VTypedesc *self );


/* DescribeTypedecl
 *  produce a description of typedecl properties
 */
VDB_EXTERN rc_t CC VSchemaDescribeTypedecl ( const VSchema *self,
    VTypedesc *desc, const VTypedecl *td );



/*--------------------------------------------------------------------------
 * VSchemaRuntimeTable
 *  an object that represents a table ( or sub-table ) created at runtime
 *  that can be used to create a VTable object
 */
typedef struct VSchemaRuntimeTable VSchemaRuntimeTable;


/* Make
 *  make a table in the schema provided
 *
 *  "tbl" [ OUT ] - return parameter for new schema table object
 *
 *  "type_name" [ IN ] - NUL terminated string giving desired typename of table
 *   may include version spec - otherwise, a version of #1.0 will be assigned.
 *
 *  "supertype_spec" [ IN, NULL OKAY ] - optional supertype spec, giving
 *   either a single super-table type string or multiple comma-separated tables
 */
VDB_EXTERN rc_t CC VSchemaMakeRuntimeTable ( VSchema *self,
    VSchemaRuntimeTable **tbl, const char *type_name, const char *supertype_spec );


/* Close
 *  close the schema table handle
 *  discards any uncomitted data
 */
VDB_EXTERN rc_t CC VSchemaRuntimeTableClose ( VSchemaRuntimeTable *self );


/* Commit
 *  commits table data
 *  closes object to further modification
 */
VDB_EXTERN rc_t CC VSchemaRuntimeTableCommit ( VSchemaRuntimeTable *self );


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
VDB_EXTERN rc_t CC VSchemaRuntimeTableAddColumn ( VSchemaRuntimeTable *self, 
    const VTypedecl *td, const char *encoding, const char *name, ... );
VDB_EXTERN rc_t CC VSchemaRuntimeTableVAddColumn ( VSchemaRuntimeTable *self, 
    const VTypedecl *td, const char *encoding, const char *name, va_list args );


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
VDB_EXTERN rc_t CC VSchemaRuntimeTableAddBooleanColumn ( VSchemaRuntimeTable *self,
    const char *name, ... );
VDB_EXTERN rc_t CC VSchemaRuntimeTableAddIntegerColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, bool has_sign,  const char *name, ... );
VDB_EXTERN rc_t CC VSchemaRuntimeTableAddFloatColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, uint32_t significant_mantissa_bits,  const char *name, ... );
VDB_EXTERN rc_t CC VSchemaRuntimeTableAddAsciiColumn ( VSchemaRuntimeTable *self,
    const char *name, ... );
VDB_EXTERN rc_t CC VSchemaRuntimeTableAddUnicodeColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, const char *name, ... );

VDB_EXTERN rc_t CC VSchemaRuntimeTableVAddBooleanColumn ( VSchemaRuntimeTable *self,
    const char *name, va_list args );
VDB_EXTERN rc_t CC VSchemaRuntimeTableVAddIntegerColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, bool has_sign,  const char *name, va_list args );
VDB_EXTERN rc_t CC VSchemaRuntimeTableVAddFloatColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, uint32_t significant_mantissa_bits,  const char *name, va_list args );
VDB_EXTERN rc_t CC VSchemaRuntimeTableVAddAsciiColumn ( VSchemaRuntimeTable *self,
    const char *name, va_list args );
VDB_EXTERN rc_t CC VSchemaRuntimeTableVAddUnicodeColumn ( VSchemaRuntimeTable *self,
    uint32_t bits, const char *name, va_list args );


#ifdef __cplusplus
}
#endif

#endif /*  _h_vdb_schema_ */
