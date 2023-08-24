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

#ifndef _h_column_priv_
#define _h_column_priv_

#ifndef _h_kdb_column_
#include <kdb/column.h>
#endif

#ifndef _h_coldata_priv_
#include "coldata-priv.h"
#endif

#ifndef _h_colidx_priv_
#include "colidx-priv.h"
#endif

#define KCOLUMN_IMPL KColumn
#include "column-base.h"

#define KCOLUMNBLOB_IMPL KColumnBlob
#include "columnblob-base.h"

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KTable;
struct KDBManager;
struct KDirectory;


/*--------------------------------------------------------------------------
 * KColumn, read-side
 */
struct KColumn
{
    KColumnBase dad;

    struct KTable const *tbl;
    struct KDBManager const *mgr;
    struct KDirectory const *dir;

    KColumnIdx idx;
    KColumnData df;

    uint32_t csbytes;
    int32_t checksum;
    char path [ 1 ];
};

rc_t KColumnMake ( KColumn **colp, const KDirectory *dir, const char *path );

/*--------------------------------------------------------------------------
 * KColumnBlob
 *  one or more rows of column data
 */

struct KColumnBlob
{
    KColumnBlobBase dad;

    /* holds existing blob loc */
    KColBlobLoc loc;
    KColumnPageMap pmorig;

    /* owning column */
    const KColumn *col;

    /* captured from idx1 for CRC32 validation */
    bool bswap;
};

rc_t KColumnBlobMake ( KColumnBlob **blobp, bool bswap );

#ifdef __cplusplus
}
#endif

#endif /* _h_column_priv_ */
