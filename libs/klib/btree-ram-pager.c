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

struct KBTreeRAMPager;
#define KREFCOUNT_V1 struct KBTreeRAMPager
#define KBTREEPAGER_V1 struct KBTreeRAMPager

#include <klib/extern.h>
#include <klib/impl.h>
#include <klib/b+tree.h>


/*--------------------------------------------------------------------------
 * KBTreeRAMPager
 */

typedef struct KBTreeRAMPager KBTreeRAMPager;
struct KBTreeRAMPager
{
    KRefcount_v1 dad;
};

static
void CC KBTreeRAMPagerWhack ( KBTreeRAMPager * self, ctx_t ctx )
{
}

static KRefcount_v1_vt KBTreeRAMPager_KRefcount_vt =
{
    KVTABLE_INITIALIZER ( KBTreeRAMPager, KRefcount_v1, 0, NULL ),
    .destroy = KBTreeRAMPagerWhack
};

/* PageSize
 *  ask pager the size of its pages
 */
static
size_t CC KBTreeRAMPagerPageSize ( const KBTreeRAMPager * self, ctx_t ctx )
{
}


/* AllocPage
 *  ask pager to allocate a page and return the page object
 *  caller is responsible for release
 *  page is locked in addressable memory space until released
 */
static
KBTreePage_v1 * CC KBTreeRAMPagerAllocPage ( KBTreeRAMPager * self, ctx_t ctx )
{
}


/* GetPageRead
 * GetPageUpdate
 *  ask pager to return a page by id
 *  caller is responsible for release
 *  page is locked in addressable memory space until released
 */
static
const KBTreePage_v1 * CC KBTreeRAMPagerGetPageRead ( const KBTreeRAMPager * self, ctx_t ctx, uint64_t pgid )
{
}

static
KBTreePage_v1 * CC KBTreeRAMPagerGetPageUpdate ( KBTreeRAMPager * self, ctx_t ctx, uint64_t pgid )
{
}

static KBTreePager_v1_vt KBTreeRAMPager_KBTreePager_vt =
{
    KVTABLE_INITIALIZER ( KBTreeRAMPager, KBTreePager_v1, 0, KBTreeRAMPager_KRefcount_vt ),

    .page_size       = KBTreeRAMPagerPageSize,
    .alloc_page      = KBTreeRAMPagerAllocPage,
    .get_page_read   = KBTreeRAMPagerGetPageRead,
    .get_page_update = KBTreeRAMPagerGetPageUpdate
};
