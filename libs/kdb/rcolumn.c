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
#include "rcolumn.h"
#include "rcolumnblob.h"
#include "rdbmgr.h"
#include "rtable.h"
#include "rkdb.h"
#include "rmeta.h"

#include <klib/rc.h>
#include <klib/printf.h>
#undef KONST

#ifdef _DEBUGGING
#define POS_DEBUG(msg) DBGMSG(DBG_KDB,DBG_FLAG(DBG_KDB_POS),msg)
#else
#define POS_DEBUG(msg)
#endif


/*--------------------------------------------------------------------------
 * KRColumn (formerly KColumn)
 *  a read-only collection of blobs indexed by oid; file system-based
 */

static rc_t CC KRColumnWhack ( KRColumn *self );
static bool CC KRColumnLocked ( const KRColumn *self );
static rc_t CC KRColumnVersion ( const KRColumn *self, uint32_t *version );
static rc_t CC KRColumnByteOrder ( const KRColumn *self, bool *reversed );
static rc_t CC KRColumnIdRange ( const KRColumn *self, int64_t *first, uint64_t *count );
static rc_t CC KRColumnFindFirstRowId ( const KRColumn * self, int64_t * found, int64_t start );
static rc_t CC KRColumnOpenManagerRead ( const KRColumn *self, const KDBManager **mgr );
static rc_t CC KRColumnOpenParentRead ( const KRColumn *self, const KTable **tbl );
static rc_t CC KRColumnOpenMetadataRead ( const KRColumn *self, const KMetadata **metap );
static rc_t CC KRColumnOpenBlobRead ( const KRColumn *self, const KColumnBlob **blobp, int64_t id );

static KColumn_vt KRColumn_vt =
{
    /* Public API */
    KRColumnWhack,
    KColumnBaseAddRef,
    KColumnBaseRelease,
    KRColumnLocked,
    KRColumnVersion,
    KRColumnByteOrder,
    KRColumnIdRange,
    KRColumnFindFirstRowId,
    KRColumnOpenManagerRead,
    KRColumnOpenParentRead,
    KRColumnOpenMetadataRead,
    KRColumnOpenBlobRead
};

#define CAST() assert( bself->vt == &KRColumn_vt ); KRColumn * self = (KRColumn *)bself

/* Whack
 */
static
rc_t KRColumnWhack ( KCOLUMN_IMPL *self )
{
    rc_t rc;

    /* shut down index */
    rc = KRColumnIdxWhack ( & self -> idx );
    if ( rc == 0 )
    {
        /* shut down data fork */
        KRColumnDataWhack ( & self -> df );

        /* release owning table
           should never fail, and our recovery is flawed */
        if ( self -> tbl != NULL )
        {
            rc = KTableSever ( self -> tbl );
            if ( rc == 0 )
                self -> tbl = NULL;
        }

        /* release manager
           should never fail */
        if ( rc == 0 )
            rc = KDBManagerSever ( self -> mgr );

        if ( rc == 0 )
        {
            KDirectoryRelease ( self -> dir );
            return KColumnBaseWhack( self );
        }
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KRColumn", "whack", "kcol" );
    return rc;
}

/* Make
 */
rc_t KRColumnMake ( KRColumn **colp, const KDirectory *dir, const char *path )
{
    KRColumn *col = malloc ( sizeof * col + strlen ( path ) );
    if ( col == NULL )
        return RC ( rcDB, rcColumn, rcConstructing, rcMemory, rcExhausted );

    memset ( col, 0, sizeof * col );
    col -> dad . vt = & KRColumn_vt;
    col -> dir = dir;
    KRefcountInit ( & col -> dad . refcount, 1, "KRColumn", "make", path );
    strcpy ( col -> path, path );

    * colp = col;
    return 0;
}

rc_t KRColumnMakeRead ( KRColumn **colp, const KDirectory *dir, const char *path )
{
    rc_t rc = KRColumnMake ( colp, dir, path );
    if ( rc == 0 )
    {
        size_t pgsize;
        uint64_t data_eof;
        KRColumn *self = * colp;

        rc = KRColumnIdxOpenRead ( & self -> idx,
            dir, & data_eof, & pgsize, & self -> checksum );
        if ( rc == 0 )
        {
            rc = KRColumnDataOpenRead ( & self -> df,
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

                return 0;
            }

            KRColumnIdxWhack ( & self -> idx );
        }

        free ( self );
    }

    * colp = NULL;
    return rc;
}

/* Locked
 *  returns non-zero if locked
 */
static
bool CC KRColumnLocked ( const KRColumn *self )
{
    rc_t rc = KDBRWritable ( self -> dir, "" );
    return GetRCState ( rc ) == rcLocked;
}

/* Version
 *  returns the column format version
 */
static
rc_t CC KRColumnVersion ( const KRColumn *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    * version = 0;

    return KRColumnIdxVersion ( & self -> idx, version );
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
rc_t CC KRColumnByteOrder ( const KRColumn *self, bool *reversed )
{
    if ( reversed == NULL )
        return RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );

    * reversed = false;

    return KRColumnIdxByteOrder ( & self -> idx, reversed );
}

/* IdRange
 *  returns id range for column
 */
static
rc_t CC KRColumnIdRange ( const KRColumn *self, int64_t *first, uint64_t *count )
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

    rc = KRColumnIdxIdRange ( & self -> idx, first, & last );
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
rc_t CC KRColumnFindFirstRowId ( const KRColumn * self, int64_t * found, int64_t start )
{
    rc_t rc;

    if ( found == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        rc = KRColumnIdxFindFirstRowId ( & self -> idx, found, start );
        if ( rc == 0 )
            return 0;

        * found = 0;
    }

    return rc;
}


/* OpenManager
 *  duplicate reference to manager
 *  NB - returned reference must be released
 */
static
rc_t CC KRColumnOpenManagerRead ( const KRColumn *self, const KDBManager **mgr )
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



/* OpenParent
 *  duplicate reference to parent table
 *  NB - returned reference must be released
 */
static
rc_t CC KRColumnOpenParentRead ( const KRColumn *self, const KTable **tbl )
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

/* GetDirectory
 */
LIB_EXPORT rc_t CC KColumnGetDirectoryRead ( const KColumn *bself, const KDirectory **dir )
{
    CAST();

    rc_t rc;

    if ( dir == NULL )
        rc = RC ( rcDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else
        {
            * dir = self -> dir;
            return KDirectoryAddRef ( * dir );
        }

        * dir = NULL;
    }

    return rc;
}

static
rc_t CC
KRColumnOpenMetadataRead ( const KRColumn *self, const KMetadata **metap )
{
    rc_t rc;
    KRMetadata *meta;

    if ( metap == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBRManagerOpenMetadataReadInt ( self -> mgr, & meta, self -> dir, 0, false );
    if ( rc == 0 )
    {
        meta -> col = KColumnAttach ( self );
        * metap = & meta -> dad;
    }

    return rc;
}

/* OpenBlobRead
 *  opens an existing blob containing row data for id
 */
static
rc_t CC
KRColumnOpenBlobRead ( const KRColumn *self, const KColumnBlob **blobp, int64_t id )
{
    rc_t rc;
    const KColumnBlob *blob;

    if ( blobp == NULL )
        return RC ( rcDB, rcColumn, rcOpening, rcParam, rcNull );

    * blobp = NULL;

    rc = KRColumnBlobMake ( & blob, self -> idx . idx1 . bswap );
    if ( rc == 0 )
    {
        rc = KRColumnBlobOpenRead ( blob, self, id );
        if ( rc == 0 )
        {
            * blobp = (const KColumnBlob *) blob;
            return 0;
        }

        free ( (KColumnBlob *)blob );
    }

    return rc;
}


