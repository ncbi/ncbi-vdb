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
#include <vdb/vdb.h>
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
    uint8_t case_sensitivity;
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
    KDataBuffer temp_buff;
    uint64_t id_count;
    int64_t start_id;
    int64_t empty_row_id_start = -1;
    int64_t empty_row_id_count = -1;
    size_t sz = 1023;
    bool attached_to_col = argc > 0 && argv[0] != NULL;
    
    /* first try to load value from the column. if returned blob is empty or row is not found, go to index */
    if ( attached_to_col ) {
        /*** this types of blobs may have holes in them ***/
        rc = VBlobSubblob(argv[0],rslt,row_id );
        if (rc != 0) {
            if (GetRCState(rc) == rcEmpty && GetRCObject(rc) == rcRow) {
                empty_row_id_start = row_id;
                empty_row_id_count = 1;
            }
            else {
                return rc;
            }
        }
        else if ((*rslt)->data.elem_count > 0) {
            return rc;
        }
        else {
            empty_row_id_start = (*rslt)->start_id;
            empty_row_id_count = (*rslt)->stop_id - (*rslt)->start_id + 1;
            
            TRACK_BLOB( VBlobRelease, *rslt );
            (void)VBlobRelease( *rslt );
        }

        assert(empty_row_id_count >= 1);
    }

    rc = KDataBufferMakeBytes( &temp_buff, sz + 1 );
    if ( rc != 0 )
        return rc;

    for ( ; ; ) {
        rc = KIndexProjectText(self->ndx, row_id, &start_id, &id_count, temp_buff.base, temp_buff.elem_count, &sz);
        if ((GetRCState(rc) == rcNotFound && GetRCObject(rc) == rcId) || sz==0 ){
            if ( !attached_to_col )
                rc = RC(rcVDB, rcFunction, rcExecuting, rcRow, rcNotFound);
            else
            {
                // return an empty row, but we don't know how many empty rows
                // are there, since even row_id+1 may have a key stored in index
                rc = 0;
                sz = 0;
                start_id = row_id;
                id_count = 1;
            }

            break;
        }
        if ( GetRCState( rc ) == rcInsufficient && GetRCObject( rc ) == (enum RCObject)rcBuffer )
        {
            rc = KDataBufferResize ( &temp_buff, (uint32_t)( sz + 1 ) );
            if (rc == 0) {
                continue;
            }
        }
        
        // When in case_sensitivity mode is case insensitive, index does not accurately represent actual values,
        // as we still store key in a column when it differs from what we inserted into index
        if (self->case_sensitivity != CASE_SENSITIVE && attached_to_col)
        {
            if ( start_id < empty_row_id_start )
            {
                id_count -= empty_row_id_start - start_id;
                start_id = empty_row_id_start;
            }

            if ( start_id + id_count > empty_row_id_start + empty_row_id_count )
            {
                id_count = empty_row_id_start + empty_row_id_count - start_id;
            }
        }
        break;
    }

    if ( rc == 0 )
    {
        /* it seems old index returns length including \0 so we have to adjust */
        while (sz > 0 && ((char *)temp_buff.base)[sz - 1] == '\0')
            --sz;

        // now we know real size of the data, lets set in data buffer too
        assert ( temp_buff.elem_count >= sz );
        if ( temp_buff.elem_count != sz )
            rc = KDataBufferResize ( &temp_buff, (uint32_t)( sz ) );
    }

    if (rc == 0)
    {
        rc = VBlobCreateFromSingleRow ( rslt, start_id, start_id + id_count - 1, &temp_buff, vboNative );
    }

    KDataBufferWhack(&temp_buff);
    return rc;
}

VTRANSFACT_BUILTIN_IMPL(idx_text_project, 1, 1, 1) (
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
                self->case_sensitivity = cp->argc >= 2 ? *cp->argv[1].data.u8 : CASE_SENSITIVE;
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
