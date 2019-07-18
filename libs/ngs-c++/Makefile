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

TOP ?= $(abspath ../..)
MODULE = libs/ngs-c++

INT_LIBS = \

EXT_LIBS = \
	libncbi-ngs-c++

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

OBJFILES = \
	$(NGS_OBJ)

include $(TOP)/build/Makefile.env

INCDIRS += \
	-I$(SRCDIR)/../ngs \
	-I$(TOP)/../ngs/ngs-sdk


#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs # compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

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
# ncbi-ngs-c++
#
$(LIBDIR)/libncbi-ngs-c++: $(addprefix $(LIBDIR)/libncbi-ngs-c++.,$(LIBX))

NGS_SRC = \
	NCBI-NGS

NGS_OBJ = \
	$(addsuffix .$(LOBX),$(NGS_SRC))

NGS_LIB = \

$(LIBDIR)/libncbi-ngs-c++.$(LIBX): $(NGS_OBJ)
	$(LP) --slib --vers $(SRCDIR)/../ncbi-vdb/libncbi-vdb.vers -o $@ $^ $(NGS_LIB)


testy-the-bear: $(TEST_BINDIR)/testy-the-ngs-bear

$(TEST_BINDIR)/testy-the-ngs-bear: testy-the-bear.o
	$(LP) --exe -o $@ $^ -lkapp -lncbi-ngs-c++ -lncbi-vdb

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)
