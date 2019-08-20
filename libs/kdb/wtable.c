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
#include "wtable-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "wcolumn-priv.h"
#include "windex-priv.h"
#include "wkdb-priv.h"

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


/* keep file system structure compatible with v0 code */
#define V0_BACKWARD_COMPAT 0

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
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;
    assert ( mgr != NULL );

    KRefcountWhack ( & self -> refcount, "KTable" );

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc != 0 )
            return rc;
        self -> db = NULL;
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

    KRefcountInit ( & self -> refcount, 1, "KTable", "whack", "ktbl" );

    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KTableAddRef ( const KTable *cself )
{
    KTable *self = ( KTable* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KTable" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcTable, rcAttaching, rcRange, rcExcessive );
        }
        ++ self -> opencount;
    }
    return 0;
}

LIB_EXPORT rc_t CC KTableRelease ( const KTable *cself )
{
    KTable *self = ( KTable* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KTable" ) )
        {
        case krefWhack:
            return KTableWhack ( ( KTable* ) self );
        case krefLimit:
            return RC ( rcDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
        -- self -> opencount;
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
            assert ( self -> opencount == 0 );
            return KTableWhack ( ( KTable* ) self );
        case krefLimit:
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
    tbl -> dir = ( KDirectory* ) dir;
    KRefcountInit ( & tbl -> refcount, 1, "KTable", "make", path );
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

static
rc_t KDBManagerInsertTable ( KDBManager * self, KTable * tbl )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &tbl->sym);
    if ( rc == 0 )
        tbl -> mgr = KDBManagerAttach ( self );
    return rc;
}


static
rc_t KDBManagerMakeTableUpdate ( KDBManager *self,
    KTable **tblp, KDirectory *wd, const char *path, bool convert, KMD5SumFmt * md5 )
{
    rc_t rc;
    KDirectory *dir;

    rc = KDirectoryOpenDirUpdate ( wd, & dir, false, "%s", path );
    if ( rc == 0 )
    {
        KTable * tbl;

        if ( convert )
        {
            /* the old table layout:
               <tbl>/
               col/...
               meta
               sealed
               skey
            */
            rc = KDirectoryVSetAccess ( dir, true, 0220, 0222, ".", NULL );
            if ( rc == 0 )
                rc = KDirectoryVCreateDir ( dir, 0775, kcmCreate, "md", NULL );
            if ( rc == 0 )
            {
                rc = KDirectoryRename ( dir, false, "meta", "md/r001" );
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;
                else if ( rc == 0 )
                {
                    rc = KDirectoryCreateAlias ( dir, 0777, kcmCreate, "md/r001", "md/cur" );
#if V0_BACKWARD_COMPAT
                    if ( rc == 0 )
                        rc = KDirectoryCreateAlias ( dir, 0777, kcmCreate, "md/r001", "meta" );
#endif
                    if( rc == 0 ) {
                        KFile* md5_file = NULL;
                        if( (rc = KDirectoryOpenFileWrite(dir, &md5_file, true, "md5")) == 0 ) {
                            KMD5SumFmt* fmd5 = NULL;
                            if( (rc = KMD5SumFmtMakeUpdate(&fmd5, md5_file)) == 0 ) {
                                if( (rc = KMD5SumFmtRename(fmd5, "meta", "md/r001")) == 0 ) {
                                    rc = KMD5SumFmtFlush(fmd5);
                                }
                                KMD5SumFmtRelease(fmd5);
                            } else {
                                KFileRelease(md5_file);
                            }
                        }
                    }
                }
            }
            if ( rc == 0 )
                rc = KDirectoryVCreateDir ( dir, 0775, kcmCreate, "idx", NULL );
            if ( rc == 0 )
            {
                rc = KDirectoryRename ( dir, false, "skey", "idx/skey" );
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;
                else if ( rc == 0 )
                {
                    KDirectoryRename ( dir, false, "skey.md5", "idx/skey.md5" );
#if V0_BACKWARD_COMPAT
                    rc = KDirectoryCreateAlias ( dir, 0777, kcmCreate, "idx/skey", "skey" );
#endif
                }
            }
            if ( rc == 0 )
            {
                if ( KDirectoryVPathType ( dir, "sealed", NULL ) == kptFile )
                {
                    rc = KDBLockDir ( dir, "." );
#if ! V0_BACKWARD_COMPAT
                    if ( rc == 0 )
                        KDirectoryVRemove ( dir, true, "sealed", NULL );
#endif
                }
            }
            
            if ( rc != 0 )
                return rc;
        }

        /* Make the base object */
        rc = KTableMake ( & tbl, dir, path, md5, false );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertTable ( self, tbl );

            if ( rc == 0 )
            {
                * tblp = tbl;
                return 0;
            }

            rc = RC ( rcDB, rcMgr, rcOpening, rcTable, rcBusy );
                
            free ( tbl );
        }
            
        KDirectoryRelease ( dir );
    }

    return rc;
}


/* CreateTable
 * VCreateTable
 *  create a new or open an existing table
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
static
rc_t KDBManagerVCreateTableInt ( KDBManager *self,
    KTable **tbl, KDirectory *wd, KCreateMode cmode, const char *path, va_list args )
{
    char tblpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        tblpath, sizeof tblpath, path, args );
    if ( rc == 0 )
    {
        bool convert = false;
        bool not_found = false;

        switch ( KDBPathType ( /*NULL,*/ wd, NULL, tblpath ) )
        {
        case kptNotFound:
            not_found = true;
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );

        case kptTable:
        case kptTable | kptAlias:
            break;

        case kptPrereleaseTbl:
        case kptPrereleaseTbl | kptAlias:
            convert = true;
            break;

        case kptDatabase:
        case kptDatabase | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcDatabase, rcExists);

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
            /* if we find a file, vary the failure if it is an archive that is a table
             * or a non related file */
            if (( KDBOpenPathTypeRead ( self, wd, tblpath, NULL, kptTable, NULL,
                                        false, NULL ) == 0 ) ||
                ( KDBOpenPathTypeRead ( self, wd, tblpath, NULL, kptPrereleaseTbl, NULL,
                                        false, NULL ) == 0 ) )
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcReadonly );
            /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        if (not_found == false)
        {
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ((cmode & kcmValueMask) == kcmCreate)
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcExists );
            if (KDBManagerOpenObjectBusy (self, tblpath))
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcBusy );
            /* test now for locked directory */
            rc = KDBWritable ( wd, tblpath );
            switch (GetRCState(rc))
            {
            default:
                return rc;
            case rcLocked:
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcLocked );
            case rcReadonly:
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcReadonly );
            case rcNotFound:
                /* not found is good but probably unreachable */
                break;
            case 0:
                break;
            }
        }

        /* [re]create directory */
        rc = KDirectoryCreateDir ( wd, 0775, cmode, "%s", tblpath );
        if ( rc == 0 )
        {
            KMD5SumFmt *md5 = NULL;

            /* create column sub-directory */
            rc = KDirectoryCreateDir ( wd, 0775, kcmOpen, "%s/col", tblpath );
            if ( rc == 0 && ( cmode & kcmMD5 ) != 0 )
            {
                KFile * f;

                /* create the md5 digest file */
                rc = KDirectoryCreateFile ( wd, &f, true, 0664, kcmOpen, "%s/md5", tblpath );
                if ( rc == 0 )
                {
                    rc = KMD5SumFmtMakeUpdate ( & md5, f );
                    if ( rc != 0 )
                        KFileRelease ( f );
                }
            }
            
            if ( rc == 0 )
                rc = KDBManagerMakeTableUpdate ( self, tbl, wd, tblpath, convert, md5 );

            KMD5SumFmtRelease ( md5 );
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

LIB_EXPORT rc_t CC KDBManagerCreateTable ( KDBManager *self,
    KTable **tbl, KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateTable ( self, tbl, cmode, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVCreateTable ( KDBManager *self,
    KTable **tbl, KCreateMode cmode, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );

    return KDBManagerVCreateTableInt ( self, tbl, self -> wd, cmode, path, args );
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
    return KDatabaseVCreateTableByMask ( self, tblp, cmode, -1, name, args );
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
            rc = KDBManagerVCreateTableInt ( self -> mgr, tblp,
                                             self -> dir, table_cmode, path, NULL );
            if ( rc == 0 )
            {
                KTable *tbl = ( KTable* ) * tblp;
                tbl -> db = KDatabaseAttach ( self );
            }
        }
    }

    return rc;
}

#include <vfs/path.h> /* VPathMakeString */
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
rc_t KDBManagerVOpenTableReadInt ( const KDBManager *cself,
    const KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, va_list args, const struct VPath *vpath )
{
    const String * str = NULL;
    char key_path[ 4096 ];
    char short_path[ 4096 ];
    size_t z;
    rc_t rc = 0;

    if (vpath != NULL) {
        assert(path == NULL);
        rc = VPathMakeString(vpath, &str);
        if (rc == 0)
            path = str->addr;
    }

    rc = string_vprintf(short_path, sizeof short_path, &z, path, args);

    if (str != NULL) {
        StringWhack(str);
        str = NULL;
        path = NULL;
    }

    if ( rc == 0 )
		rc = KDirectoryResolvePath ( wd, true, key_path, sizeof key_path, short_path );
    if ( rc == 0 )
    {
        KSymbol *sym;
        
        /* if already open */
        sym = KDBManagerOpenObjectFind( cself, key_path );
        if (sym != NULL)
        {
            const KTable * ctbl;
            rc_t obj;

            switch (sym->type)
            {
            case kptTable:
                ctbl = (KTable*)sym->u.obj;
                /* if open for update, refuse */
                if ( ctbl -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KTableAddRef ( ctbl );
                    if ( rc == 0 )
                        * tblp = ctbl;
                    return rc;
                }
                obj = rcTable;
                break;
                
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
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
			KTable * tbl;
			const KDirectory *dir;
			bool prerelease = false;

			rc = KDBOpenPathTypeRead ( cself, wd, short_path, &dir, kptTable, NULL, try_srapath, vpath );
			if ( rc != 0 )
			{
				prerelease = true;
				rc = KDBOpenPathTypeRead ( cself, wd, short_path, &dir, kptPrereleaseTbl, NULL, try_srapath, vpath );
			}        
			if ( rc == 0 )
			{
				rc = KTableMake ( & tbl, dir, key_path, NULL, true );
				if ( rc == 0 )
				{
					KDBManager * self = (KDBManager *)cself;

					rc = KDBManagerInsertTable( self, tbl );

					if (rc == 0)
					{
						tbl -> prerelease = prerelease;
						* tblp = tbl;
						return 0;
					}
					free ( tbl );
				}
				KDirectoryRelease ( dir );
			}
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

    return KDBManagerVOpenTableReadInt ( self, tbl, self -> wd, true, path, args, NULL );
}

LIB_EXPORT rc_t CC KDBManagerOpenTableReadVPath(const KDBManager *self,
    const KTable **tbl, const struct VPath *path)
{
    if (tbl == NULL)
        return RC(rcDB, rcMgr, rcOpening, rcParam, rcNull);

    *tbl = NULL;

    if (self == NULL)
        return RC(rcDB, rcMgr, rcOpening, rcSelf, rcNull);

    return KDBManagerVOpenTableReadInt(self, tbl, self->wd, true, NULL, NULL, path);
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


/* OpenTableUpdate
 * VOpenTableUpdate
 *  open a table for read/write
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving name of table
 */
static
rc_t KDBManagerVOpenTableUpdateInt ( KDBManager *self,
    KTable **tbl, KDirectory *wd, const char *path, va_list args )
{
    char tblpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        tblpath, sizeof tblpath, path, args );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KFile *f;
        KMD5SumFmt * md5 = NULL;
        bool convert = false;

        /* if already open, refuse */
        sym = KDBManagerOpenObjectFind (self, tblpath);
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
        /* only open existing tbls */
        switch ( KDBPathType ( /*NULL,*/ wd, NULL, tblpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcNotFound );

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );

        case kptFile:
        case kptFile | kptAlias:
            /* if we find a file, vary the failure if it is an archive that is a table
             * or a non related file */
            if (( KDBOpenPathTypeRead ( self, wd, tblpath, NULL, kptTable, NULL,
                                        false, NULL ) == 0 ) ||
                ( KDBOpenPathTypeRead ( self, wd, tblpath, NULL, kptPrereleaseTbl, NULL,
                                        false, NULL ) == 0 ) )
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcUnauthorized );
            /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        case kptTable:
        case kptTable | kptAlias:
            break;

        case kptPrereleaseTbl:
        case kptPrereleaseTbl | kptAlias:
            convert = true;
            break;
        }

        /* test now for locked directory */
        rc = KDBWritable ( wd, tblpath );
        switch (GetRCState(rc))
        {
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcNoPerm );

        case rcLocked:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcLocked );

        case rcReadonly:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcReadonly );

        case 0:
            rc = 0;
            break;
        }

        rc = KDirectoryOpenFileWrite ( wd, &f, true, "%s/md5", tblpath );
        if ( rc == 0 )
        {
            rc = KMD5SumFmtMakeUpdate ( & md5, f );
            if ( rc != 0 )
                KFileRelease ( f );
        }
        else if ( GetRCState ( rc ) == rcNotFound )
            rc = 0;
#if 0
        else if ( GetRCState ( rc ) == rcUnauthorized )
            return RC ( rcDB, rcMgr, rcCreating, rcTable, rUnauthorized );
#endif

        if ( rc == 0 )
            rc = KDBManagerMakeTableUpdate ( self, tbl, wd, tblpath, convert, md5 );

        KMD5SumFmtRelease ( md5 );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenTableUpdate ( KDBManager *self,
    KTable **tbl, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenTableUpdate ( self, tbl, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenTableUpdate ( KDBManager *self,
    KTable **tbl, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenTableUpdateInt ( self, tbl, self -> wd, path, args );
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
        rc = KDBManagerVOpenTableUpdateInt ( self -> mgr, tblp,
                                             self -> dir, path, NULL );
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

LIB_EXPORT rc_t CC KTableVWritable (const KTable *self, uint32_t type, const char * name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    rc = KTableLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBWritable (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KTableWritable ( const KTable *self, uint32_t type, const char * name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KTableVWritable ( self, type, name, args );
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
KDB_EXTERN bool CC KTableHasRemoteData ( const KTable *self )
{
    return true;
}
