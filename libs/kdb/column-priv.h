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

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifndef _h_coldata_priv_
#include "coldata-priv.h"
#endif

#ifndef _h_colidx_priv_
#include "colidx-priv.h"
#endif

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
 * KColumn
 */
struct KColumn
{
    struct KTable const *tbl;
    struct KDBManager const *mgr;
    struct KDirectory const *dir;

    KColumnIdx idx;
    KColumnData df;

    KRefcount refcount;
    uint32_t csbytes;
    int32_t checksum;
    char path [ 1 ];
};

/* Attach
 * Sever
 *  like Release, except called internally
 *  indicates that a child object is letting go...
 */
KColumn *KColumnAttach ( const KColumn *self );
rc_t KColumnSever ( const KColumn *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_column_priv_ */
