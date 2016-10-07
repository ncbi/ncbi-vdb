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

/* we do not want to expand the RC codes now */
#define rcRowSet rcVector
\

/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KRowSet_v1_vt KRowSet_v1_vt;
typedef struct KRowSetIterator_v1_vt KRowSetIterator_v1_vt;

extern KITFTOK_DECL ( KRowSet );
extern KITFTOK_DECL ( KRowSetIterator );

/*--------------------------------------------------------------------------
 * KRowSet
 */
struct KRowSet
{
    KRefcount_v1 dad;
    const struct KTable * table;

    /* "morphable" state */
    KROWSET_DATA * data;
};


/*--------------------------------------------------------------------------
 * KRowSet_v1_vt
 */
struct KRowSet_v1_vt
{
    KVTable dad;

    /* start minor version == 0 */
    void ( CC * destroy_data ) ( KROWSET_DATA * self, ctx_t ctx );
    /* probably need a serialization function */
    void ( CC * add_row_id_range ) ( struct KROWSET * self, ctx_t ctx, int64_t start_row_id, uint64_t count );
    uint64_t ( CC * get_num_rows ) ( const struct KROWSET * self, ctx_t ctx );
    bool ( CC * has_row_id ) ( const struct KROWSET * self, ctx_t ctx, int64_t row_id );
    struct KROWSET_IT * ( CC * get_iterator ) ( const struct KROWSET * self, ctx_t ctx );
    /* end minor version == 0 */
};

/* Init
 *  initialize a newly allocated RowSet object
 */
KDB_EXTERN void CC KRowSetInit ( struct KRowSet *self, ctx_t ctx, const KVTable *kvt,
    const char *classname );

/* MakeRowSet
 *  create specific instance of a rowset
 */
KDB_EXTERN KRowSet * CC KTableMakeRowSetSimple ( struct KTable const * self, ctx_t ctx );

/*--------------------------------------------------------------------------
 * KRowSetIterator
 */
struct KRowSetIterator
{
    KRefcount_v1 dad;
};

/*--------------------------------------------------------------------------
 * KRowSetIterator_v1_vt
 */
struct KRowSetIterator_v1_vt
{
    KVTable dad;

    /* start minor version == 0 */
    bool ( CC * next ) ( struct KROWSET_IT * self, ctx_t ctx );
    bool ( CC * is_valid ) ( const struct KROWSET_IT * self );
    int64_t ( CC * get_row_id ) ( const struct KROWSET_IT * self, ctx_t ctx );
    /* end minor version == 0 */
};

/* Init
 *  initialize a newly allocated RowSet object
 */
KDB_EXTERN void CC KRowSetIteratorInit ( struct KRowSetIterator *self, ctx_t ctx, const KVTable *kvt,
    const char *classname );


#ifdef __cplusplus
}
#endif

#endif /* _h_kdb_rowset_impl_ */
