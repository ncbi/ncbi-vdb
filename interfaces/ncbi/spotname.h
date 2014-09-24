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

#ifndef _h_ncbi_spotname_
#define _h_ncbi_spotname_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================================================
 * NCBI Sequence Read Archive schema
 */


/*--------------------------------------------------------------------------
 * types
 */

/* spot_name_token
 *  a vector describing tokens recognized within a spot name
 *
 * COMPONENTS:
 *  0 - token id
 *  1 - token starting coordinate
 *  2 - token length
 */
typedef uint16_t NCBI_SRA_spot_name_token [ 3 ];

/* token values
 *
 *  tokens are produced by a schema-specific tokenizer function
 *  this function is purposely abstract because it may rely upon
 *  whatever information it needs to perform its task. the only
 *  requirement is that it produce these tokens as its output.
 *
 *  an empty name input must produce no tokens. in this case,
 *  there is no name to tokenize or data to produce.
 *
 *  a non-empty name must produce 1 or more tokens of output.
 *  all tokens must be ordered by starting character position.
 *
 *  if a name does not conform to any pattern recognized by the
 *  tokenizer, then the tokenizer emits a single token of "unrecognized"
 *
 *  if a name conforms to some pattern but does not have any
 *  substitution tokens, the tokenizer emits a single token of "recognized"
 *
 *  if a name may be tokenized, then the resulting tokens should
 *  describe only the portions of the string that should be removed
 *  from the name, e.g. "X" or "Y".
 *
 *  the standard coordinates "X".."L" are given in unsigned decimal.
 *  alternate representations are contained within their respective
 *  namespaces: "signed", "hex" and "octal".
 *
 *  the special coordinate "Q" represents the 454-specific encoding
 *  of X and Y into base-36, where the formula for Q is:
 *    Q = 4096 * X + Y
 *  and ASCII encoding:
 *    0..25 => "A-Z", 26..35 => "0-9"
 */
enum
{
    NCBI_SRA_name_token_unrecognized =  1,
    NCBI_SRA_name_token_recognized   =  2,
    NCBI_SRA_name_token_Q            =  3,
    NCBI_SRA_name_token_X            =  4,
    NCBI_SRA_name_token_Y            =  5,
    NCBI_SRA_name_token_T            =  6,
    NCBI_SRA_name_token_L            =  7,
    NCBI_SRA_name_token_signed_X     =  8,
    NCBI_SRA_name_token_signed_Y     =  9,
    NCBI_SRA_name_token_signed_T     = 10,
    NCBI_SRA_name_token_signed_L     = 11,
    NCBI_SRA_name_token_octal_X      = 12,
    NCBI_SRA_name_token_octal_Y      = 13,
    NCBI_SRA_name_token_octal_T      = 14,
    NCBI_SRA_name_token_octal_L      = 15,
    NCBI_SRA_name_token_hex_upper_X  = 16,
    NCBI_SRA_name_token_hex_upper_Y  = 17,
    NCBI_SRA_name_token_hex_upper_T  = 18,
    NCBI_SRA_name_token_hex_upper_L  = 19,
    NCBI_SRA_name_token_hex_lower_X  = 20,
    NCBI_SRA_name_token_hex_lower_Y  = 21,
    NCBI_SRA_name_token_hex_lower_T  = 22,
    NCBI_SRA_name_token_hex_lower_L  = 23
};


/* token symbols
 *  when a name matches some pattern and tokens are recognized,
 *  the tokens are extracted from the name and sent to individual
 *  columns, and replaced with the symbols below to create a
 *  formatted name.
 */
enum
{
    NCBI_SRA_name_symbol_Q           = 'Q',
    NCBI_SRA_name_symbol_X           = 'X',
    NCBI_SRA_name_symbol_Y           = 'Y',
    NCBI_SRA_name_symbol_T           = 'T',
    NCBI_SRA_name_symbol_L           = 'L',
    NCBI_SRA_name_symbol_octal_X     = 'a',
    NCBI_SRA_name_symbol_octal_Y     = 'b',
    NCBI_SRA_name_symbol_octal_T     = 'c',
    NCBI_SRA_name_symbol_octal_L     = 'd',
    NCBI_SRA_name_symbol_hex_upper_X = 'e',
    NCBI_SRA_name_symbol_hex_upper_Y = 'f',
    NCBI_SRA_name_symbol_hex_upper_T = 'g',
    NCBI_SRA_name_symbol_hex_upper_L = 'h',
    NCBI_SRA_name_symbol_hex_lower_X = 'x',
    NCBI_SRA_name_symbol_hex_lower_Y = 'y',
    NCBI_SRA_name_symbol_hex_lower_T = 't',
    NCBI_SRA_name_symbol_hex_lower_L = 'l'
};

#ifdef __cplusplus
}
#endif

#endif /*  _h_ncbi_spotname_ */
