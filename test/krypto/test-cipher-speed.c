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
#include <kapp/main.h>
#include <kapp/args.h>

#include <krypto/ciphermgr.h>
#include <krypto/cipher.h>
#include <krypto/cipher-test.h>

#include <klib/status.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <klib/defs.h>


#include <string.h>
#include <stdlib.h>
#include <time.h>

/* #define DEFAULT_TIMER (5*60) */
#define DEFAULT_TIMER (5)

#define OPTION_TIMER  "timer"
#define ALIAS_TIMER   "t"

const char * timer_usage[] = 
{ "time per function in seconds", "Default is 5 seconds", NULL };

OptDef Options[] = 
{
    { OPTION_TIMER, ALIAS_TIMER, NULL, timer_usage, 1, true, false }
};

struct KBlockCipher;

const char * cipher_subtypes[] = 
{
    "byte",
    "vector",
    "vec-reg",
    "AES-NI"
/*     "byte", */
/*     "vector", */
/*     "vector register", */
/*     "accelerated(AES-NI)" */
};


const char * cipher_types[] = 
{
    "null",
    "AES"
/*     "null (copy) cipher", */
/*     "AES cipher" */
};

rc_t ((* make_functions[4])(KCipher **, kcipher_type)) = 
{
    KCipherTestByteMake,
    KCipherTestVecMake,
    KCipherTestVecRegMake,
    KCipherTestVecAesNiMake
};

char user_key   [32];
char cipher_block [32*1024];


/* not at all critical that randomness is high */
void random_cipher_block ()
{
    unsigned ix;

    srand (1);
    
    for (ix = 0; ix < sizeof cipher_block; ++ix)
        cipher_block[ix] = (char)rand();
}


rc_t function_timer (rc_t (*function)(KCipher * cipher), KCipher * ciphers[16], uint64_t * pcount, clock_t timer)
{
    uint64_t count;
    rc_t rc = 0;
    clock_t start;
    clock_t end;
    clock_t now;

    random_cipher_block();

    now = start = clock();
    end = start + (clock_t)(timer * CLOCKS_PER_SEC);

    for (count = 0; now < end; ++count)
    {
        rc = function(ciphers[count&15]);
        if (rc)
            break;
        now = clock();
    }
    *pcount = count;
    return rc;
}


rc_t set_encrypt_key_function_128 (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    memmove (user_key, cipher_block + ix, sizeof user_key);
    return KCipherSetEncryptKey (cipher, user_key, 16);
}


rc_t set_encrypt_key_function_192 (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 24)
        ix = 0;

    memmove (user_key, cipher_block + ix, sizeof user_key);
    return KCipherSetEncryptKey (cipher, user_key, 24);
}


rc_t set_encrypt_key_function_256 (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 32)
        ix = 0;

    memmove (user_key, cipher_block + ix, sizeof user_key);
    return KCipherSetEncryptKey (cipher, user_key, 32);
}


rc_t set_decrypt_key_function_128 (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    memmove (user_key, cipher_block + ix, sizeof user_key);
    return KCipherSetDecryptKey (cipher, user_key, 16);
}


rc_t set_decrypt_key_function_192 (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 24)
        ix = 0;

    memmove (user_key, cipher_block + ix, sizeof user_key);
    return KCipherSetDecryptKey (cipher, user_key, 24);
}


rc_t set_decrypt_key_function_256 (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 32)
        ix = 0;

    memmove (user_key, cipher_block + ix, sizeof user_key);
    return KCipherSetDecryptKey (cipher, user_key, 32);
}


rc_t encrypt_function (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    return KCipherEncrypt (cipher, cipher_block + ix, cipher_block + ix);
}


rc_t decrypt_function (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    return KCipherDecrypt (cipher, cipher_block + ix, cipher_block + ix);
}


rc_t ecb_encrypt_function (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    return KCipherEncryptECB (cipher, cipher_block, cipher_block, sizeof (cipher_block) / 16);
}


rc_t ecb_decrypt_function (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    return KCipherDecryptECB (cipher, cipher_block, cipher_block, sizeof (cipher_block) / 16);
}


rc_t cbc_encrypt_function (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    return KCipherEncryptCBC (cipher, cipher_block, cipher_block, sizeof (cipher_block) / 16);
}


rc_t cbc_decrypt_function (KCipher * cipher)
{
    static unsigned ix;

    /* just getting some random data - value shouldn't matter much */
    if (++ix > sizeof cipher_block - 16)
        ix = 0;
   
    return KCipherDecryptCBC (cipher, cipher_block, cipher_block, sizeof (cipher_block) / 16);
}


/* ==================================================== */

uint64_t encrypt_key_128[kcipher_count*4] = {0};
uint64_t encrypt_key_192[kcipher_count*4] = {0};
uint64_t encrypt_key_256[kcipher_count*4] = {0};
uint64_t decrypt_key_128[kcipher_count*4] = {0};
uint64_t decrypt_key_192[kcipher_count*4] = {0};
uint64_t decrypt_key_256[kcipher_count*4] = {0};
uint64_t encrypt_128[kcipher_count*4] = {0};
uint64_t encrypt_192[kcipher_count*4] = {0};
uint64_t encrypt_256[kcipher_count*4] = {0};
uint64_t decrypt_128[kcipher_count*4] = {0};
uint64_t decrypt_192[kcipher_count*4] = {0};
uint64_t decrypt_256[kcipher_count*4] = {0};
uint64_t ecb_encrypt_128[kcipher_count*4] = {0};
uint64_t ecb_encrypt_192[kcipher_count*4] = {0};
uint64_t ecb_encrypt_256[kcipher_count*4] = {0};
uint64_t ecb_decrypt_128[kcipher_count*4] = {0};
uint64_t ecb_decrypt_192[kcipher_count*4] = {0};
uint64_t ecb_decrypt_256[kcipher_count*4] = {0};
uint64_t cbc_encrypt_128[kcipher_count*4] = {0};
uint64_t cbc_encrypt_192[kcipher_count*4] = {0};
uint64_t cbc_encrypt_256[kcipher_count*4] = {0};
uint64_t cbc_decrypt_128[kcipher_count*4] = {0};
uint64_t cbc_decrypt_192[kcipher_count*4] = {0};
uint64_t cbc_decrypt_256[kcipher_count*4] = {0};

rc_t run_one (KCipher * ciphers[16], clock_t timer, unsigned ix)
{
    rc_t rc;

    encrypt_key_128[ix] = 0;
    encrypt_key_192[ix] = 0;
    encrypt_key_256[ix] = 0;
    decrypt_key_128[ix] = 0;
    decrypt_key_192[ix] = 0;
    decrypt_key_256[ix] = 0;

    encrypt_128[ix] = 0;
    encrypt_192[ix] = 0;
    encrypt_256[ix] = 0;
    decrypt_128[ix] = 0;
    decrypt_192[ix] = 0;
    decrypt_256[ix] = 0;

    ecb_encrypt_128[ix] = 0;
    ecb_encrypt_192[ix] = 0;
    ecb_encrypt_256[ix] = 0;
    ecb_decrypt_128[ix] = 0;
    ecb_decrypt_192[ix] = 0;
    ecb_decrypt_256[ix] = 0;

    cbc_encrypt_128[ix] = 0;
    cbc_encrypt_192[ix] = 0;
    cbc_encrypt_256[ix] = 0;
    cbc_decrypt_128[ix] = 0;
    cbc_decrypt_192[ix] = 0;
    cbc_decrypt_256[ix] = 0;

    KStsMsg ("There will be %u seconds between outputs", timer);
    KStsMsg ("set_encrypt_key_function_128");
    rc = function_timer(set_encrypt_key_function_128, ciphers, encrypt_key_128+ix, timer);
    if (rc)
        KStsMsg ("failed to run set_encrypt_key_function_128\n");
    else
    {
        KStsMsg ("set_decrypt_key_function_128");
        rc = function_timer(set_decrypt_key_function_128, ciphers, decrypt_key_128+ix, timer);
        if (rc)
            KStsMsg ("failed to run set_decrypt_key_function_128\n");
        else
        {
            KStsMsg ("encrypt_function");
            rc = function_timer(encrypt_function, ciphers, encrypt_128+ix, timer);
            if (rc)
                KStsMsg ("failed to run encrypt_function\n");
            else
            {

                KStsMsg ("decrypt_function");
                rc = function_timer(decrypt_function, ciphers, decrypt_128+ix, timer);
                if (rc)
                    KStsMsg ("failed to run decrypt_function\n");
                else
                {
                    KStsMsg ("ecb_encrypt_function");
                    rc = function_timer(ecb_encrypt_function, ciphers, ecb_encrypt_128+ix, timer);
                    if (rc)
                        KStsMsg ("failed to run ecb_encrypt_function\n");
                    else
                    {

                        KStsMsg ("ecb_decrypt_function");
                        rc = function_timer(ecb_decrypt_function, ciphers, ecb_decrypt_128+ix, timer);
                        if (rc)
                            KStsMsg ("failed to run ecb_decrypt_function\n");
                        else
                        {
                            KStsMsg ("cbc_encrypt_function");
                            rc = function_timer(cbc_encrypt_function, ciphers, cbc_encrypt_128+ix, timer);
                            if (rc)
                                KStsMsg ("failed to run cbc_encrypt_function\n");
                            else
                            {
                                KStsMsg ("cbc_decrypt_function\n");
                                rc = function_timer(cbc_decrypt_function, ciphers, cbc_decrypt_128+ix, timer);
                                if (rc)
                                    KStsMsg ("failed to run cbc_decrypt_function\n");
                            }
                        }
                    }
                }
            }
        }
    }

    KStsMsg ("set_encrypt_key_function_192\n");
    rc = function_timer(set_encrypt_key_function_192, ciphers, encrypt_key_192+ix, timer);
    if (rc)
        KStsMsg ("failed to run set_encrypt_key_function_192\n");
    else
    {
        KStsMsg ("set_decrypt_key_function_192\n");
        rc = function_timer(set_decrypt_key_function_192, ciphers, decrypt_key_192+ix, timer);
        if (rc)
            KStsMsg ("failed to run set_decrypt_key_function_192\n");
        else
        {
            KStsMsg ("encrypt_function\n");
            rc = function_timer(encrypt_function, ciphers, encrypt_192+ix, timer);
            if (rc)
                KStsMsg ("failed to run encrypt_function\n");
            else
            {
                KStsMsg ("decrypt_function\n");
                rc = function_timer(decrypt_function, ciphers, decrypt_192+ix, timer);
                if (rc)
                    KStsMsg ("failed to run decrypt_function\n");
                else
                {
                    KStsMsg ("ecb_encrypt_function\n");
                    rc = function_timer(ecb_encrypt_function, ciphers, ecb_encrypt_192+ix, timer);
                    if (rc)
                        KStsMsg ("failed to run ecb_encrypt_function\n");
                    else
                    {
                        KStsMsg ("ecb_decrypt_function\n");
                        rc = function_timer(ecb_decrypt_function, ciphers, ecb_decrypt_192+ix, timer);
                        if (rc)
                            KStsMsg ("failed to run ecb_decrypt_function\n");
                        else
                        {
                            KStsMsg ("cbc_encrypt_function\n");
                            rc = function_timer(cbc_encrypt_function, ciphers, cbc_encrypt_192+ix, timer);
                            if (rc)
                                KStsMsg ("failed to run cbc_encrypt_function\n");
                            else
                            {
                                KStsMsg ("cbc_decrypt_function\n");
                                rc = function_timer(cbc_decrypt_function, ciphers, cbc_decrypt_192+ix, timer);
                                if (rc)
                                    KStsMsg ("failed to run cbc_decrypt_function\n");
                            }
                        }
                    }
                }
            }
        }
    }

    KStsMsg ("set_encrypt_key_function_256\n");
    rc = function_timer(set_encrypt_key_function_256, ciphers, encrypt_key_256+ix, timer);
    if (rc)
        KStsMsg ("failed to run set_encrypt_key_function_256\n");
    else
    {
        KStsMsg ("set_decrypt_key_function_256\n");
        rc = function_timer(set_decrypt_key_function_256, ciphers, decrypt_key_256+ix, timer);
        if (rc)
            KStsMsg ("failed to run set_decrypt_key_function_256\n");
        else
        {
            KStsMsg ("encrypt_function\n");
            rc = function_timer(encrypt_function, ciphers, encrypt_256+ix, timer);
            if (rc)
                KStsMsg ("failed to run encrypt_function\n");
            else
            {
                KStsMsg ("decrypt_function\n");
                rc = function_timer(decrypt_function, ciphers, decrypt_256+ix, timer);
                if (rc)
                    KStsMsg ("failed to rundecrypt_function\n");
                else
                {
                    KStsMsg ("ecb_encrypt_function\n");
                    rc = function_timer(ecb_encrypt_function, ciphers, ecb_encrypt_256+ix, timer);
                    if (rc)
                        KStsMsg ("failed to run ecb_encrypt_function\n");
                    else
                    {

                        KStsMsg ("ecb_decrypt_function\n");  
                        rc = function_timer(ecb_decrypt_function, ciphers, ecb_decrypt_256+ix, timer);
                        if (rc)
                            KStsMsg ("failed to run ecb_decrypt_function\n");
                        else
                        {
                            KStsMsg ("cbc_encrypt_function\n");
                            rc = function_timer(cbc_encrypt_function, ciphers, cbc_encrypt_256+ix, timer);
                            if (rc)
                                KStsMsg ("failed to run cbc_encrypt_function\n");
                            else
                            {
                                KStsMsg ("cbc_decrypt_function\n");
                                rc = function_timer(cbc_decrypt_function, ciphers, cbc_decrypt_256+ix, timer);
                                if (rc)
                                    KStsMsg ("failed to run cbc_decrypt_function\n");
                            }
                        }
                    }
                }
            }
        }
    }
    return rc;
}


void run (clock_t timer)
{
    kcipher_type cipher_type;

    for (cipher_type = 0; cipher_type < kcipher_count; ++cipher_type)
    {
        uint32_t cipher_subtype;

        for (cipher_subtype = 0; cipher_subtype < 4; ++cipher_subtype)
        {
            KCipher * ciphers[16];
            unsigned ix;
            rc_t rc;

            for (ix = 0; ix < 16; ++ix)
                ciphers[ix] = NULL;

            for (ix = 0; ix < 16; ++ix)
            {
                rc = make_functions[cipher_subtype](&ciphers[ix], cipher_type);
                if (GetRCState(rc) == rcUnsupported)
                {
                    KOutMsg ("Not supported on this machine: test not run\n");
                    ciphers[ix] = NULL;
                    break;
                }
                else if (rc)
                {
                    KOutMsg ("Failure creating cipher: test not run\n");
                    ciphers[ix] = NULL;
                    break;
                }
            }
            if (rc == 0)
            {
                KStsMsg ("Testing the %s cipher in the %s implementation",
                         cipher_types[cipher_type],cipher_subtypes[cipher_subtype]);
                rc = run_one (ciphers, timer, cipher_type * 4 + cipher_subtype);
            }
            for (ix = 0; ix < 16; ++ix)
            {
                KCipherRelease (ciphers[ix]);
            }
        }
    }
    KOutMsg ("%14s\t%s-%s\t%s-%s\t%s-%s\t%s-%s\t%s-%s\t%s-%s\t%s-%s\t%s-%s\n",
             " ",
             cipher_types[0],cipher_subtypes[0],
             cipher_types[0],cipher_subtypes[1],
             cipher_types[0],cipher_subtypes[2],
             cipher_types[0],cipher_subtypes[3],
             cipher_types[1],cipher_subtypes[0],
             cipher_types[1],cipher_subtypes[1],
             cipher_types[1],cipher_subtypes[2],
             cipher_types[1],cipher_subtypes[3]);

    KOutMsg ("encrypt_key_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             encrypt_key_128[0],encrypt_key_128[1],encrypt_key_128[2],
             encrypt_key_128[3],encrypt_key_128[4],encrypt_key_128[5],
             encrypt_key_128[6],encrypt_key_128[7]);
    KOutMsg ("encrypt_key_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             encrypt_key_192[0],encrypt_key_192[1],encrypt_key_192[2],
             encrypt_key_192[3],encrypt_key_192[4],encrypt_key_192[5],
             encrypt_key_192[6],encrypt_key_192[7]);
    KOutMsg ("encrypt_key_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             encrypt_key_256[0],encrypt_key_256[1],encrypt_key_256[2],
             encrypt_key_256[3],encrypt_key_256[4],encrypt_key_256[5],
             encrypt_key_256[6],encrypt_key_256[7]);
    KOutMsg ("decrypt_key_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             decrypt_key_128[0],decrypt_key_128[1],decrypt_key_128[2],
             decrypt_key_128[3],decrypt_key_128[4],decrypt_key_128[5],
             decrypt_key_128[6],decrypt_key_128[7]);
    KOutMsg ("decrypt_key_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             decrypt_key_192[0],decrypt_key_192[1],decrypt_key_192[2],
             decrypt_key_192[3],decrypt_key_192[4],decrypt_key_192[5],
             decrypt_key_192[6],decrypt_key_192[7]);
    KOutMsg ("decrypt_key_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             decrypt_key_256[0],decrypt_key_256[1],decrypt_key_256[2],
             decrypt_key_256[3],decrypt_key_256[4],decrypt_key_256[5],
             decrypt_key_256[6],decrypt_key_256[7]);

    KOutMsg ("encrypt_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             encrypt_128[0],encrypt_128[1],encrypt_128[2],
             encrypt_128[3],encrypt_128[4],encrypt_128[5],
             encrypt_128[6],encrypt_128[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (encrypt_128[0]*16.0/(1024*1024))/timer,
             (encrypt_128[1]*16.0/(1024*1024))/timer,
             (encrypt_128[2]*16.0/(1024*1024))/timer,
             (encrypt_128[3]*16.0/(1024*1024))/timer,
             (encrypt_128[4]*16.0/(1024*1024))/timer,
             (encrypt_128[5]*16.0/(1024*1024))/timer,
             (encrypt_128[6]*16.0/(1024*1024))/timer,
             (encrypt_128[7]*16.0/(1024*1024))/timer);

    KOutMsg ("encrypt_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             encrypt_192[0],encrypt_192[1],encrypt_192[2],
             encrypt_192[3],encrypt_192[4],encrypt_192[5],
             encrypt_192[6],encrypt_192[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (encrypt_192[0]*16.0/(1024*1024))/timer,
             (encrypt_192[1]*16.0/(1024*1024))/timer,
             (encrypt_192[2]*16.0/(1024*1024))/timer,
             (encrypt_192[3]*16.0/(1024*1024))/timer,
             (encrypt_192[4]*16.0/(1024*1024))/timer,
             (encrypt_192[5]*16.0/(1024*1024))/timer,
             (encrypt_192[6]*16.0/(1024*1024))/timer,
             (encrypt_192[7]*16.0/(1024*1024))/timer);

    KOutMsg ("encrypt_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             encrypt_256[0],encrypt_256[1],encrypt_256[2],
             encrypt_256[3],encrypt_256[4],encrypt_256[5],
             encrypt_256[6],encrypt_256[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (encrypt_256[0]*16.0/(1024*1024))/timer,
             (encrypt_256[1]*16.0/(1024*1024))/timer,
             (encrypt_256[2]*16.0/(1024*1024))/timer,
             (encrypt_256[3]*16.0/(1024*1024))/timer,
             (encrypt_256[4]*16.0/(1024*1024))/timer,
             (encrypt_256[5]*16.0/(1024*1024))/timer,
             (encrypt_256[6]*16.0/(1024*1024))/timer,
             (encrypt_256[7]*16.0/(1024*1024))/timer);

    KOutMsg ("decrypt_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             decrypt_128[0],decrypt_128[1],decrypt_128[2],
             decrypt_128[3],decrypt_128[4],decrypt_128[5],
             decrypt_128[6],decrypt_128[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (decrypt_128[0]*16.0/(1024*1024))/timer,
             (decrypt_128[1]*16.0/(1024*1024))/timer,
             (decrypt_128[2]*16.0/(1024*1024))/timer,
             (decrypt_128[3]*16.0/(1024*1024))/timer,
             (decrypt_128[4]*16.0/(1024*1024))/timer,
             (decrypt_128[5]*16.0/(1024*1024))/timer,
             (decrypt_128[6]*16.0/(1024*1024))/timer,
             (decrypt_128[7]*16.0/(1024*1024))/timer);

    KOutMsg ("decrypt_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             decrypt_192[0],decrypt_192[1],decrypt_192[2],
             decrypt_192[3],decrypt_192[4],decrypt_192[5],
             decrypt_192[6],decrypt_192[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (decrypt_192[0]*16.0/(1024*1024))/timer,
             (decrypt_192[1]*16.0/(1024*1024))/timer,
             (decrypt_192[2]*16.0/(1024*1024))/timer,
             (decrypt_192[3]*16.0/(1024*1024))/timer,
             (decrypt_192[4]*16.0/(1024*1024))/timer,
             (decrypt_192[5]*16.0/(1024*1024))/timer,
             (decrypt_192[6]*16.0/(1024*1024))/timer,
             (decrypt_192[7]*16.0/(1024*1024))/timer);

    KOutMsg ("decrypt_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             decrypt_256[0],decrypt_256[1],decrypt_256[2],
             decrypt_256[3],decrypt_256[4],decrypt_256[5],
             decrypt_256[6],decrypt_256[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (decrypt_256[0]*16.0/(1024*1024))/timer,
             (decrypt_256[1]*16.0/(1024*1024))/timer,
             (decrypt_256[2]*16.0/(1024*1024))/timer,
             (decrypt_256[3]*16.0/(1024*1024))/timer,
             (decrypt_256[4]*16.0/(1024*1024))/timer,
             (decrypt_256[5]*16.0/(1024*1024))/timer,
             (decrypt_256[6]*16.0/(1024*1024))/timer,
             (decrypt_256[7]*16.0/(1024*1024))/timer);

    KOutMsg ("ecb_encrypt_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             ecb_encrypt_128[0],ecb_encrypt_128[1],ecb_encrypt_128[2],
             ecb_encrypt_128[3],ecb_encrypt_128[4],ecb_encrypt_128[5],
             ecb_encrypt_128[6],ecb_encrypt_128[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (ecb_encrypt_128[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_128[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("ecb_encrypt_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             ecb_encrypt_192[0],ecb_encrypt_192[1],ecb_encrypt_192[2],
             ecb_encrypt_192[3],ecb_encrypt_192[4],ecb_encrypt_192[5],
             ecb_encrypt_192[6],ecb_encrypt_192[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (ecb_encrypt_192[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_192[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("ecb_encrypt_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             ecb_encrypt_256[0],ecb_encrypt_256[1],ecb_encrypt_256[2],
             ecb_encrypt_256[3],ecb_encrypt_256[4],ecb_encrypt_256[5],
             ecb_encrypt_256[6],ecb_encrypt_256[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (ecb_encrypt_256[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_encrypt_256[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("ecb_decrypt_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             ecb_decrypt_128[0],ecb_decrypt_128[1],ecb_decrypt_128[2],
             ecb_decrypt_128[3],ecb_decrypt_128[4],ecb_decrypt_128[5],
             ecb_decrypt_128[6],ecb_decrypt_128[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (ecb_decrypt_128[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_128[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("ecb_decrypt_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             ecb_decrypt_192[0],ecb_decrypt_192[1],ecb_decrypt_192[2],
             ecb_decrypt_192[3],ecb_decrypt_192[4],ecb_decrypt_192[5],
             ecb_decrypt_192[6],ecb_decrypt_192[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (ecb_decrypt_192[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_192[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("ecb_decrypt_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             ecb_decrypt_256[0],ecb_decrypt_256[1],ecb_decrypt_256[2],
             ecb_decrypt_256[3],ecb_decrypt_256[4],ecb_decrypt_256[5],
             ecb_decrypt_256[6],ecb_decrypt_256[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (ecb_decrypt_256[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (ecb_decrypt_256[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("cbc_encrypt_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             cbc_encrypt_128[0],cbc_encrypt_128[1],cbc_encrypt_128[2],
             cbc_encrypt_128[3],cbc_encrypt_128[4],cbc_encrypt_128[5],
             cbc_encrypt_128[6],cbc_encrypt_128[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (cbc_encrypt_128[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("cbc_encrypt_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             cbc_encrypt_192[0],cbc_encrypt_192[1],cbc_encrypt_192[2],
             cbc_encrypt_192[3],cbc_encrypt_192[4],cbc_encrypt_192[5],
             cbc_encrypt_192[6],cbc_encrypt_192[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (cbc_encrypt_128[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_encrypt_128[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("cbc_encrypt_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             cbc_encrypt_256[0],cbc_encrypt_256[1],cbc_encrypt_256[2],
             cbc_encrypt_256[3],cbc_encrypt_256[4],cbc_encrypt_256[5],
             cbc_encrypt_256[6],cbc_encrypt_256[7]);
    KOutMsg ("cbc_decrypt_128\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             cbc_decrypt_128[0],cbc_decrypt_128[1],cbc_decrypt_128[2],
             cbc_decrypt_128[3],cbc_decrypt_128[4],cbc_decrypt_128[5],
             cbc_decrypt_128[6],cbc_decrypt_128[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (cbc_decrypt_128[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_128[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("cbc_decrypt_192\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             cbc_decrypt_192[0],cbc_decrypt_192[1],cbc_decrypt_192[2],
             cbc_decrypt_192[3],cbc_decrypt_192[4],cbc_decrypt_192[5],
             cbc_decrypt_192[6],cbc_decrypt_192[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (cbc_decrypt_192[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_192[7]*sizeof(cipher_block)/(1024*1024))/timer);

    KOutMsg ("cbc_decrypt_256\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\t%10lu\n",
             cbc_decrypt_256[0],cbc_decrypt_256[1],cbc_decrypt_256[2],
             cbc_decrypt_256[3],cbc_decrypt_256[4],cbc_decrypt_256[5],
             cbc_decrypt_256[6],cbc_decrypt_256[7]);
    KOutMsg ("MB/s       \t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\t%10.2f\n\n",
             (cbc_decrypt_256[0]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[1]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[2]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[3]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[4]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[5]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[6]*sizeof(cipher_block)/(1024*1024))/timer,
             (cbc_decrypt_256[7]*sizeof(cipher_block)/(1024*1024))/timer);
}


rc_t CC UsageSummary  (const char * progname)
{
    return KOutMsg ("\n"
                    "Usage:\n"
                    "  %s [OPTIONS]\n"
                    "\n"
                    "Summary:\n"
                    "  Run a null (copy) cipher and the AES cipher in all\n"
                    "  modes supported on this machine.  Encrypt a block of\n"
                    "  32KB a number of times for each mode and compute an\n"
                    "  expected throughput for each mode. Expected long delays\n"
                    "  of no output for long timer values\n",
                    progname);
}


const char UsageDefaultName[] = "test-cipher-throughput";
rc_t CC Usage (const Args * args)
{
    rc_t rc;

    rc = UsageSummary (UsageDefaultName);
    if (rc == 0)
    {
        KOutMsg ("Options:\n");
        HelpOptionLine (ALIAS_TIMER, OPTION_TIMER, "seconds", timer_usage);
        HelpOptionsStandard();
    }
    return rc;
}


ver_t CC KAppVersion (void)
{
    return 0;
}


rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;
    rc_t rc;

    rc = KStsHandlerSetStdErr();
    if (rc == 0)
    {
        rc = ArgsMakeAndHandle (&args, argc, argv, 1, Options, sizeof Options / sizeof (Options[0]));
        if (rc == 0)
        {
            clock_t timer;
            uint32_t pcount;

            rc = ArgsOptionCount (args, OPTION_TIMER, &pcount);
            if (rc == 0)
            {
                if (pcount == 0)
                    timer = 5*60;
                else
                {
                    const char * pc;

                    rc = ArgsOptionValue (args, OPTION_TIMER, 0, (const void **)&pc);
                    if (rc == 0)
                    {
                        timer = (clock_t)AsciiToU64 (pc, NULL, NULL);
                    }
                }
            }
            if (rc == 0)
                run (timer);
        }
    }

    if (rc)
        LOGERR (klogErr, rc, "Exiting with a failure status");
    return rc;
}
