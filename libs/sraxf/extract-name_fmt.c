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
#include "name-tokenizer.h"

#include <string.h>

/* extract_name_fmt
 *  generates input to .NAME_FMT column and/or updates skey index
 *
 *  on NCBI:SRA:name_token:unrecognized, produces an empty row
 *  otherwise, it creates a temporary "name_fmt" string from name row
 *
 *  an attempt is made to insert name_fmt into indicated text index
 *  ( normally 'skey' ). if the insert succeeds, i.e. associates "name_fmt"
 *  with a row_id, then the output for the row is empty.
 *
 *  if the insert fails due to key duplication, an attempt is made to
 *  extend the id range of associated rows. depending upon the type of index,
 *  this may succeed or fail, e.g. if the existing row range for "name_fmt" is
 *  n..m where m = row_id - 1, the range can be extended to n..row_id and
 *  the update succeeds. if the index supports discontiguous id ranges, the
 *  update will also succeed. upon any success updating the index, the output
 *  row will be empty.
 *
 *  finally, if the temporary "name_fmt" cannot be inserted into the index
 *  nor the existing id range updated, the output for the row will be "name_fmt".
 *
 *  "name" [ DATA ] - raw spot names from NAME column
 *
 *  "tok" [ DATA ] - delimiting tokens produced by sub-table
function ascii
    NCBI:SRA:extract_name_fmt #1 < ascii skey > ( ascii name, NCBI:SRA:spot_name_token tok );
 */

static
rc_t extract_name_fmt_2 (KIndex *ndx, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, const char *name, uint32_t namelen, const spot_name_token_t *tokens, uint32_t N )
{
    rc_t rc;
    bool recognized;
    uint32_t i, j, toksize, fmtsize, numtok;
    char sbuf[4096], *name_fmt = sbuf;

    for ( recognized = false, i = j = numtok = toksize = 0; i != N; ++i)
    {
        switch (tokens[i].s.token_type)
        {
        case nt_unrecognized:
            /* should be a single token, but in any event we don't care */
            return 0;
        case nt_recognized:
            /* should be a single token, but we'll check below */
            recognized = true;
            break;
	case nt_Q:
	case nt_X:
	case nt_Y:
            /* we should recognize the token */
            if ( tokens [ i ] . s . token_type >= nt_max_token )
                return RC ( rcSRA, rcToken, rcPositioning, rcName, rcOutofrange );

            /* enforce serial order of tokens */
            if ( tokens [ i ] . s . position < j )
                return RC ( rcSRA, rcToken, rcPositioning, rcName, rcInvalid );
            j = tokens [ i ] . s . position + tokens [ i ] . s . length;

            /* accumulate total size to be removed */
            toksize += tokens [ i ] . s . length;
	    numtok++;

            /* enforce that no token extends beyond end of name */
            if ( j > namelen )
                return RC ( rcSRA, rcToken, rcPositioning, rcName, rcOutofrange );
	    break;
        default: /*** exclude from computation ***/
		break;
        }
    }

    /* token stream cannot contain BOTH recognized and coordinates */
    if ( recognized && toksize != 0 )
        return RC ( rcSRA, rcToken, rcPositioning, rcName, rcInvalid );

    /* token stream must contain at least recognized or a non-empty coordinate */
    if ( ! recognized && toksize == 0 )
        return RC ( rcSRA, rcToken, rcPositioning, rcName, rcInvalid );

    /* calculate name_fmt buffer size */
    fmtsize = ( recognized ? namelen : namelen - toksize + numtok + numtok ) + 1;

    /* if local buffer is insufficient, allocate directly in output KDataBuffer */
    rslt -> data -> elem_bits = 8;
    if ( fmtsize > sizeof sbuf )
    {
        rc = KDataBufferResize ( rslt -> data, fmtsize );
        if ( rc != 0 )
            return rc;
        name_fmt = rslt -> data -> base;
    }

    /* create NUL-terminated formatted string */
    if ( recognized )
        string_copy ( name_fmt, fmtsize --, name, namelen );
    else
    {
        uint32_t total;
        for ( i = j = total = 0; i < N; ++i )
        {
            switch( tokens[i].s.token_type ) {
                case nt_Q:
                case nt_X:
                case nt_Y:
                    {{
                    uint32_t pos = tokens [ i ] . s . position;
                    toksize = tokens [ i ] . s . length;

                    /* detect unrecognized tokens to left */
                    if ( pos > j )
                    {
                        /* copy them in, NUL-terminating */
                        total += (uint32_t)string_copy ( & name_fmt [ total ],
                            fmtsize - total, & name [ j ], pos - j );
                    }

                    /* move marker along */
                    j = pos + toksize;

                    /* insert symbol */
                    name_fmt[total + 0] = '$';
                    name_fmt[total + 1] = TOK_ALPHABET[tokens[i].s.token_type];
                    name_fmt[total + 2] = 0;
                    total += 2;
                    }}
                    break;
                default:
                    break;
            }
        }

        /* detect unrecognized tokens to right */
        if ( j < namelen )
        {
            string_copy ( & name_fmt [ total ],
                fmtsize - total, & name [ j ], namelen - j );
        }
    }

    /* attempt to insert into index */
    rc = KIndexInsertText ( ndx, false, name_fmt, row_id );
    if ( rc != 0 )
    {
        /* insert failed for whatever reason - return name_fmt */
        fmtsize = strlen(name_fmt);
        if ( name_fmt == sbuf )
        {
            rc = KDataBufferResize ( rslt -> data, fmtsize );
            if ( rc != 0 )
                return rc;
            memcpy ( rslt -> data -> base, name_fmt, fmtsize );
        }

        rslt -> elem_count = fmtsize;
    }

    return 0;
}

static
rc_t CC extract_name_fmt( void *self, const VXformInfo *info, int64_t row_id,
                       VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    const char *name;
    uint32_t num_tokens, name_len;
    const spot_name_token_t *tokens;

    tokens = argv [ 1 ] . u . data . base;
    num_tokens = argv [ 1 ] . u . data . elem_count;
    tokens += argv [ 1 ] . u . data . first_elem;

    rslt -> elem_count = 0;
    if ( num_tokens == 0 )
        return 0;

    name = argv [ 0 ] . u . data . base;
    name_len = argv [ 0 ] . u . data . elem_count;
    name += argv [ 0 ] . u . data . first_elem;

    return extract_name_fmt_2(self, info, row_id, rslt,
        name, name_len, tokens, num_tokens);
}    

VTRANSFACT_IMPL ( NCBI_SRA_extract_name_fmt, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc;
    KIndex *ndx;
    
    rc = VTableCreateIndex ( ( VTable* ) info -> tbl, &ndx, kitText | kitProj, kcmOpen,
                             "%.*s", cp->argv[0].count, cp->argv[0].data.ascii );
    if( rc == 0 ) {
        rslt->self = ndx;
        rslt->whack = ( void ( CC * ) ( void* ) ) KIndexRelease;
        rslt->variant = vftNonDetRow;
        rslt->u.ndf = extract_name_fmt;
    }    
    return rc;
}
