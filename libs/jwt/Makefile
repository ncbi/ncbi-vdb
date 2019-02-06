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

TOP ?= $(abspath ../..)
MODULE = libs/jwt

INT_LIBS = \
	libjwt \
	libcppex

ALL_LIBS = \
	$(INT_LIBS)

include $(TOP)/build/Makefile.env

clean: stdclean

OPT += -fno-rtti

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# jwt
#
$(ILIBDIR)/libjwt: \
	$(addprefix $(ILIBDIR)/libjwt.,$(ILIBEXT))

JWT_SRC =            \
	jwt-string         \
	jwt-factory        \
	jwt-claims         \
	jws-factory        \
	jwk-hmac           \
	jwk-rsa            \
	jwk-ec             \
	jwk-parse          \
	jwk-cmn            \
	jwa-none           \
	jwa-hmac           \
	jwa-rsa            \
	jwa-factory        \
	jwt-lock           \
	jwt-exception      \
	base64             \
	parser             \
	json-object        \
	json-array         \
	json-wrapper       \
	json-primitive     \
	json-value         \
	json-exception     \
	memset_s           \
	jwt-c


JWT_OBJ = \
	$(addsuffix .$(LOBX),$(JWT_SRC))

$(ILIBDIR)/libjwt.$(ILIBEXT): $(JWT_OBJ)
	$(LD) --slib -o $@ $^

#-------------------------------------------------------------------------------
# c++ exception handling support, copied from libstdc++-v3 (git-svn-id: svn+ssh://gcc.gnu.org/svn/gcc/trunk@268424 138bc75d-0d04-0410-961f-82ee72b054a4)
# (changed extensions from .cc to .cpp)
#
$(ILIBDIR)/libcppex: \
	$(addprefix $(ILIBDIR)/libcppex.,$(ILIBEXT))

DEFINES += -DIN_GLIBCPP_V3

CPPEX_SRC =         \
	class_type_info \
	pure            \
	eh_alloc        \
	eh_throw        \
	eh_catch        \
	tinfo           \
	eh_terminate    \
	eh_call         \
	eh_exception    \
	eh_globals      \
	eh_term_handler \
	eh_unex_handler \
	vterminate      \
	eh_personality      \
	si_class_type_info  \
	eh_type             \
	cp-demangle

CPPEX_OBJ = \
	$(addsuffix .$(LOBX),$(CPPEX_SRC))

$(ILIBDIR)/libcppex.$(ILIBEXT): $(CPPEX_OBJ)
	$(LD) --slib -o $@ $^

