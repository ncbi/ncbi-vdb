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

#ifndef _hpp_vdb_table_
#define _hpp_vdb_table_

#ifndef _h_vdb_table_
#include <vdb/table.h>
#endif

#ifndef _h_vdb_cursor_
#include <vdb/cursor.h>
#endif


/*--------------------------------------------------------------------------
 * VTable
 *  a collection of columns indexed by row id, metadata, indices
 */
struct VTable
{
    /* AddRef
     * Release
     *  all objects are reference counted
     *  NULL references are ignored
     */
    inline rc_t AddRef ( const VTable *self ) const throw()
    { return VTableAddRef ( this ); }
    inline rc_t Release ( const VTable *self ) const throw()
    { return VTableRelease ( this ); }

    /* Locked
     *  returns true if locked
     */
    inline bool Locked () const throw()
    { return VTableLocked ( this ); }


    /* Writable
     *  returns 0 if object is writable
     *  or a reason why if not
     *
     *  "type" [ IN ] - a KDBPathType
     *  valid values are kptIndex and kptColumn
     *
     *  "path" [ IN ] - NUL terminated path
     */
     inline rc_t Writable ( uint32_t type, 
        const char * name, ... ) const throw()
     {
        va_list args;
        va_start ( args, name );
        rc_t rc = VTableVWritable ( this, type, name, args );
        va_end ( args );
        return rc;
     }
     inline rc_t Writable ( uint32_t type, 
        const char * name, va_list args ) const throw()
    { return VTableVWritable ( this, type, name, args ); }


    /* Lock
     *  apply lock
     *
     *  if object is already locked, the operation is idempotent
     *  and returns an rc state of rcLocked
     *
     *  "type" [ IN ] - a KDBPathType
     *  valid values are kptIndex and kptColumn
     *
     *  "name" [ IN ] - NUL terminated path
     */
    inline rc_t Lock ( uint32_t type, const char * name, ... ) throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VTableVLock ( this, type, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t Lock ( uint32_t type, const char * name, va_list args ) throw()
    { return VTableVLock ( this, type, name, args ); }

    
    /* Unlock
     *  remove lock
     *
     *  if object is already unlocked, the operation is idempotent
     *  and returns an rc state of rcUnlocked
     *
     *  "type" [ IN ] - a KDBPathType
     *  valid values are kptIndex and kptColumn
     *
     *  "name" [ IN ] - NUL terminated path
     */
     inline rc_t Unlock ( uint32_t type, const char * name, ... ) throw()
     {
        va_list args;
        va_start ( args, name );
        rc_t rc = VTableVUnlock ( this, type, name, args );
        va_end ( args );
        return rc;
     }
    inline rc_t UnLock ( uint32_t type, const char * name, va_list args ) throw()
    { return VTableVUnlock ( this, type, name, args ); }


    /* OpenMetadataRead
     * OpenMetadataUpdate
     *  opens metadata file
     *
     *  "meta" [ OUT ] - return parameter for metadata
     */
    inline rc_t OpenMetadataRead ( struct KMetadata const **meta ) const throw()
    { return VTableOpenMetadataRead ( this, meta ); }
    inline rc_t OpenMetadataUpdate ( struct KMetadata **meta ) throw()
    { return VTableOpenMetadataUpdate ( this, meta ); }
    

    /* ColumnCreateParams
     *  sets the creation parameters for physical columns
     *
     *  "cmode" [ IN ] - creation mode
     *
     *  "checksum" [ IN ] - the type of checksum information to
     *  apply when writing blobs
     *
     *  "pgsize" [ IN, DEFAULT ZERO ] - size of internal column "pages"
     *  the default value is indicated by 0 ( zero ).
     *  NB - CURRENTLY THE ONLY SUPPORTED PAGE SIZE IS 1 ( ONE ) BYTE.
     */
    inline rc_t ColumnCreateParams ( KCreateMode cmode, 
        KChecksum checksum, size_t pgsize ) throw()
    { return VTableColumnCreateParams ( this, cmode, checksum, pgsize ); }


    /* CreateIndex
     * VCreateIndex
     *  create a new or open an existing index
     *
     *  "idx" [ OUT ] - return parameter for newly opened index
     *
     *  "type" [ IN ] - type of index to create
     *
     *  "cmode" [ IN ] - creation mode
     *
     *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
     */
    inline rc_t CreateIndex ( KIndex **idx,
        KIdxType type, KCreateMode cmode, const char *name, ... ) throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VTableVCreateIndex ( this, idx, type, cmode, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t CreateIndex ( KIndex **idx,
        KIdxType type, KCreateMode cmode, const char *name, va_list args ) throw()
    { return VTableVCreateIndex ( this, idx, type, cmode, name, args ); }


    /* OpenIndexRead
     * VOpenIndexRead
     *  open an index for read
     *
     *  "idx" [ OUT ] - return parameter for newly opened index
     *
     *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
     */
    inline rc_t OpenIndexRead ( const KIndex **idx,
        const char *name, ... ) const throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VTableVOpenIndexRead ( this, idx, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenIndexRead ( const KIndex **idx,
        const char *name, va_list args ) const throw()
    { return VTableVOpenIndexRead ( this, idx, name, args ); }


    /* OpenIndexUpdate
     * VOpenIndexUpdate
     *  open an index for read/write
     *
     *  "idx" [ OUT ] - return parameter for newly opened index
     *
     *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
     */
    inline rc_t OpenIndexUpdate ( KIndex **idx,
        const char *name, ... ) throw()
    {
        va_list args;
        va_start ( args, name );
        rc_t rc = VTableVOpenIndexUpdate ( this, idx, name, args );
        va_end ( args );
        return rc;
    }
    inline rc_t OpenIndexUpdate ( KIndex **idx,
        const char *name, va_list args ) throw()
    { return VTableVOpenIndexUpdate ( this, idx, name, args ); }


    /* List
     *  create table listings
     */
    inline rc_t ListCol ( struct KNamelist **names ) const throw()
    { return VTableListCol ( this, names ); }


    /* ColumnDatatypes
     *  returns list of typedecls for named column
     *
     *  "col" [ IN ] - column name
     *
     *  "dflt_idx" [ OUT, NULL OKAY ] - returns the zero-based index
     *  into "typedecls" of the default datatype for the named column
     *
     *  "typedecls" [ OUT ] - list of datatypes available for named column
     */
    inline rc_t ColumnDatatypes ( const char *col,
        uint32_t *dflt_idx, struct KNamelist **typedecls ) const throw()
    { return VTableColumnDatatypes ( this, col, dflt_idx, typedecls ); }


    /* Reindex
     *  optimize column indices
     */
    inline rc_t Reindex () throw()
    { return VTableReindex ( this ); }


    /* OpenManager
     *  duplicate reference to manager
     *  NB - returned reference must be released
     */
    inline rc_t OpenManager ( struct VDBManager const **mgr ) const throw()
    { return VTableOpenManagerRead ( this, mgr ); }
    inline rc_t OpenManager ( struct VDBManager **mgr ) throw()
    { return VTableOpenManagerUpdate ( this, mgr ); }


    /* OpenParent
     *  duplicate reference to parent database
     *  NB - returned reference must be released
     */
    inline rc_t OpenParent ( struct VDatabase const **db ) const throw()
    { return VTableOpenParentRead ( this, db ); }
    inline rc_t OpenParent ( struct VDatabase **db ) throw()
    { return VTableOpenParentUpdate ( this, db ); }


    /* OpenSchema
     *  duplicate reference to table schema
     *  NB - returned reference must be released
     */
    inline rc_t OpenSchema ( struct VSchema const **schema ) const throw()
    { return  VTableOpenSchema ( this, schema ); }


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
    { return VTableGetUserData ( this, data ); }
    inline rc_t SetUserData ( void *data,
        void ( CC * destroy ) ( void *data ) ) const throw()
    { return VTableSetUserData ( this, data, destroy ); }


    /* CreateCursor
     *  creates a cursor object onto table
     *  multiple read cursors are allowed
     *  only a single write cursor is allowed
     *
     *  "curs" [ OUT ] - return parameter for newly created cursor
     *
     *  "mode" [ IN ] - describes update behavior
     *    kcmUpdate   : allow inserts or updates
     *    kcmReplace  : replace all existing rows with newly written rows
     *    kcmInsert   : allow only inserts, i.e. new rows
     */
    inline rc_t CreateCursor ( const VCursor **curs ) const throw()
    { return VTableCreateCursorRead ( this, curs ); }
    inline rc_t CreateCursor ( VCursor **curs, KCreateMode mode ) throw()
    { return VTableCreateCursorWrite ( this, curs, mode ); }

private:
    VTable ();
    ~ VTable ();
    VTable ( const VTable& );
    VTable &operator = ( const VTable& );
};

#endif //  _hpp_vdb_table_
