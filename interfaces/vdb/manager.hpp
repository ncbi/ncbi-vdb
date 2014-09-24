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

#ifndef _hpp_vdb_manager_
#define _hpp_vdb_manager_

#ifndef _h_vdb_manager_
#include <vdb/manager.h>
#endif

#ifndef _h_vdb_table_
#include <vdb/table.h>
#endif

#ifndef _h_vdb_schema_
#include <vdb/schema.h>
#endif

#ifndef _h_vdb_database_
#include <vdb/database.h>
#endif

/*--------------------------------------------------------------------------
 * forward decl. for CreateDB
 */
struct VDatabase;
struct VSchema;

/*--------------------------------------------------------------------------
 * VDBManager
 *  opaque handle to library
 */
struct VDBManager
{
    /* AddRef
     * Release
     *  all objects are reference counted
     *  NULL references are ignored
     */
    inline rc_t AddRef () const throw()
    { return VDBManagerAddRef ( this ); }

    inline rc_t Release () const throw()
    { return VDBManagerRelease ( this ); }


    /* CreateDB
     *  create a new or open an existing database using manager
     *
     *  "db" [ OUT ] - return parameter for newly opened database
     *
     *  "schema" [ IN ] - schema object containg database
     *  declaration to be used in creating db.
     *
     *  "typespec" [ IN ] - type and optionally version of db schema,
     *  e.g. 'MY_NAMESPACE:MyDatabase' or 'MY_NAMESPACE:MyDatabase#1.1'
     *
     *  "cmode" [ IN ] - creation mode
     *
     *  "path" [ IN ] - NUL terminated string in
     *  wd-native character set giving path to database
     */
    inline rc_t CreateDB ( VDatabase **db, struct VSchema const *schema,
        const char *typespec, KCreateMode cmode, const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVCreateDB ( this, db, schema, typespec, cmode, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CreateDB ( VDatabase **db, struct VSchema const *schema,
        const char *typespec, KCreateMode cmode, const char *path, va_list args ) throw()
    { return VDBManagerVCreateDB ( this, db, schema, typespec, cmode, path, args ); }


    /* OpenDBRead
     *  open a database for read using manager
     * OpenDBUpdate
     *  open a database for read/write using manager
     *
     *  "db" [ OUT ] - return parameter for newly opened database
     *
     *  "schema" [ IN, NULL OKAY ] - schema object containg database
     *  declaration to be used in creating db [ needed by manager ].
     *
     *  "path" [ IN ] - NUL terminated string in
     *  wd-native character set giving path to database
     */
    inline rc_t OpenDB( const VDatabase **db, struct VSchema const *schema,
        const char *path, ... ) const throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVOpenDBRead ( this, db, schema, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenDB( const VDatabase **db, struct VSchema const *schema,
        const char *path, va_list args ) const throw()
    { return VDBManagerVOpenDBRead ( this, db, schema, path, args ); }

    inline rc_t OpenDB( VDatabase **db, struct VSchema const *schema,
        const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVOpenDBUpdate ( this, db, schema, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenDB( VDatabase **db, struct VSchema const *schema,
        const char *path, va_list args ) throw()
    { return VDBManagerVOpenDBUpdate ( this, db, schema, path, args ); }


    /* CreateTable
     *  create a new or open an existing table using manager
     *
     *  "tbl" [ OUT ] - return parameter for newly opened table
     *
     *  "schema" [ IN ] - schema object containg table
     *  declaration to be used in creating tbl.
     *
     *  "typespec" [ IN ] - type and optionally version of table schema,
     *  e.g. 'MY_NAMESPACE:MyTable' or 'MY_NAMESPACE:MyTable#1.1'
     *
     *  "cmode" [ IN ] - creation mode
     *
     *  "path" [ IN ] - NUL terminated string in
     *  wd-native character set giving path to table
     */
    inline rc_t CreateTable ( VTable **tbl, struct VSchema const *schema,
        const char *typespec, KCreateMode cmode, const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVCreateTable ( this, tbl, schema, typespec, cmode, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CreateTable ( VTable **tbl, struct VSchema const *schema,
        const char *typespec, KCreateMode cmode, const char *path, va_list args ) throw()
    { return VDBManagerVCreateTable ( this, tbl, schema, typespec, cmode, path, args ); }


    /* OpenTableRead
     *  open a table for read using manager
     * OpenTableUpdate
     *  open a table for read/write using manager
     *
     *  "tbl" [ OUT ] - return parameter for newly opened table
     *
     *  "schema" [ IN, NULL OKAY ] - optional schema object with
     *  latest versions of table schema.
     *
     *  "path" [ IN ] - NUL terminated string in
     *  wd-native character set giving path to table
     */
    inline rc_t OpenTable ( const VTable **tbl, struct VSchema const *schema,
        const char *path, ... ) const throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVOpenTableRead ( this, tbl, schema, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenTable ( const VTable **tbl, struct VSchema const *schema,
        const char *path, va_list args ) const throw ()
    { return VDBManagerVOpenTableRead ( this, tbl, schema, path, args ); }

    inline rc_t OpenTable ( VTable **tbl, struct VSchema const *schema,
        const char *path, ... ) throw ()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVOpenTableUpdate ( this, tbl, schema, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenTable ( VTable **tbl, struct VSchema const *schema,
        const char *path, va_list args ) throw ()
    { return VDBManagerVOpenTableUpdate ( this, tbl, schema, path, args ); }


    /* MakeSchema
     *  create a schema object
     */
    inline rc_t MakeSchema ( VSchema **schema ) const throw()
    { return VDBManagerMakeSchema ( this, schema ); }


    /* MakeRead
     * MakeUpdate
     *  create library handle for specific use
     *  NB - only one of the functions will be implemented
     *
     *  "wd" [ IN, NULL OKAY ] - optional working directory for
     *  accessing the file system. mgr will attach its own reference.
     */
    static inline rc_t MakeRead ( const VDBManager **mgr,
        const KDirectory *wd = 0 ) throw()
    { return VDBManagerMakeRead ( mgr, wd ); }

    static inline rc_t MakeUpdate ( VDBManager **mgr,
        KDirectory *wd = 0 ) throw()
    { return VDBManagerMakeUpdate ( mgr, wd ); }


    /* Version
     *  returns the library version
     */
    inline rc_t Version ( uint32_t *version ) const throw()
    { return VDBManagerVersion ( this, version ); }


    /* Writable
     *  returns 0 if object is writable
     *  or a reason why if not
     *
     *  "path" [ IN ] - NUL terminated path
     */
    inline rc_t Writable ( const char *path, ... ) const throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVWritable ( this, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Writable ( const char *path, va_list args ) const throw()
    { return VDBManagerVWritable ( this, path, args ); }


    /* Lock
     *  apply lock
     *
     *  if object is already locked, the operation is idempotent
     *  and returns an rc state of rcLocked
     *
     *  "path" [ IN ] - NUL terminated path
     */
    inline rc_t Lock ( const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVLock ( this, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Lock ( const char *path, va_list args ) throw()
    { return VDBManagerVLock ( this, path, args ); }


    /* Unlock
     *  remove lock
     *
     *  if object is already unlocked, the operation is idempotent
     *  and returns an rc state of rcUnlocked
     *
     *  "path" [ IN ] - NUL terminated path
     */
    inline rc_t Unlock ( const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVUnlock ( this, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Unlock ( const char *path, va_list args ) throw()
    { return VDBManagerVUnlock ( this, path, args ); }


    /* Drop
     *  drop an object based on its path
     *
     *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to the vdb object
     */
    inline rc_t Drop( uint32_t obj_type, const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVDrop ( this, obj_type, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Drop( uint32_t obj_type, const char *path, va_list args ) throw()
    { return VDBManagerVDrop ( this, obj_type, path, args ); }


    /* AddSchemaIncludePath
     *  add a path to schema for locating input files
     */
    inline rc_t AddSchemaIncludePath ( const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVAddSchemaIncludePath ( this, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t AddSchemaIncludePath ( const char *path, va_list args ) throw()
    { return VDBManagerVAddSchemaIncludePath ( this, path, args ); }


    /* AddLoadLibraryPath
     *  add a path to loader for locating dynamic libraries
     */
    inline rc_t AddLoadLibraryPath ( const char *path, ... ) throw()
    {
        va_list args;
        va_start ( args, path );
        rc_t rc = VDBManagerVAddLoadLibraryPath ( this, path, args );
        va_end ( args );
        return rc;
    }
    inline rc_t AddLoadLibraryPath ( const char *path, va_list args ) throw()
    { return VDBManagerVAddLoadLibraryPath ( this, path, args ); }


    /* GetUserData
     * SetUserData
     *  store/retrieve an opaque pointer to user data
     *
     *  "data" [ OUT ] - return parameter for getting data
     *  "data" [ IN ] - parameter for setting data
     *
     *  "destroy" [ IN, NULL OKAY ] - optional destructor param
     *  invoked from destructor of "self"
     */
    inline rc_t GetUserData ( void **data ) const throw()
    { return VDBManagerGetUserData ( this, data ); }

    inline rc_t SetUserData ( void *data,
        void ( CC * destroy ) ( void *data ) = 0 ) const throw()
    { return VDBManagerSetUserData ( this, data, destroy ); }


private:
    VDBManager ();
    ~ VDBManager ();
    VDBManager ( const VDBManager& );
    VDBManager &operator = ( const VDBManager& );
};

#endif // _hpp_vdb_manager_
