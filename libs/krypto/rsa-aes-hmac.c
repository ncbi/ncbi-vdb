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
#include <krypto/rsa-aes-hmac.h>

#include <klib/rc.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/status.h>
#include <klib/data-buffer.h>

#include <mbedtls/pk.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

#define AES_KEY_BITS 256
#define AES_BLOCK_BYTES 16
#define SHA_256_BITS 256

static
size_t aes_block_trunc ( size_t s )
{
    return s & ~ ( size_t ) ( AES_BLOCK_BYTES - 1 );
}

static
size_t aes_block_round_up ( size_t s )
{
    return aes_block_trunc ( s + AES_BLOCK_BYTES - 1 );
}

typedef struct Rgn Rgn;
struct Rgn
{
    uint8_t * p;
    size_t s;
};

static
Rgn * RgnInit ( Rgn * r, void * p, size_t s )
{
    r -> p = p;
    r -> s = s;
    return r;
}

static
bool RgnEqual ( const Rgn * a, const Rgn * b )
{
    return a -> s == b -> s && memcmp ( a -> p, b -> p, a -> s ) == 0;
}

static
Rgn * RgnCopy ( Rgn * dst, const Rgn * src )
{
    memmove ( dst -> p, src -> p,
        ( dst -> s <= src -> s ) ? dst -> s : src -> s );
    return dst;
}

static
Rgn * RgnFill ( Rgn * self, uint8_t b )
{
    memset ( self -> p, b, self -> s );
    return self;
}

static
Rgn RgnSubRgn ( const Rgn * self, size_t offset )
{
    Rgn sub = * self;
    if ( offset > self -> s )
    {
        sub . p += self -> s;
        sub . s = 0;
    }
    else
    {
        sub . p += offset;
        sub . s -= offset;
    }
    return sub;
}

static
Rgn RgnSubRgn2 ( const Rgn * self, size_t offset, size_t bytes )
{
    Rgn sub = RgnSubRgn ( self, offset );
    if ( bytes < sub . s )
        sub . s = bytes;
    return sub;
}

static
Rgn KDataBufferMakeRgn ( const KDataBuffer * b )
{
    Rgn rgn;
    rgn . p = ( uint8_t * ) b -> base;
    rgn . s = KDataBufferBytes ( b );
    return rgn;
}

static
rc_t handle_mbedtls_return ( const char * func_desc, int ret )
{
    if ( ret != 0 )
    {
        return -1;
    }
    return 0;
}

#define CALL_MBEDTLS( func_name, params ) \
    handle_mbedtls_return ( # func_name, vdb_ ## func_name params )

typedef struct EncryptStuff EncryptStuff;
struct EncryptStuff
{
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context pek;
    mbedtls_md_context_t md;
    mbedtls_aes_context aes;
    uint32_t state;
    unsigned char sk [ AES_KEY_BITS / 8 ];
};

enum
{
    es_nada = 0,

    es_entropy,
    es_ctr_drbg,
    es_pek,
    es_aes_key,
    es_md,
    es_aes,
    
    es_invalid,
    es_last_valid = es_invalid - 1
};

static
void EncryptStuffInit ( EncryptStuff * s )
{
    s -> state = es_nada;
}

static
void EncryptStuffWhack ( EncryptStuff * s )
{
    switch ( s -> state )
    {
    case es_aes:
        vdb_mbedtls_aes_free ( & s -> aes );
    case es_md:
        vdb_mbedtls_md_free ( & s -> md );
    case es_aes_key:
        memset ( s -> sk, 0, sizeof s -> sk );
    case es_pek:
        vdb_mbedtls_pk_free ( & s -> pek );
    case es_ctr_drbg:
        vdb_mbedtls_ctr_drbg_free ( & s -> ctr_drbg );
    case es_entropy:
        vdb_mbedtls_entropy_free ( & s -> entropy );
        break;
    }
}

static
rc_t EncryptStuffAdvance ( EncryptStuff * s, uint32_t state )
{
    rc_t rc = 0;

    STATUS ( STAT_GEEK, "%s - advancing from state %u to %u\n", __func__, s -> state, state );

    if ( state > es_last_valid )
        rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcParam, rcInvalid );
    if ( state != s -> state + 1 )
        rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcParam, rcInvalid );

    switch ( state )
    {
    case es_entropy:
        vdb_mbedtls_entropy_init ( & s -> entropy );
        break;
    case es_ctr_drbg:
        vdb_mbedtls_ctr_drbg_init ( & s -> ctr_drbg );
        break;
    case es_pek:
        vdb_mbedtls_pk_init ( & s -> pek );
        break;
    case es_aes_key:
        rc = CALL_MBEDTLS ( mbedtls_ctr_drbg_random, ( & s -> ctr_drbg, s -> sk, sizeof s -> sk ) );
        break;
    case es_md:
        vdb_mbedtls_md_init ( & s -> md );
        break;
    case es_aes:
        vdb_mbedtls_aes_init ( & s -> aes );
        break;
    }

    if ( rc == 0 )
        s -> state = state;

    return rc;
}

static
rc_t RsaAesHmacEncryptInt ( KDataBuffer * out,
    const void * in, size_t in_bytes, const char * zpek )
{
    /* initialized, empty byte buffer */
    rc_t rc = KDataBufferMakeBytes ( out, 0 );
    if ( rc == 0 )
    {
        EncryptStuff s;
        EncryptStuffInit ( & s );

        do
        {
            uint32_t i;
            size_t esk_size;
            Rgn pt, ob, digest, ct, rgn, rgn2;
            unsigned char iv [ AES_BLOCK_BYTES ];
            unsigned char esk [ MBEDTLS_MPI_MAX_SIZE ];
            const char * pers = "krypty the AES generating bear";

            /* capture the input as "pt" plaintext region */
            STATUS ( STAT_GEEK, "%s - capturing plaintext region\n", __func__ );
            RgnInit ( & pt, ( void * ) in, in_bytes );
            
            /* initialize entropy and prng */
            STATUS ( STAT_PRG, "%s - initializing entropy\n", __func__ );
            rc = EncryptStuffAdvance ( & s, es_entropy );
            if ( rc != 0 )
                break;
            STATUS ( STAT_PRG, "%s - initializing PRNG\n", __func__ );
            rc = EncryptStuffAdvance ( & s, es_ctr_drbg );
            if ( rc != 0 )
                break;

            /* set random seed and entropy function,
               flavor oddly with some constant data... */
            STATUS ( STAT_PRG, "%s - seeding PRNG\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_ctr_drbg_seed, ( & s . ctr_drbg,
                    vdb_mbedtls_entropy_func, & s . entropy,
                    ( const unsigned char * ) pers, strlen ( pers )
                 ) );
            if ( rc != 0 )
                break;

            /* initialize the public encryption key */
            STATUS ( STAT_PRG, "%s - initializing RSA context\n", __func__ );
            rc = EncryptStuffAdvance ( & s, es_pek );
            if ( rc != 0 )
                break;

            /* parse the PEM key */
            STATUS ( STAT_PRG, "%s - parsing PEK\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_pk_parse_public_key, ( & s . pek,
                ( const unsigned char * ) zpek, strlen ( zpek ) + 1 ) );
            if ( rc != 0 )
                break;

            /* fill symmetric AES key with random bits */
            STATUS ( STAT_PRG, "%s - creating random AES key\n", __func__ );
            rc = EncryptStuffAdvance ( & s, es_aes_key );
            if ( rc != 0 )
                break;

            /* encrypt the AES key with RSA key */
            STATUS ( STAT_PRG, "%s - encrypting AES key with RSA key\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_pk_encrypt, ( & s . pek,
                    s . sk, sizeof s . sk,
                    esk, & esk_size, sizeof esk,
                    vdb_mbedtls_ctr_drbg_random, & s . ctr_drbg
                ) );
            if ( rc != 0 )
                break;

            /* at THIS point, we know how large the output buffer has to be.
               we can resize it now and then work within the allocated regions. */
            STATUS ( STAT_PRG, "%s - setting output buffer size\n", __func__ );
            rc = KDataBufferResize ( out,
                    esk_size +                      /* RSA-encrypted AES key */
                    sizeof iv +                     /* initialization vector */
                    aes_block_round_up ( pt . s ) + /* AES ciphertext        */
                   ( SHA_256_BITS / 8 )             /* SHA-256-HMAC          */
                );
            if ( rc != 0 )
                break;
            STATUS ( STAT_GEEK, "%s - size set to %lu bytes:\n", __func__, out -> elem_count );
            STATUS ( STAT_GEEK, "%s -   esk size =  %zu\n", __func__, esk_size );
            STATUS ( STAT_GEEK, "%s -   iv size =  %zu\n", __func__, sizeof iv );
            STATUS ( STAT_GEEK, "%s -   ct size =  %zu\n", __func__, aes_block_round_up ( pt . s ) );
            STATUS ( STAT_GEEK, "%s -   hmac size =  %zu\n", __func__, ( SHA_256_BITS / 8 ) );

            /* capture the output buffer in a region */
            STATUS ( STAT_GEEK, "%s - capturing output buffer as Rgn\n", __func__ );
            ob = KDataBufferMakeRgn ( out );
            STATUS ( STAT_GEEK, "%s - output Rgn ptr = %p\n", __func__, ob . p );
            STATUS ( STAT_GEEK, "%s - output Rgn size = %zu\n", __func__, ob . s );

            /* copy "esk" to output buffer */
            STATUS ( STAT_GEEK, "%s - copying %zu byte esk to output\n", __func__, esk_size );
            RgnCopy ( & ob, RgnInit ( & rgn, esk, esk_size ) );

            /* forget about this portion of the buffer
               operation is safe even for reassigning to self
             */
            STATUS ( STAT_GEEK, "%s - redefining output Rgn to skip over esk\n", __func__ );
            ob = RgnSubRgn ( & ob, esk_size );
            STATUS ( STAT_GEEK, "%s - output Rgn ptr = %p\n", __func__, ob . p );
            STATUS ( STAT_GEEK, "%s - output Rgn size = %zu\n", __func__, ob . s );

            /* capture digest region */
            STATUS ( STAT_GEEK, "%s - capturing digest as Rgn\n", __func__ );
            digest = RgnSubRgn ( & ob, ob . s - SHA_256_BITS / 8 );
            STATUS ( STAT_GEEK, "%s - digest Rgn ptr = %p\n", __func__, digest . p );
            STATUS ( STAT_GEEK, "%s - digest Rgn size = %zu\n", __func__, digest . s );
            STATUS ( STAT_GEEK, "%s - redefining output Rgn to remove digest\n", __func__ );
            ob = RgnSubRgn2 ( & ob, 0, ob . s - digest . s );
            STATUS ( STAT_GEEK, "%s - output Rgn ptr = %p\n", __func__, ob . p );
            STATUS ( STAT_GEEK, "%s - output Rgn size = %zu\n", __func__, ob . s );

            /* fill IV with random bits */
            STATUS ( STAT_PRG, "%s - filling iv buffer with random bits\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_ctr_drbg_random, ( & s . ctr_drbg, iv, sizeof iv ) );
            if ( rc != 0 )
                break;

            /* store padding adjust in lowest nibble
               and yes, we KNOW this will fit because
               each AES block is 16 bytes, meaning the
               adjustment will be 0..15
             */
            STATUS ( STAT_GEEK, "%s - encoding padding bytes into last nibble of iv\n", __func__ );
            assert ( AES_BLOCK_BYTES == 16 );
            assert ( sizeof iv == AES_BLOCK_BYTES );
            iv [ sizeof iv - 1 ] &= 0xF0;
            iv [ sizeof iv - 1 ] |= ( sizeof iv - ( in_bytes & 0xF ) ) & 0xF;

            /* output "iv" to the buffer */
            STATUS ( STAT_GEEK, "%s - copying iv into output Rgn\n", __func__ );
            RgnCopy ( & ob, RgnInit ( & rgn2, iv, sizeof iv ) );

            /* capture ciphertext region */
            STATUS ( STAT_GEEK, "%s - capturing ct Rgn\n", __func__ );
            ct = RgnSubRgn ( & ob, rgn2 . s );
            STATUS ( STAT_GEEK, "%s - ct Rgn ptr = %p\n", __func__, ct . p );
            STATUS ( STAT_GEEK, "%s - ct Rgn size = %zu\n", __func__, ct . s );
            assert ( ( ct . s % AES_BLOCK_BYTES ) == 0 );

            /* initialize message digest for SHA-256 HMAC */
            STATUS ( STAT_PRG, "%s - initializing MD context\n", __func__ );
            rc = EncryptStuffAdvance ( & s, es_md );
            if ( rc != 0 )
                break;
            STATUS ( STAT_PRG, "%s - setting up MD context for SHA-256\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_setup, ( & s . md,
                vdb_mbedtls_md_info_from_type ( MBEDTLS_MD_SHA256 ), true ) );
            if ( rc != 0 )
                break;

            /* temporarily generate a derived HMAC key in "digest"
               initialy, it will have its lower 16 bytes taken from iv,
               and its upper 16 bytes zeroed.

               do this NOW because we're about to destroy "iv"
             */
            STATUS ( STAT_GEEK, "%s - capturing initial HMAC key material from iv\n", __func__ );
            RgnCopy ( RgnFill ( & digest, 0 ), RgnInit ( & rgn2, iv, sizeof iv ) );

            /* initialize the AES context */
            STATUS ( STAT_PRG, "%s - initializing AES context\n", __func__ );
            rc = EncryptStuffAdvance ( & s, es_aes );
            if ( rc != 0 )
                break;

            /* bind the secret key to the context
               this key is still needed for later...
             */
            STATUS ( STAT_PRG, "%s - binding key to AES context\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_aes_setkey_enc, ( & s . aes, s . sk, AES_KEY_BITS ) );
            if ( rc != 0 )
                break;

            /* encrypt as many full blocks as possible */
            STATUS ( STAT_PRG, "%s - AES-encrypting whole blocks of plaintext in CBC mode\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_aes_crypt_cbc,
                    ( & s . aes, MBEDTLS_AES_ENCRYPT,
                    aes_block_trunc ( pt . s ),
                    iv, pt . p, ct . p )
                );
            if ( rc != 0 )
                break;

            /* in the very likely case that the input
               was not a perfect multiple of block size */
            if ( ( pt . s & ( AES_BLOCK_BYTES - 1 ) ) != 0 )
            {
                Rgn odd;
                unsigned char last [ AES_BLOCK_BYTES ];

                /* capture odd tail block of plaintext */
                STATUS ( STAT_GEEK, "%s - capturing last partial block of pt Rgn\n", __func__ );
                odd = RgnSubRgn ( & pt, aes_block_trunc ( pt . s ) );

                /* copy and zero-pad into a full block */
                STATUS ( STAT_GEEK, "%s - padding partial to whole block\n", __func__ );
                RgnCopy ( RgnFill ( RgnInit ( & rgn, last, sizeof last ), 0 ), & odd );

                /* get the last block of ct as destination */
                STATUS ( STAT_GEEK, "%s - capturing last block of ct Rgn\n", __func__ );
                rgn = RgnSubRgn ( & ct, ct . s - AES_BLOCK_BYTES );
                assert ( rgn . s == sizeof last );

                /* encrypt the last block */
                STATUS ( STAT_PRG, "%s - AES-encrypting last partial block of plaintext in CBC mode\n", __func__ );
                rc = CALL_MBEDTLS ( mbedtls_aes_crypt_cbc,
                        ( & s . aes, MBEDTLS_AES_ENCRYPT,
                          rgn . s, iv, last, rgn . p )
                    );
                if ( rc != 0 )
                    break;
            }

            /* now convert digest into an HMAC key derived from iv and sk */
            STATUS ( STAT_GEEK, "%s - deriving HMAC key from iv and AES key\n", __func__ );
            for ( i = 0; i < 8 * 1024; ++ i )
            {
                vdb_mbedtls_md_starts ( & s . md );
                vdb_mbedtls_md_update ( & s . md, digest . p, digest . s );
                vdb_mbedtls_md_update ( & s . md, s . sk, sizeof s . sk );
                vdb_mbedtls_md_finish ( & s . md, digest . p );
            }

            /* at this point, the digest has sensitive information */
            STATUS ( STAT_GEEK, "%s - priming MD for SHA-256-HMAC\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_hmac_starts, ( & s . md,
                digest . p, digest . s ) );
            if ( rc != 0 )
                break;

            /* hash iv + ciphertext */
            STATUS ( STAT_PRG, "%s - hashing iv + ciphertext\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_hmac_update, ( & s . md, ob . p, ob . s ) );
            if ( rc != 0 )
                break;

            /* retrieve the HMAC digest, which is no longer sensitive */
            STATUS ( STAT_GEEK, "%s - capturing HMAC digest into output buffer\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_hmac_finish, ( & s . md, digest . p ) );
            
        } while ( false );

        EncryptStuffWhack ( & s );
    }

    /* ANY sort of an error means whack the output */
    if ( rc != 0 )
        KDataBufferWipeNWhack ( out );
    
    return rc;
}

typedef struct DecryptStuff DecryptStuff;
struct DecryptStuff
{
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context pdk;
    mbedtls_md_context_t md;
    mbedtls_aes_context aes;
    uint32_t state;
    unsigned char sk [ AES_KEY_BITS / 8 ];
    unsigned char digest [ SHA_256_BITS / 8 ];
};

enum
{
    ds_nada = 0,

    ds_entropy,
    ds_ctr_drbg,
    ds_pdk,
    ds_aes_key,
    ds_md,
    ds_aes,
    
    ds_invalid,
    ds_last_valid = ds_invalid - 1
};

static
void DecryptStuffInit ( DecryptStuff * s )
{
    s -> state = ds_nada;
}

static
void DecryptStuffWhack ( DecryptStuff * s )
{
    switch ( s -> state )
    {
    case ds_aes:
        vdb_mbedtls_aes_free ( & s -> aes );
    case ds_md:
        memset ( s -> digest, 0, sizeof s -> digest );
        vdb_mbedtls_md_free ( & s -> md );
    case ds_aes_key:
        memset ( s -> sk, 0, sizeof s -> sk );
    case ds_pdk:
        vdb_mbedtls_pk_free ( & s -> pdk );
    case ds_ctr_drbg:
        vdb_mbedtls_ctr_drbg_free ( & s -> ctr_drbg );
    case ds_entropy:
        vdb_mbedtls_entropy_free ( & s -> entropy );
        break;
    }
}

static
rc_t DecryptStuffAdvance ( DecryptStuff * s, uint32_t state )
{
    rc_t rc = 0;

    STATUS ( STAT_GEEK, "%s - advancing from state %u to %u\n", __func__, s -> state, state );

    if ( state > ds_last_valid )
        rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcParam, rcInvalid );
    if ( state != s -> state + 1 )
        rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcParam, rcInvalid );

    switch ( state )
    {
    case ds_entropy:
        vdb_mbedtls_entropy_init ( & s -> entropy );
        break;
    case ds_ctr_drbg:
        vdb_mbedtls_ctr_drbg_init ( & s -> ctr_drbg );
        break;
    case ds_pdk:
        vdb_mbedtls_pk_init ( & s -> pdk );
        break;
    case ds_aes_key:
        break;
    case ds_md:
        vdb_mbedtls_md_init ( & s -> md );
        break;
    case ds_aes:
        vdb_mbedtls_aes_init ( & s -> aes );
        break;
    }

    if ( rc == 0 )
        s -> state = state;

    return rc;
}

static
rc_t RsaAesHmacDecryptInt ( KDataBuffer * out,
    const void * in, size_t in_bytes,
    const char * zpdk, const char * zpwd )
{
    /* initialized, empty byte buffer */
    rc_t rc = KDataBufferMakeBytes ( out, 0 );
    if ( rc == 0 )
    {
        DecryptStuff s;
        DecryptStuffInit ( & s );

        do
        {
            uint32_t i;
            size_t sk_size;
            Rgn ib, esk, hmac, ct, pt, rgn, rgn2;
            unsigned char iv [ AES_BLOCK_BYTES ];
            const char * pers = "krypty the AES decrypting bear";

            /* capture input as a Rgn, even casting away const... */
            STATUS ( STAT_GEEK, "%s - capturing input as Rgn\n", __func__ );
            RgnInit ( & ib, ( void * ) in, in_bytes );
            
            /* initialize entropy and prng */
            STATUS ( STAT_PRG, "%s - initializing entropy\n", __func__ );
            rc = DecryptStuffAdvance ( & s, ds_entropy );
            if ( rc != 0 )
                break;
            STATUS ( STAT_PRG, "%s - initializing PRNG\n", __func__ );
            rc = DecryptStuffAdvance ( & s, ds_ctr_drbg );
            if ( rc != 0 )
                break;

            /* set random seed and entropy function,
               flavor oddly with some constant data... */
            STATUS ( STAT_PRG, "%s - seeding PRNG\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_ctr_drbg_seed, ( & s . ctr_drbg,
                    vdb_mbedtls_entropy_func, & s . entropy,
                    ( const unsigned char * ) pers, strlen ( pers )
                 ) );
            if ( rc != 0 )
                break;

            /* initialize the private decryption key */
            STATUS ( STAT_PRG, "%s - initializing RSA context\n", __func__ );
            rc = DecryptStuffAdvance ( & s, ds_pdk );
            if ( rc != 0 )
                break;

            /* parse the PEM keyfile */
            STATUS ( STAT_PRG, "%s - parsing PDK from file\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_pk_parse_keyfile, ( & s . pdk, zpdk, zpwd ) );
            if ( rc != 0 )
                break;

            /* capture encrypted symmetric key */
            STATUS ( STAT_GEEK, "%s - capturing esk from input\n", __func__ );
            esk = RgnSubRgn2 ( & ib, 0, vdb_mbedtls_pk_get_len ( & s . pdk ) );
            STATUS ( STAT_GEEK, "%s - esk Rgn ptr = %p\n", __func__, esk . p );
            STATUS ( STAT_GEEK, "%s - esk Rgn size = %zu\n", __func__, esk . s );
            if ( esk . s != vdb_mbedtls_pk_get_len ( & s . pdk ) )
            {
                rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcData, rcInsufficient );
                break;
            }
            
            /* this is a noop - manually initialized next */
            rc = DecryptStuffAdvance ( & s, ds_aes_key );
            if ( rc != 0 )
                break;

            /* decrypt symmetric key */
            STATUS ( STAT_PRG, "%s - decrypting AES key with RSA key\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_pk_decrypt, ( & s .pdk,
                esk . p, esk . s, s . sk, & sk_size, sizeof s . sk,
                vdb_mbedtls_ctr_drbg_random, & s . ctr_drbg ) );
            if ( rc != 0 )
                break;
            STATUS ( STAT_GEEK, "%s - AES key size = %zu\n", __func__, sk_size );
            if ( sk_size != sizeof s . sk )
            {
                rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcData, rcCorrupt );
                break;
            }

            /* skip over encrypted key */
            STATUS ( STAT_GEEK, "%s - redefining input Rgn to skip over esk\n", __func__ );
            ib = RgnSubRgn ( & ib, esk . s );
            STATUS ( STAT_GEEK, "%s - input Rgn ptr = %p\n", __func__, ib . p );
            STATUS ( STAT_GEEK, "%s - input Rgn size = %zu\n", __func__, ib . s );
            if ( ib . s <= ( sizeof iv + ( SHA_256_BITS / 8 ) ) )
            {
                rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcData, rcInsufficient );
                break;
            }

            /* we can now know the size needed for the output buffer */
            STATUS ( STAT_PRG, "%s - setting output buffer size\n", __func__ );
            rc = KDataBufferResize ( out,
                    ib . s -                    /* input buffer size     */
                    sizeof iv -                 /* initialization vector */
                    ( SHA_256_BITS / 8 )        /* SHA-256-HMAC          */
                );
            if ( rc != 0 )
                break;
            STATUS ( STAT_GEEK, "%s - size set to %lu bytes:\n", __func__, out -> elem_count );
            STATUS ( STAT_GEEK, "%s -   remaining size =  %zu\n", __func__, ib . s );
            STATUS ( STAT_GEEK, "%s -   iv size =  %zu\n", __func__, sizeof iv );
            STATUS ( STAT_GEEK, "%s -   hmac size =  %zu\n", __func__, ( SHA_256_BITS / 8 ) );

            /* capture output region */
            STATUS ( STAT_GEEK, "%s - capturing output buffer as Rgn\n", __func__ );
            pt = KDataBufferMakeRgn ( out );
            STATUS ( STAT_GEEK, "%s - output Rgn ptr = %p\n", __func__, pt . p );
            STATUS ( STAT_GEEK, "%s - output Rgn size = %zu\n", __func__, pt . s );

            /* capture input regions */
            STATUS ( STAT_GEEK, "%s - capturing HMAC as Rgn\n", __func__ );
            hmac = RgnSubRgn ( & ib, ib . s - ( SHA_256_BITS / 8 ) );
            STATUS ( STAT_GEEK, "%s - hmac Rgn ptr = %p\n", __func__, hmac . p );
            STATUS ( STAT_GEEK, "%s - hmac Rgn size = %zu\n", __func__, hmac . s );
            STATUS ( STAT_GEEK, "%s - redefining input Rgn to remove HMAC\n", __func__ );
            ib = RgnSubRgn2 ( & ib, 0, ib . s - hmac . s );
            STATUS ( STAT_GEEK, "%s - input Rgn ptr = %p\n", __func__, ib . p );
            STATUS ( STAT_GEEK, "%s - input Rgn size = %zu\n", __func__, ib . s );
            STATUS ( STAT_GEEK, "%s - capturing ct Rgn\n", __func__ );
            ct = RgnSubRgn ( & ib, sizeof iv );
            STATUS ( STAT_GEEK, "%s - ct Rgn ptr = %p\n", __func__, ct . p );
            STATUS ( STAT_GEEK, "%s - ct Rgn size = %zu\n", __func__, ct . s );

            /* copy out the iv */
            STATUS ( STAT_GEEK, "%s - extracting IV\n", __func__ );
            RgnCopy ( RgnInit ( & rgn2, iv, sizeof iv ), & ib );
            if ( rgn2 . s != sizeof iv )
            {
                rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcData, rcInsufficient );
                break;
            }
            
            /* adjust output */
            STATUS ( STAT_PRG, "%s - adjusting output buffer to remove padding\n", __func__ );
            pt . s -= iv [ sizeof iv - 1 ] & 0xF;
            STATUS ( STAT_GEEK, "%s - output Rgn ptr = %p\n", __func__, pt . p );
            STATUS ( STAT_GEEK, "%s - output Rgn size = %zu\n", __func__, pt . s );

            /* initialize message digest */
            STATUS ( STAT_PRG, "%s - initializing MD context\n", __func__ );
            rc = DecryptStuffAdvance ( & s, ds_md );
            if ( rc != 0 )
                break;
            STATUS ( STAT_PRG, "%s - setting up MD context for SHA-256\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_setup, ( & s . md,
                vdb_mbedtls_md_info_from_type ( MBEDTLS_MD_SHA256 ), true ) );
            if ( rc != 0 )
                break;

            /* build an HMAC key */
            STATUS ( STAT_GEEK, "%s - capturing initial HMAC key material from iv\n", __func__ );
            RgnCopy (
                RgnFill ( RgnInit ( & rgn, s . digest, sizeof s . digest ), 0 ),
                RgnInit ( & rgn2, iv, sizeof iv )
                );

            /* now convert digest into an HMAC key derived from iv and sk */
            STATUS ( STAT_GEEK, "%s - deriving HMAC key from iv and AES key\n", __func__ );
            for ( i = 0; i < 8 * 1024; ++ i )
            {
                vdb_mbedtls_md_starts ( & s . md );
                vdb_mbedtls_md_update ( & s . md, s . digest, sizeof s . digest );
                vdb_mbedtls_md_update ( & s . md, s . sk, sizeof s . sk );
                vdb_mbedtls_md_finish ( & s . md, s . digest );
            }

            /* at this point, the digest has sensitive information */
            STATUS ( STAT_GEEK, "%s - priming MD for SHA-256-HMAC\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_hmac_starts, ( & s . md,
                s . digest, sizeof s . digest ) );
            if ( rc != 0 )
                break;

            /* hash iv + ciphertext */
            STATUS ( STAT_PRG, "%s - hashing iv + ciphertext\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_hmac_update, ( & s . md, ib . p, ib . s ) );
            if ( rc != 0 )
                break;

            /* retrieve the HMAC digest, which is no longer sensitive */
            STATUS ( STAT_GEEK, "%s - retrieving HMAC digest\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_md_hmac_finish, ( & s . md, s . digest ) );

            /* the digest must compare */
            STATUS ( STAT_PRG, "%s - comparing stored and calculated HMAC\n", __func__ );
            if ( ! RgnEqual ( & hmac, RgnInit ( & rgn, s . digest, sizeof s . digest ) ) )
            {
                STATUS ( STAT_QA, "%s - HMAC comparison failed\n", __func__ );
                rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcData, rcCorrupt );
                break;
            }

            /* just about there */
            STATUS ( STAT_PRG, "%s - initializing AES context\n", __func__ );
            rc = DecryptStuffAdvance ( & s, ds_aes );
            if ( rc != 0 )
                break;
            STATUS ( STAT_PRG, "%s - binding key to AES context\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_aes_setkey_dec, ( & s . aes, s . sk, AES_KEY_BITS ) );
            if ( rc != 0 )
                break;

            /* all buffers are multiples of whole blocks - do it in one pass */
            STATUS ( STAT_PRG, "%s - AES-decrypting whole blocks of plaintext in CBC mode\n", __func__ );
            rc = CALL_MBEDTLS ( mbedtls_aes_crypt_cbc, ( & s . aes, MBEDTLS_AES_DECRYPT,
                ct . s, iv, ct . p, pt . p ) );
            if ( rc != 0 )
                break;

            /* now remove the padding */
            STATUS ( STAT_GEEK, "%s - readjusting output buffer size to final\n", __func__ );
            STATUS ( STAT_GEEK, "%s -   prior size = %lu\n", __func__, out -> elem_count );
            STATUS ( STAT_GEEK, "%s -   requested size = %zu\n", __func__, pt . s );
            rc = KDataBufferWipeResize ( out, pt . s );
            STATUS ( STAT_GEEK, "%s -   after size = %lu\n", __func__, out -> elem_count );
        }
        while ( false );

        DecryptStuffWhack ( & s );
    }

    /* ANY sort of an error means whack the output */
    if ( rc != 0 )
        KDataBufferWipeNWhack ( out );
    
    return rc;
}

/* RsaAesHmacEncrypt
 *  encrypt a block of data using a random AES key, which is itself
 *  encrypted using an RSA key, and generate a SHA-256 HMAC into
 *  the provided binary output buffer.
 *
 *  the output will be in pure binary form, lacking a header and
 *  any text-encoding such as base64 that might be needed for
 *  transmission.
 *
 *  "out" [ IN/OUT ] - pointer to a ZEROED KDataBuffer
 *  that will receive encrypted result. must be zeroed with
 *  "memset ( & out, 0, sizeof out );" or equivalent.
 *
 *  "in" [ IN ] and "in_bytes" [ IN ] - block of data to be encrypted
 *
 *  "zpek" [ IN ] - NUL-terminated RSA public encryption key
 *  in PEM format. MUST be NUL-terminated.
 */
LIB_EXPORT rc_t CC RsaAesHmacEncrypt ( KDataBuffer * out,
    const void * in, size_t in_bytes, const char * zpek )
{
    rc_t rc;

    /* check output parameter first */
    if ( out == NULL )
        rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcBuffer, rcNull );
    else
    {
        /* check input parameters */
        if ( in_bytes == 0 )
            rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcBlob, rcNull );
        else if ( in == NULL )
            rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcBlob, rcEmpty );
        else if ( zpek == NULL )
            rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcEncryptionKey, rcNull );
        else if ( zpek [ 0 ] == 0 )
            rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcEncryptionKey, rcEmpty );
        else
        {
            /* check for ZEROED output buffer */
            size_t i, sum;
            for ( i = sum = 0; i < sizeof * out; ++ i )
                sum += ( ( const unsigned char * ) out ) [ i ];
            if ( sum != 0 )
                rc = RC ( rcKrypto, rcBlob, rcEncrypting, rcBuffer, rcInvalid );
            else
            {
                /* perform operation */
                rc = RsaAesHmacEncryptInt ( out, in, in_bytes, zpek );
            }
        }
    }

    return rc;
}


/* RsaAesHmacDecrypt
 *  decrypt a block of enciphered data produced by RsaAesHmacEncrypt().
 *
 *  "out" [ IN/OUT ] - pointer to a ZEROED KDataBuffer
 *  that will receive decrypted result. must be zeroed with
 *  "memset ( & out, 0, sizeof out );" or equivalent.
 *
 *  "in" [ IN ] and "in_bytes" [ IN ] - block of enciphered data to be decrypted
 *
 *  "zpdk" [ IN ] - a NUL-terminated FILE PATH to an
 *  RSA private decryption key in PEM format.
 *
 *  "zpwd" [ IN/OUT ] and "zpwd_size" [ IN ] - a mutable buffer containing
 *  a NUL-terminated password string used to decrypt the file designated by
 *  "zpdk". "zpwd_size" should be the total size of the BUFFER, not the
 *  size of the password itself. The buffer will be wiped after first use
 *  regardless of success or failure.
 */
LIB_EXPORT rc_t CC RsaAesHmacDecrypt ( KDataBuffer * out,
    const void * in, size_t in_bytes, const char * zpdk,
    char * zpwd, size_t zpw_size )
{
    rc_t rc;

    /* check output parameter first */
    if ( out == NULL )
        rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcBuffer, rcNull );
    else
    {
        /* check input parameters */
        if ( in_bytes == 0 )
            rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcBlob, rcNull );
        else if ( in == NULL )
            rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcBlob, rcEmpty );
        else if ( zpdk == NULL )
            rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcPath, rcNull );
        else if ( zpdk [ 0 ] == 0 )
            rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcPath, rcEmpty );
        else if ( zpwd == NULL )
            rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcParam, rcNull );
        else if ( zpw_size == 0 || zpwd [ 0 ] == 0 )
            rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcParam, rcEmpty );
        else
        {
            /* check for ZEROED output buffer */
            size_t i, sum;
            for ( i = sum = 0; i < sizeof * out; ++ i )
                sum += ( ( const unsigned char * ) out ) [ i ];
            if ( sum != 0 )
                rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcBuffer, rcInvalid );
            else
            {
                /* ensure the password is NULL-terminated within bounds */
                for ( i = 0; i < zpw_size; ++ i )
                {
                    if ( zpwd [ i ] == 0 )
                        break;
                }
                if ( i == zpw_size )
                    rc = RC ( rcKrypto, rcBlob, rcDecrypting, rcParam, rcInvalid );
                else
                {
                    /* perform operation */
                    rc = RsaAesHmacDecryptInt ( out, in, in_bytes, zpdk, zpwd );
                }
            }
        }
    }

    /* always wipe password */
    if ( zpwd != NULL && zpw_size != 0 )
    {
        /* compiler should not be in a position to optimize this away... */
        STATUS ( STAT_QA, "%s - wiping password buffer\n", __func__ );
        memset ( zpwd, ' ', zpw_size );
        zpwd [ 0 ] = 0;
    }

    return rc;
}

/* EncryptForNCBI
 *  encrypts a block using a public RSA encryption key belonging to NCBI
 */
static const char NCBI_PEK [] =
    "-----BEGIN PUBLIC KEY-----\r\n"
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArkP4ghANgXpW7UDpw+rI\r\n"
    "ttNTJV447GsCom6MDr4CxWbS1MEPwPrvJIao/otKjMJlZv+opEDNfuu+mW5Of/iM\r\n"
    "2gVDrgMKG8OP34HqJ9dbOfZtsaM3pJNN60TTPJbWZ5Wqd26RDj18gVY4NWj6mVGs\r\n"
    "Z8aY4bXwg4LJVHF/6E+bMEIZcvw6wVro1AcT4mke1eqySDRYD7MqBx/GIcrI/921\r\n"
    "siJDAfXQLW25CqVBFF2jqY2+sLcKX4ZOY0RdJ9sNNa/c2oZYuacDp9kett+UtOmH\r\n"
    "XV99VcqeX/nIBUlAn0yAfsNnRw7NSBv1ydZxRdayLvhm5px+DQ98yRP1MrZEgmdB\r\n"
    "SwIDAQAB\r\n"
    "-----END PUBLIC KEY-----\r\n";

LIB_EXPORT rc_t CC EncryptForNCBI ( struct KDataBuffer * out,
    const void * in, size_t in_bytes )
{
    return RsaAesHmacEncrypt ( out, in, in_bytes, NCBI_PEK );
}
