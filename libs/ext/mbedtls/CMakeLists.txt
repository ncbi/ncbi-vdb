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
    aes
    aesni
    arc4
    aria
    asn1parse
    asn1write
    base64
    bignum
    blowfish
    camellia
    ccm
    chacha20
    chachapoly
    cipher
    cipher_wrap
    cmac
    ctr_drbg
    des
    dhm
    ecdh
    ecdsa
    ecjpake
    ecp
    ecp_curves
    entropy
    entropy_poll
    error
    gcm
    havege
    hkdf
    hmac_drbg
    md
    md2
    md4
    md5
    memory_buffer_alloc
    nist_kw
    oid
    padlock
    pem
    pk
    pk_wrap
    pkcs12
    pkcs5
    pkparse
    pkwrite
    platform
    platform_util
    poly1305
    psa_crypto
    psa_crypto_driver_wrappers
    psa_crypto_se
    psa_crypto_slot_management
    psa_crypto_storage
    psa_its_file
    ripemd160
    rsa
    rsa_internal
    sha1
    sha256
    sha512
    threading
    timing
    version
    version_features
    xtea
)

set(X509_SRC
    certs
    pkcs11
    x509
    x509_create
    x509_crl
    x509_crt
    x509_csr
    x509write_crt
    x509write_csr
)

set(TLS_SRC
    debug
    net_sockets
    ssl_cache
    ssl_ciphersuites
    ssl_cli
    ssl_cookie
    ssl_msg
    ssl_srv
    ssl_ticket
    ssl_tls
    ssl_tls13_keys
)

GenerateStaticLibs( mbedcrypto "${CRYPTO_SRC}" )
GenerateStaticLibs( mbedx509 "${X509_SRC}" )
GenerateStaticLibs( mbedtls "${TLS_SRC}" )

add_compile_definitions( __mod__="libs/ext/mbedtls" )
