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
#include <vdb/schema.h>
#include <klib/data-buffer.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <string.h>
#include <assert.h>


typedef uint16_t text_token;
enum { tt_id, tt_start, tt_len };


/* extract_token
 *  extract a textual token from an input string
 *
 *  "idx" [ CONST ] - a zero-based index of the token
 *  if value < row_len ( tok ), then the substring of
 *  indexed token is returned. otherwise, returns empty.
 *
 *  "str" [ DATA ] - input text. type must be compatible with
 *  output production, meaning types must be same, or ascii input
 *  with utf8 output.
 *
 *  "tok" [ DATA ] - results of tokenizing "str"
 */
static
rc_t CC extract_token ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    KDataBuffer *dst = rslt -> data;
    uint32_t idx = ( uint32_t ) ( size_t ) data;

    rslt -> elem_count = 0;
    if ( ( uint64_t ) idx >= argv [ 1 ] . u . data . elem_count )
    {
        /* issue empty string */
        rc = KDataBufferResize ( dst, 0 );
    }
    else
    {
        size_t sub_bytes, elem_bytes = ( size_t ) ( argv [ 0 ] . u . data . elem_bits >> 3 );
        const char *str = argv [ 0 ] . u . data . base;
        const text_token *tok = argv [ 1 ] . u . data . base;

        str += argv [ 0 ] . u . data . first_elem * elem_bytes;
        tok += ( argv [ 1 ] . u . data . first_elem + idx ) * 3;
        sub_bytes = elem_bytes * tok [ tt_len ];

        /* set output buffer size */
        KDataBufferCast ( dst, dst, rslt -> elem_bits, true );
        rc = KDataBufferResize ( dst, tok [ tt_len ] );
        if ( rc == 0 )
        {
            /* copy substring */
            rslt -> elem_count = tok [ tt_len ];
            memcpy ( dst -> base, & str [ elem_bytes * tok [ tt_start ] ], sub_bytes );
        }
    }

    return rc;
}

VTRANSFACT_IMPL ( vdb_extract_token, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* ensure that the type of input is either identical to
       or compatible with the output */
    if ( ! VTypedeclToTypedecl ( & dp -> argv [ 0 ] . fd . td, info -> schema, & info -> fdesc . fd . td, NULL, NULL ) )
        return RC ( rcXF, rcFunction, rcConstructing, rcType, rcInconsistent );

    rslt -> self = ( void* ) ( size_t ) cp -> argv [ 0 ] . data . u32 [ 0 ];
    rslt -> variant = vftRow;
    rslt -> u . rf = extract_token;
    return 0;
}
