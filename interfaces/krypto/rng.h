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

#ifndef _h_krypto_rng_
#define _h_krypto_rng_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KRng KRng;

/* attach a new 'ownership' reference */
KRYPTO_EXTERN rc_t CC KRngAddRef (const KRng * self);

/* release an 'ownership' reference */
KRYPTO_EXTERN rc_t CC KRngRelease (const KRng * self);

/* call to the system entropy function and [re-]seed the rng */
KRYPTO_EXTERN rc_t CC KRngSeed (KRng * self);

/* fill a buffer of a given size with random values: type sepcific as to the meaning or size of elements
 * return the number of elements actually written to the buffer */
KRYPTO_EXTERN rc_t CC KRngRead (const KRng * self, void * buffer, uint32_t bsize, uint32_t * num_read);


/*
 * Cryptographically secure pseudo-random number generator
 * elements are 8 bits long
 */
KRYPTO_EXTERN rc_t CC KCSPRngMake (KRng ** pself);

#ifdef __cplusplus
}
#endif

#endif /* _h_krypto_rng_ */
