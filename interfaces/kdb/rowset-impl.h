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

#ifndef _h_kdb_rowset_impl_
#define _h_kdb_rowset_impl_

#ifndef _h_kdb_extern_
#include <kdb/extern.h>
#endif

#ifndef _h_kfc_refcount_impl_
#include <kfc/refcount-impl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KROWSET
#define KROWSET struct KRowSet
#endif

#ifndef KROWSET_DATA
#define KROWSET_DATA void
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KRowSet;
typedef union KRowSet_vt KRowSet_vt;


/*--------------------------------------------------------------------------
 * KRowSet
 */
struct KRowSet_v1
{
    KRefcount_v1 dad;

    /* possible shared state */

    /* "morphable" state */
    KROWSET_DATA * data;
};


/*--------------------------------------------------------------------------
 * KRowSet_vt_v1
 */
typedef struct KRowSet_vt_v1 KRowSet_vt_v1;
struct KRowSet_vt_v1
{
    KRefcount_v1_vt dad;

    /* start minor version == 0 */
    void ( CC * destroy_data ) ( KROWSET * self, ctx_t ctx );
    /* probably need a serialization function */
    void ( CC * add_row_id ) ( KROWSET * self, ctx_t ctx, int64_t row_id );
    void ( CC * add_row_id_range ) ( KROWSET * self, ctx_t ctx, int64_t row_id, uint64_t count );
    /* end minor version == 0 */
};


/*--------------------------------------------------------------------------
 * KRowSet_vt
 */
union KRowSet_vt
{
    KRowSet_vt_v1 v1;
};


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_impl_ */
