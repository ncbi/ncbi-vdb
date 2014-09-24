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

#ifndef _h_insdc_insdc_
#define _h_insdc_insdc_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================================================
 * INSDC types, constants
 */


/*--------------------------------------------------------------------------
 * dna
 *  represented in IUPAC characters
 */
typedef char INSDC_dna_text;


/*--------------------------------------------------------------------------
 * 4na
 *  nucleotide data with all possible ambiguity
 *  does not represent all possible EVENTS
 *
 *  text encodings use the IUPAC character set
 *  legal values: [ACMGRSVTWYHKDBNacmgrsvtwyhkdbn.]
 *  canonical values: [ACMGRSVTWYHKDBN]
 *
 *  binary values are 0..15 = { NACMGRSVTWYHKDBN }
 *
 *  4na values use bits for each letter:
 *
 *       A | C | G | T
 *    =================
 *    N    |   |   |
 *    A  * |   |   |
 *    C    | * |   |
 *    M  * | * |   |
 *    G    |   | * |
 *    R  * |   | * |
 *    S    | * | * |
 *    V  * | * | * |
 *    T    |   |   | *
 *    W  * |   |   | *
 *    Y    | * |   | *
 *    H  * | * |   | *
 *    K    |   | * | *
 *    D  * |   | * | *
 *    B    | * | * | *
 *    N  * | * | * | *
 *
 *  packed bytes have first base in upper nibble,
 *  and the following base in lower nibble.
 *
 *    bit:  76543210 76543210 76543210
 *    ================================
 *    base: 00001111 22223333 44445555 etc.
 *
 *  element offsets follow this pattern
 */
typedef uint8_t INSDC_4na_bin;
typedef uint8_t INSDC_4na_packed;

#define INSDC_4na_map_BINSET \
    { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }
#define INSDC_4na_map_CHARSET \
    ".ACMGRSVTWYHKDBN"
#define INSDC_4na_accept_CHARSET \
    ".ACMGRSVTWYHKDBNacmgrsvtwyhkdbn"


/*--------------------------------------------------------------------------
 * 2na
 *  nucleotide data { ATGC }
 * x2na
 *  nucleotide data with single ambiguity value
 *
 *  text encodings use the IUPAC character set
 *  legal values: [ACGTNacgtn.]
 *  canonical values: [ACGTN]
 *
 *  binary values are 0..4 = { ACGTN }
 *
 *  packed values exclude N:
 *    A = 0
 *    C = 1
 *    G = 2
 *    T = 3
 *
 *  packed bytes have first base in uppermost 2 bits,
 *  and the following bases in similar fashion:
 *
 *    bit:  76543210 76543210
 *    =======================
 *    base: 00112233 44556677 etc.
 *
 *  element offsets follow this pattern
 */
typedef uint8_t INSDC_2na_bin;
typedef uint8_t INSDC_x2na_bin;
typedef uint8_t INSDC_2na_packed;

#define INSDC_2na_map_BINSET \
    { 0,1,2,3 }
#define INSDC_2na_map_CHARSET \
    "ACGT"
#define INSDC_2na_accept_CHARSET \
    "ACGTacgt"
#define INSDC_x2na_map_BINSET \
    { 0,1,2,3,4 }
#define INSDC_x2na_map_CHARSET \
    "ACGTN"
#define INSDC_x2na_accept_CHARSET \
    "ACGTNacgtn."


/*--------------------------------------------------------------------------
 * color
 *  color-space data
 * 2cs
 *  color-space data 0,1,2,3
 * x2cs
 *  color-space data extended with single ambiguity value (.)
 *
 *  text encodings use the ASCII numeric character set
 *  values: [0123.]
 *
 *  x2cs values are 0..4 = { 0123. }
 *
 *  2cs values exclude '.':
 *    '0' = 0
 *    '1' = 1
 *    '2' = 2
 *    '3' = 3
 */
typedef char INSDC_color_text;
typedef uint8_t INSDC_2cs_bin;
typedef uint8_t INSDC_x2cs_bin;
typedef uint8_t INSDC_2cs_packed;

#define INSDC_2cs_map_BINSET \
    { 0,1,2,3 }
#define INSDC_2cs_map_CHARSET \
    "0123"
#define INSDC_2cs_accept_CHARSET \
    "0123"
#define INSDC_x2cs_map_BINSET \
    { 0,1,2,3,4 }
#define INSDC_x2cs_map_CHARSET \
    "0123."
#define INSDC_x2cs_accept_CHARSET \
    "0123."
#define INSDC_color_default_matrix \
{                                  \
    0, 1, 2, 3, 4,                 \
    1, 0, 3, 2, 4,                 \
    2, 3, 0, 1, 4,                 \
    3, 2, 1, 0, 4,                 \
    4, 4, 4, 4, 4                  \
}


/*--------------------------------------------------------------------------
 * protein
 *  represented in IUPAC characters
 */
typedef char INSDC_protein_text;


/*--------------------------------------------------------------------------
 * aa
 *  protein data
 *  text encodings use the IUPAC character set
 */
typedef	uint8_t INSDC_aa_bin;

#define INSDC_aa_map_BINSET \
    { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25 }
#define INSDC_aa_map_CHARSET \
    "ABCDEFGHIKLMNPQRSTVWXYZU*"
#define INSDC_aa_accept_CHARSET \
    "ABCDEFGHIKLMNPQRSTVWXYZU*abcdefghiklmnpqrstvwxyzu"


/*--------------------------------------------------------------------------
 * quality
 *  quality scoring values
 *
 *  phred legal values_ 0..63
 */
typedef uint8_t INSDC_quality_phred;
typedef int8_t INSDC_quality_log_odds;


/*--------------------------------------------------------------------------
 * coordinate
 *  zero and one based coordinates
 *  lengths are expressed as unsigned quantities
 */
typedef int32_t INSDC_coord_val;
typedef uint32_t INSDC_coord_len;
typedef INSDC_coord_val INSDC_coord_zero;
typedef INSDC_coord_val INSDC_coord_one;
typedef INSDC_coord_zero INSDC_position_zero;
typedef INSDC_coord_one INSDC_position_one;


/*--------------------------------------------------------------------------
 * read description
 *  type and filter constants
 */


/* read type
 *  describes the type of read within a spot
 *  the extended version also describes its orientation
 */
typedef uint8_t INSDC_read_type;
enum
{
    /* read_type */
    READ_TYPE_TECHNICAL  = 0,
    READ_TYPE_BIOLOGICAL = 1,

    /* orientation - applied as bits, e.g.:
       type = READ_TYPE_BIOLOGICAL | READ_TYPE_REVERSE */
    READ_TYPE_FORWARD = 2,
    READ_TYPE_REVERSE = 4
};

/* read filter
 */
typedef uint8_t INSDC_read_filter;
enum
{
    READ_FILTER_PASS = 0,
    READ_FILTER_REJECT = 1,
    READ_FILTER_CRITERIA = 2,
    READ_FILTER_REDACTED = 3
};

#ifdef __cplusplus
}
#endif

#endif /*  _h_insdc_insdc_ */
