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
MODULE = libs/kfc

INT_LIBS = \
	libkfc

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(KFC_OBJ)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/all

std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/all

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all, std
#
$(TARGDIR)/all: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# kfc
#
$(ILIBDIR)/libkfc: $(addprefix $(ILIBDIR)/libkfc.,$(ILIBEXT))

KFC_SRC = \
	refcount \
	tstate \
	sysrsrc \
	rsrc \
	vtbl \
	sysctx \
	except \
	xc \

KFC_OBJ = \
	$(addsuffix .$(LOBX),$(KFC_SRC))

KFC_LIB = \

$(ILIBDIR)/libkfc.$(LIBX): $(KFC_OBJ)
	$(LD) --slib -o $@ $^ $(KFC_LIB)


compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)


#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
