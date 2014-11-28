/*=======================================================================================
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

#ifndef _h_vdb_xform_
#define _h_vdb_xform_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_vdb_schema_
#include <vdb/schema.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KDataBuffer;
struct VCursorParams;

/*--------------------------------------------------------------------------
 * VFormatdecl
 */
typedef struct VFormatdecl VFormatdecl;
struct VFormatdecl
{
    VTypedecl td;
    uint32_t fmt;
};


/* Resolve
 *  converts a fmtdecl string to fmt id, type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved fmtdecl
 *
 *  "fmtdecl" [ IN ] - type declaration
 */
VDB_EXTERN rc_t CC VSchemaResolveFmtdecl ( struct VSchema const *self,
    VFormatdecl *resolved, const char *fmtdecl, ... );
VDB_EXTERN rc_t CC VSchemaVResolveFmtdecl ( struct VSchema const *self,
    VFormatdecl *resolved, const char *fmtdecl, va_list args );


/* ToText
 *  convert a VFormatdecl into canonical text
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated type declaration string
 */
VDB_EXTERN rc_t CC VFormatdeclToText ( const VFormatdecl *self,
    struct VSchema const *schema, char *buffer, size_t bsize );


/* ToFormatdecl
 *  attempt to cast a fmtdecl to a size-equivalent ancestor decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "ancestor" [ IN ] - target fmtdecl or typespec for cast
 *
 *  "cast" [ OUT, NULL OKAY ] - return parameter for ancestor decl
 *  valid only if function returns true. if "ancestor" contained a typeset,
 *  the value of "cast" will be the closest matching ancestral type.
 *
 *  "distances" [ OUT, NULL OKAY ] - optional linear measure of independent
 *  casting generations for format ( 0 ) and type ( 1 ). valid only if function
 *  returns true. a value of 0 means that "self" is a direct match with "ancestor",
 *  a value of 1 means that "self" is an immediate subtype of "ancestor", etc.
 *
 *  returns true if cast succeeded
 */
VDB_EXTERN bool CC VFormatdeclToFormatdecl ( const VFormatdecl *self,
    const VSchema *schema, const VFormatdecl *ancestor,
    VFormatdecl *cast, uint32_t distances [ 2 ] );


/*--------------------------------------------------------------------------
 * VBlobHeader
 *  opaque blob header
 *  used by external functions to read and populate blob headers
 */
typedef struct VBlobHeader VBlobHeader;


/* Release
 *  used to release forked headers
 */
VDB_EXTERN rc_t CC VBlobHeaderRelease ( const VBlobHeader *self );


/* Version
 *  returns previously written blob format version
 */
VDB_EXTERN uint8_t CC VBlobHeaderVersion ( const VBlobHeader *self );
VDB_EXTERN void CC VBlobHeaderSetVersion ( VBlobHeader *self, uint8_t version );


/* OpPushTail
 * OpPopHead
 * OpPopTail
 *  op-code stack interface
 *
 *  functions that apply encoding to a blob may use this stack to
 *  record one or more "op-codes" that may be useful to decoders
 *  when used in combination with the argument (operand) stack.
 *
 *  the op-code values and significance is entirely defined by the
 *  external functions, and their use is optional (a trivial example
 *  is described below in the argument stack comment).
 *
 *  during update operations, the stack may be pushed as well as popped.
 *  during read-only operations, only pop is permitted.
 */
VDB_EXTERN rc_t CC VBlobHeaderOpPushTail ( VBlobHeader *self, uint8_t op );
VDB_EXTERN rc_t CC VBlobHeaderOpPopHead ( const VBlobHeader *self, uint8_t *op );
VDB_EXTERN rc_t CC VBlobHeaderOpPopTail ( const VBlobHeader *self, uint8_t *op );


/* ArgPushTail
 * ArgPopHead
 * ArgPopTail
 *  argument (operand) stack interface
 *
 *  provides arguments to accompany encoding and decoding operations,
 *  where the operation may be implicit or indicated via a related
 *  op-code.
 *
 *  here is a trivial example for a hypothetical function that performs
 *  bit-packing on a series of signed integers. it has two possible
 *  operations: translation and pack. it translates the values by subtracting
 *  the MIN(x) value, converting the sequence into unsigned values starting
 *  at 0. pack will left-truncate bits if the translated MAX(x) can be represented
 *  with fewer bits than in use:
 *
 *    1. calculate min/max
 *    2. if MIN(x) != 0
 *       2a. ArgPushTail ( MIN ( x ) )
 *       2b. OpPushTail ( TRANSLATE-OPCODE ) # e.g. 1
 *       2c. translate sequence by - min
 *       2d. translate MAX ( x )
 *    3. calculate bits required for MAX(x)
 *    4. if bits-required < bits-used
 *       4a. ArgPushTail ( bits-required )
 *       4b. OpPushTail ( PACK-OPCODE )      # e.g. 2
 *       4c. pack sequence
 *
 *  on decode, the process would be reversed:
 *
 *    1. OpPopTail ( & op-code )
 *    2. if op-code == PACK-OPCODE
 *       2a. ArgPopTail ( & packed-bits )
 *       2b. unpack sequence from packed-bits to current size
 *       2c. OpPopTail ( & op-code )
 *    3. if op-code == TRANSLATE-OPCODE
 *       3a. ArgPopTail ( & offset )
 *       3b. translate sequence by + offset
 *
 *  during update operations, the stack may be pushed as well as popped.
 *  during read-only operations, only pop is permitted.
 */
VDB_EXTERN rc_t CC VBlobHeaderArgPushTail ( VBlobHeader *self, int64_t arg );
VDB_EXTERN rc_t CC VBlobHeaderArgPopHead ( const VBlobHeader *self, int64_t *arg );
VDB_EXTERN rc_t CC VBlobHeaderArgPopTail ( const VBlobHeader *self, int64_t *arg );


/* CreateFork
 *  create a parallel, writable blob header
 *
 *  blob headers are designed to record operations and arguments
 *  that are serially applied to data. some functions may need to
 *  run operations in parallel ( e.g. compresion ) where each fork
 *  is allowed to compete for best performance.
 *
 *  this function creates an alternate, parallel header object that
 *  may be used to replace the actual headers associated with the blob.
 *
 * NB - returned object must ALWAYS be released before exiting function
 */
VDB_EXTERN rc_t CC VBlobHeaderCreateFork ( const VBlobHeader *self, VBlobHeader **rslt );


/*==========================================================================
 * transformation functions
 *
 *  transformation functions are declared in schema and provided in
 *  dynamically loadable shared libraries. the types described in the
 *  following section will be used during data processing.
 */


/* VFormatdesc
 *  combines VFormatdecl and VTypedesc
 */
typedef struct VFormatdesc VFormatdesc;
struct VFormatdesc
{
    VFormatdecl fd;
    VTypedesc desc;
};


/* XformInfo
 *  constant input parameter block
 *
 *  "mgr" [ IN ] - db mgr object
 *
 *  "schema" [ IN ] - cursor schema
 *
 *  "tbl" [ IN ] - owning table. can be used
 *   to access enclosing database
 */
#define VTABLE_PASSED_TO_XFORM 1
#define VPRODUCTION_PASSED_TO_XFORM 1

struct VProduction;
typedef struct VXformInfo VXformInfo;
struct VXformInfo
{
#if VMGR_PASSED_TO_XFORM
    struct VDBManager const *mgr;
#endif
#if VSCHEMA_PASSED_TO_XFORM
    struct VSchema const *schema;
#endif
#if VTABLE_PASSED_TO_XFORM
    struct VTable const *tbl;
#endif
#if VPRODUCTION_PASSED_TO_XFORM
    struct VProduction const *prod;
#endif
    VFormatdesc fdesc;
};


/* VRowData
 *  row function input block
 *
 *  "base" [ IN ] and "first_elem" [ IN ] - first element in row
 *  i.e. for any row of type "T", "( ( const T* ) base ) [ first_elem ]"
 *  is the first element.
 *
 *  "elem_count" [ IN ] - the number of elements in this row
 *
 *  "element_bits" [ IN ] - the number of bits in each element
 *
 *  "variant" [ IN ] - describes variant of union "u"
 */
typedef uint8_t VRowDataVariant;
enum { vrdData, vrdControl };

typedef struct VRowData VRowData;
struct VRowData
{
    union
    {
        struct
        {
            /* the number of elements in this row */
            uint64_t elem_count;

            /* element size in bits */
            uint64_t elem_bits;

            /* page data base address plus
               offset to first element of row */
            uint64_t first_elem;
            const void *base;
			uint64_t base_elem_count; /*** the number of elements memory is valid from base; needed for prefetch***/
        } data;

    } u;
    int64_t blob_stop_id;

    /* describes variant of "u" */
    VRowDataVariant variant;
};


/* VRowResult
 *  row function output block
 *
 *  for functions that operate on a single row of input
 *  but generate an output of independent length.
 *
 *  "data" [ IN/OUT ] - externally allocated data buffer
 *  may be resized or replaced, and must be checked for
 *  adequate capacity before writing.
 *
 *  "elem_count" [ OUT, DEFAULT ZERO ] - return parameter for
 *  the number of elements in result
 *
 *  "elem_bits" [ IN ] - element size in bits
 *
 *  "no_cache" [ OUT, DEFAULT ZERO ] - to indicate whether this result is
 *  cache-able
 */
typedef struct VRowResult VRowResult;
struct VRowResult
{
    /* return slot for number of elements written */
    uint64_t elem_count;

    /* size of elements in bits */
    uint64_t elem_bits;

    /* externally allocated data buffer
       NB - must be checked for storage capacity before writing */
    struct KDataBuffer *data;
    
    uint8_t no_cache;
};


/* VFixedRowResult
 *  fixed row function output block
 *
 *  for functions that operate on a single row of input
 *  and generate an output row of the same length.
 *
 *  NB. These functions are considered to be deterministic.
 *
 *  "base" [ IN ] and "first_element" [ IN ] - first element in row
 *  i.e. for any row of type "T", "( ( const T* ) base ) [ row_start ]"
 *  is the first element.
 *
 *  "elem_count" [ IN ] - fixed length of output row
 *
 *  "elem_bits" [ IN ] - element size in bits
 */
typedef struct VFixedRowResult VFixedRowResult;
struct VFixedRowResult
{
    /* fixed row length */
    uint64_t elem_count;

    /* size of elements in bits */
    uint64_t elem_bits;

    /* externally allocated buffer of fixed length */
    uint64_t first_elem;
    void *base;
};


/* VByteOrder
 *  single byte indicator of byte order
 *
 *  "vboNone" - data are byte order independent
 *
 *  "vboNative" - data are in native byte order
 *
 *  "vboLittleEndian" - specific byte orders
 *  "vboBigEndian"
 */
typedef uint8_t VByteOrder;
enum
{
    vboNone,
    vboNative,
    vboLittleEndian,
    vboBigEndian
};


/* VBlobData
 *  blob function input block
 *
 *  describes entire blob of input without row boundaries
 *  the size of the blob in bits will be "elem_count" * "elem_bits"
 *
 *  "data" [ IN ] - pointer to first byte in blob
 *
 *  "elem_count" [ IN ] - the number of elements in blob
 *
 *  "elem_bits" [ IN ] - element size in bits
 *
 *  "byte_order" [ IN ] - byte order of input data
 */
typedef struct VBlobData VBlobData;
struct VBlobData
{
    /* element size in bits */
    uint64_t elem_bits;

    /* blob data buffer */
    uint64_t elem_count;
    const void *data;

    /* byte order of data */
    VByteOrder byte_order;
};


/* VBlobResult
 *  blob function output block
 *
 *  describes entire blob of output without row boundaries
 *
 *  "header" [ OUT, NULL OKAY ] - optional return parameter
 *  for substituted header when encoding
 *
 *  "data" [ IN ] - pointer to output buffer
 *
 *  "elem_count" [ IN/OUT ] - the maximum number
 *  of elements in blob ( at the current elem_bits )
 *
 *  "elem_bits" [ IN/OUT ] - element size in bits
 *
 *  "byte_order" [ IN/OUT ] - byte order of input data
 */
typedef struct VBlobResult VBlobResult;
struct VBlobResult
{
    /* element size in bits */
    uint64_t elem_bits;

    /* blob data buffer */
    uint64_t elem_count;
    void *data;

    /* optional replacement header */
    VBlobHeader *header;

    /* byte order of data */
    VByteOrder byte_order;
};


/* VBlobFunc
 *  functions that create blobs from a single input
 *
 *  "info" [ IN ] - runtime objects and information
 *
 *  "dst" [ IN ] - return parameter block
 *
 *  "src" [ IN ] - single input parameter
 *
 *  "hdr" [ IN ] - blob header belonging to either the "src"
 *  or "dst" blob depending upon activity. when encoding, "hdr"
 *  pertains to the "dst" blob, and should be populated with
 *  information needed when decoding, when "hdr" will pertain
 *  to "src".
 *
 *  return values:
 *    0 for no error
 *    RC ( *, rcFunction, rcExecuting, rcBuffer, rcInsufficient )
 *      when output buffer is insufficient. this condition is an
 *      error on decompression, but not on compress. on compress,
 *      the size of input buffer == size of output buffer. if no
 *      compression is possible, the library will store data as is.
 *    RC ( *, rcFunction, rcExecuting, *, * )
 *      general errors have this RC format, where your module
 *      and proper object and state defines are substituted.
 */
typedef rc_t ( CC * VBlobFunc ) ( void *self, const VXformInfo *info,
    VBlobResult *dst, const VBlobData *src, VBlobHeader *hdr );


/* VArrayFunc
 *  functions that perform element-wise transforms
 *
 *  "info" [ IN ] - runtime objects and information
 *
 *  "dst" [ IN ] - return buffer
 *
 *  "src" [ IN ] - input buffer
 *
 *  "elem_count" [ IN ] - the number of elements
 *  in both input and output
 */
typedef rc_t ( CC * VArrayFunc ) ( void *self, const VXformInfo *info,
    void *dst, const void *src, uint64_t elem_count );


/* VNonDetRowFunc
 *  functions that operate on a single row of input data
 *  and produce a single row of output data, where the
 *  number of elements in the output row is unrelated to
 *  the number of elements of input.
 *  NB. these functions are considered non-deterministic.
 *
 *  "info" [ IN ] - runtime objects and information
 *
 *  "row_id" [ IN ] - specific row id being processed
 *
 *  "rslt" [ IN ] - return parameter block
 *
 *  "argc" [ IN ] and "argv" [ IN ] - zero or more input
 *  parameter blocks
 */
typedef rc_t ( CC * VNonDetRowFunc ) ( void *self,
    const VXformInfo *info, int64_t row_id, VRowResult *rslt,
    uint32_t argc, const VRowData argv [] );


/* VRowFunc
 *  functions that operate on a single row of input data
 *  and produce a single row of output data, where the
 *  number of elements in the output row is unrelated to
 *  the number of elements of input.  The results are
 *  cached.
 *
 *  "info" [ IN ] - runtime objects and information
 *
 *  "row_id" [ IN ] - specific starting row id being processed
 *
 *  "rslt" [ IN ] - return parameter block
 *
 *  "argc" [ IN ] and "argv" [ IN ] - zero or more input
 *  parameter blocks
 */
typedef rc_t ( CC * VRowFunc ) ( void *self,
     const VXformInfo *info, int64_t row_id, VRowResult *rslt,
     uint32_t argc, const VRowData argv [] );

    
/* VFixedRowFunc
 *  functions that operate on a single row of input data
 *  and produce a single row of output data, where the
 *  number of elements in the output row is known to be
 *  equal to the number of elements of input.
 *
 *  "info" [ IN ] - runtime objects and information
 *
 *  "row_id" [ IN ] - specific row id being processed
 *
 *  "rslt" [ IN ] - return parameter block
 *
 *  "argc" [ IN ] and "argv" [ IN ] - zero or more input
 *  parameter blocks
 */
typedef rc_t ( CC * VFixedRowFunc ) ( void *self,
    const VXformInfo *info, int64_t row_id, const VFixedRowResult *rslt,
    uint32_t argc, const VRowData argv [] );



/*==========================================================================
 * factory functions
 *
 *  factory functions are declared in schema - normally implicitly but
 *  sometimes explicitly - and provided in dynamically loadable shared
 *  libraries. the types described in the following section will be used
 *  during cursor open to generate optimized code for data processing.
 */


/* XfactInfo
 *  constant input parameter block
 *  holds references to managers and container hierarchy
 *  as well as transform return type information
 *
 *  "mgr" [ IN ] - db mgr object
 *
 *  "dt" [ IN ] - type system mgr
 *
 *  "tbl" [ IN ] - owning table. can be used
 *   to access enclosing database
 *
 *  "desc" [ IN ] - description of return data type
 */
typedef struct VXfactInfo VXfactInfo;
struct VXfactInfo
{
    struct VDBManager const *mgr;
    struct VSchema const *schema;
    struct VTable const *tbl;
    struct VCursorParams const *parms;
    VFormatdesc fdesc;
};


/* VFactoryParams
 *  block of parameters to be evaluated by factory
 *  used to "templatize" runtime function
 *
 *  "argc" [ IN ] and "argv" [ IN, DYNAMIC ] - variable
 *  length array of input parameter description blocks:
 *
 *    "desc" [ IN ] - type descriptor tells data domain
 *    and size information for a single element of input
 *
 *    "count" [ IN ] - gives the dimensionality of
 *    data vector.
 *
 *    "data" [ IN, UNION ] - array of input elements
 *    selected by "desc.domain" and "desc.intrinsic_bits"
 */
typedef struct VFactoryParams VFactoryParams;
struct VFactoryParams
{
    uint32_t argc;
    uint32_t align;

    struct
    {
        /* describes constant element,
           i.e. sizeof ( arg typedecl ) */
        VTypedesc desc;

        /* describes length of param vector:
           bool, numeric: count of [ elem, elem, ... ]
           utf8: number of bytes
           other text: number of characters */
        uint32_t count;

        union
        {
            /* boolean */
            const bool *b;

            /* integer */
            const int8_t *i8;
            const int16_t *i16;
            const int32_t *i32;
            const int64_t *i64;

            /* unsigned integer */
            const uint8_t *u8;
            const uint16_t *u16;
            const uint32_t *u32;
            const uint64_t *u64;

            /* floating point */
            const float *f32;
            const double *f64;

            /* ascii */
            const char *ascii;

            /* unicode */
            const char *utf8;
            const uint16_t *utf16;
            const uint32_t *utf32;

            /* missing function pointer */

        } data;

    } argv [ 16 ];
};


/* FunctionParams
 *  block of parameter typedecls to be evaluated by factory
 *  used to "templatize" runtime function
 *
 *  "argc" [ IN ] and "argv" [ IN, DYNAMIC ] - description
 *  of runtime parameters. actual dimension is given by "argc"
 */
typedef struct VFunctionParams VFunctionParams;
struct VFunctionParams
{
    uint32_t argc;
    uint32_t align;
    VFormatdesc argv [ 16 ];
};


/* FuncDesc
 *  parameter block for describing runtime function
 *  guaranteed to be zeroed on input
 *
 *  "self"  [ OUT, OPAQUE ] - optional object or
 *  data pointer for function.
 *
 *  "whack" [ OUT, NULL OKAY ] - optional destructor
 *  called with "self" when function is released
 *
 *  "u" [ OUT ] - several variants of function
 *
 *  "variant" [ OUT ] - selects variant of "u"
 */
typedef uint32_t VFuncType;
enum
{
    vftInvalid,
    vftReserved,
    vftRow,
	vftRowFast,  /*** function is fast, so window can be stretched to the max **/
    vftIdDepRow, /*** function dependent on row_id ***/
    vftFixedRow,
    vftNonDetRow,
    vftArray,
    vftBlob
};

typedef struct VFuncDesc VFuncDesc;
struct VFuncDesc
{
    /* object and optional destructor */
    void *self;
    void ( CC * whack ) ( void *self );
    
    /* runtime function */
    union
    {
        VRowFunc rf;
        VNonDetRowFunc ndf;
        VFixedRowFunc pf;
        VArrayFunc af;
        VBlobFunc bf;
    } u;
    
    VFuncType variant;
};


/* TransformFactory
 *  unified typedef for row, page and blob transformation function factories
 *
 *  "self" [ IN, OPAQUE ] - NULL or const object pointer
 *  value registered with the manager
 *
 *  "info" [ IN ] - managers and table for the column,
 *  plus column type information
 *
 *  "rslt" [ IN ] - return value parameter block for resolved function
 *
 *  "cp" [ IN ] - constant template parameters
 *
 *  "dp" [ IN ] - type information for runtime parameters
 */
typedef
rc_t ( CC * VTransformFactory ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp );


/* VTransDesc
 *  describes a transform factory
 */
typedef struct VTransDesc VTransDesc;
struct VTransDesc
{
    /* opaque factory data object */
    void *fself;
    void ( CC * whack ) ( void *fself );

    /* factory entrypoint */
    VTransformFactory factory;

    /* interface version */
    uint32_t itf_version;
};

#define VTRANSVERS( maj, min, rel ) \
    ( ( ( maj ) << 24 ) | ( ( min ) << 16 ) | ( rel ) )

/* actual factory function implementation name */
#define VTRANSFACT_NAME( fact ) \
    fact ## _fact

/* exported factory name */
#define VTRANSFACT_DECL( fact ) \
    rc_t CC fact ( VTransDesc *desc )

/* factory declaration with no self parameter */
#define VTRANSFACT_IMPL( fact, maj, min, rel ) \
    VTRANSFACT_IMPL_SELF( fact, maj, min, rel, NULL )

/* factory declaration with static or constant self parameter */
#define VTRANSFACT_IMPL_SELF( fact, maj, min, rel, self ) \
    static rc_t CC VTRANSFACT_NAME ( fact ) ( const void*, \
        const VXfactInfo*, VFuncDesc*, const VFactoryParams*, const VFunctionParams* ); \
    rc_t CC fact ( VTransDesc *desc ) \
    { \
        desc -> fself = ( self ); \
        desc -> whack = NULL; \
        desc -> factory = VTRANSFACT_NAME ( fact ); \
        desc -> itf_version = VTRANSVERS ( maj, min, rel ); \
        return 0; \
    } \
    static rc_t CC VTRANSFACT_NAME ( fact )

/* factory declaration with dynamically allocated self parameter */
#define VTRANSFACT_IMPL_CONSTRUCT( fact, maj, min, rel, construct, destroy ) \
    static rc_t CC VTRANSFACT_NAME ( fact ) ( const void*, \
        const VXfactInfo*, VFuncDesc*, const VFactoryParams*, const VFunctionParams* ); \
    rc_t CC fact ( VTransDesc *desc ) \
    { \
        desc -> whack = ( destroy ); \
        desc -> factory = VTRANSFACT_NAME ( fact ); \
        desc -> itf_version = VTRANSVERS ( maj, min, rel ); \
        return construct ( & desc -> fself ); \
    } \
    static rc_t CC VTRANSFACT_NAME ( fact )


#ifdef __cplusplus
}
#endif

#endif /* _h_vdb_xform_ */
