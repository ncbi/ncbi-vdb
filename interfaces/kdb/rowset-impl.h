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
#define KROWSET KRowSet
#endif

#ifndef KROWSET_IT
#define KROWSET_IT KRowSetIterator
#endif

#ifndef KROWSET_DATA
#define KROWSET_DATA void
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KRowSet_vt KRowSet_vt;
typedef struct KRowSetIterator_vt KRowSetIterator_vt;

/*--------------------------------------------------------------------------
 * KRowSet
 */
struct KRowSet
{
    const KRowSet_vt *vt;
    KRefcount refcount;

    /* "morphable" state */
    KROWSET_DATA * data;
};


/*--------------------------------------------------------------------------
 * KRowSet_vt
 */
struct KRowSet_vt
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    void ( CC * destroy_data ) ( KROWSET_DATA * self, ctx_t ctx );
    /* probably need a serialization function */
    void ( CC * add_row_id_range ) ( struct KROWSET * self, ctx_t ctx, int64_t start_row_id, uint64_t count );
    uint64_t ( CC * get_num_rows ) ( const struct KROWSET * self, ctx_t ctx );
    bool ( CC * has_row_id ) ( const struct KROWSET * self, ctx_t ctx, int64_t row_id );
    struct KROWSET_IT * ( CC * get_iterator ) ( const struct KROWSET * self, ctx_t ctx );
    // TODO: add checks for all fn to KRowSetInit
    /* end minor version == 0 */
};

/* Init
 *  initialize a newly allocated RowSet object
 */
KDB_EXTERN void CC KRowSetInit ( struct KRowSet *self, ctx_t ctx, const KRowSet_vt *vt,
    const char *classname, const char *name );

/* MakeRowSet
 *  create specific instance of a rowset
 */
KDB_EXTERN KRowSet * CC KTableMakeRowSetSimple ( struct KTable const * self, ctx_t ctx );

/*--------------------------------------------------------------------------
 * KRowSetIterator
 */
struct KRowSetIterator
{
    const KRowSetIterator_vt *vt;
    KRefcount refcount;
};

/*--------------------------------------------------------------------------
 * KRowSetIterator_vt
 */
struct KRowSetIterator_vt
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    void ( CC * destroy ) ( struct KROWSET_IT * self, ctx_t ctx );
    void ( CC * next ) ( struct KROWSET_IT * self, ctx_t ctx );
    bool ( CC * is_valid ) ( const struct KROWSET_IT * self );
    int64_t ( CC * get_row_id ) ( const struct KROWSET_IT * self, ctx_t ctx );
    // TODO: add checks for all fn to KRowSetIteratorInit
    /* end minor version == 0 */
};

/* Init
 *  initialize a newly allocated RowSet object
 */
KDB_EXTERN void CC KRowSetIteratorInit ( struct KRowSetIterator *self, ctx_t ctx, const KRowSetIterator_vt *vt,
    const char *classname, const char *name );


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_impl_ */
