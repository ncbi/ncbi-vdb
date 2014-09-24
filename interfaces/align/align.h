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

#ifndef _h_align_align_
#define _h_align_align_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================================================
 * ALIGN types, constants
 */


/*--------------------------------------------------------------------------
 * ploidy
 */
typedef uint32_t NCBI_align_ploidy;


/*--------------------------------------------------------------------------
 * ro_type
 *  ref-offset type
 */
typedef uint8_t NCBI_align_ro_type;

enum
{
    NCBI_align_ro_normal            = 0, /* normal ref-offset           */
    NCBI_align_ro_soft_clip         = 1, /* soft-clipping               */
    NCBI_align_ro_intron_plus       = 2, /* intron on positive strand   */
    NCBI_align_ro_intron_minus      = 3, /* intron on negative strand   */
    NCBI_align_ro_intron_unknown    = 4, /* intron strand not specified */
    NCBI_align_ro_complete_genomics = 5  /* (future)                    */
};

#ifdef __cplusplus
}
#endif

#endif /*  _h_align_align_ */
