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
MODULE = libs/ext/zlib

include $(TOP)/build/Makefile.env

INT_LIBS = \
	libz

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(LIBZ_OBJ)

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
$(TARGDIR)/all: $(ILIBDIR)/libz

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(ILIBDIR)/libz

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libz
#
$(ILIBDIR)/libz: $(addprefix $(ILIBDIR)/libz.,$(LIBX))

LIBZ_SRC = \
	adler32 \
	compress \
	crc32 \
	deflate \
	gzclose \
	gzlib \
	gzread \
	gzwrite \
	infback \
	inffast \
	inflate \
	inftrees \
	trees \
	uncompr \
	zutil


LIBZ_OBJ = \
	$(addsuffix .$(LOBX),$(LIBZ_SRC))

$(ILIBDIR)/libz.$(LIBX): $(LIBZ_OBJ)
	$(LD) --slib -o $@ $^ $(LIBZ_LIB)


#-------------------------------------------------------------------------------
# libz download
#
ZLIB_VERS := \
	1.2.11

ZLIB_TAR = \
	zlib-$(ZLIB_VERS).tar

ZLIB_TAR_GZ = \
	$(ZLIB_TAR).gz

ifeq (win,$(OS))
ECHO_VERS = echo -ne $(ZLIB_VERS) "\n"
CFLAGS := $(CFLAGS) -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
else
ECHO_VERS = echo $(ZLIB_VERS)
endif

ifeq (sun,$(OS))
	ZLIB_TAR_GET = $(ZLIB_TAR)
	TAR_CMD = tar -xf
else
	ZLIB_TAR_GET = $(ZLIB_TAR_GZ)
	TAR_CMD = tar --no-same-owner -x -z -f
endif

ifeq (mac,$(OS))
RETRIEVE = curl -O
else
RETRIEVE = wget
endif

$(ZLIB_TAR_GZ):
	@ echo downloading the sources
	$(RETRIEVE) http://zlib.net/$@
	@ touch $@

$(ZLIB_TAR): $(ZLIB_TAR_GZ)
	gzcat $^ > $@

libz.vers: $(ZLIB_TAR_GET)
	@ echo unpacking the sources
	$(TAR_CMD) $^
	cp zlib-$(ZLIB_VERS)/*.[ch] .
	cp zlib-$(ZLIB_VERS)/win32/zlib.def libz.def
ifeq (1.2.8,$(ZLIB_VERS))
	patch < zlib-1.2.8.diff
endif
	@ $(ECHO_VERS) > $@

download: libz.vers

.PHONY: download


compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
