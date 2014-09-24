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

#ifndef _hpp_vdb_types_
#define _hpp_vdb_types_

// types are defined here
#ifndef _h_vdb_types_
#include <vdb/types.h>
#endif


/*--------------------------------------------------------------------------
 * VDatatypes
 */
struct VDatatypes
{
    /* AddRef
     * Release
     */
    inline rc_t AddRef () const throw()
    { return VDatatypesAddRef ( this ); }

    inline rc_t Release () const throw()
    { return VDatatypesRelease ( this ); }


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
#if 0 /* is omitted in sra/types.h too */
    inline rc_t RegisterOpaque ( const char *newtype,
       uint16_t bits, void ( CC *byte_swap ) ( void *elem ) ) throw()
    { return VDatatypesRegisterOpaque ( this, newtype, bits, byte_swap ); }

    inline rc_t RegisterSubtype ( const char *newtype, const char *supertype,
        uint16_t dim, void ( CC *byte_swap ) ( void *elem ) ) throw()
    { return VDatatypesRegisterSubtype ( this, newtype, supertype, dim, byte_swap ); }
#endif


    /* DefineSubtype
     *  creates a subtype definition
     *
     *    typedef = [ 'typedef' ] <supertype> <typedecl>
     */
#if 0 /* is omitted in sra/types.h too */
     inline rc_t DefineSubtype ( const char *definition, ... ) throw()
    {
        va_list args;
        va_start ( args, definition );
        rc_t rc = VDatatypesVDefineSubtype ( this, definition, args );
        va_end ( args );
        return rc;
    }
    inline rc_t DefineSubtype ( const char *definition, va_list args ) throw()
    { return VDatatypesVDefineSubtype ( this, definition, args ); }
#endif


    /* ResolveTypedecl
     *  converts a typedecl string to type id and vector dimension
     *
     *  "resolved" [ OUT ] - resolved type declaration
     *
     *  "typedecl" [ IN ] - type declaration
     */
    inline rc_t ResolveTypedecl ( VTypedecl *resolved,
        const char *typedecl, ... ) const throw()
    {
        va_list args;
        va_start ( args, typedecl );
        rc_t rc = VDatatypesVResolveTypedecl ( this, resolved, typedecl, args );
        va_end ( args );
        return rc;
    }
    inline rc_t ResolveTypedecl ( VTypedecl *resolved,
        const char *typedecl, va_list args ) const throw()
    { return VDatatypesVResolveTypedecl ( this, resolved, typedecl, args ); }


    /* ResolveTypename
     *  convert a type id into its type definition
     *
     *  "resolved" [ OUT ] - resolved type definition
     *
     *  "typname" [ IN ] - NUL-terminated type name
     */
    inline rc_t ResolveTypename ( VTypedef *resolved,
        const char *typname ) const throw()
    { return VDatatypesResolveTypename ( this, resolved, typname ); }


    /* ResolveTypeid
     *  convert a type id into its type definition
     *
     *  "resolved" [ OUT ] - resolved type definition
     *
     *  "type_id" [ IN ] - 1-based runtime type id
     */
    inline rc_t ResolveTypeid ( VTypedef *resolved,
        uint32_t type_id ) const throw()
    { return VDatatypesResolveTypeid ( this, resolved, type_id ); }


    /* SizeofTypedecl
     *  returns the bit size of type declaration
     */
    inline rc_t SizeofTypedecl ( bitsz_t *type_size,
        const VTypedecl *td ) const throw()
    { return VDatatypesSizeofTypedecl ( this, type_size, td ); }


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
    inline rc_t SizeofTypeid ( bitsz_t *type_size, uint32_t type_id,
        bool all_elements ) const throw()
    { return VDatatypesSizeofTypeid ( this, type_size, type_id, all_elements ); }


    /* MakeTypedecl
     *  convert a VTypedecl into canonical text
     *
     *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
     *  NUL terminated type declaration string
     *
     *  "typedecl" [ IN ] - binary representation of typedecl
     */
    inline rc_t MakeTypedecl ( char *buffer, size_t bsize,
        const VTypedecl *typedecl ) const throw()
    { return VDatatypesMakeTypedecl ( this, buffer, bsize, typedecl ); }


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
    inline rc_t ToSupertype ( VTypedecl *typedecl ) const throw()
    { return VDatatypesToSupertype ( this,  typedecl ); }


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
    inline rc_t ToIntrinsic ( VTypedecl *typedecl ) const throw()
    { return VDatatypesToIntrinsic ( this, typedecl ); }


    /* ToAncestor
     *  cast a typedecl to a size-equivalent ancestor decl
     *  returns true if cast can be performed
     */
    inline bool ToAncestor ( const VTypedecl *to,
        const VTypedecl *from ) const throw()
    { return VDatatypesToAncestor ( this, to, from ); }


private:
    VDatatypes ();
    ~ VDatatypes ();
    VDatatypes ( const VDatatypes& );
    VDatatypes &operator = ( const VDatatypes& );
};

#endif //  _hpp_vdb_types_
