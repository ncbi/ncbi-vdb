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
MODULE = libs/simple

INT_LIBS = \

EXT_LIBS = \
	libsra-simple

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

TEST_TOOLS = \
	testy-the-bear

include $(TOP)/build/Makefile.env
include $(SRCDIR)/Makefile.$(COMP)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: all std $(ALL_LIBS) $(TEST_TOOLS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# simple
#
$(LIBDIR)/libsra-simple: $(addprefix $(LIBDIR)/libsra-simple.,$(LIBEXT))

SIMPLE_SRC = \
	vdb-csra \
	vdb-database \
	vdb-table \
	vdb-sra \
	vdb-spot \
	vdb-spot-read \
	vdb-mgr \
	vdb-mgr-read \
	sra-object-make \
	sra-object \
	sra-spot \
	sra-sequence \
	sra-exception \
	sra-memmgr \
	sra-mem \
	sra-string \
	SRAObject \
	SRAAlignment \
	SRAReference \
	SRASpot \
	SRASequence \
	SRAException \

SIMPLE_OBJ = \
	$(addsuffix .$(LOBX),$(SIMPLE_SRC))

SIMPLE_LIB = \
	-dkfs \
	-dklib

$(LIBDIR)/libsra-simple.$(SHLX): $(SIMPLE_OBJ)
	$(LD) --dlib --vers $(SRCDIR) -o $@ $^ $(SIMPLE_LIB)

$(LIBDIR)/libsra-simple.$(LIBX): $(SIMPLE_OBJ)
	$(LD) --slib --vers $(SRCDIR) -o $@ $^ $(SIMPLE_LIB)

#-------------------------------------------------------------------------------
# testy-the-bear
#
TESTY_SRC = \
	testy-the-bear

TESTY_OBJ = \
	$(addsuffix .$(OBJX),$(TESTY_SRC))

TESTY_LIB = \
	-ssra-simple \
	-salign-reader \
	$(READONLY_SCHEMA_LIBS) \
	-svdb \
	-skdb \
	-svfs \
	-skns \
	-skfg \
	-skfs \
	-skrypto \
	-sksrch \
	-skproc \
	-sklib \
	-lpthread \
	-lm

$(TEST_BINDIR)/testy-the-bear: $(TESTY_OBJ)
	$(LP) --exe --vers $(SRCDIR) -o $@ $^ $(TESTY_LIB)
