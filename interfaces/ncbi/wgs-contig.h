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

#ifndef _h_ncbi_wgs_contig_
#define _h_ncbi_wgs_contig_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================================================
 * WGS Contig
 */


/*--------------------------------------------------------------------------
 * types
 * constants
 */

/* component_props
 *  a signed value describing either contigs or gaps
 *  where values >= 0 refer to contigs
 *  and negatives describe gaps
 */
typedef int16_t NCBI_WGS_component_props;

/* component description
 *  the sequencing status of the component
 *
 *  These typically correspond to keywords in the INSDC submission.
 *  Current acceptable values are_
 *    A           Active Finishing
 *    D           Draft HTG (often phase1 and phase2 are called Draft,
 *                whether or not they have the draft keyword).
 *    F           Finished HTG (phase3)
 *    G           Whole Genome Finishing
 *    O           Other sequence (typically means no HTG keyword)
 *    P           Pre Draft
 *    W           WGS contig
 */
enum
{
    NCBI_WGS_component_WGS                  = 0,
    NCBI_WGS_component_ActiveFinishing      = 1,
    NCBI_WGS_component_DraftHTG             = 2,
    NCBI_WGS_component_FinishedHTG          = 3,
    NCBI_WGS_component_WholeGenomeFinishing = 4,
    NCBI_WGS_component_OtherSequence        = 5,
    NCBI_WGS_component_PreDraft             = 6
};

/* strand
 *  specifies the orientation of the component relative to scaffold
 *  values given allow strand to be determined as "prop / 16"
 *  yielding_
 *    0           unknown orientation
 *    1           plus strand
 *    2           negative strand
 */
enum
{
    NCBI_WGS_strand_plus                    = 1 * 16,
    NCBI_WGS_strand_minus                   = 2 * 16
};


/* gap description
 *  These typically correspond to keywords in the INSDC submission.
 *  Current acceptable values are_
 *    N           gap with specified size
 *    U           gap of unknown size, defaulting to 100 bases.
 */
enum
{
    NCBI_WGS_gap_known                      = 1 * -1,
    NCBI_WGS_gap_unknown                    = 2 * -1
};

/* gap_linkage
 * Describes linkage and linkage evidence
 */
typedef int32_t NCBI_WGS_gap_linkage;

/* gap type
 *  scaffold          a gap between two sequence contigs in a scaffold
 *  contig            an unspanned gap between two sequence contigs
 *  centromere        a gap inserted for the centromere
 *  short_arm         a gap inserted at the start of an acrocentric chromosome
 *  heterochromatin   a gap inserted for an especially large region of heterochromatic sequence
 *  telomere          a gap inserted for the telomere
 *  repeat            an unresolvable repeat
 */
enum
{
    NCBI_WGS_gap_scaffold                   = 1 * -4,
    NCBI_WGS_gap_contig                     = 2 * -4,
    NCBI_WGS_gap_centromere                 = 3 * -4,
    NCBI_WGS_gap_short_arm                  = 4 * -4,
    NCBI_WGS_gap_heterochromatin            = 5 * -4,
    NCBI_WGS_gap_telomere                   = 6 * -4,
    NCBI_WGS_gap_repeat                     = 7 * -4
};

/* gap linkage evidence
 *  paired-ends       paired sequences from the two ends of a DNA fragment
 *  align_genus       alignment to a reference genome within the same genus
 *  align_xgenus      alignment to a reference genome within another genus
 *  align_trnscpt     alignment to a transcript from the same species
 *  within_clone      sequence on both sides of the gap is derived from
 *                    the same clone, but the gap is not spanned by paired-ends
 *  clone_contig      linkage is provided by a clone contig in the tiling path
 *  map               linkage asserted using a non-sequence based map
 *                    such as RH, linkage, fingerprint or optical
 *  strobe            strobe sequencing (PacBio)
 *  unspecified
 */
enum
{
    NCBI_WGS_gap_linkage_linked                      = 1,
    NCBI_WGS_gap_linkage_evidence_paired_ends        = 2,
    NCBI_WGS_gap_linkage_evidence_align_genus        = 4,
    NCBI_WGS_gap_linkage_evidence_align_xgenus       = 8,
    NCBI_WGS_gap_linkage_evidence_align_trnscpt      = 16,
    NCBI_WGS_gap_linkage_evidence_within_clone       = 32,
    NCBI_WGS_gap_linkage_evidence_clone_contig       = 64,
    NCBI_WGS_gap_linkage_evidence_map                = 128,
    NCBI_WGS_gap_linkage_evidence_strobe             = 256,
    NCBI_WGS_gap_linkage_evidence_unspecified        = 512,
    NCBI_WGS_gap_linkage_evidence_pcr                = 1024
};


/*--------------------------------------------------------------------------
 * accession tokenization constants
 */

/* tokenize_nuc_accession
 * tokenize_prot_accession
 *  scans name on input
 *  tokenizes into parts
 */
enum
{
    NCBI_WGS_acc_token_unrecognized =  1,
    NCBI_WGS_acc_token_prefix       =  2,
    NCBI_WGS_acc_token_contig       =  3
};


#ifdef __cplusplus
}
#endif

#endif /* _h_ncbi_wgs_contig_ */
