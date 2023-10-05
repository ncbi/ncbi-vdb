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

#include <klib/rc.h>

#include "wcolumn-priv.h"
#include "wcolumnblob-priv.h"

#include "wdbmgr.h"
#include "wtable-priv.h"
#include "wkdb-priv.h"
#include "werror-priv.h"
#include "wmeta.h"

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
static rc_t CC KWColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap );
static rc_t CC KWColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id );

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
    KWColumnOpenParentRead,
    KWColumnOpenMetadataRead,
    KWColumnOpenBlobRead
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
rc_t KWColumnMake ( KColumn **colp, const KDirectory *dir, const char *path,
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


rc_t KWColumnMakeRead ( KColumn **colp, const KDirectory *dir, const char *path, KMD5SumFmt * md5 )
{
    rc_t rc = KWColumnMake ( colp, dir, path, md5, true );
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

rc_t KColumnMakeUpdate ( KColumn **colp,
    KDirectory *dir, const char *path, KMD5SumFmt *md5 )
{
    rc_t rc = KWColumnMake ( colp, dir, path, md5, false );
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

    rc = KWColumnMake ( colp, dir, path, md5, false );

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


/* Locked
 *  returns non-zero if locked
 */
static bool CC KWColumnLocked ( const KColumn *self )
{
    rc_t rc = KDBWWritable ( self -> dir, "." );
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
        rc = KDBWWritable (self->dir, path);
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

static
rc_t CC
KWColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap )
{
    rc_t rc;
    const KMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBWManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false, &meta_is_cached );
    if ( rc == 0 )
    {
        if(!meta_is_cached) ((KMetadata*)meta) -> col = KColumnAttach ( self );
        * metap = meta;
    }

    return rc;
}

LIB_EXPORT rc_t CC KColumnOpenMetadataUpdate ( KColumn *self, KMetadata **metap )
{
    rc_t rc;
    KMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * metap = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcColumn, rcOpening, rcColumn, rcReadonly );

    rc = KDBManagerOpenMetadataUpdateInt ( self -> mgr, & meta, self -> dir, self -> md5 );
    if ( rc == 0 )
    {
        meta -> col = KColumnAttach ( self );
        * metap = meta;
    }

    return rc;
}

/* OpenBlobRead
 * OpenBlobUpdate
 *  opens an existing blob containing row data for id
 */
static
rc_t CC
KWColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id )
{
    rc_t rc;
    KColumnBlob *blob;

    if ( blobp == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );
    * blobp = NULL;

    rc = KWColumnBlobMake ( & blob, self -> idx . idx1 . bswap );
    if ( rc == 0 )
    {
        rc = KWColumnBlobOpenRead ( blob, self, id );
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

