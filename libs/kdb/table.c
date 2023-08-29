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
#include "column-priv.h"
#undef KONST

#include <kdb/extern.h>

#include <klib/debug.h> /* DBGMSG */
#include <klib/log.h>
#include <klib/namelist.h>
#include <klib/printf.h>
#include <klib/rc.h>

#include <vfs/manager.h> /* VFSManagerCheckEnvAndAd */
#include <vfs/path.h> /* VFSManagerMakePath */

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

static rc_t KRTableWhack ( KTable *self );
static bool CC KRTableLocked ( const KTable *self );
static bool CC KRTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args );
static bool CC KRTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KRTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args );
static rc_t CC KRTableOpenManagerRead ( const KTable *self, const KDBManager **mgr );
static rc_t CC KRTableOpenParentRead ( const KTable *self, const KDatabase **db );
static bool CC KRTableHasRemoteData ( const KTable *self );
static rc_t CC KRTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir );
static rc_t CC KRTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args );

static KTableBase_vt KRTable_vt =
{
    KRTableWhack,
    KTableBaseAddRef,
    KTableBaseRelease,
    KRTableLocked,
    KRTableVExists,
    KRTableIsAlias,
    KRTableVWritable,
    KRTableOpenManagerRead,
    KRTableOpenParentRead,
    KRTableHasRemoteData,
    KRTableOpenDirectoryRead,
    KRTableVOpenColumnRead
};

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
rc_t
KRTableWhack ( KTable *self )
{
    rc_t rc = 0;

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
        return KTableBaseWhack( self );
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KTable", "whack", "ktbl" );

    return rc;
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
    tbl -> dad . vt = & KRTable_vt;
    KRefcountInit ( & tbl -> dad . refcount, 1, "KTable", "make", path );
    tbl -> dir = dir;
    strcpy ( tbl -> path, path );

    /* YES,
      DBG_VFS should be used here to be printed along with other VFS messages */
    DBGMSG(DBG_VFS, DBG_FLAG(DBG_VFS_SERVICE), ("Making KTable '%s'\n", path));

    * tblp = tbl;
    return 0;
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
    const char *path, va_list args, const struct VPath *vpath,
    bool tryEnvAndAd )
{
    rc_t rc;
    char aTblpath[4096] = "";
    char * tblpath = aTblpath;
    int z = 0;
    /*VDB-4386: cannot treat va_list as a pointer!*/
/*    if (args == NULL) {
        if (path != NULL)
            z = snprintf(aTblpath, sizeof aTblpath, "%s", path);
    }
    else*/
    if (path != NULL)
        z = vsnprintf ( aTblpath, sizeof aTblpath, path, args );
    if ( z < 0 || ( size_t ) z >= sizeof aTblpath )
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    else
    {
        KTable *tbl;
        const KDirectory *dir;
        bool prerelease = false;
        const VPath *path2 = NULL;

        {
            rc_t rc = 0;
            VPath *path = NULL;
            if (vpath == NULL)
                rc = VFSManagerMakePath(self->vfsmgr, &path, "%s", aTblpath);
            if (rc == 0) {
                const String * str = NULL;
                if (tryEnvAndAd)
                    VFSManagerCheckEnvAndAd(self->vfsmgr,
                        path != NULL ? path : vpath, &path2);
                if (path2 != NULL) {
                    rc = VPathMakeString(path2, &str);
                    if (rc == 0) {
                        assert(str);
                        tblpath = calloc(1, str->size + 1);
                        if (tblpath != NULL)
                            string_printf(tblpath, str->size + 1, NULL,
                                "%S", str);
                        StringWhack(str);
                    }
                }
                else {
                    rc = VPathMakeString(path != NULL ? path : vpath, &str);
                    if (rc == 0) {
                        assert(str);
                        tblpath = calloc(1, str->size + 1);
                        if (tblpath != NULL)
                            string_printf(tblpath, str->size + 1, NULL,
                                "%S", str);
                        StringWhack(str);
                    }
                }
                VPathRelease(path);
            }
        }

        rc = KDBOpenPathTypeRead ( self, wd, tblpath, &dir, kptTable, NULL,
            try_srapath, path2 != NULL ? path2 : vpath );
        if ( rc != 0 )
        {
            prerelease = true;
            rc = KDBOpenPathTypeRead ( self, wd, tblpath, &dir,
                kptPrereleaseTbl, NULL,
                try_srapath, path2 != NULL ? path2 : vpath);
        }

        if (rc == 0)
        {
            String str;
            const char * p = tblpath;
            if (p == NULL) {
                if (path2 != NULL) {
                    rc_t rc = VPathGetPath(vpath, &str);
                    if (rc == 0)
                        p = str.addr;
                }
                else if (vpath != NULL) {
                    rc_t rc = VPathGetPath(vpath, &str);
                    if (rc == 0)
                        p = str.addr;
                }
            }

            VPathRelease(path2);
            path2 = NULL;

            rc = KTableMake ( & tbl, dir, p );
            if ( rc == 0 )
            {
                tbl -> mgr = KDBManagerAttach ( self );
                tbl -> prerelease = prerelease;
                * tblp = tbl;

                if (aTblpath != tblpath)
                    free(tblpath);

                return 0;
            }
            KDirectoryRelease ( dir );
        }
    }

    if (aTblpath != tblpath)
        free(tblpath);

    return rc;
}

static
rc_t KDBManagerVOpenTableReadInt_noargs ( const KDBManager *self,
    const KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, bool tryEnvAndAd, const struct VPath *vpath,
    ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, vpath );
    rc = KDBManagerVOpenTableReadInt ( self, tblp, wd, try_srapath, path, args, vpath, tryEnvAndAd );
    va_end ( args );

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

    return KDBManagerVOpenTableReadInt ( self, tbl, self -> wd, true, path,
        args, NULL, true );
}

LIB_EXPORT rc_t CC KDBManagerOpenTableReadVPath ( const KDBManager *self,
    const KTable **tbl, const struct VPath *path )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenTableReadInt_noargs ( self, tbl, self->wd, true, "",
        true, path );
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


/* Locked
 *  returns non-zero if locked
 */
static
bool CC
KRTableLocked ( const KTable *self )
{
    rc_t rc = KDBWritable ( self -> dir, "." );
    return GetRCState ( rc ) == rcLocked;
}


/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *
 *  "path" [ IN ] - NUL terminated path
 */
static
bool CC
KRTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args )
{
    if ( name != NULL && name [ 0 ] != 0 )
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
static
bool CC
KRTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    if ( name != NULL && name [ 0 ] != 0 )
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
static
rc_t CC
KRTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args )
{
    /* TBD */
    return -1;
}

/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static rc_t CC KRTableOpenManagerRead ( const KTable *self, const KDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KDBManagerAddRef ( self -> mgr );
        if ( rc == 0 )
        {
            * mgr = self -> mgr;
            return 0;
        }
    }

    return rc;
}


/* OpenParent
 *  duplicate reference to parent database
 *  NB - returned reference must be released
 */
static
rc_t CC
KRTableOpenParentRead ( const KTable *self, const KDatabase **db )
{
    rc_t rc;

    if ( db == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KDatabaseAddRef ( self -> db );
        if ( rc == 0 )
        {
            * db = self -> db;
            return 0;
        }
    }

    return rc;
}


/* GetDirectory
 *  access the directory in use
 */
static
rc_t CC
KRTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        * dir = self -> dir;
        return KDirectoryAddRef ( * dir );
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
static
bool CC
KRTableHasRemoteData ( const KTable *self )
{
    bool result = KDirectoryIsKArcDir ( self -> dir ) &&
            KArcDirIsFromRemote ( (const KArcDir * ) self -> dir );
    return result;
}



/* OpenColumnRead
 * VOpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVOpenColumnReadInt ( const KDBManager *self,
    const KColumn **colp, const KDirectory *wd, bool try_srapath,
    const char *path, va_list args )
{
    char colpath [ 4096 ];
    rc_t rc;
    size_t z;

/*    rc = KDirectoryVResolvePath ( wd, 1,
        colpath, sizeof colpath, path, args ); */
    rc = string_vprintf( colpath, sizeof colpath, &z, path, args );
    if ( rc == 0 )
    {
        KColumn *col;
        const KDirectory *dir;

        /* open table directory */
        rc = KDBOpenPathTypeRead ( self, wd, colpath, &dir, kptColumn, NULL,
            try_srapath, NULL );
        if ( rc == 0 )
        {
            rc = KColumnMakeRead ( & col, dir, colpath );
            if ( rc == 0 )
            {
                col -> mgr = KDBManagerAttach ( self );
                * colp = col;
                return 0;
            }

            KDirectoryRelease ( dir );
        }
    }

    return rc;
}

static
rc_t KDBManagerVOpenColumnReadInt_noargs ( const KDBManager *self,
    const KColumn **colp, const KDirectory *wd, bool try_srapath,
    const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenColumnReadInt ( self, colp, wd, try_srapath, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenColumnRead ( const KDBManager *self,
    const KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenColumnRead ( self, col, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KDBManagerVOpenColumnRead ( const KDBManager *self,
    const KColumn **col, const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * col = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenColumnReadInt
        ( self, col, self -> wd, true, path, args );
}

static
rc_t CC
KRTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( colp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * colp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "col", 3, name, args );
    if ( rc == 0 )
    {
        rc = KDBManagerVOpenColumnReadInt_noargs ( self -> mgr,
                                           colp, self -> dir, false, path );
        if ( rc == 0 )
        {
            KColumn *col = ( KColumn* ) * colp;
            col -> tbl = KTableAttach ( self );
        }
    }
    return rc;
}


