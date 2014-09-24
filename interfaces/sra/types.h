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

#ifndef _h_sra_types_
#define _h_sra_types_

#ifndef _h_sra_extern_
#include <sra/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * logical intrinsics
 *  CS101 datatypes
 *
 *  logical types are intrinsic and have no signed property. they exist
 *  mainly to support bit manipulation operations. actual numeric types
 *  inherit their size and byte-swapping from logical types.
 */

#define vdb_bit1_t          "B1"
#define vdb_bit8_t          "B8"
#define vdb_bit16_t        "B16"
#define vdb_bit32_t        "B32"
#define vdb_bit64_t        "B64"
#define vdb_bit128_t      "B128"


/*--------------------------------------------------------------------------
 * numeric types
 *  CS101 datatypes
 *
 *  in general, all numbers are named type + size in bits, where
 *  type = "U" for unsigned, "I" for signed, and "F" for floats.
 */

#define vdb_uint1_t         "U1"       /* supertype "B1"      */
#define vdb_int8_t          "I8"       /* supertype "B8"      */
#define vdb_uint8_t         "U8"       /* supertype "B8"      */
#define vdb_int16_t        "I16"       /* supertype "B16"     */
#define vdb_uint16_t       "U16"       /* supertype "B16"     */
#define vdb_int32_t        "I32"       /* supertype "B32"     */
#define vdb_uint32_t       "U32"       /* supertype "B32"     */
#define vdb_int64_t        "I64"       /* supertype "B64"     */
#define vdb_uint64_t       "U64"       /* supertype "B64"     */
#define vdb_int128_t      "I128"       /* supertype "B128"    */
#define vdb_uint128_t     "U128"       /* supertype "B128"    */
#define vdb_float32_t      "F32"       /* supertype "B32"     */
#define vdb_float64_t      "F64"       /* supertype "B64"     */


/*--------------------------------------------------------------------------
 * boolean
 */

#define vdb_bool_t      "bool"          /* supertype "U8"     */


/*--------------------------------------------------------------------------
 * character types
 */

#define vdb_utf8_t      "utf8"          /* supertype "B8"     */
#define vdb_utf16_t     "utf16"         /* supertype "B16"    */
#define vdb_utf32_t     "utf32"         /* supertype "B32"    */
#define vdb_ascii_t     "ascii"         /* supertype "utf8"   */


/*--------------------------------------------------------------------------
 * INSDC types
 */
#define insdc_fasta_t   "INSDC:dna:text"        /* supertype "ascii"  */
#define insdc_csfasta_t "INSDC:color:text"      /* supertype "ascii"  */
#define insdc_2na_t     "INSDC:2na:packed"      /* supertype "B1[2]"  */
#define insdc_4na_t     "INSDC:4na:packed"      /* supertype "B1[4]"  */
#define insdc_2cs_t     "INSDC:2cs:packed"      /* supertype "B1[2]"  */
#define insdc_phred_t   "INSDC:quality:phred"   /* supertype "U8"     */
#define insdc_logodds_t "INSDC:quality:log_odds"/* supertype "I8"     */


/*--------------------------------------------------------------------------
 * NCBI types
 *  types common within NCBI.
 *
 *  partial byte sized types do not have a registered supertype,
 *  but are commented to indicate their size
 */

#define ncbi_qual4_t    "NCBI:qual4"         /* supertype "I8[4]"  */
#define ncbi_isamp1_t   "NCBI:isamp1"        /* supertype "I16"    */
#define ncbi_isamp4_t   "NCBI:isamp4"        /* supertype "I16[4]" */
#define ncbi_fsamp1_t   "NCBI:fsamp1"        /* supertype "F32"    */
#define ncbi_fsamp4_t   "NCBI:fsamp4"        /* supertype "F32[4]" */


/* alias names */
#define ncbi_2na_t      insdc_2na_t
#define ncbi_4na_t      insdc_4na_t
#define ncbi_2cs_t      insdc_2cs_t
#define ncbi_qual1_t    insdc_phred_t


/*--------------------------------------------------------------------------
 * VTypedef
 *  used to describe properties of a type
 *
 *  when properly filled out, the structure indicates
 *  the 1-based runtime id of type, that of its supertype
 *  or 0 if none, the size in bits of type, the size in
 *  bits of the supertype ( or element type, when dim > 1 )
 *  and the vector dimension given when type was introduced,
 *  as well as whether the type is a signed numeric.
 *
 *  example: "typedef I8 NCBI:qual4 [ 4 ]" translates to:
 *    type_id   = runtime type id of NCBI:qual4
 *    super_id  = runtime type id of I8
 *    size      = 8
 *    dim       = 4
 *    sign      = true
 */
typedef struct VTypedef VTypedef;
struct VTypedef
{
    /* runtime type id */
    uint32_t type_id;

    /* supertype id */
    uint32_t super_id;

    /* size of type element */
    bitsz_t type_size;

    /* vector dimension */
    uint32_t dim;

    /* signed property */
    uint32_t sign;
};


/* Sizeof
 *  returns the bit size of declared type
 *
 *  technically, this is self -> size * self -> dim.
 */
/* bitsz_t VTypedefSizeof ( const VTypedef *self ); */
#define VTypedefSizeof(T) ((bitsz_t)(((T)->type_size) * ((T)->dim)))


/*--------------------------------------------------------------------------
 * VDatatypes
 */
typedef struct VDatatypes VDatatypes;


/* forward */
struct VTypedecl;


/* AddRef
 * Release
 */
SRA_EXTERN rc_t CC VDatatypesAddRef ( const VDatatypes *self );
SRA_EXTERN rc_t CC VDatatypesRelease ( const VDatatypes *self );


/* RegisterOpaque
 * RegisterSubtype
 *  registers new datatypes
 *
 *  "newtype" [ IN ] - NUL terminated fully qualified type name
 *
 *  "supertype" [ IN ] and "dim" [ IN ] - NUL terminated fully
 *  qualified parent equivalency type and vector dimension
 *  -OR-
 *  "bits" [ IN ] - size in bits of element.
 *
 *  "byte_swap" [ IN, CONDITIONALLY NULL OKAY ] - a byte swapping function
 *  required when size > 8 and and integral multiple of 8.
 */
#if 0
SRA_EXTERN rc_t CC VDatatypesRegisterOpaque ( VDatatypes *self,
    const char *newtype, uint16_t bits,
    void ( CC * byte_swap ) ( void *elem ) );
SRA_EXTERN rc_t CC VDatatypesRegisterSubtype ( VDatatypes *self,
    const char *newtype, const char *supertype, uint16_t dim,
    void ( CC * byte_swap ) ( void *elem ) );
#endif

/* DefineSubtype
 *  creates a subtype definition
 *
 *    typedef = [ 'typedef' ] <supertype> <typedecl>
 */
#if 0
SRA_EXTERN rc_t CC VDatatypesDefineSubtype ( VDatatypes *self,
    const char *definition, ... );
SRA_EXTERN rc_t CC VDatatypesVDefineSubtype ( VDatatypes *self,
    const char *definition, va_list args );
#endif

/* ResolveTypedecl
 *  converts a typedecl string to type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved type declaration
 *
 *  "typedecl" [ IN ] - type declaration
 */
SRA_EXTERN rc_t CC VDatatypesResolveTypedecl ( const VDatatypes *self,
    struct VTypedecl *resolved, const char *typedecl, ... );
SRA_EXTERN rc_t CC VDatatypesVResolveTypedecl ( const VDatatypes *self,
    struct VTypedecl *resolved, const char *typedecl, va_list args );


/* ResolveTypename
 *  convert a type id into its type definition
 *
 *  "resolved" [ OUT ] - resolved type definition
 *
 *  "typname" [ IN ] - NUL-terminated type name
 */
SRA_EXTERN rc_t CC VDatatypesResolveTypename ( const VDatatypes *self,
    VTypedef *resolved, const char *typname );


/* ResolveTypeid
 *  convert a type id into its type definition
 *
 *  "resolved" [ OUT ] - resolved type definition
 *
 *  "type_id" [ IN ] - 1-based runtime type id
 */
SRA_EXTERN rc_t CC VDatatypesResolveTypeid ( const VDatatypes *self,
    VTypedef *resolved, uint32_t type_id );


/* SizeofTypedecl
 *  returns the bit size of type declaration
 */
SRA_EXTERN rc_t CC VDatatypesSizeofTypedecl ( const VDatatypes *self,
    bitsz_t *type_size, struct VTypedecl const *td );


/* SizeofTypeid
 *  returns the bit size of typeid
 *
 *  "type_size" [ OUT ] - bit size of a type
 *
 *  "type_id" [ IN ] - numerical index of a type
 *
 *  "all_elements" [ IN ] - for multi-dimesional types
 *  return the sum of sizes, otherwise - only for a single element
 */
SRA_EXTERN rc_t CC VDatatypesSizeofTypeid ( const VDatatypes *self,
    bitsz_t *type_size, uint32_t type_id, bool all_elements );


/* MakeTypedecl
 *  convert a VTypedecl into canonical text
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated type declaration string
 *
 *  "typedecl" [ IN ] - binary representation of typedecl
 */
SRA_EXTERN rc_t CC VDatatypesMakeTypedecl ( const VDatatypes *self,
    char *buffer, size_t bsize, struct VTypedecl const *typedecl );


/* ToSupertype
 *  cast a typedecl to a size-equivalent supertype decl
 *  returns not-found and leaves "typedecl" unchanged
 *  if no supertype exists
 *
 *  "typedecl" [ IN, OUT ] - populated with a correct
 *  type id and vector dimension on input, will be updated
 *  to contain a supertype and equivalent dimension on
 *  output. e.g.:
 *
 *    IN -  { type_id NCBI:qual4, dim 1 }
 *    OUT - { type_id I8, dim 4 }
 */
SRA_EXTERN rc_t CC VDatatypesToSupertype ( const VDatatypes *self, struct VTypedecl *typedecl );


/* ToIntrinsic
 *  cast a typedecl ( if necessary ) to a size-equivalent
 *  intrinsic typedecl.
 *
 *  returns not-found and leaves "typedecl" unchanged
 *  if no intrinsic supertype exists
 *
 *  leaves "typedecl" unchanged if it is already intrinsic.
 *
 *  "typedecl" [ IN, OUT ] - populated with a correct
 *  type id and vector dimension on input, will be updated
 *  to contain a supertype and equivalent dimension on
 *  output. e.g.:
 *
 *    IN -  { type_id NCBI:qual4, dim 1 }
 *    OUT - { type_id I8, dim 4 }
 */
SRA_EXTERN rc_t CC VDatatypesToIntrinsic ( const VDatatypes *self, struct VTypedecl *typedecl );


/* ToAncestor
 *  cast a typedecl to a size-equivalent ancestor decl
 *  returns true if cast can be performed
 */
SRA_EXTERN bool CC VDatatypesToAncestor ( const VDatatypes *self,
    struct VTypedecl const *to, struct VTypedecl const *from );


#ifdef __cplusplus
}
#endif

#endif /*  _h_sra_types_ */
