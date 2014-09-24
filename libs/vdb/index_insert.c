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
#include <kdb/index.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

struct self_t {
    KIndex *ndx;
    char *key_buf;
    size_t key_buf_size;
};

/*
 function utf8 idx:text:insert #1.0 < ascii index_name > ( utf8 key );
 */

static
rc_t CC index_insert( void *Self, const VXformInfo *info, int64_t row_id,
                       VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    struct self_t *self = Self;
    rc_t rc;
    const char *x;
    char skey[4096];
    char *key = skey;
    uint32_t key_len;

    rslt -> elem_count = 0;

    x = argv [ 0 ] . u . data . base;
    key_len = argv [ 0 ] . u . data . elem_count;
    x += argv [ 0 ] . u . data . first_elem;
    
    if (key_len + 1 >= sizeof(skey)) {
        if (key_len + 1 >= self->key_buf_size) {
            void *temp = realloc(self->key_buf, key_len + 1);
            if (temp == NULL)
                return RC(rcVDB, rcFunction, rcExecuting, rcMemory, rcExhausted);
            self->key_buf = temp;
            self->key_buf_size = key_len + 1;
        }
        key = self->key_buf;
    }
    memcpy(key, x, key_len);
    key[key_len] = 0;

    /* attempt to insert into index */
    rc = KIndexInsertText ( self->ndx, false, key, row_id );
    if ( rc != 0 )
    {
        /* insert failed for whatever reason - return key */
        rc = KDataBufferResize ( rslt -> data, key_len );
        if ( rc != 0 )
            return rc;
        memcpy ( rslt -> data -> base, key, key_len );
        
        rslt -> elem_count = key_len;
    }
    
    return 0;
}    

static void CC self_whack(void *Self) {
    struct self_t *self = Self;
    KIndexRelease(self->ndx);
    if (self->key_buf)
        free(self->key_buf);
    free(Self);
}

VTRANSFACT_IMPL ( idx_text_insert, 1, 0, 0 ) ( const void *Self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    struct self_t *self;
    
    self = malloc(sizeof(*self));
    if (self) {
        rc_t rc;
        
        rc = VTableCreateIndex ( ( VTable* ) info -> tbl, &self->ndx, kitText | kitProj, kcmOpen,
                                "%.*s", cp->argv[0].count, cp->argv[0].data.ascii );
        if( rc == 0 ) {
            rslt->self = self;
            rslt->whack = self_whack;
            rslt->variant = vftNonDetRow;
            rslt->u.ndf = index_insert;
            self->key_buf = NULL;
            self->key_buf_size = 0;
            return 0;
        }
        free(self);
        return rc;
    }
    else
        return RC(rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
