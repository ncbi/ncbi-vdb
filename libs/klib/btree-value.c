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

struct KBTreeValue_v1;
#define KREFCOUNT_V1 struct KBTreeValue_v1

#include <klib/extern.h>
#include <klib/b+tree.h>

#include "btree-priv.h"

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

static
void CC KBTreeValueWhack_v1 ( KBTreeValue_v1 * self, ctx_t ctx )
{
}

static KRefcount_v1_vt KBTreeValue_v1_KRefcount_vt =
{
    KVTABLE_INITIALIZER ( KBTreeValue_v1, KRefcount_v1, 0, NULL ),
    .destroy = KBTreeValueWhack_v1
};

/* Size
 *  ask value for its size
 */
LIB_EXPORT size_t CC KBTreeValueSize_v1 ( const KBTreeValue_v1 * self, ctx_t ctx )
{
}


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
LIB_EXPORT const void * CC KBTreeValueAccessRead_v1 ( const KBTreeValue_v1 * self, ctx_t ctx,
    uint64_t pos, size_t * size )
{
}

LIB_EXPORT void * CC KBTreeValueAccessUpdate_v1 ( KBTreeValue_v1 * self, ctx_t ctx,
    uint64_t pos, size_t * size )
{
}


/* Overwrite
 *  overwrite value data using copying protocol
 *
 *  "buffer" [ IN ] and "size" [ IN ] - bytes to be written to value
 */
LIB_EXPORT void CC KBTreeValueOverwrite_v1 ( KBTreeValue_v1 * self, ctx_t ctx,
    const void * buffer, size_t size )
{
}
