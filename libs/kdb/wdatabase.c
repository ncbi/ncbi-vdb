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

#include <kdb/extern.h>

#include "wdatabase.h"

#include <kdb/namelist.h>
#include <kdb/kdb-priv.h>

#include "wtable.h"
#include "windex.h"
#include "wkdb.h"
#include "kdb-cmn.h"
#include "wdbmgr.h"
#include "wmeta.h"

#include <klib/namelist.h>
#include <klib/log.h>
#include <klib/rc.h>

#include <kfs/impl.h>

#include <vfs/path.h>

/*--------------------------------------------------------------------------
 * KDatabase
 *  connection to a database within file system
 */

static rc_t CC KWDatabaseWhack ( KDatabase *self );
static bool CC KWDatabaseLocked ( const KDatabase *self );
static bool CC KWDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args );
static bool CC KWDatabaseIsAlias ( const KDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name );
static rc_t CC KWDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args );
static rc_t CC KWDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr );
static rc_t CC KWDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par );
static rc_t CC KWDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir );
static rc_t CC KWDatabaseVOpenDBRead ( const KDatabase *self, const KDatabase **dbp, const char *name, va_list args );
static rc_t CC KWDatabaseVOpenTableRead ( const KDatabase *self, const KTable **tblp, const char *name, va_list args );
static rc_t CC KWDatabaseOpenMetadataRead ( const KDatabase *self, const KMetadata **metap );
static rc_t CC KWDatabaseVOpenIndexRead ( const KDatabase *cself, const KIndex **idxp, const char *name, va_list args );
static rc_t CC KWDatabaseListDB ( const KDatabase *self, KNamelist **names );
static rc_t CC KWDatabaseListTbl ( struct KDatabase const *self, KNamelist **names );
static rc_t CC KWDatabaseListIdx ( struct KDatabase const *self, KNamelist **names );
static rc_t CC KWDatabaseGetPath ( struct KDatabase const *self, const char **path );

static KDatabase_vt KRDatabase_vt =
{
    KWDatabaseWhack,
    KDatabaseBaseAddRef,
    KDatabaseBaseRelease,
    KWDatabaseLocked,
    KWDatabaseVExists,
    KWDatabaseIsAlias,
    KWDatabaseVWritable,
    KWDatabaseOpenManagerRead,
    KWDatabaseOpenParentRead,
    KWDatabaseOpenDirectoryRead,
    KWDatabaseVOpenDBRead,
    KWDatabaseVOpenTableRead,
    KWDatabaseOpenMetadataRead,
    KWDatabaseVOpenIndexRead,
    KWDatabaseListDB,
    KWDatabaseListTbl,
    KWDatabaseListIdx,
    KWDatabaseGetPath
};


/* GetPath
 *  return the absolute path to DB
 */
static
rc_t CC
KWDatabaseGetPath ( struct KDatabase const *self, const char **path )
{
    if ( path == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    *path = self->path;
    return 0;
}

/* Whack
 */
static
rc_t CC KWDatabaseWhack ( KDatabase *self )
{
    rc_t rc = 0;
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;
    assert ( mgr != NULL );

    KRefcountWhack ( & self -> dad . refcount, "KDatabase" );

    /* release parent */
    if ( self -> parent != NULL )
    {
        rc = KDatabaseSever ( self -> parent );
        if ( rc != 0 )
            return rc;
        self -> parent = NULL;
    }

    /* shut down md5 sum file if it is open */
    KMD5SumFmtRelease ( self -> md5 ), self -> md5 = NULL;

    /* remove from mgr */
    symb = KDBManagerOpenObjectFind (mgr, self->path);
    if (symb != NULL)
    {
        rc = KDBManagerOpenObjectDelete (mgr, symb);
        if (rc == 0)
        {
            /* release manager
               should never fail */
            rc = KDBManagerSever ( mgr );
            if ( rc != 0 )
                rc = KDBManagerOpenObjectAdd (mgr, symb);
            else
            {
                /* complete */
                KDirectoryRelease ( self -> dir );
                free ( self );
                return 0;
            }
        }
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KDatabase", "whack", "kdb" );

    return rc;
}

/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
rc_t
KWDatabaseMake ( KDatabase **dbp, const KDirectory *dir, const char *path, KMD5SumFmt *md5, bool read_only )
{
    KDatabase *db;
    rc_t rc = 0;

    assert ( dbp != NULL );
    assert ( path != NULL );

    size_t path_size = strlen(path);
    db = calloc ( 1, sizeof * db + path_size);
    if ( db == NULL )
    {
        * dbp = NULL;
        return RC ( rcDB, rcDatabase, rcConstructing, rcMemory, rcExhausted );
    }

    memset ( db, 0, sizeof * db );
    db -> dad . vt = & KRDatabase_vt;
    KRefcountInit ( & db -> dad . refcount, 1, "KDatabase", "make", path );

    db -> dir = ( KDirectory* ) dir;
    db -> md5 = md5;
    rc = KMD5SumFmtAddRef ( md5 );
    db -> cmode = kcmOpen; /* to be set later by the caller */
    db -> checksum = kcsNone; /* to be set later in VTableColumnCreateParams */

    if ( md5 != NULL )
        db -> cmode |= kcmMD5;

    db -> opencount = 1;
    db -> read_only = read_only;

    string_copy ( db -> path, path_size + 1, path, path_size );

    db->sym.u.obj = db;
    StringInitCString (&db->sym.name, db->path);
    db->sym.type = kptDatabase;

    * dbp = db;
    return rc;
}

rc_t
KWDatabaseMakeVPath ( KDatabase **dbp, const KDirectory *dir,
    const VPath *path, KMD5SumFmt *md5, bool read_only )
{
    const String* dbpathStr;
    rc_t rc = VPathMakeString ( path, &dbpathStr );    /* NUL-terminated */
    if ( rc == 0 )
    {
        rc = KWDatabaseMake ( dbp, dir, dbpathStr->addr, md5, read_only );
        StringWhack(dbpathStr);
    }
    return rc;
}


/* =====
 * create a KDatabase as a sub of another KDatabase
 */
LIB_EXPORT rc_t CC KDatabaseCreateDB ( KDatabase *self,
    KDatabase **db, KCreateMode cmode, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVCreateDB ( self, db, cmode, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVCreateDB ( KDatabase *self,
    KDatabase **dbp, KCreateMode cmode, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( dbp == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcParam, rcNull );

    * dbp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcCreating, rcDatabase, rcReadonly );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "db", 2, name, args );
    if ( rc == 0 )
    {
        /* ensure that the "db" namespace directory is there */
        rc = KDirectoryCreateDir ( self -> dir,
            0775, kcmOpen, "db" );
        if ( rc == 0 )
        {
            rc = KDBManagerVCreateDBInt_noargs ( self -> mgr, dbp,
                                          self -> dir, cmode, path );
            if ( rc == 0 )
            {
                KDatabase *db = ( KDatabase* ) * dbp;
                db -> parent = self;
                atomic32_inc ( & self -> dad . refcount );
            }
        }
    }

    return rc;
}


LIB_EXPORT rc_t CC KDatabaseOpenDBUpdate ( KDatabase *self,
    KDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenDBUpdate ( self, db, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVOpenDBUpdate ( KDatabase *self,
    KDatabase **dbp, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( dbp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * dbp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "db", 2, name, args );
    if ( rc == 0 )
    {
        rc = KDBManagerVOpenDBUpdateInt_noargs ( self -> mgr, dbp,
                                         self -> dir, path );
        if ( rc == 0 )
        {
            KDatabase *db = ( KDatabase* ) * dbp;
            db -> parent = self;
            atomic32_inc ( & self -> dad . refcount );
        }
    }

    return rc;
}


/* Locked
 *  returns non-zero if locked
 */
static
bool CC
KWDatabaseLocked ( const KDatabase *self )
{
    rc_t rc;

    if ( self == NULL )
        return false;

    rc = KDBWWritable ( self -> dir, "." );
    return GetRCState ( rc ) == rcLocked;
}

/* Exists
 *  returns true if requested object exists
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptTable
 *
 *  "path" [ IN ] - NUL terminated path
 */
static
bool CC
KWDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args )
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
static
bool CC
KWDatabaseIsAlias ( const KDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name )
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
rc_t KDatabaseLockInt (const KDatabase  * self, char * unused, size_t path_size,
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
        char path [ 256 ];

        ns = KDBGetNamespaceString (type);

        switch (type)
        {
        default:
            rc = RC (rcDB, rcMgr, rcAccessing, rcParam, rcInvalid);
            break;
        case kptDatabase:
        case kptTable:
/*         case kptIndex: */
            rc = KDBVMakeSubPath (self->dir, path, sizeof path, ns, (uint32_t) strlen (ns),
                                  name, args);
            break;
        }
    }
    return rc;
}

static
rc_t CC
KWDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    rc = KDatabaseLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBWWritable (self->dir, path);
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
LIB_EXPORT rc_t CC KDatabaseVLock ( KDatabase *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    rc = KDatabaseLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBLockDir (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseLock ( KDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVLock (self, type, name, args);
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
 *  valid values are kptDatabase, kptTable and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC KDatabaseVUnlock ( KDatabase *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    rc = KDatabaseLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBUnlockDir (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseUnlock ( KDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, name );
    rc = KDatabaseVUnlock (self, type, name, args);
    va_end (args);
    return rc;
}


/* Rename
 */
LIB_EXPORT rc_t CC KDatabaseRenameDB ( KDatabase *self, bool force, const char *from, const char *to )
{
    return KDBRename ( self -> dir, self -> mgr, kptDatabase, force, from, to );
}

LIB_EXPORT rc_t CC KDatabaseRenameTable ( KDatabase *self, bool force, const char *from, const char *to )
{
    return KDBRename ( self -> dir, self -> mgr, kptTable, force, from, to );
}

#if 1
LIB_EXPORT rc_t CC KDatabaseRenameIndex ( KDatabase *self, bool force, const char *from, const char *to )
{
    return KDBRename ( self -> dir, self -> mgr, kptIndex, force, from, to );
}
#endif

/* Alias
 */
LIB_EXPORT rc_t CC KDatabaseAliasDB ( KDatabase *self, const char *db, const char *alias )
{
    return KDBAlias ( self -> dir, kptDatabase, db, alias );
}

LIB_EXPORT rc_t CC KDatabaseAliasTable ( KDatabase *self, const char *tbl, const char *alias )
{
    return KDBAlias ( self -> dir, kptTable, tbl, alias );
}

LIB_EXPORT rc_t CC KDatabaseAliasIndex ( KDatabase *self, const char *idx, const char *alias )
{
    return KDBAlias ( self -> dir, kptIndex, idx, alias );
}

/* Drop
 */

LIB_EXPORT rc_t CC KDatabaseDropDB ( KDatabase *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVDropDB ( self, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVDropDB ( KDatabase *self, const char *name, va_list args )
{
    return KDBVDrop ( self -> dir, self -> mgr, kptDatabase, name, args );
}

LIB_EXPORT rc_t CC KDatabaseDropTable ( KDatabase *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVDropTable ( self, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVDropTable ( KDatabase *self, const char *name, va_list args )
{
    return KDBVDrop ( self -> dir, self -> mgr, kptTable, name, args );
}

#if 1
LIB_EXPORT rc_t CC KDatabaseDropIndex ( KDatabase *self, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVDropIndex ( self, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVDropIndex ( KDatabase *self, const char *name, va_list args )
{
    return KDBVDrop ( self -> dir, self -> mgr, kptIndex, name, args );
}
#endif

/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static
rc_t CC
KWDatabaseOpenManagerRead ( const KDatabase *self, const KDBManager **mgr )
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

LIB_EXPORT rc_t CC KDatabaseOpenManagerUpdate ( KDatabase *self, KDBManager **mgr )
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
KWDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par )
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

LIB_EXPORT rc_t CC KDatabaseOpenParentUpdate ( KDatabase *self, KDatabase **par )
{
    rc_t rc;

    if ( par == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else if ( self -> parent != NULL && self -> parent -> read_only )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcDatabase, rcReadonly );
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


/* OpenDirectory
 *  access the directory in use
 */
static
rc_t CC
KWDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir )
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

LIB_EXPORT rc_t CC KDatabaseOpenDirectoryUpdate ( KDatabase *self, KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
         else if ( self -> dir != NULL && self -> dir -> read_only )
             rc = RC ( rcDB, rcDatabase, rcAccessing, rcDirectory, rcReadonly );
        else
        {
            * dir = self -> dir;
            return KDirectoryAddRef ( * dir );
        }

        * dir = NULL;
    }

    return rc;
}

/*--------------------------------------------------------------------------
 * KNamelist
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
bool CC
KDatabaseListFilter ( const KDirectory *dir, const char *name, void *data_ )
{
    struct FilterData * data = data_;
    return ( KDBManagerOpenPathTypeRead ( data->mgr, dir, name, NULL, data->type, NULL, false,
        NULL ) == 0 );
}

static
rc_t CC
KWDatabaseListDB ( const KDatabase *self, KNamelist **names )
{
    struct FilterData data;
    data.mgr = self->mgr;
    data.type = kptDatabase;

    return KDirectoryList ( self -> dir, names, KDatabaseListFilter, &data, "db" );
}

static
rc_t CC
KWDatabaseListTbl ( struct KDatabase const *self, KNamelist **names )
{
    struct FilterData data;
    data.mgr = self->mgr;
    data.type = kptTable;

    return KDirectoryList ( self -> dir, names, KDatabaseListFilter, &data, "tbl" );
}

static
rc_t CC
KWDatabaseListIdx ( struct KDatabase const *self, KNamelist **names )
{
    struct FilterData data;
    data.mgr = self->mgr;
    data.type = kptIndex;

    return KDirectoryList ( self -> dir, names, KDatabaseListFilter, &data, "idx" );
}

KCreateMode
KDatabaseGetCmode ( const KDatabase *self)
{
    return self -> cmode;
}
KCreateMode
KDatabaseSetCmode ( KDatabase *self, KCreateMode new_val)
{
    KCreateMode old_val = self -> cmode;
    self -> cmode = new_val;
    return old_val;
}

KChecksum
KDatabaseGetChecksum ( const KDatabase *self)
{
    return self -> checksum;
}
KChecksum
KDatabaseSetChecksum ( KDatabase *self, KChecksum new_val)
{
    KChecksum old_val = self -> checksum;
    self -> checksum = new_val;
    return old_val;
}

/* ------------------------------------------------------------------------------------ */

static
rc_t
copy_meta_for_one_table_in_db ( KDatabase *self, const KDatabase *src,
                                const char * node_path, const char * tbl_name,
                                bool src_node_has_to_exist ) {
    KTable * dst_tbl;
    rc_t rc = KDatabaseOpenTableUpdate( self, &dst_tbl, tbl_name );
    if ( 0 == rc ) {
        const KTable * src_tbl;
        rc = KDatabaseOpenTableRead( src, &src_tbl, tbl_name );
        if ( 0 == rc ) {
            rc = KTableMetaCopy( dst_tbl, src_tbl, node_path, src_node_has_to_exist );
            KTableRelease( src_tbl );
        }
        KTableRelease( dst_tbl );
    }
    return rc;
}

static
rc_t
copy_meta_for_all_tables_in_db( KDatabase *self, const KDatabase *src,
                                const char * node_path, bool src_node_has_to_exist ) {
    KNamelist * tables_1;
    rc_t rc = KDatabaseListTbl( self, &tables_1 );
    if ( 0 == rc ) {
        KNamelist * tables_2;
        rc = KDatabaseListTbl( src, &tables_2 );
        if ( 0 == rc ) {
            uint32_t count;
            rc = KNamelistCount( tables_1, &count );
            if ( 0 == rc ) {
                uint32_t idx;
                for ( idx = 0; 0 == rc && idx < count; ++idx ) {
                    const char * tbl_name;
                    rc = KNamelistGet( tables_1, idx, &tbl_name );
                    if ( 0 == rc ) {
                        if ( KNamelistContains( tables_2, tbl_name ) ) {
                            rc = copy_meta_for_one_table_in_db( self, src, node_path, tbl_name, src_node_has_to_exist );
                        }
                    }
                }
            }
            KNamelistRelease( tables_2 );
        }
        KNamelistRelease( tables_1 );
    }
    return rc;
}

static
bool
is_empty( const char * s ) {
    bool res = ( NULL == s );
    if ( !res ) { res = ( 0 == s[ 0 ] ); }
    return res;
}

LIB_EXPORT rc_t CC KDatabaseMetaCopy ( KDatabase *self, const KDatabase *src,
                                       const char * node_path, const char * tbl_name,
                                       bool src_node_has_to_exist ) {
    rc_t rc = 0;
    if ( NULL == self ) {
        rc = RC ( rcDB, rcDatabase, rcComparing, rcSelf, rcNull );
    } else if ( NULL == src ) {
        rc = RC ( rcDB, rcDatabase, rcComparing, rcParam, rcNull );
    } else {
        if ( is_empty( tbl_name ) ) {
            rc = copy_meta_for_all_tables_in_db( self, src, node_path, src_node_has_to_exist );
        } else {
            rc = copy_meta_for_one_table_in_db( self, src, node_path, tbl_name, true );
        }
    }
    return rc;
}

static
rc_t CC
KWDatabaseVOpenDBRead ( const KDatabase *self, const KDatabase **dbp, const char *name, va_list args )
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
        bool is_cached;
        rc = KDBWManagerVOpenDBReadInt_noargs ( self -> mgr, dbp,
                                        self -> dir, path, & is_cached, false );
        if ( rc == 0 && ! is_cached )
        {
            KDatabase *db = ( KDatabase* ) * dbp;
            db -> parent = KDatabaseAttach ( self );
        }
    }

    return rc;
}

static
rc_t CC
KWDatabaseVOpenTableRead ( const KDatabase *self, const KTable **tblp, const char *name, va_list args )
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
        rc = KDBWManagerVOpenTableReadInt_noargs ( self -> mgr, (const KWTable**)tblp,
                                           self -> dir, false, path, NULL );
        if ( rc == 0 )
        {
            KWTable *tbl = ( KWTable* ) * tblp;
            tbl -> db = KDatabaseAttach ( self );
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenTableUpdate ( KDatabase *self,
    KTable **tbl, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenTableUpdate ( self, tbl, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVOpenTableUpdate ( KDatabase *self,
    KTable **tblp, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( tblp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * tblp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "tbl", 3, name, args );
    if ( rc == 0 )
    {
        rc = KDBManagerVOpenTableUpdateInt_noargs ( self -> mgr, (KWTable **)tblp,
                                             self -> dir, path );
        if ( rc == 0 )
        {
            KWTable *tbl = ( KWTable* ) * tblp;
            tbl -> db = KDatabaseAttach ( self );
        }
    }

    return rc;
}

/* Default function is added only to make tools/kqsh build possible since
   it requires 1 to 1 mapping between K- and V-functions
*/

LIB_EXPORT rc_t CC KDatabaseCreateTableDefault ( struct KDatabase *self,
    KTable **tbl, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVCreateTableByMask ( self, tbl, 0, 0, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseCreateTable ( KDatabase *self,
    KTable **tbl, KCreateMode cmode, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVCreateTable ( self, tbl, cmode, name, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDatabaseVCreateTable ( KDatabase *self,
    KTable **tblp, KCreateMode cmode, const char *name, va_list args )
{
    return KDatabaseVCreateTableByMask ( self, tblp, cmode, (KCreateMode) -1, name, args);
}

LIB_EXPORT rc_t CC KDatabaseCreateTableByMask ( KDatabase *self,
    KTable **tbl, KCreateMode cmode, KCreateMode cmode_mask, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVCreateTableByMask ( self, tbl, cmode, cmode_mask, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVCreateTableByMask ( KDatabase *self,
    KTable **tblp, KCreateMode cmode, KCreateMode cmode_mask,
    const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    if ( tblp == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcParam, rcNull );

    * tblp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcCreating, rcDatabase, rcReadonly );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "tbl", 3, name, args );
    if ( rc == 0 )
    {
        /* ensure that the "tbl" namespace directory is there */
        rc = KDirectoryCreateDir ( self -> dir,
            0775, kcmOpen, "tbl" );
        if ( rc == 0 )
        {
            KCreateMode table_cmode;
            /* fix mask */
            if ( (cmode_mask & kcmValueMask) != 0 )
                cmode_mask |= kcmValueMask;
            table_cmode = (self->cmode & ~cmode_mask) | (cmode & cmode_mask);
            rc = KDBManagerVCreateTableInt_noargs ( self -> mgr, (KWTable **)tblp,
                                             self -> dir, table_cmode, path );
            if ( rc == 0 )
            {
                KWTable *tbl = ( KWTable* ) * tblp;
                tbl -> db = KDatabaseAttach ( self );
            }
        }
    }

    return rc;
}

static
rc_t CC
KWDatabaseOpenMetadataRead ( const KDatabase *self, const KMetadata **metap )
{
    rc_t rc;
    const KWMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBWManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false, &meta_is_cached );
    if ( rc == 0 )
    {
        if(!meta_is_cached) ((KWMetadata*)meta) -> db = KDatabaseAttach ( self );
        * metap = & meta -> dad;
    }

    return rc;
}

static
rc_t CC
KWDatabaseVOpenIndexRead ( const KDatabase *cself, const KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( cself == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    rc = KDBVMakeSubPath ( cself -> dir,
        path, sizeof path, "idx", 3, name, args );
    if ( rc == 0 )
    {
        KWIndex *idx;
        rc = KDBWManagerOpenIndexReadInt ( cself -> mgr, (const KWIndex **)& idx,
                                          cself -> dir, path );
        if ( rc == 0 )
        {
            KDatabase *self = ( KDatabase* ) cself;
            idx -> db = KDatabaseAttach ( self );
            * idxp = & idx -> dad;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseCreateIndex ( struct KDatabase *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVCreateIndex ( self, idx, type, cmode, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVCreateIndex ( KDatabase *self, KIndex **idxp,
    KIdxType type, KCreateMode cmode, const char *name, va_list args )
{
    rc_t rc = 0;
    KDirectory *dir = NULL;

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcCreating, rcDatabase, rcReadonly );

    rc = KDirectoryCreateDir_v1 ( self -> dir, 0777, kcmOpen, "idx" );
    if ( rc == 0 )
        rc = KDirectoryOpenDirUpdate_v1 ( self -> dir, & dir, false, "idx" );
    if ( rc == 0 )
    {
        char path [ 256 ];
        rc = KDirectoryVResolvePath ( dir, false, path, sizeof path, name, args );
        if ( rc == 0 )
        {
            KWIndex * idx;
            rc = KDBManagerCreateIndexInt ( self -> mgr, & idx, dir,
                type, cmode | kcmParents, path, (self -> cmode & kcmMD5) != 0 );
            if ( rc == 0 )
            {
                idx -> db = KDatabaseAttach ( self );
                * idxp = & idx -> dad;
            }
        }

        KDirectoryRelease ( dir );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenIndexUpdate ( struct KDatabase *self,
    KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenIndexUpdate ( self, idx, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVOpenIndexUpdate ( KDatabase *self,
    KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    KDirectory *dir;

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );

    rc = KDirectoryOpenDirUpdate_v1 ( self -> dir, & dir, false, "idx" );
    if ( rc == 0 )
    {
        char path [ 256 ];
        rc = KDirectoryVResolvePath ( dir, false, path, sizeof path, name, args );
        if ( rc == 0 )
        {
            KWIndex *idx;
            rc = KDBManagerOpenIndexUpdate ( self -> mgr, & idx, dir, path );
            if ( rc == 0 )
            {
                idx -> db = KDatabaseAttach ( self );
                * idxp = & idx -> dad;
            }
        }

        KDirectoryRelease ( dir );
    }
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenMetadataUpdate ( KDatabase *self, KMetadata **metap )
{
    rc_t rc;
    KWMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );

    rc = KDBManagerOpenMetadataUpdateInt ( self -> mgr, & meta, self -> dir, self -> md5 );
    if ( rc == 0 )
    {
        meta -> db = KDatabaseAttach ( self );
        * metap = & meta -> dad;
    }

    return rc;
}
