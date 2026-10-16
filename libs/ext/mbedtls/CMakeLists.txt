# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

set(CRYPTO_SRC
    aes.c
    aesni.c
    aria.c
    asn1parse.c
    asn1write.c
    base64.c
    bignum.c
    camellia.c
    ccm.c
    chacha20.c
    chachapoly.c
    cipher.c
    cipher_wrap.c
    cmac.c
    constant_time.c
    ctr_drbg.c
    des.c
    dhm.c
    ecdh.c
    ecdsa.c
    ecjpake.c
    ecp.c
    ecp_curves.c
    entropy.c
    entropy_poll.c
    error.c
    gcm.c
    hmac_drbg.c
    md.c
    md5.c
    nist_kw.c
    oid.c
    padlock.c
    pem.c
    pk.c
    pk_wrap.c
    pkcs12.c
    pkcs5.c
    pkparse.c
    pkwrite.c
    platform.c
    platform_util.c
    poly1305.c
    psa_crypto.c
    psa_crypto_client.c
    psa_crypto_driver_wrappers.c
    psa_crypto_slot_management.c
    psa_crypto_storage.c
    psa_crypto_client.c
    psa_its_file.c
    psa_crypto_rsa.c
    psa_crypto_mac.c
    psa_crypto_ecp.c
    psa_crypto_hash.c
    psa_crypto_cipher.c
    psa_crypto_aead.c
    psa_crypto_se.c
    ripemd160.c
    rsa.c
    rsa_alt_helpers.c
    sha1.c
    sha256.c
    sha512.c
)

set(X509_SRC
    x509_crt.c
    x509.c
)

set(TLS_SRC
    debug.c
    ssl_ciphersuites.c
    ssl_client.c
    ssl_msg.c
    ssl_tls.c
    ssl_tls12_client.c
    ssl_tls12_server.c
    ssl_tls13_keys.c
    ssl_debug_helpers_generated.c
)

GenerateStaticLibs( mbedcrypto "${CRYPTO_SRC}" )
GenerateStaticLibs( mbedx509 "${X509_SRC}" )
GenerateStaticLibs( mbedtls "${TLS_SRC}" )

add_compile_definitions( __mod__="libs/ext/mbedtls" )
