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

#include "rcolidx.h"
#include "ridxblk.h"

#include <kfs/file.h>

#include <klib/rc.h>

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


/*--------------------------------------------------------------------------
 * KRColumnIdx
 *  the index fork
 */


/* EstablishIdRange
 */
static
void KRColumnIdxEstablishIdRange ( KRColumnIdx *self )
{
    int64_t first, upper;

    if ( ! KRColumnIdx0IdRange ( & self -> idx0, & self -> id_first, & self -> id_upper ) )
    {
        if ( ! KRColumnIdx1IdRange ( & self -> idx1, & self -> id_first, & self -> id_upper ) )
            self -> id_first = self -> id_upper = 1;
    }
    else if ( KRColumnIdx1IdRange ( & self -> idx1, & first, & upper ) )
    {
        if ( self -> id_first > first )
            self -> id_first = first;
        if ( self -> id_upper < upper )
            self -> id_upper = upper;
    }
}

/* Open
 */
rc_t KRColumnIdxOpenRead ( KRColumnIdx *self, const KDirectory *dir,
    uint64_t *data_eof, size_t *pgsize, int32_t *checksum )
{
    rc_t rc;
    uint64_t idx2_eof;
    uint32_t idx0_count;

    assert ( self != NULL );

    rc = KRColumnIdx1OpenRead ( & self -> idx1,
        dir, data_eof, & idx0_count, & idx2_eof, pgsize, checksum );
    if ( rc == 0 )
    {
        rc = ( self -> idx1 . vers < 3 ) ?
            KRColumnIdx0OpenRead_v1 ( & self -> idx0, dir, self -> idx1 . bswap ):
            KRColumnIdx0OpenRead ( & self -> idx0, dir, idx0_count, self -> idx1 . bswap );
        if ( rc == 0 )
        {
            rc = KRColumnIdx2OpenRead ( & self -> idx2, dir, idx2_eof );
            if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
            {
                KRColumnIdxEstablishIdRange ( self );
                return 0;
            }

            KRColumnIdx0Whack ( & self -> idx0 );
        }

        KRColumnIdx1Whack ( & self -> idx1 );
    }

    return rc;
}

/* Whack
 */
rc_t KRColumnIdxWhack ( KRColumnIdx *self )
{
    rc_t rc;

    assert ( self != NULL );

    rc = KRColumnIdx1Whack ( & self -> idx1 );
    if ( rc == 0 )
    {
        KRColumnIdx0Whack ( & self -> idx0 );
        KRColumnIdx2Whack ( & self -> idx2 );
    }

    return rc;
}

/* Version
 */
#ifndef KRColumnIdxVersion
rc_t KRColumnIdxVersion ( const KRColumnIdx *self, uint32_t *version )
{
    return KRColumnIdx1Version ( & self -> idx1, version );
}
#endif

/* IdRange
 *  returns range of ids contained within
 */
rc_t KRColumnIdxIdRange ( const KRColumnIdx *self,
    int64_t *first, int64_t *last )
{
    assert ( self != NULL );
    assert ( first != NULL );
    assert ( last != NULL );

    * first = self -> id_first;
    * last = self -> id_upper - 1;

    if ( self -> id_first == self -> id_upper )
        return RC ( rcDB, rcColumn, rcAccessing, rcRange, rcInvalid );
    return 0;
}

/* FindFirstRowId
 */
rc_t KRColumnIdxFindFirstRowId ( const KRColumnIdx * self,
    int64_t * found, int64_t start )
{
    rc_t rc0, rc1;
    KColBlockLoc bloc;
    int64_t best0, best1;

    assert ( self != NULL );
    assert ( found != NULL );

    /* global reject */
    if ( start < self -> id_first || start >= self -> id_upper )
        return RC ( rcDB, rcColumn, rcSelecting, rcRow, rcNotFound );

    /* look in idx0 */
    rc0 = KRColumnIdx0FindFirstRowId ( & self -> idx0, found, start );
    if ( rc0 == 0 )
    {
        if ( * found == start )
            return 0;

        best0 = * found;
        assert ( best0 > start );
    }

    /* look in main index */
    /* KRColumnIdx1LocateFirstRowIdBlob returns the blob containing 'start', if such blob exists, otherwise the next blob (or RC if there's no next) */
    rc1 = KRColumnIdx1LocateFirstRowIdBlob ( & self -> idx1, & bloc, start );
    if ( rc1 != 0 )
    {
        return rc0;
    }
    if ( start >= bloc . start_id )
    {   /* found inside a blob */
        best1 = start;
    }
    else
    {   /* not found; pick the start of the next blob */
        best1 = bloc . start_id;
    }

    if ( rc0 != 0 )
    {
        * found = best1;
        return 0;
    }

    /* found in both - return lesser */

    /* "found" already contains 'best0" */
    assert ( * found == best0 );
    if ( best1 < best0 )
        * found = best1;

    return 0;
}

/* LocateBlob
 *  locate an existing blob
 */
rc_t KRColumnIdxLocateBlob ( const KRColumnIdx *self,
    KColBlobLoc *loc, int64_t first, int64_t upper )
{
    rc_t rc;

    assert ( self != NULL );

    /* convert "last" to "upper" */
    if ( first >= ++ upper )
        return RC ( rcDB, rcColumn, rcSelecting, rcRange, rcInvalid );

    /* global reject */
    if ( first < self -> id_first || upper > self -> id_upper )
        return RC ( rcDB, rcColumn, rcSelecting, rcBlob, rcNotFound );

    /* look in idx0 */
    rc = KRColumnIdx0LocateBlob ( & self -> idx0, loc, first, upper );
    if ( GetRCState ( rc ) == rcNotFound )
    {
        KColBlockLoc bloc;

        /* find block containing range */
        rc = KRColumnIdx1LocateBlock ( & self -> idx1, & bloc, first, upper );
        if ( rc == 0 )
        {
            /* find location in idx2 */
            rc = KRColumnIdx2LocateBlob ( & self -> idx2,
                loc, & bloc, first, upper, self -> idx1 . bswap );
        }
    }

    return rc;
}
