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

#ifndef _h_name_tokenizer_
#define _h_name_tokenizer_

/* spot_name_token_t
 *  represents a single token
 *  the tokenizer should emit a row of zero or more of these
 */
typedef union spot_name_token_t spot_name_token_t;
union spot_name_token_t
{
    struct
    {
        uint16_t token_type;
        uint16_t position;
        uint16_t length;
    } s;
    uint16_t raw [ 3 ];
};

/* name_token
 *  token ids
 */
enum name_token
{
    nt_unrecognized = 1,
    nt_recognized,

    nt_Q,

    nt_X,
    nt_Y,
    nt_T,
    nt_L,

    nt_signed_X,
    nt_signed_Y,
    nt_signed_T,
    nt_signed_L,

    nt_octal_X,
    nt_octal_Y,
    nt_octal_T,
    nt_octal_L,

    nt_hex_upper_X,
    nt_hex_upper_Y,
    nt_hex_upper_T,
    nt_hex_upper_L,

    nt_hex_lower_X,
    nt_hex_lower_Y,
    nt_hex_lower_T,
    nt_hex_lower_L,

    nt_max_token
};

/* token_symbol
 *  substitution symbols
 */
#define TOK_ALPHABET ("---QXYTLXYTLabcdefghxytl")

enum token_symbol
{
    ts_Q          = 'Q',

    ts_X          = 'X',
    ts_Y          = 'Y',
    ts_T          = 'T',
    ts_L          = 'L',

    ts_signed_X   = 'X',
    ts_signed_Y   = 'Y',
    ts_signed_T   = 'T',
    ts_signed_L   = 'L',

    ts_oct_X      = 'a',
    ts_oct_Y      = 'b',
    ts_oct_T      = 'c',
    ts_oct_L      = 'd',

    ts_hex_upr_X  = 'e',
    ts_hex_upr_Y  = 'f',
    ts_hex_upr_T  = 'g',
    ts_hex_upr_L  = 'h',

    ts_hex_lwr_X  = 'x',
    ts_hex_lwr_Y  = 'y',
    ts_hex_lwr_T  = 't',
    ts_hex_lwr_L  = 'l'
};

#define Q_ALPHABET ("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")

#endif /* _h_name_tokenizer_ */
