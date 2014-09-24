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
 */

#include <krypto/extern.h>
#include <krypto/key.h>
#include <kapp/args.h>
#include <klib/checksum.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <string.h>

KRYPTO_EXTERN_DATA KKey KKeyEncrypt;
KRYPTO_EXTERN_DATA KKey KKeyDecrypt;
KRYPTO_EXTERN_DATA char WGAEncryptPassword[WGA_MAX_PASSWORD];
KRYPTO_EXTERN_DATA char WGADecryptPassword[WGA_MAX_PASSWORD];


static rc_t KKeyInitInt (KKey * self, KKeyType key_type, 
                         const void * password, size_t password_size)
{
    if (self == NULL)
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcSelf, rcNull);

    if (key_type >= kkeyTypeCount)
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, rcInvalid);

    if (key_type == kkeyNone)
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, rcIncorrect);

    if (password == NULL)
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, rcNull);

    if (password_size == 0)
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, rcTooShort);

    {
        SHA256State state;
        size_t z;
        uint8_t digest [64];


        SHA256StateInit (&state);
        SHA256StateAppend (&state, password, password_size);
        SHA256StateFinish (&state, digest);
        
        switch (self->type = key_type)
        {
        default:
            z = 0;
            break;

        case kkeyAES128:
            z = 128/8;
            break;

        case kkeyAES192:
            z = 192/8;
            break;

        case kkeyAES256:
            z = 256/8;
            break;
        }
        memcpy (self->text, digest, z);
    }
    return 0;
}

LIB_EXPORT rc_t CC KKeyInitRead (KKey * self, KKeyType key_type, 
                                 const void * password, size_t password_size)
{
    return KKeyInitInt (self, key_type, password, password_size);
}

LIB_EXPORT rc_t CC KKeyInitUpdate (KKey * self, KKeyType key_type, 
                                   const void * password, size_t password_size)
{
    if (password_size < kkeyMinimumSize)
    {
        rc_t rc = RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, 
                      rcTooShort);
#if FAIL_SHORT_PASSWORD
        return rc;
#else
        PLOGERR (klogWarn, (klogWarn, rc,
                            "Password is shorter than recommended '$(P)' is less than '$(Q)'"
                            , "P=%zu,Q=%u"
                            , password_size
                            , ( uint32_t ) kkeyMinimumSize
                     ));
#endif
    }
    return KKeyInitInt (self, key_type, password, password_size);
}
    


LIB_EXPORT rc_t CC KKeyInitHex()
{
    return 0;
}

/* end of file encfile.c */

