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


#include <krypto/extern.h>

#include <krypto/rng.h>
#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/refcount.h>

#include <sysalloc.h>

#include <openssl/rand.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef KRng KCSPRng;

#define KRNG_IMPL KCSPRng
#include <krypto/rng-impl.h>



static
rc_t CC KCSPRngDestroy (KCSPRng * self)
{
    free (self);
    return 0;
}

static
rc_t CC KCSPRngSeed (KRng * self)
{
    rc_t rc;
    uint8_t buff [16];

    if (self == NULL)
        return RC (rcKrypto, rcRng, rcWriting, rcSelf, rcNull);

    /* get some system entropy */
    rc = KRngSysEntropy (self, buff, sizeof (buff));
    if (rc)
        return rc;

    RAND_seed (buff, sizeof (buff));

    return 0;
}

static
rc_t CC KCSPRngReseed (KRng * self, const void * buffer_, 
                       size_t buff_size)
{
    const char * buffer;
    rc_t rc;
    int32_t ix;
    uint8_t buff [256];

    if (self == NULL)
        return RC (rcKrypto, rcRng, rcUpdating, rcSelf, rcNull);

    if (((buffer = buffer_) == NULL) && (buff_size != 0))
        return RC (rcKrypto, rcRng, rcUpdating, rcParam, rcNull);

    rc = KRngSysEntropy (self, buff, 16);
    if (rc)
        return rc;

    if (buffer)
        for (ix = (buff_size + sizeof (buff) - 1)/sizeof (buff);
             ix > 0; 
             (ix -= sizeof (buff)),
                 (buff_size -= sizeof(buff)),
                 (buffer += sizeof (buff)))
        {
            int32_t jx = (buff_size>sizeof (buff))?sizeof(buff):buff_size;
            while (--jx > 0)
                buff[jx] ^= buffer[jx];
        }

    RAND_add (buffer, buff_size, 1); /* what is that entropy supposed to do? */

    return 0;
}

static
rc_t CC KCSPRngRead (const KCSPRng * self, void * buffer, uint32_t bsize, uint32_t * num_read)
{
    int ret;

    ret = RAND_bytes (buffer, bsize);
    if (ret == -1)
    {
        *num_read = 0;
        return RC (rcKrypto, rcRng, rcReading, rcSelf, rcInsufficient);
    }
    *num_read = bsize;
    return 0;
}

static const
KRng_vt_v1 vt = 
{
    1, 0,

    /* start minor version == 0 */
    KCSPRngDestroy,
    KCSPRngSeed,
    KCSPRngReseed,
    KCSPRngRead
    /* end minor version == 0 */
};
    
LIB_EXPORT rc_t CC KCSPRngMake (KRng ** pself)
{
    KCSPRng * self;
    rc_t rc;

    if (pself == NULL)
        return RC (rcRuntime, 0, rcConstructing, rcSelf, rcNull);
    else
    {
        *pself = NULL;

        self = malloc (sizeof (*self));
        if (self == NULL)
            return RC (rcRuntime, 0, rcConstructing, rcMemory, rcExhausted);

        else
        {
            rc = KRngInit (self, (union KRng_vt*)&vt, "KCSPRng");
            *pself = self;
            return 0;
        }
    }
    return rc;
}
