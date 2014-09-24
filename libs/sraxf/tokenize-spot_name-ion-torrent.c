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
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "name-tokenizer.h"

static
rc_t CC tokenize_spot_name_IonTorrent( void *self, const VXformInfo *info, int64_t row_id,
                                       VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    const char *name, *end;
    spot_name_token_t *spot_name_tok;
    const int EXPECTED_NUMBER_OF_TOKENS = 2;
    int tok = EXPECTED_NUMBER_OF_TOKENS;
    const uint16_t types[4] = {nt_X, nt_Y};
    
    assert(rslt->elem_bits == sizeof(spot_name_tok[0]) * 8);
    rslt->data->elem_bits = sizeof(spot_name_tok[0]) * 8;
    rc = KDataBufferResize(rslt->data, EXPECTED_NUMBER_OF_TOKENS);
    if( rc != 0 )
        return rc;
    
    spot_name_tok = rslt->data->base;
    
    /* reverse line parse by format:
       /^(.+):([0-9]+):([0-9]+)$/ = (name, x, x) = ($1, $2, $3)
    */
    name = argv[0].u.data.base;
    name += argv[0].u.data.first_elem;
    end = name + argv[0].u.data.elem_count;

    while ( end > name && tok > 0 )
    {
        const char *num_start, *num_end = end;

        /* examine least significant digit */
        -- end;
        if ( ! isdigit ( * end ) )
            break;

        /* any digit 0-9 is okay as least significant */
        num_start = end;

        /* examine remaining digits */
        while ( end > name )
        {
            /* require 0-9 to stay within parse */
            -- end;
            if ( ! isdigit ( * end ) )
                break;

            /* only reset numeric start if not 0 */
            if ( * end != '0' )
                num_start = end;
        }

        /* should have stopped on ':' */
        if ( *end != ':' )
            break;

        /* we have a numeric token
           due to oddities, exclude leading zeros from numeral */
        -- tok;
        spot_name_tok[tok].s.token_type = types[tok];
        spot_name_tok[tok].s.position = num_start - name;
        spot_name_tok[tok].s.length = num_end - num_start;
    }

    /* if all tokens were seen, the parse was good */
    if( tok == 0 )
        rslt->elem_count = EXPECTED_NUMBER_OF_TOKENS;
    else
    {
        /* otherwise, treat entire string as unrecognized */
        spot_name_tok[0].s.token_type = nt_unrecognized;
        spot_name_tok[0].s.position = 0;
        spot_name_tok[0].s.length = argv[0].u.data.elem_count;
        rslt->elem_count = 1;
    }

    return 0;
}

/* tokenize_spot_name
 *  scans name on input
 *  tokenizes into parts

 extern function NCBI:SRA:spot_name_token
    NCBI:SRA:IonTorrent:tokenize_spot_name #1 ( ascii name );
 */
VTRANSFACT_IMPL ( NCBI_SRA_IonTorrent_tokenize_spot_name, 1, 0, 0 ) ( const void *self,
                  const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->variant = vftRow;
    rslt->u.rf = tokenize_spot_name_IonTorrent;
    
    return 0;
}
