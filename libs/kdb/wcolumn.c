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

#include <klib/rc.h>

#include "wcolumn-priv.h"
#include "dbmgr-priv.h"
#include "wtable-priv.h"
#include "wkdb-priv.h"
#include "werror-priv.h"
#include <kfs/impl.h>
#include <klib/data-buffer.h>

#include <stdio.h>
#include <byteswap.h>

#define KCOLUMNBLOB_IMPL KColumnBlob
#include "columnblob-base.h"

/*--------------------------------------------------------------------------
 * KWColumn (formerly KColumn)
 *  a read-write collection of blobs indexed by oid; file system-based
 */

static rc_t CC KWColumnWhack ( KColumn *self );
static rc_t CC KWColumnAddRef ( const KColumn *cself );
static rc_t CC KWColumnRelease ( const KColumn *cself );
static bool CC KWColumnLocked ( const KColumn *self );
static rc_t CC KWColumnVersion ( const KColumn *self, uint32_t *version );
static rc_t CC KWColumnByteOrder ( const KColumn *self, bool *reversed );
static rc_t CC KWColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count );
static rc_t CC KWColumnFindFirstRowId ( const KColumn * self, int64_t * found, int64_t start );
static rc_t CC KWColumnOpenManagerRead ( const KColumn *self, const KDBManager **mgr );
static rc_t CC KWColumnOpenParentRead ( const KColumn *self, const KTable **tbl );

static KColumnBase_vt KColumn_vt =
{
    /* Public API */
    KWColumnWhack,
    KWColumnAddRef,
    KWColumnRelease,
    KWColumnLocked,
    KWColumnVersion,
    KWColumnByteOrder,
    KWColumnIdRange,
    KWColumnFindFirstRowId,
    KWColumnOpenManagerRead,
    KWColumnOpenParentRead
};

/* Whack
 */
static
rc_t KWColumnWhack ( KColumn *self )
{
    rc_t rc;
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;
    assert ( mgr != NULL );

    KRefcountWhack ( & self -> dad . refcount, "KColumn" );

    /* shut down and checkpoint index */
    rc = KColumnIdxWhack ( & self -> idx,
        self -> df . eof, self -> df . pgsize, self -> checksum );
    if ( rc )
        return rc;

    /* shut down data fork */
    KColumnDataWhack ( & self -> df );

    /* shut down md5 sum file if it is open */
    KMD5SumFmtRelease ( self -> md5 ), self -> md5 = NULL;

    /* release owning table
       should never fail, and our recovery is flawed */
    if ( self -> tbl != NULL )
    {
        rc = KTableSever ( self -> tbl );
        if ( rc != 0 )
            return rc;
        self -> tbl = NULL;
    }

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
                return KColumnBaseWhack( self );
            }
        }
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KColumn", "whack", "kcol" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
static rc_t CC KWColumnAddRef ( const KColumn *cself )
{
    KColumn *self = ( KColumn* ) cself;
    switch ( KRefcountAdd ( & self -> dad . refcount, "KColumn" ) )
    {
    case krefLimit:
        return RC ( rcDB, rcColumn, rcAttaching, rcRange, rcExcessive );
    }
    ++ self -> opencount;
    return 0;
}

static rc_t CC KWColumnRelease ( const KColumn *cself )
{
    KColumn *self = ( KColumn* ) cself;
    switch ( KRefcountDrop ( & self -> dad . refcount, "KColumn" ) )
    {
    case krefWhack:
        return KWColumnWhack ( ( KColumn* ) self );
    case krefLimit:
        return RC ( rcDB, rcColumn, rcReleasing, rcRange, rcExcessive );
    }
    -- self -> opencount;
    return 0;
}


/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
rc_t KColumnMake ( KColumn **colp, const KDirectory *dir, const char *path,
		   KMD5SumFmt * md5, bool read_only )
{
    rc_t rc;
    KColumn *col = malloc ( sizeof * col + strlen ( path ) );
    if ( col == NULL )
    {
	* colp = NULL;
        return RC ( rcDB, rcColumn, rcConstructing, rcMemory, rcExhausted );
    }

    memset ( col, 0, sizeof * col );

    col -> dad . vt = & KColumn_vt;

    col -> dir = ( KDirectory* ) dir;
    col -> md5 = md5;
    rc = KMD5SumFmtAddRef ( md5 );
    KRefcountInit ( & col -> dad . refcount, 1, "KColumn", "make", path );
    col -> opencount = 1;
    col -> commit_freq = 1;
    col -> read_only = read_only;

    strcpy ( col -> path, path );

    col->sym.u.obj = col;
    StringInitCString (&col->sym.name, col->path);
    col->sym.type = kptColumn;

    * colp = col;
    return rc;
}


static
rc_t KDBManagerInsertColumn ( KDBManager * self, KColumn * col )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &col->sym);
    if ( rc == 0 )
        col -> mgr = KDBManagerAttach ( self );
    return rc;
}


static
rc_t KColumnMakeRead ( KColumn **colp, const KDirectory *dir, const char *path,
		       KMD5SumFmt * md5 )
{
    rc_t rc = KColumnMake ( colp, dir, path, md5, true );
    if ( rc == 0 )
    {
        size_t pgsize;
        uint64_t data_eof;
        KColumn *self = * colp;

        rc = KColumnIdxOpenRead ( & self -> idx,
            dir, & data_eof, & pgsize, & self -> checksum );
        if ( rc == 0 )
        {
            rc = KColumnDataOpenRead ( & self -> df,
				       dir, data_eof, pgsize );
            if ( rc == 0 )
            {
                switch ( self -> checksum )
                {
                case kcsNone:
                    break;
                case kcsCRC32:
                    self -> csbytes = 4;
                    break;
                case kcsMD5:
                    self -> csbytes = 16;
                    break;
                }

                self -> commit_freq = 0;
                return 0;
            }

            KColumnIdxWhack ( & self -> idx,
                data_eof, pgsize, self -> checksum );
        }

        free ( self );
    }

    * colp = NULL;
    return rc;
}

static
rc_t KColumnMakeUpdate ( KColumn **colp,
    KDirectory *dir, const char *path, KMD5SumFmt *md5 )
{
    rc_t rc = KColumnMake ( colp, dir, path, md5, false );
    if ( rc == 0 )
    {
        size_t pgsize;
        uint64_t data_eof;
        KColumn *self = * colp;

        rc = KColumnIdxOpenUpdate ( & self -> idx, dir,
            md5, & data_eof, & pgsize, & self -> checksum );
        if ( rc == 0 )
        {
            rc = KColumnDataOpenUpdate ( & self -> df, dir,
                md5, data_eof, pgsize );
            if ( rc == 0 )
            {
                switch ( self -> checksum )
                {
                case kcsNone:
                    break;
                case kcsCRC32:
                    self -> csbytes = 4;
                    break;
                case kcsMD5:
                    self -> csbytes = 16;
                    break;
                }

                return 0;
            }

            /* why is this here? */
            KColumnDataWhack ( & self -> df );

            KColumnIdxWhack ( & self -> idx,
                data_eof, pgsize, self -> checksum );
        }

        free ( self );
    }

    * colp = NULL;
    return rc;
}


/* Create
 * VCreate
 *  create a new or open an existing column
 *
 *  "colp" [ OUT ] - return parameter for newly opened database
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KColumnCreate ( KColumn **colp, KDirectory *dir,
    KCreateMode cmode, KChecksum checksum,
	size_t pgsize, const char *path, KMD5SumFmt *md5 )
{
    rc_t rc;

    /* only supporting pgsize of 1 */
    if ( pgsize == 0 )
        pgsize = 1;
    else if ( pgsize != 1 )
        return RC ( rcDB, rcColumn, rcConstructing, rcParam, rcInvalid );

    rc = KColumnMake ( colp, dir, path, md5, false );

    if ( rc == 0 )
    {
        uint64_t data_eof;
        KColumn *self = * colp;

        self -> checksum = ( int32_t ) checksum;

        rc = KColumnIdxCreate ( & self -> idx,
            dir, md5, cmode, & data_eof, pgsize, ( int32_t ) checksum );
        if ( rc == 0 )
        {
            rc = KColumnDataCreate ( & self -> df,
                dir, md5, cmode, data_eof, pgsize );
            if ( rc == 0 )
            {
                switch ( self -> checksum )
                {
                case kcsNone:
                    break;
                case kcsCRC32:
                    self -> csbytes = 4;
                    break;
                case kcsMD5:
                    self -> csbytes = 16;
                    break;
                }

                /* successful exit */
                return 0;
            }

            /* close data ? redundant? my thoughts exactly */
            KColumnDataWhack ( & self -> df );

            KColumnIdxWhack ( & self -> idx,
                data_eof, pgsize, self -> checksum );
        }

        free ( self );
    }

    * colp = NULL;
    return rc;
}

rc_t KColumnFileCreate ( KFile **ppf,
    KMD5File **ppfmd5, KDirectory *dir, KMD5SumFmt *md5,
    KCreateMode mode, bool append, const char *name )
{
#if 1
    rc_t rc;

    KFile *pf = NULL;
    * ppfmd5 = NULL;

    rc = KDirectoryCreateFile ( dir, & pf, true, 0664, mode, "%s", name );
    if ( rc == 0 && md5 != NULL )
    {
        /* if the file was opened in "open" mode, leave its entry alone
           if the file was opened in "init" mode, delete any existing entry
           if the file was opened in "create" mode, it will fail if there
           was an existing file, but still delete any digest entry */
        if ( ( mode & kcmValueMask ) != kcmOpen )
            rc = KMD5SumFmtDelete ( md5, name );
        if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
        {
            KMD5File *md5file;
            rc = ( append ? KMD5FileMakeAppend : KMD5FileMakeWrite ) ( & md5file, pf, md5, name );
            if ( rc == 0 )
            {
                * ppfmd5 = md5file;
                pf = KMD5FileToKFile ( md5file );
            }
        }

        if ( rc != 0 )
        {
            KFileRelease ( pf );
            pf = NULL;
        }
    }

    * ppf = pf;

    return rc;
#else

    /* this looks like it needs some rethinking */
    rc_t rc = 0;
    KFile * pf;

    *ppfmd5 = NULL;
    *ppf = NULL;
    /* -----
     * This is used even when opening for update on some files
     * so we have extra work
     *
     * If mode is kcmOpen try to open with normal open functions
     * so the MD5 part gets handled correctly
     */
    rc = KColumnFileOpenUpdate (ppf, ppfmd5, dir, md5, append, name);
    if (rc == 0)
        return 0;

    else if (GetRCState (rc) == rcNotFound)
        rc = 0; /* this is not a true failure here so go on to create it */
    if (rc == 0)
    {
        rc = KDirectoryVCreateFile (dir, &pf, true, 0664, mode, "%s", name);
        if ((rc == 0) && (md5 != NULL))
        {
            rc = KMD5SumFmtDelete (md5, name);
            if ((rc == 0) || (GetRCState (rc) == rcNotFound))
            {
                KMD5File * md5file;

                rc = (append ? KMD5FileMakeAppend : KMD5FileMakeWrite)
                    (&md5file, pf, md5, name);
                if (rc == 0)
                {
                    *ppfmd5 = md5file;
                    pf = KMD5FileToKFile (md5file);
                }
                else
                {
                    KFileRelease (pf);
                    pf = NULL;
                }
            }
        }
    }
    *ppf = pf;
    return rc;
#endif
}

rc_t KColumnFileOpenUpdate ( KFile **ppf, KMD5File **ppfmd5,
    KDirectory *dir, KMD5SumFmt *md5, bool append, const char *name )
{
    rc_t rc;

    KFile *pf = NULL;
    * ppfmd5 = NULL;

    rc = KDirectoryOpenFileWrite ( dir, & pf, true, "%s", name );
    if ( rc == 0 && md5 != NULL )
    {
        KMD5File *md5file;
        rc = ( append ? KMD5FileMakeAppend : KMD5FileMakeWrite ) ( & md5file, pf, md5, name );
        if ( rc != 0 )
        {
            KFileRelease ( pf );
            pf = NULL;
        }
        else
        {
            * ppfmd5 = md5file;
            pf = KMD5FileToKFile ( md5file );
        }
    }

    * ppf = pf;

    return rc;
}


/* CreateColumn
 * VCreateColumn
 *  create a new or open an existing column
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "checksum" [ IN ] - checksum mode
 *
 *  "pgsize" [ IN ] - size of internal column pages
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVCreateColumnInt ( KDBManager *self,
    KColumn **colp, KDirectory *wd, KCreateMode cmode,
    KChecksum checksum, size_t pgsize, const char *path, va_list args )
{
    char colpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        colpath, sizeof colpath, path, args );
    if ( rc == 0 )
    {
        KDirectory *dir;

        switch ( KDBPathType ( /*NULL,*/ wd, NULL, colpath ) )
        {
        case kptNotFound:
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );
        case kptColumn:
        case kptColumn | kptAlias:
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ( ( cmode & kcmValueMask ) == kcmCreate )
                return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcExists );
            if (KDBManagerOpenObjectBusy (self, colpath))
                return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcBusy );
            /* test now for locked directory */
            rc = KDBWritable (wd, colpath);
            if (rc)
            {
                switch (GetRCState(rc))
                {
                default:
                    return rc;
                case rcLocked:
                    return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcLocked );
                case rcReadonly:
                    return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcReadonly );
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

        case kptDatabase:
        case kptDatabase | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcDatabase, rcExists);

        case kptTable:
        case kptTable | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcTable, rcExists);

        case kptIndex:
        case kptIndex | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcIndex, rcExists);

        case kptMetadata:
        case kptMetadata | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcMetadata, rcExists);

	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a column
	     * or a non related file */
	    if ( KDBOpenPathTypeRead ( self, wd, colpath, NULL, kptColumn, NULL, false,
            NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcCreating, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        /* [re]create directory */
        rc = KDirectoryCreateDir ( wd, 0775, cmode, "%s", colpath );
        if ( rc != 0 )
            return rc;

        /* create column as a directory */
        rc = KDirectoryOpenDirUpdate ( wd, & dir, false, "%s", colpath );
        if ( rc == 0 )
        {
            KColumn *col = NULL;
            KMD5SumFmt *md5 = NULL;

            /* create an MD5 digest file for column */
            if ( KCOL_CURRENT_VERSION >= 3 && ( cmode & kcmMD5 ) != 0 )
            {
                KFile * f;

                /* create or open the md5 digest file */
                rc = KDirectoryCreateFile ( wd, &f, true, 0664, kcmOpen, "%s/md5", colpath );
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

            /* create column - will attach several references to "md5" */
            if ( rc == 0 )
                rc = KColumnCreate ( & col, dir, cmode, checksum, pgsize, colpath, md5 );

            /* release our reference to "md5" if NULL then no problem */
            if (md5)
                KMD5SumFmtRelease ( md5 );

            if ( rc == 0 )
            {
                rc = KDBManagerInsertColumn ( self, col );
                if (rc == 0 )
                {
                    * colp = col;
                    return 0;
                }

                KColumnRelease ( col );
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;
}

static
rc_t KDBManagerVCreateColumnInt_noargs ( KDBManager *self,
    KColumn **colp, KDirectory *wd, KCreateMode cmode,
    KChecksum checksum, size_t pgsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateColumnInt ( self, colp, wd, cmode, checksum, pgsize, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVCreateColumn ( KDBManager *self, KColumn **col,
    KCreateMode cmode, KChecksum checksum, size_t pgsize,
    const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcParam, rcNull );

    * col = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );

    return KDBManagerVCreateColumnInt
        ( self, col, self -> wd, cmode, checksum, pgsize, path, args );
}

LIB_EXPORT rc_t CC KDBManagerCreateColumn ( KDBManager *self, KColumn **col,
    KCreateMode cmode, KChecksum checksum, size_t pgsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateColumn ( self, col, cmode, checksum, pgsize, path, args );
    va_end ( args );

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


/* OpenColumnRead
 * VOpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path_fmt" + "args" (formatted string with varargs)  [ IN ] - NUL terminated
 *  string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVOpenColumnReadInt2 ( const KDBManager *cself,
    const KColumn **colp, const KDirectory *wd,
    const char *path_fmt, va_list args, bool *cached, bool try_srapath, va_list args2 )
{
    char colpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        colpath, sizeof colpath, path_fmt, args );
    if ( rc == 0 )
    {
        KSymbol *sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind (cself, colpath);
        if (sym != NULL)
        {
            const KColumn *ccol;
            rc_t obj;

            if(cached != NULL ) *cached = true;

            switch (sym->type)
            {
            case kptColumn:
                ccol = (const KColumn*)sym->u.obj;
                /* if open for update, refuse */
                if ( ccol -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KColumnAddRef ( ccol );
                    if ( rc == 0 )
                        * colp = ccol;
                    return rc;
                }
                obj = rcColumn;
                break;
            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptDatabase:
                obj = rcDatabase;
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
            char path[4096];
            int size = 0;

            if ( cached != NULL )
                *cached = false;

            /* TODO: check if colpath is what we want to pass to KDBOpenPathTypeRead
             * in this case we don't need to vprintf to 'path'
            */
            /* VDB-4386: cannot treat va_list as a pointer! */
            if ( path_fmt != NULL )
                size = /*( args == NULL ) ? snprintf  ( path, sizeof path, "%s", path_fmt ) :*/
                    vsnprintf ( path, sizeof path, path_fmt, args2 );
            if ( size < 0 || ( size_t ) size >=  sizeof path )
                rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );

            if (rc == 0)
                rc = KDBOpenPathTypeRead ( cself, wd, path, &dir, kptColumn, NULL,
                    try_srapath, NULL );

            if ( rc == 0 )
            {
                KColumn *col;

                rc = KColumnMakeRead ( & col, dir, colpath, NULL );

                if ( rc == 0 )
                {
                    rc = KDBManagerInsertColumn ( (KDBManager*)cself, col );
                    if ( rc == 0 )
                    {
                        * colp = col;
                        return 0;
                    }

                    KColumnRelease ( col );
                }

                KDirectoryRelease ( dir );
            }
        }
    }
    return rc;
}

static
rc_t KDBManagerVOpenColumnReadInt ( const KDBManager *cself,
    const KColumn **colp, const KDirectory *wd,
    const char *path_fmt, va_list args, bool *cached, bool try_srapath )
{
    rc_t rc;
    va_list args2;

    /* VDB-4386: cannot treat va_list as a pointer! */
/*    if ( args == NULL )
        return KDBManagerVOpenColumnReadInt2 ( cself, colp, wd, path_fmt, args, cached, try_srapath, NULL );*/

    va_copy ( args2, args );
    rc = KDBManagerVOpenColumnReadInt2 ( cself, colp, wd, path_fmt, args, cached, try_srapath, args2 );
    va_end ( args2 );

    return rc;
}

static
rc_t KDBManagerVOpenColumnReadInt_noargs ( const KDBManager *cself,
    const KColumn **colp, const KDirectory *wd,
    const char *path_fmt, bool *cached, int try_srapath, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, try_srapath );
    rc = KDBManagerVOpenColumnReadInt ( cself, colp, wd, path_fmt, args, cached, try_srapath );
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

    return KDBManagerVOpenColumnReadInt ( self, col, self -> wd, path, args , NULL, true);
}

LIB_EXPORT rc_t CC KTableOpenColumnRead ( const KTable *self,
    const KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KTableVOpenColumnRead ( self, col, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVOpenColumnRead ( const KTable *self,
    const KColumn **colp, const char *name, va_list args )
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


/* OpenColumnUpdate
 * VOpenColumnUpdate
 *  open a column for read/write
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path_fmt" formatted with "args" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVOpenColumnUpdateInt ( KDBManager *self,
    KColumn **colp, KDirectory *wd, bool try_srapath,
    const char *path_fmt, va_list args )
{
    char colpath [ 4096 ];
    rc_t rc = 0;
    int z = 0;

/*    rc = KDirectoryVResolvePath ( wd, 1,
        colpath, sizeof colpath, path_fmt, args ); */
/* VDB-4386: cannot treat va_list as a pointer! */
    if ( path_fmt != NULL )
        z = /*(args == NULL) ? snprintf  ( colpath, sizeof colpath, "%s", path_fmt) :*/
            vsnprintf ( colpath, sizeof colpath, path_fmt, args );
    if (z < 0 || z >= (int) sizeof colpath)
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    if ( rc == 0 )
    {
        KSymbol * sym;
/*         KFile *f; */
/*         KMD5SumFmt * md5 = NULL; */
        KDirectory *dir;

        /* if already open, refuse */
        sym = KDBManagerOpenObjectFind (self, colpath);
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
        switch (KDBPathType ( /*NULL,*/ wd, NULL, colpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a table
	     * or a non related file
	     * this should be changed to a readonly as it is not possible not
	     * disallowed.  rcReadonly not rcUnauthorized
	     */
	    if ( KDBOpenPathTypeRead ( self, wd, colpath, NULL, kptColumn, NULL,
            try_srapath, NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcOpening, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        case kptColumn:
        case kptColumn | kptAlias:
            break;
        }

        /* test now for locked directory */
        rc = KDBWritable (wd, colpath);
        switch (GetRCState(rc))
        {
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcUnexpected );
        case rcLocked:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcLocked );
        case rcReadonly:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcReadonly );
        case 0:
            rc = 0;
            break;
        }


        rc = KDirectoryOpenDirUpdate ( wd, & dir, 0, "%s", colpath );
        if ( rc == 0 )
        {
            KColumn *col;
            KMD5SumFmt *md5 = NULL;
            /* open existing md5 digest file */
            KFile * f;
            rc = KDirectoryOpenFileWrite_v1 ( dir, & f, true, "md5" );
            if ( rc == 0 )
            {
                rc = KMD5SumFmtMakeUpdate ( &md5, f );
                if ( rc != 0 )
                    KFileRelease ( f );
            }
            else if ( GetRCState ( rc ) == rcNotFound )
                rc = 0;

            /* make column - will attach several references to "md5" */
            if ( rc == 0 )
                rc = KColumnMakeUpdate ( & col, dir, colpath, md5 );

            /* release our reference to "md5" */
            KMD5SumFmtRelease ( md5 );

            if ( rc == 0 )
            {
                rc = KDBManagerInsertColumn ( self, col );
                if ( rc == 0 )
                {
                    * colp = col;
                    return 0;
                }

                KColumnRelease ( col );
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;
}

static
rc_t KDBManagerVOpenColumnUpdateInt_noargs ( KDBManager *self,
    KColumn **colp, KDirectory *wd, bool try_srapath,
    const char *path_fmt, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path_fmt );
    rc = KDBManagerVOpenColumnUpdateInt ( self, colp, wd, try_srapath, path_fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenColumnUpdate ( KDBManager *self,
    KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenColumnUpdate ( self, col, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenColumnUpdate ( KDBManager *self,
    KColumn **col, const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * col = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenColumnUpdateInt
        ( self, col, self -> wd, true, path, args );
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


/* Locked
 *  returns non-zero if locked
 */
static bool CC KWColumnLocked ( const KColumn *self )
{
    rc_t rc = KDBWritable ( self -> dir, "." );
    return GetRCState ( rc ) == rcLocked;
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "type" [ IN ] - a KDBPathType
 *  valid values are kptMetadata and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
static
rc_t KColumnLockInt (const KColumn  * self, char * path, size_t path_size,
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
        case kptIndex:
        case kptMetadata:
/*         case kptIndex: */
            rc = KDBVMakeSubPath (self->dir, path, sizeof path, ns, strlen (ns),
                                  name, args);
            break;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KColumnVWritable ( const KColumn *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc;
    char path [ 256 ];

    rc = KColumnLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBWritable (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KColumnWritable ( const KColumn *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KColumnVWritable ( self, type, name, args );
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
 *  valid values are kptMetadata and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC KColumnVLock ( KColumn *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    rc = KColumnLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBLockDir (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KColumnLock ( KColumn *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KColumnVLock (self, type, name, args);
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
 *  valid values are kptMetadata and kptIndex
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC KColumnVUnlock ( KColumn *self, uint32_t type, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    rc = KColumnLockInt (self, path, sizeof path, type, name, args);
    if (rc == 0)
        rc = KDBUnlockDir (self->dir, path);
    return rc;
}

LIB_EXPORT rc_t CC KColumnUnlock ( KColumn *self, uint32_t type, const char *name, ... )
{
    rc_t rc;
    va_list args;
    va_start ( args, name );
    rc = KColumnVUnlock (self, type, name, args);
    va_end (args);
    return rc;
}


/* Version
 *  returns the column format version
 */
static rc_t CC KWColumnVersion ( const KColumn *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    return KColumnIdxVersion ( & self -> idx, version );
}

/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
static
rc_t CC KWColumnByteOrder ( const KColumn *self, bool *reversed )
{
    if ( reversed == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    * reversed = false;
    return KColumnIdxByteOrder ( & self -> idx, reversed );
}

/* IdRange
 *  returns id range for column
 */
static
rc_t CC KWColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count )
{
    rc_t rc;
    int64_t dummy, last;

    if ( first == NULL && count == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    if ( first == NULL )
        first = & dummy;
    else if ( count == NULL )
        count = ( uint64_t * ) & dummy;

    * first = 0;
    * count = 0;

    rc = KColumnIdxIdRange ( & self -> idx, first, & last );
    if ( rc != 0 )
        * count = 0;
    else
        * count = last - * first + 1;

    return rc;
}


/* FindFirstRowId
 *  locates the first valid row-id starting from a given id.
 *  this will be either the start id provided, or
 *  the first row from the next blob, if available.
 *
 *  "found" [ OUT ] - will contain the value of "start" if this is contained within a blob,
 *  or the first row-id of the next blob after "start", if any.
 *
 *  "start" [ IN ] - starting row-id in search, inclusive. if this id is valid,
 *  it will be returned in "found"
 *
 *  returns 0 if id is found, rcNotFound if no more data were available.
 *  may return other codes upon error.
 */
static
rc_t CC KWColumnFindFirstRowId ( const KColumn * self, int64_t * found, int64_t start )
{
    rc_t rc;

    if ( found == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KColumnIdxFindFirstRowId ( & self -> idx, found, start );
        if ( rc == 0 )
            return 0;

        * found = 0;
    }

    return rc;
}


/* Reindex
 *  optimize indices
 */
LIB_EXPORT rc_t CC KColumnReindex ( KColumn *self )
{
    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcReindexing, rcSelf, rcNull );
    if ( self -> read_only )
        return RC ( rcDB, rcColumn, rcReindexing, rcColumn, rcReadonly );
    return KColumnIdxReindex ( & self -> idx, self -> md5, self -> commit_freq,
        self -> df . eof, self -> df . pgsize, self -> checksum );
}


/* CommitFreq
 * SetCommitFreq
 *  manage frequency of commits
 */
LIB_EXPORT rc_t CC KColumnCommitFreq ( KColumn *self, uint32_t *freq )
{
    if ( freq == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * freq = 0;
        return RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
    }

    * freq = self -> commit_freq;
    return 0;
}

LIB_EXPORT rc_t CC KColumnSetCommitFreq ( KColumn *self, uint32_t freq )
{
    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcUpdating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcColumn, rcUpdating, rcColumn, rcReadonly );

    self -> commit_freq = freq;
    return 0;
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static
rc_t CC KWColumnOpenManagerRead ( const KColumn *self, const KDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
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

LIB_EXPORT rc_t CC KColumnOpenManagerUpdate ( KColumn *self, KDBManager **mgr )
{
    rc_t rc;

    if ( mgr == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
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
 *  duplicate reference to parent table
 *  NB - returned reference must be released
 */
static
rc_t CC KWColumnOpenParentRead ( const KColumn *self, const KTable **tbl )
{
    rc_t rc;

    if ( tbl == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KTableAddRef ( self -> tbl );
        if ( rc == 0 )
        {
            * tbl = self -> tbl;
            return 0;
        }

        * tbl = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenParentUpdate ( KColumn *self, KTable **tbl )
{
    rc_t rc;

    if ( tbl == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else if ( self -> tbl != NULL && self -> tbl -> read_only )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcTable, rcReadonly );
        else
        {
            rc = KTableAddRef ( self -> tbl );
            if ( rc == 0 )
            {
                * tbl = self -> tbl;
                return 0;
            }
        }

        * tbl = NULL;
    }

    return rc;
}

/* OpenDirectory
 *  duplicate reference to the directory in use
 *  NB - returned reference must be released
 */
LIB_EXPORT rc_t CC KColumnOpenDirectoryRead ( const KColumn *self, const KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KDirectoryAddRef ( self -> dir );
            if ( rc == 0 )
            {
                * dir = self -> dir;
                return 0;
            }
        }

        * dir = NULL;
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenDirectoryUpdate ( KColumn *self, KDirectory **dir )
{
    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else if ( self -> dir != NULL && self -> dir -> read_only )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcDirectory, rcReadonly );
        else
        {
            rc = KDirectoryAddRef ( self -> dir );
            if ( rc == 0 )
            {
                * dir = self -> dir;
                return 0;
            }
        }

        * dir = NULL;
    }

    return rc;
}
