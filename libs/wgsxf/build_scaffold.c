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
#include <kdb/meta.h>
#include <klib/data-buffer.h>
#include <insdc/insdc.h>
#include <align/refseq-mgr.h>
#include <bitstr.h>
#include <sysalloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <insdc/sra.h>
#include <ncbi/wgs-contig.h>

#define READ_GAP_VALUE (0)
#define QUAL_GAP_VALUE (2)

#define CURSOR_CACHE_SIZE (4 * 1024 * 1024)

typedef struct self_t self_t;
typedef rc_t (*impl_f)(self_t const *const self, void *const Dst,
                       unsigned const components,
                       INSDC_coord_one const start[/* components */],
                       INSDC_coord_len const length[/* components */],
                       NCBI_WGS_component_props const props[/* components */],
                       int64_t const join[/* components */]);

struct self_t
{
    VCursor const *curs;
    uint32_t col_idx;
    impl_f impl;
};

static void free_self(self_t *const self)
{
    VCursorRelease(self->curs);
}

static
void CC whack(void *vp)
{
    if (vp) {
        free_self(vp);
        free(vp);
    }
}


static bool does_table_have_column( VTable const * tbl, char const column[] )
{
    KNamelist * column_names;
    bool res = false;
    rc_t rc = VTableListReadableColumns ( tbl, &column_names );
    if ( rc == 0 )
    {
        uint32_t count;
        rc = KNamelistCount ( column_names, &count );
        if ( rc == 0 && count > 0 )
        {
            uint32_t idx;
            size_t col_name_size;
            const char * col_name = string_chr ( column, string_size( column ), ')' );
            if ( col_name == NULL )
                col_name = column;
            else
                col_name++;
            col_name_size = string_size( col_name );
            for ( idx = 0; idx < count && rc == 0 && !res; ++idx )
            {
                const char * name;
                rc = KNamelistGet ( column_names, idx, &name );
                if ( rc == 0 && name != NULL )
                {
                    int cmp = string_cmp( col_name, col_name_size,
                                          name, string_size( name ), 0xFFFF );
                    if ( cmp == 0 )
                        res = true;
                }
            }
        }
        KNamelistRelease ( column_names );
    }
    return res;
}


static
rc_t init_self( self_t *self, VTable const * const srctbl, char const column[] )
{
    VDatabase const * db;
    rc_t rc = VTableOpenParentRead( srctbl, &db );
    if ( rc == 0 )
    {
        VTable const * tbl;
        rc = VDatabaseOpenTableRead( db, &tbl, "SEQUENCE" );
        VDatabaseRelease( db );
        if ( rc == 0 )
        {
            bool has_column = does_table_have_column( tbl, column );
            if ( !has_column )
                VTableRelease( tbl );
            else
            {
                VCursor const * curs;
#if CURSOR_CACHE_SIZE
                rc = VTableCreateCachedCursorRead( tbl, &curs, CURSOR_CACHE_SIZE );
#else
                rc = VTableCreateCursorRead( tbl, &curs );
#endif
                VTableRelease( tbl );
                if ( rc == 0 )
                {
                    uint32_t col_idx;
                    rc = VCursorAddColumn( curs, &col_idx, "%s", column );
                    if ( rc == 0 )
                    {
                        rc = VCursorOpen( curs );
                        if ( rc == 0 )
                        {
                            self->curs = curs;
                            self->col_idx = col_idx;
                            return 0;
                        }
                        if ( GetRCObject( rc ) == (enum RCObject)rcColumn && GetRCState( rc ) == rcUndefined )
                            rc = 0;
                    }
                    VCursorRelease( curs );
                }
            }
        }
    }
    return rc;
}

static
rc_t build_scaffold_read_impl(self_t const *const self, void *const Dst,
                              unsigned const components,
                              INSDC_coord_one const Start[/* components */],
                              INSDC_coord_len const Length[/* components */],
                              NCBI_WGS_component_props const Props[/* components */],
                              int64_t const join[/* components */])
{
    INSDC_4na_bin *const dst = Dst;
    unsigned i;
    unsigned j;
    unsigned id;
    rc_t rc;
    
    for (rc = 0, id = j = i = 0; rc == 0 && i != components; ++i) {
        INSDC_coord_len const length = Length[i];
        int const props = Props[i];

        if (props < 0) {
            /* gap */
            memset(dst + j, READ_GAP_VALUE, length);
        }
        else if (self->curs == NULL) {
            memset(dst + j, 15, length);
        }
        else {
            int const type = props & 0x0F;
            int const strand = (props & ~(NCBI_WGS_strand_plus | NCBI_WGS_strand_minus)) >> 4;
            
            if (type != 0 || strand == 3)
                rc = RC(rcXF, rcFunction, rcExecuting, rcType, rcInvalid);
            else {
                int64_t const row = join[id++];
                uint32_t elem_bits;
                uint32_t bit_offset;
                uint32_t elem_count;
                void const *base;
                
                rc = VCursorCellDataDirect(self->curs, row, self->col_idx,
                                           &elem_bits, &base, &bit_offset,
                                           &elem_count);
                assert(bit_offset == 0);
                if (rc == 0) {
                    INSDC_coord_one const start = Start[i] - 1;
                    
                    if (elem_count < start + length)
                        rc = RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                    else {
                        INSDC_4na_bin const *const src = base;
                        
                        if (strand == 2) {
                            static INSDC_4na_bin const complement[] = {
                                /* 0  0000 - 0000*/  0,
                                /* 1  0001 - 1000*/  8,
                                /* 2  0010 - 0100*/  4,
                                /* 3  0011 - 1100*/ 12,
                                /* 4  0100 - 0010*/  2,
                                /* 5  0101 - 1010*/ 10,
                                /* 6  0110 - 0110*/  6,
                                /* 7  0111 - 1110*/ 14,
                                /* 8  1000 - 0001*/  1,
                                /* 9  1001 - 1001*/  9,
                                /*10  1010 - 0101*/  5,
                                /*11  1011 - 1101*/ 13,
                                /*12  1100 - 0011*/  3,
                                /*13  1101 - 1011*/ 11,
                                /*14  1110 - 0111*/  7,
                                /*15  1111 - 1111*/ 15
                            };
                            unsigned k;
                            unsigned jj;
                            
                            for (jj = j + length, k = 0; k != length; ++k) {
                                INSDC_4na_bin const elem = src[start + k];
                                
                                assert(/* 0 <= elem && */ elem <= 15);
                                --jj;
                                dst[jj] = complement[elem];
                            }
                        }
                        else
                            memmove(&dst[j], &src[start], length);
                    }
                }
            }
        }

        j += length;
    }
    return rc;
}

static
rc_t build_scaffold_qual_impl(self_t const *const self, void *const Dst,
                              unsigned const components,
                              INSDC_coord_one const Start[/* components */],
                              INSDC_coord_len const Length[/* components */],
                              NCBI_WGS_component_props const Props[/* components */],
                              int64_t const join[/* components */])
{
    INSDC_quality_phred *const dst = Dst;
    unsigned i;
    unsigned j;
    unsigned id;
    rc_t rc;
    
    for (rc = 0, id = j = i = 0; rc == 0 && i != components; ++i) {
        INSDC_coord_len const length = Length[i];
        int const props = Props[i];
        
        if (props < 0) {
            /* gap */
            memset(dst + j, QUAL_GAP_VALUE, length);
        }
        else if (self->curs == NULL) {
            memset(dst + j, 30, length);
        }
        else {
            int const type = props & 0x0F;
            int const strand = (props & ~(NCBI_WGS_strand_plus | NCBI_WGS_strand_minus)) >> 4;
            
            if (type != 0 || strand == 3)
                rc = RC(rcXF, rcFunction, rcExecuting, rcType, rcInvalid);
            else {
                int64_t const row = join[id++];
                uint32_t elem_bits;
                uint32_t bit_offset;
                uint32_t elem_count;
                void const *base;
                
                rc = VCursorCellDataDirect(self->curs, row, self->col_idx,
                                           &elem_bits, &base, &bit_offset,
                                           &elem_count);
                assert(bit_offset == 0);
                if (rc == 0) {
                    INSDC_quality_phred const start = Start[i] - 1;
                    
                    if (elem_count < start + length)
                        rc = RC(rcXF, rcFunction, rcExecuting, rcData, rcInsufficient);
                    else {
                        INSDC_quality_phred const *const src = base;
                        
                        if (strand == 2) {
                            unsigned k;
                            unsigned jj;
                            
                            for (jj = j + length, k = 0; k != length; ++k) {
                                INSDC_quality_phred const elem = src[start + k];
                                
                                --jj;
                                dst[jj] = elem;
                            }
                        }
                        else
                            memmove(&dst[j], &src[start], length);
                    }
                }
            }
        }
        
        j += length;
    }
    return rc;
}

static
unsigned total_length(unsigned const components, INSDC_coord_len const length[/* components */])
{
    unsigned i;
    unsigned len;
    
    for (i = len = 0; i != components; ++i)
        len += length[i];
    return len;
}


/*
 * function INSDC:4na:bin NCBI:WGS:build_scaffold_read #1
 *     ( INSDC:coord:one component_start, INSDC:coord:len component_len,
 *       NCBI:WGS:component_props component_props, I64 component_id )
 *
 * function INSDC:quality:phred NCBI:WGS:build_scaffold_qual #1
 *     ( INSDC:coord:one component_start, INSDC:coord:len component_len,
 *       NCBI:WGS:component_props component_props, I64 component_id )
 */

static
rc_t CC build_scaffold_impl(void *const data, VXformInfo const *const info,
                            int64_t const row_id, VRowResult *const rslt,
                            uint32_t const argc, VRowData const argv[])
{
    self_t const *const self = data;
    INSDC_coord_one const *const start = argv[0].u.data.base;
    INSDC_coord_len const *const length = argv[1].u.data.base;
    NCBI_WGS_component_props const *const props = argv[2].u.data.base;
    int64_t const *const join = argv[3].u.data.base;
    unsigned const components = argv[0].u.data.elem_count;
    rc_t rc;
    
    assert(argv[1].u.data.elem_count == components);
    assert(argv[2].u.data.elem_count == components);
    
    assert(start != NULL);
    assert(length != NULL);
    assert(props != NULL);
    assert(join != NULL);
    
    assert(rslt->elem_bits == 8);
    rslt->elem_count = total_length(components, &length[argv[1].u.data.first_elem]);
    rc = KDataBufferResize(rslt->data, rslt->elem_count);
    if (rc == 0)
        rc = self->impl(self, rslt->data->base, components,
                        &start[argv[0].u.data.first_elem],
                        &length[argv[1].u.data.first_elem],
                        &props[argv[2].u.data.first_elem],
                        &join[argv[3].u.data.first_elem]);
    return rc;
}

VTRANSFACT_IMPL(NCBI_WGS_build_scaffold_read, 1, 0, 0)(void const * Self,
                                                       VXfactInfo const * info,
                                                       VFuncDesc * rslt,
                                                       VFactoryParams const * cp,
                                                       VFunctionParams const * dp)
{
    self_t *self = calloc(1, sizeof(*self));
    if (self) {
        rc_t const rc = init_self(self, info->tbl, "(INSDC:4na:bin)READ");
        
        if (rc == 0) {
            self->impl = build_scaffold_read_impl;
            
            rslt->self = self;
            rslt->whack = whack;
            rslt->variant = vftRow;
            rslt->u.rf = build_scaffold_impl;
        }
        return rc;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}

VTRANSFACT_IMPL(NCBI_WGS_build_scaffold_qual, 1, 0, 0)(void const * Self,
                                                       VXfactInfo const * info,
                                                       VFuncDesc * rslt,
                                                       VFactoryParams const * cp,
                                                       VFunctionParams const * dp)
{
    self_t *self = calloc(1, sizeof(*self));
    if (self) {
        rc_t const rc = init_self(self, info->tbl, "(INSDC:quality:phred)QUALITY");
    
        if (rc == 0) {
            self->impl = build_scaffold_qual_impl;
            
            rslt->self = self;
            rslt->whack = whack;
            rslt->variant = vftRow;
            rslt->u.rf = build_scaffold_impl;
        }
        return rc;
    }
    return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);
}
