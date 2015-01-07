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

#ifndef _h_csra1_pileup_
#define _h_csra1_pileup_

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct NGS_Pileup;
struct NGS_String;
struct VDatabase;
struct NGS_Cursor;

struct NGS_Pileup * CSRA1_PileupIteratorMake( ctx_t ctx,
    struct VDatabase const* db,
    struct NGS_Cursor const* curs_ref,
    const struct NGS_String* spec,
    int64_t first_row_id, 
    int64_t last_row_id,
    bool wants_primary, 
    bool wants_secondary );

struct NGS_Pileup * CSRA1_PileupIteratorMakeSlice( ctx_t ctx,
    struct VDatabase const* db,
    struct NGS_Cursor const* curs_ref,
    const struct NGS_String* spec,
    int64_t first_row_id, 
    int64_t last_row_id,
    uint64_t slice_start, 
    uint64_t slice_size,
    bool wants_primary, 
    bool wants_secondary );

#ifdef __cplusplus
}
#endif

#endif /* _h_csra1_pileup_ */
