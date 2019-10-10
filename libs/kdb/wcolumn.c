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
#include "wcolumn-priv.h"
#include "dbmgr-priv.h"
#include "wtable-priv.h"
#include "wkdb-priv.h"
#include "werror-priv.h"
#include <kdb/kdb-priv.h>
#include <klib/symbol.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/impl.h>
#include <klib/checksum.h>
#include <klib/data-buffer.h>
#include <klib/printf.h>
#include <klib/log.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <byteswap.h>

#include <os-native.h>
#include <va_copy.h>


/*--------------------------------------------------------------------------
 * KColumn
 *  a collection of blobs indexed by oid
 */

/* Whack
 */
static
rc_t KColumnWhack ( KColumn *self )
{
    rc_t rc;
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;
    assert ( mgr != NULL );

    KRefcountWhack ( & self -> refcount, "KColumn" );

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
                free ( self );
                return 0;
            }
        }
    }

    KRefcountInit ( & self -> refcount, 1, "KColumn", "whack", "kcol" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KColumnAddRef ( const KColumn *cself )
{
    KColumn *self = ( KColumn* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KColumn" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcColumn, rcAttaching, rcRange, rcExcessive );
        }
        ++ self -> opencount;
    }
    return 0;
}

LIB_EXPORT rc_t CC KColumnRelease ( const KColumn *cself )
{
    KColumn *self = ( KColumn* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KColumn" ) )
        {
        case krefWhack:
            return KColumnWhack ( ( KColumn* ) self );
        case krefLimit:
            return RC ( rcDB, rcColumn, rcReleasing, rcRange, rcExcessive );
        }
        -- self -> opencount;
    }
    return 0;
}


/* Attach
 */
KColumn *KColumnAttach ( const KColumn *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KColumn" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KColumn* ) self;
}

/* Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
rc_t KColumnSever ( const KColumn *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KColumn" ) )
        {
        case krefWhack:
            return KColumnWhack ( ( KColumn* ) self );
        case krefLimit:
            return RC ( rcDB, rcColumn, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
static
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
    col -> dir = ( KDirectory* ) dir;
    col -> md5 = md5;
    rc = KMD5SumFmtAddRef ( md5 );
    KRefcountInit ( & col -> refcount, 1, "KColumn", "make", path );
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

        rc = KDBManagerVCreateColumnInt ( self -> mgr, colp,
                                          self -> dir, cmode | kcmParents, checksum, pgsize, path, NULL );
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
            int size;

            if ( cached != NULL )
                *cached = false;

            /* TODO: check if colpath is what we want to pass to KDBOpenPathTypeRead
             * in this case we don't need to vprintf to 'path'
            */
            size = ( args == NULL ) ?
                snprintf  ( path, sizeof path, "%s", path_fmt ) :
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

    if ( args == NULL )
        return KDBManagerVOpenColumnReadInt2 ( cself, colp, wd, path_fmt, args, cached, try_srapath, NULL );

    va_copy ( args2, args );
    rc = KDBManagerVOpenColumnReadInt2 ( cself, colp, wd, path_fmt, args, cached, try_srapath, args2 );
    va_end ( args2 );

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
        rc = KDBManagerVOpenColumnReadInt ( self -> mgr,
                                            colp, self -> dir, path, NULL, &col_is_cached, false );
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
    int z;

/*    rc = KDirectoryVResolvePath ( wd, 1,
        colpath, sizeof colpath, path_fmt, args ); */
    z = (args == NULL) ?
        snprintf  ( colpath, sizeof colpath, "%s", path_fmt) :
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
            rc = KDirectoryVOpenFileWrite ( dir, & f, true, "md5", NULL );
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
        rc = KDBManagerVOpenColumnUpdateInt ( self -> mgr,
                                              colp, self -> dir, false, path, NULL );
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
LIB_EXPORT bool CC KColumnLocked ( const KColumn *self )
{
    rc_t rc;

    if ( self == NULL )
        return false;

    rc = KDBWritable ( self -> dir, "." );
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
LIB_EXPORT rc_t CC KColumnVersion ( const KColumn *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
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
LIB_EXPORT rc_t CC KColumnByteOrder ( const KColumn *self, bool *reversed )
{
    if ( reversed == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * reversed = false;
        return RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
    }
     
    return KColumnIdxByteOrder ( & self -> idx, reversed );
}

/* IdRange
 *  returns id range for column
 */
LIB_EXPORT rc_t CC KColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count )
{
    rc_t rc;
    int64_t dummy, last;

    if ( first == NULL && count == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    if ( first == NULL )
        first = & dummy;
    else if ( count == NULL )
        count = ( uint64_t * ) & dummy;

    if ( self == NULL )
    {
        * first = 0;
        * count = 0;
        return RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
    }

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
LIB_EXPORT rc_t CC KColumnFindFirstRowId ( const KColumn * self, int64_t * found, int64_t start )
{
    rc_t rc;

    if ( found == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = KColumnIdxFindFirstRowId ( & self -> idx, found, start );
            if ( rc == 0 )
                return 0;
        }

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
LIB_EXPORT rc_t CC KColumnOpenManagerRead ( const KColumn *self, const KDBManager **mgr )
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
LIB_EXPORT rc_t CC KColumnOpenParentRead ( const KColumn *self, const KTable **tbl )
{
    rc_t rc;

    if ( tbl == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
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


/*--------------------------------------------------------------------------
 * KColumnBlob
 *  one or more rows of column data
 */
struct KColumnBlob
{
    /* holds either an existing blob loc
       or new blob index range */
    KColBlobLoc loc;

    /* holds old and new page maps */
    KColumnPageMap pmorig;
    KColumnPageMap pmnew;

    /* owning column */
    KColumn *col;

    /* refcount */
    atomic32_t refcount;

    /* number of bytes written to blob */
    uint32_t num_writ;

    /* checksums */
    uint32_t crc32;
    MD5State md5;

    /* open mode */
    uint8_t read_only;

    /* for validation */
    bool bswap;
};


/* Whack
 */
static
rc_t KColumnBlobWhack ( KColumnBlob *self )
{
    KColumn *col = self -> col;
    assert ( col != NULL );

    KColumnPageMapWhack ( & self -> pmorig, & col -> df );
    if ( ! self -> read_only )
        KColumnPageMapWhack ( & self -> pmnew, & col -> df );

    /* cannot recover from errors here,
       since the page maps needed whacking first,
       and the column is needed for that. */
    KColumnSever ( col );

    free ( self );
    return 0;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KColumnBlobAddRef ( const KColumnBlob *cself )
{
    if ( cself != NULL )
    {
        atomic32_inc ( & ( ( KColumnBlob* ) cself ) -> refcount );
    }
    return 0;
}

LIB_EXPORT rc_t CC KColumnBlobRelease ( const KColumnBlob *cself )
{
    KColumnBlob *self = ( KColumnBlob* ) cself;
    if ( cself != NULL )
    {
        if ( atomic32_dec_and_test ( & self -> refcount ) )
            return KColumnBlobWhack ( self );
    }
    return 0;
}

/* OpenRead
 * OpenUpdate
 */
static
rc_t KColumnBlobOpenRead ( KColumnBlob *self, const KColumn *col, int64_t id )
{
    /* locate blob */
    rc_t rc = KColumnIdxLocateBlob ( & col -> idx, & self -> loc, id, id );
    if ( rc == 0 )
    {
        /* indicates not written */
        /*assert ( self -> num_writ == 0 );*/

        /* open page map to blob */
        rc = KColumnPageMapOpen ( & self -> pmorig,
            ( KColumnData* ) & col -> df, self -> loc . pg, self -> loc . u . blob . size );
        if ( rc == 0 )
        {
            /* existing blob must have proper checksum bytes */
            if ( self -> loc . u . blob . size >= col -> csbytes )
            {
                /* remove them from apparent blob size */
                self -> loc . u . blob . size -= col -> csbytes;
                return 0;
            }

            /* the blob is corrupt */
            KColumnPageMapWhack ( & self -> pmorig, & col -> df );
            rc = RC ( rcDB, rcBlob, rcOpening, rcBlob, rcCorrupt );
        }
    }

    return rc;
}

static
rc_t KColumnBlobOpenUpdate ( KColumnBlob *self, KColumn *col, int64_t id )
{
    /* open existing blob */
    rc_t rc = KColumnBlobOpenRead ( self, col, id );
    if ( rc == 0 )
    {
        /* create a new page map for replacement */
        rc = KColumnPageMapCreate ( & self -> pmnew, & col -> df );
        if ( rc == 0 )
        {
            /* initialize for writing checksums */
            switch ( col -> checksum )
            {
            case kcsCRC32:
                self -> crc32 = 0;
                break;
            case kcsMD5:
                MD5StateInit ( & self -> md5 );
                break;
            }
        }

        /* tear down results of opening for read */
        KColumnPageMapWhack ( & self -> pmorig, & col -> df );
    }

    return rc;
}

/* Create
 */
static
rc_t KColumnBlobCreate ( KColumnBlob *self, KColumn *col )
{
    rc_t rc;

    if ( col -> md5 != NULL )
    {
	/* this perhaps should be split into wcoldat.c and wcolidx.c */
	rc = KMD5FileBeginTransaction ( col -> df . fmd5 );
	if ( rc == 0 )
	{
	    rc = KMD5FileBeginTransaction ( col -> idx . idx2 . fmd5 );
	    if ( rc == 0 )
	    {
		rc = KMD5FileBeginTransaction ( col -> idx . idx0 . fmd5 );
		if ( rc == 0 )
		{
		    rc = KMD5FileBeginTransaction ( col -> idx . idx1 . fmd5 );
		    if ( rc == 0 )
		    {
			rc = KMD5FileBeginTransaction ( col -> idx . idx1 . fidxmd5 );
			if ( rc != 0 )
			    KMD5FileCommit ( col -> idx . idx1 .  fmd5 );
		    }
		    if ( rc != 0 )
			KMD5FileCommit ( col -> idx . idx0 .  fmd5 );
		}
		if ( rc != 0 )
		    KMD5FileCommit ( col -> idx . idx2 .  fmd5 );
	    }
	    if ( rc != 0 )
		KMD5FileCommit ( col -> df . fmd5 );
	}
	if ( rc != 0 )
	    return rc;
    }

    /* no location yet */
    memset ( & self -> loc, 0, sizeof self -> loc );

    /* invalid existing page map */
    memset ( & self -> pmorig, 0, sizeof self -> pmorig );

    /* create a new page map */
    rc = KColumnPageMapCreate ( & self -> pmnew, & col -> df );
    if ( rc == 0 )
    {
        /* initialize for writing checksums */
        switch ( col -> checksum )
        {
        case kcsCRC32:
            self -> crc32 = 0;
            break;
        case kcsMD5:
            MD5StateInit ( & self -> md5 );
            break;
        }
    }

    return rc;
}

/* Make
 */
static
rc_t KColumnBlobMake ( KColumnBlob **blobp, bool bswap )
{
    KColumnBlob *blob = malloc ( sizeof * blob );
    if ( blob == NULL )
        return RC ( rcDB, rcBlob, rcConstructing, rcMemory, rcExhausted );

    memset ( blob, 0, sizeof * blob );
    atomic32_set ( & blob -> refcount, 1 );
    blob -> bswap = bswap;

    * blobp = blob;
    return 0;
}

/* OpenBlobRead
 * OpenBlobUpdate
 *  opens an existing blob containing row data for id
 */
LIB_EXPORT rc_t CC KColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id )
{
    rc_t rc;
    KColumnBlob *blob;

    if ( blobp == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );
    * blobp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );

    rc = KColumnBlobMake ( & blob, self -> idx . idx1 . bswap );
    if ( rc == 0 )
    {
        rc = KColumnBlobOpenRead ( blob, self, id );
        if ( rc == 0 )
        {
            blob -> col = KColumnAttach ( self );
            blob -> read_only = true;
            * blobp = blob;
            return 0;
        }
        
        free ( blob );
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenBlobUpdate ( KColumn *self, KColumnBlob **blobp, int64_t id )
{
    rc_t rc;

    if ( blobp == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );
    * blobp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );
    if ( self -> read_only )
        return RC ( rcDB, rcColumn, rcOpening, rcColumn, rcReadonly );

    rc = KColumnBlobMake ( blobp, self -> idx . idx1 . bswap );
    if ( rc == 0 )
    {
        KColumnBlob *blob = * blobp;
        rc = KColumnBlobOpenUpdate ( blob, self, id );
        if ( rc == 0 )
        {
            blob -> col = KColumnAttach ( self );
            * blobp = blob;
            return 0;
        }
                    
        free ( blob );
    }

    return rc;
}

/* CreateBlob
 *  creates a new, unassigned blob
 */
LIB_EXPORT rc_t CC KColumnCreateBlob ( KColumn *self, KColumnBlob **blobp )
{
    rc_t rc;

    if ( blobp == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );
    * blobp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );
    if ( self -> read_only )
        return RC ( rcDB, rcColumn, rcOpening, rcColumn, rcReadonly );

    rc = KColumnBlobMake ( blobp, self -> idx . idx1 . bswap );
    if ( rc == 0 )
    {
        KColumnBlob *blob = * blobp;
        rc = KColumnBlobCreate ( blob, self );
        if ( rc == 0 )
        {
            blob -> col = KColumnAttach ( self );
            * blobp = blob;
            return 0;
        }
                    
        free ( blob );
    }

    return rc;
}

/* IdRange
 *  returns id range for blob
 *
 *  "first" [ OUT, NULL OKAY ] - optional return parameter for first id
 *
 *  "last" [ OUT, NULL OKAY ] - optional return parameter for last id
 */
LIB_EXPORT rc_t CC KColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count )
{
    rc_t rc;

    if ( first == NULL || count == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    else if ( self == NULL )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcSelf, rcNull );
    else if ( self -> loc . id_range == 0 )
        rc = RC ( rcDB, rcBlob, rcAccessing, rcRange, rcEmpty );
    else
    {
        * first = self -> loc . start_id;
        * count = self -> loc . id_range;
        return 0;
    }

    if ( first != NULL )
        * first = 0;
    if ( count != NULL )
        * count = 0;

    return rc;
}

/* KColumnBlobValidate
 *  runs checksum validation on unmodified blob
 */
static
rc_t KColumnBlobValidateCRC32 ( const KColumnBlob *self )
{
    rc_t rc;
    const KColumn *col = self -> col;

    uint8_t buffer [ 8 * 1024 ];
    size_t to_read, num_read, total, size;

    uint32_t cs, crc32 = 0;

    /* calculate checksum */
    for ( size = self -> loc . u . blob. size, total = 0; total < size; total += num_read )
    {
        to_read = size - total;
        if ( to_read > sizeof buffer )
            to_read = sizeof buffer;

        rc = KColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        crc32 = CRC32 ( crc32, buffer, num_read );
    }

    /* read stored checksum */
    rc = KColumnDataRead ( & col -> df,
        & self -> pmorig, size, & cs, sizeof cs, & num_read );
    if ( rc != 0 )
        return rc;
    if ( num_read != sizeof cs )
        return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

    if ( self -> bswap )
        cs = bswap_32 ( cs );

    if ( cs != crc32 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

static
rc_t KColumnBlobValidateMD5 ( const KColumnBlob *self )
{
    rc_t rc;
    const KColumn *col = self -> col;

    uint8_t buffer [ 8 * 1024 ];
    size_t to_read, num_read, total, size;

    MD5State md5;
    uint8_t digest [ 16 ];

    MD5StateInit ( & md5 );

    /* calculate checksum */
    for ( size = self -> loc . u . blob . size, total = 0; total < size; total += num_read )
    {
        to_read = size - total;
        if ( to_read > sizeof buffer )
            to_read = sizeof buffer;

        rc = KColumnDataRead ( & col -> df,
            & self -> pmorig, total, buffer, to_read, & num_read );
        if ( rc != 0 )
            return rc;
        if ( num_read == 0 )
            return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

        MD5StateAppend ( & md5, buffer, num_read );
    }

    /* read stored checksum */
    rc = KColumnDataRead ( & col -> df,
        & self -> pmorig, size, buffer, sizeof digest, & num_read );
    if ( rc != 0 )
        return rc;
    if ( num_read != sizeof digest )
        return RC ( rcDB, rcBlob, rcValidating, rcTransfer, rcIncomplete );

    /* finish MD5 digest */
    MD5StateFinish ( & md5, digest );

    if ( memcmp ( buffer, digest, sizeof digest ) != 0 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

LIB_EXPORT rc_t CC KColumnBlobValidate ( const KColumnBlob *self )
{
    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcValidating, rcSelf, rcNull );

    if ( self -> num_writ != 0 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcBusy );

    if ( self -> loc . u . blob . size != 0 ) switch ( self -> col -> checksum )
    {
    case kcsCRC32:
        return KColumnBlobValidateCRC32 ( self );
    case kcsMD5:
        return KColumnBlobValidateMD5 ( self );
    }

    return 0;
}

/* ValidateBuffer
 *  run checksum validation on buffer data
 *
 *  "buffer" [ IN ] - returned blob buffer from ReadAll
 *
 *  "cs_data" [ IN ] and "cs_data_size" [ IN ] - returned checksum data from ReadAll
 */
static
rc_t KColumnBlobValidateBufferCRC32 ( const void * buffer, size_t size, uint32_t cs )
{
    uint32_t crc32 = CRC32 ( 0, buffer, size );

    if ( cs != crc32 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

static
rc_t KColumnBlobValidateBufferMD5 ( const void * buffer, size_t size, const uint8_t cs [ 16 ] )
{
    MD5State md5;
    uint8_t digest [ 16 ];

    MD5StateInit ( & md5 );

    /* calculate checksum */
    MD5StateAppend ( & md5, buffer, size );

    /* finish MD5 digest */
    MD5StateFinish ( & md5, digest );

    if ( memcmp ( cs, digest, sizeof digest ) != 0 )
        return RC ( rcDB, rcBlob, rcValidating, rcBlob, rcCorrupt );

    return 0;
}

LIB_EXPORT rc_t CC KColumnBlobValidateBuffer ( const KColumnBlob * self,
    const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    size_t bsize;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcValidating, rcSelf, rcNull );
    if ( buffer == NULL || cs_data == NULL )
        return RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );

    bsize = KDataBufferBytes ( buffer );
    if ( bsize < self -> loc . u . blob . size )
        return RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
    if ( bsize > self -> loc . u . blob . size )
        return RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );

    if ( bsize != 0 ) switch ( self -> col -> checksum )
    {
    case kcsNone:
        break;
    case kcsCRC32:
        return KColumnBlobValidateBufferCRC32 ( buffer -> base, bsize,
            self -> bswap ? bswap_32 ( cs_data -> crc32 ) : cs_data -> crc32 );
    case kcsMD5:
        return KColumnBlobValidateBufferMD5 ( buffer -> base, bsize, cs_data -> md5_digest );
    }

    return 0;
}

/* KColumnBlobRead
 *  read data from blob
 *
 *  "offset" [ IN ] - starting offset into blob
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of bytes remaining to be read. specifically,
 *  "offset" + "num_read" + "remaining" == sizeof blob
 */
LIB_EXPORT rc_t CC KColumnBlobRead ( const KColumnBlob *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    rc_t rc;

    size_t ignore;
    if ( remaining == NULL )
        remaining = & ignore;

    if ( num_read == NULL )
        rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcBlob, rcReading, rcSelf, rcNull );
        else
        {
            size_t size = self -> num_writ;
            const KColumn *col = self -> col;

            if ( size == 0 )
                size = self -> loc . u . blob . size;

            if ( offset > size )
                offset = size;

            if ( bsize == 0 )
                rc = 0;
            else if ( buffer == NULL )
                rc = RC ( rcDB, rcBlob, rcReading, rcBuffer, rcNull );
            else
            {
                size_t to_read = size - offset;
                if ( to_read > bsize )
                    to_read = bsize;

                *num_read = 0;
                while (*num_read < to_read)
		{
                    size_t nread = 0;

                    rc = KColumnDataRead ( & col -> df, & self -> pmorig, offset + *num_read,
                        & ( ( char * ) buffer ) [ * num_read ], to_read - * num_read, & nread );
                    if ( rc != 0 )
                        break;
                    if (nread == 0)
                    {
                        rc = RC ( rcDB, rcBlob, rcReading, rcFile, rcInsufficient );
                        break;
                    }

                    *num_read += nread;
                }

                if ( rc == 0 )
                {
                    * remaining = size - offset - * num_read;
                    return 0;
                }
            }

            * remaining = size - offset;
            * num_read = 0;
            return rc;
        }

        * num_read = 0;
    }

    * remaining = 0;
    return rc;
}

/* ReadAll
 *  read entire blob, plus any auxiliary checksum data
 *
 *  "buffer" [ OUT ] - pointer to a KDataBuffer structure that will be initialized
 *  and resized to contain the entire blob. upon success, will contain the number of bytes
 *  in buffer->elem_count and buffer->elem_bits == 8.
 *
 *  "opt_cs_data [ OUT, NULL OKAY ] - optional output parameter for checksum data
 *  associated with the blob in "buffer", if any exist.
 *
 *  "cs_data_size" [ IN ] - sizeof of * opt_cs_data if not NULL, 0 otherwise
 */
LIB_EXPORT rc_t CC KColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer,
    KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    rc_t rc = 0;

    if ( opt_cs_data != NULL )
        memset ( opt_cs_data, 0, cs_data_size );

    if ( buffer == NULL )
        rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcBlob, rcReading, rcSelf, rcNull );
        else
        {
            /* determine blob size */
            size_t bsize = self -> loc . u . blob . size;

            /* ignore blobs of size 0 */
            if ( bsize == 0 )
                rc = 0;
            else
            {
                /* initialize the buffer */
                rc = KDataBufferMakeBytes ( buffer, bsize );
                if ( rc == 0 )
                {
                    /* read the blob */
                    size_t num_read, remaining;
                    rc = KColumnBlobRead ( self, 0, buffer -> base, bsize, & num_read, & remaining );
                    if ( rc == 0 )
                    {
                        /* test that num_read is everything and we have no remaining */
                        if ( num_read != bsize || remaining != 0 )
                            rc = RC ( rcDB, rcBlob, rcReading, rcTransfer, rcIncomplete );

                        else
                        {
                            /* set for MD5 - just due to switch ordering */
                            size_t cs_bytes = 16;

                            /* if not reading checksum data, then we're done */
                            if ( opt_cs_data == NULL )
                                return 0;

                            /* see what checksumming is in use */
                            switch ( self -> col -> checksum )
                            {
                            case kcsNone:
                                return 0;

                            case kcsCRC32:
                                /* reset for CRC32 */
                                cs_bytes = 4;

                                /* no break */

                            case kcsMD5:
                                if ( cs_data_size < cs_bytes )
                                {
                                    rc = RC ( rcDB, rcBlob, rcReading, rcParam, rcTooShort );
                                    break;
                                }

                                /* read checksum information */
                                rc = KColumnDataRead ( & self -> col -> df,
                                    & self -> pmorig, bsize, opt_cs_data, cs_bytes, & num_read );
                                if ( rc == 0 )
                                {
                                    if ( num_read != cs_bytes )
                                        rc = RC ( rcDB, rcBlob, rcReading, rcTransfer, rcIncomplete );
                                    else
                                    {
                                        /* success - read the blob AND the checksum data */
                                        return 0;
                                    }
                                }
                                break;
                            }
                        }
                    }

                    KDataBufferWhack ( buffer );
                }
            }
        }

        memset ( buffer, 0, sizeof * buffer );
    }

    return rc;
}

/* KColumnBlobAppend
 *  append data to open blob
 *
 *  "buffer" [ IN ] and "size" [ IN ] - blob data
 */
LIB_EXPORT rc_t CC KColumnBlobAppend ( KColumnBlob *self, const void *buffer, size_t size )
{
    KColumn *col;
    size_t total, num_writ;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcWriting, rcSelf, rcNull );

    /* data fork will refuse write anyway, but... */
    if ( self -> read_only )
        return RC ( rcDB, rcBlob, rcWriting, rcBlob, rcReadonly );

    col = self -> col;

    for ( total = 0; total < size; total += num_writ )
    {
        rc_t rc = KColumnDataWrite ( & col -> df,
            & self -> pmnew, self -> num_writ + total,
            ( const char* ) buffer + total, size - total, & num_writ );
        if ( rc != 0 )
            return rc;
        if ( num_writ == 0 )
            return RC ( rcDB, rcBlob, rcWriting, rcTransfer, rcIncomplete );
    }

    self -> num_writ += size;

    switch ( col -> checksum )
    {
    case kcsCRC32:
        self -> crc32 = CRC32 ( self -> crc32, buffer, size );
        break;
    case kcsMD5:
        MD5StateAppend ( & self -> md5, buffer, size );
        break;
    }
    
    return 0;
}

/* KColumnBlobAssignRange
 *  assign a total id range to blob at any time before commit
 *
 *  "first" [ IN ] and "count" [ IN ] - range parameters for assign
 */
LIB_EXPORT rc_t CC KColumnBlobAssignRange ( KColumnBlob *self, int64_t first, uint32_t count )
{
    rc_t rc;
    const KColumn *col;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcUpdating, rcSelf, rcNull );

    if ( count == 0 )
        return RC ( rcDB, rcBlob, rcUpdating, rcRange, rcEmpty );

    if ( self -> read_only )
        return RC ( rcDB, rcBlob, rcUpdating, rcBlob, rcReadonly );

    if ( self -> loc . id_range != 0 )
    {
        /* allow benign reassignment */
        if ( self -> loc . start_id == first &&
             self -> loc . id_range == count )
            return 0;

        /* can't be reset */
        return RC ( rcDB, rcBlob, rcUpdating, rcBlob, rcBusy );
    }

    col = self -> col;

    /* try to open read map */
    rc = KColumnBlobOpenRead ( self, col, first );
    if ( rc == 0 )
    {
        /* blob already exists
           again, allow benign reassignment */                    
        if ( self -> loc . start_id == first &&
             self -> loc . id_range == count )
            return 0;

        /* conflicting assignment */
        KColumnPageMapWhack ( & self -> pmorig, & col -> df );
        memset ( & self -> loc, 0, sizeof self -> loc );
        memset ( & self -> pmorig, 0, sizeof self -> pmorig );
        return RC ( rcDB, rcBlob, rcUpdating, rcRange, rcIncorrect );
    }

    /* expect status of not found */
    if ( GetRCState ( rc ) != rcNotFound )
        return rc;

    /* assign the range */
    self -> loc . pg = 0;
    self -> loc . u . blob . remove = 0;
    self -> loc . start_id = first;
    self -> loc . id_range = count;
    return 0;
}

/* KColumnBlobCommit
 *  commit changes to blob
 *  close to further updates
 */
static
char zero [ 4096 ];

static
rc_t KColumnBlobZeroPad ( KColumnBlob *self )
{
    KColumn *col = self -> col;
    size_t pad_bytes = self -> num_writ % col -> df . pgsize;
    if ( pad_bytes != 0 )
    {
        size_t total, num_writ;

        pad_bytes = col -> df . pgsize - pad_bytes;
        for ( total = 0; total < pad_bytes; total += num_writ )
        {
            rc_t rc;

            size_t to_write = pad_bytes - total;
            if ( to_write > sizeof zero )
                to_write = sizeof zero;

            rc = KColumnDataWrite ( & col -> df,
                & self -> pmnew, self -> num_writ + total,
                zero, to_write, & num_writ );
            if ( rc != 0 )
                return rc;
            if ( num_writ == 0 )
                return RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );
        }
    }
    return 0;
}

static
rc_t KColumnBlobDoCommit ( KColumnBlob *self )
{
    rc_t rc;
    KColBlobLoc loc;
    KColumn *col = self -> col;

    /* finish checksum */
    if ( col -> csbytes != 0 )
    {
        MD5State md5;
        uint32_t crc32;
        uint8_t digest [ 16 ];

        void *cs;
        size_t num_writ;

        switch ( col -> checksum )
        {
        case kcsCRC32:
            crc32 = self -> crc32;
            if ( self -> bswap )
                crc32 = bswap_32 ( crc32 );
            cs = & crc32;
            break;
        case kcsMD5:
        default: /* to quiet compiler warnings */
            /* work on copy in case of failure */
            md5 = self -> md5;
            MD5StateFinish ( & md5, digest );
            cs = digest;
            break;
        }

        rc = KColumnDataWrite ( & col -> df,
            & self -> pmnew, self -> num_writ,
            cs, col -> csbytes, & num_writ );
        if ( rc != 0 )
            return rc;
        if ( num_writ != col -> csbytes )
            return RC ( rcDB, rcBlob, rcCommitting, rcTransfer, rcIncomplete );

        self -> num_writ += num_writ;
    }

    /* extract index information */
    rc = KColumnPageMapId ( & self -> pmnew, & col -> df, & loc . pg );
    if ( rc == 0 )
    {
        loc . u . blob . size = ( uint32_t ) self -> num_writ;
        loc . u . blob . remove = 0;
        loc . start_id = self -> loc . start_id;
        loc . id_range = self -> loc . id_range;

        /* pad out rest of page */
        if ( col -> df . pgsize > 1 )
            rc = KColumnBlobZeroPad ( self );
        if ( rc == 0 )
        {
            /* commit data fork */
            rc = KColumnDataCommit ( & col -> df,
                & self -> pmnew, self -> num_writ );
            if ( rc == 0 )
            {
                /* commit index fork */
                rc = KColumnIdxCommit ( & col -> idx, col -> md5,
                    & loc, col -> commit_freq, col -> df . eof,
                    col -> df . pgsize, col -> checksum );
                if ( rc == 0 || rc == kdbReindex )
                {
                    rc_t status = rc;

                    /* release old pages */
                    if ( self -> loc . u . blob . size == 0 )
                        rc = 0;
                    else
                    {
                        rc = KColumnDataFree ( & col -> df,
                            & self -> pmorig, self -> loc . u . blob . size + col -> csbytes );
                        if ( GetRCState ( rc ) == rcInvalid )
                            rc = 0;
                    }

                    if ( rc == 0 )
                    {
                        /* transfer new map */
                        self -> pmorig = self -> pmnew;
                        memset ( & self -> pmnew, 0, sizeof self -> pmnew );

                        /* fill out location */
                        loc . u . blob . size -= col -> csbytes;
                        self -> loc = loc;

                        /* HACK - should open new pm on demand
                           but since the code does not yet support it,
                           disallow further writes */
                        self -> read_only = true;
                        
                        /* mark blob as clean */
                        self -> num_writ = 0;

			/* these must not be a point of failure 
			   The only failure from the KMD5FileCommit
			   behind these is on NULL parameter */
			rc = KColumnDataCommitDone ( & col -> df );
			if ( rc == 0 )
			    rc = KColumnIdxCommitDone ( & col -> idx );

                        return status;
                    }
                }

                /* revert data fork */
                KColumnDataFree ( & col -> df,
                    & self -> pmnew, self -> num_writ );
		KMD5FileRevert ( self -> col -> df . fmd5 );
            }
        }
    }

    /* remove checksum bytes */
    self -> num_writ -= col -> csbytes;
    return rc;
}

LIB_EXPORT rc_t CC KColumnBlobCommit ( KColumnBlob *self )
{
    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcBlob, rcCommitting, rcSelf, rcNull );

    if ( self -> loc . id_range == 0 )
        return RC ( rcDB, rcBlob, rcCommitting, rcRange, rcInvalid );

    if ( self -> num_writ == 0 && self -> loc . u . blob . size != 0 )
        return 0;

    assert ( self -> read_only == false );

    rc = KColumnBlobDoCommit ( self );

    if ( rc == kdbReindex )
        rc = KColumnReindex ( self -> col );

    return rc;
}
