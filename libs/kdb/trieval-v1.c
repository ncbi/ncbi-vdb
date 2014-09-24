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

#include <kdb/extern.h>

#include "index-priv.h"

#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>


rc_t KTrieIndexCheckConsistency_v1 ( const KTrieIndex_v1 *self,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes,
    struct KIndex const *outer, bool key2id, bool id2key )
{
    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcValidating, rcParam, rcNull );

    return KPTrieIndexCheckConsistency_v1 ( & self -> pt,
        start_id, id_range, num_keys, num_rows, num_holes,
        outer, key2id, id2key );
}
