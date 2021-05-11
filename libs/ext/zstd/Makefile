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
MODULE = libs/ext/zstd

include $(TOP)/build/Makefile.env

INT_LIBS = \
	libzstd

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(LIBZSTD_OBJ)

#-------------------------------------------------------------------------------
# outer targets
#
std all: makedirs compile # download
	$(MAKE_CMD) $(TARGDIR)/$@

$(INT_LIBS): makedirs # download
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all
#
$(TARGDIR)/all: $(ILIBDIR)/libzstd

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(ILIBDIR)/libzstd

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libzstd
#
$(ILIBDIR)/libzstd: $(addprefix $(ILIBDIR)/libzstd.,$(LIBX))

VPATH += $(SRCDIR)/common  $(SRCDIR)/compress  $(SRCDIR)/decompress

# When upgrading the library do not forget to replace interfaces/ext/zstd.h !
INCDIRS += $(addprefix -I,$(TOP)/interfaces/ext)

LIBZSTD_SRC = \
	debug \
	error_private \
	pool \
	xxhash \
	entropy_common \
	fse_decompress \
	threading \
	zstd_common \
	fse_compress   \
	zstd_compress   \
	zstd_compress_superblock   \
	zstd_lazy   \
	zstdmt_compress   \
	hist   \
	zstd_compress_literals   \
	zstd_double_fast   \
	zstd_ldm   \
	huf_compress   \
	zstd_compress_sequences   \
	zstd_fast   \
	zstd_opt   \
	huf_decompress   \
	zstd_ddict   \
	zstd_decompress   \
	zstd_decompress_block   \


LIBZSTD_OBJ = \
	$(addsuffix .$(LOBX),$(LIBZSTD_SRC))

$(ILIBDIR)/libzstd.$(LIBX): $(LIBZSTD_OBJ)
	$(LD) --slib -o $@ $^ $(LIBZSTD_LIB)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
