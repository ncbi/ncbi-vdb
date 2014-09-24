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
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/rc.h>
#include "name-tokenizer.h"
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/* extract_spot_name
 *  generates input to .SPOT_NAME column
 *
 *  on NCBI:SRA:name_token:unrecognized, produces the entire spot name row
 *  otherwise, produces an empty row
 *
 *  "name" [ DATA ] - raw spot names from NAME column
 *
 *  "tok" [ DATA ] - delimiting tokens produced by sub-table
function ascii
    NCBI:SRA:extract_spot_name #1 ( ascii name, NCBI:SRA:spot_name_token tok );
 */

#define NAM 0
#define TOK 1

static
rc_t CC extract_spot_name ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    uint32_t i;
    bool name_copy;

    const spot_name_token_t *token = argv [ TOK ] . u . data . base;
    uint32_t num_tokens = argv [ TOK ] . u . data . elem_count;
    token += argv [ TOK ] . u . data . first_elem;

    for ( name_copy = false, i = 0; i < num_tokens; ++ i )
    {
        if ( token [ i ] . s . token_type == nt_unrecognized )
        {
            name_copy = true;
            break;
        }
    }

    rslt -> elem_count = 0;
    rslt -> data -> elem_bits = 8;

    if ( name_copy || num_tokens == 0 )
    {
        const char *name = argv [ NAM ] . u . data . base;
        uint32_t name_len = argv [ NAM ] . u . data . elem_count;

        rc_t rc = KDataBufferResize ( rslt -> data, name_len );
        if ( rc != 0 )
            return rc;

        memcpy ( rslt -> data -> base, name += argv [ NAM ] . u . data . first_elem, name_len );
        rslt -> elem_count = name_len;
    }

    return 0;
}

VTRANSFACT_IMPL ( NCBI_SRA_extract_spot_name, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftRow;
    rslt->u.rf = extract_spot_name;
    
    return 0;
}
