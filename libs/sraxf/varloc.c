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
#include <insdc/insdc.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <vdb/schema.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/****************************** tokenize_var_id *******************************/
/* typedef uint16_t text_token [ 3 ]; */

static
rc_t CC tokenize_var_id ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc = 0;
    int pos = 0;
    unsigned const var_id_len = argv[0].u.data.elem_count;
    struct
    {
        uint16_t token_type;
        uint16_t position;
        uint16_t length;
    } *dst;
    const char *var_id	= argv[0].u.data.base;
    var_id += argv[0].u.data.first_elem;
    
    rslt->data->elem_bits = sizeof(dst[0]) * 8;
    rc = KDataBufferResize( rslt -> data, 2 );
    if ( rc != 0 ) return rc;
    rslt -> elem_count = 2;
    dst = rslt -> data -> base;
    memset(dst, 0, 2 * sizeof *dst);
 
    /* ([A-Za-z]*)(\d*) */
    if (var_id_len > 0) {
        for (pos = var_id_len - 1; pos >= 0; --pos) {
            if (var_id[pos] < '0' || var_id[pos] > '9') {
                ++pos;
                break;
            }
            if (pos == 0) { /* all numbers */
                break;
            }
        }
    }
    dst [ 1 ] . position = pos;
    dst [ 1 ] . length = var_id_len - pos;
    dst [ 0 ] . length = var_id_len - dst [ 1 ] . length;

    return rc;
}

/* 
 * tokenize_var_id
 *   splits into 2 tokens
 *   0 - prefix
 *   1 - suffix
 *
 * extern function
 * text:token NCBI:var:tokenize_var_id #1 ( ascii var_id );
 */
VTRANSFACT_IMPL ( NCBI_var_tokenize_var_id, 1, 0, 0 ) ( const void *Self,
    const VXfactInfo *info, VFuncDesc *rslt,
    const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->u.rf = tokenize_var_id;
    rslt->variant = vftRow;
    return 0;
}
