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
MODULE = libs/schema

INT_LIBS = \
    libschema

ALL_LIBS = \
	$(INT_LIBS)

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

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# schema parser library
#
$(ILIBDIR)/libschema: $(addprefix $(ILIBDIR)/libschema.,$(ILIBEXT))

SCHEMA_SRC = \
    schema-grammar \
    schema-lex \
    Token \
    SchemaScanner \
    SchemaParser \
    ParseTree \
    schema-ast \
    AST \
    ASTBuilder \
    ASTBuilder-type \
    ASTBuilder-func \
    ASTBuilder-phys \
    ASTBuilder-tbl \
    ASTBuilder-db \
	ErrorReport \
	ASTBuilder-view \

SCHEMA_OBJ = \
	$(addsuffix .$(LOBX),$(SCHEMA_SRC))

$(ILIBDIR)/libschema.$(LIBX):  $(SCHEMA_OBJ)
	$(LD) --slib -o $@ $^

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
#
# flex/bison for the schema parser
#
# Will not be executed in a regular build. Use 'make bison' and/or 'make flex' to re-generate C files
#

bison: grammar ast

grammar: $(SRCDIR)/schema-grammar.y
	bison -r state -v --no-lines -o $(SRCDIR)/schema-grammar.cpp --defines=$(SRCDIR)/schema-tokens.h $(SRCDIR)/schema-grammar.y

ast: $(SRCDIR)/schema-ast.y
	bison -r state -v --no-lines -o $(SRCDIR)/schema-ast.cpp --defines=$(SRCDIR)/schema-ast-tokens.h $(SRCDIR)/schema-ast.y

flex: $(SRCDIR)/schema-lex.l $(SRCDIR)/schema-grammar.cpp
	flex -d --noline -t $(SRCDIR)/schema-lex.l> $(SRCDIR)/schema-lex.c

.PHONY: bison flex grammar ast

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
