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

#define WRITE_SIDE 0
#include "rna-flag-common.c"

static
rc_t CC use_rna_flag ( void *Self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{   /* meta<RNA_FLAG> == "0" or not set: no translation to be done
       meta<RNA_FLAG> == "1": translate 'T' to 'U' */
    struct Self *const self = Self;
    rc_t rc;
    uint32_t row_len = (uint32_t) argv [ 0 ] . u . data . elem_count;
    uint8_t const *seq = argv [ 0 ] . u . data . base;

    assert(argv[0].u.data.elem_bits == 8);
    seq += argv [ 0 ] . u . data . first_elem;

    rslt -> data -> elem_bits = 8;
    rslt -> elem_count = row_len;
    rc = KDataBufferResize ( rslt -> data, row_len );
    if (rc == 0)
        rc = self->function(self, rslt->data->base, seq, row_len);

    return rc;
}

/* useRnaFlag
 *  if metadata(RNA) is set translate T to U
 *  otherwise, do nothing
 *
 * extern function INSDC:dna:text NCBI:sra:useRnaFlag ( INSDC:dna:text in_read );
 */

VTRANSFACT_IMPL ( NCBI_SRA_useRnaFlag, 1, 0, 0 ) ( const void *Self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    const KMetadata * meta;
    rc_t rc = VTableOpenMetadataRead((VTable*)info->tbl, & meta);
    if (rc == 0) {
        rc = makeSelf(rslt, meta);
        if (rc == 0)
            rslt->u.rf = use_rna_flag;
    }
    return rc;
}
