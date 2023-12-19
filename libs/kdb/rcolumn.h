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

#include <kdb/column.h>

#include "rcoldata.h"
#include "rcolidx.h"

typedef struct KRColumn KRColumn;
#define KCOLUMN_IMPL KRColumn
#include "column-base.h"

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
struct KRColumn
{
    KColumn dad;

    struct KTable const *tbl;
    struct KDBManager const *mgr;
    struct KDirectory const *dir;

    KRColumnIdx idx;
    KRColumnData df;

    uint32_t csbytes;
    int32_t checksum;
    char path [ 1 ];
};

rc_t KRColumnMake ( KRColumn **colp, const KDirectory *dir, const char *path );
rc_t KRColumnMakeRead ( KRColumn **colp, const KDirectory *dir, const char *path );

#ifdef __cplusplus
}
#endif

