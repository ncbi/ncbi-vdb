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
#include "wcoldata-priv.h"
#include "wcolumn-priv.h"
#include <kfs/file.h>
#include <kfs/buffile.h>
#include <kfs/md5.h>
#include <kfs/impl.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


#define DATA_READ_FILE_BUFFER 256 * 1024


/*--------------------------------------------------------------------------
 * KColumnData
 */


/* Init
 */
static
rc_t KColumnDataInit ( KColumnData *self, uint64_t pos, size_t pgsize )
{
    rc_t rc = KFileSize ( self -> f, & self -> eof );
    if ( rc == 0 )
    {
        if ( self -> eof < pos || pgsize == 0 || ( pos % pgsize ) != 0 )
            rc = RC ( rcDB, rcColumn, rcConstructing, rcData, rcCorrupt );
        else
        {
            self -> eof = pos;
            self -> pgsize = pgsize;
            return 0;
        }
    }

    KFileRelease ( self -> f );
    self -> f = NULL;
    return rc;
}

/* Create
 */
rc_t KColumnDataCreate ( KColumnData *self, KDirectory *dir,
    KMD5SumFmt *md5, KCreateMode mode, uint64_t eof, size_t pgsize )
{
    rc_t rc = KColumnFileCreate ( & self -> f, & self -> fmd5, dir, md5, mode, true, "data" );
    if ( rc == 0 )
        rc = KColumnDataInit ( self, eof, pgsize );
    return rc;
}

/* Open
 */
rc_t KColumnDataOpenRead ( KColumnData *self,
    const KDirectory *dir, uint64_t eof, size_t pgsize )
{
    rc_t rc = KDirectoryOpenFileRead ( dir,
        ( const KFile** ) & self -> f, "data" );
#if DATA_READ_FILE_BUFFER
    if ( rc == 0 )
    {
        KFile * orig = self -> f;
        rc = KBufFileMakeRead ( ( const KFile** ) & self -> f, self -> f, DATA_READ_FILE_BUFFER );
        if ( rc == 0 )
	{
	    KFileRelease ( orig );
	}
	else
        {
            self -> f = orig;
            rc = 0;
        }
    }
#endif
    if ( rc == 0 )
        rc = KColumnDataInit ( self, eof, pgsize );
    return rc;
}

rc_t KColumnDataOpenUpdate ( KColumnData *self, KDirectory *dir,
    KMD5SumFmt *md5, uint64_t eof, size_t pgsize )
{
    rc_t rc = KColumnFileOpenUpdate ( & self -> f, & self -> fmd5, dir, md5, true, "data" );
    if ( rc == 0 )
        rc = KColumnDataInit ( self, eof, pgsize );
    return rc;
}

/* Whack
 */
rc_t KColumnDataWhack ( KColumnData *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
    {
        self -> f = NULL;
        self -> fmd5 = NULL;
    }
    return rc;
}

/* Read
 *  reads from the data fork using a blob map
 */
rc_t KColumnDataRead ( const KColumnData *self, const KColumnPageMap *pm,
    size_t offset, void *buffer, size_t bsize, size_t *num_read )
{
    uint64_t pos;

    assert ( self != NULL );
    assert ( pm != NULL );

    if ( bsize == 0 )
    {
        assert ( num_read != NULL );
        * num_read = 0;
        return 0;
    }

    pos = pm -> pg * self -> pgsize;
    return KFileRead ( self -> f, pos + offset, buffer, bsize, num_read );
}

/* Write
 *  writes to the data fork using a blob map
 */
rc_t KColumnDataWrite ( KColumnData *self, KColumnPageMap *pm,
    size_t offset, const void *buffer, size_t bytes, size_t *num_writ )
{
    uint64_t pos;

    assert ( self != NULL );
    assert ( pm != NULL );

    if ( bytes == 0 )
    {
        assert ( num_writ != NULL );
        * num_writ = 0;
        return 0;
    }

    pos = pm -> pg * self -> pgsize;
    return KFileWrite ( self -> f, pos + offset, buffer, bytes, num_writ );
}

/* Commit
 *  keeps changes indicated by page map and blob size
 */
rc_t KColumnDataCommit ( KColumnData *self,
    const KColumnPageMap *pm, size_t bytes )
{
    uint64_t pos;
    size_t remainder;

    assert ( self != NULL );
    assert ( pm != NULL );

    if ( self -> pgsize == 1 )
    {
        /* can only commit if extending page */
        if ( pm -> pg != self -> eof )
            return RC ( rcDB, rcBlob, rcCommitting, rcPagemap, rcInvalid );

        self -> eof += bytes;
        return 0;
    }

    assert ( ( self -> eof % self -> pgsize ) == 0 );
    pos = self -> eof / self -> pgsize;

    /* can only commit if extending page */
    if ( pm -> pg != pos )
        return RC ( rcDB, rcBlob, rcCommitting, rcPagemap, rcInvalid );

    /* advance eof */
    if ( bytes == 0 )
        return 0;

    pos = self -> eof + bytes;

    /* maintain page boundary */
    remainder = bytes % self -> pgsize;
    if ( remainder != 0 )
    {
        pos += self -> pgsize - remainder;
        assert ( ( pos % self -> pgsize ) == 0 );
    }

    self -> eof = pos;
    return 0;
}

rc_t KColumnDataCommitDone ( KColumnData * self )
{
    rc_t rc = 0;

    assert ( self != NULL );
    if ( self -> fmd5 != NULL )
    {
	assert ( self -> f == KMD5FileToKFile ( self -> fmd5 ) );

	rc = KMD5FileCommit ( self -> fmd5 );
    }
    return rc ;
}

/* Free
 *  frees pages from a map
 */
rc_t KColumnDataFree ( KColumnData *self,
    const KColumnPageMap *pm, size_t bytes )
{
    uint64_t pos;
    size_t remainder;

    if ( self -> pgsize == 1 )
    {
        if ( pm -> pg + bytes != self -> eof )
            return RC ( rcDB, rcBlob, rcReleasing, rcPagemap, rcInvalid );

        self -> eof = pm -> pg;
        return 0;
    }

    assert ( ( self -> eof % self -> pgsize ) == 0 );
    pos = pm -> pg * self -> pgsize;

    remainder = bytes % self -> pgsize;
    if ( pos + bytes + remainder != self -> eof )
        return RC ( rcDB, rcBlob, rcReleasing, rcPagemap, rcInvalid );

    self -> eof = pos;
    return 0;
}


/*--------------------------------------------------------------------------
 * KColumnPageMap
 *  map of pages involved in column blob
 */

/* Create
 *  creates a new page map using the first available page id
 *  obtains first free data fork page
 */
rc_t KColumnPageMapCreate (  KColumnPageMap *self, KColumnData *cd )
{
    assert ( cd != NULL );
    if ( ! cd -> f -> write_enabled )
        return RC ( rcDB, rcPagemap, rcConstructing, rcColumn, rcReadonly );

    assert ( self != NULL );
    assert ( ( cd -> eof % cd -> pgsize ) == 0 );
    self -> pg = cd -> eof / cd -> pgsize;

    return 0;
}

/* Open
 *  opens an blob by raw page id and size
 *
 *  "pm" [ OUT ] - modifiable parameter for blob page map
 *
 *  "pg" [ IN ] and "sz" [ IN ] - identifies pages of data fork included
 *  within the blob.
 */
rc_t KColumnPageMapOpen ( KColumnPageMap *self,
    KColumnData *cd, uint64_t pg, size_t sz )
{
    uint64_t pos;

    assert ( cd != NULL );
    pos = pg * cd -> pgsize;
    if ( pos + sz > cd -> eof )
    {
        if ( pos >= cd -> eof )
            return RC ( rcDB, rcColumn, rcOpening, rcParam, rcExcessive );
        return RC ( rcDB, rcColumn, rcOpening, rcRange, rcExcessive );
    }

    assert ( self != NULL );
    self -> pg = pg;

    return 0;
}

/* Whack
 *  disposes of memory in the case of a page array
 */
void KColumnPageMapWhack ( KColumnPageMap *self, const KColumnData *cd )
{
    assert ( self != NULL );
    assert ( cd != NULL );
}

/* Id
 *  captures id of initial page
 */
rc_t KColumnPageMapId ( const KColumnPageMap *self,
    const KColumnData *cd, uint64_t *pg )
{
    assert ( self != NULL );
    assert ( cd != NULL );
    assert ( pg != NULL );

    * pg = self -> pg;
    return 0;
}
