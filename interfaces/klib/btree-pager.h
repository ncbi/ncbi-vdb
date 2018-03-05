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

#ifndef _h_klib_b_tree_pager_
#define _h_klib_b_tree_pager_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_kfc_refcount_
#include <kfc/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * defines
 */
#ifndef KBTREEPAGER_VERS
#define KBTREEPAGER_VERS 1
#endif

#if KBTREEPAGER_VERS == 1
#define KBTreePage KBTreePage_v1
#define KBTreePager KBTreePager_v1
#endif

/*--------------------------------------------------------------------------
 * KBTreePage_v1
 *  a simple handle to a page of in-core memory
 */
typedef struct KBTreePage_v1 KBTreePage_v1;


/* Duplicate
 * Release
 *  ignores NULL references
 *  releasing a page may cause its memory to be expelled from cache
 *  releasing a modified page may cause its memory to be written to backing
 */
static __inline__
KBTreePage_v1 * KBTreePageDuplicate_v1 ( const KBTreePage_v1 * self, ctx_t ctx, caps_t rm )
{
    return ( KBTreePage_v1 * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KBTreePageRelease_v1 ( const KBTreePage_v1 * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}


/* Size
 *  ask page its size
 */
KLIB_EXTERN size_t CC KBTreePageSize_v1 ( const KBTreePage_v1 * self, ctx_t ctx );


/* AccessRead
 * AccessUpdate
 *  ask page for its memory
 *  both of them access the same memory, but asking for updatable memory
 *  marks the page as modified, causing it to be flushed out to storage
 */
KLIB_EXTERN const void * CC KBTreePageAccessRead_v1 ( const KBTreePage_v1 * self, ctx_t ctx );
KLIB_EXTERN void * CC KBTreePageAccessUpdate_v1 ( KBTreePage_v1 * self, ctx_t ctx );



/*--------------------------------------------------------------------------
 * KBTreePager_v1
 *  interface to control requests for paged storage
 *
 *  to discover the page size that is used by a pager, call KBTreePagerPageSize()
 *
 *  to allocate a new page, call KBTreePagerAllocPage()
 *  it returns a KBTreePage.
 *
 *  to retrieve an existing page by id,
 *  call KBTreePagerGetPageRead() or KBTreePagerGetPageUpdate().
 *  these return a KBTreePage if found.
 *
 *  to discover the size of a KBTreePage, call KBTreePageSize()
 *
 *  to obtain a pointer to addressable page memory,
 *  call KBTreePageAccessRead() or KBTreePageAccessUpdate().
 *  the latter may fail if the page is not writable.
 *
 *  to release a page so that it may be updated to storage
 *  and purged from addressable memory, call KBTreePageRelease()
 */
typedef struct KBTreePager_v1 KBTreePager_v1;

/* Duplicate
 * Release
 *  ignores NULL references
 */
static __inline__
KBTreePager_v1 * KBTreePagerDuplicate_v1 ( const KBTreePager_v1 * self, ctx_t ctx, caps_t rm )
{
    return ( KBTreePager_v1 * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KBTreePagerRelease_v1 ( const KBTreePager_v1 * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}


/* PageSize
 *  ask pager the size of its pages
 */
KLIB_EXTERN size_t CC KBTreePagerPageSize_v1 ( const KBTreePager_v1 * self, ctx_t ctx );


/* AllocPage
 *  ask pager to allocate a page and return the page object
 *  caller is responsible for release
 *  page is locked in addressable memory space until released
 */
KLIB_EXTERN KBTreePage_v1 * CC KBTreePagerAllocPage_v1 ( KBTreePager_v1 * self, ctx_t ctx );


/* GetPageRead
 * GetPageUpdate
 *  ask pager to return a page by id
 *  caller is responsible for release
 *  page is locked in addressable memory space until released
 */
KLIB_EXTERN const KBTreePage_v1 * CC KBTreePagerGetPageRead_v1 ( const KBTreePager_v1 * self, ctx_t ctx, uint64_t pgid );
KLIB_EXTERN KBTreePage_v1 * CC KBTreePagerGetPageUpdate_v1 ( KBTreePager_v1 * self, ctx_t ctx, uint64_t pgid );


/*--------------------------------------------------------------------------
 * KBTreePage
 * KBTreePager
 */
#define KBTreePageDuplicate NAME_VERS ( KBTreePageDuplicate, KBTREEPAGER_VERS )
#define KBTreePageRelease NAME_VERS ( KBTreePageRelease, KBTREEPAGER_VERS )
#define KBTreePageSize NAME_VERS ( KBTreePageSize, KBTREEPAGER_VERS )
#define KBTreePageAccessRead NAME_VERS ( KBTreePageAccessRead, KBTREEPAGER_VERS )
#define KBTreePageAccessUpdate NAME_VERS ( KBTreePageAccessUpdate, KBTREEPAGER_VERS )

#define KBTreePagerDuplicate NAME_VERS ( KBTreePagerDuplicate, KBTREEPAGER_VERS )
#define KBTreePagerRelease NAME_VERS ( KBTreePagerRelease, KBTREEPAGER_VERS )
#define KBTreePagerPageSize NAME_VERS ( KBTreePagerPageSize, KBTREEPAGER_VERS )
#define KBTreePagerAllocPage NAME_VERS ( KBTreePagerAllocPage, KBTREEPAGER_VERS )
#define KBTreePagerGetPageRead NAME_VERS ( KBTreePagerGetPageRead, KBTREEPAGER_VERS )
#define KBTreePagerGetPageUpdate NAME_VERS ( KBTreePagerGetPageUpdate, KBTREEPAGER_VERS )

#ifdef __cplusplus
}
#endif

#endif /*  _h_klib_b_tree_pager_ */
