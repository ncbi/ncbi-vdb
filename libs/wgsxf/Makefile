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
MODULE = libs/wgsxf

INT_LIBS = \
	libwgsxf \
	libwwgsxf

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =       \
	$(WGSXF_OBJ) \
	$(WWGSXF_OBJ)

include $(TOP)/build/Makefile.env

RWORKDIR = $(BINDIR)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(TARGDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(TARGDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# wgs-vdb transform library
#
$(TARGDIR)/libwgsxf: $(addprefix $(ILIBDIR)/libwgsxf.,$(ILIBEXT))

WGSXF_SRC = \
	build_scaffold \
	build_read_type

WGSXF_OBJ = \
	$(addsuffix .$(LOBX),$(WGSXF_SRC))

WGSXF_LIB = \
	-dvdb \
	-dklib \
	-dm \
	-dz \
	-dbz2

$(ILIBDIR)/libwgsxf.$(LIBX): $(WGSXF_OBJ)
	$(LD) --slib -o $@ $^ $(WGSXF_LIB)

.PHONY: $(TARGDIR)/libwgsxf

#-------------------------------------------------------------------------------
# update-side wgs transform library
#

$(TARGDIR)/libwwgsxf: $(addprefix $(ILIBDIR)/libwwgsxf.,$(ILIBEXT))

WWGSXF_SRC = \
	$(WGSXF_SRC)

WWGSXF_OBJ = \
	$(addsuffix .$(LOBX),$(WWGSXF_SRC))

# temporary
WWGSXF_LIB = \
	-dwvdb \
	-dwkdb \
	-dklib \
	-dm \
	-dz \
	-dbz2

$(ILIBDIR)/libwwgsxf.$(LIBX): $(WWGSXF_OBJ)
	$(LD) --slib -o $@ $^ $(WWGSXF_LIB)

libwwgsxf.vers.h:
	@ true

.PHONY: $(TARGDIR)/libwwgsxf

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)
#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
