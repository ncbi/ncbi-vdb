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

struct KBPlusTree_v1;
#define KREFCOUNT_V1 struct KBPlusTree_v1

#include <klib/extern.h>
#include <klib/b+tree.h>

#include "btree-priv.h"


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

static
void CC KBPlusTreeWhack_v1 ( KBPlusTree_v1 * self, ctx_t ctx )
{
}

static KRefcount_v1_vt KBPlusTree_v1_KRefcount_vt =
{
    KVTABLE_INITIALIZER ( KBPlusTree_v1, KRefcount_v1, 0, NULL ),
    .destroy = KBPlusTreeWhack_v1
};


/* MakeRead
 *  create a b+tree for read-only access
 *
 *  "pager" [ IN ] - a pager interface for accessing pages in storage.
 */
LIB_EXPORT const KBPlusTree_v1 * CC KBPlusTreeMakeRead_v1 ( ctx_t ctx, const KBTreePager_v1 * pager )
{
}


/* MakeUpdate
 *  create a b+tree for read/write access
 *
 *  "opt_pager" [ IN, NULL OKAY ] - a pager interface for accessing, creating, deleting
 *   pages in storage. default is to create a new, empty, RAM-based storage.
 *   required if passing in an existing tree.
 */
LIB_EXPORT KBPlusTree_v1 * CC KBPlusTreeMakeUpdate_v1 ( ctx_t ctx, KBTreePager_v1 * opt_pager )
{
}


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
LIB_EXPORT KBTreeValue_v1 * CC KBPlusTreeInsert_v1 ( KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size, const void * value, size_t value_size )
{
}


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
LIB_EXPORT const KBTreeValue_v1 * CC KBPlusTreeFind_v1 ( const KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size )
{
}


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
LIB_EXPORT KBTreeValue_v1 * CC KBPlusTreeUpdate_v1 ( KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size )
{
}


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
LIB_EXPORT bool CC KBPlusTreeDelete_v1 ( KBPlusTree_v1 * self, ctx_t ctx,
    const void * key, size_t key_size )
{
}


/* ForEach
 *  executes a function on each tree element
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 */

LIB_EXPORT void CC KBPlusTreeForEach_v1 ( KBPlusTree_v1 * self, ctx_t ctx, bool reverse,
    void ( CC * f ) ( KBPlusTree_v1 * bt, ctx_t ctx, const void * key, size_t key_size,
        const void * value, size_t value_size, void * data ), void * data )
{
}


/* DoUntil
 *  executes a function on each tree element until "f" returns "true"
 *
 *  "reverse" [ IN ] - if true, iterate in reverse order
 *
 *  "f" [ IN ] and "data" [ IN, OPAQUE ] - callback function
 *
 *  returns "true" if "f" returned true, "false" otherwise
 */

LIB_EXPORT bool CC KBPlusTreeDoUntil_v1 ( KBPlusTree_v1 * self, ctx_t ctx, bool reverse,
    bool ( CC * f ) ( KBPlusTree_v1 * bt, ctx_t ctx, const void * key, size_t key_size,
        const void * value, size_t value_size, void * data ), void * data )
{
}

