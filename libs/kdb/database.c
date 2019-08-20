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
#include "database-priv.h"
#include "dbmgr-priv.h"
#include "kdb-priv.h"
#undef KONST

#include <klib/namelist.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <klib/printf.h>
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
 *  connection to a database within file system
 */

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
rc_t KDatabaseWhack ( KDatabase *self )
{
    rc_t rc = 0;

    KRefcountWhack ( & self -> refcount, "KDatabase" );

    /* release dad */
    if ( self -> dad != NULL )
    {
        rc = KDatabaseSever ( self -> dad );
        if ( rc == 0 )
            self -> dad = NULL;
    }

    /* remove from mgr */
    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );

    /* complete */
    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "KDatabase", "whack", "kdb" );

    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KDatabaseAddRef ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcDatabase, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KDatabaseRelease ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KDatabase" ) )
        {
        case krefWhack:
            return KDatabaseWhack ( ( KDatabase* ) self );
        case krefNegative:
            return RC ( rcDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
KDatabase *KDatabaseAttach ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KDatabase* ) self;
}

rc_t KDatabaseSever ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KDatabase" ) )
        {
        case krefWhack:
            return KDatabaseWhack ( ( KDatabase* ) self );
        case krefNegative:
            return RC ( rcDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make
 *  make an initialized structure
 */
static
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

    db -> mgr = NULL;
    db -> dad = NULL;
    db -> dir = dir;

    /* for open mode we don't care about creation mode or checksum, setting defaults */
    db -> cmode = kcmOpen;
    db -> checksum = kcsNone;

    KRefcountInit ( & db -> refcount, 1, "KDatabase", "make", path );
    strcpy ( db -> path, path );

    * dbp = db;
    return 0;
}

static
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


/* OpenDBRead
 * VOpenDBRead
 *  open a database for read
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KDBManagerVOpenDBReadInt ( const KDBManager *self, const KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, va_list args )
{
    rc_t rc;

    /* MUST use vsnprintf because the documented behavior of "path"
       is that of stdc library's printf, not vdb printf */
    char dbpath [ 4096 ];
    int z = ( args == NULL ) ?
        snprintf ( dbpath, sizeof dbpath, "%s", path ):
        vsnprintf ( dbpath, sizeof dbpath, path, args );
    if ( z < 0 || ( size_t ) z >= sizeof dbpath )
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    else
    {
        const KDirectory *dir;

        /* open the directory if its a database */
        rc = KDBOpenPathTypeRead ( self, wd, dbpath, &dir, kptDatabase, NULL,
            try_srapath, NULL );
        if ( rc == 0 )
        {
            KDatabase *db;

            /* allocate a new guy */
            rc = KDatabaseMake ( & db, dir, dbpath );
            if ( rc == 0 )
            {
                db -> mgr = KDBManagerAttach ( self );
                * dbp = db;
                return 0;
            }

            KDirectoryRelease ( dir );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBRead ( self, db, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenDBRead ( const KDatabase *self,
    const KDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenDBRead ( self, db, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenDBReadInt ( self, db, self -> wd, true, path, args );
}

LIB_EXPORT rc_t CC KDatabaseVOpenDBRead ( const KDatabase *self,
    const KDatabase **dbp, const char *name, va_list args )
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
        rc = KDBManagerVOpenDBReadInt ( self -> mgr, dbp,
                                        self -> dir, false, path, NULL );
        if ( rc == 0 )
        {
            KDatabase *db = ( KDatabase* ) * dbp;
            db -> dad = KDatabaseAttach ( self );
        }
    }

    return rc;
}


/* Locked
 *  returns non-zero if locked
 */
LIB_EXPORT bool CC KDatabaseLocked ( const KDatabase *self )
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
LIB_EXPORT bool CC KDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args )
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

LIB_EXPORT bool CC KDatabaseExists ( const KDatabase *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KDatabaseVExists ( self, type, name, args );

    va_end ( args );

    return exists;
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
LIB_EXPORT bool CC KDatabaseIsAlias ( const KDatabase *self, uint32_t type,
    char *resolved, size_t rsize, const char *name )
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
LIB_EXPORT rc_t CC KDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return -1;
}

LIB_EXPORT rc_t CC KDatabaseWritable ( const KDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = KDatabaseVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC KDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr )
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
LIB_EXPORT rc_t CC KDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par )
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
            rc = KDatabaseAddRef ( self -> dad );
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


/* GetDirectory
 *  access the directory in use
 */
LIB_EXPORT rc_t CC KDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir )
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

LIB_EXPORT rc_t CC KDBManagerVPathOpenLocalDBRead ( struct KDBManager const * self,
    struct KDatabase const ** p_db, struct VPath const * vpath )
{
    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
    if ( p_db == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    if ( vpath == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
        
    {   
        /* vpath has already been resolved and is known to be a local path. 
           open it if it is a database; avoid an additional round of resolution */
        const KDirectory *dir;
        rc_t rc = VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt ( self -> vfsmgr, self -> wd, &dir, vpath );
        if ( rc == 0 )
        {
            if ( ( (~kptAlias) & KDBPathType ( dir, NULL, "." ) ) != kptDatabase )
            {
                rc = RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcIncorrect );
            }
            else
            {   /* allocate a new guy */
                KDatabase *db;
                rc = KDatabaseMakeVPath ( & db, dir, vpath );
                if ( rc == 0 )
                {
                    db -> mgr = KDBManagerAttach ( self ); 
                    * p_db = db;
                    return 0;
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
} 

LIB_EXPORT rc_t CC KDBManagerVPathOpenRemoteDBRead ( struct KDBManager const * self,
    struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )
{
    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
    if ( p_db == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    if ( remote == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    /* cache == NULL is OK */    
    
    {   
        /*  vpath has already been resolved and is known to be a remote URL. 
            Open it if it is a database; use the provided cache; avoid an additional round of resolution */
        const KDirectory *dir;
        rc_t rc = VFSManagerOpenDirectoryReadDecryptRemote( self -> vfsmgr, &dir, remote, cache );
        if ( rc == 0 )
        {
            if ( ( (~kptAlias) & KDBPathType ( dir, NULL, "." ) ) != kptDatabase )
            {
                rc = RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcIncorrect );
            }
            else
            {   /* allocate a new guy */
                KDatabase *db;
                rc = KDatabaseMakeVPath ( & db, dir, remote );
                if ( rc == 0 )
                {
                    db -> mgr = KDBManagerAttach ( self ); 
                    * p_db = db;
                    return 0;
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
}
