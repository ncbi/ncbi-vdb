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

struct KBTreePage_v1;
#define KREFCOUNT_V1 struct KBTreePage_v1

#include <klib/extern.h>
#include <klib/b+tree.h>

#include "btree-priv.h"

/*--------------------------------------------------------------------------
 * KBTreePage_v1
 */

static
void CC KBTreePageWhack_v1 ( KBTreePage_v1 * self, ctx_t ctx )
{
}

static KRefcount_v1_vt KBTreePage_v1_KRefcount_vt =
{
    KVTABLE_INITIALIZER ( KBTreePage_v1, KRefcount_v1, 0, NULL ),
    .destroy = KBTreePageWhack_v1
};


/* Size
 *  ask page its size
 */
LIB_EXPORT size_t CC KBTreePageSize_v1 ( const KBTreePage_v1 * self, ctx_t ctx )
{
}


/* AccessRead
 * AccessUpdate
 *  ask page for its memory
 */
LIB_EXPORT const void * CC KBTreePageAccessRead_v1 ( const KBTreePage_v1 * self, ctx_t ctx )
{
}

LIB_EXPORT void * CC KBTreePageAccessUpdate_v1 ( KBTreePage_v1 * self, ctx_t ctx )
{
}
