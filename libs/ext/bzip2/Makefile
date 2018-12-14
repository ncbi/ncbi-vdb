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
MODULE = libs/ext/bzip2

include $(TOP)/build/Makefile.env

INT_LIBS = \
	libbz2

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(LIBBZ2_OBJ)

#-------------------------------------------------------------------------------
# outer targets
#
std all: makedirs compile # download
	@ $(MAKE_CMD) $(TARGDIR)/$@

$(INT_LIBS): makedirs # download
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all
#
$(TARGDIR)/all: $(ILIBDIR)/libbz2

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(ILIBDIR)/libbz2

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean
	@ rm -f libbz2.vers

.PHONY: clean

#-------------------------------------------------------------------------------
# libbz2
#
$(ILIBDIR)/libbz2: $(addprefix $(ILIBDIR)/libbz2.,$(LIBX))

LIBBZ2_SRC = \
	blocksort \
	huffman \
	crctable \
	randtable \
	compress \
	decompress \
	bzlib

LIBBZ2_OBJ = \
	$(addsuffix .$(LOBX),$(LIBBZ2_SRC))

$(ILIBDIR)/libbz2.$(LIBX): $(LIBBZ2_OBJ)
	$(LD) --slib -o $@ $^ $(LIBBZ2_LIB)


#-------------------------------------------------------------------------------
# libbz2 download
#
BZIP2_VERS := \
	1.0.6

BZIP2_TAR = \
	bzip2-$(BZIP2_VERS).tar

BZIP2_TAR_GZ = \
	$(BZIP2_TAR).gz

ifeq (win,$(OS))
ECHO_VERS = echo -ne $(BZIP2_VERS) "\n"
CFLAGS := $(CFLAGS) -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
else
ECHO_VERS = echo $(BZIP2_VERS)
endif

ifeq (sun,$(OS))
	BZIP2_TAR_GET = $(BZIP2_TAR)
	TAR_CMD = tar -xf
else
	BZIP2_TAR_GET = $(BZIP2_TAR_GZ)
	TAR_CMD = tar --no-same-owner -x -z -f
endif

ifeq (mac,$(OS))
RETRIEVE = curl -O
else
RETRIEVE = wget
endif

$(BZIP2_TAR_GZ):
	@ echo downloading the sources
	$(RETRIEVE) http://bzip.org/$(BZIP2_VERS)/bzip2-$(BZIP2_VERS).tar.gz
	@ touch $@

$(BZIP2_TAR): $(BZIP2_TAR_GZ)
	gzcat $^ > $@

libbz2.vers: $(BZIP2_TAR_GET)
	@ echo unpacking the sources
	$(TAR_CMD) $^
	cp bzip2-$(BZIP2_VERS)/*.[ch] .
	cp bzip2-$(BZIP2_VERS)/*.def .
	#-rm -rf bzip2-$(BZIP2_VERS)
ifeq (1.0.6,$(BZIP2_VERS))
	patch < bzip2-1.0.6.diff
endif
	@ $(ECHO_VERS) > $@

download: libbz2.vers

.PHONY: download

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
