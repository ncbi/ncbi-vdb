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
/*
 * don't compare inverse cipher when we know we are using 
 * equivalent inverse cipher
 */
#define COMPARE_INV_CIPHER 0

/*
 * do compare equivalent inverse cipher when we know we are using 
 * equivalent inverse cipher
 */
#define COMPARE_EQ_INV_CIPHER 1

#include <kapp/main.h>
#include <kapp/args.h>

#include <krypto/ciphermgr.h>
#include <krypto/cipher.h>
#include <krypto/cipher-test.h>
#include "../../libs/krypto/blockcipher-priv.h"
#include "../../libs/krypto/cipher-priv.h"
#include "../../libs/krypto/cipher-impl.h"
#include "../../libs/krypto/aes-priv.h"

#include <klib/status.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <klib/defs.h>

#include <byteswap.h>

#include <string.h>
#include <stdlib.h>

/*
  https://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf

  http://www.inconteam.com/software-development/41-encryption/55-aes-test-vectors
*/

const char * types[]
= 
{
    "AES byte",
    "AES vector",
    "AES vector register",
    "AES accelerated(AES-NI)"
};

static const char * passed_string[] = {"DID NOT MATCH ++++++++++","matched"};

int error_count;
int test_count;
int subtype_iterator;


bool key_compare (const AES_KEY * control, const AES_KEY * test)
{
    uint32_t ix;
    uint32_t limit;
    bool passed_rounds = true;
    bool passed_keys = true;

    passed_rounds = (control->rounds == test->rounds);

    STSMSG (4,("rounds:\t%d : %d\t%s\n", control->rounds, test->rounds,
               passed_string[passed_rounds]));

    limit = (control->rounds + 1) * 4;
    for (ix = 0; ix < limit; ++ix)
    {
        bool this_key;

        this_key = ((control->rd_key[ix] == bswap_32(test->rd_key[ix])) ||
                    (control->rd_key[ix] == test->rd_key[ix]));

        passed_keys = passed_keys && this_key;

        STSMSG (4,("rd_key[%d]\t%0.8x : %0.8x\t%s\n",
                   ix, control->rd_key[ix], test->rd_key[ix],
                   passed_string[this_key]));
    }

    return passed_rounds && passed_keys;
}


bool block_compare (const uint8_t control [16], const uint8_t test [16])
{
    uint32_t ix;
    bool passed = true;

    for (ix = 0; ix < 16; ++ix)
    {
        bool passed_this;

        passed_this = (control[ix] == test[ix]);

        passed = passed && passed_this;

        STSMSG (4,("byte %u: control %0.2x test %0.2x %s\n",
                   ix, 
                   control[ix], test[ix],
                   passed_string[passed_this]));
    }
    return passed;
}


typedef struct key_expansion
{
    uint8_t key [32];
    AES_KEY key_schedule;
    unsigned Nk;
} key_expansion;


const key_expansion  a_1 = 
{
    {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    },
    {
        {
            0x16157e2b, 0xa6d2ae28, 0x8815f7ab, 0x3c4fcf09,
            0x17fefaa0, 0xb12c5488, 0x3939a323, 0x05766c2a,
            0xf295c2f2, 0x43b9967a, 0x7a803559, 0x7ff65973,
            0x7d47803d, 0x3efe1647, 0x447e231e, 0x3b887a6d,
            0x41a544ef, 0x7f5b52a8, 0x3b2571b6, 0x00ad0bdb,
            0xf8c6d1d4, 0x879d837c, 0xbcb8f2ca, 0xbc15f911,
            0x7aa3886d, 0xfd3e0b11, 0x4186f9db, 0xfd9300ca,
            0x0ef7544e, 0xf3c95f5f, 0xb24fa684, 0x4fdca64e,
            0x2173d2ea, 0xd2ba8db5, 0x60f52b31, 0x2f298d7f,
            0xf36677ac, 0x21dcfa19, 0x4129d128, 0x6e005c57,
            0xa8f914d0, 0x8925eec9, 0xc80c3fe1, 0xa60c63b6
        },
        10
    },
    16
};


const key_expansion a_2 = {
    { 0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52,
      0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5,
      0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b },
    { 
        { 0xf7b0738e, 0x52640eda, 0x2bf310c8, 0xe5799080, /* 0-3 */
          0xd2eaf862, 0x7b6b2c52, 0xf7910cfe, 0xa5f50224, /* 4-7 */
          0x8e0612ec, 0x6b7f826c, 0xb9957a0e, 0xc2fe565c, /* 8-11 */
          0xbdb4b74d, 0x1841b569, 0x9647a785, 0xfd3825e9, /* 12-15 */
          0x44ad5fe7, 0x865309bb, 0x57f05a48, 0x4fb1ef21, /* 16-19 */
          0xd9f648a4, 0x24ce6d4d, 0x606332aa, 0xe6303b11, /* 20-23 */
          0xd57e5ea2, 0x9acfb183, 0x4339f927, 0x67f7946a, /* 24-27 */
          0x0794a6c0, 0xe1a49dd1, 0xeb8617ec, 0x7149a66f, /* 28-31 */
          0x32705f48, 0x5587cb22, 0x52136de2, 0xb3b7f033, /* 32-35 */
          0x28ebbe40, 0x59a2182f, 0x6bd24767, 0x3e558c45, /* 36-39 */
          0x6c46e1a7, 0xdff11194, 0x0a751f82, 0x53d707ad, /* 40-43 */
          0x380540ca, 0x0650cc8f, 0x6a162d28, 0xb5e73cbc, /* 44-47 */
          0x6fa08be9, 0x3c778c44, 0x0472cc8e, 0x02220001  /* 48-51 */
        },
        12
    },
    24
};

const key_expansion a_3 = {
    { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
      0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,

      0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 },
    {    { 0x10eb3d60, 0xbe71ca15, 0xf0ae732b, 0x81777d85,
           0x072c351f, 0xd708613b, 0xa310982d, 0xf4df1409,
           0x1154a39b, 0xaf25698e, 0x5f8b1aa5, 0xdefc6720,
           0x1a9cb0a8, 0xcd94d193, 0x6e8449be, 0x9a5b5db7,
           0xb8ec9ad5, 0x17c9f35b, 0x4842e9fe, 0x96be8ede,
           0x8a32a9b5, 0x47a67826, 0x29223198, 0xb3796c2f,
           0xad812c81, 0xba48dfda, 0xf20a3624, 0x64b4b8fa,
           0xc9bfc598, 0x8e19bdbe, 0xa73b8c26, 0x1442e009,
           0xac7b0068, 0x1633dfb2, 0xe439e996, 0x808d516c,
           0x04e214c8, 0x8afba976, 0x2dc02550, 0x3982c559,
           0x676913de, 0x715acc6c, 0x956325fa, 0x15ee7496,
           0x5dca8658, 0xd7312f2e, 0xfaf10a7e, 0xc373cf27,
           0xab479c74, 0xda1d5018, 0x4f7e75e2, 0x5a900174,
           0xe3aafaca, 0x349bd5e4, 0xce6adf9a, 0x0d1910bd,
           0xd19048fe, 0x0b8d18e6, 0x44f36d04, 0x1e636c70, },
         14
    },
    32
};

bool KeyExpansion (KCipher * cipher, const key_expansion * ke)
{
    rc_t rc;

    rc = KCipherSetEncryptKey (cipher, ke->key, ke->Nk);
    if (rc)
    {
        STSMSG (1,("Could not run test"));
        return false;
    }
    else
    {
        return (key_compare (&ke->key_schedule, cipher->encrypt_key));
    }
}


typedef struct cipher_example
{
    uint8_t key [16];
    AES_KEY key_schedule;
    uint8_t plain [16];
    uint8_t cipher [16];
} cipher_example;

const cipher_example b = 
{
    { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
      0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    },
    {
        { 0x16157e2b, 0xa6d2ae28, 0x8815f7ab, 0x3c4fcf09,
          0x17fefaa0, 0xb12c5488, 0x3939a323, 0x05766c2a,
          0xf295c2f2, 0x43b9967a, 0x7a803559, 0x7ff65973,
          0x7d47803d, 0x3efe1647, 0x447e231e, 0x3b887a6d,
          0x41a544ef, 0x7f5b52a8, 0x3b2571b6, 0x00ad0bdb,
          0xf8c6d1d4, 0x879d837c, 0xbcb8f2ca, 0xbc15f911,
          0x7aa3886d, 0xfd3e0b11, 0x4186f9db, 0xfd9300ca,
          0x0ef7544e, 0xf3c95f5f, 0xb24fa684, 0x4fdca64e,
          0x2173d2ea, 0xd2ba8db5, 0x60f52b31, 0x2f298d7f,
          0xf36677ac, 0x21dcfa19, 0x4129d128, 0x6e005c57,
          0xa8f914d0, 0x8925eec9, 0xc80c3fe1, 0xa60c63b6 },
        10
    },
    {
        
        0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D,
        0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34
    },
    {
        0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
        0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32
    }
};


bool CipherExample (KCipher * cipher)
{
    uint8_t cipher_text [16];
    rc_t rc;
    bool passed_key;
    bool passed_block;

    memset (cipher_text, 0, sizeof cipher_text);

    rc = KCipherSetEncryptKey (cipher, b.key, 128/8);
    if (rc)
        ;
    else
    {
        passed_key = key_compare (&b.key_schedule, cipher->encrypt_key);
        rc = KCipherEncrypt (cipher, b.plain, cipher_text);
        if (rc)
            ;
        else
        {
            passed_block = block_compare (b.cipher, cipher_text);
        }
    }
    if (rc)
    {
        STSMSG (1,("Could not run test"));
        return false;
    }
    return passed_key && passed_block;
}

typedef struct example_vectors
{
    uint8_t plain [16];
    uint8_t key [32];
    uint8_t cipher [16];
    AES_KEY key_enc;
    AES_KEY key_dec;
    AES_KEY key_dec_alt;
} example_vectors;


example_vectors c_1 = 
{
    /* Plain text */
    {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    },
    /* User Key */
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    },
    /* Cipher Text */
    {
        0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a
    },
    {
/* EncKey */
        {


            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
            0xfd74aad6, 0xfa72afd2, 0xf178a6da, 0xfe76abd6,
            0x0bcf92b6, 0xf1bd3d64, 0x00c59bbe, 0xfeb33068,
            0x4e74ffb6, 0xbfc9c2d2, 0xbf0c596c, 0x41bf6904,
            0xbcf7f747, 0x033e3595, 0xbc326cf9, 0xfd8d05fd,
            0xe8a3aa3c, 0xeb9d9fa9, 0x57aff350, 0xaa22f6ad,
            0x7d0f395e, 0x9692a6f7, 0xc13d55a7, 0x6b1fa30a,
            0x1a70f914, 0x8ce25fe3, 0x4ddf0a44, 0x26c0a94e,
            0x35874347, 0xb9651ca4, 0xf4ba16e0, 0xd27abfae,
            0xd1329954, 0x685785f0, 0x9ced9310, 0x4e972cbe,
            0x7f1d1113, 0x174a94e3, 0x8ba707f3, 0xc5302b4d
        },
        10
    },
    {
/* DecKey */
        {
            0x7f1d1113, 0x174a94e3, 0x8ba707f3, 0xc5302b4d,
            0xd1329954, 0x685785f0, 0x9ced9310, 0x4e972cbe,
            0x35874347, 0xb9651ca4, 0xf4ba16e0, 0xd27abfae,
            0x1a70f914, 0x8ce25fe3, 0x4ddf0a44, 0x26c0a94e,
            0x7d0f395e, 0x9692a6f7, 0xc13d55a7, 0x6b1fa30a,
            0xe8a3aa3c, 0xeb9d9fa9, 0x57aff350, 0xaa22f6ad,
            0xbcf7f747, 0x033e3595, 0xbc326cf9, 0xfd8d05fd,
            0x4e74ffb6, 0xbfc9c2d2, 0xbf0c596c, 0x41bf6904,
            0x0bcf92b6, 0xf1bd3d64, 0x00c59bbe, 0xfeb33068,
            0xfd74aad6, 0xfa72afd2, 0xf178a6da, 0xfe76abd6,
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c
        },
        10
    },
    {
/* EquivDecKey */
        {
            0x7f1d1113, 0x174a94e3, 0x8ba707f3, 0xc5302b4d,
            0xbe29aa13, 0xf6af8f9c, 0x80f570f7, 0x03bff700,
            0x63a46213, 0x4886258f, 0x765aff6b, 0x834a87f7,
            0x74fc828d, 0x2b22479c, 0x3edcdae4, 0xf510789c,
            0x8d09e372, 0x5fdec511, 0x15fe9d78, 0xcbcca278,
            0x2710c42e, 0xd2d72663, 0x4a205869, 0xde323f00,
            0x04f5a2a8, 0xf5c7e24d, 0x98f77e0a, 0x94126769,
            0x91e3c6c7, 0xf13240e5, 0x6d309c47, 0x0ce51963,
            0x9902dba0, 0x60d18622, 0x9c02dca2, 0x61d58524,
            0xf0df568c, 0xf9d35d82, 0xfcd35a80, 0xfdd75986,
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
        },
        10
    }
};

example_vectors c_2 = 
{
    /* Plain text */
    {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    },
    /* User Key */
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
    },
    /* Cipher Text */
    {
        0xdd, 0xa9, 0x7c, 0xa4, 0x86, 0x4c, 0xdf, 0xe0, 0x6e, 0xaf, 0x70, 0xa0, 0xec, 0x0d, 0x71, 0x91
    },
    {
/* EncKey */
        {
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
            0x13121110, 0x17161514, 0xf9f24658, 0xfef4435c,
            0xf5fe4a54, 0xfaf04758, 0xe9e25648, 0xfef4435c,
            0xb349f940, 0x4dbdba1c, 0xb843f048, 0x42b3b710,
            0xab51e158, 0x55a5a204, 0x41b5ff7e, 0x0c084562,
            0xb44bb52a, 0xf6f8023a, 0x5da9e362, 0x080c4166,
            0x728501f5, 0x7e8d4497, 0xcac6f1bd, 0x3c3ef387,
            0x619710e5, 0x699b5183, 0x9e7c1534, 0xe0f151a3,
            0x2a37a01e, 0x16095399, 0x779e437c, 0x1e0512ff,
            0x880e7edd, 0x68ff2f7e, 0x42c88f60, 0x54c1dcf9,
            0x235f9f85, 0x3d5a8d7a, 0x5229c0c0, 0x3ad6efbe,
            0x781e60de, 0x2cdfbc27, 0x0f8023a2, 0x32daaed8,
            0x330a97a4, 0x09dc781a, 0x71c218c4, 0x5d1da4e3
        },
        12
    },
    {
/* DecKey */
        {
            0x330a97a4, 0x09dc781a, 0x71c218c4, 0x5d1da4e3,
        },
        12
    },
    {
/* EquivDecKey */
        {
            0x330a97a4, 0x09dc781a, 0x71c218c4, 0x5d1da4e3,
            0x0dbdbed6, 0x49ea09c2, 0x8073b04d, 0xb91b023e,
            0xc999b98f, 0x3968b273, 0x9dd8f9c7, 0x728cc685,
            0xc16e7df7, 0xef543f42, 0x7f317853, 0x4457b714,
            0x90654711, 0x3b66cf47, 0x8dce0e9b, 0xf0f10bfc,
            0xb6a8c1dc, 0x7d3f0567, 0x4a195ccc, 0x2e3a42b5,
            0xabb0dec6, 0x64231e79, 0xbe5f05a4, 0xab038856,
            0xda7c1bdd, 0x155c8df2, 0x1dab498a, 0xcb97c4bb,
            0x08f7c478, 0xd63c8d31, 0x01b75596, 0xcf93c0bf,
            0x10efdc60, 0xce249529, 0x15efdb62, 0xcf20962f,
            0xdbcb4e4b, 0xdacf4d4d, 0xc7d75257, 0xdecb4949,
            0x1d181f1a, 0x191c1b1e, 0xd7c74247, 0xdecb4949,
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c
        },
        12
    }
};

example_vectors c_3 = 
{
    /* Plain text */
    {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    },
    /* User Key */
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    },
    /* Cipher Text */
    {
        0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf, 0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89
    },
    {
/* EncKey */
        {
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
            0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c,
            0x9fc273a5, 0x98c476a1, 0x93ce7fa9, 0x9cc072a5,
            0xcda85116, 0xdabe4402, 0xc1a45d1a, 0xdeba4006,
            0xf0df87ae, 0x681bf10f, 0xfbd58ea6, 0x6715fc03,
            0x48f1e16d, 0x924fa56f, 0x53ebf875, 0x8d51b873,
            0x7f8256c6, 0x1799a7c9, 0xec4c296f, 0x8b59d56c,
            0x753ae23d, 0xe7754752, 0xb49ebf27, 0x39cf0754,
            0x5f90dc0b, 0x48097bc2, 0xa44552ad, 0x2f1c87c1,
            0x60a6f545, 0x87d3b217, 0x334d0d30, 0x0a820a64,
            0x1cf7cf7c, 0x54feb4be, 0xf0bbe613, 0xdfa761d2,
            0xfefa1af0, 0x7929a8e7, 0x4a64a5d7, 0x40e6afb3,
            0x71fe4125, 0x2500f59b, 0xd5bb1388, 0x0a1c725a,
            0x99665a4e, 0xe04ff2a9, 0xaa2b577e, 0xeacdf8cd,
            0xcc79fc24, 0xe97909bf, 0x3cc21a37, 0x36de686d
        },
        14
    },
    {
/* DecKey */
        {


            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c
        },
        14
    },
    {
/* EquivDecKey */
        {
            0xcc79fc24, 0xe97909bf, 0x3cc21a37, 0x36de686d,
            0xffd1f134, 0x2faacebf, 0x5fe2e9fc, 0x6e015825,
            0xeb48165e, 0x0a354c38, 0x46b77175, 0x84e680dc,
            0x8005a3c8, 0xd07b3f8b, 0x70482743, 0x31e3b1d9,
            0x138e70b5, 0xe17d5a66, 0x4c823d4d, 0xc251f1a9,
            0xa37bda74, 0x507e9c43, 0xa03318c8, 0x41ab969a,
            0x1597a63c, 0xf2f32ad3, 0xadff672b, 0x8ed3cce4,
            0xf3c45ff8, 0xf3054637, 0xf04d848b, 0xe1988e52,
            0x9a4069de, 0xe7648cef, 0x5f0c4df8, 0x232cabcf,
            0x1658d5ae, 0x00c119cf, 0x0348c2bc, 0x11d50ad9,
            0xbd68c615, 0x7d24e531, 0xb868c117, 0x7c20e637,
            0x0f85d77f, 0x1699cc61, 0x0389db73, 0x129dc865,
            0xc940282a, 0xc04c2324, 0xc54c2426, 0xc4482720,
            0x1d181f1a, 0x191c1b1e, 0x15101712, 0x11141316,
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c
        },
        14
    }
};

bool ExampleVector (KCipher * cipher, const example_vectors * ev)
{
    uint8_t cipher_text [16];
    uint8_t plain_text [16];
    uint32_t Nk;
    rc_t rc;
    bool passed_enckey;
    bool passed_deckey;
    bool passed_enc;
    bool passed_dec;

    switch (ev->key_enc.rounds)
    {
    default:
        STSMSG (1,("Bad test vector"));
        return false;
    case 10:
        Nk = 16;
        break;
    case 12:
        Nk = 24;
        break;
    case 14:
        Nk = 32;
        break;
    }

    memset (cipher_text, 0, sizeof cipher_text);
    memset (plain_text, 0, sizeof plain_text);

    rc = KCipherSetEncryptKey (cipher, ev->key, Nk);
    if (rc)
        ;
    else
    {
        passed_enckey = key_compare (&ev->key_enc, cipher->encrypt_key);

        rc = KCipherSetDecryptKey (cipher, ev->key, Nk);
        if (rc)
            ;
        else
        {
#if COMPARE_INV_CIPHER
            passed_deckey = key_compare (&ev->key_dec, cipher->decrypt_key);
#endif
#if COMPARE_EQ_INV_CIPHER
            passed_deckey = key_compare (&ev->key_dec_alt, cipher->decrypt_key);
#endif

            rc = KCipherEncrypt (cipher, ev->plain, cipher_text);
            if (rc)
                ;
            else
            {
                passed_enc = block_compare (ev->cipher, cipher_text);
                
                rc = KCipherDecrypt (cipher, ev->cipher, plain_text);
                if (rc)
                    ;
                else
                {
                    passed_dec = block_compare (ev->plain, plain_text);
                }
            }
        }
    }
    if (rc)
    {
        STSMSG (1,("Could not run test"));
        return false;
    }
    return passed_enckey && passed_deckey && passed_enc && passed_dec;
}


/* ==================================================== */

/* typedef uint8_t VVV __attribute__ ((vector_size (16))); unused and non-portable */ 

bool run_one (KCipher * cipher)
{
    bool p_a_1 = true;
    bool p_a_2 = true;
    bool p_a_3 = true;
    bool p_b = true;
    bool p_c_1 = true;
    bool p_c_2 = true;
    bool p_c_3 = true;

    STSMSG (2,("Appendix A - Key Expansion Examples"));
    STSMSG (3,("A.1 Expansion of a 128-bit Cipher Key"));
    p_a_1 = KeyExpansion (cipher, &a_1);
    ++test_count;
    if (p_a_1 == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    STSMSG (3,("A.2 Expansion of a 192-bit Cipher Key"));
    p_a_2 = KeyExpansion (cipher, &a_2);
    ++test_count;
    if (p_a_2 == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    STSMSG (3,("A.3 Expansion of a 256-bit Cipher Key"));
    p_a_3 = KeyExpansion (cipher, &a_3);
    ++test_count;
    if (p_a_3 == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    STSMSG (2,("Appendix B - Cipher Example"));
    p_b = CipherExample (cipher);
    ++test_count;
    if (p_b == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    STSMSG (2,("Appendix C - Example Vectors"));
    STSMSG (3,("C.1 AES-128 (Nk=4, Nr=10)"));
    p_c_1 = ExampleVector (cipher, &c_1);
    ++test_count;
    if (p_c_1 == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    STSMSG (3,("C.2 AES-192 (Nk=6, Nr=12)"));
    p_c_2 = ExampleVector (cipher, &c_2);
    ++test_count;
    if (p_c_2 == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    STSMSG (3,("C.3 AES-256 (Nk=8, Nr=14)"));
    p_c_3= ExampleVector (cipher, &c_3);
    ++test_count;
    if (p_c_3 == false)
    {
        STSMSG (1,("TEST FAILED"));
        ++error_count;
    }

    return p_a_1 && p_a_2 && p_a_3 && p_b && p_c_1 && p_c_2 && p_c_3;
}


rc_t run ()
{
    KCipher * cipher;
    rc_t rc = 0, orc = 0;

#if 1
    subtype_iterator = 0;

    STSMSG (1, ("======================================================================"));
    STSMSG (1, ("%d: %s\n", subtype_iterator, types[subtype_iterator]));
    rc = KCipherTestByteMake (&cipher, kcipher_AES);
    if (rc == 0)
    {
        run_one (cipher);
        KCipherRelease (cipher);
    }
    else if (GetRCState(rc) == rcUnsupported)
        STSMSG (1, ("Not supported on this machine"));
    else
        orc = rc;
#endif
#if 1
    subtype_iterator = 1;
    STSMSG (1, ("======================================================================"));
    STSMSG (1, ("%d: %s\n", subtype_iterator, types[subtype_iterator]));
    rc = KCipherTestVecMake (&cipher, kcipher_AES);
    if (rc == 0)
    {
        run_one (cipher);
        KCipherRelease (cipher);
    }
    else if (GetRCState(rc) == rcUnsupported)
        STSMSG (1, ("Not supported on this machine"));
    else
        orc = rc;
#endif
#if 1
    subtype_iterator = 2;
    STSMSG (1, ("======================================================================"));
    STSMSG (1, ("%d: %s\n", subtype_iterator, types[subtype_iterator]));
    rc = KCipherTestVecRegMake (&cipher, kcipher_AES);
    if (rc == 0)
    {
        run_one (cipher);
        KCipherRelease (cipher);
    }
    else if (GetRCState(rc) == rcUnsupported)
        STSMSG (1, ("Not supported on this machine"));
    else
        orc = rc;
#endif
#if 1
    subtype_iterator = 3;
    STSMSG (1, ("======================================================================"));
    STSMSG (1, ("%d: %s\n", subtype_iterator, types[subtype_iterator]));
    rc = KCipherTestVecAesNiMake (&cipher, kcipher_AES);
    if (rc == 0)
    {
        run_one (cipher);
        KCipherRelease (cipher);
    }
    else if (GetRCState(rc) == rcUnsupported)
        STSMSG (1, ("Not supported on this machine"));
    else
        orc = rc;
#endif
    return orc;
}


rc_t CC UsageSummary  (const char * progname)
{
    return 0;
}


const char UsageDefaultName[] = "test-aes_ciphers";
rc_t CC Usage (const Args * args)
{
    return KOutMsg ("\n"
                    "Usage:\n"
                    "  %s [OPTIONS]\n"
                    "\n"
                    "Summary:\n"
                    "  Test all of the AES Ciphers we support.\n"
                    "  use -v one or more times for more details.\n"
                    "  Exit is the number of attempted tests that failed\n",
                    UsageDefaultName);
}


ver_t CC KAppVersion (void)
{
    return 0;
}
rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;
    rc_t rc;


    rc = ArgsMakeAndHandle (&args, argc, argv, 0);
    if (rc == 0)
    {
        STSMSG (3, ("Starting tests",__func__));

        error_count = 0;
        rc = run();
    }

    if (error_count)
        STSMSG (0, ("Failed %d tests of %d run out of 28 possible",error_count, test_count));
    else
        STSMSG (0, ("Passed %d tests out of 28 possible\n", test_count));
    if (rc)
        LOGERR (klogErr, rc, "Exiting with a failure status");
    exit (error_count);
}
