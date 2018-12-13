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
MODULE = libs/kfg

INT_LIBS = \
	libkfg

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(KFG_OBJ)

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
#INTERM_SRC = \
	$(SRCDIR)/config-lex.c \
	$(SRCDIR)/config-grammar.c \
	$(SRCDIR)/config-tokens.h

clean: stdclean
	@ rm -f $(addsuffix /default.kfg,$(NCBIDIR))

.PHONY: clean

#.SECONDARY: $(INTERM_SRC)

#-------------------------------------------------------------------------------
# kfg
#
$(ILIBDIR)/libkfg: \
	$(TARGDIR)/default.kfg \
	$(TARGDIR)/certs.kfg \
	$(addprefix $(ILIBDIR)/libkfg.,$(ILIBEXT))

$(TARGDIR)/default.kfg: $(addsuffix /default.kfg,$(NCBIDIR))

$(TARGDIR)/certs.kfg: $(addsuffix /certs.kfg,$(NCBIDIR))

$(addsuffix /default.kfg,$(NCBIDIR)): $(SRCDIR)/default.kfg
	cp $^ $@

$(addsuffix /certs.kfg,$(NCBIDIR)): $(SRCDIR)/certs.kfg
	cp $^ $@

KFG_SRC = \
	config-lex \
	config-grammar \
	config \
	config-aws \
	kart \
	repository \
	properties \
	ngc \
	report-kfg \
    keystore

# flex/bison should only be invoked manually in an environment ensures the correct versions:
# bison 2.5, flex 2.5.35
bison:
	bison -o $(SRCDIR)/config-grammar.c --defines=$(SRCDIR)/config-tokens.h -v --no-lines $(SRCDIR)/config-grammar.y

flex:
	flex --noline -t $(SRCDIR)/config-lex.l > $(SRCDIR)/config-lex.c

#$(SRCDIR)/config-grammar.c $(SRCDIR)/config-tokens.h: $(SRCDIR)/config-grammar.y
#	$(YACC) -o $(SRCDIR)/config-grammar.c --defines=$(SRCDIR)/config-tokens.h $^

KFG_OBJ = \
	$(addsuffix .$(LOBX),$(KFG_SRC))

KFG_LIB = \
	-dkfs \
	-dklib

$(ILIBDIR)/libkfg.$(LIBX): $(KFG_OBJ)
	$(LD) --slib -o $@ $^ $(KFG_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
FUZZ_OBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .fuzz.$(LOBX),$(KFG_SRC)))

fuzz: std $(FUZZ_OBJ)
	$(AR) $(FLIBDIR)/libkfg.$(LIBX) $(FUZZ_OBJ)