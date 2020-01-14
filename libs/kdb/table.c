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

#define TRACK_REFERENCES 0

#define KONST const
#include "table-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "kdb-priv.h"
#undef KONST

#include <kdb/extern.h>

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h>
#include <klib/namelist.h>
#include <klib/printf.h>
#include <klib/rc.h>

#include <kfs/arc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KTable
 *  a collection of columns indexed by row id, metadata, indices
 */

/* GetPath
 *  return the absolute path to table
 */
LIB_EXPORT rc_t CC KTableGetPath ( const KTable *self,
    const char **path )
{
    if ( self == NULL )
        return RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
    if ( path == NULL )
        return RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );

    *path = self->path;

    return 0;
}

/* Whack
 */
static
rc_t KTableWhack ( KTable *self )
{
    rc_t rc = 0;

    KRefcountWhack ( & self -> refcount, "KTable" );

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc == 0 )
            self -> db = NULL;
    }

    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );

    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "KTable", "whack", "ktbl" );

    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KTableAddRef ( const KTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KTable" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcTable, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KTableRelease ( const KTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KTable" ) )
        {
        case krefWhack:
            return KTableWhack ( ( KTable* ) self );
        case krefNegative:
            return RC ( rcDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 */
KTable *KTableAttach ( const KTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KTable" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KTable* ) self;
}

/* Sever
 *  like KTableRelease, except called internally
 *  indicates that a child object is letting go...
 */
rc_t KTableSever ( const KTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KTable" ) )
        {
        case krefWhack:
            return KTableWhack ( ( KTable* ) self );
        case krefNegative:
            return RC ( rcDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

void KTableGetName(KTable const *self, char const **rslt)
{
    char *sep;
    
    *rslt = self->path;
    sep = strrchr(self->path, '/');
    if (sep != NULL)
        *rslt = sep + 1;
}

/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
static
rc_t KTableMake ( KTable **tblp, const KDirectory *dir, const char *path )
{
    KTable *tbl;

    assert ( tblp != NULL );
    assert ( path != NULL );

    tbl = malloc ( sizeof * tbl + strlen ( path ) );
    if ( tbl == NULL )
    {
        * tblp = NULL;
        return RC ( rcDB, rcTable, rcConstructing, rcMemory, rcExhausted );
    }

    memset ( tbl, 0, sizeof * tbl );
    tbl -> dir = dir;
    KRefcountInit ( & tbl -> refcount, 1, "KTable", "make", path );
    strcpy ( tbl -> path, path );

    * tblp = tbl;
    return 0;
}

static void ad(const KDBManager * self, const char * aPath, char ** path)
{
    const char *slash = strrchr(aPath, '/');

    assert(self && aPath && path);

    if (slash != NULL)
        ++slash;
    else
        slash = aPath;

    if ((KDirectoryPathType(self->wd, aPath) & ~kptAlias) != kptDir)
        return;

    if ((KDirectoryPathType(self->wd, "%s/%s.sra", aPath, slash)
        & ~kptAlias) != kptFile)
    {
        return;
    }

    *path = calloc(1, strlen(aPath) + 6 + strlen(slash));

    if (*path != NULL)
        sprintf(*path, "%s/%s.sra", aPath, slash);

    /* YES, DBG_VFS should be used here to be printed along with other VFS messages */
    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_SERVICE),
        ("'%s' was sesolved to '%s'\n", aPath, *path ));
}

/* OpenTableRead
 * VOpenTableRead
 *  open a table for read
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
static
rc_t KDBManagerVOpenTableReadInt ( const KDBManager *self,
    const KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, va_list args, const struct VPath *vpath )
{
    rc_t rc;
    char aTblpath[4096];
    char * tblpath = aTblpath;
    int z = ( args == NULL ) ?
        snprintf ( aTblpath, sizeof aTblpath, "%s", path ):
        vsnprintf ( aTblpath, sizeof aTblpath, path, args );
    if ( z < 0 || ( size_t ) z >= sizeof aTblpath )
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    else
    {
        KTable *tbl;
        const KDirectory *dir;
        bool prerelease = false;

        ad(self, aTblpath, &tblpath);

        rc = KDBOpenPathTypeRead ( self, wd, tblpath, &dir, kptTable, NULL,
            try_srapath, vpath );
        if ( rc != 0 )
        {
            prerelease = true;
            rc = KDBOpenPathTypeRead ( self, wd, tblpath, &dir, kptPrereleaseTbl, NULL,
                try_srapath, vpath );
        }

        if (aTblpath != tblpath)
            free(tblpath);

        if ( rc == 0 )
        {
            rc = KTableMake ( & tbl, dir, tblpath );
            if ( rc == 0 )
            {
                tbl -> mgr = KDBManagerAttach ( self );
                tbl -> prerelease = prerelease;
                * tblp = tbl;
                return 0;
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC KDBManagerOpenTableRead ( const KDBManager *self,
    const KTable **tbl, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenTableRead ( self, tbl, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenTableRead ( const KDBManager *self,
    const KTable **tbl, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenTableReadInt ( self, tbl, self -> wd, true, path, args,
        NULL );
}

LIB_EXPORT rc_t CC KDBManagerOpenTableReadVPath ( const KDBManager *self,
    const KTable **tbl, const struct VPath *path )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenTableReadInt ( self, tbl, self->wd, true, NULL, NULL, path );
}


LIB_EXPORT rc_t CC KDatabaseOpenTableRead ( const KDatabase *self,
    const KTable **tbl, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenTableRead ( self, tbl, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVOpenTableRead ( const KDatabase *self,
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
        rc = KDBManagerVOpenTableReadInt ( self -> mgr, tblp,
                                          self -> dir, false, path, NULL, NULL );
        if ( rc == 0 )
        {
            KTable *tbl = ( KTable* ) * tblp;
            tbl -> db = KDatabaseAttach ( self );
        }
    }

    return rc;
}


/* Locked
 *  returns non-zero if locked
 */
LIB_EXPORT bool CC KTableLocked ( const KTable *self )
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
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT bool CC KTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args )
{
    if ( self != NULL && name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        const char *ns;
        char path [ 256 ];

        switch ( type )
        {
        case kptIndex:
            ns = "idx";
            break;

        case kptColumn:
            ns = "col";
            break;

        default:
            return false;
        }

        rc = KDBVMakeSubPath ( self -> dir, path, sizeof path, ns, 3, name, args );
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

LIB_EXPORT bool CC KTableExists ( const KTable *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KTableVExists ( self, type, name, args );

    va_end ( args );

    return exists;
}


/* IsAlias
 *  returns true if object name is an alias
 *  returns path to fundamental name if it was aliased
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptIndex and kptColumn
 *
 *  "resolved" [ OUT ] and "rsize" [ IN ] - optional output buffer
 *  for fundamenta object name if "alias" is not a fundamental name, 
 *
 *  "name" [ IN ] - NUL terminated object name
 */
LIB_EXPORT bool CC KTableIsAlias ( const KTable *self, uint32_t type,
    char *resolved, size_t rsize, const char *name )
{
    if ( self != NULL && name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        const char *ns;
        char path [ 256 ];

        switch ( type )
        {
        case kptIndex:
            ns = "idx";
            break;

        case kptColumn:
            ns = "col";
            break;

        default:
            return false;
        }

        rc = KDBMakeSubPath ( self -> dir, path, sizeof path, ns, 3, name );
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
 *  valid values are kptIndex and kptColumn
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC KTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return -1;
}

LIB_EXPORT rc_t CC KTableWritable ( const KTable *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = KTableVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC KTableOpenManagerRead ( const KTable *self, const KDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
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
LIB_EXPORT rc_t CC KTableOpenParentRead ( const KTable *self, const KDatabase **db )
{
    rc_t rc;

    if ( db == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KDatabaseAddRef ( self -> db );
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


/* GetDirectory
 *  access the directory in use
 */
LIB_EXPORT rc_t CC KTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
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
LIB_EXPORT rc_t CC KTableModDate ( const KTable *self, KTime_t *mtime )
{
    rc_t rc;

    if ( mtime == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            /* HACK ALERT - there needs to be a proper way to record modification times */
            const KDirectory *dir = self -> dir;

            /* this only tells the last time the table was locked,
               which may be close to the last time it was modified */
            rc = KDirectoryDate ( dir, mtime, "lock" );
            if ( rc == 0 )
                return 0;

            if ( GetRCState ( rc ) == rcNotFound )
            {
                rc = KDirectoryDate ( dir, mtime, "sealed" );
                if ( rc == 0 )
                    return 0;
            }

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
 * KNamelist
 */

/* List
 *  create table listings
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

LIB_EXPORT rc_t CC KTableListCol ( const KTable *self, KNamelist **names )
{
    if ( self != NULL )
    {
        struct FilterData data;

        data.mgr = self->mgr;
        data.type = kptColumn;

        return KDirectoryList ( self -> dir,
            names, KDatabaseListFilter, &data, "col" );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcTable, rcListing, rcSelf, rcNull );
}

static
bool CC KTableListIdxFilter ( const KDirectory *dir, const char *name, void *data )
{
    const size_t sz = strlen(name);
    
    if (sz > 4 && strcmp(&name[sz - 4], ".md5") == 0)
        return false;
    return true;
}

static
bool CC KTableListSkeyFilter ( const KDirectory *dir, const char *name, void *data )
{
    if ( strcmp ( name, "skey" ) == 0 )
        return true;
    return false;
}

LIB_EXPORT rc_t CC KTableListIdx ( const KTable *self, KNamelist **names )
{
    if ( self != NULL )
    {
        if ( ! self -> prerelease )
        {
            return KDirectoryList ( self -> dir,
                names, KTableListIdxFilter, NULL, "idx" );
        }

        return KDirectoryList ( self -> dir,
            names, KTableListSkeyFilter, NULL, "." );
    }

    if ( names != NULL )
        * names = NULL;

    return RC ( rcDB, rcTable, rcListing, rcSelf, rcNull );
}

/* HasRemoteData
 *  indicates whether some/all table data comes from network resource
 *  such as HttpFile or CacheteeFile
 */
KDB_EXTERN bool CC KTableHasRemoteData ( const KTable *self )
{
    bool result = self != NULL && KDirectoryIsKArcDir ( self -> dir ) &&
            KArcDirIsFromRemote ( (const KArcDir * ) self -> dir );
    return result;
}
