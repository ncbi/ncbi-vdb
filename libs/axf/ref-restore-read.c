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

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/debug.h>

#include <vdb/cursor.h>
#include <vdb/database.h> /* VDatabaseRelease */
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/table.h>
#include <vdb/vdb-priv.h> /* VDatabaseGetAccession */
#include <vdb/xform.h>

#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <insdc/insdc.h>
#include <bitstr.h>
#include <sysalloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "restore-read.h"

#ifdef _DEBUGGING
#define SUB_DEBUG(msg) DBGMSG(DBG_SRA,DBG_FLAG(DBG_SRA_SUB),msg)
#else
#define SUB_DEBUG(msg)
#endif

static
rc_t CC ref_restore_read_impl ( void *data, const VXformInfo *info, int64_t row_id,
                                VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    RestoreRead *const self = data;
    uint8_t* dst;
    uint32_t read_len     = (uint32_t)argv[ 0 ].u.data.elem_count;
    const uint8_t *read   = argv[ 0 ].u.data.base;
    uint32_t seqid_len    = (uint32_t)argv[ 1 ].u.data.elem_count;
    const char* seqid     = argv[ 1 ].u.data.base;
    INSDC_coord_one   seq_start;
    INSDC_coord_len   seq_len;

    assert( argv[ 0 ].u.data.elem_bits == 8 );
    assert( argv[ 1 ].u.data.elem_bits == 8 );
    assert( argv[ 2 ].u.data.elem_bits == sizeof( INSDC_coord_one ) * 8 );
    assert( argv[ 3 ].u.data.elem_bits == sizeof( INSDC_coord_len ) * 8 );
    assert( argv[ 2 ].u.data.elem_count == 1 );
    assert( argv[ 3 ].u.data.elem_count == 1 );

    read   += argv [ 0 ] . u . data . first_elem;
    seqid  += argv [ 1 ] . u . data . first_elem;
    seq_start = ( ( INSDC_coord_one* )argv[ 2 ].u.data.base )[ argv[ 2 ].u.data.first_elem ];
    seq_len   = ( ( INSDC_coord_len* )argv[ 3 ].u.data.base )[ argv[ 3 ].u.data.first_elem ];

    if ( seq_len < read_len )
    {
        rc = RC( rcXF, rcFunction, rcExecuting, rcData, rcInvalid );
    }
    else
    {
        /* resize output row for the total number of bases */    
        rslt->data->elem_bits = 8;
        rc = KDataBufferResize( rslt->data, seq_len );
        if ( rc == 0 )
        {
            rslt->elem_count = seq_len;
            dst = rslt->data->base;

            if ( seq_len > 0 )
            {
                if ( read_len > 0 )
                {
                    memmove( dst, read, read_len );
                    if ( read_len < seq_len )
                    {
                        memset( dst + read_len, 15, seq_len - read_len ); /* pad with 'N' */
                    }
                }
                else if ( seq_start == 0 )
                {
                    memset( dst, 15, seq_len ); /* fill with 'N' */
                }
                else
                {
                    unsigned read2 = 0;

                    SUB_DEBUG( ( "SUB.Rd in 'ref_restore_read.c' at: %.*s at %u.%u\n", seqid_len, seqid, seq_start, seq_len ) );
                    rc = RestoreReadGetSequence(self, seq_start - 1, seq_len, dst, seqid_len, seqid, &read2, info->tbl);
                    if ( rc == 0 )
                    {
                        if ( read2 != seq_len )
                        {
                            rc = RC( rcXF, rcFunction, rcExecuting, rcData, read2 < seq_len ? rcTooShort : rcTooLong );
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/* 
 * function
 * INSDC:4na:bin ALIGN:ref_restore_read #1 (  INSDC:4na:bin rd , ascii seq_id , INSDC:coord:one seq_start, INSDC:coord:len seq_len);
 */
VTRANSFACT_IMPL ( ALIGN_ref_restore_read, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
                                                     VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rc_t rc = 0;
    RestoreRead *fself = RestoreReadMake(info->mgr, &rc);
    if ( rc == 0 )
    {
        rslt->self = fself;
        rslt->u.ndf = ref_restore_read_impl;
        rslt->variant = vftRow;
        rslt -> whack = RestoreReadFree;
    }
    return rc;
}
