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
#include <vdb/extern.h>
#include <vdb/xform.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>

#include <kdb/meta.h>

#include <insdc/insdc.h>
#include <insdc/sra.h>

#include <align/refseq-mgr.h>

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/debug.h>
#include <klib/time.h>
#include <klib/vector.h>
#include <klib/data-buffer.h>
#include <klib/sort.h>

#include <vdb/vdb-priv.h>

#include <bitstr.h>
#include <sysalloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif

typedef struct Restorer Restorer;
struct Restorer
{
    const VCursor *curs;
    uint32_t colId;
};

static
void CC restorerWhack(void *const obj)
{
    if (obj != NULL) {
        Restorer const *const self = obj;
        VCursorRelease(self->curs);
    }
    free(obj);
}

static
rc_t CC seqRestoreLinkageGroup(void *const Self,
                               VXformInfo const *const info,
                               int64_t const row_id,
                               VRowResult *const rslt,
                               uint32_t argc, VRowData const argv[/* argc */])
{
    Restorer const *const self = (Restorer const *)Self;
    int64_t const *const alignId = &((int64_t const *)argv[1].u.data.base)[argv[1].u.data.first_elem];
    unsigned const n = (unsigned)argv[1].u.data.elem_count;
    char const *const linkageGroup = &((char const *)argv[0].u.data.base)[argv[0].u.data.first_elem];
    unsigned const linkageGroupLen = (unsigned)argv[0].u.data.elem_count;
    int64_t foreignKey = 0;
    unsigned i;

    assert(argv[0].u.data.elem_bits == 8 * sizeof(linkageGroup[0]));
    assert(argv[1].u.data.elem_bits == 8 * sizeof(alignId[0]));

    rslt->data->elem_bits = 8;
    for (i = 0; i < n && foreignKey == 0; ++i) {
        foreignKey = alignId[i];
    }
    if (foreignKey == 0) {
        rc_t const rc = KDataBufferResize(rslt->data, linkageGroupLen);
        rslt->elem_count = linkageGroupLen;
        if (rc == 0)
            memmove(rslt->data->base, linkageGroup, linkageGroupLen);
        return rc;
    }
    else {
        void const *linkageGroup = NULL;
        uint32_t linkageGroupLen = 0;
        uint32_t elem_size = 0;
        uint32_t offset = 0;
        rc_t const rc = VCursorCellDataDirect(self->curs, foreignKey, self->colId, &elem_size, &linkageGroup, &offset, &linkageGroupLen);

        if (rc == 0) {
            rc_t const rc = KDataBufferResize(rslt->data, linkageGroupLen);
            assert(elem_size == rslt->data->elem_bits);
            assert(offset == 0);
            rslt->elem_count = linkageGroupLen;
            if (rc == 0)
                memmove(rslt->data->base, linkageGroup, linkageGroupLen);
            return rc;
        }
        return rc;
    }
}


/* 
 * function
   ascii NCBI:align:seq_restore_linkage_group #1 ( ascii cmp_linkage_group,
                                                   I64 align_id )

 */
VTRANSFACT_IMPL ( ALIGN_seq_restore_linkage_group, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                     VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    VCursor const *const nativeCurs = (VCursor const *)info->parms;

    VCursor const *myCurs = NULL;
    uint32_t colId = 0;

    rc_t rc = VCursorLinkedCursorGet(nativeCurs, "PRIMARY_ALIGNMENT", &myCurs);
    if (GetRCObject(rc) == rcName && GetRCState(rc) == rcNotFound) {
        VTable const *tbl = NULL;
        VDatabase const *db = NULL;

        rc = VTableOpenParentRead(info->tbl, &db);
        assert(rc == 0 && db != NULL);
        if (rc != 0 || db == NULL) abort();

        rc = VDatabaseOpenTableRead(db, &tbl, "PRIMARY_ALIGNMENT");
        VDatabaseRelease(db);
        if (rc == 0) {
            rc = VTableCreateCachedCursorRead(tbl, &myCurs, 32UL * 1024UL * 1024UL);
            VTableRelease(tbl);
        }
        if (rc == 0)
            rc = VCursorLinkedCursorSet(nativeCurs, "PRIMARY_ALIGNMENT", myCurs);
    }
    if (rc == 0) {
        rc = VCursorAddColumn(myCurs, &colId, "LINKAGE_GROUP");
        if (GetRCState(rc) == rcExists)
            rc = 0;
        if (rc == 0)
            rc = VCursorOpen(myCurs);
    }
    if (rc == 0)
    {
        Restorer *fself = malloc(sizeof(*fself));

        assert(fself);
        if (fself == NULL) abort();

        fself->curs = myCurs;
        fself->colId = colId;

        rslt->self = fself;
        rslt->u.ndf = seqRestoreLinkageGroup;
        rslt->variant = vftRow;
        rslt->whack = restorerWhack;
    }
    return rc;
}
