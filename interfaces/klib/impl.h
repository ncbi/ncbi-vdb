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

#ifndef _h_klib_impl_
#define _h_klib_impl_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_namelist_
#include <klib/namelist.h>
#endif

#ifndef _h_kfc_refcount_impl_
#include <kfc/refcount-impl.h>
#endif

#ifndef _h_atomic_
#include <atomic.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KBTreePager_v1;
typedef union KNamelist_vt KNamelist_vt;


/*--------------------------------------------------------------------------
 * KNamelist
 *  a virtual directory listing
 */
struct KNamelist
{
    const KNamelist_vt *vt;
    atomic_t refcount;
};

#ifndef KNAMELIST_IMPL
#define KNAMELIST_IMPL KNamelist
#endif

typedef struct KNamelist_vt_v1 KNamelist_vt_v1;
struct KNamelist_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KNAMELIST_IMPL *self );
    rc_t ( CC * count ) ( const KNAMELIST_IMPL *self, uint32_t *count );
    rc_t ( CC * get ) ( const KNAMELIST_IMPL *self, uint32_t idx, const char **name );
    /* end minor version == 0 */
};

union KNamelist_vt
{
    KNamelist_vt_v1 v1;
};

/* Init
 */
KLIB_EXTERN rc_t CC KNamelistInit ( KNamelist *self, const KNamelist_vt *vt );


/*--------------------------------------------------------------------------
 * KBTreePager
 *  a page storage interface
 */
struct KBTreePager_v1
{
    KRefcount_v1 dad;
};

extern KITFTOK_DECL ( KBTreePager_v1 );

#ifndef KBTREEPAGER_V1
#define KBTREEPAGER_V1 struct KBTreePager_v1
#endif

typedef struct KBTreePager_vt_v1 KBTreePager_vt_v1;
struct KBTreePager_vt_v1
{
    KVTable dad;

    /* start minor version == 0 */
    size_t ( CC * page_size ) ( const KBTREEPAGER_V1 * self, ctx_t ctx );
    KBTreePage_v1 * ( CC * alloc_page ) ( KBTREEPAGER_V1 * self, ctx_t ctx );
    const KBTreePage_v1 * ( CC * get_page_read ) ( const KBTREEPAGER_V1 * self, ctx_t ctx, uint64_t pgid );
    KBTreePage_v1 * ( CC * get_page_update ) ( KBTREEPAGER_V1 * self, ctx_t ctx, uint64_t pgid );
    /* end minor version == 0 */
};

/* Init
 */
KLIB_EXTERN void CC KBTreePagerInit_v1 ( KBTreePager_v1 * self, ctx_t ctx,
    const KVTable * vt, const char * instance_name );

/* Destroy
 */
KLIB_EXTERN void CC KBTreePagerDestroy_v1 ( KBTreePager_v1 * self, ctx_t ctx );

#ifdef __cplusplus
}
#endif

#endif /* _h_klib_impl_ */
