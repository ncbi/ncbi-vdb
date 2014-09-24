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

#ifndef _h_simple_SRASpot_priv_
#define _h_simple_SRASpot_priv_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifndef SRASPOT
#define SRASPOT SRASpot
#endif

#ifndef _h_simple_SRASpot_
#include "sra-spot.h"
#endif

#ifndef SRASEQUENCE
#define SRASEQUENCE SRASPOT
#endif

#ifndef _h_simple_SRASequence_priv_
#include "sra-sequence-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * SRASpot_vt
 */
typedef struct SRA_Spot_vt_v1 SRASpot_vt_v1;
struct SRA_Spot_vt_v1
{
    uint32_t maj, min;

    /* BEGIN MINOR VERSION 0 */
    SRASPOT* ( CC * dup ) ( const SRASPOT *self, struct SRAException *x );
    SRAString ( CC * platform ) ( const SRASPOT *self, struct SRAException *x );
    SRAString ( CC * spot_group ) ( const SRASPOT *self, struct SRAException *x );
    int64_t ( CC * id ) ( const SRASPOT *self, struct SRAException *x );
    uint32_t ( CC * num_reads ) ( const SRASPOT *self, struct SRAException *x );
    struct SRA_Sequence* ( CC * reads ) ( const SRASPOT *self, struct SRAException *x );
    uint32_t ( CC * num_bio_reads ) ( const SRASPOT *self, struct SRAException *x );
    struct SRA_Sequence* ( CC * bio_reads ) ( const SRASPOT *self, struct SRAException *x, bool trimmed );
    /* END MINOR VERSION 0 */
};

typedef union SRASpot_vt SRASpot_vt;
union SRASpot_vt
{
    SRASpot_vt_v1 v1;
};


/*--------------------------------------------------------------------------
 * SRASpot_mvt
 */
typedef struct SRA_Spot_mvt SRASpot_mvt;
struct SRA_Spot_mvt
{
    SRASequence_mvt dad;
    const SRASpot_vt *spot;
};

/*--------------------------------------------------------------------------
 * SRASpot
 *  just extend the guy
 */
struct SRA_Spot
{
    SRASequence dad;
};


/* Init
 *  returns true if successful
 */
bool SRASpotInit ( SRASpot *self, struct SRAException *x, const SRASpot_mvt *mvt );


/* GetMVT
 *  retrieve spot multi-vTable
 */
#define SRASpotGetMVT( self ) \
    ( ( const SRASpot_mvt* ) ( self ) -> dad . mvt )


/* GetVT
 *  retrieve spot vTable
 */
#define SRASpotGetVT( self ) \
    ( SRASpotGetMVT ( self ) -> spot )


#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRASpot_priv_ */
