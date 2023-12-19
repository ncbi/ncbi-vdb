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

#pragma once

#include <kfs/directory.h>

#include "colfmt.h"

#include "rcolidx0.h"
#include "rcolidx1.h"
#include "rcolidx2.h"

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KRColumnIdx
 *  the index fork
 */
typedef struct KRColumnIdx KRColumnIdx;
struct KRColumnIdx
{
    /* first active id within db
       and first id on upper limit
       i.e. outside of db such that
       id_upper - id_first == num_ids */
    int64_t id_first;
    int64_t id_upper;

    /* level 0 index */
    KRColumnIdx0 idx0;

    /* level 1 index */
    KRColumnIdx1 idx1;

    /* level 2 index */
    KRColumnIdx2 idx2;
};

/* Open
 */
rc_t KRColumnIdxOpenRead ( KRColumnIdx *self, const KDirectory *dir,
    uint64_t *data_eof, size_t *pgsize, int32_t *checksum );

/* Whack
 */
rc_t KRColumnIdxWhack ( KRColumnIdx *self );

/* Version
 */
rc_t KRColumnIdxVersion ( const KRColumnIdx *self, uint32_t *version );
#define KRColumnIdxVersion( self, version ) \
    KRColumnIdx1Version ( & ( self ) -> idx1, version )

/* ByteOrder
 */
rc_t KRColumnIdxByteOrder ( const KRColumnIdx *self, bool *reversed );
#define KRColumnIdxByteOrder( self, reversed ) \
    KRColumnIdx1ByteOrder ( & ( self ) -> idx1, reversed )

/* IdRange
 *  returns range of ids contained within
 */
rc_t KRColumnIdxIdRange ( const KRColumnIdx *self,
    int64_t *first, int64_t *last );

/* FindFirstRowId
 */
rc_t KRColumnIdxFindFirstRowId ( const KRColumnIdx * self,
    int64_t * found, int64_t start );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KRColumnIdxLocateBlob ( const KRColumnIdx *self,
    KColBlobLoc *loc, int64_t first, int64_t last );


#ifdef __cplusplus
}
#endif

