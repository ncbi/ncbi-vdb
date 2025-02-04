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

#include <klib/container.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KRColumnIdx0
 *  level 0 index - event journaling
 */
typedef struct KRColumnIdx0 KRColumnIdx0;
struct KRColumnIdx0
{
    /* the in-core indices */
    BSTree bst;
    size_t count;
};


/* Open
 */
rc_t KRColumnIdx0OpenRead_v1 ( KRColumnIdx0 *self,
    const KDirectory *dir, bool bswap );
rc_t KRColumnIdx0OpenRead ( KRColumnIdx0 *self,
    const KDirectory *dir, uint32_t count, bool bswap );

/* Whack
 */
void KRColumnIdx0Whack ( KRColumnIdx0 *self );

/* IdRange
 *  returns range of ids contained within
 */
bool KRColumnIdx0IdRange ( const KRColumnIdx0 *self,
    int64_t *first, int64_t *upper );

/* FindFirstRowId
 */
rc_t KRColumnIdx0FindFirstRowId ( const KRColumnIdx0 * self,
    int64_t * found, int64_t start );

/* LocateBlob
 *  locate an existing blob
 */
rc_t KRColumnIdx0LocateBlob ( const KRColumnIdx0 *self,
    KColBlobLoc *loc, int64_t first, int64_t upper );


#ifdef __cplusplus
}
#endif

