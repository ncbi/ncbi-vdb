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
MODULE = libs/ext/magic

INT_LIBS = \
    magic

EXT_LIBS = \

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

OBJFILES = \
	$(LIBMAGIC_OBJ)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile # $(ALL_LIBS)
	@ $(MAKE_CMD) $(TARGDIR)/$@

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all
#
$(TARGDIR)/all: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(ILIBDIR)/magic

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean
	@ rm -f $(ILIBDIR)/libmagic-static.$(LIBX)

.PHONY: clean

#-------------------------------------------------------------------------------
# libmagic, version 5.04 (ftp://ftp.astron.com/pub/file/file-5.04.tar.gz) configured for 64 bit Linux
#
$(ILIBDIR)/magic: 

ifeq (linux,$(OS))
ifeq (64,$(BITS))

$(ILIBDIR)/magic: $(addprefix $(ILIBDIR)/$(LPFX)magic-static.,$(LIBX))

CFLAGS += -DHAVE_CONFIG_H

LIBMAGIC_SRC = \
	magic \
	apprentice \
	softmagic \
	ascmagic \
	encoding \
	compress \
	is_tar \
	readelf \
	print \
	fsmagic \
	funcs \
	apptype \
	cdf \
	cdf_time \
	readcdf \
    strlcat \
    strlcpy \

LIBMAGIC_OBJ = \
	$(addsuffix .$(LOBX),$(LIBMAGIC_SRC))

LIBMAGIC_LIB = \

$(ILIBDIR)/libmagic-static.$(LIBX): $(LIBMAGIC_OBJ)
	$(LD) --slib -o $@ $^ $(LIBMAGIC_LIB)

endif
endif

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)
