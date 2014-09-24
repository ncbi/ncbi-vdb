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

typedef int8_t qual4[4];

enum code_book {
	known_bad = 81,
	known_good,
	pattern_a_1, /* val, -val, min, min */
	pattern_a_2, /* val, min, -val, min */
	pattern_a_3, /* val, min, min, -val */
	pattern_b_1, /* val, -val + 1, min, min */
	pattern_b_2, /* val, min, -val + 1, min */
	pattern_b_3, /* val, min, min, -val + 1 */
	pattern_c_1, /* val, -val - 1, min, min */
	pattern_c_2, /* val, min, -val - 1, min */
	pattern_c_3, /* val, min, min, -val - 1 */
	cb_last
};

