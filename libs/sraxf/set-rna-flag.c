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

#include <klib/rc.h>
#include <vdb/table.h>
#include <vdb/xform.h>
#include <kdb/meta.h>
#include <klib/data-buffer.h>

static
rc_t CC set_rna_flag ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    rc_t rc;
    uint32_t row_len = argv [ 0 ] . u . data . elem_count;
    const uint8_t *dnatext = argv [ 0 ] . u . data . base;

    assert(argv[0].u.data.elem_bits == 8);

    dnatext += argv [ 0 ] . u . data . first_elem;

    rslt -> data -> elem_bits = 8;
    rslt -> elem_count = row_len;
    rc = KDataBufferResize ( rslt -> data, row_len );
    if ( rc == 0 )
    {
        uint32_t i;
        uint8_t * result = rslt -> data -> base;
        KMetadata * meta = self;
        char buf[1];
        size_t num_read;
        size_t remaining;
        bool firstTime;
        KMDataNode * node;

        /* cannot keep the metadata node open for update between calls since
            use-rna-flag() will be trying to open it for read */
        rc = KMetadataOpenNodeUpdate ( meta, & node, "RNA_FLAG" );
        if ( rc == 0 )
        {
            rc = KMDataNodeRead ( node, 0, buf, 0, & num_read, & remaining );
            if ( rc == 0 )
            {
                firstTime = ( remaining == 0 );
                for ( i = 0; i < row_len; ++i )
                {
                    char ch = dnatext [ i ];
                    if ( firstTime && ( ch == 'U' || ch == 'T' ) )
                    {
                        rc = KMDataNodeWrite ( node, ch == 'U' ? "1" : "0", 1 );
                        firstTime = false;
                        if ( rc != 0 )
                        {
                            break;
                        }
                    }
                    result [ i ] = ( ch == 'U' ? 'T' : ch );
                }
            }

            if ( rc == 0 )
            {
                rc = KMDataNodeRelease ( node );
            }
            else
            {
                KMDataNodeRelease ( node );
            }
        }
    }

    return rc;
}

/* setRnaFlag
 *  if metadata(RNA) is not set and in_read contains u/U, set metadata(RNA) to true
 *  if metadata(RNA) is not set and in_read contains t/T, set metadata(RNA) to false
 *  otherwise, do nothing
 * return in_read
 *
 * extern function INSDC:dna:text NCBI:SRA:setRnaFlag ( INSDC:dna:text in_read );
 */
VTRANSFACT_IMPL ( NCBI_SRA_setRnaFlag, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    KMetadata * meta;
    rc_t rc = VTableOpenMetadataUpdate((VTable*)info->tbl, & meta);
    if ( rc == 0 )
    {
        rslt->u.rf = set_rna_flag;
        rslt->variant = vftRow;
        rslt->self = meta;
        rslt->whack = ( void ( * ) ( void * ) ) & KMetadataRelease;
    }
    return rc;
}
