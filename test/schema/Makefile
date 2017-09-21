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


default: runtests

TOP ?= $(abspath ../..)

MODULE = test/schema

TEST_TOOLS = \
	wb-test-schema-lex \
	wb-test-schema-parser \
	wb-test-schema-ast \

INT_TOOLS = \
    test-schema-scan \
    test-schema-parse \

ALL_TOOLS = $(TEST_TOOLS) $(INT_TOOLS)

include $(TOP)/build/Makefile.env

$(ALL_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(ALL_TOOLS)

clean: stdclean

TEST_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb \
    -sschema \

#-------------------------------------------------------------------------------
# lexer test
#
TESTLEX_SRC = \
	wb-test-schema-lex

TESTLEX_OBJ = \
	$(addsuffix .$(OBJX),$(TESTLEX_SRC))

$(TEST_BINDIR)/wb-test-schema-lex: $(TESTLEX_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# command line scanner test
#
TESTSCAN_SRC = \
	test-schema-scan

TESTSCAN_OBJ = \
	$(addsuffix .$(OBJX),$(TESTSCAN_SRC))

$(TEST_BINDIR)/test-schema-scan: $(TESTSCAN_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# parser test
#
TESTGRAM_SRC = \
	wb-test-schema-parser

TESTGRAM_OBJ = \
	$(addsuffix .$(OBJX),$(TESTGRAM_SRC))

$(TEST_BINDIR)/wb-test-schema-parser: $(TESTGRAM_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# command line parser test
#
TESTPARSE_SRC = \
	test-schema-parse

TESTPARSE_OBJ = \
	$(addsuffix .$(OBJX),$(TESTPARSE_SRC))

$(TEST_BINDIR)/test-schema-parse: $(TESTPARSE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# AST test
#
TESTAST_SRC = \
	AST_Fixture \
	wb-test-schema-ast \

TESTAST_OBJ = \
	$(addsuffix .$(OBJX),$(TESTAST_SRC))

$(TEST_BINDIR)/wb-test-schema-ast: $(TESTAST_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# runtests
#
runtests: scantest parsetest

SCHEMATA := $(shell find $(TOP) -name "*.vschema")

OUT = $(SRCDIR)/actual/test-schema-scan

scantest: test-schema-scan
	@mkdir -p $(SRCDIR)/actual;
	-$(TEST_BINDIR)/test-schema-scan $(SCHEMATA) >$(OUT)
	#@diff $(OUT) $(SRCDIR)/expected/test-schema-scan
	@echo "Schema scanner test complete"

parsetest: test-schema-parse
	-$(TEST_BINDIR)/test-schema-parse $(SCHEMATA)

.PHONY: scantest parsetest