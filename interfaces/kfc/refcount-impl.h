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

#ifndef _h_kfc_refcount_impl_
#define _h_kfc_refcount_impl_

#ifndef _h_kfc_extern_
#include <kfc/extern.h>
#endif

#ifndef _h_kfc_refcount_
#include <kfc/refcount.h>
#endif

#ifndef _h_kfc_vtbl_impl_
#include <kfc/vtbl-impl.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KRefcount_v1
 *   the class
 */
struct KRefcount_v1
{
    const KVTable * vt;
    KRefcount refcount;

#if _ARCH_BITS > 32
    uint8_t align [ sizeof ( void* ) - sizeof ( KRefcount ) ];
#endif

};

extern KITFTOK_DECL ( KRefcount_v1 );

typedef struct KRefcount_v1_vt KRefcount_v1_vt;
struct KRefcount_v1_vt
{
    KVTable dad;

    /* begin 1.0 */
    void ( CC * destroy ) ( KRefcount_v1 *self, ctx_t ctx );
    void * ( CC * dup ) ( const KRefcount_v1 *self, ctx_t ctx, caps_t rm );
    /* end 1.0 */
};


/* Init
 *  initialize the refcount to 1
 *  may perform logging
 */
KFC_EXTERN void CC KRefcountInit_v1 ( KRefcount_v1 *self, ctx_t ctx,
    const KVTable * vt, const char * instance_name );

/* Destroy
 *  has nothing to destroy
 *  may perform logging
 */
KFC_EXTERN void CC KRefcountDestroy_v1 ( KRefcount_v1 *self, ctx_t ctx );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_refcount_impl_ */
