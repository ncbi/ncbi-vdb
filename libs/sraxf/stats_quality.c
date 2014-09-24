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

#include <sra/sradb.h>
#include <vdb/xform.h>
#include <vdb/table.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <kdb/meta.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_QUALITY (256)

typedef struct phred_stats_data_struct {
    KMetadata* meta;
    KMDataNode* root;
    uint64_t count[MAX_QUALITY];
} phred_stats_data;

static
void CC phred_stats_whack( void *data )
{
    uint32_t i;
    phred_stats_data* self = data;

    for(i = 0; i < MAX_QUALITY; i++) {
        if( self->count[i] > 0 ) {
            KMDataNode* node;
            if( KMDataNodeOpenNodeUpdate(self->root, &node, "PHRED_%u", i) == 0 ) {
                KMDataNodeWriteB64(node, &self->count[i]);
                KMDataNodeRelease(node);
            }
        }
    }
    KMDataNodeRelease(self->root);
    KMetadataRelease(self->meta);
    free(self);
}

static
rc_t phred_stats_make(phred_stats_data** self, VTable* vtbl)
{
    rc_t rc = 0;
    phred_stats_data* data = calloc(1, sizeof(*data));

    assert(self != NULL && vtbl != NULL);

    if( data == NULL ) {
        rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
    } else if( (rc = VTableOpenMetadataUpdate(vtbl, &data->meta)) == 0 ) {
        rc = KMetadataOpenNodeUpdate(data->meta, &data->root, "STATS/QUALITY");
    }
    if( rc == 0 ) {
        *self = data;
    } else {
        phred_stats_whack(data);
    }
    return rc;
}

static
rc_t CC phred_stats_trigger(void *data, const VXformInfo *info, int64_t row_id,
                              VRowResult *rslt, uint32_t argc, const VRowData argv[])
{
    rc_t rc = 0;
    phred_stats_data* self = data;
    uint64_t len;

    assert(self != NULL);
    assert(argc == 1);
    assert(argv[0].u.data.elem_bits == (sizeof(INSDC_quality_phred) * 8));

    len = argv[0].u.data.elem_count;
    if( len > 0 ) {
        const INSDC_quality_phred* q = argv[0].u.data.base;
        assert(argv[0].u.data.base != NULL);
        q += argv[0].u.data.first_elem;
        do {
            self->count[q[--len]]++;
        } while( len > 0 );
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_phred_stats_trigger, 1, 0, 0 )
    ( const void *self, const VXfactInfo *info, VFuncDesc *rslt,
      const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    phred_stats_data *data;

    assert(dp->argc == 1);

    if( (rc = phred_stats_make(&data, (VTable*)info->tbl)) == 0 ) {
        rslt->self = data;
        rslt->whack = phred_stats_whack;
        rslt->variant = vftNonDetRow;
        rslt->u.rf = phred_stats_trigger;
    }
    return rc;
}
