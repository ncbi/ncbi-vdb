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
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "fix_read_seg_impl.h"

/* readn_read_desc
 *  "nreads" [ DATA ] - bases for entire spot
 */
typedef struct readn_read_desc readn_read_desc;
struct readn_read_desc
{
    uint32_t read_type;
    uint32_t read_start;
    uint32_t read_len;
    uint32_t label_start;
    uint32_t label_len;
};

static
rc_t CC equal_reads_from_spot ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t i, end;
    readn_read_desc *rdesc;
    KDataBuffer *dst = rslt -> data;

    const uint8_t *nreads = argv [ 0 ] . u . data . base;
    const uint32_t *spot_len = argv [ 1 ] . u . data . base;
    nreads += argv [ 0 ] . u . data . first_elem;
    spot_len += argv [ 1 ] . u . data . first_elem;

    dst -> elem_bits = sizeof * rdesc * 8;
    rc = KDataBufferResize ( dst, * nreads );
    if ( rc != 0 )
        return rc;

    rdesc = dst -> base;

    assert ( * spot_len > 0 );
    assert ( * nreads != 0 );

    for ( i = end = 0; i < * nreads; ++ i )
    {
        uint32_t rem = * nreads - i;
        uint32_t len = ( * spot_len - end + rem - 1 ) / rem;

        rdesc [ i ] . read_type = SRA_READ_TYPE_BIOLOGICAL;
        rdesc [ i ] . read_start = end;
        rdesc [ i ] . read_len = len;
        rdesc [ i ] . label_start = 0;
        rdesc [ i ] . label_len = 0;

        end += len;
    }

    rslt -> elem_count = * nreads;

    return 0;
}

static
rc_t CC read_seg_from_meta ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint8_t nreads;	
    uint32_t spot_len;
    const KDataBuffer *self = ( const void* ) data;
    const readn_read_desc *rd_src = self->base;
    KDataBuffer *dst = rslt -> data;
    readn_read_desc *rd_dst;;

    nreads   = ((uint8_t*) argv[0].u.data.base)[argv[0].u.data.first_elem];
    spot_len = ((uint32_t*)argv[1].u.data.base)[argv[1].u.data.first_elem];
    assert ( spot_len > 0 );
    assert ( nreads > 0 );

    rc = KDataBufferResize ( dst, nreads );
    if(rc != 0)
        return rc;

    rd_dst = dst->base;

    memcpy(rd_dst,rd_src,nreads*sizeof(*rd_dst));

    /*** if not covered by equal_reads_from_spot() - valid for illumina consisting of bio reads only **/
    if ( rd_dst[nreads-1].read_start+rd_dst[nreads-1].read_len==0)
    {
        int i;
        rd_dst[0].read_start=0;
        rd_dst[0].read_len=spot_len;
        for(i=1;i<nreads;i++)
        {
            rd_dst[i].read_start=spot_len;
            rd_dst[i].read_len=0;
        }
    }
    /*** illumina stop reads before reaching designed cycles ***/
    else if(rd_dst[nreads-1].read_start+rd_dst[nreads-1].read_len > spot_len)
    {
        int i;
        for(i=0;i<nreads;i++)
        {
            if(rd_dst[i].read_start > spot_len)
            {
                rd_dst[i].read_start = spot_len;
                rd_dst[i].read_len = 0;
            }
            else if(rd_dst[i].read_start + rd_dst[i].read_len > spot_len)
            {
                rd_dst[i].read_len = spot_len - rd_dst[i].read_start;
            }
        }
    }
    rslt -> elem_count = nreads;
    return 0;
}

static
rc_t parse_read_structure ( uint32_t idx, char *buf_val, readn_read_desc *rdesc )
{
    char    *rtype;
    char    *pos;
    int     fseqlen=0;
    char    *ptr;
    rc_t     rc;

    memset(rdesc,0,sizeof(*rdesc));

    pos = buf_val;

    rtype = strsep( &pos, "|" );
    if ( pos == NULL )
    {
        rc = RC ( rcSRA, rcTable, rcOpening, rcMetadata, rcCorrupt );
        PLOGERR ( klogErr, ( klogErr, rc, "bad read struct '$(name)' = '$(value)'",
                             "name=READ_%u,value=%s", idx, buf_val ));
        return rc;
    }

    rdesc -> read_len = atoi ( pos );
    strsep(&pos,"|");
    if ( pos == NULL )
    {
        rc = RC ( rcSRA, rcTable, rcOpening, rcMetadata, rcCorrupt );
        PLOGERR ( klogErr, ( klogErr, rc, "bad read struct '$(name)' = '$(value)'",
                             "name=READ_%u,value=%s", idx, buf_val ));
        return rc;
    }

    fseqlen = (int)strlen( pos );
    if(fseqlen > 0)
    {
        if(pos[fseqlen-1]=='|') fseqlen--;
        /* linker sequence is here */
    }

    switch ( rtype [ 0 ] )
    {
    case 'B':
        rdesc -> read_type = SRA_READ_TYPE_BIOLOGICAL;
        break;
    case 'T':
        rdesc -> read_type = SRA_READ_TYPE_TECHNICAL;
        break;
    default:
        rc = RC ( rcSRA, rcTable, rcOpening, rcMetadata, rcCorrupt );
        PLOGERR ( klogErr, ( klogErr, rc, "bad read struct '$(name)' = '$(value)'",
                             "name=READ_%u,value=%s", idx, buf_val ));
        return rc;
    }
    
    pos=rtype;
    ptr=strsep(&pos,":");
    if( pos != NULL )
    {
        ptr=strsep(&pos,":");
        /* label text is in ptr */
        rdesc -> label_len = (uint32_t)strlen ( ptr );
    }

    return 0;
}

static
void CC whack_data_buffer ( void *data )
{
    KDataBufferWhack ( data );
    free ( data );
}

VTRANSFACT_IMPL ( NCBI_SRA_read_seg_from_readn, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    const KMetadata *meta;
    rc_t rc = VTableOpenMetadataRead ( info -> tbl, & meta );
    if ( rc == 0 )
    {
        KDataBuffer *fself;
        readn_read_desc rd [ 16 ];
        int32_t i, last, tech_read_cnt;

        /* scan all metadata READ_N nodes */
        for ( last = -1, i = 0; i < sizeof rd / sizeof rd [ 0 ] && rc ==0; ++ i )
        {
            const KMDataNode *node;

            /* look for metadata descriptor */
            rc = KMetadataOpenNodeRead ( meta, & node, "READ_%u", i );
            if ( rc == 0 )
            {
                size_t bytes;
                char str [ 4096 ];
                rc = KMDataNodeReadCString ( node, str, sizeof str, & bytes );
                KMDataNodeRelease ( node );
                if ( rc == 0 )
                {
                    rc = parse_read_structure ( i, str, & rd [ i ] );
                    if ( rc != 0 )
                        break;
                    last = i;
                }
            }

        }

        /* ignore all other errors */
        rc = 0;

        KMetadataRelease ( meta );

        /* initial starts are always at 0 */
        rd [ 0 ] . read_start = 0;
        rd [ 0 ] . label_start = 0;

        /* handle case when none-exist */
        if ( last < 0 )
        {
            rslt -> u . ndf = equal_reads_from_spot;
            rslt -> variant = vftNonDetRow;
            return 0;
        }

        /* integrate lengths and count technical reads*/
        for ( i = tech_read_cnt = 0; i < last; ++ i )
        {
            if ( rd [ i ] . read_type == SRA_READ_TYPE_TECHNICAL)
                ++ tech_read_cnt;

            rd [ i + 1 ] . read_start = rd [ i ] . read_start + rd [ i ] . read_len;
            rd [ i + 1 ] . label_start = rd [ i ] . label_start + rd [ i ] . label_len;
        }

        /* temporary for cases where no read length has been given */
        if ( tech_read_cnt == 0 && rd [ last ] . read_start + rd [ last ] . read_len == 0 )
        {
            rslt -> u . ndf = equal_reads_from_spot;
            rslt -> variant = vftNonDetRow;
            return 0;
        }

        /* now produce a static row */
        fself = malloc ( sizeof * fself );
        if ( fself == NULL )
            rc = RC ( rcSRA, rcFunction, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KDataBufferMake ( fself, sizeof rd [ 0 ] * 8, ++ last );
            if ( rc == 0 )
            {
                memcpy ( fself -> base, rd, last * sizeof rd [ 0 ] );
                rslt -> self = fself;
                rslt -> whack = whack_data_buffer;
                rslt -> u . ndf = read_seg_from_meta;
                rslt -> variant = vftNonDetRow;
                return 0;
            }

            free ( fself );
        }
    }

    return rc;
}


static
rc_t CC fix_read_seg ( void *data, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;

    const uint16_t *rs_src = argv [ 0 ] . u . data . base;
    const uint32_t nreads = argv [ 0 ] . u . data . elem_count;
    const uint32_t *spot_lenp = argv [ 1 ] . u . data . base;
    const uint32_t spot_len = spot_lenp [ argv [ 1 ] . u . data . first_elem ];

    rs_src += argv [ 0 ] . u . data . first_elem;

    assert ( spot_len > 0 );
    assert ( nreads > 0);
    
    rc = KDataBufferResize ( rslt -> data, nreads );
    if ( rc == 0 )
    {
        uint32_t * rs_dst = rslt -> data -> base;
        fix_read_seg_impl(rs_src, nreads, spot_len, rs_dst);
        rslt -> elem_count = nreads;
    }
    return rc;
}

VTRANSFACT_IMPL ( NCBI_SRA_fix_read_seg, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt -> variant = vftRow;
    rslt -> u . rf = fix_read_seg;
    return 0;
}
