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

#include <kdb/extern.h>
#include "database-priv.h"
#include "wdbmgr.h"
#include "wtable-priv.h"
#include "wcolumn-priv.h"
#include "windex-priv.h"
#include "wkdb-priv.h"
#include "wmeta.h"

#include <klib/namelist.h>
#include <klib/symbol.h>
#include <kdb/namelist.h>
#include <klib/log.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/impl.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


/* keep file system structure compatible with v0 code */
#define V0_BACKWARD_COMPAT 0

/*--------------------------------------------------------------------------
 * KTable
 *  a collection of columns indexed by row id, metadata, indices
 */

static rc_t KWTableWhack ( KTable *self );
static bool CC KWTableLocked ( const KTable *self );
static bool CC KWTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args );
static bool CC KWTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KWTableVWritable ( const KTable *self, uint32_t type, const char *name, va_list args );
static rc_t CC KWTableOpenManagerRead ( const KTable *self, const KDBManager **mgr );
static rc_t CC KWTableOpenParentRead ( const KTable *self, const KDatabase **db );
static bool CC KWTableHasRemoteData ( const KTable *self );
static rc_t CC KWTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir );
static rc_t CC KWTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args );
static rc_t CC KWTableOpenMetadataRead ( const KTable *self, const KMetadata **metap );

static KTableBase_vt KWTable_vt =
{
    KWTableWhack,
    KTableBaseAddRef,
    KTableBaseRelease,
    KWTableLocked,
    KWTableVExists,
    KWTableIsAlias,
    KWTableVWritable,
    KWTableOpenManagerRead,
    KWTableOpenParentRead,
    KWTableHasRemoteData,
    KWTableOpenDirectoryRead,
    KWTableVOpenColumnRead,
    KWTableOpenMetadataRead
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
KWTableWhack ( KTable *self )
{
    rc_t rc = 0;
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;
    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc != 0 )
            return rc;
        self -> db = NULL;
    }

    /* shut down md5 sum file if it is open */
    KMD5SumFmtRelease ( self -> md5 ), self -> md5 = NULL;

    if ( mgr != NULL )
    {
        /* remove from mgr */
        symb = KDBManagerOpenObjectFind (mgr, self->path);
        if (symb != NULL)
        {
            rc = KDBManagerOpenObjectDelete (mgr, symb);
            if (rc == 0)
            {
                rc = KDBManagerSever ( mgr );
                if ( rc != 0 )
                    rc = KDBManagerOpenObjectAdd (mgr, symb);
            }
        }
    }

    if ( rc == 0 )
    {
        /* complete */
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
rc_t KTableMake ( KTable **tblp, const KDirectory *dir, const char *path,
		  KMD5SumFmt * md5, bool read_only )
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
    tbl -> dad . vt = & KWTable_vt;
    KRefcountInit ( & tbl -> dad . refcount, 1, "KTable", "make", path );
    tbl -> dir = ( KDirectory* ) dir;
    tbl -> opencount = 1;
    tbl -> md5 = md5;
    KMD5SumFmtAddRef ( md5 );
    tbl -> use_md5 = ( md5 == NULL ) ? false : true;
    tbl -> read_only = read_only;
    strcpy ( tbl -> path, path );

    tbl->sym.u.obj = tbl;
    StringInitCString (&tbl->sym.name, tbl->path);
    tbl->sym.type = kptTable;

    * tblp = tbl;
    return 0;
}

/* Locked
 *  returns non-zero if locked
 */
static
bool CC
KWTableLocked ( const KTable *self )
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
KWTableVExists ( const KTable *self, uint32_t type, const char *name, va_list args )
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
KWTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name )
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
rc_t KTableLockInt (const KTable  * self, char * path, size_t path_size,
                        int type, const char * name, va_list args )
{
    rc_t rc;

    if (self == NULL)
        rc = RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );

    else if (name == NULL)
        rc =  RC (rcDB, rcMgr, rcAccessing, rcPath, rcNull);

    else
    {
        const char * ns;

        ns = KDBGetNamespaceString (type);

        switch (type)
        {
        default:
            rc = RC (rcDB, rcMgr, rcAccessing, rcParam, rcInvalid);
            break;
        case kptColumn:
        case kptIndex:
            rc = KDBVMakeSubPath (self->dir, path, path_size, ns, strlen (ns),
                                  name, args);
            break;
        }
    }
    return rc;
}

static
rc_t CC
KWTableVWritable (const KTable *self, uint32_t type, const char * name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    rc = KTableLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBWritable (self->dir, path);
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
LIB_EXPORT rc_t CC KTableVLock ( KTable *self, uint32_t type, const char * name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    rc = KTableLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBLockDir (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KTableLock ( KTable *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KTableVLock (self, type, name, args);
    va_end (args);
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
LIB_EXPORT rc_t CC KTableVUnlock ( KTable *self, uint32_t type, const char * name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    rc = KTableLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBUnlockDir (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KTableUnlock ( KTable *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, name );
    rc = KTableVUnlock ( self, type, name, args );
    va_end ( args );
    return rc;
}


/* Rename
 */
LIB_EXPORT rc_t CC KTableRenameColumn ( KTable *self, bool force, const char *from, const char *to )
{
    return KDBRename ( self -> dir, self -> mgr, kptColumn, force, from, to );
}

LIB_EXPORT rc_t CC KTableRenameIndex ( KTable *self, bool force, const char *from, const char *to )
{
    return KDBRename ( self -> dir, self -> mgr, kptIndex, force, from, to );
}

/* Alias
 */
LIB_EXPORT rc_t CC KTableAliasColumn ( KTable *self, const char *col, const char *alias )
{
    return KDBAlias ( self -> dir, kptColumn, col, alias );
}

LIB_EXPORT rc_t CC KTableAliasIndex ( KTable *self, const char *idx, const char *alias )
{
    return KDBAlias ( self -> dir, kptIndex, idx, alias );
}

/* Drop
 */
LIB_EXPORT rc_t CC KTableDropColumn ( KTable *self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KTableVDropColumn ( self, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVDropColumn ( KTable *self, const char *name, va_list args )
{
    return KDBVDrop ( self -> dir, self -> mgr, kptColumn, name, args );
}

LIB_EXPORT rc_t CC KTableDropIndex ( KTable *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KTableVDropIndex ( self, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVDropIndex ( KTable *self, const char *name, va_list args )
{
    return KDBVDrop ( self -> dir, self -> mgr, kptIndex, name, args );
}


/* Reindex
 *  optimize column indices
 */
LIB_EXPORT rc_t CC KTableReindex ( KTable *self )
{
    if ( self != NULL )
    {
        KNamelist *names;
        rc_t rc = KTableListCol ( self, & names );
        if ( rc == 0 )
        {
            uint32_t count;
            rc = KNamelistCount ( names, & count );
            if ( rc == 0 )
            {
                uint32_t i;
                for ( i = 0; i < count; ++ i )
                {
                    KColumn *col;

                    const char *name;
                    rc = KNamelistGet ( names, i, & name );
                    if ( rc != 0 )
                        break;

                    /* check it the column has idx0 data
                       TBD - this whole operation goes away when
                       idx0 is used for cursor sessions */
                    if ( KTableColumnNeedsReindex ( self, name ) )
                    {
                        rc = KTableOpenColumnUpdate ( self, & col, "%s", name );
                        if ( rc != 0 )
                        {
                            if ( GetRCState ( rc ) == rcBusy )
                                continue;
                            break;
                        }

                        rc = KColumnReindex ( col );
                        KColumnRelease ( col );
                        if ( rc != 0 )
                            break;
                    }
                }
            }

            KNamelistRelease ( names );
        }
        return rc;
    }
    return RC ( rcDB, rcTable, rcReindexing, rcSelf, rcNull );
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static
rc_t CC
KWTableOpenManagerRead ( const KTable *self, const KDBManager **mgr )
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

        * mgr = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KTableOpenManagerUpdate ( KTable *self, KDBManager **mgr )
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
static
rc_t CC
KWTableOpenParentRead ( const KTable *self, const KDatabase **db )
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

        * db = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KTableOpenParentUpdate ( KTable *self, KDatabase **db )
{
    rc_t rc;

    if ( db == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
        else if ( self -> db != NULL && self -> db -> read_only )
            rc = RC ( rcVDB, rcTable, rcAccessing, rcDatabase, rcReadonly );
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


/* OpenDirectory
 *  access the directory in use
 */
static
rc_t CC
KWTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir )
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

LIB_EXPORT rc_t CC KTableOpenDirectoryUpdate ( KTable *self, KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
         else if ( self -> dir != NULL && self -> dir -> read_only )
             rc = RC ( rcDB, rcTable, rcAccessing, rcDirectory, rcReadonly );
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
        if ( self != NULL )
            return KDBGetObjModDate ( self -> dir, mtime );

        rc = RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
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

    return KDBOpenPathTypeRead(data->mgr, dir, name, NULL, data->type, NULL, false,
        NULL ) == 0;
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
KWTableHasRemoteData ( const KTable *self )
{
    return true;
}

/** @brief Copy a column using directory operations
 */
static rc_t KTableCopyObject_int(  KTable *self
                                 , KTable const *source
                                 , const char *name
                                 , char const *type)
{
    KDirectory *dst = NULL;
    KDirectory const *src = NULL;
    rc_t rc;

    rc = KDirectoryOpenDirRead(source->dir, &src, true, "%s", type);
    if (rc)
        return rc;

    rc = KDirectoryOpenDirUpdate(self->dir, &dst, true, "%s", type);
    if (rc == 0) {
        rc = KDirectoryCopyPaths(src, dst, true, name, name);
        KDirectoryRelease(dst);
    }
    KDirectoryRelease(src);
    return rc;
}

/** @brief A good source exists and is not being written to.
 */
static rc_t KTableObjectIsGoodSource(KTable const *self
                                     , char const *name
                                     , char const *type)
{
    char path [ 256 ];
    rc_t rc = KDBMakeSubPath(self->dir, path, sizeof(path), type, 3, "%s", name);
    assert(rc == 0);
    if (rc)
        return rc;

    rc = KDBWritable(self->dir, path);
    if (rc == 0) {
        if (KDBManagerOpenObjectBusy(self->mgr, path))
            return RC ( rcDB, rcTable, rcCopying, rcPath, rcBusy );
    }
    else if (GetRCState(rc) == rcLocked || GetRCState(rc) == rcReadonly)
        rc = 0;
    return rc;
}

/** @brief A good destination does not exist.
 */
static rc_t KTableObjectIsGoodDestination(KTable *self
                                          , char const *name
                                          , char const *type)
{
    char path [ 256 ];
    rc_t rc = KDBMakeSubPath(self->dir, path, sizeof(path), type, 3, "%s", name);
    assert(rc == 0);
    if (rc)
        return rc;

    rc = KDBWritable(self->dir, path);
    if (rc == 0)
        return RC(rcDB, rcTable, rcCopying, rcPath, rcExists);
    if (GetRCState(rc) == rcLocked || GetRCState(rc) == rcReadonly)
        return RC(rcDB, rcTable, rcCopying, rcPath, GetRCState(rc));
    return 0;
}

static rc_t KTableCopyObject(  KTable *self
                             , KTable const *source
                             , char const *name
                             , char const *type)
{
    rc_t rc = KTableObjectIsGoodSource(source, name, type);
    if (rc)
        return rc;

    rc = KTableObjectIsGoodDestination(self, name, type);
    if (rc)
        return rc;

    return KTableCopyObject_int(self, source, name, type);
}

LIB_EXPORT rc_t CC KTableCopyColumn(  KTable *self
                                    , KTable const *source
                                    , const char *name)
{
    rc_t rc = 0;

    if (self == NULL)
        return RC(rcDB, rcTable, rcCopying, rcSelf, rcNull);

    if (self->read_only)
        return RC(rcDB, rcTable, rcCopying, rcSelf, rcReadonly);

    rc = KTableCopyObject(self, source, name, "col");
    if (rc)
        return RC(rcDB, rcTable, rcCopying, rcColumn, GetRCState(rc));
    return rc;
}

LIB_EXPORT rc_t CC KTableCopyIndex(  KTable *self
                                    , KTable const *source
                                    , const char *name)
{
    rc_t rc = 0;

    if (self == NULL)
        return RC(rcDB, rcTable, rcCopying, rcSelf, rcNull);

    if (self->read_only)
        return RC(rcDB, rcTable, rcCopying, rcSelf, rcReadonly);

    rc = KTableCopyObject(self, source, name, "idx");
    return RC(rcDB, rcTable, rcCopying, rcIndex, GetRCState(rc));
}

static
rc_t CC
KWTableVOpenColumnRead ( const KTable *self, const KColumn **colp, const char *name, va_list args )
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
	bool col_is_cached;
        rc = KDBManagerVOpenColumnReadInt_noargs ( self -> mgr,
                                            colp, self -> dir, path, &col_is_cached, false );
        if ( rc == 0 )
        {
            KColumn *col = ( KColumn* ) * colp;
            if(!col_is_cached) col -> tbl = KTableAttach ( self );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KTableCreateColumn ( KTable *self, KColumn **col,
    KCreateMode cmode, KChecksum checksum, size_t pgsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KTableVCreateColumn ( self, col, cmode, checksum, pgsize, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVCreateColumn ( KTable *self, KColumn **colp,
    KCreateMode cmode, KChecksum checksum, size_t pgsize,
    const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( colp == NULL )
        return RC ( rcDB, rcTable, rcCreating, rcParam, rcNull );

    * colp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcCreating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcTable, rcCreating, rcColumn, rcReadonly );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "col", 3, name, args );
    if ( rc == 0 )
    {
        /* set MD5 mode according to table */
        if ( self -> use_md5 )
            cmode |= kcmMD5;
        else
            cmode &= ~ kcmMD5;

        rc = KDBManagerVCreateColumnInt_noargs ( self -> mgr, colp,
                                          self -> dir, cmode | kcmParents, checksum, pgsize, path );
        if ( rc == 0 )
        {
            KColumn *col = * colp;
            col -> tbl = KTableAttach ( self );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KTableOpenColumnUpdate ( KTable *self,
    KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KTableVOpenColumnUpdate ( self, col, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVOpenColumnUpdate ( KTable *self,
    KColumn **colp, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( colp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * colp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcTable, rcOpening, rcColumn, rcReadonly );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "col", 3, name, args );
    if ( rc == 0 )
    {
        rc = KDBManagerVOpenColumnUpdateInt_noargs ( self -> mgr,
                                              colp, self -> dir, false, path );
        if ( rc == 0 )
        {
            KColumn *col = * colp;
            col -> tbl = KTableAttach ( self );
        }
    }
    return rc;
}

bool KTableColumnNeedsReindex ( KTable *self, const char *colname )
{
    if ( self != NULL )
    {
        char path [ 256 ];
        rc_t rc = KDBMakeSubPath ( self -> dir,
            path, sizeof path, "col", 3, colname );
        if ( rc == 0 )
        {
            uint64_t idx0_size;
            rc = KDirectoryFileSize ( self -> dir, & idx0_size, "%s/idx0", path );
            if ( rc == 0 && idx0_size > 0 )
                return true;
        }
    }

    return false;
}

static
rc_t CC
KWTableOpenMetadataRead ( const KTable *self, const KMetadata **metap )
{
    rc_t rc;
    const KMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, self -> prerelease, &meta_is_cached );
    if ( rc == 0 )
    {
        if(!meta_is_cached) ((KMetadata*)meta) -> tbl = KTableAttach ( self );
        * metap = meta;
    }

    return rc;
}

