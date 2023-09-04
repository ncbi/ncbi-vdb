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

#define KONST const
#include <kdb/extern.h>
#include "rdatabase.h"
#include "kdb-priv.h"
#include "table-priv.h"
#include "rdbmgr.h"
#undef KONST

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h>
#include <klib/namelist.h>
#include <klib/printf.h>
#include <klib/rc.h>

#include <os-native.h>
#include <sysalloc.h>

#include <vfs/path.h>
#include <vfs/manager.h>
#include <vfs/manager-priv.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KDatabase
 *  connection to a database within file system, read side
 */

static rc_t CC KRDatabaseWhack ( KDatabase *self );
static bool CC KRDatabaseLocked ( const KDatabase *self );
static bool CC KRDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args );
static bool CC KRDatabaseIsAlias ( const KDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KRDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args );
static rc_t CC KRDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr );
static rc_t CC KRDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par );
static rc_t CC KRDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir );
static rc_t CC KRDatabaseVOpenDBRead ( const KDatabase *self, const KDatabase **dbp, const char *name, va_list args );
static rc_t CC KRDatabaseVOpenTableRead ( const KDatabase *self, const KTable **tblp, const char *name, va_list args );

static KDatabase_vt KRDatabase_vt =
{
    KRDatabaseWhack,
    KDatabaseBaseAddRef,
    KDatabaseBaseRelease,
    KRDatabaseLocked,
    KRDatabaseVExists,
    KRDatabaseIsAlias,
    KRDatabaseVWritable,
    KRDatabaseOpenManagerRead,
    KRDatabaseOpenParentRead,
    KRDatabaseOpenDirectoryRead,
    KRDatabaseVOpenDBRead,
    KRDatabaseVOpenTableRead
};


/* GetPath
 *  return the absolute path to DB
 */
LIB_EXPORT rc_t CC KDatabaseGetPath ( KDatabase const *self,
    const char **path )
{
    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );

    *path = self->path;

    return 0;
}

/* Whack
 */
static
rc_t CC
KRDatabaseWhack ( KDatabase *self )
{
    rc_t rc = 0;

    /* release parent */
    if ( self -> parent != NULL )
    {
        rc = KDatabaseSever ( self -> parent );
        if ( rc == 0 )
            self -> parent = NULL;
    }

    /* remove from mgr */
    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );

    /* complete */
    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        return KDatabaseBaseWhack( self );
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KDatabase", "whack", "kdb" );

    return rc;
}

/* Make
 *  make an initialized structure
 */
rc_t KDatabaseMake ( KDatabase **dbp, const KDirectory *dir, const char *path )
{
    KDatabase *db;

    assert ( dbp != NULL );
    assert ( path != NULL );

    db = malloc ( sizeof * db + strlen ( path ) );
    if ( db == NULL )
    {
        * dbp = NULL;
        return RC ( rcDB, rcDatabase, rcConstructing, rcMemory, rcExhausted );
    }

    memset ( db, 0, sizeof * db );
    db -> dad . vt = & KRDatabase_vt;
    KRefcountInit ( & db -> dad . refcount, 1, "KDatabase", "make", path );

    db -> mgr = NULL;
    db -> parent = NULL;
    db -> dir = dir;

    /* for open mode we don't care about creation mode or checksum, setting defaults */
    db -> cmode = kcmOpen;
    db -> checksum = kcsNone;

    strcpy ( db -> path, path );

    /* YES,
     DBG_VFS should be used here to be printed along with other VFS messages */
    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_SERVICE),
        ("KDatabaseMake: Making KDatabase '%s'\n", path));

    * dbp = db;
    return 0;
}

rc_t KDatabaseMakeVPath ( KDatabase **dbp, const KDirectory *dir, const VPath* path )
{
    const String* dbpathStr;
    rc_t rc = VPathMakeString ( path, &dbpathStr );    /* NUL-terminated */
    if ( rc == 0 )
    {
        rc = KDatabaseMake ( dbp, dir, dbpathStr->addr );
        StringWhack(dbpathStr);
    }
    return rc;
}

static rc_t CC KRDatabaseVOpenDBRead ( const KDatabase *self, const KDatabase **dbp, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( dbp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * dbp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "db", 2, name, args );
    if ( rc == 0 )
    {
        rc = KDBManagerVOpenDBReadInt_noargs ( self -> mgr, dbp,
                                        self -> dir, false, path );
        if ( rc == 0 )
        {
            KDatabase *db = ( KDatabase* ) * dbp;
            db -> parent = KDatabaseAttach ( self );
        }
    }

    return rc;
}


/* Locked
 *  returns non-zero if locked
 */
static
bool CC
KRDatabaseLocked ( const KDatabase *self )
{
    rc_t rc;

    if ( self == NULL )
        return false;

    rc = KDBWritable ( self -> dir, "." );
    return GetRCState ( rc ) == rcLocked;
}

/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
static bool CC KRDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    if ( self != NULL && name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        uint32_t len;
        const char *ns;
        char path [ 256 ];

        switch ( type )
        {
        case kptDatabase:
            ns = "db";
            len = 2;
            break;

        case kptTable:
            ns = "tbl";
            len = 3;
            break;

        case kptIndex:
            ns = "idx";
            len = 3;
            break;

        default:
            return false;
        }

        rc = KDBVMakeSubPath ( self -> dir, path, sizeof path, ns, len, name, args );
        if ( rc == 0 )
        {
            switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
            {
            case kptFile:
            case kptDir:
            case kptFile | kptAlias:
            case kptDir | kptAlias:
                return true;
            }
        }
    }
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
static bool CC KRDatabaseIsAlias ( const KDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    if ( self != NULL && name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        uint32_t len;
        const char *ns;
        char path [ 256 ];

        switch ( type )
        {
        case kptDatabase:
            ns = "db";
            len = 2;
            break;

        case kptTable:
            ns = "tbl";
            len = 3;
            break;

        case kptIndex:
            ns = "idx";
            len = 3;
            break;

        default:
            return false;
        }

        rc = KDBMakeSubPath ( self -> dir, path, sizeof path, ns, len, name );
        if ( rc == 0 )
        {
            switch ( KDirectoryPathType ( self -> dir, "%s", path ) )
            {
            case kptFile | kptAlias:
            case kptDir | kptAlias:
                if ( resolved != NULL && rsize != 0 )
                {
                    const KDirectory *sub;
                    rc = KDirectoryOpenDirRead ( self -> dir, & sub, false, "%s", ns );
                    * resolved = 0;
                    if ( rc != 0 )
                        PLOGERR ( klogWarn, ( klogWarn, rc, "failed to open subdirectory '$(sub)'", "sub=%s", ns ) );
                    else
                    {
                        rc = KDirectoryResolveAlias ( sub, false, resolved, rsize, "%s", name );
                        KDirectoryRelease ( sub );
                        if ( rc != 0 )
                            PLOGERR ( klogWarn, ( klogWarn, rc, "failed to resolve alias '$(alias)'", "alias=%s", name ) );
                    }
                }
                return true;
            }
        }
    }

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
static
rc_t CC
KRDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return -1;
}

/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static
rc_t CC
KRDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KDBManagerAddRef ( self -> mgr );
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
static
rc_t CC
KRDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par )
{
    rc_t rc;

    if ( par == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KDatabaseAddRef ( self -> parent );
            if ( rc == 0 )
            {
                * par = self -> parent;
                return 0;
            }
        }

        * par = NULL;
    }

    return rc;
}


/* GetDirectory
 *  access the directory in use
 */
static
rc_t CC
KRDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            * dir = self -> dir;
            return KDirectoryAddRef ( * dir );
        }

        * dir = NULL;
    }

    return rc;
}


/* ModDate
 *  get modification date
 */
LIB_EXPORT rc_t CC KDatabaseModDate ( const KDatabase *self, KTime_t *mtime )
{
    rc_t rc;

    if ( mtime == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else
        {
            /* HACK ALERT - there needs to be a proper way to record modification times */
            const KDirectory *dir = self -> dir;

            /* this only tells the last time the table was locked,
               which may be close to the last time it was modified */
            rc = KDirectoryDate ( dir, mtime, "lock" );
            if ( rc == 0 )
                return 0;

            /* get directory timestamp */
            rc = KDirectoryDate ( dir, mtime, "." );
            if ( rc == 0 )
                return 0;
        }

        * mtime = 0;
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KNameList
 */

/* List
 *  create database listings
 */
struct FilterData
{
    const KDBManager * mgr;
    int type;
};

static
bool CC KDatabaseListFilter ( const KDirectory *dir, const char *name, void *data_ )
{
    struct FilterData * data = data_;
    return ( KDBOpenPathTypeRead ( data->mgr, dir, name,
        NULL, data->type, NULL, false, NULL ) == 0 );
}

LIB_EXPORT rc_t CC KDatabaseListDB ( const KDatabase *self, KNamelist **names )
{
    if ( self != NULL )
    {
        struct FilterData data;
        data.mgr = self->mgr;
        data.type = kptDatabase;

        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, &data, "db" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcDatabase, rcListing, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC KDatabaseListTbl ( struct KDatabase const *self, KNamelist **names )
{
    if ( self != NULL )
    {
        struct FilterData data;
        data.mgr = self->mgr;
        data.type = kptTable;

        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, &data, "tbl" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcDatabase, rcListing, rcSelf, rcNull );
}

LIB_EXPORT rc_t CC KDatabaseListIdx ( struct KDatabase const *self, KNamelist **names )
{
    if ( self != NULL )
    {
        struct FilterData data;
        data.mgr = self->mgr;
        data.type = kptIndex;

        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, &data, "idx" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcDatabase, rcListing, rcSelf, rcNull );
}

static
rc_t CC
KRDatabaseVOpenTableRead ( const KDatabase *self,
    const KTable **tblp, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( tblp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * tblp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( name == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "tbl", 3, name, args );
    if ( rc == 0 )
    {
        rc = KDBManagerVOpenTableReadInt_noargs ( self -> mgr, tblp,
                                self -> dir, false, path, false, NULL );
        if ( rc == 0 )
        {
            KTable *tbl = ( KTable* ) * tblp;
            tbl -> db = KDatabaseAttach ( self );
        }
    }

    return rc;
}

