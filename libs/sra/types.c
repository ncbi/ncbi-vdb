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

#include <sra/extern.h>
#include <klib/defs.h>
#include <sra/types.h>
#include <vdb/schema.h>
#include <vdb/vdb-priv.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VTypedef
 */


/* Sizeof
 *  returns the bit size of declared type
 *
 *  technically, this is self -> size * self -> dim.
 */
#ifndef VTypedefSizeof
LIB_EXPORT bitsz_t CC VTypedefSizeof ( const VTypedef *self )
{
    if ( self == NULL )
        return 0;
    return ( bitsz_t ) self -> type_size * self -> dim;
}
#endif

/*--------------------------------------------------------------------------
 * VDatatypes
 */


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC VDatatypesAddRef ( const VDatatypes *self )
{
    return VSchemaAddRef ( ( const VSchema* ) self );
}

LIB_EXPORT rc_t CC VDatatypesRelease ( const VDatatypes *self )
{
    return VSchemaRelease ( ( const VSchema* ) self );
}


/* ResolveTypedecl
 *  converts a typedecl string to type id and vector dimension
 *
 *  "resolved" [ OUT ] - resolved type declaration
 *
 *  "typedecl" [ IN ] - type declaration
 */
LIB_EXPORT rc_t CC VDatatypesVResolveTypedecl ( const VDatatypes *self,
    VTypedecl *resolved, const char *typedecl, va_list args )
{
    return VSchemaVResolveTypedecl ( ( const VSchema* ) self, resolved, typedecl, args );
}

LIB_EXPORT rc_t CC VDatatypesResolveTypedecl ( const VDatatypes *self,
    VTypedecl *resolved, const char *typedecl, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, typedecl );
    rc = VSchemaVResolveTypedecl ( ( const VSchema* ) self, resolved, typedecl, args );
    va_end ( args );

    return rc;
}



/* ResolveTypename
 *  convert a type id into its type definition
 *
 *  "resolved" [ OUT ] - resolved type definition
 *
 *  "typname" [ IN ] - NUL-terminated type name
 */
LIB_EXPORT rc_t CC VDatatypesResolveTypename ( const VDatatypes *self,
    VTypedef *resolved, const char *typname )
{
    rc_t rc;

    if ( resolved == NULL )
        rc = RC ( rcSRA, rcSchema, rcResolving, rcParam, rcNull );
    else
    {
        VTypedecl td;
        rc = VSchemaResolveTypedecl ( ( const VSchema* ) self, & td, "%s", typname );
        if ( rc == 0 )
        {
            rc = VSchemaDescribeTypedef ( ( const VSchema* ) self, resolved, td . type_id );
            if ( rc == 0 )
                return rc;
        }

        memset ( resolved, 0, sizeof * resolved );
    }
    return rc;
}


/* ResolveTypeid
 *  convert a type id into its type definition
 *
 *  "resolved" [ OUT ] - resolved type definition
 *
 *  "type_id" [ IN ] - 1-based runtime type id
 */
LIB_EXPORT rc_t CC VDatatypesResolveTypeid ( const VDatatypes *self,
    VTypedef *resolved, uint32_t type_id )
{
    return VSchemaDescribeTypedef ( ( const VSchema* ) self, resolved, type_id );
}


/* SizeofTypedecl
 *  returns the bit size of type declaration
 */
LIB_EXPORT rc_t CC VDatatypesSizeofTypedecl ( const VDatatypes *self,
    bitsz_t *type_size, const VTypedecl *td )
{
    rc_t rc;
    if ( type_size == NULL )
        rc = RC ( rcSRA, rcSchema, rcAccessing, rcParam, rcNull );
    else
    {
        VTypedesc desc;
        rc = VSchemaDescribeTypedecl ( ( const VSchema* ) self, & desc, td );
        if ( rc == 0 )
        {
            * type_size = VTypedescSizeof ( & desc );
            return 0;
        }

        * type_size = 0;
    }
    return rc;
}


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
LIB_EXPORT rc_t CC VDatatypesSizeofTypeid ( const VDatatypes *self,
    bitsz_t *type_size, uint32_t type_id, bool all_elements )
{
    rc_t rc;

    if ( type_size == NULL )
        rc = RC ( rcVDB, rcType, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcType, rcAccessing, rcSelf, rcNull );
        else
        {
            VTypedef td;
            rc = VDatatypesResolveTypeid ( self, & td, type_id );
            if ( rc == 0 )
            {
                * type_size = td . type_size;
                if ( all_elements )
                    * type_size *=  td . dim;
            }
        }
    }

    return rc;
}


/* MakeTypedecl
 *  convert a VTypedecl into canonical text
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated type declaration string
 *
 *  "typedecl" [ IN ] - binary representation of typedecl
 */
LIB_EXPORT rc_t CC VDatatypesMakeTypedecl ( const VDatatypes *self,
    char *buffer, size_t bsize, const VTypedecl *typedecl )
{
    return VTypedeclToText ( typedecl, ( const VSchema* ) self, buffer, bsize );
}


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
LIB_EXPORT rc_t CC VDatatypesToSupertype ( const VDatatypes *self, VTypedecl *typedecl )
{
    return VTypedeclToSupertype ( typedecl, ( const VSchema* ) self, typedecl );
}


/* ToIntrinsic - DEPRECATED
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
LIB_EXPORT rc_t CC VDatatypesToIntrinsic ( const VDatatypes *self, VTypedecl *typedecl )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcSRA, rcType, rcCasting, rcSelf, rcNull );
    else if ( typedecl == NULL )
        rc = RC ( rcSRA, rcType, rcCasting, rcParam, rcNull );
    else
    {
        const VSchema *schema = ( const VSchema* ) self;
        uint32_t last_intrinsic = VSchemaLastIntrinsicTypeId ( schema );
        if ( last_intrinsic == 0 )
            rc = RC ( rcSRA, rcType, rcCasting, rcSelf, rcInvalid );
        else for ( rc = 0; typedecl -> type_id > last_intrinsic; )
        {
            if ( ! VTypedeclToSupertype ( typedecl, schema, typedecl ) )
                break;
        }
    }

    return rc;
}


/* ToAncestor
 *  cast a typedecl to a size-equivalent ancestor decl
 *  returns true if cast can be performed
 */
LIB_EXPORT bool CC VDatatypesToAncestor ( const VDatatypes *self,
    const VTypedecl *to, const VTypedecl *from )
{
    VTypedecl cast;
    return VTypedeclToTypedecl ( from, ( const VSchema* ) self, to, & cast, NULL );
}
