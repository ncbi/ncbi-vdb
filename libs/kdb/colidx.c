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
#include "colidx-priv.h"
#include "idxblk-priv.h"
#include <kfs/file.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


/*--------------------------------------------------------------------------
 * KColumnIdx
 *  the index fork
 */


/* EstablishIdRange
 */
static
void KColumnIdxEstablishIdRange ( KColumnIdx *self )
{
    int64_t first, upper;

    if ( ! KColumnIdx0IdRange ( & self -> idx0, & self -> id_first, & self -> id_upper ) )
    {
        if ( ! KColumnIdx1IdRange ( & self -> idx1, & self -> id_first, & self -> id_upper ) )
            self -> id_first = self -> id_upper = 1;
    }
    else if ( KColumnIdx1IdRange ( & self -> idx1, & first, & upper ) )
    {
        if ( self -> id_first > first )
            self -> id_first = first;
        if ( self -> id_upper < upper )
            self -> id_upper = upper;
    }
}

/* Open
 */
rc_t KColumnIdxOpenRead ( KColumnIdx *self, const KDirectory *dir,
    uint64_t *data_eof, size_t *pgsize, int32_t *checksum )
{
    rc_t rc;
    uint64_t idx2_eof;
    uint32_t idx0_count;

    assert ( self != NULL );

    rc = KColumnIdx1OpenRead ( & self -> idx1,
        dir, data_eof, & idx0_count, & idx2_eof, pgsize, checksum );
    if ( rc == 0 )
    {
        rc = ( self -> idx1 . vers < 3 ) ?
            KColumnIdx0OpenRead_v1 ( & self -> idx0, dir, self -> idx1 . bswap ):
            KColumnIdx0OpenRead ( & self -> idx0, dir, idx0_count, self -> idx1 . bswap );
        if ( rc == 0 )
        {
            rc = KColumnIdx2OpenRead ( & self -> idx2, dir, idx2_eof );
            if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
            {
                KColumnIdxEstablishIdRange ( self );
                return 0;
            }

            KColumnIdx0Whack ( & self -> idx0 );
        }

        KColumnIdx1Whack ( & self -> idx1 );
    }

    return 0;
}

/* Whack
 */
rc_t KColumnIdxWhack ( KColumnIdx *self )
{
    rc_t rc;

    assert ( self != NULL );

    rc = KColumnIdx1Whack ( & self -> idx1 );
    if ( rc == 0 )
    {
        KColumnIdx0Whack ( & self -> idx0 );
        KColumnIdx2Whack ( & self -> idx2 );
    }

    return rc;
}

/* Version
 */
#ifndef KColumnIdxVersion
rc_t KColumnIdxVersion ( const KColumnIdx *self, uint32_t *version )
{
    return KColumnIdx1Version ( & self -> idx1, version );
}
#endif

/* IdRange
 *  returns range of ids contained within
 */
rc_t KColumnIdxIdRange ( const KColumnIdx *self,
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

/* LocateBlob
 *  locate an existing blob
 */
rc_t KColumnIdxLocateBlob ( const KColumnIdx *self,
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
    rc = KColumnIdx0LocateBlob ( & self -> idx0, loc, first, upper );
    if ( GetRCState ( rc ) == rcNotFound )
    {
        KColBlockLoc bloc;

        /* find block containing range */
        rc = KColumnIdx1LocateBlock ( & self -> idx1, & bloc, first, upper );
        if ( rc == 0 )
        {
            /* find location in idx2 */
            rc = KColumnIdx2LocateBlob ( & self -> idx2,
                loc, & bloc, first, upper, self -> idx1 . bswap );
        }
    }

    return rc;
}
