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

#ifndef _h_simple_SRAObject_priv_
#define _h_simple_SRAObject_priv_

#ifndef _h_simple_SRAObject_
#include "sra-object.h"
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SRAOBJECT
#define SRAOBJECT SRAObject
#endif


/*--------------------------------------------------------------------------
 * SRAObject_vt
 */
typedef struct SRAObject_vt_v1 SRAObject_vt_v1;
struct SRAObject_vt_v1
{
    /* interface version */
    uint32_t maj, min;

    /* begin v1.0 messages */

    void ( CC * whack ) ( SRAOBJECT *self );
    SRAOBJECT* ( CC * dup ) ( const SRAOBJECT *self, struct SRAException *x );

    SRAOBJECT* ( CC * slice_by_reference ) ( const SRAOBJECT *self,
        struct SRAException *x, struct SRAString const *referenceSpec );
    SRAOBJECT* ( CC * slice_by_range ) ( const SRAOBJECT *self, struct SRAException *x,
        uint64_t reference_start, uint64_t reference_length, bool fully_contained );

    void ( CC * open ) ( SRAOBJECT *self, struct SRAException *x );
    void ( CC * close ) ( SRAOBJECT *self );

    struct SRA_Reference* ( CC * get_reference_iterator ) ( const SRAOBJECT *self, struct SRAException *x );

    struct SRA_Alignment* ( CC * get_primary_alignment_iterator ) ( const SRAOBJECT *self, struct SRAException *x );
    struct SRA_Alignment* ( CC * get_primary_alignment_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count );

    struct SRA_Alignment* ( CC * get_secondary_alignment_iterator ) ( const SRAOBJECT *self, struct SRAException *x );
    struct SRA_Alignment* ( CC * get_secondary_alignment_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count );

    struct SRA_Alignment* ( CC * get_alignment_iterator ) ( const SRAOBJECT *self, struct SRAException *x );

    struct SRA_Sequence* ( CC * get_aligned_sequence_iterator ) ( const SRAOBJECT *self, struct SRAException *x );
    struct SRA_Sequence* ( CC * get_aligned_sequence_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count );

    struct SRA_Sequence* ( CC * get_unaligned_sequence_iterator ) ( const SRAOBJECT *self, struct SRAException *x );
    struct SRA_Sequence* ( CC * get_unaligned_sequence_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count );

    struct SRA_Sequence* ( CC * get_sequence_iterator ) ( const SRAOBJECT *self, struct SRAException *x );

    struct SRA_Spot* ( CC * get_aligned_spot_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, bool partially_aligned );
    struct SRA_Spot* ( CC * get_aligned_spot_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count, bool partially_aligned );

    struct SRA_Spot* ( CC * get_unaligned_spot_iterator ) ( const SRAOBJECT *self, struct SRAException *x );
    struct SRA_Spot* ( CC * get_unaligned_spot_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count );

    struct SRA_Spot* ( CC * get_spot_iterator ) ( const SRAOBJECT *self, struct SRAException *x );
    struct SRA_Spot* ( CC * get_spot_range_iterator ) ( const SRAOBJECT *self,
        struct SRAException *x, uint64_t startIdx, uint64_t count );

    /* end v1.0 messages */
};

typedef union SRAObject_vt SRAObject_vt;
union SRAObject_vt
{
    SRAObject_vt_v1 v1;
};

/*--------------------------------------------------------------------------
 * SRAObject
 *  pairs an interface vTable with an implementation object
 */
struct SRA_Object
{
    const SRAObject_vt *vt;
    KRefcount refcount;
};


/* Init
 *  returns true on success
 */
bool SRAObjectInit ( SRAObject *self, struct SRAException *x, const SRAObject_vt *vt );

/* AddRef
 *  default behavior for Duplicate
 */
SRAOBJECT* CC SRAObjectAddRef ( const SRAOBJECT *self, struct SRAException *x );

#ifdef __cplusplus
}
#endif

#endif /* _h_simple_SRAObject_priv_ */
