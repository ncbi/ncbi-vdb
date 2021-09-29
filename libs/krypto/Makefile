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

default: all

USE_GCC_IF_ICC = 1

TOP ?= $(abspath ../..)
MODULE = libs/krypto

INT_LIBS = \
	libkrypto

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(KRYPTO_OBJ)

include $(TOP)/build/Makefile.env

ifeq ($(OS),linux)
	INT_LIBS += libkryptotest
	OBJFILES += $(KRYPTOTEST_OBJ)
endif


#-------------------------------------------------------------------------------
# outer targets
#
# This differs from other sister Makefiles in needing three other symbols defined
# VEC_SUPPORTED, VECREG_SUPPORTED and AESNI_SUPPORTED
# The successive symbols are additive
# In order they are the compiler supports  the vector 'container' type,
# the processor has supported vector registers
# the processor supports the Intel AES-NI instruction set
#
# The prerequisite call to make compiler-test will have tried to compile objects
# that show such support if the objec can be made
all std: makedirs compiler-test compile
	@ $(MAKE_CMD) \
		VEC_SUPPORTED=$(shell if test -f $(OBJDIR)/compile-test.vec.$(LOBX) ; then echo "true"; else echo "false" ; fi) \
		VECREG_SUPPORTED=$(shell if test -f $(OBJDIR)/compile-test.vecreg.$(LOBX) ; then echo "true"; else echo "false" ; fi) \
		AESNI_SUPPORTED=$(shell if test -f $(OBJDIR)/compile-test.aes-ni.$(LOBX) ; then echo "true"; else echo "false" ; fi) \
		$(TARGDIR)/std || \
	( echo "reattempting without AES-NI instructions"; \
	  $(MAKE_CMD) \
		VEC_SUPPORTED=$(shell if test -f $(OBJDIR)/compile-test.vec.$(LOBX) ; then echo "true"; else echo "false" ; fi) \
		VECREG_SUPPORTED=$(shell if test -f $(OBJDIR)/compile-test.vecreg.$(LOBX) ; then echo "true"; else echo "false" ; fi) \
		AESNI_SUPPORTED=false \
		$(TARGDIR)/std ) || \
	( echo "reattempting without vector register instructions"; \
	  $(MAKE_CMD) \
		VEC_SUPPORTED=$(shell if test -f $(OBJDIR)/compile-test.vec.$(LOBX) ; then echo "true"; else echo "false" ; fi) \
		VECREG_SUPPORTED=false \
		AESNI_SUPPORTED=false \
		$(TARGDIR)/std ) || \
	( echo "reattempting without vector instructions"; \
	  $(MAKE_CMD) \
		VEC_SUPPORTED=false \
		VECREG_SUPPORTED=false \
		AESNI_SUPPORTED=false \
		$(TARGDIR)/std )

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))
#	@echo VEC_SUPPORTED $(VEC_SUPPORTED)
#	@echo VECREG_SUPPORTED $(VECREG_SUPPORTED)
#	@echo AESNI_SUPPORTED $(AESNI_SUPPORTED)

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# compiler-test
#
.PHONY: compiler-test $(TARGDIR)/compiler-test

# Silently try to compile three objects
compiler-test : makedirs
	@ $(MAKE_CMD) -i $(TARGDIR)/compiler-test >/dev/null 2>/dev/null


COMPILE_TEST_SRC = \
	compile-test.vec \
	compile-test.vecreg \
	compile-test.aes-ni

COMPILE_TEST_OBJ = \
	$(addsuffix .$(LOBX),$(COMPILE_TEST_SRC))

$(TARGDIR)/compiler-test : $(COMPILE_TEST_OBJ)
	@echo VEC_SUPPORTED $(VEC_SUPPORTED)
	@echo VECREG_SUPPORTED $(VECREG_SUPPORTED)
	@echo AESNI_SUPPORTED $(AESNI_SUPPORTED)

#-------------------------------------------------------------------------------
# define some additional compiler flags for special object file types
# currently only gcc is supported and it turns out only with 4.4.0 and up
# though other compilers could also be supported
ifeq ($(COMP),gcc)
CC_LISTING = -Wa,-ahlms=$(<D)/$(@F).list
_CC_AES_NI  = -funsafe-math-optimizations -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -maes -Wa,-march=generic64+aes $(CC_LISTING)
_CC_VECREG  = -funsafe-math-optimizations -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -Wa,-march=generic64+sse4 $(CC_LISTING)
_CC_VEC     = $(CC_LISTING)
else
_CC_LISTING =
_CC_AES_NI  =
_CC_VECREG  =
_CC_VEC     =
endif

ifeq ($(VEC_SUPPORTED),true)
CC_VEC = $(_CC_VEC)
else
CC_VEC =
endif

ifeq ($(VECREG_SUPPORTED),true)
CC_VECREG = $(_CC_VECREG)
else
CC_VECREG =
endif

ifeq ($(AESNI_SUPPORTED),true)
CC_AES_NI = $(_CC_AES_NI)
else
CC_AES_NI =
endif

# Rules for making objects using compiler vectors
compile-test.vec.pic.o : compile-test.c
	@ echo +++++ Making $@ using its own rule
	@ echo $@ $(_CC_VEC)
	$(CC) -o $@ -fPIC $(OPT) -D_LIBRARY -DUSEVEC $(_CC_VEC) $<


%.vec.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) -D_LIBRARY -DUSEVEC $(CC_VEC) $<


# Rules for making objects using vectors in registers
compile-test.vecreg.pic.o : compile-test.c
	@ echo +++++ Making $@ using its own rule
	@ echo $@ $(_CC_VECREG)
	$(CC) -o $@ -fPIC $(OPT) -D_LIBRARY -DUSEVEC -DUSEVECREG $(_CC_VECREG) $<

%.vecreg.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) -D_LIBRARY -DUSEVEC -DUSEVECREG $(CC_VECREG) $<


# Rules for making objects using AES-NI extensions vectors in registers
compile-test.aes-ni.pic.o : compile-test.c
	@ echo +++++ Making $@ using its own rule
	@ echo $@ $(_CC_AES_NI)
	$(CC) -o $@ -fPIC $(OPT) -D_LIBRARY -DUSEVEC -DUSEVECREG -DUSEAESNI $(_CC_AES_NI) $<

%.aes-ni.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) -D_LIBRARY -DUSEVEC -DUSEVECREG -DUSEAESNI $(CC_AES_NI) $<



#-------------------------------------------------------------------------------
# krypto
#
$(ILIBDIR)/libkrypto: $(addprefix $(ILIBDIR)/libkrypto.,$(ILIBEXT))

ALL_LIB_SRC = \
	ciphermgr \
	wgaencrypt \
	encfile \
	encfilev2 \
	cipher \
	aes_core \
	aes-byte \
	cipher-byte \
	key \
	reencfile \
	rsa-aes-hmac

LIB_SRC = \
	no-null-ncbi

TEST_LIB_SRC =  \
	testcipher

ifeq ($(VEC_SUPPORTED),true)
ALL_LIB_SRC += \
	cipher-vec.vec \
	aes-ncbi.vec
LIB_SRC += \
	no-null-ncbi.vec
TEST_LIB_SRC +=
else
ALL_LIB_SRC += \
	cipher-no-vec.vec
LIB_SRC += \
	no-null-ncbi.vec
TEST_LIB_SRC +=
endif

ifeq ($(VECREG_SUPPORTED),true)
ALL_LIB_SRC += \
	cipher-vec.vecreg \
	aes-ncbi.vecreg
LIB_SRC += \
	no-null-ncbi.vecreg
TEST_LIB_SRC +=
else
ALL_LIB_SRC += \
	cipher-no-vec.vecreg
LIB_SRC += \
	no-null-ncbi.vecreg
TEST_LIB_SRC +=
endif

ifeq ($(AESNI_SUPPORTED),true)
ALL_LIB_SRC += \
	cipher-vec.aes-ni \
	aes-ncbi.aes-ni
LIB_SRC += \
	no-null-ncbi.aes-ni
TEST_LIB_SRC +=
else
ALL_LIB_SRC += \
	cipher-no-vec.aes-ni
LIB_SRC += \
	no-null-ncbi.aes-ni
TEST_LIB_SRC +=
endif


KRYPTO_SRC = $(ALL_LIB_SRC) $(LIB_SRC)

KRYPTO_OBJ = \
	$(addsuffix .$(LOBX),$(KRYPTO_SRC))

KRYPTO_LIB = \

$(ILIBDIR)/libkrypto.$(LIBX): $(KRYPTO_OBJ)
	$(LD) --slib -o $@ $^ $(KRYPTO_LIB)


#-------------------------------------------------------------------------------
# kryptotest
#

$(ILIBDIR)/libkryptotest: $(addprefix $(ILIBDIR)/libkryptotest.,$(ILIBEXT))

KRYPTOTEST_SRC = $(TEST_LIB_SRC)

KRYPTOTEST_OBJ = \
	$(addsuffix .$(LOBX),$(KRYPTOTEST_SRC))

KRYPTOTEST_LIB = \

$(ILIBDIR)/libkryptotest.$(LIBX): $(KRYPTOTEST_OBJ)
	$(LD) --slib -o $@ $^ $(KRYPTOTEST_LIB)


compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
