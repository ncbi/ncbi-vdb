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

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>
#include <kfs/mmap.h>
#include <klib/pbstree.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <string.h>

typedef struct KU64Index_PNode KU64Index_PNode;
struct KU64Index_PNode
{
    uint64_t key;
    uint64_t key_size;
    int64_t id;
    uint64_t id_qty;
};

rc_t KU64IndexOpen_v3( KU64Index_v3* self, const KMMap *mm, bool byteswap )
{
    rc_t rc;
    const KIndexFileHeader_v3 *hdr;

    /* when opened for create, there will be no existing index */
    if( mm == NULL )
        return 0;

    /* open the prior index in persisted mode */
    rc = KMMapAddrRead ( mm, ( const void** ) & hdr );
    if( rc == 0 )
    {
        size_t msize;
        rc = KMMapSize ( mm, & msize );
        if ( rc == 0 )
        {
            if ( msize == 0 )
                return 0;
            if ( msize < sizeof * hdr )
                return RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
            rc = PBSTreeMake ( & self -> tree, hdr + 1, msize - sizeof * hdr, byteswap);
            if( rc == 0 )
            {
                rc = KMMapAddRef(mm);
                if ( rc == 0 )
                {
                    self -> mm = mm;
                    return 0;
                }
            }
        }
    }

    KU64IndexWhack_v3(self);
    return rc;
}

rc_t KU64IndexWhack_v3(KU64Index_v3* self)
{
    PBSTreeWhack(self->tree);
    KMMapRelease(self->mm);
    self->tree = NULL;
    return 0;
}

typedef struct KU64Index_GrepData_struct {
    rc_t rc;
    rc_t (CC*func)(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data);
    void* data;
    KU64Index_PNode search;
    uint64_t* key;
    uint64_t* key_size;
    int64_t* id;
    uint64_t* id_qty;
} KU64Index_GrepData;

/*
 * return true: if found or break DoUntil for FindAll
 */
static
bool CC KU64Index_Grep(PBSTNode *node, void *data)
{
    const KU64Index_PNode* n = node->data.addr;
    KU64Index_GrepData* d = data;

    if( d->search.key >= n->key && (d->search.key - n->key) < n->key_size ) {
        if( d->func ) {
            d->rc = (*d->func)(n->key, n->key_size, n->id, n->id_qty, d->data);
            if( d->rc != 0 ) {
                return true;
            }
        } else {
            *d->key = n->key;
            *d->key_size = n->key_size;
            *d->id = n->id;
            *d->id_qty = n->id_qty;
            return true;
        }
    }
    return false;
}

rc_t KU64IndexFind_v3( const KU64Index_v3* self, uint64_t offset, uint64_t* key, 
    uint64_t* key_size, int64_t* id, uint64_t* id_qty )
{
    KU64Index_GrepData d;

    memset(&d, 0, sizeof(KU64Index_GrepData));
    d.search.key = offset;
    d.key = key;
    d.key_size = key_size;
    d.id = id;
    d.id_qty = id_qty;
    if( !PBSTreeDoUntil(self->tree, false, KU64Index_Grep, &d) ) {
        d.rc = RC(rcDB, rcIndex, rcSelecting, rcId, rcNotFound);
    }
    return d.rc;
}


rc_t KU64IndexFindAll_v3( const KU64Index_v3* self, uint64_t offset, 
    rc_t (CC*f)(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data), void* data)
{
    KU64Index_GrepData d;

    memset(&d, 0, sizeof(KU64Index_GrepData));
    d.func = f;
    d.data = data;
    d.search.key = offset;
    PBSTreeDoUntil(self->tree, false, KU64Index_Grep, &d);
    return d.rc;
}
