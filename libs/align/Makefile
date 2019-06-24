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
MODULE = libs/align

INT_LIBS = \
	libncbi-bam \
	libalign-writer \
	libalign-access \
	libsam-extract \
	libalign-reader

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =              \
	$(BAM_OBJ)          \
	$(ALIGN_ACCESS_OBJ) \
	$(ALIGN_READER_OBJ) \
	$(ALIGN_WRITER_OBJ)

include $(TOP)/build/Makefile.env

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
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libncbi-bam
#
$(ILIBDIR)/libncbi-bam: $(addprefix $(ILIBDIR)/libncbi-bam.,$(ILIBEXT))

BAM_SRC = \
	bam

BAM_OBJ = \
	$(addsuffix .$(LOBX),$(BAM_SRC))

BAM_LIB =

$(ILIBDIR)/libncbi-bam.$(LIBX): $(BAM_OBJ)
	$(LD) --slib -o $@ $^ $(BAM_LIB)


#-------------------------------------------------------------------------------
# align-access
#
$(ILIBDIR)/libalign-access: $(addprefix $(ILIBDIR)/libalign-access.,$(ILIBEXT))

ALIGN_ACCESS_SRC = \
	align-access

ALIGN_ACCESS_OBJ = \
	$(addsuffix .$(LOBX),$(ALIGN_ACCESS_SRC))

ALIGN_ACCESS_LIB =

$(ILIBDIR)/libalign-access.$(LIBX): $(ALIGN_ACCESS_OBJ)
	$(LD) --slib -o $@ $^ $(ALIGN_ACCESS_LIB)

#-------------------------------------------------------------------------------
# align-reader
#
$(ILIBDIR)/libalign-reader: $(addprefix $(ILIBDIR)/libalign-reader.,$(ILIBEXT))

ALIGN_READER_SRC = \
	align_mgr \
	al_iterator \
	unsupported_pileup_estimator \
	ref_iterator \
	pl_iterator \
	dna-reverse-cmpl \
	reference-cmn \
	reader-cmn \
	reader-refseq \
	reader-wgs \
	reference \
	refseq-mgr \
	quality-quantizer

ALIGN_READER_OBJ = \
	$(addsuffix .$(LOBX),$(ALIGN_READER_SRC))

ALIGN_READER_LIB =

$(ILIBDIR)/libalign-reader.$(LIBX): $(ALIGN_READER_OBJ)
	$(LD) --slib -o $@ $^ $(ALIGN_READER_LIB)

#-------------------------------------------------------------------------------
# align-writer
#
$(ILIBDIR)/libalign-writer: $(addprefix $(ILIBDIR)/libalign-writer.,$(LIBX))

ALIGN_WRITER_SRC = \
    dna-reverse-cmpl \
	reader-cmn \
	reference-cmn \
	reader-refseq \
	reader-wgs \
	refseq-mgr \
	writer-cmn \
	writer-refseq \
	writer-alignment \
	writer-sequence \
	writer-ref \
	writer-reference

ALIGN_WRITER_OBJ = \
	$(addsuffix .$(LOBX),$(ALIGN_WRITER_SRC))

ALIGN_WRITER_LIB =

$(ILIBDIR)/libalign-writer.$(LIBX): $(ALIGN_WRITER_OBJ)
	$(LD) --slib -o $@ $^ $(ALIGN_WRITER_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# samextract
#
$(ILIBDIR)/libsam-extract: $(addprefix $(ILIBDIR)/libsam-extract.,$(LIBX))

SAM_EXTRACT_SRC = \
     samextract-grammar \
     samextract-lex \
     samextract-lib \
     samextract-bam \
     samextract-pool

SAM_EXTRACT_OBJ = \
	$(addsuffix .$(LOBX),$(SAM_EXTRACT_SRC))

SAM_EXTRACT_LIB =

bison:
	bison --no-lines -g -r all -Werror=all \
		-o $(SRCDIR)/samextract-grammar.c \
	--defines=$(SRCDIR)/samextract-tokens.h \
	-v -Wall --no-lines \
	$(SRCDIR)/samextract-grammar.y

ifeq (dbg,$(BUILD))
  FLEX_DEBUG=-d
endif

flex:
	flex --noline --fast -v -p $(FLEX_DEBUG) \
		-o $(SRCDIR)/samextract-lex.c \
		-DYY_BUF_SIZE=16777216 \
		$(SRCDIR)/samextract-lex.l

$(ILIBDIR)/libsam-extract.$(LIBX): $(SAM_EXTRACT_OBJ)
	$(LD) --slib -o $@ $^ $(SAM_EXTRACT_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/