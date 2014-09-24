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

#ifndef _hpp_vdb_schema_
#define _hpp_vdb_schema_

#ifndef _h_vdb_schema_
#include <vdb/schema.h>
#endif

/*--------------------------------------------------------------------------
 * VSchema
 *  a virtual table schema
 */
struct VSchema
{
    /* AddRef
     * Release
     *  all objects are reference counted
     *  NULL references are ignored
     */
    inline rc_t AddRef () const throw()
    { return VSchemaAddRef ( this ); }
    inline rc_t Release () const throw()
    { return VSchemaRelease ( this ); }


    /* AddIncludePath
     *  add an include path to schema for locating input files
     */
    inline rc_t AddIncludePath ( const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VSchemaVAddIncludePath ( this, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t AddIncludePath ( const char *path, va_list args ) throw()
    { return VSchemaVAddIncludePath ( this, path, args ); }


    /* ParseText
     *  parse schema text
     *  add productions to existing schema
     *
     *  "name" [ IN, NULL OKAY ] - optional name
     *  representing text, e.g. filename
     *
     *  "text" [ IN ] and "bytes" [ IN ] - input buffer of text
     */
    inline rc_t ParseText ( const char *name, const char *text, size_t bytes ) throw()
    { return VSchemaParseText ( this, name, text, bytes ); }


    /* ParseFile
     *  parse schema file
     *  add productions to existing schema
     *
     *  "name" [ IN ] - filename, absolute or valid relative to
     *  working directory or within an include path
     */
    inline rc_t ParseFile ( const char *name, ... ) throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VSchemaVParseFile ( this, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t ParseFile ( const char *name, va_list args ) throw()
    { return VSchemaVParseFile ( this, name, args ); }


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
    inline rc_t Dump ( uint32_t mode, const char *decl,
        rc_t ( CC * flush ) ( void *dst, const void *buffer, size_t bsize ), 
        void *dst ) const throw()
    { return VSchemaDump ( this, mode, decl, flush, dst ); }


    /* IncludeFiles
     *  generates a list of paths to include file
     */
    inline rc_t IncludeFiles ( struct KNamelist const **list ) const throw()
    { return VSchemaIncludeFiles ( this, list ); }


    /* Resolve
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
        rc_t rc = VSchemaVResolveTypedecl ( this, resolved, typedecl, args );
        va_end ( args );
        return rc;
    }
    inline rc_t ResolveTypedecl ( VTypedecl *resolved,
        const char *typedecl, va_list args ) const throw()
    { return VSchemaVResolveTypedecl ( this, resolved, typedecl, args ); }


    /* ToText
     *  convert a VTypedecl into canonical text
     *
     *  "schema" [ IN ] - schema object that would know about this type
     *
     *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
     *  NUL terminated type declaration string
     */
    inline bool TypedeclToText ( const VTypedecl *decl, char *buffer,
        size_t bsize ) const throw()
    { return VTypedeclToText ( decl, this, buffer, bsize ); }


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
    inline bool TypedeclToSupertype ( const VTypedecl *decl,
        VTypedecl *cast ) const throw()
    { return VTypedeclToSupertype ( decl, this, cast ); }


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
    inline bool TypedeclToType ( const VTypedecl *decl, uint32_t ancestor,
        VTypedecl *cast, uint32_t *distance ) const throw()
    { return VTypedeclToType ( decl, this, ancestor, cast, distance ); }


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
    inline bool TypedeclToTypedecl ( const VTypedecl *decl, const VTypedecl *ancestor,
        VTypedecl *cast, uint32_t *distance ) const throw()
    { return VTypedeclToTypedecl ( decl, this, ancestor, cast, distance ); }


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
    inline bool TypedeclCommonAncestor ( const VTypedecl *decl, const VTypedecl *peer,
        VTypedecl *ancestor, uint32_t *distance ) const throw()
    { return VTypedeclCommonAncestor ( decl, this, peer, ancestor, distance ); }


    /* DescribeTypedecl
     *  produce a description of typedecl properties
     */
    inline rc_t DescribeTypedecl ( VTypedesc *desc, const VTypedecl *td ) const throw()
    { return VSchemaDescribeTypedecl ( this, desc, td ); }


private:
    VSchema ();
    ~ VSchema ();
    VSchema ( const VSchema& );
    VSchema &operator = ( const VSchema& );
};

#endif