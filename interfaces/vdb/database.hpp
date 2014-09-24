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

#ifndef _hpp_vdb_database_
#define _hpp_vdb_database_

#ifndef _h_vdb_database_
#include <vdb/database.h>
#endif

#ifndef _h_vdb_table_
#include <vdb/table.h>
#endif

/*--------------------------------------------------------------------------
 * VDatabase
 *  opaque connection to a database within file system
 */
struct VDatabase
{

    /* AddRef
     * Release
     *  all objects are reference counted
     *  NULL references are ignored
    */
    inline rc_t AddRef () const throw ()
    { return VDatabaseAddRef ( this ); }
    inline rc_t Release () const throw ()
    { return VDatabaseRelease ( this ); }


    /* CreateDB
     *  create a new or open an existing database under parent database
     *
     *  "db" [ OUT ] - return parameter for newly opened database
     *
     *  "member" [ IN ] - name of database member template under parent
     *  the named member is a db template rather than a named database.
     *
     *  "cmode" [ IN ] - creation mode
     *
     *  "name" [ IN ] - NUL terminated string in
     *  db-native character set giving actual table name
    */
    inline rc_t CreateDB ( VDatabase **db, const char *member,
        KCreateMode cmode, const char *name, ... ) throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVCreateDB ( this, db, member, cmode, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CreateDB ( VDatabase **db, const char *member,
        KCreateMode cmode, const char *name, va_list args ) throw ()
    { return VDatabaseVCreateDB ( this, db, member, cmode, name, args ); }


    /* OpenDBRead
     *  open a database for read under parent db
     * OpenDBUpdate
     *  open or create a database for read/write under parent db
     *
     *  "db" [ OUT ] - return parameter for newly opened database
     *
     *  "name" [ IN ] - NUL terminated string in
     *  db-native character set giving name of database member
     */
    inline rc_t OpenDB ( const VDatabase **db, const char *name, ... ) const throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVOpenDBRead ( this, db, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenDB ( const VDatabase **db, const char *name, va_list args ) const throw ()
    { return VDatabaseVOpenDBRead ( this, db, name, args ); }

    inline rc_t OpenDB ( VDatabase **db, const char *name, ... ) throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVOpenDBUpdate ( this, db, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenDB ( VDatabase **db, const char *name, va_list args ) throw ()
    { return VDatabaseVOpenDBUpdate ( this, db, name, args ); }


    /* CreateTable
     *  create a new or open an existing table under database
     *
     *  "tbl" [ OUT ] - return parameter for newly opened table
     *
     *  "member" [ IN ] - name of table member template under database
     *  the named member is a table template rather than a named table.
     *
     *  "cmode" [ IN ] - creation mode
     *
     *  "name" [ IN ] - NUL terminated string in
     *  db-native character set giving actual table name
     */
    inline rc_t CreateTable ( VTable **tbl, const char *member,
        KCreateMode cmode, const char *name, ... ) throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVCreateTable ( this, tbl, member, cmode, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CreateTable ( VTable **tbl, const char *member,
        KCreateMode cmode, const char *name, va_list args ) throw ()
    { return VDatabaseVCreateTable ( this, tbl, member, cmode, name, args ); }


    /* OpenTableRead
     *  open a table for read under database
     * OpenTableUpdate
     *  open or create a table for read/write under database
     *
     *  "tbl" [ OUT ] - return parameter for newly opened table
     *
     *  "name" [ IN ] - NUL terminated string in
     *  db-native character set giving name of table member
     */
    inline rc_t OpenTable( const VTable **tbl,
        const char *name, ... ) const throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVOpenTableRead ( this, tbl, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenTable( const VTable **tbl,
        const char *name, va_list args ) const throw ()
    { return VDatabaseVOpenTableRead ( this, tbl, name, args ); }

    inline rc_t OpenTable( VTable **tbl, const char *name, ... ) throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVOpenTableUpdate ( this, tbl, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenTable( VTable **tbl, const char *name, va_list args ) throw ()
    { return VDatabaseVOpenTableUpdate ( this, tbl, name, args ); }


    /* Locked
     *  returns true if locked
     */
    inline bool Locked ( ) const throw ()
    { return VDatabaseLocked ( this ); }


    /* Writable
     *  returns 0 if object is writable
     *  or a reason why if not
     *
     *  "type" [ IN ] - a KDBPathType
     *  valid values are kptDatabase, kptTable and kptIndex
     *
     *  "path" [ IN ] - NUL terminated path
     */
    inline rc_t Writable ( uint32_t type, const char *name, ... ) const throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVWritable ( this, type, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Writable ( uint32_t type, const char *name, va_list args ) const throw ()
    { return VDatabaseVWritable ( this, type, name, args ); }


    /* Lock
     *  apply lock
     *
     *  if object is already locked, the operation is idempotent
     *  and returns an rc state of rcLocked
     *
     *  "type" [ IN ] - a KDBPathType
     *  valid values are kptDatabase, kptTable and kptIndex
     *
     *  "path" [ IN ] - NUL terminated path
     */
    inline rc_t CC Lock ( uint32_t type, const char *name, ... ) throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVLock ( this, type, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CC Lock ( uint32_t type, const char *name, va_list args ) throw ()
    { return VDatabaseVLock ( this, type, name, args ); }


    /* Unlock
     *  remove lock
     *
     *  if object is already unlocked, the operation is idempotent
     *  and returns an rc state of rcUnlocked
     *
     *  "type" [ IN ] - a KDBPathType
     *  valid values are kptDatabase, kptTable and kptIndex
     *
     *  "path" [ IN ] - NUL terminated path
     */
    inline rc_t CC Unlock ( uint32_t type, const char *name, ... ) throw ()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VDatabaseVUnlock ( this, type, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CC Unlock ( uint32_t type, const char *name, va_list args ) throw ()
    { return VDatabaseVUnlock ( this, type, name, args ); }


    /* OpenMetadataRead
     * OpenMetadataUpdate
     *  opens metadata file
     *
     *  "meta" [ OUT ] - return parameter for metadata
     */
    inline rc_t OpenMetadata ( const KMetadata **meta ) const throw ()
    { return VDatabaseOpenMetadataRead ( this, meta ); }
    inline rc_t OpenMetadata ( KMetadata **meta ) throw ()
    { return VDatabaseOpenMetadataUpdate ( this, meta ); }


    /* OpenManager
     *  duplicate reference to manager
     *  NB - returned reference must be released
     */
    inline rc_t OpenManager ( const VDBManager **mgr ) const throw ()
    { return VDatabaseOpenManagerRead ( this, mgr ); }
    inline rc_t OpenManager ( VDBManager **mgr ) throw ()
    { return VDatabaseOpenManagerUpdate ( this, mgr ); }


    /* OpenParent
     *  duplicate reference to parent database
     *  NB - returned reference must be released
     */
    inline rc_t OpenParent ( const VDatabase **par ) const throw ()
    { return VDatabaseOpenParentRead ( this, par ); }
    inline rc_t OpenParent ( VDatabase **par ) throw ()
    { return VDatabaseOpenParentUpdate ( this, par ); }


    /* lists the tables of the database
     *
     * "names" [ OUT ] - return parameter for table 
     */
    inline rc_t ListTbl( KNamelist **names ) const throw ()
    { return VDatabaseListTbl( this, names ); }


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
    inline rc_t GetUserData ( void **data ) const throw ()
    { return VDatabaseGetUserData ( this, data ); }
    inline rc_t SetUserData ( void *data,
        void ( CC * destroy ) ( void *data ) ) throw ()
    { return VDatabaseSetUserData ( this, data, destroy ); }


private:
    VDatabase ();
    ~ VDatabase ();
    VDatabase ( const VDatabase& );
    VDatabase &operator = ( const VDatabase& );
};

#endif
