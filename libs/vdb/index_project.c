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
#include <kdb/index.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <sysalloc.h>
#include <atomic32.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

typedef struct tag_self_t {
    const KIndex *ndx;
    uint32_t elem_bits;
} self_t;

static void CC self_whack( void *Self )
{
    self_t *self = Self;
    
    KIndexRelease( self->ndx );
    free( self );
}

#include <stdio.h>

static
rc_t CC index_project_impl(
                            void *Self,
                            const VXformInfo *info,
                            int64_t row_id,
                            VBlob **rslt,
                            uint32_t argc, const VBlob *argv[]
) {
    rc_t rc;
    const self_t *self = Self;
    KDataBuffer temp;
    uint64_t id_count;
    int64_t start_id;
    char key_buf[1024];
    char *key = key_buf;
    size_t sz = sizeof(key_buf) - 1;
    
    /* first try to load value from the column. if returned blob is empty, go to index */
    if (argc > 0 && argv[0] != NULL) {
        /*** this types of blobs may have holes in them ***/
        rc = VBlobSubblob(argv[0],rslt,row_id );
        if (rc != 0 || (*rslt)->data.elem_count > 0) {
            return rc;
        }
    }

    for ( ; ; ) {
        rc = KIndexProjectText(self->ndx, row_id, &start_id, &id_count, key, sz + 1, &sz);
        if ((GetRCState(rc) == rcNotFound && GetRCObject(rc) == rcId) || sz==0 ){
/*          fprintf(stderr, "row %u not in index\n", (unsigned)row_id); */
            rc = RC(rcVDB, rcFunction, rcExecuting, rcRow, rcNotFound);
            break;
        }
        if ( GetRCState( rc ) == rcInsufficient && GetRCObject( rc ) == (enum RCObject)rcBuffer && key == key_buf )
        {
            rc = KDataBufferMakeBytes( &temp, (uint32_t)( sz + 1 ) );
            if (rc) {
                key = temp.base;
                continue;
            }
        }
        if (rc == 0) {
            VBlob *y;
        
            /* it seems old index returns length including \0 so we have to adjust */
            while (sz > 0 && key[sz - 1] == '\0')
                --sz;

            rc = VBlobNew(&y, start_id, start_id + id_count - 1, "vdb:index:project");
            if (rc == 0) {
                rc = PageMapNewSingle( &y->pm, (uint32_t)id_count, (uint32_t)sz );
                if (rc == 0) {
                    if (key == key_buf) {
                        rc = KDataBufferMakeBytes( &y->data, (uint32_t)sz );
                        if (rc == 0)
                            memcpy(y->data.base, key, sz);
                    }
                    else
                        KDataBufferSub( &temp, &y->data, 0, (uint32_t)sz );
                }
                if (rc == 0) 
                    *rslt = y;
                else {
                    TRACK_BLOB( VBlobRelease, ((VBlob*)y) );
                    (void)VBlobRelease(y);
                }
            }
        }
        break;
    }
    if (key != key_buf)
        KDataBufferWhack(&temp);
    return rc;
}

VTRANSFACT_BUILTIN_IMPL(idx_text_project, 1, 0, 1) (
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
                rslt->self = self;
                rslt->whack = self_whack;
                rslt->variant = vftBlobN;
                VFUNCDESC_INTERNAL_FUNCS(rslt)->bfN = index_project_impl;
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
