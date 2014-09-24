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

#include "blob.h"
#include "blob-headers.h"
#include "page-map.h"
#include "blob-priv.h"
#include "xform-priv.h"

#include <vdb/xform.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <kdb/index.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/pbstree.h>
#include <klib/log.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <assert.h>

typedef struct tag_self_t {
    const       KIndex *ndx;
    char        query_key[1024];
    uint32_t    query_key_len;
    const struct VCursorParams * parms;
    uint32_t    elem_bits;
} self_t;

static void CC self_whack( void *Self )
{
    self_t *self = Self;
    
    KIndexRelease( self->ndx );
    free( self );
}


static
rc_t CC index_lookup_impl(
                          void *Self,
                          const VXformInfo *info,
                          int64_t row_id,
                          VRowResult *rslt,
                          uint32_t argc,
                          const VRowData argv[]
                          )
{
    rc_t rc;
    const self_t *self = Self;
    KDataBuffer *query_buf = NULL;
    
    rslt->elem_count = 0;
    rslt->no_cache = 1;
    
    rc = VCursorParamsGet(self->parms, self->query_key, &query_buf);
    if (GetRCState(rc) == rcNotFound && GetRCObject(rc) == rcName) {
        rc = KDataBufferResize(rslt->data, 0);
        return rc;
    }
    if (rc == 0) {
        char  squery[4096];
        char *hquery = NULL;
        char *query = squery;
        uint64_t id_count;
        int64_t start_id;
        
        if (query_buf->elem_count >= sizeof(squery)) {
            hquery = malloc(query_buf->elem_count + 1);
            if (hquery == NULL)
                return RC(rcVDB, rcIndex, rcReading, rcMemory, rcExhausted);
            query = hquery;
        }
        memcpy(query, query_buf->base, query_buf->elem_count);
        query[query_buf->elem_count] = '\0';
        rc = KIndexFindText(self->ndx, query, &start_id, &id_count,NULL,NULL);
        if (hquery)
            free(hquery);
        if (rc == 0) {
            rc = KDataBufferResize ( rslt -> data, 1 );
            if( rc == 0) {
                int64_t *out = rslt -> data->base;
                
                out[0] = start_id;
                out[1] = start_id + id_count - 1;

                rslt -> elem_count = 1;
                rslt -> no_cache = 1; /***** This row should never be cached **/
            }
        }
    }
    return rc;
}

/*
 * function vdb:row_id_range  idx:text:lookup #1 < ascii index_name , ascii query_by_name > ();
 */
VTRANSFACT_BUILTIN_IMPL(idx_text_lookup, 1, 0, 0) (
                                           const void *Self,
                                           const VXfactInfo *info,
                                           VFuncDesc *rslt,
                                           const VFactoryParams *cp,
                                           const VFunctionParams *dp
) {
    rc_t rc;
    const KIndex *ndx;
    KIdxType type;
    
    rc = VTableOpenIndexRead(info->tbl, &ndx, "%.*s", (int)cp->argv[0].count, cp->argv[0].data.ascii);
    if ( rc != 0 )
    {
        if ( GetRCState ( rc ) != rcNotFound )
            PLOGERR (klogErr, (klogErr, rc, "Failed to open index '$(index)'", "index=%.*s", (int)cp->argv[0].count, cp->argv[0].data.ascii));
        return rc;
    }
    
    rc = KIndexType(ndx, &type);
    if (rc == 0) {
        if (type == kitProj + kitText) {
            self_t *self;
            
            self = malloc(sizeof(*self));
            if (self) {
                self->ndx = ndx;
                self->elem_bits = VTypedescSizeof(&info->fdesc.desc);
                memcpy(self->query_key,cp->argv[1].data.ascii,cp->argv[1].count);
                self->query_key_len = cp->argv[1].count;
                self->query_key[self->query_key_len] = '\0';
                self->parms = info->parms;
                
                rslt->self = self;
                rslt->whack = self_whack;
                rslt->variant = vftNonDetRow;
                rslt->u.ndf = index_lookup_impl;
                return 0;
            }
            rc = RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
        }
        else
            rc = RC(rcVDB, rcFunction, rcConstructing, rcIndex, rcIncorrect);
    }
    KIndexRelease(ndx);
    return rc;
}
