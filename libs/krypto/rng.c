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
#include <krypto/rng-impl.h>

#include <klib/defs.h>
#include <klib/rc.h>
#include <klib/refcount.h>

#include <stdlib.h>
#include <assert.h>

LIB_EXPORT rc_t CC KRngAddRef (const KRng * self)
{
    if (self) switch (KRefcountAdd (&self->refcount, "KRng"))
    {
    case krefLimit:
        return RC (rcRuntime, rcNoTarg, rcAttaching, rcRange, rcExcessive);
    }

    return 0;
}


LIB_EXPORT rc_t CC KRngRelease (const KRng * self)
{
    if (self) switch (KRefcountDrop (&self->refcount, "KRng"))
    {
    case krefWhack:
        switch (self->vt->v1.maj)
        {
        case 1:
            return (self->vt->v1.destroy ((KRng*)self));
        }
        return RC (rcKrypto, rcRng, rcAccessing, rcSelf, rcBadVersion);
    }
    return 0;
}


LIB_EXPORT rc_t CC KRngSeed (KRng * self)
{
    if (self == NULL)
        return RC (rcKrypto, rcRng, rcReading, rcSelf, rcNull);

    switch (self->vt->v1.maj)
    {
    case 1:
        return (self->vt->v1.seed (self));
    }
    return RC (rcKrypto, rcRng, rcAccessing, rcSelf, rcBadVersion);
}


LIB_EXPORT rc_t CC KRngReseed (KRng * self, void * buffer, size_t bsize)
{
    if (self == NULL)
        return RC (rcKrypto, rcRng, rcReading, rcSelf, rcNull);
    if (buffer == NULL)
        return RC (rcKrypto, rcRng, rcReading, rcBuffer, rcNull);

    switch (self->vt->v1.maj)
    {
    case 1:
        return (self->vt->v1.reseed (self, buffer, bsize));
    }
    return RC (rcKrypto, rcRng, rcAccessing, rcSelf, rcBadVersion);
}

LIB_EXPORT rc_t CC KRngRead (const KRng * self, void * buffer, uint32_t bsize, uint32_t * num_read)
{
    if (num_read == NULL)
        return RC (rcKrypto, rcRng, rcReading, rcParam, rcNull);
    *num_read = 0;
    if (self == NULL)
        return RC (rcKrypto, rcRng, rcReading, rcSelf, rcNull);
    if (buffer == NULL)
        return RC (rcKrypto, rcRng, rcReading, rcBuffer, rcNull);

    switch (self->vt->v1.maj)
    {
    case 1:
        return (self->vt->v1.read (self, buffer, bsize, num_read));
    }
    return RC (rcKrypto, rcRng, rcAccessing, rcSelf, rcBadVersion);
}

LIB_EXPORT rc_t CC KRngInit (KRng * self, union KRng_vt * vt, const char * type)
{
    self->vt = vt;
    KRefcountInit (&self->refcount, 1, "KRng", "Init", type);
    return 0;
}

    
