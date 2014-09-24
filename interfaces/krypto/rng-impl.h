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

#ifndef _h_kryptoimpl_
#define _h_kryptoimpl_

#include <krypto/extern.h>
#include <krypto/rng.h>
#include <klib/refcount.h>


#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */

typedef union KRng_vt KRng_vt;

struct KRng
{
    const union KRng_vt *vt;
    KRefcount refcount;
};

#ifndef KRNG_IMPL
#define KRNG_IMPL KRng
#endif

typedef struct KRng_vt_v1 KRng_vt_v1;
struct KRng_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t (CC * destroy) (KRNG_IMPL * self);
    rc_t (CC * seed)    (KRNG_IMPL * self);
    rc_t (CC * reseed)  (KRNG_IMPL * self, const void * buff, size_t buff_size);
    rc_t (CC * read)    (const KRNG_IMPL *self, void * buff, uint32_t buff_len,
                         uint32_t * num_read);
    /* end minor version == 0 */
    /* start minor version == 1 */
    /* end minor version == 1 */
    /* end version == 1.x */
};

union KRng_vt
{
    KRng_vt_v1 v1;
};

KRYPTO_EXTERN rc_t CC KRngInit (KRng * self, union KRng_vt * vt, const char * type);
KRYPTO_EXTERN rc_t CC KRngSysEntropy (KRng * self, uint8_t * buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _h_kryptoimpl_ */
