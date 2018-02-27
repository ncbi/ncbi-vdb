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

#ifndef _h_klib_btree_priv_
#define _h_klib_btree_priv_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_kfc_refcount_impl_
#include <kfc/refcount-impl.h>
#endif

#ifndef _h_klib_b_plus_tree_
#include <klib/b+tree.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * KBTreePage_v1
 */
struct KBTreePage_v1
{
    KRefcount_v1 dad;
    KBTreePager_v1 * pager;
    void * mem;
    uint32_t size;
    bool dirty;
};

/*--------------------------------------------------------------------------
 * KBTreeValue_v1
 */
struct KBTreeValue_v1
{
    KRefcount_v1 dad;
    Vector pages;
    size_t size;
    bool read_only;
};

/*--------------------------------------------------------------------------
 * KBPlusTree_v1
 */
struct KBPlusTree_v1
{
    KRefcount_v1 dad;
    KBTreePager_v1 * pager;
};


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_btree_priv_ */
