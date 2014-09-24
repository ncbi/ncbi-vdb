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

#ifndef _h_idstats_priv_
#define _h_idstats_priv_

#ifndef _h_klib_container_
#include <klib/container.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KIdStats
 *  maintains statistics about id mappings
 */
typedef struct KIdStats KIdStats;
struct KIdStats
{
    int64_t i_min_id, x_max_id;
    uint64_t num_entries;
    uint64_t num_ids;
    uint64_t num_holes;
    BSTree ids;
};


/* Init
 *  initialize the object
 */
void KIdStatsInit ( KIdStats *s );

/* Whack
 *  tear down the object
 */
void KIdStatsWhack ( KIdStats *self );

/* Insert
 *  add an entry representing 1 or more consecutive ids
 */
rc_t KIdStatsInsert ( KIdStats *self, int64_t id, uint64_t count );


#ifdef __cplusplus
}
#endif

#endif /* _h_idstats_priv_ */
