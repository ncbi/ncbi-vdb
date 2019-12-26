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


default: runtests

TOP ?= $(abspath ../..)
MODULE = test/krypto

INT_TOOLS = \

ifeq (linux,$(OS))
INT_TOOLS += \
	test-cipher-speed
endif

TEST_TOOLS = \
	test-modes \
	test-encdec \
	test-reenc \
	test-rsa-aes-hmac \
#	test-csprng \ --- not even being compiled in ncbi-vdb, skipping for now

ifeq (linux,$(OS))
TEST_TOOLS += \
	test-aes-ciphers
endif

SLOW_TEST_TOOLS= \
	test-krypto-slow \

EXT_TOOLS =

ALL_TOOLS = \
	$(INT_TOOLS) \
	$(EXT_TOOLS) \
	$(TEST_TOOLS) \
	$(SLOW_TEST_TOOLS) \

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(ALL_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: all std $(ALL_TOOLS)

#-------------------------------------------------------------------------------
# all
#
$(TARGDIR)/all: \
	$(addprefix $(BINDIR)/,$(ALL_TOOLS))

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(TEST_BINDIR)/,$(ALL_TOOLS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# test encryption modes
#
TEST_MODES_SRC = \
	test-modes

TEST_MODES_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_MODES_SRC))

TEST_MODES_LIB = \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/test-modes: $(TEST_MODES_OBJ)
	$(LD) --exe -o $@ $^ $(TEST_MODES_LIB)

#-------------------------------------------------------------------------------
# test AES encryption ciphers
#
TEST_AES_CIPHERS_SRC = \
	test-aes-ciphers

TEST_AES_CIPHERS_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_AES_CIPHERS_SRC))

TEST_AES_CIPHERS_LIB = \
	-skryptotest \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/test-aes-ciphers: $(TEST_AES_CIPHERS_OBJ)
	$(LD) --exe -o $@ $^ $(TEST_AES_CIPHERS_LIB)

#-------------------------------------------------------------------------------
# test cipher speed
# run a null cipher and the aes cipher in all implmentations supported on the
# test machine.
# Comparing specific two versions will test possible speed gains from verious
# optimizations.  Comparing the null cipher (just a copy) versions against each
# other offer insight into whether vector and vector register optimiztions are
# beneficial and by how much
#
TEST_CIPHER_SPEED_SRC = \
	test-cipher-speed

TEST_CIPHER_SPEED_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_CIPHER_SPEED_SRC))

TEST_CIPHER_SPEED_LIB = \
	-skryptotest \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/test-cipher-speed: $(TEST_CIPHER_SPEED_OBJ)
	$(LD) --exe -o $@ $^ $(TEST_CIPHER_SPEED_LIB)

#-------------------------------------------------------------------------------
# test KCSPRng : cryptographically secure pseudo-random number generator
#
TEST_CSPRNG_SRC = \
	test-csprng

TEST_CSPRNG_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_CSPRNG_SRC))

TEST_CSPRNG_LIB = \
	-skapp \
	-sncbi-vdb


$(TEST_BINDIR)/test-csprng: $(TEST_CSPRNG_OBJ)
	$(LD) --exe -o $@ $^ $(TEST_CSPRNG_LIB)

#-------------------------------------------------------------------------------
# test-encdec
#
TEST_ENCDEC_SRC = \
	test-encdec \
	test-cmn

TEST_ENCDEC_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_ENCDEC_SRC))

TEST_ENCDEC_LIB = \
	-skapp \
    -sncbi-vdb \
    -sktst

$(TEST_BINDIR)/test-encdec: $(TEST_ENCDEC_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_ENCDEC_LIB)

#-------------------------------------------------------------------------------
# test-reenc
#
TEST_REENC_SRC = \
	test-reenc \
	test-cmn

TEST_REENC_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_REENC_SRC))

TEST_REENC_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst

$(TEST_BINDIR)/test-reenc: $(TEST_REENC_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_REENC_LIB)

#-------------------------------------------------------------------------------
# test-rsa-aes-hmac
#
TEST_RSA_AES_HMAC_SRC = \
	test-rsa-aes-hmac

TEST_RSA_AES_HMAC_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_RSA_AES_HMAC_SRC))

TEST_RSA_AES_HMAC_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst

$(TEST_BINDIR)/test-rsa-aes-hmac: $(TEST_RSA_AES_HMAC_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_RSA_AES_HMAC_LIB)

#-------------------------------------------------------------------------------
# test-krypto-slow
#
TEST_KRYPTO_SLOW_SRC = \
	test-krypto-slow \
	test-cmn

TEST_KRYPTO_SLOW_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_KRYPTO_SLOW_SRC))

TEST_KRYPTO_SLOW_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst


$(TEST_BINDIR)/test-krypto-slow: $(TEST_KRYPTO_SLOW_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KRYPTO_SLOW_LIB)

