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
#include "database-priv.h"
#include "dbmgr-priv.h"
#include "kdb-priv.h"
#undef KONST

#include <sysalloc.h>
#include <klib/namelist.h>
#include <klib/rc.h>
#include <kfs/md5.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KDatabase
 *  connection to a database within file system
 */

/* Whack
 */
static
rc_t KDatabaseWhack ( KDatabase *self )
{
    rc_t rc = 0;

    /* unused: const KDBManager *mgr; */

    KRefcountWhack ( & self -> refcount, "KDatabase" );

    /* shut down md5 fmt file */
    if ( self -> md5 != NULL )
    {
        rc = KMD5SumFmtRelease ( self -> md5 );
        if ( rc == 0 )
            self -> md5 = NULL;
    }
    if ( rc == 0 )
    {
        /* release dad */
        if ( self -> dad != NULL )
        {
            rc = KDatabaseSever ( self -> dad );
            if ( rc == 0 )
                self -> dad = NULL;
        }
        /* remove from mgr */
        if ( rc == 0 )
        {
            /* rc = KDBManagerSever ( self -> mgr, & self -> sym ); */
			rc = KDBManagerSever ( self -> mgr );

            /* complete */
            if ( rc == 0 )
            {
                KDirectoryRelease ( self -> dir );
                free ( self );
                return 0;
            }
        }
    }

    KRefcountInit ( & self -> refcount, 1, "KDatabase", "whack", "kdb" );

    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t KDatabaseAddRef ( const KDatabase *self )
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

rc_t KDatabaseRelease ( const KDatabase *self )
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

    memset ( db, 0, sizeof * db );
    db -> dir = dir;
    KRefcountInit ( & db -> refcount, 1, "KDatabase", "make", path );
    strcpy ( db -> path, path );

    * dbp = db;
    return 0;
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
rc_t KDBManagerVOpenDBReadInt ( const KDBManager *self,
    const KDatabase **dbp, const KDirectory *wd,
    const char *path, va_list args )
{
    /* generate absolute path to db */
    char dbpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
    {
        KDatabase *db;
        const KDirectory *dir;

        /* open the directory if its a database */
        rc = KDBOpenPathTypeRead ( wd, dbpath, &dir, kptDatabase, NULL );
        if ( rc == 0 )
        {
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

rc_t KDBManagerOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBRead ( self, db, path, args );
    va_end ( args );

    return rc;
}

rc_t KDatabaseOpenDBRead ( const KDatabase *self,
    const KDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenDBRead ( self, db, name, args );
    va_end ( args );

    return rc;
}

rc_t KDBManagerVOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenDBReadInt ( self, db, self -> wd, path, args );
}

rc_t KDatabaseVOpenDBRead ( const KDatabase *self,
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
        rc = KDBManagerOpenDBReadInt ( self -> mgr, dbp,
                                       self -> dir, "%s", path );
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
bool KDatabaseLocked ( const KDatabase *self )
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
bool KDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return false;
}

bool KDatabaseExists ( const KDatabase *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KDatabaseVExists ( self, type, name, args );

    va_end ( args );

    return exists;
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
rc_t KDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return -1;
}

rc_t KDatabaseWritable ( const KDatabase *self, uint32_t type, const char *name, ... )
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
rc_t KDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr )
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
rc_t KDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par )
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


/*--------------------------------------------------------------------------
 * KNameList
 */

/* List
 *  create database listings
 */
static
bool KDatabaseListFilter ( const KDirectory *dir, const char *name, void *data )
{
    return ( KDBOpenPathTypeRead ( dir, name, NULL, (( long int ) data ), NULL ) == 0 );
}

rc_t KDatabaseListDB ( const KDatabase *self, KNamelist **names )
{
    if ( self != NULL )
    {
        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, ( void* ) kptDatabase, "db" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcDatabase, rcListing, rcSelf, rcNull );
}

rc_t KDatabaseListTbl ( struct KDatabase const *self, KNamelist **names )
{
    if ( self != NULL )
    {
        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, ( void* ) kptTable, "tbl" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcDatabase, rcListing, rcSelf, rcNull );
}

rc_t KDatabaseListIdx ( struct KDatabase const *self, KNamelist **names )
{
    if ( self != NULL )
    {
        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, ( void* ) kptIndex, "idx" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcDatabase, rcListing, rcSelf, rcNull );
}
   
