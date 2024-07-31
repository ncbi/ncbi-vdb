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

#include "coldata.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
typedef union KRColumnPageMap KRColumnPageMap;


/*--------------------------------------------------------------------------
 * KRColumnData
 *  data fork
 *
 *  only handling append-mode today
 *
 *  kept 64-bit aligned
 */
typedef struct KRColumnData KRColumnData;
struct KRColumnData
{
    /* cached end of data fork */
    uint64_t eof;

    /* data fork itself */
    struct KFile const *f;

    /* page size */
    size_t pgsize;
};

/* DefaultPageSize
 *  static method
 */
#define KRColumnDataDefaultPageSize( reuse_pages ) \
    ( ( reuse_pages ) ? 4096 : 1 )

/* Open
 */
rc_t KRColumnDataOpenRead ( KRColumnData *self,
    const KDirectory *dir, uint64_t eof, size_t pgsize );

/* Whack
 */
rc_t KRColumnDataWhack ( KRColumnData *self );

/*--------------------------------------------------------------------------
 * KRColumnPageMap
 *  map of pages involved in column blob
 */
union KRColumnPageMap
{
    /* for non-paged data forks, a single page id
       describes the start of the blob, where the
       remainder is sequential */
    uint64_t pg;
};

/* Open
 *  opens a page map by raw page id and size
 *
 *  "pm" [ OUT ] - modifiable parameter for blob page map
 *
 *  "pg" [ IN ] and "sz" [ IN ] - identifies pages of data fork included
 *  within the blob.
 */
rc_t KRColumnPageMapOpen ( KRColumnPageMap *pm,
    KRColumnData *cd, uint64_t pg, size_t sz );

/* Id
 *  captures id of initial page
 */
rc_t KRColumnPageMapId ( const KRColumnPageMap *self,
    const KRColumnData *cd, uint64_t *pg );


#ifdef __cplusplus
}
#endif

