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


default: std

TOP ?= $(abspath ../../..)
MODULE = libs/ext/mbedtls

include $(TOP)/build/Makefile.env

INT_LIBS =        \
	libmbedcrypto \
	libmbedx509   \
	libmbedtls

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =        \
	$(CRYPTO_OBJ) \
	$(X509_OBJ)   \
	$(TLS_OBJ)

#-------------------------------------------------------------------------------
# outer targets
#
std all: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/$@

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all
#
$(TARGDIR)/all: $(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libmbedcrypto
#
$(ILIBDIR)/libmbedcrypto: $(addprefix $(ILIBDIR)/libmbedcrypto.,$(LIBX))

CRYPTO_SRC =            \
    aes                 \
    aesni               \
    arc4                \
    asn1parse           \
    asn1write           \
    base64              \
    bignum              \
    blowfish            \
    camellia            \
    ccm                 \
    chacha20            \
    chachapoly          \
    cipher              \
    cipher_wrap         \
    ctr_drbg            \
    des                 \
    dhm                 \
    ecdh                \
    ecdsa               \
    ecjpake             \
    ecp                 \
    ecp_curves          \
    entropy             \
    entropy_poll        \
    error               \
    gcm                 \
    havege              \
    hmac_drbg           \
    md                  \
    md2                 \
    md4                 \
    md5                 \
    md_wrap             \
    memory_buffer_alloc \
    oid                 \
    padlock             \
    pem                 \
    pk                  \
    pk_wrap             \
    pkcs12              \
    pkcs5               \
    pkparse             \
    pkwrite             \
    platform            \
    platform_util       \
    poly1305            \
    ripemd160           \
    rsa                 \
    rsa_internal        \
    sha1                \
    sha256              \
    sha512              \
    threading           \
    timing              \
    version             \
    version_features    \
    xtea

CRYPTO_OBJ = \
	$(addsuffix .$(LOBX),$(CRYPTO_SRC))

$(ILIBDIR)/libmbedcrypto.$(LIBX): $(CRYPTO_OBJ)
	$(LD) --slib -o $@ $^


#-------------------------------------------------------------------------------
# libmbedx509
#
$(ILIBDIR)/libmbedx509: $(addprefix $(ILIBDIR)/libmbedx509.,$(LIBX))

X509_SRC =        \
	certs         \
	pkcs11        \
	x509          \
	x509_create   \
	x509_crl      \
	x509_crt      \
	x509_csr      \
	x509write_crt \
	x509write_csr

X509_OBJ = \
	$(addsuffix .$(LOBX),$(X509_SRC))

$(ILIBDIR)/libmbedx509.$(LIBX): $(X509_OBJ)
	$(LD) --slib -o $@ $^


#-------------------------------------------------------------------------------
# libmbedtls
#
$(ILIBDIR)/libmbedtls: $(addprefix $(ILIBDIR)/libmbedtls.,$(LIBX))

TLS_SRC =            \
	debug            \
	ssl_cache        \
	ssl_ciphersuites \
	ssl_cli          \
	ssl_cookie       \
	ssl_srv          \
	ssl_ticket       \
	ssl_tls

#	net              \

TLS_OBJ = \
	$(addsuffix .$(LOBX),$(TLS_SRC))

$(ILIBDIR)/libmbedtls.$(LIBX): $(TLS_OBJ)
	$(LD) --slib -o $@ $^


compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
