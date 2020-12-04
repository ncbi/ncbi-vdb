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
MODULE = libs/klib

INT_LIBS = \
	libklib

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(KLIB_OBJ)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all: makedirs compile
	@ $(MAKE) -C $(SRCDIR)/judy all
	@ $(MAKE_CMD) $(TARGDIR)/all

std: makedirs compile
	@ $(MAKE) -C $(SRCDIR)/judy std
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE) -C $(SRCDIR)/judy std
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all, std
#
$(TARGDIR)/all $(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean
	@ $(MAKE) -C $(SRCDIR)/judy clean

.PHONY: clean

#-------------------------------------------------------------------------------
# klib
#
$(ILIBDIR)/libklib: $(addprefix $(ILIBDIR)/libklib.,$(ILIBEXT))

KLIB_SRC = \
	text \
	time \
	utf8 \
	iso8859 \
	iso8859-1 \
	cp1252 \
	token \
	symtab \
	container \
	vector \
	namelist \
	all-vdb-version \
	vector_namelist \
	trie \
	pbstree \
	pbstree-impl.nopt \
	pbstree-impl.swap.nopt \
	bstpersist.nopt \
	ptrie.nopt \
	ptpersist.nopt \
	crc32 \
	md5 \
	misc \
	SHA-32bit \
	SHA-64bit \
	qsort \
	ksort \
	bsearch \
	pack \
	unpack \
	vlen-encode \
	data-buffer \
	refcount \
	printf \
	status-rc-strings \
	status-rc \
	syserrcode \
	syswriter \
	out \
	status \
	log \
	writer \
	syslog \
	systime \
	rc-tbl \
	rc-idx-tbl \
	judy-vector \
	report-klib \
	num-gen \
	progressbar \
	sra-release-version \
	hashtable \
	hashfile \
	btree \
	json \
	json-grammar \
	json-lex \
	base64 \
	guid
#	path

ifeq (win,$(OS))
	KLIB_SRC += sysalloc
endif

ifeq (dbg,$(BUILD))
	KLIB_SRC += debug
endif

KLIB_OBJ = \
	$(addsuffix .$(LOBX),$(KLIB_SRC))

KLIB_LIB = \
	-sjudy \
	-lm

$(ILIBDIR)/libklib.$(LIBX): $(KLIB_OBJ)
	$(LD) --slib -o $@ $^ $(KLIB_LIB)


# compilation
compile: makedirs
	@ $(MAKE) -C $(SRCDIR)/judy compile
	@ $(MAKE_CMD) -j$(MAKE_JOBS) $(TARGDIR)/compile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
FUZZ_OBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .fuzz.$(LOBX),$(KLIB_SRC)))

fuzz: std $(FUZZ_OBJ)
	$(AR) $(FLIBDIR)/libklib.$(LIBX) $(FUZZ_OBJ)

#-------------------------------------------------------------------------------
#
# flex/bison for the Json parser
#
# Will not be executed in a regular build. Use 'make bison' and/or 'make flex' to re-generate C files
#

bison: $(SRCDIR)/json-grammar.y
	bison -Wno-other --no-lines -r state -o $(SRCDIR)/json-grammar.c --defines=$(SRCDIR)/json-tokens.h $(SRCDIR)/json-grammar.y

flex: $(SRCDIR)/json-lex.l $(SRCDIR)/json-tokens.h
	flex --noline -d -t $(SRCDIR)/json-lex.l > $(SRCDIR)/json-lex.c

