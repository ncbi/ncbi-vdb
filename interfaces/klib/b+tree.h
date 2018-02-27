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

#ifndef _h_klib_b_plus_tree_
#define _h_klib_b_plus_tree_

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
extern "C" {}
#endif


/*--------------------------------------------------------------------------
 * defines
 */
#ifndef KBPLUSTREE_VERS
#define KBPLUSTREE_VERS 1
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
typedef struct KBTreePager_v1 KBTreePager_v1;
typedef struct KBTreeValue_v1 KBTreeValue_v1;


/*--------------------------------------------------------------------------
 * KBPlusTree_v1
 *  a generic B+Tree implementation
 *  mapping arbitrary keys to arbitrary values
 *
 *  keys are "canonicalized" so that they can be treated as
 *  a sequence of natural numbers < 256, i.e. an array of bytes.
 *
 *  values are opaque, and can be any size
 *  supported by the underlying system
 *
 *  a new, empty B+Tree is created by KBPlusTreeMakeUpdate().
 *  an existing B+Tree is made by KBPlusTreeMakeRead() or KBPlusTreeMakeUpdate().
 *
 *  to insert a mapping, use KBPlusTreeInsert(). when the key is not unique,
 *  a handle to the existing value is returned rather than an exception.
 *  the handle is a KBTreeValue and can be used to read or modify the prior value.
 *
 *  to select/retrieve a value with a given key, use KBPlusTreeFind()
 *  a read-only handle to the value is returned if found.
 *
 *  to update a value, use KBPlusTreeUpdate()
 *  a read/write handle to the value is returned if found.
 *
 *  to delete a mapping, use KBPlusTreeDelete()
 *
 *  to visit all mappings, use KBPlusTreeForEach() or KBPlusTreeDoUntil()
 */
typedef struct KBPlusTree_v1 KBPlusTree_v1;

#if KBPLUSTREE_VERS == 1
#define KBPlusTree KBPlusTree_v1
#endif


/* Duplicate
 * Release
 *  ignores NULL references
 */
static __inline__
KBPlusTree_v1 * KBPlusTreeDuplicate_v1 ( const KBPlusTree_v1 * self, ctx_t ctx, caps_t rm )
{
    return ( KBPlusTree_v1 * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KBPlusTreeRelease_v1 ( const KBPlusTree_v1 * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}


/* MakeRead
 *  create a b+tree for read-only access
 *
 *  "pager" [ IN ] - a pager interface for accessing pages in storage.
 */
KLIB_EXTERN const KBPlusTree_v1 * CC KBPlusTreeMakeRead_v1 ( ctx_t ctx, const KBTreePager_v1 * pager );


/* MakeUpdate
 *  create a b+tree for read/write access
 *
 *  "opt_pager" [ IN, NULL OKAY ] - a pager interface for accessing, creating, deleting
 *   pages in storage. default is to create a new, empty, RAM-based storage.
 *   required if passing in an existing tree.
 */
KLIB_EXTERN KBPlusTree_v1 * CC KBPlusTreeMakeUpdate_v1 ( ctx_t ctx, KBTreePager_v1 * opt_pager );


/* Insert
 *  searches for a match or creates a new entry
 *  key must be canonicalized, meaning that key bytes 0..N
 *  are comparable and lexically ordered as natural integer codes
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key treated as a sequence of natural integers
 *
 *  "value" [ IN ] and "value_size" [ IN ] - describes an
 *   opaque value recorded as a sequence of bytes
 *
 *  returns NULL if key did not exist prior to insertion,
 *  or a KBTreeValue * otherwise.
 *  caller is responsible for releasing return value.
 */
KLIB_EXTERN KBTreeValue_v1 * CC KBPlusTreeInsert_v1 ( KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size, const void * value, size_t value_size );


/* Find
 *  searches for a key match
 *  key must be canonicalized, meaning that key bytes 0..N
 *  are comparable and lexically ordered as natural integer codes
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key treated as a sequence of natural integers
 *
 *  returns NULL if key not found, or a KBTreeValue * otherwise.
 *  caller is responsible for releasing return value.
 */
KLIB_EXTERN const KBTreeValue_v1 * CC KBPlusTreeFind_v1 ( const KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size );


/* Update
 *  searches for a key match
 *  key must be canonicalized, meaning that key bytes 0..N
 *  are comparable and lexically ordered as natural integer codes
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key treated as a sequence of natural integers
 *
 *  returns NULL if key not found, or a KBTreeValue * otherwise.
 *  caller is responsible for releasing return value.
 */
KLIB_EXTERN KBTreeValue_v1 * CC KBPlusTreeUpdate_v1 ( KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size );


/* Delete
 *  deletes matching entry
 *  key must be canonicalized, meaning that key bytes 0..N
 *  are comparable and lexically ordered as natural integer codes
 *
 *  "key" [ IN ] and "key_size" [ IN ] - describes an
 *   opaque key treated as a sequence of natural integers
 *
 *  returns "true" if entry was found ( and deleted )
 */
KLIB_EXTERN bool CC KBPlusTreeDelete_v1 ( KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size );


/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */

KLIB_EXTERN void CC KBPlusTreeForEach_v1 ( KBPlusTree_v1 * self, ctx_t ctx, bool reverse,
    void ( CC * f ) ( KBPlusTree_v1 * bt, ctx_t ctx, const void * key, size_t key_size,
        const void * value, size_t value_size, void * data ), void * data );


/* DoUntil
 *  executes a function on each tree element until "f" returns "true"
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 *
 *  returns "true" if "f" returned true, "false" otherwise
 */

KLIB_EXTERN bool CC KBPlusTreeDoUntil_v1 ( KBPlusTree_v1 * self, ctx_t ctx, bool reverse,
    bool ( CC * f ) ( KBPlusTree_v1 * bt, ctx_t ctx, const void * key, size_t key_size,
        const void * value, size_t value_size, void * data ), void * data );


/*--------------------------------------------------------------------------
 * KBTreePage_v1
 * KBTreePager_v1
 *  interfaces to control requests for paged storage
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
typedef struct KBTreePage_v1 KBTreePage_v1;

#ifndef KBTREEPAGER_VERS
#define KBTREEPAGER_VERS 1
#endif

#if KBTREEPAGER_VERS == 1
#define KBTreePage KBTreePage_v1
#define KBTreePager KBTreePager_v1
#endif


/* Duplicate
 * Release
 *  ignores NULL references
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


/* Size
 *  ask page its size
 */
KLIB_EXTERN size_t CC KBTreePageSize_v1 ( const KBTreePage_v1 * self, ctx_t ctx );


/* AccessRead
 * AccessUpdate
 *  ask page for its memory
 */
KLIB_EXTERN const void * CC KBTreePageAccessRead_v1 ( const KBTreePage_v1 * self, ctx_t ctx );
KLIB_EXTERN void * CC KBTreePageAccessUpdate_v1 ( KBTreePage_v1 * self, ctx_t ctx );


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
 * KBTreeValue_v1
 *  an accessor of a single value within paged memory
 *  when a KBTreeValue exists, all of its affected pages
 *  are held within addressable memory.
 *
 *  to access page memory for read, use KBTreeValueAccessRead().
 *  it will return a pointer to the portion of the value given at
 *  requested position, as well as the size of valid portion of
 *  mapped memory, since the value may span multiple pages.
 *
 *  to access page memory for read/write, use KBTreeValueAccessUpdate().
 *  this behaves like the reading API, except that it returns a pointer
 *  to writable data. This allows
 */

#ifndef KBTREEVALUE_VERS
#define KBTREEVALUE_VERS 1
#endif

#if KBTREEVALUE_VERS == 1
#define KBTreeValue KBTreeValue_v1
#endif

/* Duplicate
 * Release
 *  ignores NULL references
 */
static __inline__
KBTreeValue_v1 * KBTreeValueDuplicate_v1 ( const KBTreeValue_v1 * self, ctx_t ctx, caps_t rm )
{
    return ( KBTreeValue_v1 * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KBTreeValueRelease_v1 ( const KBTreeValue_v1 * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}


/* Size
 *  ask value for its size
 */
KLIB_EXTERN size_t CC KBTreeValueSize_v1 ( const KBTreeValue_v1 * self, ctx_t ctx );


/* AccessRead
 * AccessUpdate
 *  access raw memory for stated access at a given offset
 *
 *  "pos" [ IN ] - position within value to access
 *
 *  "size" [ OUT ] - number of valid bytes in page being accessed
 *
 *  returns pointer to raw memory, valid for "size" bytes
 */
KLIB_EXTERN const void * CC KBTreeValueAccessRead_v1 ( const KBTreeValue_v1 * self, ctx_t ctx,
    uint64_t pos, size_t * size );
KLIB_EXTERN void * CC KBTreeValueAccessUpdate_v1 ( KBTreeValue_v1 * self, ctx_t ctx,
    uint64_t pos, size_t * size );


/* Overwrite
 *  overwrite value data using copying protocol
 *
 *  "buffer" [ IN ] and "size" [ IN ] - bytes to be written to value
 */
KLIB_EXTERN void CC KBTreeValueOverwrite_v1 ( KBTreeValue_v1 * self, ctx_t ctx,
    const void * buffer, size_t size );


/*--------------------------------------------------------------------------
 * KBPlusTree
 */
#define KBPlusTreeDuplicate NAME_VERS ( KBPlusTreeDuplicate, KBPLUSTREE_VERS )
#define KBPlusTreeRelease NAME_VERS ( KBPlusTreeRelease, KBPLUSTREE_VERS )
#define KBPlusTreeMakeRead NAME_VERS ( KBPlusTreeMakeRead, KBPLUSTREE_VERS )
#define KBPlusTreeMakeUpdate NAME_VERS ( KBPlusTreeMakeUpdate, KBPLUSTREE_VERS )
#define KBPlusTreeInsert NAME_VERS ( KBPlusTreeInsert, KBPLUSTREE_VERS )
#define KBPlusTreeFind NAME_VERS ( KBPlusTreeFind, KBPLUSTREE_VERS )
#define KBPlusTreeUpdate NAME_VERS ( KBPlusTreeUpdate, KBPLUSTREE_VERS )
#define KBPlusTreeDelete NAME_VERS ( KBPlusTreeDelete, KBPLUSTREE_VERS )
#define KBPlusTreeForEach NAME_VERS ( KBPlusTreeForEach, KBPLUSTREE_VERS )
#define KBPlusTreeDoUntil NAME_VERS ( KBPlusTreeDoUntil, KBPLUSTREE_VERS )

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

#endif /*  _h_klib_b_plus_tree_ */
