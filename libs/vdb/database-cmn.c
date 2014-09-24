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

#define KONST const
#include "database-priv.h"
#include "dbmgr-priv.h"
#undef KONST

#include "schema-priv.h"
#include "linker-priv.h"

#include <vdb/manager.h>
#include <vdb/database.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <klib/debug.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/namelist.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VDatabase
 *  opaque connection to a database within file system
 */


/* Whack
 */
rc_t CC VDatabaseWhack ( VDatabase *self )
{
    rc_t rc = 0;

    KRefcountWhack ( & self -> refcount, "VDatabase" );

    /* release user data */
    if ( self -> user != NULL && self -> user_whack != NULL )
    {
        ( * self -> user_whack ) ( self -> user );
        self -> user = NULL;
        self -> user_whack = NULL;
    }

    /* release dad */
    if ( self -> dad != NULL )
    {
        rc = VDatabaseSever ( self -> dad );
        if ( rc == 0 )
            self -> dad = NULL;
    }

    /* remove from mgr */
    if ( rc == 0 )
        rc = VDBManagerSever ( self -> mgr );

    if ( rc == 0 )
    {
        /* complete */
        KMetadataRelease ( self -> meta );
        KDatabaseRelease ( self -> kdb );
        VSchemaRelease ( self -> schema );

        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "VDatabase", "whack", "vdb" );
    return rc;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC VDatabaseAddRef ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "VDatabase" ) )
        {
        case krefLimit:
            return RC ( rcVDB, rcDatabase, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC VDatabaseRelease ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "VDatabase" ) )
        {
        case krefWhack:
            return VDatabaseWhack ( ( VDatabase* ) self );
        case krefNegative:
            return RC ( rcVDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 *  internal reference management
 */
VDatabase *VDatabaseAttach ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "VDatabase" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( VDatabase* ) self;
}

rc_t VDatabaseSever ( const VDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "VDatabase" ) )
        {
        case krefWhack:
            return VDatabaseWhack ( ( VDatabase* ) self );
        case krefNegative:
            return RC ( rcVDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make - PRIVATE
 *  creates an initialized database object
 *  expects a mgr and optionally a parent db,
 *  enforces schema validity according to some rules,
 */
rc_t VDatabaseMake ( VDatabase **dbp,
    const VDBManager *mgr, const VDatabase *dad, const VSchema *schema )
{
    rc_t rc;

    /* create a structure */
    VDatabase *db = calloc ( 1, sizeof * db );
    if ( db == NULL )
        rc = RC ( rcVDB, rcDatabase, rcConstructing, rcMemory, rcExhausted );
    else
    {
        /* create a modifiable schema */
        rc = VSchemaMake ( ( VSchema** ) & db -> schema, schema );
        if ( rc == 0 )
        {
            db -> mgr = VDBManagerAttach ( mgr );
            db -> dad = VDatabaseAttach ( dad );
            KRefcountInit ( & db -> refcount, 1, "VDatabase", "make", "vdb" );

            db -> cmode = ( uint8_t ) kcmOpen;
            db -> checksum = ( uint8_t ) kcsNone;

            * dbp = db;
            return 0;
        }

        VDatabaseWhack ( db );
    }

    * dbp = NULL;

    return rc;
}


/* OpenRead
 *  finish opening a db for read
 */
static
rc_t CC VDatabaseOpenRead ( VDatabase *self )
{
    /* open metadata */
    rc_t rc = KDatabaseOpenMetadataRead ( self -> kdb, & self -> meta );
    if ( rc == 0 )
    {
        /* fetch stored schema */
        rc = VDatabaseLoadSchema ( self );
        if ( rc == 0 && self -> sdb == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSchema, rcNotFound );
    }

    DBGMSG(DBG_VDB, DBG_FLAG(DBG_VDB_VDB), ("VDatabaseOpenRead = %d\n", rc));

    return rc;
}


/* OpenDBRead
 * VOpenDBRead
 *  open a database for read
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "schema" [ IN, NULL OKAY ] - schema object containg database
 *  declaration to be used in creating db.
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
LIB_EXPORT rc_t CC VDBManagerVOpenDBRead ( const VDBManager *self,
    const VDatabase **dbp, const VSchema *schema,
    const char *path, va_list args )
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
            VDatabase *db;

            if ( schema == NULL )
                schema = self -> schema;

            rc = VDatabaseMake ( & db, self, NULL, schema );
            if ( rc == 0 )
            {
                db -> read_only = true;

                rc = KDBManagerVOpenDBRead ( self -> kmgr, & db -> kdb, path, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenRead ( db );
                    if ( rc == 0 )
                    {
                        * dbp = db;
                        return 0;
                    }
                }

                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDBManagerOpenDBRead ( const VDBManager *self,
    const VDatabase **db, const VSchema *schema,
    const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = VDBManagerVOpenDBRead ( self, db, schema, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseVOpenDBRead ( const VDatabase *self,
    const VDatabase **dbp, const char *name, va_list args )
{
    rc_t rc;

    if ( dbp == NULL )
        rc = RC ( rcVDB, rcDatabase, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcOpening, rcSelf, rcNull );
        else
        {
            VDatabase *db;
            rc = VDatabaseMake ( & db, self -> mgr, self, self -> schema );
            if ( rc == 0 )
            {
                db -> read_only = true;

                rc = KDatabaseVOpenDBRead ( self -> kdb, & db -> kdb, name, args );
                if ( rc == 0 )
                {
                    rc = VDatabaseOpenRead ( db );
                    if ( rc == 0 )
                    {
                        * dbp = db;
                        return 0;
                    }
                }

                VDatabaseWhack ( db );
            }
        }

        * dbp = NULL;
    }
    return rc;
}

LIB_EXPORT rc_t CC VDatabaseOpenDBRead ( const VDatabase *self,
    const VDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = VDatabaseVOpenDBRead ( self, db, name, args );
    va_end ( args );

    return rc;
}


/* Locked
 *  returns true if locked
 */
LIB_EXPORT bool CC VDatabaseLocked ( const VDatabase *self )
{
    /* TBD - no concept of virtual database locking
       other than physical database locking */
    if ( self != NULL )
        return KDatabaseLocked ( self -> kdb );
    return false;
}


/* IsAlias
 *  returns true if object name is an alias
 *  returns path to fundamental name if it was aliased
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - optional output buffer
 *  for fundamenta object name if "alias" is not a fundamental name, 
 *
 *  "name" [ IN ] - NUL terminated object name
 */
LIB_EXPORT bool CC VDatabaseIsAlias ( const VDatabase *self, uint32_t type,
    char *resolved, size_t rsize, const char *name )
{
    if ( self != NULL )
        return KDatabaseIsAlias ( self -> kdb, type, resolved, rsize, name );

    if ( resolved != NULL && rsize != 0 )
        * resolved = 0;

    return false;
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC VDatabaseVWritable ( const VDatabase *self, uint32_t type, 
        const char *name, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
    else
        rc = KDatabaseVWritable ( self -> kdb, type, name, args );

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseWritable ( const VDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = VDatabaseVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}

/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
LIB_EXPORT rc_t CC VDatabaseOpenMetadataRead ( const VDatabase *self, const KMetadata **meta )
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


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VDatabaseOpenManagerRead ( const VDatabase *self, const VDBManager **mgr )
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
LIB_EXPORT rc_t CC VDatabaseOpenParentRead ( const VDatabase *self, const VDatabase **par )
{
    rc_t rc;

    if ( par == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
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


/* OpenSchema
 *  duplicate reference to table schema
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC VDatabaseOpenSchema ( const VDatabase *self, const VSchema **schema )
{
    rc_t rc;

    if ( schema == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VSchemaAddRef ( self -> schema );
            if ( rc == 0 )
            {
                * schema = self -> schema;
                return 0;
            }
        }

        * schema = NULL;
    }

    return rc;
}


/* lists the tables of the database
 * 
 * "names" [ OUT ] - return parameter for tables 
 */
LIB_EXPORT rc_t CC VDatabaseListTbl ( const VDatabase *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcListing, rcSelf, rcNull );
        else
        {
            if ( self->kdb == NULL )
                rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
            else
            {
                return KDatabaseListTbl ( self->kdb, names );
            }
        }
        *names = NULL;
    }
    return rc;
}

/* lists the sub-databases of the database
 * 
 * "names" [ OUT ] - return parameter for databases 
 */
LIB_EXPORT rc_t CC VDatabaseListDB ( const VDatabase *self, KNamelist **names )
{
    rc_t rc;

    if ( names == NULL )
        rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcListing, rcSelf, rcNull );
        else
        {
            if ( self->kdb == NULL )
                rc = RC ( rcVDB, rcDatabase, rcListing, rcParam, rcNull );
            else
            {
                return KDatabaseListDB ( self->kdb, names );
            }
        }
        *names = NULL;
    }
    return rc;
}

/* GetUserData
 * SetUserData
 *  store/retrieve an opaque pointer to user data
 */
LIB_EXPORT rc_t CC VDatabaseGetUserData ( const VDatabase *self, void **data )
{
    rc_t rc;

    if ( data == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            * data = self -> user;
            return 0;
        }

        * data = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC VDatabaseSetUserData ( const VDatabase *cself,
    void *data, void ( CC * destroy ) ( void *data ) )
{
    VDatabase *self = ( VDatabase* ) cself;
    if ( cself == NULL )
        return RC ( rcVDB, rcDatabase, rcUpdating, rcSelf, rcNull );

    self -> user = data;
    self -> user_whack = destroy;

    return 0;
}


/* OpenKDatabase
 *  returns a new reference to underlying KDatabase
 */
LIB_EXPORT rc_t CC VDatabaseOpenKDatabaseRead ( const VDatabase *self, const KDatabase **kdb )
{
    rc_t rc;

    if ( kdb == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
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



/* Typespec
 *  retrieve db typespec
 *
 *  AVAILABILITY: version 2.2
 *
 *  "ts_buff" [ IN ] and "ts_size" [ IN ] - buffer for return of NUL-terminated
 *  db typespec, e.g. 'NCBI:db:abc#1.1'
 */
LIB_EXPORT rc_t CC VDatabaseTypespec ( const VDatabase *self, char *ts_buff, size_t ts_size )
{
    rc_t rc;

    if ( ts_size == 0 )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcBuffer, rcInsufficient );
    else if ( ts_buff == NULL )
        rc = RC ( rcVDB, rcDatabase, rcAccessing, rcBuffer, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc_t rc2;
            rc2 = string_printf ( ts_buff, ts_size, NULL, "%N#%V", self -> sdb -> name, self -> sdb -> version );
            if ( rc2 == 0 )
                return 0;
            rc = rc2;
        }

        ts_buff [ 0 ] = 0;
    }

    return rc;
}


/* IsCSRA
 *  ask an open database if it conforms to cSRA schema
 */
LIB_EXPORT bool CC VDatabaseIsCSRA ( const VDatabase *self )
{
    if ( self != NULL )
        return KDatabaseExists ( self -> kdb, kptTable, "PRIMARY_ALIGNMENT" );

    return false;
}
