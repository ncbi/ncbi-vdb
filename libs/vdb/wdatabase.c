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

#include "database-priv.h"
#include "dbmgr-priv.h"
#include "schema-priv.h"
#include "schema-parse.h"
#include "linker-priv.h"

#include <kdb/kdb-priv.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/meta.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VDatabase
 *  opaque connection to a database within file system
 */


/* StoreSchema
 */
rc_t VDatabaseStoreSchema ( VDatabase *self )
{
    /* open schema node */
    KMDataNode *node;
    rc_t rc = KMetadataOpenNodeUpdate ( self -> meta, & node, "schema" );
    if ( rc == 0 )
    {
        size_t num_writ;
        char expr [ 256 ];
        rc = VSchemaToText ( self -> schema, expr, sizeof expr - 1, & num_writ,
            "%N%V", self -> sdb -> name, self -> sdb -> version );
        if ( rc != 0 )
            LOGERR ( klogInt, rc, "failed to determine database schema" );
        else
        {
            expr [ num_writ ] = 0;
            rc = KMDataNodeWriteAttr ( node, "name", expr );
            if ( rc != 0 )
                PLOGERR (klogInt, ( klogInt, rc, "failed to write database type '$(expr)'", "expr=%s", expr ));
            else
            {
                /* truncate existing schema */
                rc = KMDataNodeWrite ( node, "", 0 );
                if ( rc == 0 )
                {
                    rc = VSchemaDump ( self -> schema, sdmCompact, expr,
                        ( rc_t ( CC * ) ( void*, const void*, size_t ) ) KMDataNodeAppend, node );
                }
                if ( rc != 0 )
                    PLOGERR (klogInt, ( klogInt, rc, "failed to write database schema '$(expr)'", "expr=%s", expr ));
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
rc_t VDatabaseOpenUpdate ( VDatabase *self, const char *decl )
{
    /* open metadata */
    rc_t rc = KDatabaseOpenMetadataUpdate ( self -> kdb, & self -> meta );
    if ( rc == 0 )
    {
        /* fetch stored schema */
        rc = VDatabaseLoadSchema ( self );
        if ( rc == 0 )
        {
            /* fetch requested schema */
            const SDatabase *sdb = self -> sdb;
            if ( decl != NULL && decl [ 0 ] != 0 )
            {
                uint32_t type;
                const SNameOverload *name;
                sdb = ( self -> dad != NULL ) ?
                    SDatabaseFind ( self -> dad -> sdb, self -> schema,
                        & name, & type, decl, "VDatabaseOpenUpdate" ):
                    VSchemaFind ( self -> schema,
                        & name, & type, decl, "VDatabaseOpenUpdate", true );
                if ( sdb != NULL && type != eDatabase )
                {
                    PLOGMSG ( klogWarn, ( klogWarn, "expression '$(expr)' is not a database",
                               "expr=%s", decl ));
                    sdb = NULL;
                }
            }

            /* error if the two definitions differ */
            if ( sdb != NULL && self -> sdb != NULL && sdb != self -> sdb )
                rc = RC ( rcVDB, rcDatabase, rcOpening, rcSchema, rcIncorrect );
            else if ( sdb == NULL && self -> sdb == NULL )
                rc = RC ( rcVDB, rcDatabase, rcOpening, rcSchema, rcNotFound );
            else
            {
                if ( sdb != NULL )
                    self -> sdb = sdb;

                /* write schema to metadata */
                rc = VDatabaseStoreSchema ( self );
                if ( rc == 0 )
                    return 0;
            }
        }
    }

    DBGMSG(DBG_VDB, DBG_FLAG(DBG_VDB_VDB), ("VDatabaseOpenUpdate = %d\n", rc));

    return rc;
}


/* CreateDB
 * VCreateDB
 *  create a new or open an existing database
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "schema" [ IN ] - schema object containg database
 *  declaration to be used in creating db [ needed by manager ].
 *
 *  "decl" [ IN ] - type and optionally version of db schema
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
LIB_EXPORT rc_t CC VDBManagerVCreateDB ( VDBManager *self, VDatabase **dbp,
    const VSchema *schema, const char *decl,
    KCreateMode cmode, const char *path, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcMgr, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcCreating, rcSelf, rcNull );
        else if ( schema == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSchema, rcNull );
        else if ( decl == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcNull );
        else if ( decl [ 0 ] == 0 )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcEmpty );
        else
        {
            rc = VDatabaseMake ( dbp, self, NULL, schema );
            if ( rc == 0 )
            {
                VDatabase *db = * dbp;

                rc = KDBManagerVCreateDB ( self -> kmgr, & db -> kdb, cmode, path, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenUpdate ( db, decl );
                    if ( rc == 0 )
                        return 0;

                    rc = ResetRCContext ( rc, rcVDB, rcMgr, rcCreating );
                }

                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDBManagerCreateDB ( VDBManager *self, VDatabase **db,
    const VSchema *schema, const char *decl,
    KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVCreateDB ( self, db, schema, decl, cmode, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVCreateDB ( VDatabase *self, VDatabase **dbp,
    const char *decl, KCreateMode cmode, const char *name, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcCreating, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcCreating, rcSelf, rcNull );
        else if ( decl == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcNull );
        else if ( decl [ 0 ] == 0 )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcName, rcEmpty );
        else if ( self -> read_only )
            rc = RC ( rcVDB, rcDatabase, rcCreating, rcDatabase, rcReadonly );
        else
        {
            rc = VDatabaseMake ( dbp, NULL, self, self -> schema );
            if ( rc == 0 )
            {
                VDatabase *db = * dbp;

                rc = KDatabaseVCreateDB ( self -> kdb, & db -> kdb, cmode, name, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenUpdate ( db, decl );
                    if ( rc == 0 )
                        return 0;

                    rc = ResetRCContext ( rc, rcVDB, rcDatabase, rcCreating );
                }

                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseCreateDB ( VDatabase *self, VDatabase **db,
    const char *decl, KCreateMode cmode, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VDatabaseVCreateDB ( self, db, decl, cmode, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVDropDB ( VDatabase *self,
                                     const char *name, va_list args)
{
    return KDatabaseVDropDB(self->kdb, name, args);
}

LIB_EXPORT rc_t CC VDatabaseDropDB ( VDatabase *self,
                                    const char *name, ... )
{
    rc_t rc;
    va_list args;
    
    va_start ( args, name );
    rc = VDatabaseVDropDB(self, name, args);
    va_end ( args );
    
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVDropTable ( VDatabase *self,
                                    const char *name, va_list args)
{
    return KDatabaseVDropTable(self->kdb, name, args);
}

LIB_EXPORT rc_t CC VDatabaseDropTable ( VDatabase *self,
                                      const char *name, ... )
{
    rc_t rc;
    va_list args;
    
    va_start ( args, name );
    rc = VDatabaseVDropTable(self, name, args);
    va_end ( args );
    
    return rc;
}


/* OpenDBUpdate
 * VOpenDBUpdate
 *  open a database for read/write
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "schema" [ IN, NULL OKAY ] - schema object containg database
 *  declaration to be used in creating db [ needed by manager ].
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
LIB_EXPORT rc_t CC VDBManagerVOpenDBUpdate ( VDBManager *self, VDatabase **dbp,
    const VSchema *schema, const char *path, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcMgr, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcMgr, rcOpening, rcSelf, rcNull );
        else
        {
            if ( schema == NULL )
                schema = self -> schema;

            rc = VDatabaseMake ( dbp, self, NULL, schema );
            if ( rc == 0 )
            {
                VDatabase *db = * dbp;

                rc = KDBManagerVOpenDBUpdate ( self -> kmgr, & db -> kdb, path, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenUpdate ( db, NULL );
                    if ( rc == 0 )
                        return 0;
                }
                        
                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDBManagerOpenDBUpdate ( VDBManager *self, VDatabase **db,
    const VSchema *schema, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVOpenDBUpdate ( self, db, schema, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVOpenDBUpdate ( VDatabase *self, VDatabase **dbp,
    const char *name, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );
        else
        {
            rc = VDatabaseMake ( dbp, NULL, self, self -> schema );
            if ( rc == 0 )
            {
                VDatabase *db = * dbp;

                rc = KDatabaseVOpenDBUpdate ( self -> kdb, & db -> kdb, name, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenUpdate ( db, NULL );
                    if ( rc == 0 )
                        return 0;
                }

                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseOpenDBUpdate ( VDatabase *self, VDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VDatabaseVOpenDBUpdate ( self, db, name, args );
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
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VDatabaseVLock ( VDatabase *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcDatabase, rcLocking, rcSelf, rcNull );
    else
        rc = KDatabaseVLock ( self -> kdb, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseLock ( VDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VDatabaseVLock ( self, type, name, args );

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
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VDatabaseVUnlock ( VDatabase *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcDatabase, rcUnlocking, rcSelf, rcNull );
    else
        rc = KDatabaseVUnlock ( self -> kdb, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseUnlock ( VDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VDatabaseVUnlock ( self, type, name, args );

    va_end ( args );

    return rc;
}


/* OpenMetadataUpdate
 *  opens metadata for update
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
LIB_EXPORT rc_t CC VDatabaseOpenMetadataUpdate ( VDatabase *self, KMetadata **meta )
{
    rc_t rc;
    if ( meta == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        * meta = NULL;

        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            /* we operate under the notion of
               single-threaded operation, so hand out
               read or update capable object */
            rc = KMetadataAddRef ( self -> meta );
            if ( rc == 0 )
                * meta = self -> meta;
        }
    }

    return rc;
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
LIB_EXPORT rc_t CC VDatabaseColumnCreateParams ( VDatabase *self,
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
LIB_EXPORT rc_t CC VDatabaseOpenManagerUpdate ( VDatabase *self, VDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
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
LIB_EXPORT rc_t CC VDatabaseOpenParentUpdate ( VDatabase *self, VDatabase **par )
{
    rc_t rc;

    if ( par == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else if ( self -> dad != NULL && self -> dad -> read_only )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcDatabase, rcReadonly );
        else
        {
            rc = VDatabaseAddRef ( self -> dad );
            if ( rc == 0 )
            {
                * par = self -> dad;
                return 0;
            }
        }

        * par = NULL;
    }

    return rc;
}


/* OpenKDatabase
 *  returns a new reference to underlying KDatabase
 */
LIB_EXPORT rc_t CC VDatabaseOpenKDatabaseUpdate ( VDatabase *self, KDatabase **kdb )
{
    rc_t rc;

    if ( kdb == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcDatabase, rcReadonly );
        else
        {
            rc = KDatabaseAddRef ( self -> kdb );
            if ( rc == 0 )
            {
                * kdb = self -> kdb;
                return 0;
            }
        }

        * kdb = NULL;
    }

    return rc;
}
