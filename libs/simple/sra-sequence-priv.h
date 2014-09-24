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

#ifndef _h_simple_SRASequence_priv_
#define _h_simple_SRASequence_priv_

#ifndef _h_simple_extern_
#include <simple/extern.h>
#endif

#ifndef _h_simple_SRASequence_
#include "sra-sequence.h"
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SRASEQUENCE
#define SRASEQUENCE SRASequence
#endif


/*--------------------------------------------------------------------------
 * SRASequence_vt
 */
typedef struct SRA_Sequence_vt_v1 SRASequence_vt_v1;
struct SRA_Sequence_vt_v1
{
    uint32_t maj, min;

    /* BEGIN MINOR VERSION 0 */

    void ( CC * whack ) ( SRASEQUENCE * self );
    SRASEQUENCE* ( CC * dup ) ( const SRASEQUENCE *self, struct SRAException *x );

    SRAString ( CC * name ) ( const SRASEQUENCE *self, struct SRAException *x );
    bool ( CC * is_technical ) ( const SRASEQUENCE *self, struct SRAException *x );
    bool ( CC * is_reverse ) ( const SRASEQUENCE *self, struct SRAException *x );
    uint64_t ( CC * length ) ( const SRASEQUENCE *self, struct SRAException *x );
    SRAString ( CC * bases ) ( const SRASEQUENCE *self,
        struct SRAException *x, uint64_t offset, uint64_t *remaining );
    bool ( CC * color_space_native ) ( const SRASEQUENCE *self, struct SRAException *x );
    bool ( CC * has_color_space ) ( const SRASEQUENCE *self );
    char ( CC * color_space_key ) ( const SRASEQUENCE *self, struct SRAException *x );
    SRAString ( CC * color_space ) ( const SRASEQUENCE *self,
        struct SRAException *x, uint64_t offset, uint64_t *remaining );
    SRAMem ( CC * qualities ) ( const SRASEQUENCE *self,
        struct SRAException *x, uint64_t offset, uint64_t *remaining );
    bool ( CC * has_primary_alignment ) ( const SRASEQUENCE *self );
    struct SRA_Alignment* ( CC * primary_alignment ) ( const SRASEQUENCE *self, struct SRAException *x );

    /* END MINOR VERSION 0 */
};

typedef union SRASequence_vt SRASequence_vt;
union SRASequence_vt
{
    SRASequence_vt_v1 v1;
};


/*--------------------------------------------------------------------------
 * SRASequenceIterator_vt
 */
typedef struct SRA_SequenceIterator_vt_v1 SRASequenceIterator_vt_v1;
struct SRA_SequenceIterator_vt_v1
{
    uint32_t maj, min;

    /* BEGIN MINOR VERSION 0 */
    SRASEQUENCE* ( CC * dup ) ( const SRASEQUENCE *self, struct SRAException *x );
    bool ( CC * next ) ( SRASEQUENCE *self, struct SRAException *x );
    /* END MINOR VERSION 0 */
};

typedef union SRASequenceIterator_vt SRASequenceIterator_vt;
union SRASequenceIterator_vt
{
    SRASequenceIterator_vt_v1 v1;
};


/*--------------------------------------------------------------------------
 * SRASequence_mvt
 */
typedef struct SRA_Sequence_mvt SRASequence_mvt;
struct SRA_Sequence_mvt
{
    const SRASequence_vt *seq;
    const SRASequenceIterator_vt *it;
};


/*--------------------------------------------------------------------------
 * SRASequence
 *  supports dual-level interface ( SRASequence, SRASequenceIterator )
 */
struct SRA_Sequence
{
    const SRASequence_mvt *mvt;
    KRefcount refcount;
};


/* Init
 *  returns true on success
 */
bool SRASequenceInit ( SRASequence *self,
    struct SRAException *x, const SRASequence_mvt *mvt );


#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRASequence_priv_ */
