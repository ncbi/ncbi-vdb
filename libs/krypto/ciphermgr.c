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

#define USE_AES_NI false

#include <krypto/extern.h>
#include <krypto/ciphermgr.h>
#include <krypto/cipher.h>

#include "cipher-priv.h"

#include <kproc/lock.h>

#include <klib/rc.h>
#include <klib/refcount.h>
#include <klib/debug.h>

#include <sysalloc.h>

#include <atomic.h>

#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUGGING
#define MGR_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_MGR), msg)
#else
#define MGR_DEBUG(msg)
#endif



static const char kciphermanager_classname [] = "KCipherManager";


/*--------------------------------------------------------------------------
 * KCipherManager
 */
/* currently expected to be a singleton and not use a vtable but
 * be fully fleshed out here */
static atomic_ptr_t singleton;
/*static KCipherManager * singleton = NULL;*/

struct KCipherManager
{
    KRefcount refcount;

    KLock * lock;

};


static
rc_t KCipherManagerAlloc (KCipherManager ** ppobj)
{
    KCipherManager * pobj;

    assert (ppobj);

    pobj = calloc (sizeof *pobj, 1);
    if (pobj)
    {
        *ppobj = pobj;
        return 0;
    }

    *ppobj = NULL;
    return RC (rcKrypto, rcMgr, rcConstructing, rcMemory, rcExhausted);
}


static
rc_t KCipherManagerInit (KCipherManager * self)
{
    rc_t rc;

    rc = KLockMake (&self->lock);
    if (rc == 0)
    {
        KRefcountInit (&self->refcount, 1, kciphermanager_classname, "init",
                       "singleton");
    }

    return rc;
}


/* Destroy
 *  destroy
 */
LIB_EXPORT rc_t CC KCipherManagerDestroy ( KCipherManager *self )
{
    rc_t rc = 0;

    if ( self == NULL )
        rc = RC ( rcKrypto, rcMgr, rcDestroying, rcSelf, rcNull );
    else
    {
        if (self == singleton . ptr)
        {
            KCipherManager * reread;

            reread = atomic_test_and_set_ptr (&singleton, NULL, self);

            /* ignore results: just going for guaranteed atomicity though might not need it */
            ( void ) reread;
        }

        /* no return value */
        KRefcountWhack (&self->refcount, kciphermanager_classname);

        rc = KLockRelease (self->lock);

        free (self);
    }
    return rc;
}


/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KCipherManagerAddRef ( const KCipherManager *self )
{
    if (self != NULL)
    {
        switch (KRefcountAdd (&self->refcount, kciphermanager_classname))
        {
        case krefOkay:
            break;
        case krefZero:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcIncorrect);
        case krefLimit:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcExhausted);
        case krefNegative:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
        }
    }
    return 0;
}


/* Release
 *  discard reference to manager
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KCipherManagerRelease ( const KCipherManager *self )
{
    rc_t rc = 0;
    if (self != NULL)
    {
        switch (KRefcountDrop (&self->refcount, kciphermanager_classname))
        {
        case krefOkay:
        case krefZero:
            break;
        case krefWhack:
            rc = KCipherManagerDestroy ((KCipherManager*)self);
            break;
        case krefNegative:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            rc = RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
            break;            
        }
    }
    return rc;
}


/* Make
 *  we have a shared singleton for the cipher manager
 *  first call actually makes the managerblo
 *  subsequent calls get added references
 */
LIB_EXPORT rc_t CC KCipherManagerMake (KCipherManager ** mgr)
{
    rc_t rc = 0;

    if (mgr == NULL)
        return RC (rcKrypto, rcMgr, rcConstructing, rcSelf, rcNull);

    *mgr = NULL;

make_race_retry:

    if (singleton . ptr)
    {
        rc = KCipherManagerAddRef (singleton.ptr);
        if (rc == 0)
        {
            *mgr = singleton . ptr;
            return 0;
        }
    }
    else
    {
        KCipherManager * self;

        rc = KCipherManagerAlloc (&self);
        if (rc == 0)
        {
            rc = KCipherManagerInit (self);
            if (rc == 0)
            {
                KCipherManager * reread;

                reread = atomic_test_and_set_ptr (&singleton, self, NULL);

                if (reread)
                {
                    KCipherManagerDestroy (self);
                    goto make_race_retry;
                }

                *mgr = self;
                return 0;
            }
            else
            {
                KCipherManagerDestroy (self);
            }
        }
    }
    return rc;
}


static __inline__
rc_t KCipherManagerMakeCipherInt (const KCipherManager *self,
                                  KCipher ** pcipher,
                                  kcipher_type type)
{
    KCipher * pc;
    rc_t rc;

    assert (self);
    assert (pcipher);

    rc = KCipherMake (&pc, type);

    *pcipher = rc ? NULL : pc;

    return rc;
}


LIB_EXPORT
rc_t CC KCipherManagerMakeCipher (const KCipherManager * self,
                                  struct KCipher ** pcipher,
                                  kcipher_type type)
{
    rc_t rc;
    
    if (self == NULL)
        return RC (rcKrypto, rcMgr, rcConstructing, rcSelf, rcNull);

    if (pcipher == NULL)
        return RC (rcKrypto, rcMgr, rcConstructing, rcParam, rcNull);

    *pcipher = NULL;

    rc = KCipherManagerMakeCipherInt (self, pcipher, type);

    return rc;
}



