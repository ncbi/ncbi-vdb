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
    ASTBuilder-func \
    ASTBuilder-tbl \
    ASTBuilder-db \
	ErrorReport \

schema-grammar.$(LOBX): $(SRCDIR)/schema-grammar.cpp
	$(CP) -o $@ -fPIC $(OPT) -D_LIBRARY $< $(LOC_INFO) -D__file_ext__=cpp

bison: grammar ast

grammar: $(SRCDIR)/schema-grammar.cpp

ast: $(SRCDIR)/schema-ast.cpp

#looak at the major version of Bison
BISON_VERSION = $(word 4,$(subst ., ,$(shell bison -V | grep "(GNU Bison)")))

ifeq (1,$(shell echo "3<=$(BISON_VERSION)" | bc -l))

$(SRCDIR)/schema-grammar.cpp: $(SRCDIR)/schema-grammar.y
	bison -r state -o $(SRCDIR)/schema-grammar.cpp --defines=$(SRCDIR)/schema-tokens.h $(SRCDIR)/schema-grammar.y

$(SRCDIR)/schema-ast.cpp: $(SRCDIR)/schema-ast.y
	bison -r state -o $(SRCDIR)/schema-ast.cpp --defines=$(SRCDIR)/schema-ast-tokens.h $(SRCDIR)/schema-ast.y

else

BISON_ERROR = "NOTE: The current Bison version is $(BISON_VERSION). Version 3.0 or newer is required in the PATH. Using the existing $@."
$(SRCDIR)/schema-grammar.cpp: $(SRCDIR)/schema-grammar.y
	@echo $(BISON_ERROR)

$(SRCDIR)/schema-ast.cpp: $(SRCDIR)/schema-ast.y
	@echo $(BISON_ERROR)

endif

schema-lex.$(LOBX): $(SRCDIR)/schema-lex.c
	$(CC)  -Wno-unused-function -o $@ -fPIC $(OPT) -D_LIBRARY $< $(LOC_INFO) -D__file_ext__=c

flex $(SRCDIR)/schema-lex.c: $(SRCDIR)/schema-lex.l $(SRCDIR)/schema-grammar.cpp
	flex -d -t $(SRCDIR)/schema-lex.l > $(SRCDIR)/schema-lex.c

SCHEMA_OBJ = \
	$(addsuffix .$(LOBX),$(SCHEMA_SRC))

$(ILIBDIR)/libschema.$(LIBX):  $(SCHEMA_OBJ)
	$(LD) --slib -o $@ $^

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

.PHONY: bison flex grammar ast