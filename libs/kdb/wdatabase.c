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
#include "database-priv.h"
#include "dbmgr-priv.h"
#include "wtable-priv.h"
#include "windex-priv.h"
#include "wkdb-priv.h"
#include <klib/namelist.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/impl.h>
#include <kfs/md5.h>
#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <vfs/path.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KDatabase
 *  connection to a database within file system
 */

/* GetPath
 *  return the absolute path to DB
 */
LIB_EXPORT rc_t CC KDatabaseGetPath ( struct KDatabase const *self,
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
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;
    assert ( mgr != NULL );

    KRefcountWhack ( & self -> refcount, "KDatabase" );

    /* release dad */
    if ( self -> dad != NULL )
    {
        rc = KDatabaseSever ( self -> dad );
        if ( rc != 0 )
            return rc;
        self -> dad = NULL;
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

    KRefcountInit ( & self -> refcount, 1, "KDatabase", "whack", "kdb" );

    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KDatabaseAddRef ( const KDatabase *cself )
{
    KDatabase *self = ( KDatabase* ) cself;
    if ( cself != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcDatabase, rcAttaching, rcRange, rcExcessive );
        }
        ++ self -> opencount;
    }
    return 0;
}

LIB_EXPORT rc_t CC KDatabaseRelease ( const KDatabase *cself )
{
    KDatabase *self = ( KDatabase* ) cself;
    if ( cself != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KDatabase" ) )
        {
        case krefWhack:
            return KDatabaseWhack ( ( KDatabase* ) self );
        case krefLimit:
            return RC ( rcDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
        -- self -> opencount;
    }
    return 0;
}


/* Attach
 */
KDatabase *KDatabaseAttach ( const KDatabase *cself )
{
    KDatabase *self = ( KDatabase* ) cself;
    if ( cself != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return self;
}

/* Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
rc_t KDatabaseSever ( const KDatabase *cself )
{
    KDatabase *self = ( KDatabase* ) cself;
    if ( cself != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KDatabase" ) )
        {
        case krefWhack:
            return KDatabaseWhack ( ( KDatabase* ) self );
        case krefLimit:
            return RC ( rcDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
static
rc_t KDatabaseMake ( KDatabase **dbp, const KDirectory *dir,
    const char *path, KMD5SumFmt *md5, bool read_only )
{
    KDatabase *db;
    rc_t rc = 0;

    assert ( dbp != NULL );
    assert ( path != NULL );

    db = malloc ( sizeof * db + strlen ( path ) );
    if ( db == NULL )
    {
        * dbp = NULL;
        return RC ( rcDB, rcDatabase, rcConstructing, rcMemory, rcExhausted );
    }

    memset ( db, 0, sizeof * db );
    db -> dir = ( KDirectory* ) dir;
    db -> md5 = md5;
    rc = KMD5SumFmtAddRef ( md5 );
    db -> cmode = kcmOpen; /* to be set later by the caller */
    db -> checksum = kcsNone; /* to be set later in VTableColumnCreateParams */

    if ( md5 != NULL )
        db -> cmode |= kcmMD5;

    KRefcountInit ( & db -> refcount, 1, "KDatabase", "make", path );
    db -> opencount = 1;
    db -> read_only = read_only;

    strcpy ( db -> path, path );

    db->sym.u.obj = db;
    StringInitCString (&db->sym.name, db->path);
    db->sym.type = kptDatabase;

    * dbp = db;
    return rc;
}

static
rc_t KDatabaseMakeVPath ( KDatabase **dbp, const KDirectory *dir,
    const VPath *path, KMD5SumFmt *md5, bool read_only )
{
    const String* dbpathStr;
    rc_t rc = VPathMakeString ( path, &dbpathStr );    /* NUL-terminated */
    if ( rc == 0 )
    {
        rc = KDatabaseMake ( dbp, dir, dbpathStr->addr, md5, read_only );
        StringWhack(dbpathStr);
    }
    return rc;
}

static
rc_t KDBManagerInsertDatabase ( KDBManager * self, KDatabase * db )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &db->sym);
    if ( rc == 0 )
        db -> mgr = KDBManagerAttach ( self );
    return rc;
}


static
rc_t KDBManagerMakeDBUpdate ( KDBManager *self,
    KDatabase **dbp, KDirectory *wd, const char *path, KMD5SumFmt *md5 )
{
    rc_t rc;
    KDirectory *dir;

    rc = KDirectoryOpenDirUpdate ( wd, & dir, false, "%s", path );
    if ( rc == 0 )
    {
        KDatabase *db;
	    
        /* Make the base object */
        rc = KDatabaseMake ( & db, dir, path, md5, false );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertDatabase ( self, db );

            if ( rc == 0 )
            {
                * dbp = db;
                return 0;   /* successful exit */
            }

            rc = RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcBusy );
                
            free ( db );
        }
        KDirectoryRelease ( dir );
    }
    return rc;
}


/* CreateDB
 * VCreateDB
 *  create a new or open an existing database
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KDBManagerVCreateDBInt ( KDBManager *self,
    KDatabase **db, KDirectory *wd, KCreateMode cmode,
    const char *path, va_list args )
{
    char dbpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
    {
        /* we won't try accession resolution here */
        int type = KDBPathType ( /*NULL,*/ wd, NULL, dbpath );
        switch ( type )
        {
        case kptNotFound:
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );

        case kptDatabase:
        case kptDatabase | kptAlias:
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ((cmode & kcmValueMask) == kcmCreate)
                return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcExists );
            if (KDBManagerOpenObjectBusy (self, dbpath))
                return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcBusy );
            /* test now for locked directory */
            rc = KDBWritable ( wd, dbpath );
            if (rc)
            {
                switch (GetRCState(rc))
                {
                default:
                    return rc;
                case rcLocked:
                    return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcLocked );
                case rcReadonly:
                    return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcReadonly );
                case rcNotFound:
                    /* not found is good but probably unreachable */
                    break;
                case 0:
                    rc = 0;
                    break;
                }
            }
            /* second good path */
            break;

        case kptTable:
        case kptTable | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcTable, rcExists);

        case kptColumn:
        case kptColumn | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcColumn, rcExists);

        case kptIndex:
        case kptIndex | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcIndex, rcExists);

        case kptMetadata:
        case kptMetadata | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcMetadata, rcExists);

	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a database
	     * or a non related file */
	    if ( KDBOpenPathTypeRead ( self, wd, dbpath, NULL, kptDatabase, NULL, false,
            NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcCreating, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        /* [re]create directory */
        rc = KDirectoryCreateDir ( wd, 0775, cmode, "%s", dbpath );
        if (rc == 0)
            /* create tbl subdirectory as required for db */
            rc = KDirectoryCreateDir ( wd, 0775, kcmOpen, "%s/tbl", dbpath );

        if ( rc == 0 )
        {
            KMD5SumFmt *md5 = NULL;

            if ( ( cmode & kcmMD5 ) != 0 )
            {
                KFile * f;

                /* if needed create the md5 digest file */
                rc = KDirectoryCreateFile ( wd, &f, true, 0664, kcmOpen, "%s/md5", dbpath );
                if ( rc == 0 )
                {
                    /* create a formatter around file
                       formatter will own "f" afterward */
                    rc = KMD5SumFmtMakeUpdate ( & md5, f );

                    /* if failed to create formatter, release "f" */
                    if ( rc != 0 )
                        KFileRelease ( f );
                }
            }

            if ( rc == 0 )
                rc = KDBManagerMakeDBUpdate ( self, db, wd, dbpath, md5 );
            /* TODO: for now set cmode here, externally to KDatabaseMake
                maybe it's needed to refactor the whole chain from exported
                Open/Create DB functions to the KDatabaseMake
            */
            if ( rc == 0 )
                KDatabaseSetCmode ( *db, cmode );

            KMD5SumFmtRelease ( md5 );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVCreateDB ( KDBManager *self,
    KDatabase **db, KCreateMode cmode, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );

    return KDBManagerVCreateDBInt ( self, db, self -> wd, cmode, path, args );
}


LIB_EXPORT rc_t CC KDBManagerCreateDB ( KDBManager *self,
    KDatabase **db, KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateDB ( self, db, cmode, path, args );
    va_end ( args );

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
            rc = KDBManagerVCreateDBInt ( self -> mgr, dbp,
                                          self -> dir, cmode, path, NULL );
            if ( rc == 0 )
            {
                KDatabase *db = ( KDatabase* ) * dbp;
                db -> dad = self;
                atomic32_inc ( & self -> refcount );
            }
        }
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
rc_t KDBManagerVOpenDBReadInt ( const KDBManager *cself,
    const KDatabase **dbp, KDirectory *wd,
    const char *path, va_list args, bool *cached, bool try_srapath )
{
    char key_path[ 4096 ];
	char short_path[ 4096 ];
	size_t z;
	rc_t rc = string_vprintf( short_path, sizeof short_path, &z, path, args );
	if ( rc == 0 )
		rc = KDirectoryResolvePath ( wd, true, key_path, sizeof key_path, short_path );
    if ( rc == 0 )
    {
        KSymbol *sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind( cself, key_path );
        if ( sym != NULL )
        {
            const KDatabase *cdb;
            rc_t obj;

            if ( cached != NULL )
                * cached = true;

            switch (sym->type)
            {
            case kptDatabase:
                cdb = (KDatabase*)sym->u.obj;
                /* if open for update, refuse */
                if ( cdb -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KDatabaseAddRef ( cdb );
                    if ( rc == 0 )
                        * dbp = cdb;
                    return rc;
                }
                obj = rcDatabase;
                break;

            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            rc = RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
        }
        else
        {
			const KDirectory *dir;

			if ( cached != NULL )
				* cached = false;

			/* open the directory if its a database */
			rc = KDBOpenPathTypeRead ( cself, wd, short_path, &dir, kptDatabase, NULL,
                try_srapath, NULL );
			if ( rc == 0 )
			{
				KDatabase *db;

				rc = KDatabaseMake ( &db, dir, key_path, NULL, true );
				if ( rc == 0 )
				{
					KDBManager *self = ( KDBManager* ) cself;

					rc = KDBManagerInsertDatabase ( self, db );
					if ( rc == 0 )
					{
						* dbp = db;
						return 0;
					}
					free (db);
				}
				KDirectoryRelease (dir);
			}
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

LIB_EXPORT rc_t CC KDBManagerVOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenDBReadInt ( self, db, self -> wd, path, args, NULL, true );
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
        bool is_cached;
        rc = KDBManagerVOpenDBReadInt ( self -> mgr, dbp,
                                        self -> dir, path, NULL, & is_cached, false );
        if ( rc == 0 && ! is_cached )
        {
            KDatabase *db = ( KDatabase* ) * dbp;
            db -> dad = KDatabaseAttach ( self );
        }
    }

    return rc;
}


/* OpenDBUpdate
 * VOpenDBUpdate
 *  open a database for read/write
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KDBManagerVOpenDBUpdateInt ( KDBManager *self,
    KDatabase **db, KDirectory *wd, const char *path, va_list args )
{
    char dbpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KFile *f;
        KMD5SumFmt * md5 = NULL;

        /* if already open, refuse */
        sym = KDBManagerOpenObjectFind (self, dbpath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }
        /* only open existing dbs */
        switch (KDBPathType ( /*self,*/ wd, NULL, dbpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a database
	     * or a non related file */
	    if ( KDBOpenPathTypeRead ( self, wd, dbpath, NULL, kptDatabase, NULL, false,
            NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcOpening, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        case kptDatabase:
        case kptDatabase | kptAlias:
            break;
        }

        /* test now for locked directory */
        rc = KDBWritable (wd, dbpath);
        switch (GetRCState (rc))
        {
        default:
#if 0
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcNoPerm ); /* TBD: better state? */
#endif

        case rcLocked:
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcLocked );

        case rcReadonly:
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcReadonly );


        case 0:
            rc = 0;
            break;
        }

        rc = KDirectoryOpenFileWrite ( wd, &f, true, "%s/md5", dbpath );
        if ( rc == 0 )
        {
            rc = KMD5SumFmtMakeUpdate ( & md5, f );
            if ( rc != 0 )
                KFileRelease ( f );
        }
        else if ( GetRCState ( rc ) == rcNotFound )
            rc = 0;

        if ( rc == 0 )
            rc = KDBManagerMakeDBUpdate ( self, db, wd, dbpath, md5 );

        KMD5SumFmtRelease ( md5 );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenDBUpdate ( KDBManager *self,
    KDatabase **db, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBUpdate ( self, db, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenDBUpdate ( KDBManager *self,
    KDatabase **db, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenDBUpdateInt ( self, db, self -> wd, path, args );
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
        rc = KDBManagerVOpenDBUpdateInt ( self -> mgr, dbp,
                                         self -> dir, path, NULL );
        if ( rc == 0 )
        {
            KDatabase *db = ( KDatabase* ) * dbp;
            db -> dad = self;
            atomic32_inc ( & self -> refcount );
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
 *  valid values are kptTable
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT bool CC KDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    if ( self != NULL && name != NULL && name [ 0 ] != 0 )
    {
        rc_t rc;
        size_t len;
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
        size_t len;
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
rc_t KDatabaseLockInt (const KDatabase  * self, char * path, size_t path_size,
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
            rc = KDBVMakeSubPath (self->dir, path, sizeof path, ns, strlen (ns),
                                  name, args);
            break;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    rc = KDatabaseLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBWritable (self->dir, path);
    return rc;
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

LIB_EXPORT rc_t CC KDatabaseOpenParentUpdate ( KDatabase *self, KDatabase **par )
{
    rc_t rc;

    if ( par == NULL )
        rc = RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        else if ( self -> dad != NULL && self -> dad -> read_only )
            rc = RC ( rcDB, rcDatabase, rcAccessing, rcDatabase, rcReadonly );
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


/* OpenDirectory
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
        if ( self != NULL )
            return KDBGetObjModDate ( self -> dir, mtime );

        rc = RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
        * mtime = 0;
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
bool CC KDatabaseListFilter ( const KDirectory *dir, const char *name, void *data_ )
{
    struct FilterData * data = data_;
    return ( KDBOpenPathTypeRead ( data->mgr, dir, name, NULL, data->type, NULL, false,
        NULL ) == 0 );
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
            {
                KDatabase *db;

                rc = KDatabaseMakeVPath ( & db, dir, vpath, NULL, true );
                if ( rc == 0 )
                {
                    rc = KDBManagerInsertDatabase ( ( KDBManager* ) self, db );
                    if ( rc == 0 )
                    {
                        * p_db = db;
                        return 0;
                    }
                    free (db);
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
            {
                KDatabase *db;

                /* allocate a new guy */
                rc = KDatabaseMakeVPath ( & db, dir, remote, NULL, true );
                if ( rc == 0 )
                {
                    rc = KDBManagerInsertDatabase ( ( KDBManager* ) self, db );
                    if ( rc == 0 )
                    {
                        * p_db = db;
                        return 0;
                    }
                    free (db);
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
}

KCreateMode KDatabaseGetCmode ( const KDatabase *self)
{
    return self -> cmode;
}
KCreateMode KDatabaseSetCmode ( KDatabase *self, KCreateMode new_val)
{
    KCreateMode old_val = self -> cmode;
    self -> cmode = new_val;
    return old_val;
}

KChecksum KDatabaseGetChecksum ( const KDatabase *self)
{
    return self -> checksum;
}
KChecksum KDatabaseSetChecksum ( KDatabase *self, KChecksum new_val)
{
    KCreateMode old_val = self -> checksum;
    self -> checksum = new_val;
    return old_val;
}
