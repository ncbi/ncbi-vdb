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

#include "rcoldata.h"

#include <kfs/file.h>
#include <kfs/buffile.h>
#include <kfs/impl.h>

#include <klib/rc.h>

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


#define DATA_READ_FILE_BUFFER 0


/*--------------------------------------------------------------------------
 * KRColumnData
 */


/* Init
 */
static
rc_t KRColumnDataInit ( KRColumnData *self, uint64_t pos, size_t pgsize )
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

/* Open
 */
rc_t KRColumnDataOpenRead ( KRColumnData *self,
    const KDirectory *dir, uint64_t eof, size_t pgsize )
{
    rc_t rc = KDirectoryOpenFileRead ( dir,
        & self -> f, "data" );
#if DATA_READ_FILE_BUFFER
    if ( rc == 0 )
    {
        const KFile * orig = self -> f;
        rc = KBufFileMakeRead ( & self -> f, self -> f, DATA_READ_FILE_BUFFER );
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
        rc = KRColumnDataInit ( self, eof, pgsize );
    return rc;
}

/* Whack
 */
rc_t KRColumnDataWhack ( KRColumnData *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
        self -> f = NULL;
    return rc;
}

/*--------------------------------------------------------------------------
 * KRColumnPageMap
 *  map of pages involved in column blob
 */

/* Open
 *  opens an blob by raw page id and size
 *
 *  "pm" [ OUT ] - modifiable parameter for blob page map
 *
 *  "pg" [ IN ] and "sz" [ IN ] - identifies pages of data fork included
 *  within the blob.
 */
rc_t KRColumnPageMapOpen ( KRColumnPageMap *self,
    KRColumnData *cd, uint64_t pg, size_t sz )
{
    uint64_t pos;

    assert ( cd != NULL );
    pos = pg * cd -> pgsize;
    if ( pos + sz > cd -> eof )
    {
        if ( pos >= cd -> eof )
            return RC ( rcDB, rcBlob, rcOpening, rcPagemap, rcExcessive );
        return RC ( rcDB, rcBlob, rcOpening, rcRange, rcExcessive );
    }

    assert ( self != NULL );
    self -> pg = pg;

    return 0;
}

/* Id
 *  captures id of initial page
 */
rc_t KRColumnPageMapId ( const KRColumnPageMap *self,
    const KRColumnData *cd, uint64_t *pg )
{
    assert ( self != NULL );
    assert ( cd != NULL );
    assert ( pg != NULL );

    * pg = self -> pg;
    return 0;
}
