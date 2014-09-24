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
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/schema.h>

#include <klib/defs.h>
#include <klib/rc.h>

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <insdc/insdc.h>
#include <insdc/sra.h>
#include <ncbi/wgs-contig.h>

static
rc_t build_read_type(INSDC_SRA_xread_type dst[],
                     unsigned const components,
                     NCBI_WGS_component_props const Props[/* components */])
{
    unsigned i;
    
    for (i = 0; i != components; ++i) {
        int const props = Props[i];
        
        if (props < 0) {
            dst[i] = SRA_READ_TYPE_TECHNICAL;
        }
        else {
            int const type = props & 0x0F;
            
            if (type == 0) {
                int const strand = props & ~(NCBI_WGS_strand_plus | NCBI_WGS_strand_minus);
                
                switch (strand) {
                case 0:
                    dst[i] = SRA_READ_TYPE_BIOLOGICAL;
                    break;
                case NCBI_WGS_strand_plus:
                    dst[i] = SRA_READ_TYPE_BIOLOGICAL | SRA_READ_TYPE_FORWARD;
                    break;
                case NCBI_WGS_strand_minus:
                    dst[i] = SRA_READ_TYPE_BIOLOGICAL | SRA_READ_TYPE_REVERSE;
                    break;
                default:
                    return RC(rcXF, rcFunction, rcExecuting, rcType, rcInvalid);
                }
            }
            else
                return RC(rcXF, rcFunction, rcExecuting, rcType, rcInvalid);
        }
    }
    return 0;
}

/*
 * function INSDC:SRA:xread_type NCBI:WGS:build_read_type #1
 *     ( NCBI:WGS:component_props component_props )
 */

static
rc_t CC build_read_type_impl(void *self, VXformInfo const *info, int64_t row_id,
                             VFixedRowResult const *rslt,
                             uint32_t argc, VRowData const argv[])
{
    NCBI_WGS_component_props const *const props = argv[0].u.data.base;
    unsigned const components = argv[0].u.data.elem_count;
    INSDC_SRA_xread_type *const dst = rslt->base;

    return build_read_type(&dst[rslt->first_elem], components,
                           &props[argv[0].u.data.first_elem]);
}

VTRANSFACT_IMPL(NCBI_WGS_build_read_type, 1, 0, 0)(void const *const Self,
                                                   VXfactInfo const *const info,
                                                   VFuncDesc *const rslt,
                                                   VFactoryParams const *const cp,
                                                   VFunctionParams const *const dp)
{
    rslt->variant = vftFixedRow;
    rslt->u.pf = build_read_type_impl;
    return 0;
}
