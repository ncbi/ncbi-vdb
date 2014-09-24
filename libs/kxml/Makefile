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
MODULE = libs/kxml

INT_LIBS = \
	libkxml \
	libkxfs

ALL_LIBS = \
	$(INT_LIBS)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# where to find includes
#
DFLT_LIBXML_INCLUDES ?= /usr/include/libxml2

LIBXML_INCLUDES ?= $(DFLT_LIBXML_INCLUDES)

INCDIRS += $(addprefix -I,$(LIBXML_INCLUDES))

#-------------------------------------------------------------------------------
# outer targets
#
ifeq (win,$(OS))
all std:
	@ true
else
all std: makedirs
	@ $(MAKE_CMD) LIBXML_INCLUDES=$(LIBXML_INCLUDES) $(TARGDIR)/std
endif

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) LIBXML_INCLUDES=$(LIBXML_INCLUDES) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# tiny, lightweight interface to libxml
#
$(ILIBDIR)/libkxml: $(addprefix $(ILIBDIR)/libkxml.,$(ILIBEXT))

XML_SRC = \
	xml

XML_OBJ = \
	$(addsuffix .$(LOBX),$(XML_SRC))

XML_LIB = \
	-dkfs \
	-dklib 

ifdef LIBXML_LPATH
XML_LIB += \
	-X$(LIBXML_LPATH) -lxml2
endif

$(ILIBDIR)/libkxml.$(LIBX): $(XML_OBJ)
	$(LD) --slib -o $@ $^ $(XML_LIB)

#-------------------------------------------------------------------------------
# kxfs
#
$(ILIBDIR)/libkxfs: $(addprefix $(ILIBDIR)/libkxfs.,$(ILIBEXT))

KXFS_SRC = \
	xtoc \
	xtoc-parsexml

KXFS_OBJ = \
	$(addsuffix .$(LOBX),$(KXFS_SRC))

KXFS_LIB = \
	-dkxml \
	-dklib \
	-ddl \
	-dz \
	-dbz2 \

$(ILIBDIR)/libkxfs.$(LIBX): $(KXFS_OBJ)
	$(LD) --slib -o $@ $^ $(KXFS_LIB)

