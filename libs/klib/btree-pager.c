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

#include <klib/extern.h>
#include <klib/impl.h>
#include <klib/b+tree.h>


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

/* PageSize
 *  ask pager the size of its pages
 */
LIB_EXPORT size_t CC KBTreePagerPageSize_v1 ( const KBTreePager_v1 * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcRuntime, rcTree, rcAccessing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "pager reference" );
    else
    {
        const KBTreePager_v1_vt * vt;

        vt = KVTABLE_CAST ( self -> dad . vt, ctx, KBTreePager_v1 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KBTreePager_v1 interface" );
        else
        {
            return ( * vt -> page_size ) ( self, ctx );
        }
    }

    return 0;
}


/* AllocPage
 *  ask pager to allocate a page and return the page object
 *  caller is responsible for release
 *  page is locked in addressable memory space until released
 */
LIB_EXPORT KBTreePage_v1 * CC KBTreePagerAllocPage_v1 ( KBTreePager_v1 * self, ctx_t ctx )
{
}


/* GetPageRead
 * GetPageUpdate
 *  ask pager to return a page by id
 *  caller is responsible for release
 *  page is locked in addressable memory space until released
 */
LIB_EXPORT const KBTreePage_v1 * CC KBTreePagerGetPageRead_v1 ( const KBTreePager_v1 * self, ctx_t ctx, uint64_t pgid )
{
}

LIB_EXPORT KBTreePage_v1 * CC KBTreePagerGetPageUpdate_v1 ( KBTreePager_v1 * self, ctx_t ctx, uint64_t pgid )
{
}
