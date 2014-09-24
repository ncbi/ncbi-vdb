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

#include <vdb/extern.h>
#include "table-priv.h"
#include "cursor-priv.h"
#include "dbmgr-priv.h"
#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-dump.h"
#include "database-priv.h"
#include "linker-priv.h"

#include <vdb/schema.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/table.h>
#include <kdb/index.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <klib/symbol.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VTable
 *  a collection of columns indexed by row id, metadata, indices
 */


/* StoreSchema
 *  stores schema definition in metadata
 *
 *  <schema name="">...</schema>
 */
LIB_EXPORT rc_t VTableStoreSchema ( VTable *self )
{
    /* open schema node */
    KMDataNode *node;
    rc_t rc = KMetadataOpenNodeUpdate ( self -> meta, & node, "schema" );
    if ( rc == 0 )
    {
        size_t num_writ;
        char expr [ 256 ];
        rc = VSchemaToText ( self -> schema, expr, sizeof expr - 1, & num_writ,
            "%N%V", self -> stbl -> name, self -> stbl -> version );
        if ( rc != 0 )
            LOGERR ( klogInt, rc, "failed to determine table schema" );
        else
        {
            expr [ num_writ ] = 0;

            /* if table has a default view declaration,
               store the table information under a new attribute */
            if ( self -> stbl -> dflt_view != NULL )
            {
                uint32_t type;
                const SNameOverload *name;
                const STable *view = VSchemaFind ( self -> schema, & name, & type,
                    self -> stbl -> dflt_view-> addr, __func__, false );
                if ( view == NULL )
                {
                    rc = RC ( rcVDB, rcTable, rcUpdating, rcSchema, rcNotFound );
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to locate default view schema '$(expr)'",
                                         "expr=%S", self -> stbl -> dflt_view ));
                }
                else
                {
                    rc = KMDataNodeWriteAttr ( node, "table", expr );
                    if ( rc != 0 )
                        PLOGERR ( klogInt, ( klogInt, rc, "failed to write table type '$(expr)'", "expr=%s", expr ));
                    else
                    {
                        rc = VSchemaToText ( self -> schema, expr, sizeof expr - 1, & num_writ,
                            "%N%V", view -> name, view -> version );
                        if ( rc != 0 )
                            LOGERR ( klogInt, rc, "failed to determine table default view schema" );
                        else
                            expr [ num_writ ] = 0;
                    }
                }
            }

            if ( rc == 0 )
            {
                rc = KMDataNodeWriteAttr ( node, "name", expr );
                if ( rc != 0 )
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to write table name '$(expr)'", "expr=%s", expr ));
            }
            if ( rc == 0 )
            {
                /* truncate existing schema */
                rc = KMDataNodeWrite ( node, "", 0 );
                if ( rc == 0 )
                {
                    rc = VSchemaDump ( self -> schema, sdmCompact, expr,
                        ( rc_t ( CC * ) ( void*, const void*, size_t ) ) KMDataNodeAppend, node );
                }
                if ( rc != 0 )
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to write table schema '$(expr)'", "expr=%s", expr ));
            }
        }

        KMDataNodeRelease ( node );
    }
    return rc;
}


/* OpenUpdate
 *  finish create operation
 */
static
rc_t VTableOpenUpdate ( VTable *self, const char *decl )
{
    /* open metadata */
    rc_t rc = KTableOpenMetadataUpdate ( self -> ktbl, & self -> meta );
    if ( rc == 0 )
    {
        /* open "col" node */
        rc = KMetadataOpenNodeUpdate ( self -> meta, & self -> col_node, "col" );
        if ( rc == 0 )
        {
            /* fetch stored schema */
            rc = VTableLoadSchema ( self );
            if ( rc == 0 )
            {
                /* fetch requested schema */
                const STable *stbl = self -> stbl;
                if ( decl != NULL && decl [ 0 ] != 0 )
                {
                    uint32_t type;
                    const SNameOverload *name;

                    if ( self -> db != NULL )
                    {
                        const STblMember *mbr = SDatabaseFind ( self -> db -> sdb,
                            self -> schema, & name, & type, decl, "VTableOpenUpdate" );
                        if ( mbr == NULL || type != eTblMember )
                        {
                            PLOGMSG ( klogWarn, ( klogWarn, "expression '$(expr)' is not a table member",
                                       "expr=%s", decl ));
                            stbl = NULL;
                        }
                        else
                        {
                            stbl = mbr -> tbl;
                            assert ( stbl != NULL );
                        }
                    }
                    else
                    {
                        stbl = VSchemaFind ( self -> schema,
                            & name, & type, decl, "VTableOpenUpdate", true );
                        if ( stbl != NULL && type != eTable )
                        {
                            PLOGMSG ( klogWarn, ( klogWarn, "expression '$(expr)' is not a table",
                                       "expr=%s", decl ));
                            stbl = NULL;
                        }
                    }
                }

                /* error if the two definitions differ */
                if ( stbl != NULL && self -> stbl != NULL && stbl != self -> stbl )
                    rc = RC ( rcVDB, rcTable, rcOpening, rcSchema, rcIncorrect );
                else if ( stbl == NULL && self -> stbl == NULL )
                    rc = RC ( rcVDB, rcTable, rcOpening, rcSchema, rcNotFound );
                else if ( self -> stbl == NULL )
                {

                    /* write schema to metadata */
                    self -> stbl = stbl;
                    rc = VTableStoreSchema ( self );
                }
                else if ( stbl != NULL )
                {
                    /* use latest schema but don't overwrite in metadata */
                    self -> stbl = stbl;
                }
            }
        }
    }

    DBGMSG(DBG_VDB, DBG_FLAG(DBG_VDB_VDB), ("VTableOpenUpdate = %d\n", rc));

    return rc;
}


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
LIB_EXPORT rc_t CC VDBManagerVCreateTable ( VDBManager *self, VTable **tblp,
    const VSchema *schema, const char *typespec,
    KCreateMode cmode, const char *path, va_list args )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcMgr, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcCreating, rcSelf, rcNull );
        else if ( schema == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSchema, rcNull );
        else if ( typespec == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcNull );
        else if ( typespec [ 0 ] == 0 )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcEmpty );
        else
        {
            /* create object with new schema */
            rc = VTableMake ( tblp, self, NULL, schema );
            if ( rc == 0 )
            {
                VTable *tbl = * tblp;

                /* create physical object */
                rc = KDBManagerVCreateTable ( self -> kmgr, & tbl -> ktbl, cmode, path, args );
                if ( rc == 0 )
                {
                    rc = VTableOpenUpdate ( tbl, typespec );
                    if ( rc == 0 )
                    {
#if LAZY_OPEN_COL_NODE
                        KMDataNodeRelease ( tbl -> col_node );
                        tbl -> col_node = NULL;
#endif
                        return 0;
                    }

                    rc = ResetRCContext ( rc, rcVDB, rcMgr, rcCreating );
                }
                VTableWhack ( tbl );
            }
        }

        * tblp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDBManagerCreateTable ( VDBManager *self, VTable **tbl,
    const VSchema *schema, const char *typespec,
    KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVCreateTable ( self, tbl, schema, typespec, cmode, path, args );
    va_end ( args );

    return rc;
}

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
LIB_EXPORT rc_t CC VDatabaseVCreateTable ( VDatabase *self, VTable **tblp,
    const char *member, KCreateMode cmode, const char *name, va_list args )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcCreating, rcSelf, rcNull );
        else if ( member == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcNull );
        else if ( member [ 0 ] == 0 )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcEmpty );
        else if ( self -> read_only )
            rc = RC ( rcVDB, rcDatabase, rcCreating, rcDatabase, rcReadonly );
        else
        {
            rc = VTableMake ( tblp, self -> mgr, self, self -> schema );
            if ( rc == 0 )
            {
                VTable *tbl = * tblp;

                rc = KDatabaseVCreateTable ( self -> kdb, & tbl -> ktbl, cmode, name, args );
                if ( rc == 0 )
                {
                    rc = VTableOpenUpdate ( tbl, member );
                    if ( rc == 0 )
                    {
                        tbl -> pgsize = self -> pgsize;
                        tbl -> cmode = self -> cmode;
                        tbl -> checksum = self -> checksum;
#if LAZY_OPEN_COL_NODE
                        KMDataNodeRelease ( tbl -> col_node );
                        tbl -> col_node = NULL;
#endif
                        return 0;
                    }

                    rc = ResetRCContext ( rc, rcVDB, rcDatabase, rcCreating );
                }

                VTableWhack ( tbl );
            }
        }

        * tblp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseCreateTable ( VDatabase *self, VTable **tbl,
    const char *member, KCreateMode cmode, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VDatabaseVCreateTable ( self, tbl, member, cmode, name, args );
    va_end ( args );

    return rc;
}


/* OpenTableUpdate
 * VOpenTableUpdate
 *  open a table for read/write
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving name of table
 */
LIB_EXPORT rc_t CC VDBManagerVOpenTableUpdate ( VDBManager *self, VTable **tblp,
    const VSchema *schema, const char *path, va_list args )
{
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSelf, rcNull );
        else
        {
            /* if no schema is given, always pass intrinsic */
            if ( schema == NULL )
                schema = self -> schema;

            rc = VTableMake ( tblp, self, NULL, schema );
            if ( rc == 0 )
            {
                VTable *tbl = * tblp;
                rc = KDBManagerVOpenTableUpdate ( self -> kmgr, & tbl -> ktbl, path, args );
                if ( rc == 0 )
                {
                    rc = VTableOpenUpdate ( tbl, NULL );
                    if ( rc == 0 )
                    {
#if LAZY_OPEN_COL_NODE
                        KMDataNodeRelease ( tbl -> col_node );
                        tbl -> col_node = NULL;
#endif
                        return 0;
                    }
                }
                VTableWhack ( tbl );
            }
        }
        * tblp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDBManagerOpenTableUpdate ( VDBManager *self,
    VTable **tbl, const VSchema *schema, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVOpenTableUpdate ( self, tbl, schema, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVOpenTableUpdate ( VDatabase *self,
    VTable **tblp, const char *name, va_list args )
{ 
    rc_t rc;

    if ( tblp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSelf, rcNull );
        else
        {
            rc = VTableMake ( tblp, self -> mgr, self, self -> schema );
            if ( rc == 0 )
            {
                VTable *tbl = * tblp;

                rc = KDatabaseVOpenTableUpdate ( self -> kdb, & tbl -> ktbl, name, args );
                if ( rc == 0 )
                {
                    rc = VTableOpenUpdate ( tbl, NULL );
                    if ( rc == 0 )
                    {
#if LAZY_OPEN_COL_NODE
                        KMDataNodeRelease ( tbl -> col_node );
                        tbl -> col_node = NULL;
#endif
                        return 0;
                    }
                }

                VTableWhack ( tbl );
            }
        }

        * tblp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseOpenTableUpdate ( VDatabase *self, VTable **tbl, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VDatabaseVOpenTableUpdate ( self, tbl, name, args );
    va_end ( args );

    return rc;
}


/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VTableVLock ( VTable *self, uint32_t type, const char * name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcTable, rcLocking, rcSelf, rcNull );
    else
        rc = KTableVLock ( self -> ktbl, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VTableLock ( VTable *self, uint32_t type, const char * name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VTableVLock ( self, type, name, args );

    va_end ( args );

    return rc;
}


/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VTableVUnlock ( VTable *self, uint32_t type,
    const char * name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcTable, rcUnlocking, rcSelf, rcNull );
    else
        rc = KTableVUnlock ( self -> ktbl, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VTableUnlock ( VTable *self, uint32_t type,
    const char * name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VTableVUnlock ( self, type, name, args );

    va_end ( args );

    return rc;
}


/* OpenMetadataUpdate
 *  opens metadata file
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
LIB_EXPORT rc_t CC VTableOpenMetadataUpdate ( VTable *self, struct KMetadata **meta )
{
    rc_t rc;
    if ( meta == NULL )
        rc = RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
        else
        {
            * meta = self -> meta;
            return KMetadataAddRef ( self -> meta );
        }

        * meta = NULL;
    }

    return rc;
}


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
LIB_EXPORT rc_t CC VTableCreateIndex ( VTable *self, struct KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VTableVCreateIndex ( self, idx, type, cmode, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VTableVCreateIndex ( VTable *self, struct KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, va_list args )
{
    if ( self != NULL )
        return KTableVCreateIndex ( self -> ktbl, idx, type, cmode, name, args );

    if ( idx == NULL )
        return RC ( rcVDB, rcTable, rcCreating, rcParam, rcNull );

    return RC ( rcVDB, rcTable, rcCreating, rcSelf, rcNull );
}


/* OpenIndexUpdate
 * VOpenIndexUpdate
 *  open an index for read/write
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */

LIB_EXPORT rc_t CC VTableOpenIndexUpdate ( VTable *self,
    struct KIndex **idx, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VTableVOpenIndexUpdate ( self, idx, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VTableVOpenIndexUpdate ( VTable *self,
    struct KIndex **idx, const char *name, va_list args )
{
    if ( self != NULL )
        return KTableVOpenIndexUpdate ( self -> ktbl, idx, name, args );

    if ( idx == NULL )
        return RC ( rcVDB, rcTable, rcOpening, rcParam, rcNull );

    return RC ( rcVDB, rcTable, rcOpening, rcSelf, rcNull );
}


/* ListColumns
 *  list writable column names
 *
 *  "names" [ OUT ] - return parameter for namelist
 *
 *  availability: v2.1
 */
static
rc_t list_writable_columns ( VTable *self )
{
    rc_t rc;
    VCursor *curs;

    if ( self -> read_only )
    {
        self -> write_col_cache_valid = true;
        return 0;
    }

    rc = VTableCreateCursorWriteInt ( self, & curs, kcmInsert, false );
    if (  rc == 0 )
    {
	/* no need for schema-based triggers to fire **/
	VCursorSuspendTriggers ( curs );
        /* let this private VCursor-function list the columns */
        rc = VCursorListWritableColumns ( curs, & self -> write_col_cache );
        VCursorRelease ( curs );
        if ( rc == 0 )
            self -> write_col_cache_valid = true;
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableListWritableColumns ( VTable *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else
        {
            if ( ! self -> write_col_cache_valid )
                rc = list_writable_columns ( self );

            if ( self -> write_col_cache_valid )
                return make_column_namelist ( & self -> write_col_cache, names );
        }

        * names = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VTableListSeededWritableColumns ( VTable *self,
    KNamelist **names, const KNamelist *seed )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        * names = NULL;

        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else if ( seed == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
        else
        {
            BSTree cache;
            BSTreeInit ( & cache );

            rc = 0;

            if ( ! self -> read_only )
            {
                VCursor *curs;
                rc = VTableCreateCursorWriteInt ( self, & curs, kcmInsert, false );
                if (  rc == 0 )
                {
                    /* let this private VCursor-function list the columns */
                    rc = VCursorListSeededWritableColumns ( curs, & cache, seed );
                    VCursorRelease ( curs );
                }
            }

            if ( rc == 0 )
                rc = make_column_namelist ( & cache, names );

            BSTreeWhack ( & cache, VColumnRefWhack, NULL );
        }
    }

    return rc;
}


/* ListDatatypes
 *  returns list of typedecls for named column
 *
 *  "col" [ IN ] - column name
 *
 *  "dflt_idx" [ OUT, NULL OKAY ] - returns the zero-based index
 *  into "typedecls" of the default datatype for the named column
 *
 *  "typedecls" [ OUT ] - list of datatypes available for named column
 *
 *  availability: v2.1
 */
LIB_EXPORT rc_t CC VTableListWritableDatatypes ( VTable *self,
    const char *col, KNamelist **typedecls )
{
    rc_t rc;
    
    if ( typedecls == NULL )
        rc = RC ( rcVDB, rcTable, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcListing, rcSelf, rcNull );
        else if ( col == NULL )
            rc = RC ( rcVDB, rcSchema, rcListing, rcName, rcNull );
        else if ( col [ 0 ] == 0 )
            rc = RC ( rcVDB, rcSchema, rcListing, rcName, rcEmpty );
        else
        {
            if ( ! self -> write_col_cache_valid )
                rc = list_writable_columns ( self );

            if ( self -> write_col_cache_valid )
            {
                uint32_t dummy;
                return make_column_typelist ( & self -> write_col_cache, col, & dummy, typedecls );
            }
        }

        * typedecls = NULL;
    }

    return rc;
}


/* Reindex
 *  optimize column indices
 */
LIB_EXPORT rc_t CC VTableReindex ( VTable *self )
{
    if ( self == NULL )
        return RC ( rcVDB, rcTable, rcReindexing, rcSelf, rcNull );

    /* now allow the KTable to do its job */
    return KTableReindex ( self -> ktbl );
}


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
LIB_EXPORT rc_t CC VTableColumnCreateParams ( struct VTable *self,
    KCreateMode cmode, KChecksum checksum, size_t pgsize )
{
    if ( self == NULL )
        return RC ( rcVDB, rcTable, rcUpdating, rcSelf, rcNull );

    self -> cmode = cmode;
    self -> checksum = checksum;
    self -> pgsize = pgsize;

    return 0;
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VTableOpenManagerUpdate ( VTable *self, struct VDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VDBManagerAddRef ( self -> mgr );
            if ( rc == 0 )
            {
                * mgr = self -> mgr;
                return 0;
            }
        }

        * mgr = NULL;
    }

    return rc;
}


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VTableOpenParentUpdate ( VTable *self, struct VDatabase **db )
{
    rc_t rc;

    if ( db == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else if ( self -> db != NULL && self -> db -> read_only )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcDatabase, rcReadonly );
        else
        {
            rc = VDatabaseAddRef ( self -> db );
            if ( rc == 0 )
            {
                * db = self -> db;
                return 0;
            }
        }

        * db = NULL;
    }

    return rc;
}

/* OpenKTable
 *  returns a new reference to underlying KTable
 */
LIB_EXPORT rc_t CC VTableOpenKTableUpdate ( VTable *self, KTable **ktbl )
{
    rc_t rc;

    if ( ktbl == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            * ktbl = self -> ktbl;
            return KTableAddRef ( * ktbl );
        }

        * ktbl = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VTableVDropColumn(VTable *self, const char fmt[], va_list args)
{
    return KTableVDropColumn(self->ktbl, fmt, args);
}

LIB_EXPORT rc_t CC VTableDropColumn(VTable *self, const char fmt[], ...)
{
    va_list va;
    rc_t rc;
    
    va_start(va, fmt);
    rc = VTableVDropColumn(self, fmt, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC VTableRenameColumn ( struct VTable *self, bool force,
    const char *from, const char *to )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcTable, rcAccessing, rcSelf, rcNull );
    else
    {
        rc = KTableRenameColumn ( self->ktbl, force, from, to );
        if ( GetRCState(rc) == rcNotFound )
           rc = KMDataNodeRenameChild( self->col_node, from, to );
    }

    return rc;
}

