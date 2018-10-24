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

#include "linker-priv.h"
#include "schema-parse.h"
#include "xform-priv.h"

#include <kfs/dyload.h>
#include <klib/token.h>
#include <klib/symtab.h>
#include <klib/symbol.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <kfg/config.h>

#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include <assert.h>

extern VTRANSFACT_DECL ( vdb_cast );
extern VTRANSFACT_DECL ( vdb_redimension );
extern VTRANSFACT_DECL ( vdb_row_id );
extern VTRANSFACT_DECL ( vdb_row_len );
extern VTRANSFACT_DECL ( vdb_fixed_row_len );
extern VTRANSFACT_DECL ( vdb_compare );
extern VTRANSFACT_DECL ( vdb_no_compare );
extern VTRANSFACT_DECL ( vdb_range_validate );
extern VTRANSFACT_DECL ( vdb_merge );
extern VTRANSFACT_DECL ( vdb_split );
extern VTRANSFACT_DECL ( vdb_transpose );
extern VTRANSFACT_DECL ( vdb_detranspose );
extern VTRANSFACT_DECL ( vdb_delta_average );
extern VTRANSFACT_DECL ( vdb_undelta_average );
extern VTRANSFACT_DECL ( meta_read );
extern VTRANSFACT_DECL ( meta_value );
extern VTRANSFACT_DECL ( meta_attr_read );
extern VTRANSFACT_DECL ( idx_text_project );
extern VTRANSFACT_DECL ( idx_text_lookup );
extern VTRANSFACT_DECL ( parameter_read );
extern VTRANSFACT_DECL ( environment_read );

/* newly imported things */
extern VTRANSFACT_DECL ( ALIGN_align_restore_read );
extern VTRANSFACT_DECL ( ALIGN_cigar );
extern VTRANSFACT_DECL ( ALIGN_cigar_2 );
extern VTRANSFACT_DECL ( ALIGN_generate_has_mismatch );
extern VTRANSFACT_DECL ( ALIGN_generate_mismatch );
extern VTRANSFACT_DECL ( ALIGN_generate_mismatch_qual );
extern VTRANSFACT_DECL ( ALIGN_project_from_sequence );
extern VTRANSFACT_DECL ( ALIGN_raw_restore_read );
extern VTRANSFACT_DECL ( ALIGN_ref_restore_read );
extern VTRANSFACT_DECL ( ALIGN_ref_sub_select );
extern VTRANSFACT_DECL ( ALIGN_seq_restore_read );
extern VTRANSFACT_DECL ( ALIGN_seq_restore_linkage_group );
extern VTRANSFACT_DECL ( INSDC_SEQ_rand_4na_2na );
extern VTRANSFACT_DECL ( INSDC_SRA_format_spot_name );
extern VTRANSFACT_DECL ( INSDC_SRA_format_spot_name_no_coord );
extern VTRANSFACT_DECL ( INSDC_SRA_read2spot_filter );
extern VTRANSFACT_DECL ( INSDC_SRA_spot2read_filter );
extern VTRANSFACT_DECL ( NCBI_SRA_ABI_tokenize_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_Helicos_tokenize_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_Illumina_tokenize_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_IonTorrent_tokenize_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_GenericFastq_tokenize_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA__454__dynamic_read_desc );
extern VTRANSFACT_DECL ( NCBI_SRA__454__process_position );
extern VTRANSFACT_DECL ( NCBI_SRA__454__tokenize_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_bio_start );
extern VTRANSFACT_DECL ( NCBI_SRA_bio_end );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_CLIP );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_INTENSITY );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_NOISE );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_POSITION );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_QUALITY );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_READ );
extern VTRANSFACT_DECL ( NCBI_SRA_decode_SIGNAL );
extern VTRANSFACT_DECL ( NCBI_SRA_denormalize );
extern VTRANSFACT_DECL ( NCBI_SRA_extract_coordinates );
extern VTRANSFACT_DECL ( NCBI_SRA_extract_name_coord );
extern VTRANSFACT_DECL ( NCBI_SRA_fix_read_seg );
extern VTRANSFACT_DECL ( NCBI_SRA_linker_from_readn );
extern VTRANSFACT_DECL ( NCBI_SRA_lookup );
extern VTRANSFACT_DECL ( NCBI_SRA_make_position );
extern VTRANSFACT_DECL ( NCBI_SRA_make_read_desc );
extern VTRANSFACT_DECL ( NCBI_SRA_make_spot_desc );
extern VTRANSFACT_DECL ( NCBI_SRA_make_spot_filter );
extern VTRANSFACT_DECL ( NCBI_SRA_normalize );
#if HAVE_PREFIX_TREE_TO_NAME
extern VTRANSFACT_DECL ( NCBI_SRA_prefix_tree_to_name );
#endif
extern VTRANSFACT_DECL ( NCBI_SRA_qual4_decode );
extern VTRANSFACT_DECL ( NCBI_SRA_qual4_decompress_v1 );
#if HAVE_READ_LEN_FROM_NREADS
extern VTRANSFACT_DECL ( NCBI_SRA_read_len_from_nreads );
extern VTRANSFACT_DECL ( NCBI_SRA_read_start_from_nreads );
#endif
extern VTRANSFACT_DECL ( NCBI_SRA_read_seg_from_readn );
extern VTRANSFACT_DECL ( NCBI_SRA_rewrite_spot_name );
extern VTRANSFACT_DECL ( NCBI_SRA_rotate );
extern VTRANSFACT_DECL ( NCBI_SRA_swap );
extern VTRANSFACT_DECL ( NCBI_SRA_syn_quality );

extern VTRANSFACT_DECL ( NCBI_WGS_build_read_type );
extern VTRANSFACT_DECL ( NCBI_WGS_build_scaffold_qual );
extern VTRANSFACT_DECL ( NCBI_WGS_build_scaffold_read );
extern VTRANSFACT_DECL ( NCBI_WGS_tokenize_nuc_accession );
extern VTRANSFACT_DECL ( NCBI_WGS_tokenize_prot_accession );

extern VTRANSFACT_DECL ( NCBI_align_clip );
extern VTRANSFACT_DECL ( NCBI_align_clip_2 );
extern VTRANSFACT_DECL ( NCBI_align_compress_quality );
extern VTRANSFACT_DECL ( NCBI_align_decompress_quality );
extern VTRANSFACT_DECL ( NCBI_align_edit_distance );
extern VTRANSFACT_DECL ( NCBI_align_edit_distance_2 );
extern VTRANSFACT_DECL ( NCBI_align_edit_distance_3 );
extern VTRANSFACT_DECL ( NCBI_align_generate_mismatch_qual_2 );
extern VTRANSFACT_DECL ( NCBI_align_generate_preserve_qual );
extern VTRANSFACT_DECL ( NCBI_align_get_clipped_cigar );
extern VTRANSFACT_DECL ( NCBI_align_get_clipped_cigar_2 );
extern VTRANSFACT_DECL ( NCBI_align_get_clipped_ref_offset );
extern VTRANSFACT_DECL ( NCBI_align_get_left_soft_clip );
extern VTRANSFACT_DECL ( NCBI_align_get_left_soft_clip_2 );
extern VTRANSFACT_DECL ( NCBI_align_get_mate_align_id );
extern VTRANSFACT_DECL ( NCBI_align_get_mismatch_read );
extern VTRANSFACT_DECL ( NCBI_align_get_ref_delete );
extern VTRANSFACT_DECL ( NCBI_align_get_ref_insert );
extern VTRANSFACT_DECL ( NCBI_align_get_ref_len );
extern VTRANSFACT_DECL ( NCBI_align_get_ref_len_2 );
extern VTRANSFACT_DECL ( NCBI_align_get_ref_mismatch );
extern VTRANSFACT_DECL ( NCBI_align_get_ref_preserve_qual );
extern VTRANSFACT_DECL ( NCBI_align_get_right_soft_clip );
extern VTRANSFACT_DECL ( NCBI_align_get_right_soft_clip_2 );
extern VTRANSFACT_DECL ( NCBI_align_get_right_soft_clip_3 );
extern VTRANSFACT_DECL ( NCBI_align_get_right_soft_clip_4 );
extern VTRANSFACT_DECL ( NCBI_align_get_right_soft_clip_5 );
extern VTRANSFACT_DECL ( NCBI_align_get_sam_flags );
extern VTRANSFACT_DECL ( NCBI_align_get_sam_flags_2 );
extern VTRANSFACT_DECL ( NCBI_align_get_seq_preserve_qual );
extern VTRANSFACT_DECL ( NCBI_align_local_ref_id );
extern VTRANSFACT_DECL ( NCBI_align_local_ref_start );
extern VTRANSFACT_DECL ( NCBI_align_make_cmp_read_desc );
extern VTRANSFACT_DECL ( NCBI_align_make_read_start );
extern VTRANSFACT_DECL ( NCBI_align_mismatch_restore_qual );
extern VTRANSFACT_DECL ( NCBI_align_not_my_row );
extern VTRANSFACT_DECL ( NCBI_align_raw_restore_qual );
extern VTRANSFACT_DECL ( NCBI_align_ref_name );
extern VTRANSFACT_DECL ( NCBI_align_ref_pos );
extern VTRANSFACT_DECL ( NCBI_align_ref_seq_id );
extern VTRANSFACT_DECL ( NCBI_align_ref_sub_select_preserve_qual );
extern VTRANSFACT_DECL ( NCBI_align_rna_orientation );
extern VTRANSFACT_DECL ( NCBI_align_seq_construct_read );
extern VTRANSFACT_DECL ( NCBI_align_template_len );
extern VTRANSFACT_DECL ( NCBI_color_from_dna );
extern VTRANSFACT_DECL ( NCBI_dna_from_color );
extern VTRANSFACT_DECL ( NCBI_SRA_useRnaFlag );
extern VTRANSFACT_DECL ( NCBI_fp_extend );
extern VTRANSFACT_DECL ( NCBI_lower_case_tech_reads );
extern VTRANSFACT_DECL ( NCBI_unpack );
extern VTRANSFACT_DECL ( NCBI_unzip );
extern VTRANSFACT_DECL ( NCBI_var_tokenize_var_id );
extern VTRANSFACT_DECL ( vdb_add_row_id );
extern VTRANSFACT_DECL ( vdb_bit_or );
extern VTRANSFACT_DECL ( vdb_bunzip );
extern VTRANSFACT_DECL ( vdb_ceil );
extern VTRANSFACT_DECL ( vdb_clip );
extern VTRANSFACT_DECL ( vdb_cut );
extern VTRANSFACT_DECL ( vdb_delta );
extern VTRANSFACT_DECL ( vdb_deriv );
extern VTRANSFACT_DECL ( vdb_diff );
extern VTRANSFACT_DECL ( vdb_echo );
extern VTRANSFACT_DECL ( vdb_exists );
extern VTRANSFACT_DECL ( vdb_extract_token );
extern VTRANSFACT_DECL ( vdb_fixed_vec_sum );
extern VTRANSFACT_DECL ( vdb_floor );
extern VTRANSFACT_DECL ( vdb_funzip );
extern VTRANSFACT_DECL ( vdb_integral );
extern VTRANSFACT_DECL ( vdb_integral_0 );
extern VTRANSFACT_DECL ( vdb_iunzip );
extern VTRANSFACT_DECL ( vdb_izip );
extern VTRANSFACT_DECL ( vdb_map );
extern VTRANSFACT_DECL ( vdb_max );
extern VTRANSFACT_DECL ( vdb_min );
extern VTRANSFACT_DECL ( vdb_outlier_decode );
extern VTRANSFACT_DECL ( vdb_outlier_encode );
extern VTRANSFACT_DECL ( vdb_pack );
extern VTRANSFACT_DECL ( vdb_paste );
extern VTRANSFACT_DECL ( vdb_rldecode );
extern VTRANSFACT_DECL ( vdb_round );
extern VTRANSFACT_DECL ( vdb_simple_sub_select );
extern VTRANSFACT_DECL ( vdb_simple_sub_select_1 );
extern VTRANSFACT_DECL ( vdb_sprintf );
extern VTRANSFACT_DECL ( vdb_strtonum );
extern VTRANSFACT_DECL ( vdb_subtract_row_id );
extern VTRANSFACT_DECL ( vdb_sum );
extern VTRANSFACT_DECL ( vdb_trim );
extern VTRANSFACT_DECL ( vdb_trunc );
extern VTRANSFACT_DECL ( vdb_undelta );
extern VTRANSFACT_DECL ( vdb_unpack );
extern VTRANSFACT_DECL ( vdb_unzip );
extern VTRANSFACT_DECL ( vdb_vec_sum );

struct KTable;
struct KMetadata;
extern bool CC NCBI_SRA_accept_untyped ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA__454__untyped_0 ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA__454__untyped_1_2a ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA__454__untyped_1_2b ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA_Illumina_untyped_0a ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA_Illumina_untyped_0b ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA_Illumina_untyped_1a ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA_Illumina_untyped_1b ( struct KTable const *tbl, struct KMetadata const *meta );
extern bool CC NCBI_SRA_ABI_untyped_1 ( struct KTable const *tbl, struct KMetadata const *meta );

/* these functions need something to fill in VFuncDesc */
static
rc_t CC fake_stub_func ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    assert(!"THIS FUNCTION IS NEVER TO BE CALLED");
    abort();
    return 0;
}

/* select is REALLY internal */
VTRANSFACT_BUILTIN_IMPL ( vdb_select, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    /* set function pointer to non-NULL */
    rslt -> u . rf = fake_stub_func;
    rslt -> variant = vftSelect;
    return 0;
}

#if THIS_IS_A_STUB_FOR_COPYPASTA
/*
function < type T >
T passthru #1.0 ( T target )
     = vdb:stub:function;
 */
/* all pass-through functions are REALLY internal */
VTRANSFACT_BUILTIN_IMPL ( vdb_stub_function, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    bool shouldPassThru = false;

    /* pass-through functions MUST have exactly one argument */
    assert(dp->argc == 1);
    
    /* This is where to implement logic to determine if pass-through is wanted
     */

    /* from here to the end of the function stays the same */
    if (shouldPassThru) {
        /* set function pointer to non-NULL */
        rslt -> u . rf = fake_stub_func;
        
        /* this is the magic that causes VFunctionProdRead to pass the Read
         * message on to the first argument of this function */
        rslt -> variant = vftPassThrough;
        return 0;
    }

    /* some non-zero RC to signal that pass-through is NOT supposed to happen
     * this will cause the schema to fall-through to the next rule or fail the
     * production
     */
    return SILENT_RC(rcVDB, rcFunction, rcConstructing, rcFunction, rcIgnored);
}
#endif

/* Pass-through function constructors should use this to prepare the result */
static rc_t maybePass(bool const shouldPass, VFuncDesc *const rslt)
{
    if (shouldPass) {
        rslt -> u . rf = fake_stub_func;
        rslt -> variant = vftPassThrough;
        return 0;
    }
    return SILENT_RC(rcVDB, rcFunction, rcConstructing, rcFunction, rcIgnored);
}

/* This function ALWAYS passes through
function < type T >
T passthru #1.0 ( T target )
     = vdb:passthru;
 */
/* all pass-through functions are REALLY internal */
VTRANSFACT_BUILTIN_IMPL ( vdb_passthru, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    return maybePass(true, rslt);
}

static bool compare_node_value(KConfigNode const *node, unsigned const len, char const *const value)
{
    char buffer[4096];
    size_t num_read = 0, remaining = 0, offset = 0;

    do {
        KConfigNodeRead(node, offset, buffer, sizeof(buffer), &num_read, &remaining);
        if (offset + num_read + remaining != len)
            return false;
        assert(offset + num_read <= len);
        if (memcmp(buffer, value + offset, num_read) != 0)
            return false;
        offset += num_read;
    } while (num_read > 0 && remaining > 0);
    return offset == len;
}

static bool check_config_node(VFactoryParams const * const name_value)
{
    rc_t rc = 0;
    bool result = false;
    assert(name_value->argc == 2);
    {
        KConfig *cfg;
        rc = KConfigMake(&cfg, NULL);
        if (rc == 0) {
            KConfigNode const *node;
            rc = KConfigOpenNodeRead(cfg, &node, "%.*s"
                                     , (int)name_value->argv[0].count
                                     , name_value->argv[0].data.ascii);
            if (rc == 0) {
                result = compare_node_value(node
                                            , name_value->argv[1].count
                                            , name_value->argv[1].data.ascii);
                KConfigNodeRelease(node);
            }
            KConfigRelease(cfg);
        }
    }
    return result;
}

/*
function < type T >
T is_configuration_set #1.0 < ascii node, ascii value > ( T target )
     = vdb:is_configuration_set;
 */
/* all pass-through functions are REALLY internal */
VTRANSFACT_BUILTIN_IMPL ( vdb_is_configuration_set, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    return maybePass(check_config_node(cp), rslt);
}

/* temporary silly stuff
 */

static
rc_t CC hello_func ( void *self, const VXformInfo *info, int64_t row_id,
    VRowResult *rslt, uint32_t argc, const VRowData argv [] )
{
    char *func_hello = self;
    OUTMSG (( "%s - row id %ld\n", func_hello, row_id ));
    return 0;
}

VTRANSFACT_BUILTIN_IMPL ( vdb_hello, 1, 0, 0 ) ( const void *self,
    const VXfactInfo *info, VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    const char *fact_hello = "vdb:hello factory";
    const char *func_hello = "vdb:hello function";

    if ( cp -> argc > 0 )
    {
        fact_hello = cp -> argv [ 0 ] . data . ascii;
        if ( cp -> argc > 1 )
            func_hello = cp -> argv [ 1 ] . data . ascii;
    }

    rslt -> self = malloc ( strlen ( func_hello ) + 1 );
    if ( rslt -> self == NULL )
        return RC ( rcVDB, rcFunction, rcConstructing, rcMemory, rcExhausted );
    strcpy ( rslt -> self, func_hello );
    rslt -> whack = free;
    rslt -> u . rf = hello_func;
    rslt -> variant = vftRow;

    OUTMSG (( "%s - %u factory params, %u function params\n", fact_hello, cp -> argc, dp -> argc ));
    return 0;
}

/* InitFactories
 */
static
rc_t CC VLinkerEnterFactory ( KSymTable *tbl, const SchemaEnv *env,
    LFactory *lfact, const char *name )
{
    rc_t rc;

    KTokenSource src;
    KTokenText tt;
    KToken t;

    KTokenTextInitCString ( & tt, name, "VLinkerEnterFactory" );
    KTokenSourceInit ( & src, & tt );
    next_token ( tbl, & src, & t );

    rc = create_fqn ( tbl, & src, & t, env, ltFactory, lfact );
    if ( rc == 0 )
        lfact -> name = t . sym;

    return rc;
}


rc_t VLinkerAddFactories ( VLinker *self,
    const VLinkerIntFactory *fact, uint32_t count,
    KSymTable *tbl, const SchemaEnv *env )
{
    uint32_t i;
    for ( i = 0; i < count; ++ i )
    {
        rc_t rc;
        LFactory *lfact = malloc ( sizeof * lfact );
        if ( lfact == NULL )
            return RC ( rcVDB, rcFunction, rcRegistering, rcMemory, rcExhausted );

        /* invoke factory to get description */
        rc = ( * fact [ i ] . f ) ( & lfact -> desc );
        if ( rc != 0 )
        {
            free ( lfact );
            return rc;
        }

        /* I am intrinsic and have no dl symbol */
        lfact -> addr = NULL;
        lfact -> name = NULL;
        lfact -> external = false;

        /* add to linker */
        rc = VectorAppend ( & self -> fact, & lfact -> id, lfact );
        if ( rc != 0 )
        {
            LFactoryWhack ( lfact, NULL );
            return rc;
        }

        /* create name */
        rc = VLinkerEnterFactory ( tbl, env, lfact, fact [ i ] . name );
        if ( rc != 0 )
        {
            void *ignore;
            VectorSwap ( & self -> fact, lfact -> id, NULL, & ignore );
            LFactoryWhack ( lfact, NULL );
            return rc;
        }
    }

    return 0;
}


static
rc_t CC VLinkerEnterSpecial ( KSymTable *tbl, const SchemaEnv *env,
    LSpecial *lspec, const char *name )
{
    rc_t rc;

    KTokenSource src;
    KTokenText tt;
    KToken t;

    KTokenTextInitCString ( & tt, name, "VLinkerEnterSpecial" );
    KTokenSourceInit ( & src, & tt );
    next_token ( tbl, & src, & t );

    rc = create_fqn ( tbl, & src, & t, env, ltUntyped, lspec );
    if ( rc == 0 )
        lspec -> name = t . sym;

    return rc;
}

typedef struct VLinkerIntSpecial VLinkerIntSpecial;
struct VLinkerIntSpecial
{
    bool ( CC * f ) ( struct KTable const *tbl, struct KMetadata const *meta );
    const char *name;
};

static
rc_t VLinkerAddUntyped ( VLinker *self,
    const VLinkerIntSpecial *special, uint32_t count,
    KSymTable *tbl, const SchemaEnv *env )
{
    uint32_t i;
    for ( i = 0; i < count; ++ i )
    {
        rc_t rc;
        LSpecial *lspec = malloc ( sizeof * lspec );
        if ( lspec == NULL )
            return RC ( rcVDB, rcFunction, rcRegistering, rcMemory, rcExhausted );

        /* I am intrinsic and have no dl symbol */
        lspec -> addr = NULL;
        lspec -> name = NULL;
        lspec -> func = special [ i ] . f;

        /* add to linker */
        rc = VectorAppend ( & self -> special, & lspec -> id, lspec );
        if ( rc != 0 )
        {
            LSpecialWhack ( lspec, NULL );
            return rc;
        }

        /* create name */
        rc = VLinkerEnterSpecial ( tbl, env, lspec, special [ i ] . name );
        if ( rc != 0 )
        {
            void *ignore;
            VectorSwap ( & self -> special, lspec -> id, NULL, & ignore );
            LSpecialWhack ( lspec, NULL );
            return rc;
        }
    }

    return 0;
}

/* InitFactories
 */
rc_t VLinkerInitFactoriesRead ( VLinker *self,  KSymTable *tbl, const SchemaEnv *env )
{
    static VLinkerIntFactory fact [] =
    {
        { vdb_cast, "vdb:cast" },
        { vdb_redimension, "vdb:redimension" },
        { vdb_row_id, "vdb:row_id" },
        { vdb_row_len, "vdb:row_len" },
        { vdb_fixed_row_len, "vdb:fixed_row_len" },
        { vdb_select, "vdb:select" },
        { vdb_is_configuration_set, "vdb:is_configuration_set" },
        { vdb_passthru, "vdb:passthru" },
        { vdb_compare, "vdb:compare" },
        { vdb_no_compare, "vdb:no_compare" },
        { vdb_range_validate, "vdb:range_validate" },
        { vdb_merge, "vdb:merge" },
        { vdb_split, "vdb:split" },
        { vdb_transpose, "vdb:transpose" },
        { vdb_detranspose, "vdb:detranspose" },
        { vdb_delta_average, "vdb:delta_average" },
        { vdb_undelta_average, "vdb:undelta_average" },
        { meta_read, "meta:read" },
        { meta_value, "meta:value" },
        { meta_attr_read, "meta:attr:read" },
        { idx_text_project, "idx:text:project" },
        { idx_text_lookup, "idx:text:lookup" },
        { parameter_read, "parameter:read" },
/*        { environment_read, "environment:read" }, */

        { ALIGN_align_restore_read, "ALIGN:align_restore_read" },
        { ALIGN_cigar, "ALIGN:cigar" },
        { ALIGN_cigar_2, "ALIGN:cigar_2" },
        { ALIGN_generate_has_mismatch, "ALIGN:generate_has_mismatch" },
        { ALIGN_generate_mismatch, "ALIGN:generate_mismatch" },
        { ALIGN_generate_mismatch_qual, "ALIGN:generate_mismatch_qual" },
        { ALIGN_project_from_sequence, "ALIGN:project_from_sequence" },
        { ALIGN_raw_restore_read, "ALIGN:raw_restore_read" },
        { ALIGN_ref_restore_read, "ALIGN:ref_restore_read" },
        { ALIGN_ref_sub_select, "ALIGN:ref_sub_select" },
        { ALIGN_seq_restore_read, "ALIGN:seq_restore_read" },
        { ALIGN_seq_restore_linkage_group, "ALIGN:seq_restore_linkage_group" },
        { INSDC_SEQ_rand_4na_2na, "INSDC:SEQ:rand_4na_2na" },
        { INSDC_SRA_format_spot_name, "INSDC:SRA:format_spot_name" },
        { INSDC_SRA_format_spot_name_no_coord, "INSDC:SRA:format_spot_name_no_coord" },
        { INSDC_SRA_read2spot_filter, "INSDC:SRA:read2spot_filter" },
        { INSDC_SRA_spot2read_filter, "INSDC:SRA:spot2read_filter" },
        { NCBI_SRA_ABI_tokenize_spot_name, "NCBI:SRA:ABI:tokenize_spot_name" },
        { NCBI_SRA_Helicos_tokenize_spot_name, "NCBI:SRA:Helicos:tokenize_spot_name" },
        { NCBI_SRA_Illumina_tokenize_spot_name, "NCBI:SRA:Illumina:tokenize_spot_name" },
        { NCBI_SRA_IonTorrent_tokenize_spot_name, "NCBI:SRA:IonTorrent:tokenize_spot_name" },
        { NCBI_SRA_GenericFastq_tokenize_spot_name, "NCBI:SRA:GenericFastq:tokenize_spot_name" },
        { NCBI_SRA__454__dynamic_read_desc, "NCBI:SRA:_454_:dynamic_read_desc" },
        { NCBI_SRA__454__process_position, "NCBI:SRA:_454_:process_position" },
        { NCBI_SRA__454__tokenize_spot_name, "NCBI:SRA:_454_:tokenize_spot_name" },
        { NCBI_SRA_bio_start, "NCBI:SRA:bio_start" },
        { NCBI_SRA_bio_end, "NCBI:SRA:bio_end" },
        { NCBI_SRA_decode_CLIP, "NCBI:SRA:decode:CLIP" },
        { NCBI_SRA_decode_INTENSITY, "NCBI:SRA:decode:INTENSITY" },
        { NCBI_SRA_decode_NOISE, "NCBI:SRA:decode:NOISE" },
        { NCBI_SRA_decode_POSITION, "NCBI:SRA:decode:POSITION" },
        { NCBI_SRA_decode_QUALITY, "NCBI:SRA:decode:QUALITY" },
        { NCBI_SRA_decode_READ, "NCBI:SRA:decode:READ" },
        { NCBI_SRA_decode_SIGNAL, "NCBI:SRA:decode:SIGNAL" },
        { NCBI_SRA_denormalize, "NCBI:SRA:denormalize" },
        { NCBI_SRA_extract_coordinates, "NCBI:SRA:extract_coordinates" },
        { NCBI_SRA_extract_name_coord, "NCBI:SRA:extract_name_coord" },
        { NCBI_SRA_fix_read_seg, "NCBI:SRA:fix_read_seg" },
#if HAVE_LINKER_FROM_READN
        { NCBI_SRA_linker_from_readn, "NCBI:SRA:linker_from_readn" },
#endif
        { NCBI_SRA_lookup, "NCBI:SRA:lookup" },
        { NCBI_SRA_make_position, "NCBI:SRA:make_position" },
        { NCBI_SRA_make_read_desc, "NCBI:SRA:make_read_desc" },
        { NCBI_SRA_make_spot_desc, "NCBI:SRA:make_spot_desc" },
        { NCBI_SRA_make_spot_filter, "NCBI:SRA:make_spot_filter" },
        { NCBI_SRA_normalize, "NCBI:SRA:normalize" },
#if HAVE_PREFIX_TREE_TO_NAME
        { NCBI_SRA_prefix_tree_to_name, "NCBI:SRA:prefix_tree_to_name" },
#endif
        { NCBI_SRA_qual4_decode, "NCBI:SRA:qual4_decode" },
        { NCBI_SRA_qual4_decompress_v1, "NCBI:SRA:qual4_decompress_v1" },
#if HAVE_READ_LEN_FROM_NREADS
        { NCBI_SRA_read_len_from_nreads, "NCBI:SRA:read_len_from_nreads" },
        { NCBI_SRA_read_start_from_nreads, "NCBI:SRA:read_start_from_nreads" },
#endif
        { NCBI_SRA_read_seg_from_readn, "NCBI:SRA:read_seg_from_readn" },
        { NCBI_SRA_rewrite_spot_name, "NCBI:SRA:rewrite_spot_name" },
        { NCBI_SRA_rotate, "NCBI:SRA:rotate" },
        { NCBI_SRA_swap, "NCBI:SRA:swap" },
        { NCBI_SRA_syn_quality, "NCBI:SRA:syn_quality" },

        { NCBI_WGS_build_read_type, "NCBI:WGS:build_read_type" },
        { NCBI_WGS_build_scaffold_qual, "NCBI:WGS:build_scaffold_qual" },
        { NCBI_WGS_build_scaffold_read, "NCBI:WGS:build_scaffold_read" },
        { NCBI_WGS_tokenize_nuc_accession, "NCBI:WGS:tokenize_nuc_accession" },
        { NCBI_WGS_tokenize_prot_accession, "NCBI:WGS:tokenize_prot_accession" },

        { NCBI_align_clip, "NCBI:align:clip" },
        { NCBI_align_clip_2, "NCBI:align:clip_2" },
        { NCBI_align_compress_quality, "NCBI:align:compress_quality" },
        { NCBI_align_decompress_quality, "NCBI:align:decompress_quality" },
        { NCBI_align_edit_distance, "NCBI:align:edit_distance" },
        { NCBI_align_edit_distance_2, "NCBI:align:edit_distance_2" },
        { NCBI_align_edit_distance_3, "NCBI:align:edit_distance_3" },
        { NCBI_align_generate_mismatch_qual_2, "NCBI:align:generate_mismatch_qual_2" },
        { NCBI_align_generate_preserve_qual, "NCBI:align:generate_preserve_qual" },
        { NCBI_align_get_clipped_cigar, "NCBI:align:get_clipped_cigar" },
        { NCBI_align_get_clipped_cigar_2, "NCBI:align:get_clipped_cigar_2" },
        { NCBI_align_get_clipped_ref_offset, "NCBI:align:get_clipped_ref_offset" },
        { NCBI_align_get_left_soft_clip, "NCBI:align:get_left_soft_clip" },
        { NCBI_align_get_left_soft_clip_2, "NCBI:align:get_left_soft_clip_2" },
        { NCBI_align_get_mate_align_id, "NCBI:align:get_mate_align_id" },
        { NCBI_align_get_mismatch_read, "NCBI:align:get_mismatch_read" },
        { NCBI_align_get_ref_delete, "NCBI:align:get_ref_delete" },
        { NCBI_align_get_ref_insert, "NCBI:align:get_ref_insert" },
        { NCBI_align_get_ref_len, "NCBI:align:get_ref_len" },
        { NCBI_align_get_ref_len_2, "NCBI:align:get_ref_len_2" },
        { NCBI_align_get_ref_mismatch, "NCBI:align:get_ref_mismatch" },
        { NCBI_align_get_ref_preserve_qual, "NCBI:align:get_ref_preserve_qual" },
        { NCBI_align_get_seq_preserve_qual, "NCBI:align:get_seq_preserve_qual" },
        { NCBI_align_get_right_soft_clip, "NCBI:align:get_right_soft_clip" },
        { NCBI_align_get_right_soft_clip_2, "NCBI:align:get_right_soft_clip_2" },
        { NCBI_align_get_right_soft_clip_3, "NCBI:align:get_right_soft_clip_3" },
        { NCBI_align_get_right_soft_clip_4, "NCBI:align:get_right_soft_clip_4" },
        { NCBI_align_get_right_soft_clip_5, "NCBI:align:get_right_soft_clip_5" },
        { NCBI_align_get_sam_flags, "NCBI:align:get_sam_flags" },
        { NCBI_align_get_sam_flags_2, "NCBI:align:get_sam_flags_2" },
        { NCBI_align_local_ref_id, "NCBI:align:local_ref_id" },
        { NCBI_align_local_ref_start, "NCBI:align:local_ref_start" },
        { NCBI_align_make_cmp_read_desc, "NCBI:align:make_cmp_read_desc" },
        { NCBI_align_make_read_start, "NCBI:align:make_read_start" },
        { NCBI_align_mismatch_restore_qual, "NCBI:align:mismatch_restore_qual" },
        { NCBI_align_not_my_row, "NCBI:align:not_my_row" },
        { NCBI_align_raw_restore_qual, "NCBI:align:raw_restore_qual" },
        { NCBI_align_ref_name, "NCBI:align:ref_name" },
        { NCBI_align_ref_pos, "NCBI:align:ref_pos" },
        { NCBI_align_ref_seq_id, "NCBI:align:ref_seq_id" },
        { NCBI_align_ref_sub_select_preserve_qual, "NCBI:align:ref_sub_select_preserve_qual" },
        { NCBI_align_rna_orientation, "NCBI:align:rna_orientation" },
        { NCBI_align_seq_construct_read, "NCBI:align:seq_construct_read" },
        { NCBI_align_template_len, "NCBI:align:template_len" },
        { NCBI_color_from_dna, "NCBI:color_from_dna" },
        { NCBI_dna_from_color, "NCBI:dna_from_color" },
        { NCBI_SRA_useRnaFlag, "NCBI:SRA:useRnaFlag" },
        { NCBI_fp_extend, "NCBI:fp_extend" },
        { NCBI_lower_case_tech_reads, "NCBI:lower_case_tech_reads" },
        { NCBI_unpack, "NCBI:unpack" },
        { NCBI_unzip, "NCBI:unzip" },
        { NCBI_var_tokenize_var_id, "NCBI:var:tokenize_var_id" },
        { vdb_add_row_id, "vdb:add_row_id" },
        { vdb_bit_or, "vdb:bit_or" },
        { vdb_bunzip, "vdb:bunzip" },
        { vdb_ceil, "vdb:ceil" },
        { vdb_clip, "vdb:clip" },
        { vdb_cut, "vdb:cut" },
        { vdb_delta, "vdb:delta" },
        { vdb_deriv, "vdb:deriv" },
        { vdb_diff, "vdb:diff" },
        { vdb_echo, "vdb:echo" },
        { vdb_exists, "vdb:exists" },
        { vdb_extract_token, "vdb:extract_token" },
        { vdb_fixed_vec_sum, "vdb:fixed_vec_sum" },
        { vdb_floor, "vdb:floor" },
        { vdb_funzip, "vdb:funzip" },
        { vdb_integral, "vdb:integral" },
        { vdb_integral_0, "vdb:integral_0" },
        { vdb_iunzip, "vdb:iunzip" },
        { vdb_izip, "vdb:izip" },
        { vdb_map, "vdb:map" },
        { vdb_max, "vdb:max" },
        { vdb_min, "vdb:min" },
        { vdb_outlier_decode, "vdb:outlier_decode" },
        { vdb_outlier_encode, "vdb:outlier_encode" },
        { vdb_pack, "vdb:pack" },
        { vdb_paste, "vdb:paste" },
        { vdb_rldecode, "vdb:rldecode" },
        { vdb_round, "vdb:round" },
        { vdb_simple_sub_select_1, "vdb:simple_sub_select_1" },
        { vdb_sprintf, "vdb:sprintf" },
        { vdb_strtonum, "vdb:strtonum" },
        { vdb_subtract_row_id, "vdb:subtract_row_id" },
        { vdb_sum, "vdb:sum" },
        { vdb_trim, "vdb:trim" },
        { vdb_trunc, "vdb:trunc" },
        { vdb_undelta, "vdb:undelta" },
        { vdb_unpack, "vdb:unpack" },
        { vdb_unzip, "vdb:unzip" },
        { vdb_vec_sum, "vdb:vec_sum" },

        { vdb_hello, "vdb:hello" }
    };

    static VLinkerIntSpecial special [] =
    {
        { NCBI_SRA_accept_untyped, "NCBI:SRA:accept_untyped" },
        { NCBI_SRA__454__untyped_0, "NCBI:SRA:_454_:untyped_0" },
        { NCBI_SRA__454__untyped_1_2a, "NCBI:SRA:_454_:untyped_1_2a" },
        { NCBI_SRA__454__untyped_1_2b, "NCBI:SRA:_454_:untyped_1_2b" },
        { NCBI_SRA_Illumina_untyped_0a, "NCBI:SRA:Illumina:untyped_0a" },
        { NCBI_SRA_Illumina_untyped_0b, "NCBI:SRA:Illumina:untyped_0b" },
        { NCBI_SRA_Illumina_untyped_1a, "NCBI:SRA:Illumina:untyped_1a" },
        { NCBI_SRA_Illumina_untyped_1b, "NCBI:SRA:Illumina:untyped_1b" },
        { NCBI_SRA_ABI_untyped_1, "NCBI:SRA:ABI:untyped_1" }
    };

    rc_t rc = VLinkerAddFactories ( self, fact, sizeof fact / sizeof fact [ 0 ], tbl, env );
    if ( rc == 0 )
        rc = VLinkerAddUntyped ( self, special, sizeof special / sizeof special [ 0 ], tbl, env );
    return rc;
}


/* MakeIntrinsic
 *  creates an initial, intrinsic linker
 *  pre-loaded with intrinsic factories
 */
rc_t VLinkerMakeIntrinsic ( VLinker **lp )
{
    KDyld *dl;
    rc_t rc = KDyldMake ( & dl );
    if ( rc == 0 )
    {
        rc = VLinkerMake ( lp, NULL, dl );
        KDyldRelease ( dl );

        if ( rc == 0 )
        {
            KSymTable tbl;
            VLinker *self = * lp;

            /* create symbol table with no intrinsic scope */
            rc = KSymTableInit ( & tbl, NULL );
            if ( rc == 0 )
            {
                SchemaEnv env;
                SchemaEnvInit ( & env, EXT_SCHEMA_LANG_VERSION );

                /* make intrinsic scope modifiable */
                KSymTablePushScope ( & tbl, & self -> scope );

                /* add intrinsic functions */
                rc = VLinkerInitFactories ( self, & tbl, & env );
                if ( rc == 0 )
                {
                    KSymTableWhack ( & tbl );
                    return 0;
                }

                KSymTableWhack ( & tbl );
            }

            VLinkerRelease ( self );
        }
    }

    * lp = NULL;

    return rc;
}
